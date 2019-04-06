//
//  ship_names.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/1/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef ship_names_hpp
#define ship_names_hpp

#include <string>
#include "Pirates.hpp"

void load_pirate_shipnames();
void save_last_flag(std::string);
std::string save_last_shiptype(info_for_line_decode);
std::string translate_shipname(info_for_line_decode);

#endif /* ship_names_hpp */
