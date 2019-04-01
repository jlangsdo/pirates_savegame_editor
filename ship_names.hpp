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

void load_pirate_shipnames();
std::string translate_shipname(std::string value);
std::string save_last_flag(std::string value);
std::string save_last_shiptype(std::string value);


enum shipname_types { MERCHANT, WARSHIP,  PIRATE};

const std::vector<shipname_types> shipname_type_by_class = {
    MERCHANT, PIRATE,   WARSHIP,
    MERCHANT, MERCHANT, MERCHANT,
    WARSHIP,  WARSHIP,  MERCHANT,
    
    MERCHANT, WARSHIP,  WARSHIP,
    MERCHANT, MERCHANT, MERCHANT,
    WARSHIP,  WARSHIP,  MERCHANT,
    
    MERCHANT, WARSHIP,  WARSHIP,
    MERCHANT, MERCHANT, MERCHANT,
    WARSHIP,  WARSHIP,  MERCHANT
};

#endif /* ship_names_hpp */
