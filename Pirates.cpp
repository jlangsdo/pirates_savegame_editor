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
#include "ship_names.hpp"
using namespace std;



//
enum translation_type { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, mFLOAT, uFLOAT, MAP, BULK, ZERO };

// The translation types have one character abbreviations in the pst file.
const map<translation_type,char> char_for_method = {
    {TEXT0,'t'}, {TEXT8,'t'}, {INT, 'V'}, {HEX, 'h'}, {BINARY, 'B'}, {SHORT, 's'}, {CHAR, 'C'}, {MAP, 'm'}, {BULK, 'H'},
    {ZERO,'x'}, {uFLOAT,'G'},
};

const map<translation_type,char> size_for_method = {
    {TEXT0,0}, {TEXT8,8}, {INT,4}, {HEX,4}, {BINARY,1}, {SHORT,2}, {CHAR,1}, {MAP,291}, {ZERO,0},
    {uFLOAT,4},
};

enum translatable {
    // All translatable enums should be mapped in the translation_lists
    NIL, RANK, DIFFICULTY, NATION, FLAG, SKILL, SPECIAL_MOVE, SHIP_TYPE,
    DISPOSITION, BEAUTY, UPGRADES, CITYNAME, DIRECTION,
    // or in the translation_functions
    DIR, SHIPNAME, STORE_CITYNAME,
};

map <translatable, vector<string>> translation_lists = {
    { RANK,
        { "No Rank", "Letter_of_Marque", "Captain", "Major", "Colonel",
        "Admiral", "Baron", "Count", "Marquis", "Duke"}},
    { DIFFICULTY,
        { "Apprentice", "Journeyman", "Adventurer", "Rogue", "Swashbuckler" }},
    { NATION, { "Spanish", "English", "French", "Dutch"}},
    { FLAG, {"Spanish", "English", "French", "Dutch", "Pirate", "Indian", "Jesuit", "Settlement"}},
    { SKILL, { "Fencing", "Gunnery", "Navigation", "Medicine", "Wit and Charm"}},
    { SPECIAL_MOVE, { "IDLE", "high chop", "jump", "swing", "parry", "dodge", "duck",
        "low slash", "quick thrust", "taunt", "NONE"}},
    { SHIP_TYPE,
        {   "War Canoe",   "Sloop",        "Brigatine",   "Coastal Barque", "Fluyt",         "Merchantman",       "Frigate",          "Fast Galleon", "Trade Galleon",
            "Pinnace",     "Sloop of War", "Brig",        "Barque",         "Large Fluyt",   "Large Merchantman", "Large Frigate",    "War Galleon",  "Royal Galleon",
            "Mail Runner", "Royal Sloop",  "Brig of War", "Ocean Barque",   "West Indiaman", "East Indiaman",     "Ship of the Line", "Flag Galleon", "Treasure Galleon"}},
    
    { DISPOSITION, { "", "pirate hunter", "privateer", "raider", "smuggler", "?", "escort"}},
    { BEAUTY, {"rather plain", "attractive", "beautiful"}},
    { UPGRADES, {"copper plating", "cotton sails", "triple hammocks", "iron scantlings",
        "chain shot", "grape shot", "fine grain powder", "bronze cannon"}},
    { DIRECTION, {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW", "N"}},
    // CITYNAME is loaded during the reading of the CityName section, for use in other sections like Ship.
};

string translate_dir (string value);

string store_cityname (string value) {
    // Save names of cities for later translations.
    translation_lists[CITYNAME].push_back(value);
    return "";
}

string store_flag(string value);

// Translations that require special effort or which are called to store data.
map <translatable, string (*)(string)> translation_functions = {
    { DIR, translate_dir },
    { STORE_CITYNAME, store_cityname },
    { FLAG, store_flag },
    { SHIP_TYPE, save_last_shiptype },
    { SHIPNAME, translate_shipname },
};


int read_hex (char c) { // Reads a char ascii value, returns digital equivalent when read as hex.
    int res = (int)(c-'0');
    if (res >= 0 && res <= 9) { return res;}
    
    res = (int)(c-'A'+10);
    if (res >=10 && res <=15) { return res;}
   
    throw out_of_range("Bad digit conversion to hex for " + to_string(c));
}


string simple_translate (translatable t, string value) {
    if (translation_lists.count(t)) {
        // Simple translation from a list.
        int as_int = stoi(value);
        vector<string> list = translation_lists.at(t);
        if (as_int >= 0 && as_int < list.size()) {
            if (list.at(as_int).size() > 0) {
                return "(" + list.at(as_int) + ")";
            } else { return ""; }
        } else {
            // For backward compatability to perl version of this code.
            return "(NIL)";
        }
    }
    return "";
}

string translate(translatable t, string value) {
    if (t == NIL) { return ""; }
    
    string return_value = "";
    
    if (translation_functions.count(t)) {
        // Special translations that require their own functions,
        // or which store this data for future translations.
        return_value = translation_functions.at(t)(value);
    }
    
    if (translation_lists.count(t)) {
        return_value = simple_translate(t, value);
    }
    
    // Placeholders are allowed, so a translatable that doesn't show up in either map is OK.
    // TODO: Eventually this might want to be an error.
    
    // Note that if a translatable has both a function and a simple list, then we call the function
    // first and then return the value from the list. This is to cover the common case where the function
    // is for storing the value somewhere.
    
    return return_value;
}


string store_flag(string value){
    string myflag = simple_translate(FLAG, value);
    myflag = regex_replace(myflag, regex("[\\(\\)]"),"");
    save_last_flag(myflag);
    return "";
}

string translate_dir (string value) {
    char first_char = value.at(0);
    char second_char = value.at(1);
    
    int dir = read_hex(first_char);
    int next_digit = read_hex(second_char);
    if (next_digit > 7) { dir++; }
    
    return translate(DIRECTION, to_string(dir));
    
}
string load_city(string value) {
    return "";
}

struct decode_for_line {
    string comment = "";
    translatable t = NIL;
};

// These do not have to be in the order that they are in the file, but it makes things easier to read if they are.
map<string,decode_for_line> line_decode = {
    {"Intro_1",        {"You are here x"}},
    {"Intro_2",        {"You are here y"}},
    {"Intro_4",        {"Difficulty", DIFFICULTY}},
    {"Intro_5",        {"Last city visited"}}, // TODO: Update this message to be more clear
    {"CityName_x",     {"", STORE_CITYNAME}},
    {"Personal_2_0",   {"on land/marching perspective/0/0/0/not in battle or city/0/0"}},
    {"Personal_5_0",   {"Spanish Attitude"}},
    {"Personal_5_1",   {"English Attitude"}},
    {"Personal_6_0",   {"French Attitude"}},
    {"Personal_6_1",   {"Dutch Attitude"}},
    {"Personal_9_0",   {"Spanish Rank", RANK}},
    {"Personal_9_1",   {"English Rank", RANK}},
    {"Personal_10_0",  {"French Rank",  RANK}},
    {"Personal_10_1",  {"Dutch Rank",   RANK}},
    {"Personal_17",    {"Starting nation", NATION}},
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
    //    {"Ship_0_0_0", {"Player Flagship Type", SHIP_TYPE }},   // TODO: Put this back in later. Perl code has a bug that makes this not work.
    {"Ship_x_0_0", {"Ship Type",       SHIP_TYPE }},//    translate_ship_type}},
    {"Ship_x_0_1", {"Disposition",     DISPOSITION }},//      translate_disposition}},
    {"Ship_x_0_2", {"Flag",            FLAG   }},
    {"Ship_x_0_4", {"Target Ship"}},
    {"Ship_x_0_6", {"x Coordinate"}},
    {"Ship_x_1_0", {"y Coordinate"}},
    {"Ship_x_1_1", {"direction",       DIR  }},
    {"Ship_x_1_2", {"speed"}},
    {"Ship_x_2_0", {"% Damage Sails"}},
    {"Ship_x_2_1", {"% Damage Hull"}},
    {"Ship_x_2_2", {"",                   }},//     translate_is_following}},
    {"Ship_x_2_3", {"Crew aboard"}},
    {"Ship_x_2_4", {"Cannon aboard"}},
    {"Ship_x_2_6_0", {"upgrades bronze/powder/grape/chain/scantlings/hammocks/sails/copper"}},
    {"Ship_x_2_7", {"Name code",        SHIPNAME  }},//
    {"Ship_x_3_0", {"Gold aboard"}},
    {"Ship_x_3_1", {"Food aboard"}},
    {"Ship_x_3_2", {"Luxuries aboard"}},
    {"Ship_x_3_3", {"Goods aboard"}},
    {"Ship_x_3_4", {"Spice aboard"}},
    {"Ship_x_3_5", {"Sugar aboard"}},
    {"Ship_x_3_7", {"Starting city",       CITYNAME }},
    {"Ship_x_4_0", {"Destination city",    CITYNAME }},
    {"Ship_x_4_4", {"DateStamp",           }},//    translate_date}},
    {"Ship_x_4_7", {"sails"}},
    {"Ship_x_1_3", {"roll"}},
    {"Ship_x_5_4_0", {"returning/?/?/?/?/?/?/?"}},
    {"Ship_x_5_4_1", {"?/?/?/treasure fleet/?/notable/?/docked"}},
    {"Ship_x_5_5", {"",                    }},//    translate_specialist_aboard}},
    {"Ship_x_5_6", {"Countdown until leaving port"}},
    {"Ship_x_6_0", {"Battling"}},
    {"Ship_x_6_1", {"Escorted By"}},
};


// Sections of the savegame file, in the order that they appear.
const string sections[] = {"Intro", "CityName", "Personal", "Ship", "f", "City", "CityInfo", "Log", "j", "e", "Quest","LogCount","TopoMap","FeatureMap", "TreasureMap", "SailingMap", "vv", "vvv", "Top10","d", "Villain", "t", "CityLoc", "CoastMap", "k", "LandingParty","m", "ShipName", "Skill"};

struct section {
    string name;
    int count;                        // i.e. how many lines to divide into
    int bytes_per_line;               // Except for text.
    translation_type method = BULK;    // Decode method for this section.
};

// Main description of contents and size of each section, in order.
//    sections with single letter names are generally not understood.
//
const vector<section> section_vector = {
    {"Intro",             6,       4,  INT },
    {"CityName",        128,       0,  TEXT8 },
    {"Personal",         57,       4,  INT },
    {"Ship",            128,    1116, },
    {"f",               128,    1116, },
    {"City",            128,      32, },
    {"CityInfo",        128,     148, },
    {"Log",            1000,      28, },
    {"j",                 1,       4, },
    {"e",                30,      32, },
    {"Quest",            64,      32, },
    {"LogCount",          1,       4, },
    {"TopoMap",         462,     586, },
    {"FeatureMap",      462,     293, },
    {"TreasureMap",       4,     328, },
    {"SailingMap",      462,     293, },
    {"vv",              256,      12, },
    {"vvv",               2,       4, },
    {"Top10",            10,      28, },
    {"d",                 1,      36, },
    {"Villain",          28,      36, },
    {"t",                 8,      15, },
    {"CityLoc",         128,      16, },
    {"CoastMap",        462,     293, },
    {"k",                 8,       4, },
    {"LandingParty",      8,      32, },
    {"m",                 1,      12, },
    {"ShipName",          8,       0,  TEXT8 },
    {"Skill",             1,       4, },
    
};

struct subsection_info {
    translation_type method;
    int byte_count=size_for_method.at(method);
    int multiplier=1;
};

// Split up a line into multiple lines of identically sized smaller types.
// The size of the new translation_type should divide evenly into the
// original line size.
map<string,translation_type> subsection_simple_decode = {
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
    
};

// split up a section into multiple sections that may be of different types and sizes.
// Make sure the bytes add up to the size of the original section.
map<string,vector<subsection_info>> subsection_manual_decode = {
    {"Personal_51",   {{CHAR}, {ZERO,3}}},           // 4 = 1+3
    {"Personal_52",   {{BINARY}, {BULK,3}}},         // 4 = 1+3
    {"Ship_x",        {{BULK,16, 10}, {ZERO,956}}}, // 1116 = 16*10+956
    {"Ship_x_0",      {{SHORT,2, 6}, {uFLOAT}}},        // 16 = 6*2 + 4
    {"Ship_x_1",      {{uFLOAT}, {HEX,4, 3}}},         // 16 = 4 + 4*3
    {"Ship_x_2_6",    {{BINARY}, {BULK,1}}},           // 2 = 1+1
    {"Ship_x_4",      {{SHORT,2,4}, {INT,4,1}, {ZERO,0,1}, {SHORT,2,2}}},   // 16 = 2*4+4+0+2*2
};

// The zero length zero string for Ship_x_4_5 happened because two adjacent shorts were switched
// for an INT and a ZERO. This manuever is required to avoid renumbering (Ship_x_4_7 numbering remained unchanged)
// This would almost be the same as adding these subsection_simple_decodes:
// {"Ship_x_4",      SHORT},
// {"Ship_x_4_5",    INT},
// {"Ship_x_4_6",    ZERO},
// but that isn't allowed. The problem is that ship_x_4_5 would be starting as a SHORT and moving to INT,
// which means growing from 2 bytes to 4. That's not OK; it means the subsection_decode would be going outside of the subsection.


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
    // but it puts in a comment on each one which is constant regardless of the value.
    //
    // This is better than a translate for the items because you want to be able to see the item names
    // for each line, whether you have them or not.
    //
    int c = sizeof(items)/sizeof(items[0])/2;
    
    for (int i=0; i<c-3; i++) {
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

void unpack_section (section section, ifstream & in, ofstream & out, int offset=0);

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
        sprintf(buffer, "%02X", (unsigned char)b[i]);
        res += buffer;
        if (i>0) res += '.';
    }
    return res;
}

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); } // correct
                   );
    return s;
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

string read_uFloat(ifstream &in) {
    int raw = read_int(in);
    return to_string(double(raw)/1000000);
}

void unpack_section (section section, ifstream & in, ofstream & out, int offset) {
    for (int c=offset; c<section.count+offset;c++) {
        string subsection = section.name + "_" + to_string(c);
        string subsection_x = regex_replace(subsection, regex(R"(^([^_]+)_\d+)"), "$1_x");
        
        // The subsection or subsection_x have higher priority in translating this line.
        // Example:
        //   subsection   = Ship_0_0_0
        //   subsection_x = Ship_x_0_0
        //   section      = Ship_0_0
        
        if (subsection_simple_decode.count(subsection) || subsection_simple_decode.count(subsection_x)) {
            translation_type submeth = subsection_simple_decode.count(subsection) ?
            subsection_simple_decode.at(subsection) : subsection_simple_decode.at(subsection_x);
            
            if (submeth != section.method) { // Avoids infinite loop.
                // Take the the section.byte_count and divide
                // it equally to set up the subsections.
                int count = 1;
                if (size_for_method.at(submeth)>0) {
                    count = section.bytes_per_line/size_for_method.at(submeth);
                }
                int suboffset = 0;
                
                if (count ==1) {
                    // If there is only one count for the subsection, then we aren't really splitting it up;
                    // we are changing the translation method of this particular line.
                    // So, remove the numeric ending and call it again with the corrected method.
                    // But note the if above to avoid an infinite loop.
                    subsection = section.name;
                    suboffset = c;
                }
                
                struct::section sub = {subsection,count, size_for_method.at(submeth) , submeth };
                unpack_section(sub, in, out, suboffset);
                continue;
            }
        }
        
        if (subsection_manual_decode.count(subsection) || subsection_manual_decode.count(subsection_x)) {
            
            vector<subsection_info> info = subsection_manual_decode.count(subsection) ?
            subsection_manual_decode.at(subsection) : subsection_manual_decode.at(subsection_x);
            
            int suboffset = 0;
            int byte_count_check = 0;
            for (auto subinfo : info) {
                translation_type submeth = subinfo.method;
                struct::section sub = {subsection,subinfo.multiplier, subinfo.byte_count , submeth };
                // cerr << subsection << "," << subinfo.multiplier  << "," << subinfo.byte_count << "\n";
                unpack_section(sub, in, out, suboffset);
                suboffset += subinfo.multiplier;
                byte_count_check += subinfo.multiplier*subinfo.byte_count;
            }
            if (byte_count_check != section.bytes_per_line) {
                cerr << "Error decoding line " << subsection << " subsections don't add up: " << byte_count_check << " != " << section.bytes_per_line << "\n";
                out.close();
                abort();
                // It will probably crash later anyway.
            }
            continue;
        }
        
        auto method = section.method;
        auto bytes_per_line = section.bytes_per_line;
        
        string value;
        char buffer[255] = "";
        unsigned int size_of_string;
        
        switch (method) {
            case TEXT0 : // Stores a length in chars, followed by the text, possibly followed by 0 padding.
                size_of_string = read_int(in);
                if (size_of_string > 100) { out.close(); abort(); }
                in.read((char *)& buffer, size_of_string);
                value = buffer;
                break;
            case TEXT8 : //size_of_string = read_int(in);
                size_of_string = read_int(in);
                if (size_of_string > 100) { out.close(); abort(); }
                in.read((char *)& buffer, size_of_string);
                value = buffer;
                in.read(buffer, 8); // Padding
                break;
            case INT :
                value = to_string(read_int(in));
                break;
            case ZERO :
                try {
                    value = read_zeros(in, bytes_per_line);
                } catch(logic_error) {
                    out.close();
                    abort();
                }
                break;
            case BULK :
                value = read_bulk_hex(in, bytes_per_line);
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
            case uFLOAT :
                value = read_uFloat(in);
                break;
            case CHAR :
                value = read_char(in);
                break;
            default:
                break;
        }
        
        string translation;
           
        if (line_decode.count(subsection_x)) {
            translation = translate(line_decode.at(subsection_x).t, value);
        }
        if (line_decode.count(subsection)) {
            translation = translate(line_decode.at(subsection).t, value);
        }
        
        string comment;
        if (line_decode.count(subsection_x)) {
            comment = line_decode.at(subsection_x).comment;
        }
        if (line_decode.count(subsection)) {
            comment = line_decode.at(subsection).comment;
        }
        
        int linesize = bytes_per_line;
        if (method==TEXT0) { linesize = 0;}
        if (method==TEXT8) { linesize = 8;}
        
        out << subsection << "   : " << char_for_method.at(method) << to_string(linesize);
        out << "   :   " << value << "   :   " << comment << " " << translation << "\n";
    
    }
    
}

