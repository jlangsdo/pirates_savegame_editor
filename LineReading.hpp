//
//  LineReading.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/5/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef LineReading_hpp
#define LineReading_hpp

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "Pirates.hpp"
enum translation_type : char;

info_for_line_decode read_line(std::ifstream &in, std::ofstream &out, std::string line_code, translation_type method, int bytes_per_line, std::vector<info_for_line_decode> &features);
int read_int(std::ifstream & in);
std::string str_tolower(std::string);

#endif /* LineReading_hpp */
