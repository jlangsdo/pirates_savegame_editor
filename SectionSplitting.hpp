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
enum translation_type : char;

struct subsection_info;
struct section;

void unpack(std::ifstream & in, std::ofstream & out);
void unpack_section (std::ifstream & in, std::ofstream & out, section section, int offset=0, bool stopnow=false);
bool is_world_map(translation_type m);
int index_from_linecode (std::string line_code);

#endif /* SectionSplitting_hpp */
