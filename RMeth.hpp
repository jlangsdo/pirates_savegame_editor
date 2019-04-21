//
//  RMeth.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 3/30/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef Pirates_hpp
#define Pirates_hpp

// This file holds the rmeth enum used to describe different types of data that could be read into a PstLine.

#include <string>


enum rmeth : char           {TEXT, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO, FEATURE };
extern const int standard_rmeth_size[];
extern const std::string char_for_meth[];

bool constexpr is_world_map(rmeth m) {      // world maps get special handling.
    return (m==SMAP || m==CMAP || m==FMAP);
}
rmeth meth_for_char(std::string chars);
void set_up_rmeth();

#endif /* Pirates_hpp */
