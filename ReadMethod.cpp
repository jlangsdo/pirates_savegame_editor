//
//  ReadMethod.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/6/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "ReadMethod.hpp"
#include "LineReading.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Pirates.hpp"
using namespace std;

void rmTEXT0::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                   std::vector<info_for_line_decode> &features) {
    char b[200];
    auto size_of_string = read_int(in);
    if (size_of_string > sizeof(b)) { throw logic_error("Requested string size too big"); }
    in.read((char *)& b, size_of_string);
    info.value = b;
    
    int pad = padding();
    for (int i=0;i<pad; i++) {
        in.read((char *)& b, 1);
        if (b[0] != 0) { throw logic_error("Non-zero found in string padding"); }
    }
}
void rmMAP::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                 std::vector<info_for_line_decode> &features) {
    char b[500];
    if (bytes_to_read > sizeof(b)) throw logic_error("map line too long");
    in.read((char*)&b, bytes_to_read);
    
    int clen = bytes_to_read/4;
    if (bytes_to_read % 4) { clen++; }
    vector<bitset<4> > bs(clen, 0);
    
    // The bytes read have values 00, 09, or FF at each byte, except for anomolies.
    // 00 represents sea, FF is land. 09 is for the boundary.
    // To make the output more readable and editable, we compress the land/sea down to single bits
    // and make note of anomolies in the features vector.
    for (int i=0; i<bytes_to_read; i++) {
        auto j = i/4;
        auto k = i % 4;
        
        char buf[3];
        if (b[i] > max_sea()) { // Call it land for the map.
            bs.at(j)[3-k] = 1;
        }
        if (b[i] != 0 && b[i] != land()) { // Anomoly, store in features list.
            sprintf(buf, "%02x", b[i]);
            features.push_back({buf, b[i], info.line_code + "_" + to_string(i)});
        }
    }
    
    // Now compressing the single bits of the map into hex for printing the map without anomolies.
    if (!throw_away_string()) {
        stringstream ss;
        for (int j=0; j<bytes_to_read/4+1; j++) {
            ss << std::noshowbase << std::hex << bs[j].to_ulong();
        }
        info.value = ss.str();
    }
}

void rmBULK::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                 std::vector<info_for_line_decode> &features) {
    char b[500];
    if (bytes_to_read > sizeof(b)) throw logic_error("map line too long");
    in.read((char*)&b, bytes_to_read);
    
    stringstream ss;
    for (int i=0;i<bytes_to_read;i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << (unsigned char)b[i];
        if (check_against_zero() && b[i] != 0) {
            throw logic_error("Non-zero found in expected zero string");
        }
    }
    info.value = ss.str();
}
void rmNUMBER::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                    std::vector<info_for_line_decode> &features) {
    if (bytes_to_read != this->standard_size()) {
        throw logic_error("Incorrect size request for fixed size number");
    }
    char b[4];
    stringstream ss;
    in.read((char*)&b, standard_size());
    for (int i=standard_size(); i>=0; i--) {
        info.v = (info.v<<8) + (unsigned char)b[i];
        if (write_hex()) {
            ss << std::noshowbase << std::hex << (unsigned char)b[i];
            if (i != 0) { ss << ":";}
        }
    }
    if(write_hex()) {
        info.value = ss.str();
    } else {
        info.value = to_string(info.v);
    }
}
void rmBINARY::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                    std::vector<info_for_line_decode> &features) {
    // A rmBINARY is a rmCHAR that is reports the value in binary
    rmCHAR::read(in, info, bytes_to_read, features);
    std::bitset<8> asbits(info.v);
    info.value = asbits.to_string();
}
void rmuFLOAT::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                    std::vector<info_for_line_decode> &features) {
    rmINT::read(in, info, bytes_to_read, features);
    stringstream ss;
    ss << std::right << std::fixed << setprecision(6) << setw(10) << double(info.v)/1'000'000;
    info.value = ss.str();
}
void rmmFLOAT::read(std::ifstream & in, info_for_line_decode & info, int bytes_to_read,
                    std::vector<info_for_line_decode> &features) {
    rmINT::read(in, info, bytes_to_read, features);
    stringstream ss;
    ss << std::right << std::fixed << setprecision(6) << setw(10) << double(info.v)/1000;
    info.value = ss.str();
}
