//
//  LineReading.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/5/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "LineReading.hpp"
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

void read_hex(ifstream & in, info_for_line_decode & i) { // Read 4 bytes from in and report in hex
    char b[4];
    in.read((char*)&b, sizeof(b));
    char buffer[255];
    for (int j=3;j>=0;j--) {
        sprintf(buffer, "%02X", (unsigned char)b[j]);
        i.value += buffer;
        if (j>0) i.value += '.';
    }
    i.v = (int)b[3];
}

string read_bulk_hex(ifstream & in, int bytecount) { // Read bytecount bytes from in and report in one big bulk hex
    char b[bytecount];
    in.read((char*)&b, bytecount);
    string res;
    char buffer[20];
    for (int i=0;i<bytecount;i++) {
        sprintf(buffer, "%02X", (unsigned char)b[i]);
        res += buffer;
    }
    return str_tolower(res);
}

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
                   );
    return s;
}

string read_zeros(ifstream & in, int bytecount) { // Read bytecount bytes from in that should all be zero.
    char b[bytecount];
    in.read((char*)&b, bytecount);
    for (int i=0;i<bytecount;i++) {
        if (b[i]!= 0) {
            throw logic_error("Found non-zeros");
        }
    }
    return "zero_string";
}


void read_binary(ifstream & in, info_for_line_decode & i) { // Read 1 byte from in and report as binary string
    char b;
    in.read((char*)&b, sizeof(b));
    std::bitset<8> asbits(b);
    i.value = asbits.to_string();
    i.v = (int)b;
}

void read_char(ifstream & in, info_for_line_decode & i) { // Read 1 byte from in and report as an int string
    char b;
    in.read((char*)&b, sizeof(b));
    i.value = to_string(b);
    i.v = (int)b;
}

void read_short(ifstream &in, info_for_line_decode & i) { // Read 2 bytes as a signed int and return as string
    char b[2];
    in.read((char*)&b, sizeof(b));
    i.v = int((unsigned char)(b[0]) | (char)(b[1]) << 8);
    i.value = to_string(i.v);
}

int read_int(ifstream & in) { // Read 4 bytes from in (little endian) and convert to integer
    char b[4];
    in.read((char*)&b, sizeof(b));
    int B = (int)((unsigned char)(b[0]) |
                  (unsigned char)(b[1]) << 8 |
                  (unsigned char)(b[2]) << 16 |
                  (char)(b[3]) << 24    );
    return B;
}


string read_uFloat(ifstream &in) {
    auto raw = read_int(in);
    if (raw == 0) { return "  0.000000"; }
    stringstream ss;
    ss << std::right << std::fixed << setprecision(6) << setw(10) << double(raw)/1000000;
    return ss.str();
}


string read_mFloat(ifstream &in) {
    auto raw = read_int(in);
    string retstring = to_string(double(raw)/1000);
    // backward compatibility to match perl:
    retstring = regex_replace(retstring, regex("000$"), "");
    if (retstring == "0.000") { retstring = "0"; }
    return retstring;
}

string read_world_map(ifstream &in, int bytecount, translation_type m, string line_code, vector<info_for_line_decode> & features) {
    unsigned char b[600];
    in.read((char*)&b, bytecount);
    
    vector<bitset<4> > bs(bytecount/4+1, 0);
    
    // The bytes read have values 00, 09, or FF at each byte, except for anomolies.
    // 00 represents sea, FF is land. 09 is for the boundary.
    // To make the output more readable and editable, we compress the land/sea down to single bits
    // and make note of anomolies in the features vector.
    for (int i=0; i<bytecount; i++) {
        auto j = i/4;
        auto k = i % 4;
        
        char buf[3];
        
        if ((m==CMAP && b[i]>4) ||
            (m!=CMAP && b[i]!= 0)) {
            // LAND
            bs.at(j)[3-k] = 1;
        }
        
        if (b[i] != 0) {
            if ((m==CMAP && b[i] != 9) ||
                (m!=CMAP &&  b[i] != (unsigned char)(-1)) ) {
                // Anomoly. Add to the features vector for printing after the main map.
                sprintf(buf, "%02x", b[i]);
                features.push_back({buf, b[i], line_code + "_" + to_string(i)});
            }
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

info_for_line_decode read_line(std::ifstream &in, std::ofstream &out, string line_code, translation_type method, int bytes_per_line, vector<info_for_line_decode> &features) {
    info_for_line_decode i = {"", 0, line_code};  // Defaults.
    char buffer[255] = "";
    int size_of_string;
    
    switch (method) {
        case TEXT0 : // Reads the string length, then the string
            size_of_string = read_int(in);
            if (size_of_string > 100) { out.close(); abort(); }
            in.read((char *)& buffer, size_of_string);
            i.value = buffer;
            return i;
        case TEXT8 : // Reads the string length, then the string, then 8 bytes of padding.
            size_of_string = read_int(in);
            if (size_of_string > 100) { out.close(); abort(); }
            in.read((char *)& buffer, size_of_string);
            i.value = buffer;
            in.read(buffer, 8);       //  Padding
            return i;
        case INT :
            i.v = read_int(in);
            i.value = to_string(i.v);
            return i;
        case ZERO :
            try {
                i.value = read_zeros(in, bytes_per_line);
                return i;
            } catch(logic_error) {
                out.close();
                abort();
            }
        case BULK :
            i.value = read_bulk_hex(in, bytes_per_line);
            return i;
        case HEX :
            read_hex(in, i);   // Edits i
            return i;
        case FMAP :
        case SMAP :
        case CMAP :
            i.value = read_world_map(in,bytes_per_line,method, line_code, features);
            return i;
        case BINARY :
            read_binary(in, i);
            return i;
        case SHORT :
            // set i.v
            read_short(in, i);
            return i;;
        case mFLOAT :
            i.value = read_mFloat(in);
            return i;
        case uFLOAT :
            i.value = read_uFloat(in);
            return i;
        case CHAR :
            read_char(in, i);
            return i;
        case LCHAR : // Not sure yet how this differs from char.
            read_char(in, i);
            return i;
        default: ;
    }
    throw logic_error("Problem with line reading");
}
