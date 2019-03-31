//
//  Pirates.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 3/30/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "Pirates.hpp"
#include <iostream>
#include <string>
#include <regex>
#include <sys/stat.h>
#include <fstream>
#include <map>
#include <optional>
#include <vector>
using namespace std;



//
enum translation_type { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, mFLOAT, uFLOAT, MAP, BULK };

// The translation types have one character abbreviations in the pst file.
const map<translation_type,char> char_for_method = {
    {TEXT0,'t'}, {TEXT8,'t'}, {INT, 'V'}, {HEX, 'h'}, {BINARY, 'B'}, {SHORT, 's'}, {CHAR, 'C'}, {MAP, 'm'}, {BULK, 'H'},
};

const map<translation_type,char> count_for_method = {
    {TEXT0,0}, {TEXT8,0}, {INT,1}, {HEX,1}, {BINARY,4}, {SHORT,2}, {CHAR,4}, {MAP,1}
};

const map<translation_type,char> size_for_method = {
    {TEXT0,0}, {TEXT8,8}, {INT,4}, {HEX,4}, {BINARY,1}, {SHORT,2}, {CHAR,1}, {MAP,291}
};

struct decode_for_group {
    translation_type method = BULK;
    int count = count_for_method.at(method);
};

struct decode_for_line {
    string comment = "";
    string (*translate_fn)(string line) = nullptr;
    bool has_alt_decode = false;
    translation_type method = HEX;
};


string translate_skill(string line) { return line; }

string translate_difficulty(string value) {
    int as_int = stoi(value);
    const string difficulties[] = { "Apprentice", "Journeyman", "Adventurer", "Rogue", "Swashbuckler" };
    if (as_int >= 0 && as_int < sizeof(difficulties)) {
        return difficulties[as_int];
    } else {
        return "UNKNOWN";
    }
}
string translate_rank(string value) {
    int as_int = stoi(value);
    const string ranks[] = { "No Rank", "Letter_of_Marque", "Captain", "Major", "Colonel",
        "Admiral", "Baron", "Count", "Marquis", "Duke"};
    if (as_int >= 0 && as_int < sizeof(ranks)) {
        return ranks[as_int];
    } else {
        return "UNKNOWN";
    }
}

string translate_nation(string value) {
    int as_int = stoi(value);
    const string ranks[] = { "Spanish", "English", "French", "Dutch"};
    if (as_int >= 0 && as_int < sizeof(ranks)) {
        return ranks[as_int];
    } else {
        return "UNKNOWN";
    }
}

string load_city(string value) {
    return "";
}

map<string,decode_for_line> line_decode = {
    {"Intro_0", {"", nullptr, true, TEXT0}},
    {"Intro_1", {"You are here x", nullptr, true, INT}},
    {"Intro_2", {"You are here y", nullptr, true, INT}},
    {"Intro_4", {"Difficulty", translate_difficulty, true, INT}},
    {"Intro_5", {"Last city visited", nullptr, true, INT}}, // Update this to be more clear
    {"Personal_2_0",   {"on land/marching perspective/0/0/0/not in battle or city/0/0"}},
    {"Personal_5_0",   {"Spanish Attitude"}},
    {"Personal_5_1",   {"English Attitude"}},
    {"Personal_6_0",   {"French Attitude"}},
    {"Personal_6_1",   {"Dutch Attitude"}},
    {"Personal_9_0",   {"Spanish Rank", translate_rank}},
    {"Personal_9_1",   {"English Rank", translate_rank}},
    {"Personal_10_0",  {"French Rank", translate_rank}},
    {"Personal_10_1",  {"Dutch Rank",  translate_rank}},
    {"Personal_17",    {"Starting nation", translate_nation}},
    {"Personal_18_0",  {"000000/have ever danced/1"}},
    {"Personal_19",    {"Crew"}},
    {"Personal_20",    {"Gold on hand"}},
    {"Personal_21",    {"Food"}},
    {"Personal_22",    {"Luxuries"}},
    {"Personal_23",    {"Goods"}},
    {"Personal_24",    {"Spice"}},
    {"Personal_25",    {"Sugar"}},
    {"Personal_26",    {"Cannon"}},
    {"Personal_27",    {"Ship(s) in Fleet"}},
    {"Personal_44",    {"Wealth in gold"}},
    {"Personal_45_1",  {"months at sea"}},
    {"Personal_46_0",  {"Relatives found"}},
    {"Personal_46_1",  {"Lost cities found"}},
    {"Personal_52_0",  {"cook/quartermaster/navigator/surgeon/gunner/cooper/sailmaker/carpenter"}},
};


// Sections of the savegame file, in the order that they appear.
const string sections[] = {"Intro", "CityName", "Personal", "Ship", "f", "City", "CityInfo", "Log", "j", "e", "Quest","LogCount","TopoMap","FeatureMap", "TreasureMap", "SailingMap", "vv", "vvv", "Top10","d", "Villain", "t", "CityLoc", "CoastMap", "k", "LandingParty","m", "ShipName", "Skill"};

struct section {
    string name;
    int bytes_per_line;               // Except for text.
    int count;                        // i.e. how many lines to divide into
    translation_type method = BULK;    // Decode method for this section.
};

// Main description of contents and size of each section, in order.
//    sections with single letter names are generally not understood.
//
const vector<section> section_vector = {
    {"Intro",             4,      6,  HEX},
    {"CityName",          0,    128,  TEXT8},
    {"Personal",          4,     57,  INT},
    {"Ship",           1116,    128,},
    {"f",              1116,    128,},
    {"City",             32,    128,},
    {"CityInfo",        148,    128,},
    {"Log",              28,   1000,},
    {"j",                 4,      1,},
    {"e",                32,     30,},
    {"Quest",            32,     64,},
    {"LogCount",          4,      1,},
    {"TopoMap",         586,    462,},
    {"FeatureMap",      293,    462,},
    {"TreasureMap",     328,      4,},
    {"SailingMap",      293,    462,},
    {"vv",               12,    256,},
    {"vvv",               4,      2,},
    {"Top10",            28,     10,},
    {"d",                36,      1,},
    {"Villain",          36,     28,},
    {"t",                15,      8,},
    {"CityLoc",          16,    128,},
    {"CoastMap",        293,    462,},
    {"k",                 4,      8,},
    {"LandingParty",     32,      8,},
    {"m",                12,      1,},
    {"ShipName",          0,      8,},
    {"Skill",             4,      1,},
};


map<string,decode_for_group> subsection_decode = {
    {"Personal_2",    {BINARY}},
    {"Personal_5",    {SHORT}},
    {"Personal_6",    {SHORT}},
    {"Personal_9",    {SHORT}},
    {"Personal_10",   {SHORT}},
    {"Personal_18",   {BINARY}},
    {"Personal_45",   {SHORT}},
    {"Personal_46",   {SHORT}},
    {"Personal_47",   {CHAR}},
    {"Personal_48",   {CHAR}},
    {"Personal_49",   {CHAR}},
    {"Personal_50",   {CHAR}},
    {"Personal_51",   {CHAR}},
    {"Personal_52",   {BINARY}},
};


const string items[] = {
    "balanced swords", "one shot pistol","leather vest", "fencing shirt (puffy)",
    "quality spyglass","dutch rutter","weather glass","3 string fiddle",
    "false mustache","medicinal herbs","shrunken head","golden cross",
    "ruby ring","calfskin boots","french chapeau","lock picking kit",
    "signaling mirror","","","",
    
    "perfectly balance swords", "brace of pistols", "metal cuiraiss", "silk shirt",
    "fine telescope", "spanish rutter", "precision barometer", "concertina",
    "theatrical disguise", "Incan mystic salve", "carved shaman stick", "sacred relic",
    "diamond necklace", "dancing slippers", "ostrich feather hat", "skeleton key",
    "signal flare","","","",
};

void augment_decoder_groups() {
    // The items need comments in the decoder group that are all the same,
    // so I'm adding these programmatically. This is sort of like a translation function,
    // but just puts in a comment on each one which is constant regardless of the value.
    int c = sizeof(items)/sizeof(items[0])/2;
    
    for (int i=0; i<c; i++) {
        int p = 47 + (i/4);
        int pp = i%4;
        string line = "Personal_" + to_string(p) + "_" + to_string(pp);
        string comment = "1=" + items[i] + ",    2=" + items[i+c];
        line_decode[line] = {comment};
    }
}

string find_file(string dir, string file, string suffix) {
    // finds a file that is to be packed or unpacked
    // returns the full pathname of the file.
    string game = file;
    game = regex_replace(game, regex("\\." + pst), "");
    game = regex_replace(game, regex("\\." +  pg), "");
    
    const string possible_files[] = {
        game + "." + suffix,
        dir + "/" + game + "." + suffix,
    };
    for (auto afile : possible_files) {
        ifstream f(afile.c_str());
        if (f.good()) {
            return afile;
        }
    }
    cout << "Could not find file " << file << "\n";
    cout << "Looked for: \n";
    for (auto afile : possible_files) {
        cout << "  '" << afile << "'\n";
    }
    exit(1);
}

void unpack_section (section section, ifstream & in, ofstream & out);

void unpack_pg_to_pst(string pg_file, string pst_file) {
    ifstream pg_in = ifstream(pg_file, ios::binary);
    if (! pg_in.is_open()) {
        cerr << "Failed to read from " << pg_file << "\n";
        exit(1);
    }
    cout << "Unpacking " << pg_file << "\n";
    
    ofstream pst_out = ofstream(pst_file);
    if (! pst_out.is_open()) {
        cerr << "Failed to write to " << pst_file << "\n";
        pg_in.close();
        exit(1);
    }
    cout << "Writing " << pst_file << "\n\n";
    
    for (auto section : section_vector) {
        pst_out << "## " << section.name << " starts at byte " << pg_in.tellg() << "\n";
        unpack_section(section, pg_in, pst_out);
    }
    
    pg_in.close();
    pst_out.close();
}

string read_hex(ifstream & in) { // Read 4 bytes from in and report in hex
    char b[4];
    in.read((char*)&b, sizeof(b));
    string res;
    char buffer[255];
    for (int i=3;i>=0;i--) {
        sprintf(buffer, "%02X", b[i]);
        res += buffer;
        if (i>0) res += '.';
    }
    return res;
}

string read_bulk_hex(ifstream & in, int bytecount) { // Read 4 bytes from in and report in hex
    char b[bytecount];
    in.read((char*)&b, bytecount);
    string res;
    char buffer[bytecount*4];
    for (int i=0;i<bytecount;i++) {
        sprintf(buffer, "%02X", b[i]);
    }
    res = buffer;
    return res;
}

string read_binary(ifstream & in) { // Read 1 byte from in and report as binary string
    char b;
    in.read((char*)&b, sizeof(b));
    std::bitset<8> asbits(b);
    return asbits.to_string();
}

string read_char(ifstream & in) { // Read 1 byte from in and report as an int string
    char b;
    in.read((char*)&b, sizeof(b));
    return to_string(b);
}

string read_short(ifstream &in) { // Read 2 bytes as a signed int and return as string
    char b[2];
    in.read((char*)&b, sizeof(b));
    int B = int((unsigned char)(b[0]) | (char)(b[1]) << 8);
    return to_string(B);
}


unsigned int read_int(ifstream & in) { // Read 4 bytes from in (little endian) and convert to integer
    char b[4];
    in.read((char*)&b, sizeof(b));
    unsigned int B = (unsigned int)((unsigned char)(b[0]) |
                                    (unsigned char)(b[1]) << 8 |
                                    (unsigned char)(b[2]) << 16 |
                                    (unsigned char)(b[3]) << 24    );
    return B;
}

void unpack_section (section section, ifstream & in, ofstream & out) {
    for (int c=0; c<section.count;c++) {
            string subsection = section.name + "_" + to_string(c);
            string subsection_x = regex_replace(subsection, regex(R"(^([^_]+)_\d+)"), "\\${1}_x");
            
            // The subsection or subsection_x have higher priority in translating this line.
            // Example:
            //   subsection   = Ship_0_0_0
            //   subsection_x = Ship_x_0_0
            //   section      = Ship_0_0
            
            if (subsection_decode.count(subsection)) {
                translation_type submeth = subsection_decode.at(subsection).method;
                struct::section sub = {subsection, size_for_method.at(submeth), section.bytes_per_line/size_for_method.at(submeth),  submeth };
                unpack_section(sub, in, out); continue;
            } else if (subsection_decode.count(subsection_x)) {
                translation_type submeth = subsection_decode.at(subsection_x).method;
                struct::section sub = {subsection_x, size_for_method.at(submeth), section.bytes_per_line/size_for_method.at(submeth),  submeth };
                unpack_section(sub, in, out); continue;
            }
    
        auto method = section.method;
            if (line_decode.count(subsection)) { // Rarely, a line can override the section decode method.
                if (line_decode.at(subsection).has_alt_decode) {
                    method = line_decode.at(subsection).method; }
            }
            
            string value;
            char buffer[255] = "";
            unsigned int size_of_string;
            
            switch (method) {
                case TEXT0 : // Stores a length in chars, followed by the text, possibly followed by 0 padding.
                    size_of_string = read_int(in);
                    if (size_of_string > 100) { abort(); }
                    in.read((char *)& buffer, size_of_string);
                    value = buffer;
                    break;
                case TEXT8 : //size_of_string = read_int(in);
                    size_of_string = read_int(in);
                    if (size_of_string > 100) { abort(); }
                    in.read((char *)& buffer, size_of_string);
                    value = buffer;
                    in.read(buffer, 8); // Padding
                    break;
                case INT :
                    value = to_string(read_int(in));
                    break;
                case BULK :
                    value = read_bulk_hex(in, section.bytes_per_line);
                    break;
                case HEX :
                    value = read_hex(in);
                    break;
                case BINARY :
                    value = read_binary(in);
                    break;
                case SHORT :
                    value = read_short(in);
                    break;
                case CHAR :
                    value = read_char(in);
                    break;
                default:
                    break;
            }
            
            string translation;
            if (line_decode.count(subsection)) {
                if (line_decode.at(subsection).translate_fn != nullptr) {
                    translation = line_decode.at(subsection).translate_fn(value);
                    if (translation != "") {
                        translation = "(" + translation + ")";
                    }
                }
            }
            
            string comment;
            if (line_decode.count(subsection)) {
                comment = line_decode.at(subsection).comment;
            }
            
            out << subsection << "   : " << char_for_method.at(method) << to_string(section.bytes_per_line);
            out << "   :   " << value << "   :   " << comment << " " << translation << "\n";
            
        }
    
}

