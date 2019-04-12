//
//  PstLine.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/2/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

// This file handles one line of the pst file - decoding the comment and translation,
// and getting the spacing right to match the perl version of unpacking.
//
// Note that PstLine is used to pass in data (because a decode function may use .v, .value, or .line_code)
// but the result of the many different translate_* functions is not put into the PstLine, it is returned as a string.
// This is so complicated translations can be built up from different smaller translations.

#include "PstLine.hpp"   // For the read_int routine, needed for starting_year.
#include "ship_names.hpp"   // ship_names is a subset of linedecoding.
#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <regex>
#include <bitset>
#include <sstream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "RMeth.hpp"
using namespace std;

const int number_of_true_cities = 44; // Cities after this number are settlements, indian villages, Jesuit missions, or pirate bases.
int starting_year;

enum translatable : char {
    // All translatable enums should be mapped in the translation_lists
    NIL, NONE, RANK, DIFFICULTY, NATION, FLAG, SKILL, SPECIAL_MOVE, SHIP_TYPE,
    DISPOSITION, BEAUTY, SHORT_UPGRADES, LONG_UPGRADES, CITYNAME, DIR16,
    DIR8, PIRATE,
    EVENT, EVENTS3, EVENTS15, EVENTS32, EVENTS64,
    PURPOSE, PURPOSE0, PURPOSE30, PURPOSE40,
    WEALTH_CLASS, POPULATION_CLASS, SOLDIERS, FLAG_TYPE, SPECIALIST,
    ITEM, BETTER_ITEM, PIRATE_HANGOUT,
    // or in the translation_functions
    SHIPNAME, STORE_CITYNAME, DATE, FOLLOWING, CITY_BY_LINECODE, WEALTH, POPULATION,
    POPULATION_TYPE, ACRES, LUXURIES_AND_SPICES, BEAUTY_AND_SHIPWRIGHT, FURTHER_EVENT, SHIP_SPECIALIST,
    PEACE_AND_WAR, DATE_AND_AGE, TREASURE_MAP, LANDMARK,
    // If it is not mapped in either, that is not an error: it is hook for future code).
};

// Utilities?
int index_from_linecode (string line_code) { // Given Ship_23_1_4, returns 23
    string line = regex_replace(line_code, regex("^[^_]+_"), "");
    line = regex_replace(line,regex("[ _].*"), "");
    return stoi(line);
}
int suffix_from_linecode (string line_code) { // Given Log_1_4, returns 4
    string line = regex_replace(line_code, regex("^.*_"), "");
    return stoi(line);
}

// These lists are most of the strings for 'translation' - explaining what the numerical value
// on a particular line stands for. strings go here if they are indexed by a small number in the savegame file.
// Some additional text is coded into the translation functions below.
unordered_map <translatable, vector<string>> translation_lists = {
    { RANK,
        { "No Rank", "Letter_of_Marque", "Captain", "Major", "Colonel",
            "Admiral", "Baron", "Count", "Marquis", "Duke"}},
    { DIFFICULTY,
        { "Apprentice", "Journeyman", "Adventurer", "Rogue", "Swashbuckler" }},
    { NATION, { "Spanish", "English", "French", "Dutch"}},
    { FLAG,      {"Spanish",      "English",      "French",      "Dutch",      "Pirate", "Indian", "Jesuit", "Settlement"}},
    { FLAG_TYPE, {"Spanish City", "English City", "French City", "Dutch City", "Pirate", "Indian", "Jesuit", "Settlement"}},
    { PIRATE, { "Cap'n Incognito","Henry Morgan",   "Blackbeard",   "Captain Kidd", "Jean Lafitte",   "Stede Bonnet",
        "L'Ollonais","Roc Brasiliano", "Bart Roberts", "Jack Rackham",}},
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
    { DIR16, {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW", "N"}},
    { DIR8, {"N", "NE", "E", "SE", "S", "SW", "W", "NW", "N"}},
    // CITYNAME is loaded during the reading of the CityName section, for use in other sections
    { CITYNAME, vector<string> (128) },
    { WEALTH_CLASS, {"quiet and desolate", "baking in the sun", "bustling with activity","clean and prosperous", "brimming with wealth",}},
    { POPULATION_CLASS, {"Farmers", "Colonists",  "Craftsmen", "Landowners", "Citizens","Merchants"}},
    { SPECIALIST, {"carpenter", "sailmaker", "cooper", "gunner", "surgeon", "navigator", "quartermaster", "cook"}},
    { EVENTS3,  {"Immigrants are flocking to", "Malaria strikes", "Governor sailing to",
        "Pirates attack", "captures city of","Healthy sugar crop in", "Indians attack", "Reinforcements arrive in"}},
    { EVENTS15, {"Pirate hunter sails from", "War between", "No longer war between", "Sign peace treaty",
        "Cancelled peace treaty", "", "settlement captured by"}},
    { EVENTS32, {"Engaged Ship", "Captured Ship", "Sank Ship", "Visited", "Promoted", "Acquired", "Recruited a skilled",
        "Captured wanted criminal in", "Met Governor's daughter", "Dueled daughter's jealous suitor in", "", "", "Found Pirate Treasure",
        "", "","sacked/installed governor", "Defeated notorious pirate", "Marooned on a desert island", "", "Vanquished the Marquis Montalban"}},
    { EVENTS64, {"Treasure fleet headed for","","smuggler sighted", "", "Evil character seen in", "Wanted criminal hiding in" }},
    { EVENT, vector<string> (70)},
    { PURPOSE, vector<string> (70)},
    { PURPOSE0, {"quest","Delivering ultimatum", "Delivering peace treaty", "Transporting immigrants", "Delivering vital medicines",
        "Transporting new governor", "Pirate raiders", "Invasion force", "Transporting sugar plants", "Indian war canoe",
        "Transporting troops", "Proposing amnesty", "Treasure ship", "Military payroll", "Grain transport", "New warship"}},
    { PURPOSE30, {"criminal"}}, {PURPOSE40, {"Raymondo","Mendoza", "Montalban"}},
    { ITEM, {"balanced swords", "one shot pistol","leather vest", "fencing shirt (puffy)", "quality spyglass","dutch rutter",
        "weather glass","3 string fiddle","false mustache","medicinal herbs","shrunken head","golden cross",
        "ruby ring","calfskin boots","french chapeau","lock picking kit","signaling mirror"}},
    { BETTER_ITEM, {"perfectly balance swords", "brace of pistols", "metal cuiraiss", "silk shirt", "fine telescope", "spanish rutter",
        "precision barometer", "concertina","theatrical disguise", "Incan mystic salve", "carved shaman stick", "sacred relic",
        "diamond necklace", "dancing slippers", "ostrich feather hat", "skeleton key","signal flare"}},
    { LANDMARK, {"", "city/settlement", "shipwreck", "" ,"", "", "", "", "" ,"", "", "", "city/settlement","", "", "",
        "Landmark", "X", "cool temple (unruined)", "inca temple", "abandoned cabin", "abandoned cabin", "geyser",
        "dead tree(s)", "arch rock", "palms", "stone head", "totem pole"}},
};

// Translations that require special effort or which are called to store data.
unordered_map <translatable, string (*)(const PstLine & i)> translation_functions = {
    { SHIPNAME, translate_shipname },
    { SHIP_TYPE, save_last_shiptype },  // Is this really that much better than a switch/case statement?
    { STORE_CITYNAME, store_cityname }, // Turns out it is. Having lots of little functions
    { FLAG, store_flag },               // makes it possible to compose them.
    { DATE, translate_date },
    { FOLLOWING, translate_following },
    { SHIP_SPECIALIST, translate_ship_specialist },
    { CITY_BY_LINECODE, translate_city_by_linecode},
    { SOLDIERS, translate_soldiers},
    { POPULATION, translate_population},
    { WEALTH, translate_wealth},
    { LUXURIES_AND_SPICES, translate_luxuries_and_spices},
    { BEAUTY_AND_SHIPWRIGHT, translate_beauty_and_shipwright},
    { POPULATION_TYPE, translate_population_type},
    { ACRES, translate_acres},
    { EVENT, translate_event},
    { FURTHER_EVENT, translate_event},
    { PEACE_AND_WAR, translate_peace_and_war},
    { DATE_AND_AGE, translate_date_and_age},
    { TREASURE_MAP, translate_treasure_map},
    { PIRATE_HANGOUT, translate_pirate_hangout},
};



string translate_soldiers(const PstLine & i) {
    if (i.v > 0) { return to_string(i.v*20);}
    else { return "None";}
}
string translate_acres(const PstLine & i) {
    return to_string(50*i.v) + " acres";
    
}
string translate_luxuries_and_spices(const PstLine & i) {
    int as_int = i.v/2;
    if (as_int > 49 || as_int < 0) { as_int = 49;}   // TODO: This may be mimicing a bug in the perl
    if (as_int == 0) { return ""; }
    return to_string(as_int);
}
string translate_pirate_hangout(const PstLine & i) {
    if (i.v == -1) { return "N/A"; }
    return simple_translate(CITYNAME, i.v);
}
string translate_population(const PstLine & i) {
    return to_string( 200 * (int)(unsigned char)i.v);
}

string translate_following(const PstLine & i) {
    if (i.v == -2) { return "following player";}
    else { return ""; }
}

string store_cityname (const PstLine & i) {
    // Save names of cities for later translations.
    int index = index_from_linecode(i.line_code);
    translation_lists[CITYNAME][index] = i.value;
    return "";
}


string simple_translate (translatable t, int as_int) {
    if (translation_lists.count(t)) {
        // Simple translation from a list.
        vector<string> & list = translation_lists.at(t);
        if (as_int >= 0 && as_int < list.size()) {
            if (list.at(as_int).size() > 0) {
                return list.at(as_int);
            } else { return ""; }
        } else if (as_int==-1) {
            switch (t) {
                    // Matching the perl code, some arrays have a special response if the value is -1.
                    // Perhaps I need a separate array for values at -1, or add one before accessing all arrays.
                case SHIP_TYPE    : return "NIL";
                case SPECIAL_MOVE : return "NONE";
                    // case CITYNAME     : return "N/A";
                default           : return "";
            }
        }
    }
    return "";
}

string translate(translatable t, const PstLine & i) {
    if (t == NIL) { return ""; }
    
    string return_value = "";
    
    if (translation_functions.count(t)) {
        // Special translations that require their own functions,
        // or which store this data for future translations.
        return_value = translation_functions.at(t)(i);
    }
    
    if (translation_lists.count(t)) {
        return_value = simple_translate(t, i.v);
    }
    
    // Placeholders are allowed, so a translatable that doesn't show up in either map is OK.
    
    // Note that if a translatable has both a function and a simple list, then we call the function
    // first and then return the value from the list. This is to cover the common case where the function
    // is for storing the value somewhere.
    
    if (return_value != "") { return_value = " (" + return_value + ")"; }
    return return_value;
}



string store_flag(const PstLine & i){
    string myflag = simple_translate(FLAG, i.v);
    myflag = regex_replace(myflag, regex("[\\(\\)]"),"");
    save_last_flag(myflag);
    return "";
}

vector<int> stored_city_wealth (128);
string translate_wealth(const PstLine & i) {
    int index = index_from_linecode(i.line_code);
    stored_city_wealth[index] = i.v;
    if (i.v != 300 ) {
        return simple_translate(WEALTH_CLASS, i.v/40 );
    } else { return ""; }
}

string translate_population_type (const PstLine & i) {
    // Cities are classified as having different sorts of population as a combination
    // of the Economy CityInfo_x_0_4 and the wealth City_x_5
    
    int magic_number = (int)translation_lists[POPULATION_CLASS].size() /2; // == 3
    
    int index = index_from_linecode(i.line_code);
    if (index >= number_of_true_cities) { return ""; } // Settlements do not get this
    
    int pop_group = i.v/67;
    if (pop_group >= magic_number || pop_group < 0) {
        pop_group = magic_number-1;
    } // Probably mimicing perl incorrect handling of negative numbers here.
    
    int is_wealthy = (stored_city_wealth[index] > 100) ? 1:0;
    
    return simple_translate(POPULATION_CLASS, (int)pop_group + magic_number*is_wealthy);
}

string translate_event_flags(const PstLine & i) {
    std::bitset<8> asbits(i.v);
    string retval = "";
    for (int j=0; j<6; j++) {  // Main nations reported only, plus Pirates and Indians?
        if (asbits[j]) {
            retval += " and " + simple_translate(FLAG, j);
        }
    }
    retval = regex_replace(retval, regex("^ and "), "");
    return retval;
}

int stored_event;
int subevent;
int temp_i;
string translate_event(const PstLine & i) {
    auto as_two = make_pair(i.v/16, i.v%16);
    int index = suffix_from_linecode(i.line_code);
    switch (index) {
        case 0:
            stored_event = i.v;
            subevent = 0;
            return "";
        case 1:
            if (i.v < 0) return "Greatly pleased " + translate_event_flags(i);
            if (i.v > 0) return "Pleased " + translate_event_flags(i);
            return "";
        case 2:
            if (i.v > 0) return "Offended " + translate_event_flags(i);
            return "";
        case 4:
            switch(stored_event) {
                case 16 : case 17 : case 18 : case 19 :
                    return simple_translate(FLAG,as_two.first) + " and " + simple_translate(FLAG,as_two.second);
                case 48:
                    return "pirate name";
                case 33:
                    return simple_translate(SHIP_TYPE, i.v);
                case 35: case 15: case 13: case 64:
                case 5: case 40: case 68: case 10:
                case 47: case 41: case 6 : case 3 : case 9:
                    return simple_translate(CITYNAME, i.v);
                case 69: case 39:
                    return (i.v < 0) ? "" : simple_translate(PURPOSE, i.v);
                case 36:
                    return "to " + simple_translate(FLAG,as_two.first)  + " " + simple_translate(RANK,as_two.second);
                case 38:
                    return simple_translate(SPECIALIST, i.v);
                case 37:
                    subevent = i.v; // Need to store this data if this is an upgrade.
                    return simple_translate(ITEM, i.v);
                case 44:
                    return "belonging to " + simple_translate(PIRATE, i.v);
                case 66:
                    return simple_translate(FLAG,i.v);
                default : ;
            }// end of _4
            break;
        case 5:
            if (stored_event == 33 || stored_event == 21) { return simple_translate(FLAG, i.v); }
            break;
        case 8 :
            switch(stored_event) {
                case 33:
                    return "Near " + simple_translate(CITYNAME, i.v);
                case 36:
                    return (i.v > 0) ? "Governor encouraged plundering of " + translate_event_flags(i) : "";
                case 37:
                    if (i.v == 1) {
                        return "Upgrade to " + simple_translate(BETTER_ITEM, subevent);
                    } else { return ""; }
                case 39: case 21: case 69:
                    return simple_translate(CITYNAME, i.v);
                case 44:
                    return "worth " + i.value + " gold";
                case 47:
                    if (subevent > 0) { return "--"; }
                    if (i.v > 0) {
                        return "installed " + simple_translate(FLAG, i.v) + " governor";
                    } else { return  "declined to install governor"; }
                case 66:
                    return "headed for " + simple_translate(CITYNAME, i.v);
                case 68:
                    // Is this a bug in the ordering of the evil characters in the perl script?
                    temp_i = i.v;
                    if (temp_i == 40 || temp_i == 41) { temp_i = 81 - temp_i; }
                    return simple_translate(PURPOSE, temp_i);
                default: ;
            }
            break;
        case 9:
            if (stored_event == 33) {
                if (i.v >= 0) return simple_translate(PURPOSE, i.v);
                return "";
            }
            break;
        default : ;
    }
    if (i.v != 0 && (i.v != -1 || i.method==INT)) { return "???"; }  // Log item not yet translated. Often -1.
    return "";
}

string translate_city_by_linecode (const PstLine & i) { // In this case, we aren't translating the value,
    int index = index_from_linecode(i.line_code);           // but rather noting which cityname goes with this line_code.
    return simple_translate(CITYNAME, index);
}

string translate_peace_and_war (const PstLine & i) {
    auto nation1 = index_from_linecode(i.line_code) - 16;
    if (nation1 < 0 || nation1 > 5) { return "";}
    auto nation2 = suffix_from_linecode(i.line_code) + 1;
    if (nation2 < 0 || nation2 > 5) { return "";}
    string nations = simple_translate(FLAG, nation1) + " and " + simple_translate(FLAG, nation2);
    if (i.v ==  1) { return nations + " at war"; }
    if (i.v == -1) { return nations + " at peace"; }
    return "";
}

string translate_date_and_age(const PstLine & i) {
    string date = translate_date(i);
    int age = stoi(regex_replace(date, regex(".* "), "")) - starting_year + 18;
    return "Approx Date: " + translate_date(i) + "; Age: " + to_string(age);
}
string translate_treasure_map(const PstLine & i) {
    if (i.v==0 || i.v == -1) { return ""; }
    auto index = suffix_from_linecode(i.line_code);
    string retval = "feature";
    if (index==0 || index == 16) { retval = "";}
    if (index == 1 || index == 17) { retval = "Key Landmark";}
    if (index & 16) { retval += " y coord "; } else { retval += " x coord "; }
    return retval;
}
string translate_ship_specialist (const PstLine & i) {
    // If a specialist is on board, then Ship_x_5_5 will be set to 10
    // and which specialist it is depends on the ship number.
    if (i.v != 10) { return ""; }
    int index = index_from_linecode(i.line_code);
    return simple_translate(SPECIALIST, index%8) + " on board";
}

string translate_beauty_and_shipwright(const PstLine & i) {
    int city_index = index_from_linecode(i.line_code);
    int city_value = (i.v+city_index)%8;
    string retval = "Shipwright can provide " + simple_translate(LONG_UPGRADES, city_value);
    if (city_index < number_of_true_cities) { // Only main cities also have a governor's daughter.
        retval = "Daughter is " + simple_translate(BEAUTY, i.v) + ", and " + retval;
    }
    return retval;
}

// These are the list of comments and translations - explaining what a line refers to.
// Comments depend on the line_code, not the value there; translations depend on both.
// This map does not have to be in the order that the lines appear in the savegame file, but it makes things easier to maintain if it is.

struct decode_for_line {
    std::string comment = "";
    translatable t ;
};

unordered_map<string,decode_for_line> line_decode = {
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
    // {"Personal_52_0",  {"cook/quartermaster/navigator/surgeon/gunner/cooper/sailmaker/carpenter"}}, // Done automatically below.
    //    {"Ship_0_0_0", {"Player Flagship Type", SHIP_TYPE }},   // TODO: Put this back in later. Perl code has a bug that makes this not work.
    {"Ship_x_0_0", {"Ship Type",       SHIP_TYPE }},
    {"Ship_x_0_1", {"Disposition",     DISPOSITION }},
    {"Ship_x_0_2", {"Flag",            FLAG   }},
    {"Ship_x_0_4", {"Target Ship"}},
    {"Ship_x_0_6", {"x Coordinate"}},
    {"Ship_x_1_0", {"y Coordinate"}},
    {"Ship_x_1_1", {"direction",       DIR16  }},
    {"Ship_x_1_2", {"speed"}},
    {"Ship_x_2_0", {"% Damage Sails"}},
    {"Ship_x_2_1", {"% Damage Hull"}},
    {"Ship_x_2_2", {"",                 FOLLOWING  }},
    {"Ship_x_2_3", {"Crew aboard"}},
    {"Ship_x_2_4", {"Cannon aboard"}},
    // {"Ship_x_2_6_0", {"upgrades bronze/powder/grape/chain/scantlings/hammocks/sails/copper"}}, // Done automatically below
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
    {"Ship_x_5_5", {"",                    SHIP_SPECIALIST }},
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
    {"CityInfo_x_0_8",   {"Land Grant", ACRES}},
    {"CityInfo_x_0_1",   {"", BEAUTY_AND_SHIPWRIGHT}},
    {"CityInfo_x_0_2_0", {"Hearts"}},
    {"CityInfo_x_0_4",   {"Economy (0-200)", POPULATION_TYPE}},
    {"CityInfo_x_0_5",   {"Mayor Delivered"}},
    {"CityInfo_x_1_0",   {"3x Merchant's gold on hand", }},
    {"CityInfo_x_1_2",   {"Merchant's Food on hand",}},
    {"CityInfo_x_1_3",   {"Merchant's Luxuries on hand", LUXURIES_AND_SPICES}},
    {"CityInfo_x_1_4",   {"Merchant's Goods on hand"}},
    {"CityInfo_x_1_5",   {"Merchant's Spice on hand",    LUXURIES_AND_SPICES}},
    {"CityInfo_x_1_6",   {"Merchant's Sugar on hand"}},
    {"CityInfo_x_1_10",  {"World Map price of Food"}},
    {"CityInfo_x_1_11",  {"World Map price of Luxuries"}},
    {"CityInfo_x_1_12",  {"World Map price of Goods"}},
    {"CityInfo_x_1_13",  {"World Map price of Spice"}},
    {"CityInfo_x_1_14",  {"World Map price of Sugar"}},
    {"CityInfo_x_1_15",  {"World Map price of Cannon"}},
    {"CityInfo_x_1_16",  {"Recruitable Crew Baseline", CITY_BY_LINECODE}},
    {"CityInfo_x_3_11",  {"ship launch direction 0-7", DIR8}},
    {"CityInfo_x_4",     {"", CITY_BY_LINECODE}},
    {"Log_x_0",          {"", EVENT}},
    {"Log_x_x",          {"", FURTHER_EVENT}},
    {"Log_x_10",         {"DateStamp", DATE}},
    {"Log_x_11",         {"x coordinate", NONE}},
    {"Log_x_12",         {"y coordinate", NONE}},
    // e_1_0 might be a bitstring. 16 for apprentice
    {"e_x_x",             {"", PEACE_AND_WAR}},
    {"e_23_7",            {"Date", DATE_AND_AGE}},
    {"Quest_x_0",         {"30 for wanted criminal, 10 for escort"}},
    {"Quest_x_1",         {"City", CITYNAME}},
    {"Quest_x_2",         {"Ship number of pirate"}},
    {"Quest_x_3",         {"Ship number to be escorted"}},
    {"Quest_x_4",         {"DateStamp", DATE}},
    {"Quest_x_5",         {"Purpose", PURPOSE}},
    {"LogCount_0",        {"Next Ship's Log Entry"}},
    {"TreasureMap_x_68_1",{"treasure map missing  SW/SE/NE/NW"}}, // this could be more clear.
    {"TreasureMap_x_68_3",{"treasure already found"}},
    {"TreasureMap_x_69",  {"", PIRATE}},
    {"TreasureMap_x_x",   {"", TREASURE_MAP}},
    {"TreasureMap_0_0",   {"Pirate Treasure", NIL}},
    {"TreasureMap_1_0",   {"Lost Relative"}},
    {"TreasureMap_2_0",   {"Lost City"}},
    {"TreasureMap_3_0",   {"Montalban's Hideout"}},
    {"TreasureMap_0_16",  {"Pirate Treasure", NIL}},
    {"TreasureMap_1_16",  {"Lost Relative"}},
    {"TreasureMap_2_16",  {"Lost City"}},
    {"TreasureMap_3_16",  {"Montalban's Hideout"}},
    {"Villain_x_0",       {"Flag 06 = pirate, 40 = Raymondo, 42 = Montalban, 30 = wanted criminal"}},
    {"Villain_x_1",       {"Ship number"}},
    {"Villain_x_2",       {"Pirate hangout",   PIRATE_HANGOUT}},
    {"Villain_x_3",       {"criminal letter"}},
    {"Villain_x_4_0",     {"visibility 0/active/ship/special_move/ship/ship_upgrades/ship/active"}},
    {"Villain_x_8",       {"Icon 1-9 named pirates, 30-33 criminals, 41=Raymondo"}},
    {"Villain_x_10",      {"DateStamp",   DATE}},
    {"Villain_x_5",       {"Special Move",   SPECIAL_MOVE}},
    {"Villain_x_15",      {"Starting City",   CITYNAME}},
    {"Villain_x_16",      {"Destination City",   CITYNAME}},
    {"t_7_3",             {"Starting Year"}},
    {"CityLoc_x_0",       {"x coord", CITY_BY_LINECODE}},
    {"CityLoc_x_1",       {"y coord",}},
    {"CityLoc_x_2",       {"pier direction", DIR16}},
    {"CityLoc_x_3",       {"fort direction", DIR16}},
    {"CoastMap_x_x",      {"", LANDMARK}},
    {"SailingMap_x_x",    {"", LANDMARK}},
    {"FeatureMap_x_x",    {"", LANDMARK}},
    {"Top10_x_0",         {"Gold Plundered"}},
    {"Top10_x_1_0",       {"status ?/D/D/?/?/D/D/treasure_found  (D bits went to 1 on capture)"}},
    {"Top10_x_11",        {"Unique Items"}},
    {"Top10_x_10",        {"Treasures Found"}},
    {"Top10_x_9",         {"Promotions Earned"}},
    {"Top10_x_8",         {"Towns Ransacked"}},
    {"Top10_x_7",         {"Ships Captured"}},
    {"LandingParty_0_0",  {"x coordinate"}},
    {"LandingParty_1_0",  {"y coordinate"}},
    {"LandingParty_3_0",  {"Direction", DIR16}},
    {"Skill_0",           { "", SKILL}},
    
};


void augment_from_translation_list(translatable t, string line_code, string prefix = "", string delimiter = "/") {
    // Upgrade and Specialist lists are accessed in the translation_lists because they can appear in decodes.
    // BUT they also appear in a binary decode where I need to see all of them in a line in reverse order.
    // To keep the comment in sync, autogenerate it from the list.
    string sp = "";
    for (auto up : translation_lists.at(t)) {
        sp = delimiter + up + sp;
    }
    sp = regex_replace(sp,regex("^" + delimiter), prefix);
    line_decode[line_code] = {sp};
}

void augment_cross_translation_list(translatable to, translatable from, int offset) {
    // Some translation lists would have too many holes, so build it up one part at a time.
    for (int i=0; i<translation_lists.at(from).size(); i++) {
        translation_lists[to][i+offset] = translation_lists.at(from)[i];
    }
}

void augment_decoder_groups() {
    // Some decoder groups have "some assembly required".
    //
    // The items comments need to mention both items controlled by each line.
    for (int i=0; i<translation_lists[ITEM].size(); i++) {
        int p = 47 + (i/4);
        int pp = i%4;
        string line = "Personal_" + to_string(p) + "_" + to_string(pp);
        string comment = "1=" + translation_lists[ITEM][i] + ",    2=" + translation_lists[BETTER_ITEM][i];
        stringstream ss;
        ss << std::left << std::setw(25) << "1=" + translation_lists[ITEM][i] + ", " << "2=" << translation_lists[BETTER_ITEM][i];
        line_decode[line] = {ss.str()};
    }
    
    // These have lists in the comment where I also need the components of the lists separately.
    augment_from_translation_list(SPECIALIST, "Personal_52_0");
    augment_from_translation_list(SHORT_UPGRADES, "Ship_x_2_6_0", "upgrades ");
    
    // This translation_list has too many blanks, so split it into different lists.
    augment_cross_translation_list(EVENT, EVENTS3, 3);
    augment_cross_translation_list(EVENT, EVENTS15, 15);
    augment_cross_translation_list(EVENT, EVENTS32, 32);
    augment_cross_translation_list(EVENT, EVENTS64, 64);
    
    augment_cross_translation_list(PURPOSE, PURPOSE0,  0);
    augment_cross_translation_list(PURPOSE, PURPOSE30, 30);
    augment_cross_translation_list(PURPOSE, PURPOSE40, 40);
}

string translate_date(const PstLine & i) { // Translate the datestamp into a date in game time.
    if (i.v == -1) { return ""; }
    double stamp = (unsigned int)i.v;   // Leaving it negative would be more correct, but I'm matching perl here.
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
    string retval = regex_replace(st.str(), regex("  "), " ");
    
    st.str("");   // Clear the stream.
    st << std::put_time(std::gmtime(& myt), "%Y\n");
    string syear = st.str();
    int year = stoi(syear) - 1970 + starting_year;   // Changing the epoch
    retval += to_string(year);
    
    return retval;
}

string PstLine::get_translation() {
    // Created lca to eliminate regex operations. We need to check Ship_1_2_3, Ship_x_2_3, and Ship_x_x_3
    for (string lc : lca) {
        if (line_decode.count(lc) && line_decode.at(lc).t != NIL) {
            return translate(line_decode.at(lc).t, *this);
        }
    }
    return "";
}

string PstLine::get_comment() {
    for (string lc : lca) {
        if (line_decode.count(lc) && line_decode.at(lc).comment.length() > 0) {
            return line_decode.at(lc).comment;
        }
    }
    return "";
}


void check_for_specials(std::ifstream &in, std::ofstream &out, string line_code) {
    // The savegame file has variable length parts at the beginning and end,
    // and a huge fixed length section in the middle. Once we hit the start of the fixed length section,
    // it makes sense to peek far ahead to read the starting year, so that it can be used in all of the datestamps.
    if (line_code == "Personal") {
        constexpr int jump_dist = 887276;
        in.seekg(jump_dist, ios_base::cur);
        starting_year = read_int(in);
        in.seekg(-jump_dist-4, ios_base::cur);
    }
    // The perl code had an extra comment just before this section.
    if (line_code == "Log") {
        out << "# Ship's Log\n";
    }
}

void print_field (std::ofstream &out, string value, int default_width) {
    // Prints a field with appropriate spacing to keep the colons lined up for similar lines with different width values.
    int lw = (int)value.length();
    int width = default_width * ((lw/default_width)+1);
    out << std::left << setw(width) << value << " : " ;
}

void PstLine::write_text(std::ofstream &out) {
    
    string typecode = mcode();
    string comment =  get_comment();
    string translation = get_translation();
    
    // Perl script reports 4-byte integers as unsigned.
    // This is misleading, they act more like signed, so I am holding them
    // as signed internally, but printing unsigned to match.
    if (method==INT && v < 0) {
        value = to_string((unsigned int)v);
    }
    
    if (method==FEATURE) {
        // Spacing is different but simpler for F1 Feature case.
        out << line_code << "  : " << typecode << " : " << value << " :";
        if (comment=="" && translation=="") { out << " "; }
        out << comment << translation << "\n";
    } else {
        // Regular spacing method uses the print_field function to line up spaces.
        print_field(out, line_code, 8);
        print_field(out, typecode,  3);
        
        int value_width = 1;
        if (method == TEXT) { value_width = 20; }
        else if (bytes<=4) {
            if (method==INT || method==BULK || method==SHORT || method==CHAR || method==LCHAR) {
                value_width = 9;
            }
        }
        print_field(out, value, value_width);
        
        out << comment << translation << "\n";
    }
}

string PstLine::mcode() {
    return char_for_method.left.at(method) + to_string(bytes);
}
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


void PstLine::read_binary_world_map(ifstream &in, boost::ptr_deque<PstLine> & features) {
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
            features.push_back( new PstLine{line_code + "_" + to_string(i), FEATURE, b[i],  ss.str(), lca.back() + "_x"});
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
    v = -2;    // Prevent MAP lines from being translated, even though FEATURE lines are.
}

void PstLine::expand_map_value() {
    // This is the reverse of read_binary_world_map
    // Take the compressed map (where one bit indicates sea or land)
    // and expand it to a string that looks like BULK: 2 chars per byte of binary.
        
    const string sea  = "00";
    const string land = method==CMAP ? "09" : "ff";
    stringstream ss;
    stringstream bs;
    
    int compbytes = bytes/4;
    if (bytes % 4) compbytes++;
    
    for (int b=0; b<compbytes; b++) {
        char hexchar = method==SMAP ? '0' : value[b];                               // Read one character (or assume 0 for SMAP)
        int intval = (hexchar >= 'a') ? (hexchar - 'a' + 10) : (hexchar - '0');     // convert to decimal.
        std::bitset<4> asbits(intval);                                              // Convert to binary
        for (int j=0; j<4; j++) {
            if (asbits[3-j]) {                        // Now for each bit of the binary, write a byte
                ss << land;
            } else {
                ss << sea;
            }
        }
    }
    // Replace the compressed value in the PstLine with this expanded value.
    value = ss.str();
    //
}

void PstLine::read_binary(std::ifstream &in, boost::ptr_deque<PstLine> & features) {
    if (is_world_map(method)) {
        this->read_binary_world_map(in, features);
        line_code += "_293";
    } else {
        this->read_binary(in);
    }
}

constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void PstLine::read_binary(std::ifstream &in) {
    char b[2000] = "";
    stringstream ss;
    int size_of_string;
    switch (method) {
        case TEXT : // Reads the string length, then the string
            size_of_string = read_int(in);
            if (size_of_string > sizeof(b)-2) throw logic_error("expected tring too long");
            in.read((char *)& b, size_of_string);
            value = b;
            if (bytes == 8) {
                if (read_int(in) != 0) {} //throw logic_error("Unexpected non-zero after text8");
                if (read_int(in) != 0) {} //throw logic_error("Unexpected non-zero after text8");
            }
            break;
        case BULK :
            in.read((char*)&b, bytes);
            // Old method, somewhat slow:
            // ss << std::noshowbase << std::hex << nouppercase << setfill('0');
            // for (int i=0;i<bytes;i++) {
            //    ss << setw(2) << (int)(unsigned char)b[i];
            // }
            value = string(bytes * 2, ' ');
            for (int i = 0; i < bytes; ++i) {
                value[2 * i]     = hexmap[(b[i] & 0xF0) >> 4];
                value[2 * i + 1] = hexmap[b[i] & 0x0F];
            }
           
            break;
        case ZERO :
            in.read((char*)&b, bytes);
            for (int i=0;i<bytes;i++) {
                if (b[i] != 0) throw logic_error("Non-zero found in expected zero-string");
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
                    ss << std::right << std::fixed << setprecision(6) << setw(10) << double((unsigned int)v)/1'000'000;
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
                case CHAR:
                    v = (int)(unsigned char)v;
                default:
                    ss << to_string(v);
            }
            value = ss.str();
            break;
        default: ;
    }
}

void PstLine::update_map_value(int column, std::string feature_value) {
    if (! is_world_map(method))  throw logic_error("Cannot update_map_value on a non-map rmeth!");
    if (value.length() < 293*2)  throw logic_error("Cannot update_map_value before expanding map value");
    
    value.replace(column*2,2,feature_value);
}

void PstLine::write_binary(std::ofstream & out) {
    
    // For the numeric types, first convert to an unsigned int with a length.
    // This is also used by TEXT for the length-of-string int, and for HEX (which converts back to an int)
    unsigned int data = 0;
    int bytes_to_write = bytes;

    switch (method) {
        case FEATURE: // FEATURE does not write directly, it is used to edit the map lines.
            return;
        case HEX:   // For HEX, the byte order is reversed from the text. Also there are periods to skip.
            for (auto b=0; b<bytes; b++) {
                out << (unsigned char)stoi(value.substr(3*(bytes-b-1),2), nullptr, 16);
            }
            return;
        case BULK : // BULK doesn't go through vcopy. Just read the hex 2 characters at a time to write one byte.
        case SMAP : // The three worldmaps have been expanded so they look like BULK.
        case CMAP :
        case FMAP :
            for (auto b=0; b<bytes; b++) {
                out << (unsigned char)stoi(value.substr(2*b,2), nullptr, 16);
            }
            return;
        case TEXT:
            data = (unsigned int)value.length();
            bytes_to_write = 4;
            break;
        case INT:
            data = (unsigned int)stoul(value);
            break;
        case SHORT:
        case CHAR:
        case LCHAR:
            data = (unsigned int)(stoi(value));
            break;
        case uFLOAT:
             data = (unsigned int)(stod(value) * 1000000+.5);
            break;
        case mFLOAT:
            data = (unsigned int)(stod(value) * 1000+.5);
            break;
        case BINARY:
            data = (unsigned int)stoul(value,nullptr,2);
            break;
        case ZERO:
        default:
            data = 0;
    }
    
    // Now send the numeric data out in binary form.
    for (auto b=0;b<bytes_to_write;b++) {
        out << (unsigned char)(data % 256);
        data = data >> 8;
    }
      
    // For TEXT, we have only sent the length so far, so now send the actual text, followed by padding if necessary.
    if (method == TEXT) {
        out << value;
        for (auto b=0; b<bytes; b++) {
            out << (char)0;
        }
    }
}
