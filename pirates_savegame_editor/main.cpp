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
#include "PGetoptLong.hpp"

// This file handles processing the input switches.

using namespace std;

string save_dir;  // global var to avoid passing it to every read/write routine in PiratesFiles.

int main(int argc, char **argv)
{
    auto opt = PGetOptions(argc, argv, {
        "advanced_help",
        "auto",
        "clone=s",
        "dir=s",
        "donor=s",
        "in=s",
        "not=s",
        "out=s",
        "pack=s",
        "set=s",
        "splice=s",
        "sweep",
        "test=s",
        "unpack=s"
    });
    
    // Get the pirates module ready to go.
    set_up_decoding();
    
    // Setting the default pirates savegame dir.
    string env_user = "USER_NOT_DEFINED";
    if(const char* env_p = std::getenv("USER")) { env_user = env_p; }
    save_dir = "/Users/" + env_user + "/Library/Preferences/Firaxis Games/Sid Meier's Pirates!/My Games/Game";
    
   
    if (opt.count("auto") && opt.count("splice")) throw invalid_argument("Do not combine -splice and -auto");
    // -auto is implied by -not and by using commas in -donor.
    if (opt.count("not") || ( opt.count("donor") && opt["donor"].find(",")!=string::npos) ) {
        opt["auto"] = "1";
        if (opt.count("splice")) throw invalid_argument("-not or multiple -donor files implies -auto. Do not combine -auto with -splice");
    }
    
    if (opt.count("unpack")) {
        auto list = split_by_commas(opt["unpack"]);
        for (auto afile : list) {
            unpack(afile);  // takes a short filename.
        }
    } else if (opt.count("pack")) {
        auto list = split_by_commas(opt["pack"]);
        for (auto afile : list) {
            pack(afile);
        }
    } else if (opt.count("test") || opt.count("sweep")) {
        vector<std::string> list;
        if (opt.count("sweep")) {
            list = find_pg_files();
        }
        if (opt.count("test")) { // I'm not sure it makes sense to -sweep and -test, but you could do it if -test had full paths.
            auto tlist = split_by_commas(opt["test"]);
            list.insert(list.end(), tlist.begin(), tlist.end());
        }
        for (auto afile : list) {
            unpack(afile);
            testpack(afile);
            comparePg(afile);
        }
    } else if (opt.count("in") && opt.count("out") && opt.count("splice")) {
        if (opt.count("donor") && opt.count("set"))   throw invalid_argument("Do not use -donor and -set together");
        if (opt.count("donor") && opt.count("clone")) throw invalid_argument("Do not use -donor and -clone together");
        if (opt.count("clone") && opt.count("set"))   throw invalid_argument("Do not use -clone and -set together");
        splice(opt["in"], opt["donor"], opt["out"], opt["splice"], opt["clone"], opt["set"], opt["not"]);
    } else if (opt.count("in") && opt.count("out") && opt.count("donor") && opt.count("auto")) {
        auto_splice(opt["in"], opt["donor"], opt["out"], opt["not"]);
    } else {
        cout << "Unrecognized combination of options.\n";
    }
    
    exit(0);
}


