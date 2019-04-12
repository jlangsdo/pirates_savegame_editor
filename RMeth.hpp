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
#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/bimap.hpp"
#include "boost/assign.hpp"
#include <list>


enum rmeth : char { TEXT, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO, FEATURE };

// And many of them have a typical or required size.
const std::unordered_map<rmeth,char> standard_rmeth_size = {
    {TEXT, 8},
    {INT,4}, {HEX,4},  {uFLOAT,4},  {mFLOAT, 4},
    {SHORT,2}, {CHAR,1},{LCHAR, 1},{BINARY,1},
    {BULK, 4},  {ZERO,0}, {FEATURE,1},
};

bool constexpr is_world_map(rmeth m) {
    // world maps get special handling.
    return (m==SMAP || m==CMAP || m==FMAP);
}

// They also have short names (most derived from perl 'pack' characters) that are used as identifiers in the pst file
typedef boost::bimap<rmeth, std::string> rmethstrings;
extern rmethstrings char_for_method;
void set_up_rmeth();

#endif /* Pirates_hpp */
