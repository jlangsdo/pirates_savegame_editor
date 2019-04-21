//
//  Rmeth.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/9/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "RMeth.hpp"
#include <string>
using namespace std;

//enum rmeth : char               {TEXT, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO, FEATURE };
const int standard_rmeth_size[] = { 0,    4,   4,   1,      2,     1,    1,     4,      4,      0,    0,    0,    4,    0,    1       };
const std::string char_for_meth[]={"t",  "h", "V", "B",    "s",   "C",  "c",   "g",    "G",    "M",  "m",  "MM", "H",  "x",  "F"      };

rmeth meth_for_most_char[256];
void set_up_rmeth() {
    for (char i=0; i<20; ++i) {
        if (char_for_meth[i].length()==1) {
            meth_for_most_char[char_for_meth[i][0]] = (rmeth)i;
        }
    }
}
rmeth meth_for_char(std::string chars) {  // Reverse of char_for_meth
    if (chars.length()==2) { return CMAP; }
    return meth_for_most_char[chars[0]];
}
