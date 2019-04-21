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
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>
#include "PstSection.hpp"
#include "PstLine.hpp"
#include "PiratesFiles.hpp"

using Sortcode = unsigned long long;

void compare_binary_filestreams(std::ifstream & in1, std::ifstream & in2);
Sortcode index_to_sortcode(std::string numbers);

class PstFile {
public:
    std::string filename;
    void read_text(std::ifstream & i);
    void read_text(std::string afile, std::string suffix);
    void write_pg(std::ofstream & i);
    PstFile() {}
    explicit PstFile(std::string afile, std::string suffix=pst_suffix) { read_text(afile, suffix); }
    
    //     Map   of    sections ->  map of sortnum -> PstLine
    std::unordered_map<std::string, std::map<Sortcode, std::unique_ptr<PstLine> > >  data;
    
    // Syntactic Sugar
    std::map<Sortcode, std::unique_ptr<PstLine> > & operator[](const PstSection & section){ return data[section.name]; }
    bool matches(const PstSection & section, Sortcode sortcode, const std::string & value) {
        return data[section.name].count(sortcode) != 0  &&
        data[section.name][sortcode]->value == value;
    }
};


#endif /* PstFile_hpp */
