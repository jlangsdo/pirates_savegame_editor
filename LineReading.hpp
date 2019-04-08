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
#include <boost/ptr_container/ptr_deque.hpp>
enum rmeth : char;
PstLine read_line(std::ifstream &in, std::string line_code, rmeth method, int bytes, boost::ptr_deque<PstLine> & features);
int read_int(std::ifstream & in);
std::string str_tolower(std::string);

#endif /* LineReading_hpp */
