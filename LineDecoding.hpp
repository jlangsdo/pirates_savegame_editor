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

// Public routines
void check_for_specials(std::ifstream &in, std::ofstream &out, std::string line_code);
void augment_decoder_groups();

// Internal routines.
std::string full_translate(const PstLine &) ;
std::string full_comment(const PstLine &) ;

std::string translate(translatable t, const PstLine &);
std::string simple_translate (translatable t, int as_int);
std::string simple_translate (translatable t, std::string value);

// Utilities?
int index_from_linecode (std::string);
int suffix_from_linecode (std::string);

// If this were a class, I would mark these private:
// Stubs for routines called by translation_functions
std::string translate_soldiers(const PstLine &) ;
std::string translate_acres(const PstLine &) ;
std::string translate_luxuries_and_spices(const PstLine &) ;
std::string translate_population(const PstLine &) ;
std::string translate_following(const PstLine &) ;
std::string store_cityname (const PstLine &) ;
std::string store_flag(const PstLine &);
std::string translate_wealth(const PstLine &) ;
std::string translate_population_type (const PstLine &) ;
std::string translate_event_flags(const PstLine &) ;
std::string translate_event(const PstLine &) ;
std::string translate_city_by_linecode (const PstLine &) ;
std::string translate_ship_specialist (const PstLine &) ;
std::string translate_beauty_and_shipwright(const PstLine &) ;
std::string translate_date(const PstLine &) ;
std::string translate_date_and_age(const PstLine &) ;
std::string translate_peace_and_war(const PstLine &) ;
std::string translate_treasure_map(const PstLine &) ;
std::string translate_pirate_hangout(const PstLine &);

#endif /* LineDecoding_hpp */
