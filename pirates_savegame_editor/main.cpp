//
//  main.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 3/29/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <getopt.h>
#include <regex>
#include <cstdio>
#include "boost/filesystem.hpp"
#include "RMeth.hpp"
#include "PstSection.hpp"
#include "PstLine.hpp"
#include "PstFile.hpp"
#include "ship_names.hpp"

// This file handles processing the input switches and opening filehandles.
// Other than the help messages, it knows nothing about the structure of the savegame file.

using namespace std;

string find_file(string dir, string file, string suffix);
void unpack_pg_to_pst(string pg_file, string pst_file);
void pack_pst_to_pg(string pst_file, string pg_file);
void test_file_to_test(string save_dir, string file_to_test);
vector<std::string> find_all_files(string dir, string suffix);

const string usage = R"USAGE(

Basic switches:
-dir <>         Directory to find pirates_savegame files
-unpack <>      pirates_savegame file(s) to be unpacked to a pst
-pack <>        pst file(s) to pack into pirates_savegame
-advanced_help  More information, including splicing switches

The pst file is a text file which contains all of the information
required to rewrite the original pirates savegame.

After running -unpack on a pirates_savegame file,
you can edit the pst file in any text editor,
then run -pack to rebuild the pirates_savegame file.
Leave off the file extension.

The advanced switches give you ways to splice parts of one
pst file into another without a text editor.

)USAGE";

const string ausage = R"AUSAGE(

The advanced switches have not been implemented yet.


)AUSAGE";

// Filename suffixes
const std::string pg_suffix   = "pirates_savegame";
const std::string pst_suffix  = "pst";
const std::string test_suffix = pg_suffix + ".test";

int main(int argc, char **argv)
{    int c;
    const string script_name = argv[0];
    const static struct option long_options[] =
    {
        {"advanced_help", no_argument, 0, 'a'},
        {"dir",     required_argument, 0, 'd'},
        {"help",          no_argument, 0, 'h'},
        {"pack",    required_argument, 0, 'p'},
        {"sweep",         no_argument, 0, 's'},
        {"test",    required_argument, 0, 't'},
        {"unpack",  required_argument, 0, 'u'},
    };
    
    string unpack;
    string pack;
    string test;
    string save_dir = "/usr";
    bool do_sweep = false;
    
    // Get the pirates module ready to go.
    set_up_rmeth();
    augment_decoder_groups();
    load_pirate_shipnames();
    
    while (1) {
        int option_index = 0;
        c = getopt_long_only (argc, argv, "", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
            case 'a':  cout << script_name << ausage; return 0;
            case 'd': save_dir = optarg; break;
            case 'h':  cout << script_name << usage; return 0;
            case 'p': pack = optarg; break;
            case 's' : do_sweep = true; break;
            case 't': test = optarg; break;
            case 'u': unpack = optarg; break;
            default: abort();
        }
    }
    // Except for -help and -advanced_help, we have to process all of the options
    // before doing anything. -dir might come at the end of the list, and it affects all of the others.
    
    // pack, test, and unpack all allow a comma-separated list of files to process.
    if (pack.size()) {
        string file_to_pack;
        std::istringstream tokenStream(pack);
        while(std::getline(tokenStream, file_to_pack, ',')) {
            string pst_file = find_file(save_dir, file_to_pack, pst_suffix);
            string pg_file = regex_replace(pst_file, regex("\\." + pst_suffix + "$"), "." + pg_suffix);
            pack_pst_to_pg(pst_file, pg_file);
        }
    }
    if (do_sweep) {
        auto file_list = find_all_files(save_dir, pg_suffix);
        cout << "Preparing to unpack an test " << file_list.size() << " files\n";
        for (auto afile : file_list) {
            test_file_to_test(save_dir, afile);
        }
        cout << "\nAll savegame files unpacked and regressions passed\n";
    }
    
    if (test.size()) {
        string file_to_test;
        std::istringstream tokenStream(test);
        while(std::getline(tokenStream, file_to_test, ',')) {
            test_file_to_test(save_dir, file_to_test);
        }
    }
    if (unpack.size()) {
        string file_to_unpack;
        std::istringstream tokenStream(unpack);
        while(std::getline(tokenStream, file_to_unpack, ',')) {
            string pg_file = find_file(save_dir, file_to_unpack, pg_suffix);
            string pst_file = regex_replace(pg_file, regex("\\." + pg_suffix + "$"), "." + pst_suffix);
            unpack_pg_to_pst(pg_file, pst_file);
        }
    }
}

void compare_binary_files(std::string file1, std::string file2) {
    ifstream fs1 = ifstream(file1, ios::binary);   // Looks like opend()
    if (! fs1.is_open()) {
        cerr << "Failed to read from " << file1 << "\n";
        exit(1);
    }
    ifstream fs2 = ifstream(file2, ios::binary);
    if (! fs2.is_open()) {
        cerr << "Failed to read from " << file2 << "\n";
        exit(1);
    }
    cout << "Comparing " << file1 << " to " << file2 << "\n";
    compare_binary_filestreams(fs1,fs2);
    fs1.close();
    fs2.close();
}

void test_file_to_test(string save_dir, string file_to_test) {
    string pg_file = find_file(save_dir, file_to_test, pg_suffix);
    string pst_file = regex_replace(pg_file, regex("\\." + pg_suffix + "$"), "." + pst_suffix);
    unpack_pg_to_pst(pg_file, pst_file);

    string test_file = regex_replace(pst_file, regex("\\." + pst_suffix + "$"), "." + test_suffix);
    pack_pst_to_pg(pst_file, test_file);
    
    compare_binary_files(pg_file, test_file);  // This routine will abort() on failure.
    std::remove(test_file.c_str());
    cout << "PASS!\n";
}

string find_file(string dir, string file, string suffix) {
    // finds a file that is to be packed or unpacked
    // returns the full pathname of the file.
    string game = file;
    game = regex_replace(game, regex("\\." + pst_suffix), "");
    game = regex_replace(game, regex("\\." +  pg_suffix), "");
    
    const string possible_files[] = {
        game + "." + suffix,
        dir + "/" + game + "." + suffix,
    };
    for (auto afile : possible_files) {
        ifstream f(afile.c_str());
        if (f.good()) {
            return afile;
        }
    }
    cout << "Could not find file " << file << "\n";
    cout << "Looked for: \n";
    for (auto afile : possible_files) {
        cout << "  '" << afile << "'\n";
    }
    exit(1);
}

void unpack_pg_to_pst(string pg_file, string pst_file) {
    ifstream pg_in = ifstream(pg_file, ios::binary);
    if (! pg_in.is_open()) {
        cerr << "Failed to read from " << pg_file << "\n";
        exit(1);
    }
    cout << "Unpacking " << pg_file << "\n";
    
    ofstream pst_out = ofstream(pst_file);
    if (! pst_out.is_open()) {
        cerr << "Failed to write to " << pst_file << "\n";
        pg_in.close();
        exit(1);
    }
    cout << "Writing " << pst_file << "\n\n";
    
    unpack(pg_in, pst_out);
    pg_in.close();
    pst_out.close();
}

void pack_pst_to_pg(string pst_file, string pg_file) {
    ifstream pst_in = ifstream(pst_file);
    if (! pst_in.is_open()) {
        cerr << "Failed to read from " << pst_file << "\n";
        exit(1);
    }
    cout << "Reading " << pst_file << "\n";
    
    ofstream pg_out = ofstream(pg_file);
    if (! pg_out.is_open()) {
        cerr << "Failed to write to " << pg_file << "\n";
        pst_in.close();
        exit(1);
    }
    cout << "Writing " << pg_file << "\n\n";
    
    pack(pst_in, pg_out);
    pst_in.close();
    pg_out.close();
}

vector<std::string> find_all_files(string dir, string suffix) {
    using namespace boost::filesystem;
    using namespace boost;
    vector<std::string> results;
    path dir_path(dir);
    if (is_directory(dir_path)) {
        for (directory_entry& x : directory_iterator(dir_path)) {
            string afile = x.path().string();
            if (regex_search(afile, regex("." + suffix + "$"))) {
                results.push_back(afile);
            }
        }
    }
    return results;
}
