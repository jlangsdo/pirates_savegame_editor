//
//  Pirates.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 3/30/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef Pirates_hpp
#define Pirates_hpp

// This file holds the structs and enums used to pass data betweeen the different modules,
// so the modules do not need to include one another's files.

#include <string>
#include <map>

enum translatable : char;
enum translation_type : char { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO };

// The translation types have one (or two) character abbreviations in the pst file.
const std::map<translation_type,std::string> char_for_method = {
    {TEXT0,"t"}, {TEXT8,"t"}, {INT, "V"}, {HEX, "h"}, {BINARY, "B"}, {SHORT, "s"}, {CHAR, "C"}, {FMAP, "M"}, {BULK, "H"},
    {ZERO,"x"}, {uFLOAT,"G"}, {mFLOAT, "g"},{LCHAR, "c"}, {SMAP, "m"}, {CMAP, "MM"},
};

// And many of them have a typical or required size.
const std::map<translation_type,char> size_for_method = {
    {TEXT0,0}, {TEXT8,8},
    {INT,4}, {HEX,4},  {uFLOAT,4},  {mFLOAT, 4},
    {SHORT,2}, {CHAR,1},{LCHAR, 1},{BINARY,1},
    {BULK, 4},  {ZERO,0},
};

struct decode_for_line {
    std::string comment = "";
    translatable t ;
};

class PstLine {
public:
    std::string line_code;
    int v;                // value reduced to a small integer for lookups
    std::string value;
    PstLine(std::string lc, int v, std::string value) : line_code(lc), v(v), value(value) {}
    PstLine(std::string lc) : line_code(lc) {}
};


#endif /* Pirates_hpp */
