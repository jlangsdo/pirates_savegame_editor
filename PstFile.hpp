//
//  PstFile.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/8/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef PstFile_hpp
#define PstFile_hpp

#include <stdio.h>
#include <boost/ptr_container/ptr_map.hpp>
#include <string>
#include <map>
#include <vector>
#include "PstSection.hpp"
#include "PstLine.hpp"

void packPst(std::ifstream & in, std::ofstream & out);
void compare_binary_filestreams(std::ifstream & in1, std::ifstream & in2);

class PstFile {
public:
    void read_text(std::ifstream & i);
    void write_pg(std::ofstream & i);
private:
    void remove_features();
    void apply_features();
    std::unordered_map<std::string, std::map<unsigned long long, std::unique_ptr<PstLine> > >  data;
};


#endif /* PstFile_hpp */
