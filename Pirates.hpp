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
#include <boost/ptr_container/ptr_deque.hpp>

enum translatable : char;
enum rmeth : char { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO, FEATURE };

// The translation types have one (or two) character abbreviations in the pst file.
const std::map<rmeth,std::string> char_for_method = {
    {TEXT0,"t"}, {TEXT8,"t"}, {INT, "V"}, {HEX, "h"}, {BINARY, "B"}, {SHORT, "s"}, {CHAR, "C"}, {FMAP, "M"}, {BULK, "H"},
    {ZERO,"x"}, {uFLOAT,"G"}, {mFLOAT, "g"},{LCHAR, "c"}, {SMAP, "m"}, {CMAP, "MM"}, {FEATURE, "F"},
};

// And many of them have a typical or required size.
const std::map<rmeth,char> standard_rmeth_size = {
    {TEXT0,0}, {TEXT8,8},
    {INT,4}, {HEX,4},  {uFLOAT,4},  {mFLOAT, 4},
    {SHORT,2}, {CHAR,1},{LCHAR, 1},{BINARY,1},
    {BULK, 4},  {ZERO,0}, {FEATURE,1},
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
    rmeth method;
    int bytes = standard_rmeth_size.at(method);
    
    PstLine(std::string lc, rmeth rm, int v, std::string value) : line_code(lc), method(rm), v(v), value(value) {}
    PstLine(std::string lc) : line_code(lc) {}
    PstLine(std::string lc, rmeth rm, int b) : line_code(lc), method(rm), bytes(b) {}
    
    
    std::string mcode();
    void print (std::ofstream &out);
    std::string get_comment();
    std::string get_translation();
};

struct PstSplit {
    rmeth method;
    int bytes=standard_rmeth_size.at(method);
    int count=1;
    PstSplit(rmeth meth, int b, int c) : method(meth), bytes(b), count(c) {};
    PstSplit(rmeth meth, int b)        : method(meth), bytes(b)           {};
    PstSplit(rmeth meth)               : method(meth)                     {};
};


class PstSection { // This could become a name + subsection_info.
public:
    std::string name;
    PstSplit split;
    PstSection(std::string n, int c, int b, rmeth meth) : name(n), split(meth, b, c) {};
    PstSection(std::string n, int c, int b)             : name(n), split(BULK, b, c) {};
};


#endif /* Pirates_hpp */
