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

using namespace std;

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
