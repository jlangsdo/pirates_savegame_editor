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

void unpackPst(std::ifstream & in, std::ofstream & out);
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
    std::list<std::string> lca = {};   // line_code_aliases
    
    PstSection(std::string n, int c, int b, rmeth meth) : name(n), splits{PstSplit(meth, b, c)}, lca({n}) {};
    PstSection(std::string n, int c, int b)             : name(n), splits{PstSplit(BULK, b, c)}, lca({n}) {};
    PstSection(std::string n, PstSplit split)           : name(n), splits{split}, lca({n}) {};
    PstSection(const PstSection & parent, int c, PstSplit split)         :  splits{split} {
        std::string underscore_c = "_" + std::to_string(c);
        name = parent.name + underscore_c;
        
        for (auto a : parent.lca) {
            lca.emplace_back(a + underscore_c);
        }
        if (lca.size() < 3) {
            lca.emplace_back(parent.lca.back() + "_x");
        }
    };
    void unpack(std::ifstream & in, std::ofstream & out);
};
extern const std::vector<PstSection> section_vector;

#endif /* SectionSplitting_hpp */
