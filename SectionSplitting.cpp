//
//  SectionSplitting.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/5/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//
// This file maintains the algorithms and data for splitting up a section into
// individual lines that can be read and translated, to unpack the savegame file.

#include <map>
#include <vector>
#include <string>
#include <regex>
#include <iostream>
#include "SectionSplitting.hpp"
#include "LineDecoding.hpp"
#include "LineReading.hpp"
#include "Pirates.hpp"
#include <boost/ptr_container/ptr_deque.hpp>
using namespace std;


bool is_world_map(rmeth m) {
    // world maps get special handling.
    return (m==SMAP || m==CMAP || m==FMAP);
}

// Main description of contents and size of each section, in order.
//    sections with single letter names are generally not understood.
//
// Sections will then be broken up further and further, recurvively. There are two key rules
//   1. Dewey Decimal Rule: (DDR) You can break up a section into subsections, as long as their bytes add up to that of the parent.
//                          This allows more translation of bytes within a subsection without renumbering any other sections.
//   2. Backward Compatible: The pst gives enough information to restore the pirates_savegame file,
//                           even if the decoding arrays below have changed.
//

const vector<PstSection> section_vector = {
    {"Intro",             6,       4,  INT },
    {"CityName",        128,       8,  TEXT8 },
    {"Personal",         57,       4,  INT },
    {"Ship",            128,    1116, },
    {"f",               128,    1116, },
    {"City",            128,      32, },
    {"CityInfo",        128,     148, },
    {"Log",            1000,      28, },
    {"j",                 1,       4, HEX },
    {"e",                30,      32, }, // e is really at least 3 parts: unknown, peace_and_war, and date and age.
    {"Quest",            64,      32, },
    {"LogCount",          1,       4, INT},
    {"TopoMap",         462,     586, },
    {"FeatureMap",      462,     293, FMAP},
    {"TreasureMap",       4,     328, },
    {"SailingMap",      462,     293, SMAP},
    {"vv",              256,      12, },
    {"vvv",               2,       4, INT},
    {"Top10",            10,      28, },
    {"d",                 1,      36, ZERO},
    {"Villain",          28,      36, },
    {"t",                 8,      16, }, // Note that the first piece is actually shorter
    {"CityLoc",         128,      16, },
    {"CoastMap",        462,     293, CMAP},
    {"k",                 8,       4, INT},
    {"LandingParty",      8,      32, },
    {"m",                 1,      12, },
    {"ShipName",          8,       8,  TEXT8 },
    {"Skill",             1,       4, INT },
    
};

// This map lets you split up a section into multiple lines of identically sized smaller types,
// assuming that they will use the default byte counts for that type.
// The size of the new translation_type should divide evenly into the original line size (this is checked at runtime)
map<string,rmeth> subsection_simple_decode = {
    {"Intro_0",       TEXT0},
    {"Intro_3",       HEX},
    {"Personal_2",    BINARY},
    {"Personal_5",    SHORT},
    {"Personal_6",    SHORT},
    {"Personal_9",    SHORT},
    {"Personal_10",   SHORT},
    {"Personal_18",   BINARY},
    {"Personal_45",   SHORT},
    {"Personal_46",   SHORT},
    {"Personal_47",   CHAR},
    {"Personal_48",   CHAR},
    {"Personal_49",   CHAR},
    {"Personal_50",   CHAR},
    {"Ship_x_2",      SHORT},
    {"Ship_x_3",      SHORT},
    {"Ship_x_5",      SHORT},
    {"Ship_x_5_4",    BINARY},
    {"Ship_x_6",      SHORT},
    {"City_x",        INT},
    {"City_x_2",      BINARY},
    {"City_x_4",      BULK},
    {"City_x_7",      BULK},
    {"CityInfo_x_0_2", SHORT},
    {"CityInfo_x_0_3", SHORT},
    {"CityInfo_x_3",   SHORT},
    // {"Log_x_1",   BINARY }   // Pleased/Greatly Pleased
    // {"Log_x_2",   BINARY }   // Offended
    {"e_x",           INT},
    {"Quest_x",       INT},
    {"TreasureMap_x", INT},
    {"vv_x",          INT},
    {"Villain_x_4",   BINARY},
    {"t_7",           INT},
    {"LandingParty_0",uFLOAT},
    {"LandingParty_1",uFLOAT},
    {"LandingParty_x",HEX},
};

// This map lets you split up a section into multiple sections that may be of different or variable types and sizes.
// Make sure the bytes add up to the size of the original section (this is checked at runtime....
// however, if you split a section into only ONE item, then it will permit you to change the size
// (temporarily breaking the Dewey Decimal Rule). Make sure to account for it elsewhere.)
//
// The zero length zero string for Ship_x_4_5 happened because two adjacent shorts were switched
// for an INT and a ZERO. This manuever is required by DDR, to avoid renumbering later pieces
// (Ship_x_4_7 numbering remained unchanged)
//
map<string,vector<PstSplit>> subsection_manual_decode = {
    {"Personal_51",   {{CHAR}, {ZERO,3}}},           // 4 = 1+3
    {"Personal_52",   {{BINARY}, {BULK,3}}},         // 4 = 1+3
    {"Ship_x",        {{BULK,16, 10}, {ZERO,956}}}, // 1116 = 16*10+956
    {"Ship_x_0",      {{SHORT,2, 6}, {uFLOAT}}},        // 16 = 6*2 + 4
    {"Ship_x_1",      {{uFLOAT}, {HEX,4, 3}}},         // 16 = 4 + 4*3
    {"Ship_x_2_6",    {{BINARY}, {BULK,1}}},           // 2 = 1+1
    {"Ship_x_4",      {{SHORT,2,4}, {INT,4,1}, {ZERO,0,1}, {SHORT,2,2}}},   // 16 = 2*4+4+0+2*2
    {"f_x",           {{BULK,2}, {ZERO,98}, {BULK,2}, {ZERO, 1014}}},   // 2+98+2+1014 = 1116
    {"City_x_3",      {{CHAR,1,3}, {BULK,1}}},       // 4 = 3+1. Calling it BULK to match perl. Nonstandard BULK size.
    {"CityInfo_x",    {{BULK, 36}, {BULK, 48}, {BULK, 28}, {BULK, 32}, {BULK,4}}}, // 148 = 36+48+28+32+4
    {"CityInfo_x_0",  {{BULK}, {INT,4,4}, {BULK,4,3}, {INT}}},    //  36 = 4*(1+4+3+1)
    {"CityInfo_x_1",  {{INT}, {BULK}, {INT,4,5}, {SHORT,2,10}}},               //  48 = 4*(1+1+5)+2*10;
    {"Log_x",         {{LCHAR,1,8}, {INT,4,3}, {uFLOAT,4,2}}}, // 28 = 8+4*3+4*2
    {"TreasureMap_x_68", {{BULK,1}, {BINARY,1}, {BULK,1}, {BINARY,1}}}, // 4 = 1+1+1=1
    {"Villain_x",     {{SHORT,2,10}, {INT,4}, {SHORT,2,6}}}, // 36 = 2*10 + 4 + 2*6
    {"CityLoc_x",     {{mFLOAT,4,2}, {HEX,4,2}}}, // 16 = 4*2 + 4*2
    {"t_0",           {{BULK,8,1}}},   // 16 != 8. This is accounted for by having the starting size of t be 8 too large.
    {"Top10_x",       {{INT,4,2}, {SHORT,2,10}}}, // 28 = 4*2 + 2*10
    {"Top10_x_1",     {{BINARY}, {ZERO,3}}},      // 4 = 1+3
};

void unpack(ifstream & in, ofstream & out) {
    for (auto section : section_vector) {
        out << "## " << section.name << " starts at byte " << in.tellg() << "\n";
        check_for_specials(in, out, section.name);
        unpack_section(in, out, section);
    }
}

void unpack_section (ifstream & in, ofstream & out, PstSection mysection, int offset, bool stopnow) {
    
    boost::ptr_deque<PstLine> features;
    
    // section: Ship_0_0
    for (int c=offset; c<mysection.split.count+offset;c++) {
        string subsection = mysection.name + "_" + to_string(c);
        // subsection (could be a line_code)           Ship_0_0_2
        string subsection_x = regex_replace(subsection, regex(R"(^([^_]+)_\d+)"), "$1_x");
        // subsection_x (could be a generic line_code) Ship_x_0_2
        
        // The subsection or subsection_x have higher priority in translating this line
        // than the parent section directive.
        
        if(! stopnow) {
            bool stopnext = false;
            if (subsection_simple_decode.count(subsection) || subsection_simple_decode.count(subsection_x)) {
                rmeth submeth = subsection_simple_decode.count(subsection) ?
                subsection_simple_decode.at(subsection) : subsection_simple_decode.at(subsection_x);
                
                //if (submeth != mysection.method) { // Avoids infinite loop.
                // Take the the section.byte_count and divide
                // it equally to set up the subsections.
                int count = 1;
                if (standard_rmeth_size.at(submeth)>0) {
                    count = mysection.split.bytes/standard_rmeth_size.at(submeth);
                    
                    if (mysection.split.bytes % standard_rmeth_size.at(submeth) != 0) {
                        cerr << "Error decoding line " << subsection << " byte counts are not divisible\n";
                        out.close();
                        abort();
                    }
                }
                int suboffset = 0;
                if (count ==1) {
                    // If there is only one count for the subsection, then we aren't really splitting it up;
                    // we are changing the translation method of this particular line.
                    // So, remove the numeric ending and call it again with the corrected method.
                    // But note the if above to avoid an infinite loop.
                    subsection = mysection.name;
                    suboffset = c;
                    stopnext = true;
                }
                
                struct::PstSection sub = {subsection,count, standard_rmeth_size.at(submeth) , submeth };
                unpack_section(in, out, sub, suboffset, stopnext);
                continue;
                // }
            }
            
            if (subsection_manual_decode.count(subsection) || subsection_manual_decode.count(subsection_x)) {
                
                vector<PstSplit> info = subsection_manual_decode.count(subsection) ?
                subsection_manual_decode.at(subsection) : subsection_manual_decode.at(subsection_x);
                
                int suboffset = 0;
                int byte_count_check = 0;
                int count = 0;
                bool stopnext = false;
                for (auto subinfo : info) {
                    count += subinfo.count;
                    byte_count_check += subinfo.count*subinfo.bytes;
                }
                if (count == 1) {
                    subsection = mysection.name;
                    suboffset = c;
                    stopnext = true;
                } else if (byte_count_check != mysection.split.bytes) {
                    cerr << "Error decoding line " << subsection << " subsections don't add up: " << byte_count_check << " != " << mysection.split.bytes << "\n";
                    out.close();
                    abort();
                    // It will probably crash later anyway.
                }
                for (auto subinfo : info) {
                    rmeth submeth = subinfo.method;
                    struct::PstSection sub = {subsection,subinfo.count, subinfo.bytes , submeth };
                    
                    // cerr << subsection << "," << subinfo.multiplier  << "," << subinfo.byte_count << "\n";
                    unpack_section(in, out, sub, suboffset, stopnext);
                    suboffset += subinfo.count;
                    
                }
                
                continue; // Fall out of for loop on c, do not print a line.
                
            }
        }
        // When we get here, it means that there was just one line of data to read,
        // process, and print.
        auto aline = read_line(in, subsection, mysection.split.method, mysection.split.bytes, features);
        if (is_world_map(mysection.split.method)) { aline.line_code += "_293"; }
        
        aline.print(out);
        
    }
    
    // world_map sections accumulate features, which we print after the map.
    if (is_world_map(mysection.split.method)) {
        for (auto aline : features) {
            aline.print(out);
        }
        features.release();   // I think this deletes all of the PstLine objects created in read_world_map.
    }
}

