//
//  SectionSplitting.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/5/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef SectionSplitting_hpp
#define SectionSplitting_hpp

#include <fstream>
#include <string>
#include "RMeth.hpp"

void unpack(std::ifstream & in, std::ofstream & out);
int index_from_linecode (std::string line_code);

struct PstSplit {
    rmeth method;
    int bytes=standard_rmeth_size.at(method);
    int count=1;
    PstSplit(rmeth meth, int b, int c) : method(meth), bytes(b), count(c) {};
    PstSplit(rmeth meth, int b)        : method(meth), bytes(b)           {};
    PstSplit(rmeth meth)               : method(meth)                     {};
};


class PstSection {
public:
    std::string name;
    std::list<PstSplit> splits;
    
    PstSection(std::string n, int c, int b, rmeth meth) : name(n), splits{PstSplit(meth, b, c)} {};
    PstSection(std::string n, int c, int b)             : name(n), splits{PstSplit(BULK, b, c)} {};
    PstSection(std::string n, PstSplit split)           : name(n), splits{split} {};
    PstSection(std::string n, std::list<PstSplit> splits)    : name(n), splits(splits) {};
    
    void unpack(std::ifstream & in, std::ofstream & out);
};

#endif /* SectionSplitting_hpp */
