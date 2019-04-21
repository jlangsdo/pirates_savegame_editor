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
#include "PstLine.hpp"

void load_pirate_shipnames();
void save_last_flag(std::string);
std::string save_last_shiptype(const PstLine &);
std::string translate_shipname(const PstLine &);

#endif /* ship_names_hpp */
