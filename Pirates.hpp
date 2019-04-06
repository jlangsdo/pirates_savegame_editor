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

enum translatable : char;
enum translation_type : char { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, LCHAR, mFLOAT, uFLOAT, FMAP, SMAP, CMAP, BULK, ZERO };

struct decode_for_line {
    std::string comment = "";
    translatable t ;
};

struct info_for_line_decode {
    std::string value;
    int v;                // value reduced to an integer
    std::string line_code;
};


#endif /* Pirates_hpp */
