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

Sortcode index_to_sortcode(std::string numbers) {
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

int sortcode_get_index(Sortcode sortcode, const int index) { // The sortcode is a numeric version of the linecode,
    for (auto b=index; b<6; b++) {                           // So we can recover any piece of the linecode.
        sortcode /= 1000;
    }
    return (int) (sortcode % 1000);
}

std::vector<size_t> special_fast_regex(const std::string & line, const std::string & front_set, const std::string & back_set ) {
    // This routine optimizes searching in a string to find the locations of substrings that can be pulled out
    // using substr. It replaces a regex that is of the special form ^([^a]+)([^b+])...(.*?)..([^y+])([^z+])$
    // where you can find all of the boundaries by making a pass from the front followed by a pass from the back.
    // The input values are the tokens the indicate passing from one capture group to the next.
    // Note that d, D act like \d, \D, and S acts like not-space.
    size_t rsize = front_set.length() + back_set.length()+2;
    auto r = std::vector<size_t>(rsize);
    r[0] = 0;
    size_t fsize = front_set.length();
    for (size_t i=0; i < fsize; ++i) {
        switch (front_set[i]) {
            case 'D':
                r[i+1] = line.find_first_not_of("1234567890", r[i]);
                break;
            case 'S':
                r[i+1] = line.find_first_not_of(" ", r[i]);
                break;
            case 'd':
                r[i+1] = line.find_first_of("1234567890", r[i]);
                break;
            default:
                r[i+1] = line.find(front_set[i],r[i]);
        }
    }
    r[rsize-1] = string::npos;
    size_t bsize = back_set.length();
    for(size_t i=bsize; i!=0; --i) {
        switch(back_set[i-1]) {
            case 'D':
                r[fsize+i] = line.find_last_not_of("1234567890",r[fsize+i+1]);
                break;
            case 'S':
                r[fsize+i] = line.find_last_not_of(" ",r[fsize+i+1]);
                break;
            case 'd':
                r[fsize+i] = line.find_last_of("1234567890",r[fsize+i+1]);
                break;
            default:
                r[fsize+i] = line.rfind(back_set[i-1],r[fsize+i+1]);
        }
    }
    // When we find from the back, the boundary is shifted off-by-one.
    for(size_t i=bsize; i!=0; --i) { ++r[fsize+i]; }
    return r;
}

inline
std::string special_fast_regex_result(const std::string & str, const std::vector<size_t> & r, size_t index) {
    return str.substr(r[index],r[index+1]-r[index]);
}

void PstFile::read_text(std::string afile, std::string suffix) {
    if (afile.length() > 0) {
        filename = find_file(afile, suffix);
        std::ifstream pst_in(filename);
        if (! pst_in.is_open()) {
            std::cerr << "Failed to read from " << filename << "\n";
            exit(1);
        }
        std::string short_file = regex_replace(filename, std::regex(".*\\/"), "");
        std::cout << "Reading " << short_file << "\n";
        read_text(pst_in);
        pst_in.close();
    }
}
void PstFile::read_text(std::ifstream & in) {
    string line;
    
    while(getline(in, line)) {
        if (line[0] == '#') { continue; } // Ignore comments.
        
        auto r = special_fast_regex(line, "_ : Sd : S", "S :");
        string section   = special_fast_regex_result(line, r, 0);
        string line_code = special_fast_regex_result(line, r, 1);
        rmeth method =  meth_for_char(special_fast_regex_result(line, r, 5));
        int bytes   = stoi(special_fast_regex_result(line, r, 6));
        string value     = special_fast_regex_result(line, r, 10);
        
        // Convert the line_code numbers into a big integer for quick sorting.
        // Line order in the pst file is assumed to be scrambled.
        Sortcode sortcode = index_to_sortcode(line_code);
        
        data[section].emplace(sortcode, std::make_unique<PstLine>(line_code, method, bytes, value) );
    }
    if (in.bad())
        throw runtime_error("Error while reading pst file");
}

void PstFile::write_pg(std::ofstream & out) {
    for (auto section : section_vector) {
        if (data[section.name].size() == 0 ) { continue; }  // Support for changing the section_vector
        
        if (is_world_map(section.splits.front().method)) {
            // First, expand all of the non-FEATURE strings to full size.
            for (auto&& pair : data[section.name]) {
                if (pair.second->method != FEATURE) {
                    pair.second->expand_map_value();
                }
            }
            // Then, insert the features into the expanded maps.
            for (auto&& pair : data[section.name]) {
                if (pair.second->method == FEATURE) {  // FeatureMap_35_202  : F1 : 10 : (Landmark)
                    // pair.first = 1'032'202'000'000'000'000
                    int row = sortcode_get_index(pair.first,1);
                    int col = sortcode_get_index(pair.first,2);
                    
                    // 293 is a magic number: the width of a map.
                    // For a Feature at FeatureMap_35_202,
                    // we need to edit FeatureMap_35_293 column 202, so construct the appropriate line_code, and edit that PstLine.
                    Sortcode target = index_to_sortcode("_" + to_string(row) + "_293");
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
    out.close();
}
