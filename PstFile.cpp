//
//  PstFile.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/8/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//
// A PstFile corresponds to the full data from a pst file read in from text,
// with each line read corresponding to one PstLine.

#include "PstFile.hpp"
#include "PstSection.hpp"
#include <string>
#include <regex>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

void pack(std::ifstream & in, std::ofstream & out) {
    PstFile contents{};
    contents.read_text(in);
    contents.write_pg(out);
}

void PstFile::read_text(std::ifstream & in) {
    string line;
    
    //                               Section1 Number2     rmeth3  bytes4          value5    comments/translation
    auto const line_regex = regex(R"(([^_ ]+)(_\S+)\s+:\s+([^\W\d]+)(\d+)\s+:\s+(.*?)\s+:.*)");
    auto const digit_regex = regex("_(\\d+)");
    while(getline(in, line)) {
        
        std::smatch matches;    // same as std::match_results<string::const_iterator> sm;
        if (std::regex_match(line, matches, line_regex, std::regex_constants::match_default)) {
            
            // Convert the line_code numbers into a decimal for quick sorting.
            // Ship_17_4_2 => 0. 017 004 002 = 0.017004002
            stringstream digits;
            digits << "0." ;
            smatch index;
            string numbers = matches[2];
            while (std::regex_search(numbers,index,digit_regex)) { // This is like a split operation.
                digits << setw(3) << setfill ('0') << index[1].str();
                numbers = index.suffix().str();
            }
            double sortcode = stod(digits.str());
            
        //    cout << "Section " << matches[1] << " " << sortcode << " = PstLine ";
        //    cout << matches[3] << " " << matches[4] << " = " << matches[5] << "\n";
            
            rmeth method = char_for_method.right.at(matches[3]);
            string section = matches[1];
            int bytes = stoi(matches[4]);
            string value = matches[5];
            data[section].emplace(sortcode, std::make_unique<PstLine>(method, bytes, value) );
            
        } else {
         //   cout << line << "\n";  // Stripping out comments.
        }
    }
    if (in.bad())
        throw runtime_error("Error while reading pst file");
}
void PstFile::write_pg(std::ofstream & out) {
    for (auto section : section_vector) {
        if (data[section.name].size() > 0 ) {
            cout << "Writing " << section.name << "\n";
            
            if (is_world_map(section.splits.front().method)) {
                cout << "   Careful, that's a world_map section\n";
            }
            for (auto&& pair : data[section.name]) {
                pair.second->write_binary(out);
            }
        }
    }
}
