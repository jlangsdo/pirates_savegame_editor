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
int index_from_linecode (std::string line_code);

#endif /* SectionSplitting_hpp */
