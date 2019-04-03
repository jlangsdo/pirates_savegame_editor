//
//  ship_names.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/1/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef ship_names_hpp
#define ship_names_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include "LineDecoding.hpp"
void load_pirate_shipnames();

std::string translate_shipname(info_for_line_decode);
void save_last_flag(std::string);
std::string save_last_shiptype(info_for_line_decode);

const std::vector<std::string> shipname_type_by_class = {
    "MERCHANT SHIPS", "PIRATES",        "WARSHIPS",
    "MERCHANT SHIPS", "MERCHANT SHIPS", "MERCHANT SHIPS",
    "WARSHIPS",       "WARSHIPS",       "MERCHANT SHIPS",
    
    "MERCHANT SHIPS", "WARSHIPS",       "WARSHIPS",
    "MERCHANT SHIPS", "MERCHANT SHIPS", "MERCHANT SHIPS",
    "WARSHIPS",       "WARSHIPS",       "MERCHANT SHIPS",
    
    "MERCHANT SHIPS", "WARSHIPS",       "WARSHIPS",
    "MERCHANT SHIPS", "MERCHANT SHIPS", "MERCHANT SHIPS",
    "WARSHIPS",       "WARSHIPS",       "MERCHANT SHIPS"
};

#endif /* ship_names_hpp */
