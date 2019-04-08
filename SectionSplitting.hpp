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
enum rmeth : char;

struct PstSplit;
struct PstSection;

void unpack(std::ifstream & in, std::ofstream & out);
void unpack_section (std::ifstream & in, std::ofstream & out, PstSection section, int offset=0, bool stopnow=false);
int index_from_linecode (std::string line_code);

#endif /* SectionSplitting_hpp */
