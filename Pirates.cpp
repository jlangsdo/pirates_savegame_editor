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
#include <cmath>
#include "ship_names.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

int index_from_linecode (string line_code);
const int number_of_true_cities = 44;

//
enum translation_type { TEXT0, TEXT8, HEX, INT, BINARY, SHORT, CHAR, mFLOAT, uFLOAT, MAP, BULK, ZERO };

// The translation types have one character abbreviations in the pst file.
const map<translation_type,char> char_for_method = {
    {TEXT0,'t'}, {TEXT8,'t'}, {INT, 'V'}, {HEX, 'h'}, {BINARY, 'B'}, {SHORT, 's'}, {CHAR, 'C'}, {MAP, 'm'}, {BULK, 'H'},
    {ZERO,'x'}, {uFLOAT,'G'},
};

const map<translation_type,char> size_for_method = {
    {TEXT0,0}, {TEXT8,8}, {INT,4}, {HEX,4}, {BINARY,1}, {SHORT,2}, {CHAR,1}, {MAP,291}, {ZERO,0},
    {uFLOAT,4}, {BULK, 4},
};

enum translatable {
    // All translatable enums should be mapped in the translation_lists
    NIL, RANK, DIFFICULTY, NATION, FLAG, SKILL, SPECIAL_MOVE, SHIP_TYPE,
    DISPOSITION, BEAUTY, SHORT_UPGRADES, LONG_UPGRADES, CITYNAME, DIRECTION,
    SHORT_DIRECTION,
    WEALTH_CLASS, POPULATION_CLASS, SOLDIERS, FLAG_TYPE, SPECIALISTS,
    // or in the translation_functions
    DIR, SHIPNAME, STORE_CITYNAME, DATE, FOLLOWING, SPECIALIST, CITY_BY_LINECODE, WEALTH, POPULATION,
    POPULATION_TYPE, ACRES, LUXURIES_AND_SPICES, BEAUTY_AND_SHIPWRIGHT
};

map <translatable, vector<string>> translation_lists = {
    { RANK,
        { "No Rank", "Letter_of_Marque", "Captain", "Major", "Colonel",
        "Admiral", "Baron", "Count", "Marquis", "Duke"}},
    { DIFFICULTY,
        { "Apprentice", "Journeyman", "Adventurer", "Rogue", "Swashbuckler" }},
    { NATION, { "Spanish", "English", "French", "Dutch"}},
    { FLAG,      {"Spanish",      "English",      "French",      "Dutch",      "Pirate", "Indian", "Jesuit", "Settlement"}},
    { FLAG_TYPE, {"Spanish City", "English City", "French City", "Dutch City", "Pirate", "Indian", "Jesuit", "Settlement"}},
    { SKILL, { "Fencing", "Gunnery", "Navigation", "Medicine", "Wit and Charm"}},
    { SPECIAL_MOVE, { "IDLE", "high chop", "jump", "swing", "parry", "dodge", "duck",
        "low slash", "quick thrust", "taunt", "NONE"}},
    { SHIP_TYPE,
        {   "War Canoe",   "Sloop",        "Brigatine",   "Coastal Barque", "Fluyt",         "Merchantman",       "Frigate",          "Fast Galleon", "Trade Galleon",
            "Pinnace",     "Sloop of War", "Brig",        "Barque",         "Large Fluyt",   "Large Merchantman", "Large Frigate",    "War Galleon",  "Royal Galleon",
            "Mail Runner", "Royal Sloop",  "Brig of War", "Ocean Barque",   "West Indiaman", "East Indiaman",     "Ship of the Line", "Flag Galleon", "Treasure Galleon"}},
    
    { DISPOSITION, { "", "pirate hunter", "privateer", "raider", "smuggler", "?", "escort"}},
    { BEAUTY, {"rather plain", "attractive", "beautiful"}},
    { SHORT_UPGRADES, {
        "copper",                "sails",        "hammocks",      "scantlings",
        "chain",      "grape",                 "powder", "bronze"}},
    { LONG_UPGRADES, {
        "copper plating", "cotton sails", "triple hammocks", "iron scantlings",
        "chain shot", "grape shot", "fine grain powder", "bronze cannon"}},
    { DIRECTION, {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW", "N"}},
    { SHORT_DIRECTION, {"N", "NE", "E", "SE", "S", "SW", "W", "NW", "N"}},
    // CITYNAME is loaded during the reading of the CityName section, for use in other sections like Ship.
    { WEALTH_CLASS, {"quiet and desolate", "baking in the sun", "bustling with activity","clean and prosperous", "brimming with wealth",}},
    { POPULATION_CLASS, {"Farmers", "Colonists",  "Craftsmen", "Landowners", "Citizens","Merchants"}},
    { SPECIALISTS, {"carpenter", "sailmaker", "cooper", "gunner", "surgeon", "navigator", "quartermaster", "cook"}},

};

string translate_dir (string value, string line_code);
string translate_date (string value, string line_code);
string translate_specialist (string value, string line_code);
string translate_city_by_linecode(string value, string line_code);
string translate_wealth(string value, string line_code);
string translate_population_type(string value, string line_code);
string translate_population(string value, string line_code);

string translate_soldiers(string value, string line_code) {
    int as_int = stoi(value);
    if (as_int > 0) { return to_string(as_int*20);}
    else { return "None";}
}
string translate_acres(string value, string line_code) {
    return to_string(50*stol(value)) + " acres";
    
}
string translate_luxuries_and_spices(string value, string line_code) {
    long as_int = stol(value)/2;
    if (as_int > 49) { as_int = 49;}
    if (as_int == 0) { return ""; }
    return to_string(as_int);
}

string translate_population(string value, string line_code) {
    return to_string( 200 * stol(value));
}

string translate_following(string value, string line_code) {
    if (value == "-2") { return "following player";}
    else { return ""; }
}

string store_cityname (string value, string line_code) {
    // Save names of cities for later translations.
    translation_lists[CITYNAME].push_back(value);
    return "";
}

string store_flag(string value, string line_code);
string translate_beauty_and_shipwright(string value, string line_code);

// Translations that require special effort or which are called to store data.
map <translatable, string (*)(string, string)> translation_functions = {
    { DIR, translate_dir },
    { STORE_CITYNAME, store_cityname },
    { FLAG, store_flag },
    { SHIP_TYPE, save_last_shiptype },
    { SHIPNAME, translate_shipname },
    { DATE, translate_date },
    { FOLLOWING, translate_following },
    { SPECIALIST, translate_specialist },
    { CITY_BY_LINECODE, translate_city_by_linecode},
    { SOLDIERS, translate_soldiers},
    { POPULATION, translate_population},
    { WEALTH, translate_wealth},
    { LUXURIES_AND_SPICES, translate_luxuries_and_spices},
    { BEAUTY_AND_SHIPWRIGHT, translate_beauty_and_shipwright},
    { POPULATION_TYPE, translate_population_type},
    { ACRES, translate_acres},
};


int read_hex (char c) { // Reads a char ascii value, returns digital equivalent when read as hex.
    int res = (int)(c-'0');
    if (res >= 0 && res <= 9) { return res;}
    
    res = (int)(c-'A'+10);
    if (res >=10 && res <=15) { return res;}
   
    throw out_of_range("Bad digit conversion to hex for " + to_string(c));
}


string simple_translate (translatable t, int as_int) {
    if (translation_lists.count(t)) {
        // Simple translation from a list.
        vector<string> list = translation_lists.at(t);
        if (as_int >= 0 && as_int < list.size()) {
            if (list.at(as_int).size() > 0) {
                return list.at(as_int);
            } else { return ""; }
        } else {
            // For backward compatability to perl version of this code.
            return "NIL";
        }
    }
    return "";
}
string simple_translate (translatable t, string value) {    return simple_translate(t, (int)stol(value)); }



string translate(translatable t, string value, string line_code) {
    if (t == NIL) { return ""; }
    
    string return_value = "";
    
    if (translation_functions.count(t)) {
        // Special translations that require their own functions,
        // or which store this data for future translations.
        return_value = translation_functions.at(t)(value, line_code);
    }
    
    if (translation_lists.count(t)) {
        return_value = simple_translate(t, value);
    }
    
    // Placeholders are allowed, so a translatable that doesn't show up in either map is OK.
    // TODO: Eventually this might want to be an error.
    
    // Note that if a translatable has both a function and a simple list, then we call the function
    // first and then return the value from the list. This is to cover the common case where the function
    // is for storing the value somewhere.
    
    if (return_value != "") { return_value = "(" + return_value + ")"; }
    return return_value;
}


string store_flag(string value, string line_code){
    string myflag = simple_translate(FLAG, value);
    myflag = regex_replace(myflag, regex("[\\(\\)]"),"");
    save_last_flag(myflag);
    return "";
}

vector<int> stored_city_wealth (128);
string translate_wealth(string value, string line_code) {
    int index = index_from_linecode(line_code);
    int wealth = (int)stol(value);    // Again, problem may be that INT should be signed.
    stored_city_wealth[index] = wealth;
    
    if (wealth != 300 ) {
        return simple_translate(WEALTH_CLASS, wealth/40 );
    } else { return ""; }
}

string translate_population_type (string value, string line_code) {
    // Cities are classified as having different sorts of population as a combination
    // of the Economy CityInfo_x_0_4 and the wealth City_x_5
    
    int magic_number = (int)translation_lists[POPULATION_CLASS].size() /2; // == 3
    
    int index = index_from_linecode(line_code);
    if (index >= number_of_true_cities) { return ""; } // Settlements do not get this
    
    long pop_group = stol(value)/67;
    if (pop_group >= magic_number) { pop_group = magic_number-1;} // TODO I think the real problem is that INT should be signed.
    int is_wealthy = (stored_city_wealth[index] > 100) ? 1:0;
    
    return simple_translate(POPULATION_CLASS, (int)pop_group + magic_number*is_wealthy);
}

string translate_dir (string value, string line_code) {
    char first_char = value.at(0);
    char second_char = value.at(1);
    
    int dir = read_hex(first_char);
    int next_digit = read_hex(second_char);
    if (next_digit > 7) { dir++; }
    
    return simple_translate(DIRECTION, dir);
}

int index_from_linecode (string line_code) { // Given Ship_23_1_4, returns 23
    string line = regex_replace(line_code, regex("^[^_]+_"), "");
    line = regex_replace(line,regex("[ _].*"), "");
    return stoi(line);
}

string translate_city_by_linecode (string value, string line_code) { // In this case, we aren't translating the value,
    int index = index_from_linecode(line_code);           // but rather noting which cityname goes with this line_code.
    return simple_translate(CITYNAME, index);
}

string translate_specialist (string value, string line_code) {
    // If a specialist is on board, then Ship_x_5_5 will be set to 10
    // and which specialist it is depends on the ship number.
    if (value != "10") { return ""; }
    int index = index_from_linecode(line_code);
    return simple_translate(SPECIALISTS, index%8) + " on board";
}

string translate_beauty_and_shipwright(string value, string line_code) {
    int city_index = index_from_linecode(line_code);
    int city_value = (stoi(value)+city_index)%8;
    string retval = "Shipwright can provide " + simple_translate(LONG_UPGRADES, city_value);
    if (city_index < number_of_true_cities) { // Only main cities also have a governor's daughter.
        retval = "Daughter is " + simple_translate(BEAUTY, value) + ", and " + retval;
    }
    return retval;
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
    // {"Personal_52_0",  {"cook/quartermaster/navigator/surgeon/gunner/cooper/sailmaker/carpenter"}}, // do this automatically below.
    //    {"Ship_0_0_0", {"Player Flagship Type", SHIP_TYPE }},   // TODO: Put this back in later. Perl code has a bug that makes this not work.
    {"Ship_x_0_0", {"Ship Type",       SHIP_TYPE }},
    {"Ship_x_0_1", {"Disposition",     DISPOSITION }},
    {"Ship_x_0_2", {"Flag",            FLAG   }},
    {"Ship_x_0_4", {"Target Ship"}},
    {"Ship_x_0_6", {"x Coordinate"}},
    {"Ship_x_1_0", {"y Coordinate"}},
    {"Ship_x_1_1", {"direction",       DIR  }},
    {"Ship_x_1_2", {"speed"}},
    {"Ship_x_2_0", {"% Damage Sails"}},
    {"Ship_x_2_1", {"% Damage Hull"}},
    {"Ship_x_2_2", {"",                 FOLLOWING  }},
    {"Ship_x_2_3", {"Crew aboard"}},
    {"Ship_x_2_4", {"Cannon aboard"}},
    // {"Ship_x_2_6_0", {"upgrades bronze/powder/grape/chain/scantlings/hammocks/sails/copper"}}, // do this automatically below
    {"Ship_x_2_7", {"Name code",        SHIPNAME  }},//
    {"Ship_x_3_0", {"Gold aboard"}},
    {"Ship_x_3_1", {"Food aboard"}},
    {"Ship_x_3_2", {"Luxuries aboard"}},
    {"Ship_x_3_3", {"Goods aboard"}},
    {"Ship_x_3_4", {"Spice aboard"}},
    {"Ship_x_3_5", {"Sugar aboard"}},
    {"Ship_x_3_7", {"Starting city",       CITYNAME }},
    {"Ship_x_4_0", {"Destination city",    CITYNAME }},
    {"Ship_x_4_4", {"DateStamp",           DATE }},
    {"Ship_x_4_7", {"sails"}},
    {"Ship_x_1_3", {"roll"}},
    {"Ship_x_5_4_0", {"returning/?/?/?/?/?/?/?"}},
    {"Ship_x_5_4_1", {"?/?/?/treasure fleet/?/notable/?/docked"}},
    {"Ship_x_5_5", {"",                    SPECIALIST }},
    {"Ship_x_5_6", {"Countdown until leaving port"}},
    {"Ship_x_6_0", {"Battling"}},
    {"Ship_x_6_1", {"Escorted By"}},
    {"City_x_0",   {"x coordinate",     CITY_BY_LINECODE }},
    {"City_x_1",   {"y coordinate",           }},
    {"City_x_2_0", {"?/0/0/0/0/?/has a port/details visible"}},
    {"City_x_2_1", {"city without port/0/?/news this month/grudge/indian_or_jesuit/0/?", CITY_BY_LINECODE}},
    {"City_x_2_2", {"0/0/0/0/0/daughter visible/grateful/unfriendly", CITY_BY_LINECODE}},
    {"City_x_2_3", {"city_off/0/0/0/0/?/?/?",  CITY_BY_LINECODE}},
    {"City_x_3_0", {"Flag",        FLAG}},
    {"City_x_3_1", {"Population",  POPULATION}},
    {"City_x_3_2", {"Soldiers",    SOLDIERS}},
     // "City_x_3_3" is a number from 0..4. The 3 and 4 values are for wealthy spanish capitals.
     // The value is the same for all savegames. Probably a target city class.
    {"City_x_5",   { "Wealth",    WEALTH}},
    {"City_x_6",   {"Type",       FLAG_TYPE}},
    {"CityInfo_x_0_3_0", {"Popularity"}},
    {"CityInfo_x_0_3_1", {"Unpopularity"}},
    {"CityInfo_x_0_8", {"Land Grant", ACRES}},
    {"CityInfo_x_0_1", {"", BEAUTY_AND_SHIPWRIGHT}},
    {"CityInfo_x_0_2_0", {"Hearts"}},
    {"CityInfo_x_0_4", {"Economy (0-200)", POPULATION_TYPE}},
    {"CityInfo_x_0_5", {"Mayor Delivered"}},
    {"CityInfo_x_1_0", {"3x Merchant's gold on hand", }},
    {"CityInfo_x_1_2", {"Merchant's Food on hand",}},
    {"CityInfo_x_1_3", {"Merchant's Luxuries on hand", LUXURIES_AND_SPICES}},
    {"CityInfo_x_1_4", {"Merchant's Goods on hand"}},
    {"CityInfo_x_1_5", {"Merchant's Spice on hand",    LUXURIES_AND_SPICES}},
    {"CityInfo_x_1_6", {"Merchant's Sugar on hand"}},
    {"CityInfo_x_1_10", {"World Map price of Food"}},
    {"CityInfo_x_1_11", {"World Map price of Luxuries"}},
    {"CityInfo_x_1_12", {"World Map price of Goods"}},
    {"CityInfo_x_1_13", {"World Map price of Spice"}},
    {"CityInfo_x_1_14", {"World Map price of Sugar"}},
    {"CityInfo_x_1_15", {"World Map price of Cannon"}},
    {"CityInfo_x_1_16", {"Recruitable Crew Baseline", CITY_BY_LINECODE}},
    {"CityInfo_x_3_11", {"ship launch direction 0-7", SHORT_DIRECTION}},
    {"CityInfo_x_4", {"", CITY_BY_LINECODE}},

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
// Sections will then be broken up further and further, recurvively. There are two key rules
//   1. Dewey Decimal Rule: You can break up a section into subsections, as long as their bytes add up to that of the parent.
//                          This allows more translation of bytes within a subsection without renumbering any other sections.
//   2. Backward Compatible: The pst file gives enough information about each line to restore the pirates_savegame file,
//                           even if the pst decoding has changed.
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

// Split up a section into multiple lines of identically sized smaller types, assuming that they will
// use the default byte counts for that type.
// The size of the new translation_type should divide evenly into the original line size (this is checked at runtime)
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
    {"City_x",        INT},
    {"City_x_2",      BINARY},
    {"City_x_4",      BULK},
    {"City_x_7",      BULK},
    {"CityInfo_x_0_2", SHORT},
    {"CityInfo_x_0_3", SHORT},
    {"CityInfo_x_3",   SHORT},
};

// split up a section into multiple sections that may be of different or variable types and sizes.
// Make sure the bytes add up to the size of the original section (this is checked at runtime).
//
map<string,vector<subsection_info>> subsection_manual_decode = {
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


void augment_from_translation_list(translatable t, string line_code, string prefix = "", string delimiter = "/") {
    // Upgrade and Specialist lists are accessed in the translation_lists because they can appear in decodes.
    // BUT they also appear in a binary decode where I need to see all of them in a line in reverse order.
    string sp = "";
    for (auto up : translation_lists.at(t)) {
        sp = delimiter + up + sp;
    }
    sp = regex_replace(sp,regex("^" + delimiter), prefix);
    line_decode[line_code] = {sp};
}

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
    
    augment_from_translation_list(SPECIALISTS, "Personal_52_0");
    augment_from_translation_list(SHORT_UPGRADES, "Ship_x_2_6_0", "upgrades ");
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

int starting_year;
void store_startingyear(ifstream & in) {
    // We're going to jump way forward in the file to read the start year, and then put it into a persistent
    // variable, for use in decoding DateStamps. Then jump right back.
    constexpr int jump_dist = 887272;
    in.seekg(jump_dist, ios_base::cur);
    starting_year = (int)read_int(in);
    in.seekg(-jump_dist-4, ios_base::cur);
}

string translate_date(string value, string section) { // Translate the datestamp into a date in game time.
    double stamp = stoi(value);
    if (stamp == 0 || stamp == -1) { return ""; }
    
    // I have no idea where the 197.2 comes from; in this formula.
    // I probably derived it empirically when working out the perl version
    // Perhaps it should really be 200.
    //
    // As I read it, the datestamp integer increases by about 197.2 each day of game time.
    //
    time_t myt = stamp*24*3600/197.2;
    stringstream st;
    
    st << std::put_time(std::gmtime(& myt), "%b %e, "); // month and day
    string retval = st.str();
    
    st.str("");   // Clear the stream.
    st << std::put_time(std::gmtime(& myt), "%Y\n");
    string syear = st.str();
    int year = stoi(syear) - 1970 + starting_year;   // Changing the epoch
    retval += to_string(year);

    return retval;
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
            translation = translate(line_decode.at(subsection_x).t, value, subsection);
        }
        if (line_decode.count(subsection)) {
            translation = translate(line_decode.at(subsection).t, value, subsection);
        }
        
        string comment;
        if (line_decode.count(subsection_x)) {
            comment = line_decode.at(subsection_x).comment;
        }
        if (line_decode.count(subsection)) {
            comment = line_decode.at(subsection).comment;
        }
        
        // Fix this.
        int linesize = bytes_per_line;
        if (method==TEXT0) { linesize = 0;}
        if (method==TEXT8) { linesize = 8;}
        
        //
        if (subsection == "Personal_0") {
            store_startingyear(in);
        }
        
        out << subsection << "   : " << char_for_method.at(method) << to_string(linesize);
        out << "   :   " << value << "   :   " << comment << " " << translation << "\n";
    
    }
    
}

