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
#include <unordered_map>
#include <boost/ptr_container/ptr_deque.hpp>
#include <list>

enum rmeth : char { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO, FEATURE };

// The translation types have one (or two) character abbreviations in the pst file.
const std::unordered_map<rmeth,std::string> char_for_method = {
    {TEXT0,"t"}, {TEXT8,"t"}, {INT, "V"}, {HEX, "h"}, {BINARY, "B"}, {SHORT, "s"}, {CHAR, "C"}, {FMAP, "M"}, {BULK, "H"},
    {ZERO,"x"}, {uFLOAT,"G"}, {mFLOAT, "g"},{LCHAR, "c"}, {SMAP, "m"}, {CMAP, "MM"}, {FEATURE, "F"},
};

// And many of them have a typical or required size.
const std::unordered_map<rmeth,char> standard_rmeth_size = {
    {TEXT0,0}, {TEXT8,8},
    {INT,4}, {HEX,4},  {uFLOAT,4},  {mFLOAT, 4},
    {SHORT,2}, {CHAR,1},{LCHAR, 1},{BINARY,1},
    {BULK, 4},  {ZERO,0}, {FEATURE,1},
};

bool constexpr is_world_map(rmeth m) {
    // world maps get special handling.
    return (m==SMAP || m==CMAP || m==FMAP);
}

#endif /* Pirates_hpp */
