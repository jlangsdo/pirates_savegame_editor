//
//  PiratesFiles.cpp
//  
//
//  Created by Langsdorf on 4/10/19.
//
// This module has the file handling routines so the calls in main() can be as straightforward as possible.


#include "PiratesFiles.hpp"
#include "PstFile.hpp"
#include "ship_names.hpp"
#include "PstLine.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include "boost/filesystem.hpp"

// Filename suffixes
const std::string pg_suffix   = "pirates_savegame";
const std::string pst_suffix  = "pst";
const std::string test_suffix = pg_suffix + ".test";

extern std::string save_dir;

using namespace std;

void print_help() {
    
    const string help_message = R"USAGE(
    
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
    cout << help_message;
    exit(0);
}

vector<std::string> split_by_commas(std::string arg) {
    vector<std::string> results;
    string temp;
    std::istringstream tokenStream(arg);
    while(std::getline(tokenStream, temp, ',')) {
        results.push_back(temp);
    }
    return results;
}

void print_advanced_help() {
    const string advanced_help_message = R"AUSAGE(
    
    The advanced switches have not been implemented yet.
    
    
    )AUSAGE";
    cout << advanced_help_message;
    exit(0);
    
}

void set_up_decoding() {
    // Initialization of some variables that is done at runtime, mainly for unpack().
    set_up_rmeth();
    augment_decoder_groups();
    load_pirate_shipnames();
}

void comparePg(std::string afile) {
    string file1 = find_file(afile, pg_suffix);
    ifstream fs1 = ifstream(file1, ios::binary);   // Looks like opend()
    if (! fs1.is_open()) {
        cerr << "Failed to read from " << file1 << "\n";
        exit(1);
    }
    
    string file2 = find_file(afile, test_suffix);
    ifstream fs2 = ifstream(file2, ios::binary);
    if (! fs2.is_open()) {
        cerr << "Failed to read from " << file2 << "\n";
        exit(1);
    }
    cout << "Comparing " << file1 << " to " << file2 << "\n";
    compare_binary_filestreams(fs1,fs2); // Aborts on failure.
    fs1.close();
    fs2.close();
    cout << "PASS!\n";
    std::remove(file2.c_str());
}

string find_file(string game, string suffix) {
    // Remove suffix.
    game = regex_replace(game, regex("\\.[^\\/]+$"), "");
    
    // Look for the game with the right suffix. It should either have
    // come with a full path, or be in the save_dir.
    const string possible_files[] = {
        game + "." + suffix,
        save_dir + "/" + game + "." + suffix,
    };
    for (auto afile : possible_files) {
        ifstream f(afile.c_str());
        if (f.good()) {
            return afile;
        }
    }
    cout << "Could not find file " << game << "\n";
    cout << "Looked for: \n";
    for (auto afile : possible_files) {
        cout << "  '" << afile << "'\n";
    }
    exit(1);
}

void unpack(std::string afile) {
    string pg_file = find_file(afile, pg_suffix);
    ifstream pg_in = ifstream(pg_file, ios::binary);
    if (! pg_in.is_open()) {
        cerr << "Failed to read from " << pg_file << "\n";
        exit(1);
    }
    cout << "Unpacking " << pg_file << "\n";
    
    string pst_file = regex_replace(pg_file, regex(pg_suffix + "$"), pst_suffix);
    ofstream pst_out = ofstream(pst_file);
    if (! pst_out.is_open()) {
        cerr << "Failed to write to " << pst_file << "\n";
        pg_in.close();
        exit(1);
    }
    cout << "Writing " << pst_file << "\n\n";
    
    unpackPst(pg_in, pst_out);
    pg_in.close();
    pst_out.close();
}

void testpack(string afile) {  pack(afile, test_suffix); }
void pack(string afile)     {  pack(afile, pg_suffix); }
void pack(string afile, string suffix) {
    string pst_file = find_file(afile, pst_suffix);
    ifstream pst_in = ifstream(pst_file);
    if (! pst_in.is_open()) {
        cerr << "Failed to read from " << pst_file << "\n";
        exit(1);
    }
    cout << "Reading " << pst_file << "\n";
    
    string pg_file = regex_replace(pst_file, regex(pst_suffix + "$"), suffix);
    ofstream pg_out = ofstream(pg_file);
    if (! pg_out.is_open()) {
        cerr << "Failed to write to " << pg_file << "\n";
        pst_in.close();
        exit(1);
    }
    cout << "Writing " << pg_file << "\n\n";
    
    packPst(pst_in, pg_out);
    pst_in.close();
    pg_out.close();
}

std::vector<std::string> find_pg_files() {
    using namespace boost::filesystem;
    using namespace boost;
    vector<std::string> results;
    path dir_path(save_dir);
    if (is_directory(dir_path)) {
        for (directory_entry& x : directory_iterator(dir_path)) {
            string afile = x.path().string();
            if (regex_search(afile, regex("." + pg_suffix + "$"))) {
                results.push_back(afile);
            }
        }
    }
    return results;
}
void splice(std::string infile, std::string donor, std::string outfiles,
            std::string splice, std::string clone, std::string set, bool do_auto,
            std::string notfiles) {
    
}
