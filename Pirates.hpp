//
//  Pirates.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 3/30/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef Pirates_hpp
#define Pirates_hpp

#include <stdio.h>
#include <string>

// Filename suffixes
const std::string pg  = "pirates_savegame";
const std::string pst = "pst";

std::string find_file(std::string dir, std::string file, std::string suffix);
void unpack_pg_to_pst(std::string pg, std::string pst);
void pack_pst_to_pg(std::string pst, std::string pg);

#endif /* Pirates_hpp */
