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


void compare_binary_filestreams(std::ifstream & in1, std::ifstream & in2) {
    constexpr int c = 16;  // This is like diff, but avoiding the unix system call to be portable with Windows.
    char b1[c];
    char b2[c];
    
    while(in1) {
        in1.read((char*)&b1, c);
        in2.read((char*)&b2, c);
        string s1(b1,c);
        string s2(b2,c);
        if (s1 != s2) {
            cout << "Failed at byte " << in1.tellg() << "\n";
            abort();
        }
    }
}

unsigned long long index_to_sortcode(std::string numbers) {
    // Faster, non-regex version of routine below.
    stringstream digits;
    digits << "1";
    size_t index = numbers.find_first_of('_');
    size_t next_index;
    for (auto i=0; i<6; i++) {
        if (index != string::npos) {
            next_index = numbers.find_first_of('_', index+1);
            if (next_index == string::npos) {
                digits << setw(3) << setfill ('0') << numbers.substr(index+1, next_index);
            } else {
                digits << setw(3) << setfill ('0') << numbers.substr(index+1, next_index-index-1);
            }
            index = next_index;
        } else {
            digits << "000";
        }
    }
    return stoull(digits.str());
}


unsigned long long slow_index_to_sortcode(std::string numbers) {
    // Convert a number code like _23_5_2 into a giant long integer like: 1'023'005'002'000'000'000
    // so in the PstFile map they will sort them into place.
    // Note that it would be illegal to have lines_codes like Ship_23 and Ship_23_0 as they would sort together.
    stringstream digits;
    digits << 1;
    string original_numbers = numbers;
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

//                               Section1 Number2     rmeth3  bytes4          value5    comments/translation
auto const line_regex = regex(R"(([^_ ]+)(_\S+)\s+:\s+([^\W\d]+)(\d+)\s+:\s+(.*?)\s+:.*)");
void PstFile::read_text(std::ifstream & in) {
    string line;
    
    while(getline(in, line)) {
        if (line[0] == '#') { continue; } // Strip out comments.
        
        // Attempting to avoid the regex above.
        size_t line_l = line.length();
        size_t index = line.find_first_of('_');
        size_t last_index = index; // index at _
        string section   = line.substr(0, index);
        index = line.find_first_of(' ', index);
        string line_code = line.substr(last_index, index-last_index);
        index = line.find_first_of(':', index);       // Index at :
        last_index = line.find_first_not_of(' ', index+1); // last_index now at start of meth_code
        for(index = last_index; (line[index] < '0' || line[index] > '9') && index<line_l; ++index) {} // index at start of bytes
        string meth_code = line.substr(last_index, index-last_index);
        last_index = line.find_first_of(' ', last_index);  // last_index now at space
        int bytes = stoi( line.substr(index, last_index-index));
        index = line.find_first_of(':', last_index); // Index now at colon before value
        index = line.find_first_not_of(' ', index+1); // Index at start of value;
        last_index = line.find_first_of(':', index); // last_index at the colon after value
        last_index = line.find_last_not_of(' ', last_index-1); // last_index now at end of value;
        string value = line.substr(index, last_index-index+1);
        
        //   std::smatch matches;    // same as std::match_results<string::const_iterator> sm;
        //   if (std::regex_match(line, matches, line_regex, std::regex_constants::match_default)) {
        
        // Convert the line_code numbers into a big integer for quick sorting.
        unsigned long long sortcode = index_to_sortcode(line_code);
        
        rmeth method = char_for_method.right.at(meth_code);
        data[section].emplace(sortcode, std::make_unique<PstLine>(line_code, method, bytes, value) );
    }
    if (in.bad())
        throw runtime_error("Error while reading pst file");
}
void PstFile::write_pg(std::ofstream & out) {
    for (auto section : section_vector) {
        if (data[section.name].size() > 0 ) {
            if (is_world_map(section.splits.front().method)) {
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
                        
                        // 293 is a magic number: the width of a map.
                        // For a Feature at FeatureMap_35_202,
                        // we need to edit FeatureMap_35_293 column 202, so construct the appropriate line_code, and edit that PstLine.
                        unsigned long long target = index_to_sortcode("_" + to_string(row) + "_293");
                        if (data[section.name].count(target) != 1) throw logic_error ("Tried to add features to missing row");
                        data[section.name].at(target)->update_map_value(col, pair.second->value);
                    }
                }
            }
            // Now we are ready to write out the binary for the section.
            for (auto&& pair : data[section.name]) {
                pair.second->write_binary(out);
            }
        }
    }
    out.close();
}
