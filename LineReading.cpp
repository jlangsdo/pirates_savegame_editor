//
//  LineReading.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/5/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "LineReading.hpp"
#include <boost/ptr_container/ptr_deque.hpp>
#include "Pirates.hpp"
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <regex>
using namespace std;

// This file includes the functions for reading one line of text according to a translation_method,
// and returning the result as a string value (and optionally an integer value for a translated comment).

// Utilities?
int read_int(ifstream & in) { // Read 4 bytes from in (little endian) and convert to integer
    char b[4];
    in.read((char*)&b, sizeof(b));
    int B = (int)((unsigned char)(b[0]) |
                  (unsigned char)(b[1]) << 8 |
                  (unsigned char)(b[2]) << 16 |
                  (char)(b[3]) << 24    );
    return B;
}


string read_world_map(ifstream &in, int bytecount, rmeth m, string line_code, boost::ptr_deque<PstLine> & features) {
    unsigned char b[600];
    in.read((char*)&b, bytecount);
    
    vector<bitset<4> > bs(bytecount/4+1, 0);
    
    const unsigned char sea = 0;
    const unsigned char land    = m==CMAP ? 9 : (unsigned char)(-1);
    const unsigned char max_sea = m==CMAP ? 4 : 0;
    
    for (int i=0; i<bytecount; i++) {
        auto j = i/4;
        auto k = i % 4;
        
        if (b[i]>max_sea) { bs.at(j)[3-k] = 1; }
        
        if (b[i] != sea && b[i] != land) {
            // Anomoly. Add to the features vector for printing after the main map.
            stringstream ss;
            ss << std::noshowbase << std::hex << nouppercase << setw(2) << setfill('0') << (int)(unsigned char)b[i];
            features.push_back( new PstLine{line_code + "_" + to_string(i), b[i],  ss.str()});
        }
    }
    // Now compressing the single bits of the map into hex for printing.
    stringstream ss;
    for (int j=0; j<bytecount/4+1; j++) {
        ss << std::noshowbase << std::hex << bs[j].to_ulong();
    }
    if (m==SMAP) { return ""; }
    return ss.str();
    
}

PstLine read_line(std::ifstream &in, std::ofstream &out, string line_code, rmeth method, int bytes_per_line, boost::ptr_deque<PstLine> & features) {
    PstLine info {line_code};  // Defaults.
    char b[100] = "";
    stringstream ss;
    int size_of_string;
    switch (method) {
        case TEXT0 : // Reads the string length, then the string
        case TEXT8:
            size_of_string = read_int(in);
            if (size_of_string > sizeof(b)-2) throw logic_error("expected tring too long");
            in.read((char *)& b, size_of_string);
            info.value = b;
            if (method == TEXT8) {
                if (read_int(in) != 0) {} //throw logic_error("Unexpected non-zero after text8");
                if (read_int(in) != 0) {} //throw logic_error("Unexpected non-zero after text8");
            }
            return info;
        case BULK :
            for (int i=0;i<bytes_per_line;i++) {
                in.read((char*)&b, 1);
                ss << std::noshowbase << std::hex << nouppercase << setw(2) << setfill('0') << (int)(unsigned char)b[0];
            }
            info.value = ss.str();
            return info;
        case ZERO :
            for (int i=0;i<bytes_per_line;i++) {
                in.read((char*)&b, 1);
                if (b[0] != 0) throw logic_error("Non-zero found in expected zero-string");
            }
            info.value = "zero_string";
            return info;
        case INT :
        case HEX :
        case uFLOAT:
        case mFLOAT:
        case SHORT:
        case CHAR:
        case LCHAR:
        case BINARY:
            if (bytes_per_line != size_for_method.at(method))
                    throw logic_error("Incorrect size request for fixed size number");
            in.read((char*)&b, bytes_per_line);
            for (int i=bytes_per_line-1; i>=0; i--) {
                if (i<bytes_per_line-1) {
                    info.v = (info.v<<8) + (unsigned char)b[i];
                } else {
                    info.v = (int)(char)b[i];
                }
                if (method == HEX) {
                    ss << std::noshowbase << std::hex << uppercase << setw(2) << setfill('0') << (int)(unsigned char)b[i];
                    if (i != 0) { ss << ".";}
                }
            }
            switch (method) {
                case uFLOAT:
                    ss << std::right << std::fixed << setprecision(6) << setw(10) << double(info.v)/1'000'000;
                    break;
                case mFLOAT:
                    if (info.v == 0) {
                        ss << "0";
                    } else {
                        ss << std::left << std::fixed << setprecision(3) << setw(6) << double(info.v)/1000;
                    }
                    break;
                case BINARY:
                    ss << std::bitset<8>(info.v);
                    break;
                case HEX: ;
                    // ss was already loaded for hex
                    // However, we might want the first byte as a number 0..16 for lookup
                    info.v = ((unsigned char)b[3]+8)/16;
                    break;
                default:
                    ss << to_string(info.v);
            }
            info.value = ss.str();
            return info;
        case FMAP :
        case SMAP :
        case CMAP :
            info.value = read_world_map(in,bytes_per_line,method, line_code, features);
            return info;
        default: ;
    }
}
