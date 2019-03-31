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
using namespace std;

// Sections of the savegame file, in the order that they appear.
const string sections[] = {"Intro", "CityName", "Personal", "Ship", "f", "City", "CityInfo", "Log", "j", "e", "Quest","LogCount","TopoMap","FeatureMap", "TreasureMap", "SailingMap", "vv", "vvv", "Top10","d", "Villain", "t", "CityLoc", "CoastMap", "k", "LandingParty","m", "ShipName", "Skill"};

//
enum translation_type { TEXT, HEX, INT, BINARY, SHORT, CHAR, mFLOAT, uFLOAT, MAP };

// The translation types have one character abbreviations in the pst file.
const map<translation_type,char> char_for_method = {
    {TEXT,'t'}, {INT, 'V'}, {HEX, 'h'}, {BINARY, 'B'}, {SHORT, 's'}, {CHAR, 'C'}
};

struct decoder_group {
    int count;
    translation_type method;
    int size;
    string comment = "";
    string (*translate_fn)(string line) = nullptr;
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

map<string,decoder_group> decoder_groups = {
    {"Intro",   {6, HEX, 4 }},
    {"Intro_0", {1, TEXT, 0 }},
    {"Intro_1", {1, INT, 4, "You are here x",}},
    {"Intro_2", {1, INT, 4, "You are here y"}},
    {"Intro_4", {1, INT, 4, "Difficulty", translate_difficulty }},
    {"Intro_5", {1, INT, 4, "Last city visited"}}, // Update this to be more clear
    {"Skill",   {1, INT, 4, "", translate_skill}},
    {"CityName", {128, TEXT, 8, "", load_city}},
    {"Personal",     {57, INT, 4}},
    {"Personal_2",    {4,BINARY, 1}},
    {"Personal_2_0",   {1, BINARY,1, "on land/marching perspective/0/0/0/not in battle or city/0/0"}},
    {"Personal_5",    {2, SHORT, 2}},
    {"Personal_5_0",   {1, SHORT, 2, "Spanish Attitude"}},
    {"Personal_5_1",   {1, SHORT, 2, "English Attitude"}},
    {"Personal_6",    {2, SHORT, 2}},
    {"Personal_6_0",   {1, SHORT, 2, "French Attitude"}},
    {"Personal_6_1",   {1, SHORT, 2, "Dutch Attitude"}},
    {"Personal_9",    {2, SHORT, 2}},
    {"Personal_9_0",   {1, SHORT, 2, "Spanish Rank", translate_rank}},
    {"Personal_9_1",   {1, SHORT, 2, "English Rank", translate_rank}},
    {"Personal_10",   {2, SHORT, 2}},
    {"Personal_10_0",  {1, SHORT, 2, "French Rank", translate_rank}},
    {"Personal_10_1",  {1, SHORT, 2, "Dutch Rank",  translate_rank}},
    {"Personal_17",   {1, INT, 4, "Starting nation", translate_nation}},
    {"Personal_18",   {4, BINARY, 1}},
    {"Personal_18_0",  {1, BINARY, 1, "000000/have ever danced/1"}},
    {"Personal_19",   {1, INT, 4, "Crew"}},
    {"Personal_20",   {1, INT, 4, "Gold on hand"}},
    {"Personal_21",   {1, INT, 4, "Food"}},
    {"Personal_22",   {1, INT, 4, "Luxuries"}},
    {"Personal_23",   {1, INT, 4, "Goods"}},
    {"Personal_24",   {1, INT, 4, "Spice"}},
    {"Personal_25",   {1, INT, 4, "Sugar"}},
    {"Personal_26",   {1, INT, 4, "Cannon"}},
    {"Personal_27",   {1, INT, 4, "Ship(s) in Fleet"}},
    {"Personal_44",   {1, INT, 4, "Wealth in gold"}},
    {"Personal_45",   {2, SHORT, 2}},
    {"Personal_45_1",   {1, SHORT, 2, "months at sea"}},
    {"Personal_46",   {2, SHORT, 2}},
    {"Personal_46_0",   {1, SHORT, 2, "Relatives found"}},
    {"Personal_46_1",   {1, SHORT, 2, "Lost cities found"}},
    {"Personal_47",   {4, CHAR, 1}},
    {"Personal_48",   {4, CHAR, 1}},  // NOTE augment_decoder_groups below.
    {"Personal_49",   {4, CHAR, 1}},
    {"Personal_50",   {4, CHAR, 1}},
    {"Personal_51",   {4, CHAR, 1}},
    {"Personal_52",   {4, BINARY,1}},
    {"Personal_52_0",  {1, BINARY,1, "cook/quartermaster/navigator/surgeon/gunner/cooper/sailmaker/carpenter"}},
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
        string group = "Personal_" + to_string(p) + "_" + to_string(pp);
        string comment = "1=" + items[i] + ",    2=" + items[i+c];
        decoder_groups[group] = {1, CHAR, 1, comment};
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
    
    for (auto section : sections) {
        pst_out << "## " << section << " starts at byte " << pg_in.tellg() << "\n";
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

void unpack_section (string section, ifstream & in, ofstream & out) {
    if (decoder_groups.count(section) != 0) {
        decoder_group dec = decoder_groups.at(section);
        
        for (int c=0; c<dec.count;c++) {
            string subsection = section + "_" + to_string(c);
            string subsection_x = regex_replace(subsection, regex(R"(^([^_]+)_\d+)"), "\\${1}_x");
            
            auto decode = dec;
            // The subsection or subsection_x have higher priority in translating this line.
            // Example:
            //   subsection   = Ship_0_0_0
            //   subsection_x = Ship_x_0_0
            //   section      = Ship_0_0
            
            if (decoder_groups.count(subsection)) {
                decode = decoder_groups.at(subsection);
                if (decode.count > 1) { unpack_section(subsection, in, out); continue; }
            } else if (decoder_groups.count(subsection)) {
                decode = decoder_groups.at(subsection_x);
                if (decode.count > 1) { unpack_section(subsection, in, out); continue; }
            }
            
            string value;
            char buffer[255] = "";
            unsigned int size_of_string;
            
            switch (decode.method) {
                case TEXT : // Stores a length in chars, followed by the text, possibly followed by 0 padding.
                    size_of_string = read_int(in);
                    if (size_of_string > 100) { abort(); }
                    in.read((char *)& buffer, size_of_string);
                    value = buffer;
                    in.read(buffer, decode.size); // Padding
                    break;
                case INT :
                    // Only legal if decode.size == 4
                    value = to_string(read_int(in));
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
            if (decode.translate_fn != nullptr) {
                translation = decode.translate_fn(value);
                if (translation != "") {
                    translation = "(" + translation + ")";
                }
            }
            
            out << subsection << "   : " << char_for_method.at(decode.method) << to_string(decode.size);
            out << "   :   " << value << "   :   " << decode.comment << " " << translation << "\n";
            
        }
    }
}

