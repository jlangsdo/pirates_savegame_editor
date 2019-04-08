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


void PstLine::read_world_map(ifstream &in, boost::ptr_deque<PstLine> & features) {
    unsigned char b[600];
    in.read((char*)&b, bytes);
    
    vector<bitset<4> > bs(bytes/4+1, 0);
    
    const unsigned char sea = 0;
    const unsigned char land    = method==CMAP ? 9 : (unsigned char)(-1);
    const unsigned char max_sea = method==CMAP ? 4 : 0;
    
    for (int i=0; i<bytes; i++) {
        auto j = i/4;
        auto k = i % 4;
        
        if (b[i]>max_sea) { bs.at(j)[3-k] = 1; }
        
        if (b[i] != sea && b[i] != land) {
            // Anomoly. Add to the features vector for printing after the main map.
            stringstream ss;
            ss << std::noshowbase << std::hex << nouppercase << setw(2) << setfill('0') << (int)(unsigned char)b[i];
            features.push_back( new PstLine{line_code + "_" + to_string(i), FEATURE, b[i],  ss.str()});
        }
    }
    // Now compressing the single bits of the map into hex for printing. SMAP would be all zeros, so it saves nothing.
    if (method != SMAP) {
        stringstream ss;
        for (int j=0; j<bytes/4+1; j++) {
            ss << std::noshowbase << std::hex << bs[j].to_ulong();
        }
        value = ss.str();
    }
}

void PstLine::read(std::ifstream &in, boost::ptr_deque<PstLine> & features) {
    if (is_world_map(method)) {
        this->read_world_map(in, features);
        line_code += "_293";
    } else {
        this->read(in);
    }
}

void PstLine::read(std::ifstream &in) {
    char b[100] = "";
    stringstream ss;
    int size_of_string;
    switch (method) {
        case TEXT0 : // Reads the string length, then the string
        case TEXT8:
            size_of_string = read_int(in);
            if (size_of_string > sizeof(b)-2) throw logic_error("expected tring too long");
            in.read((char *)& b, size_of_string);
            value = b;
            if (method == TEXT8) {
                if (read_int(in) != 0) {} //throw logic_error("Unexpected non-zero after text8");
                if (read_int(in) != 0) {} //throw logic_error("Unexpected non-zero after text8");
            }
            break;
        case BULK :
            for (int i=0;i<bytes;i++) {
                in.read((char*)&b, 1);
                ss << std::noshowbase << std::hex << nouppercase << setw(2) << setfill('0') << (int)(unsigned char)b[0];
            }
            value = ss.str();
            break;
        case ZERO :
            for (int i=0;i<bytes;i++) {
                in.read((char*)&b, 1);
                if (b[0] != 0) throw logic_error("Non-zero found in expected zero-string");
            }
            value = "zero_string";
            break;
        case INT :
        case HEX :
        case uFLOAT:
        case mFLOAT:
        case SHORT:
        case CHAR:
        case LCHAR:
        case BINARY:
            if (bytes != standard_rmeth_size.at(method))
                    throw logic_error("Incorrect size request for fixed size number");
            in.read((char*)&b, bytes);
            for (int i=bytes-1; i>=0; i--) {
                if (i<bytes-1) {
                    v = (v<<8) + (unsigned char)b[i];
                } else {
                    v = (int)(char)b[i];
                }
                if (method == HEX) {
                    ss << std::noshowbase << std::hex << uppercase << setw(2) << setfill('0') << (int)(unsigned char)b[i];
                    if (i != 0) { ss << ".";}
                }
            }
            switch (method) {
                case uFLOAT:
                    ss << std::right << std::fixed << setprecision(6) << setw(10) << double(v)/1'000'000;
                    break;
                case mFLOAT:
                    if (v == 0) {
                        ss << "0";
                    } else {
                        ss << std::left << std::fixed << setprecision(3) << setw(6) << double(v)/1000;
                    }
                    break;
                case BINARY:
                    ss << std::bitset<8>(v);
                    break;
                case HEX: ;
                    // ss was already loaded for hex
                    // However, we might want the first byte as a number 0..16 for lookup
                    v = ((unsigned char)b[3]+8)/16;
                    break;
                default:
                    ss << to_string(v);
            }
            value = ss.str();
            break;
        default: ;
    }
}
