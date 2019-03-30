//
//  main.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 3/29/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include <iostream>
#include <getopt.h>
using namespace std;

const string usage = R"USAGE(

Basic switches:
-unpack <>      pirates_savegame -> pst
-pack <>        pst -> pirates_savegame
-advanced_help  More information, including splicing switches

The pst file is a text file which contains all of the information
required to rewrite the original pirates savegame.

After running unpack on a pirates_savegame file,
you can edit the pst file in any text editor,
then run -pack to rebuild the pirates_savegame file.

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
        {"help",          no_argument, 0, 'h'},
        {"pack",    required_argument, 0, 'p'},
        {"unpack",  required_argument, 0, 'u'},
    };
    while (1) {
        int option_index = 0;
        c = getopt_long_only (argc, argv, "", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
            case 'a':  cout << script_name << ausage; return 0;
            case 'h':  cout << script_name << usage; return 0;
            case 'p' : cout << "Packing " << optarg << "\n"; break;
            case 'u' : cout << "Unpacking " << optarg << "\n"; break;
            default: abort();
        }
    }

}
