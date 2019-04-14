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
#include "PiratesFiles.hpp"

// This file handles processing the input switches and opening filehandles.
// Other than the help messages, it knows nothing about the structure of the savegame file.

using namespace std;

string save_dir;  // global var to avoid passing it to every routine in PiratesFiles.

int main(int argc, char **argv)
{    int c;
    const string script_name = argv[0];
    const static struct option long_options[] =
    {
        {"advanced_help", no_argument, 0, 'a'},
        {"auto",          no_argument, 0, 'b'},
        {"clone",   required_argument, 0, 'c'},
        {"dir",     required_argument, 0, 'd'},
        {"donor",   required_argument, 0, 'r'},
        
        {"help",          no_argument, 0, 'h'},
        {"in",      required_argument, 0, 'i'},
        {"not",     required_argument, 0, 'n'},
        {"out",     required_argument, 0, 'o'},
        {"pack",    required_argument, 0, 'p'},
        
        {"set",     required_argument, 0, 'e'},
        {"splice",  required_argument, 0, 's'},
        {"sweep",         no_argument, 0, 'w'},
        {"test",    required_argument, 0, 't'},
        {"unpack",  required_argument, 0, 'u'},
    };
    
    string unpackfiles, packfiles, testfiles, infile, outfiles, notfiles, donorfiles;
    string spliceregex, cloneregex, setregex;
    bool do_sweep = false, do_auto = false;
    
    // Get the pirates module ready to go.
    set_up_decoding();
    
    // Actually parse the options. Seems awfully redundant to have to set up the long_options and then switch through the results.
    // But, except for -help and -advanced_help, we have to process all of the options
    // before doing anything, because they come in combinations.
    
    while (1) {
        int option_index = 0;
        c = getopt_long_only (argc, argv, "", long_options, &option_index);
        if (c == -1) break;
        switch (c) {
            case 'a':  print_advanced_help();
            case 'b':  do_auto = true; break;
            case 'c':  cloneregex = optarg; break;
            case 'd':  save_dir = optarg; break;
            case 'e':  setregex = optarg; break;
                
            case 'h':  print_help();
            case 'i':  infile = optarg; break;
            case 'n':  notfiles = optarg; break;
            case 'o':  outfiles = optarg; break;
            case 'p':  packfiles = optarg; break;
                
            case 'r':  donorfiles = optarg; break;
            case 's':  spliceregex = optarg; break;
            case 'w':  do_sweep = true; break;
            case 't':  testfiles = optarg; break;
            case 'u':  unpackfiles = optarg; break;
            default: abort();
        }
    }
    
    if (do_auto && spliceregex != "") throw invalid_argument("Do not combine -splice and -auto");
    // -auto is implied by -not and by using commas in -donor.
    if (notfiles != "" || donorfiles.find(",")!=string::npos) {
        do_auto = true;
        if (spliceregex != "") throw invalid_argument("-not or multiple -donor files implies -auto. Do not combine -auto with -splice");
    }
    
    //for (auto i=0; i<10;i++) { // Loop for profiling
    if (unpackfiles.size()) {
        auto list = split_by_commas(unpackfiles);
        for (auto afile : list) {
            unpack(afile);
        }
    } else if (packfiles.size()) {
        auto list = split_by_commas(packfiles);
        for (auto afile : list) {
            pack(afile);
        }
    } else if (testfiles.size() || do_sweep) {
        vector<std::string> list;
        if (do_sweep) {
            list = find_pg_files();
        }
        if (testfiles.size()) { // I'm not sure it makes sense to -sweep and -test, but you could do it if -test had full paths.
            auto tlist = split_by_commas(testfiles);
            list.insert(list.end(), tlist.begin(), tlist.end());
        }
        for (auto afile : list) {
            unpack(afile);
            testpack(afile);
            comparePg(afile);
        }
    } else if (infile != "" && outfiles != "" && spliceregex != "") {
        if (donorfiles != "" && setregex != "")   throw invalid_argument("Do not use -donor and -set together");
        if (donorfiles != "" && cloneregex != "") throw invalid_argument("Do not use -donor and -clone together");
        if (cloneregex != "" && setregex != "")   throw invalid_argument("Do not use -clone and -set together");
        
        cout << "Setting up to splice\n";
        splice(infile, donorfiles, outfiles, spliceregex, cloneregex, setregex, do_auto, notfiles);
    } else if (infile != "" && outfiles != "" && donorfiles !="" && do_auto) {
        cout << "Setting up auto-splice\n";
        auto_splice(infile, donorfiles, outfiles, notfiles);
    } else {
        cout << "Unrecognized combination of options.\n";
    }
    //}
    exit(0);
}


