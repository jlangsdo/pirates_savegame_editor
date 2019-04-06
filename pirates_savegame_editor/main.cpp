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
#include "Pirates.hpp"
#include "ship_names.hpp"
#include "SectionSplitting.hpp"
#include "LineDecoding.hpp"

// This file handles processing the input switches and opening filehandles.
// Other than the help messages, it knows nothing about the structure of the savegame file.

using namespace std;

string find_file(string dir, string file, string suffix);
void unpack_pg_to_pst(string pg_file, string pst_file);

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
const std::string pg  = "pirates_savegame";
const std::string pst = "pst";

int main(int argc, char **argv)
{    int c;
    const string script_name = argv[0];
    const static struct option long_options[] =
    {
        {"advanced_help", no_argument, 0, 'a'},
        {"dir",     required_argument, 0, 'd'},
        {"help",          no_argument, 0, 'h'},
        {"pack",    required_argument, 0, 'p'},
        {"unpack",  required_argument, 0, 'u'},
    };
    
    string unpack;
    string pack;
    string save_dir = "/usr";
    
    // Get the pirates module ready to go.
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
            case 'u': unpack = optarg; break;
            default: abort();
        }
    }
    if (unpack.size()) {
        string file_to_unpack;
        std::istringstream tokenStream(unpack);
        while(std::getline(tokenStream, file_to_unpack, ',')) {
            string pg_file = find_file(save_dir, file_to_unpack, pg);
            string pst_file = regex_replace(pg_file, regex("\\." + pg + "$"), "." + pst);
            unpack_pg_to_pst(pg_file, pst_file);
        }
    }
    if (pack.size()) {
        cout << "Packing " << pack << "\n";
    }
}

string find_file(string dir, string file, string suffix) {
    // finds a file that is to be packed or unpacked
    // returns the full pathname of the file.
    string game = file;
    game = regex_replace(game, regex("\\." + pst), "");
    game = regex_replace(game, regex("\\." +  pg), "");
    
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
