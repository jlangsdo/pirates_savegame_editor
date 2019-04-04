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
#include "Pirates.hpp"

enum translatable : char;

struct decode_for_line {
    std::string comment = "";
    translatable t ;
};

struct info_for_line_decode {
    std::string value;
    int v;                // value reduced to an integer
    std::string line_code;
};
// Master routines
void print_pst_line (std::ofstream &out, std::string , info_for_line_decode);
void check_for_specials(std::ifstream &in, std::string line_code);


std::string full_translate(info_for_line_decode) ;
std::string full_comment(info_for_line_decode) ;

std::string translate(translatable t, info_for_line_decode);
std::string simple_translate (translatable t, int as_int);
std::string simple_translate (translatable t, std::string value);

// Utilities?
int read_hex (char c);
int index_from_linecode (std::string);
int suffix_from_linecode (std::string);
std::string str_tolower(std::string);

// If this were a class, I would mark these private:
// Stubs for routines called by translation_functions
std::string translate_soldiers(info_for_line_decode) ;
std::string translate_acres(info_for_line_decode) ;
std::string translate_luxuries_and_spices(info_for_line_decode) ;
std::string translate_population(info_for_line_decode) ;
std::string translate_following(info_for_line_decode) ;
std::string store_cityname (info_for_line_decode) ;
std::string store_flag(info_for_line_decode);
std::string translate_wealth(info_for_line_decode) ;
std::string translate_population_type (info_for_line_decode) ;
std::string translate_dir (info_for_line_decode) ;
std::string translate_event_flags(info_for_line_decode) ;
std::string translate_event(info_for_line_decode) ;
std::string translate_city_by_linecode (info_for_line_decode) ;
std::string translate_ship_specialist (info_for_line_decode) ;
std::string translate_beauty_and_shipwright(info_for_line_decode) ;
std::string translate_date(info_for_line_decode) ;
std::string translate_date_and_age(info_for_line_decode) ;
std::string translate_peace_and_war(info_for_line_decode) ;
std::string translate_treasure_map(info_for_line_decode) ;
std::string translate_pirate_hangout(info_for_line_decode);

#endif /* LineDecoding_hpp */
