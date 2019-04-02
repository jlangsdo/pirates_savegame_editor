//
//  LineDecoding.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/2/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef LineDecoding_hpp
#define LineDecoding_hpp

#include <stdio.h>
#include <map>
#include <vector>
#include <string>

enum translatable {
    // All translatable enums should be mapped in the translation_lists
    NIL, RANK, DIFFICULTY, NATION, FLAG, SKILL, SPECIAL_MOVE, SHIP_TYPE,
    DISPOSITION, BEAUTY, SHORT_UPGRADES, LONG_UPGRADES, CITYNAME, DIRECTION,
    SHORT_DIRECTION,
    EVENT, EVENTS3, EVENTS15, EVENTS32, EVENTS64,
    WEALTH_CLASS, POPULATION_CLASS, SOLDIERS, FLAG_TYPE, SPECIALIST,
    // or in the translation_functions
    DIR, SHIPNAME, STORE_CITYNAME, DATE, FOLLOWING, CITY_BY_LINECODE, WEALTH, POPULATION,
    POPULATION_TYPE, ACRES, LUXURIES_AND_SPICES, BEAUTY_AND_SHIPWRIGHT, FURTHER_EVENT, SHIP_SPECIALIST,
    // If it is not mapped in either, that is not an error: it is hook for future code).
};

struct decode_for_line {
    std::string comment = "";
    translatable t = NIL;
};

// Master routines

std::string full_translate(std::string subsection, std::string subsection_x, std::string value) ;
std::string full_comment(std::string subsection, std::string subsection_x, std::string value) ;

std::string translate(translatable t, std::string value, std::string line_code);
std::string simple_translate (translatable t, int as_int);
std::string simple_translate (translatable t, std::string value);

// Stubs for routines called by translation_functions
std::string translate_soldiers(std::string value, std::string line_code) ;
std::string translate_acres(std::string value, std::string line_code) ;
std::string translate_luxuries_and_spices(std::string value, std::string line_code) ;
std::string translate_population(std::string value, std::string line_code) ;
std::string translate_following(std::string value, std::string line_code) ;
std::string store_cityname (std::string value, std::string line_code) ;
std::string store_flag(std::string value, std::string line_code);
std::string translate_wealth(std::string value, std::string line_code) ;
std::string translate_population_type (std::string value, std::string line_code) ;
std::string translate_dir (std::string value, std::string line_code) ;
std::string translate_event_flags(std::string value, std::string line_code) ;
std::string translate_event(std::string value, std::string line_code) ;
std::string translate_city_by_linecode (std::string value, std::string line_code) ;
std::string translate_ship_specialist (std::string value, std::string line_code) ;
std::string translate_beauty_and_shipwright(std::string value, std::string line_code) ;
std::string translate_date(std::string value, std::string section) ;


// Utilities?
int read_hex (char c);
int index_from_linecode (std::string line_code);
std::string str_tolower(std::string s);

#endif /* LineDecoding_hpp */
