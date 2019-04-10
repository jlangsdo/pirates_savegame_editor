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

unsigned long long index_to_sortcode(std::string numbers) {
    // Convert a number code like _23_5_2 into a giant long integer like: 1'023'005'002'000'000'000
    // so in the PstFile map they will sort them into place.
    // Note that it would be illegal to have lines_codes like Ship_23 and Ship_23_0 as they would sort together.
    stringstream digits;
    digits << 1;
    auto const digit_regex = regex("_(\\d+)");
    smatch index;
    
    for (auto i=0; i<6; i++) {
        if (std::regex_search(numbers,index,digit_regex)) {          // Strip off digits in the front
            digits << setw(3) << setfill ('0') << index[1].str();
            numbers = index.suffix().str();
        } else {                                                     // Or put in zeros.
            digits << "000";
        }
    }
    return stoull(digits.str());
}
int sortcode_get_index(unsigned long long sortcode, const int index) {
    for (auto b=index; b<6; b++) {
        sortcode /= 1000;
    }
    return (int) (sortcode % 1000);
}



void PstFile::read_text(std::ifstream & in) {
    string line;
    
    //                               Section1 Number2     rmeth3  bytes4          value5    comments/translation
    auto const line_regex = regex(R"(([^_ ]+)(_\S+)\s+:\s+([^\W\d]+)(\d+)\s+:\s+(.*?)\s+:.*)");
    while(getline(in, line)) {
        
        std::smatch matches;    // same as std::match_results<string::const_iterator> sm;
        if (std::regex_match(line, matches, line_regex, std::regex_constants::match_default)) {
            
            // Convert the line_code numbers into a big integer for quick sorting.
            unsigned long long sortcode = index_to_sortcode(matches[2]);
            
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
                // First, inflate all of the non-FEATURE strings to full size.
                for (auto&& pair : data[section.name]) {
                    if (pair.second->method != FEATURE) {
                        pair.second->expand_map_value();
                    }
                }
                // Then, insert the features.
                for (auto&& pair : data[section.name]) {
                    if (pair.second->method == FEATURE) {  // FeatureMap_35_202  : F1 : 10 : (Landmark)
                                                           // pair.first = 1'032'202'000'000'000'000
                        int row = sortcode_get_index(pair.first,1);
                        int col = sortcode_get_index(pair.first,2);
                        if (row <= 35) {
                            // Just for debug
                        }
                        // We need to edit FeatureMap_35_293 column 202, so construct the appropriate line_code
                        unsigned long long target = index_to_sortcode("_" + to_string(row) + "_293");
                        if (data[section.name].count(target) != 1) throw logic_error ("Tried to add features to missing row");
                        // Take the value from the FEATURE and use it to update the value in the map line.
                        
                        data[section.name].at(target)->update_map_value(col, pair.second->value);
                    }
                }
            }
            for (auto&& pair : data[section.name]) {
                pair.second->write_binary(out);
            }
        }
    }
}
