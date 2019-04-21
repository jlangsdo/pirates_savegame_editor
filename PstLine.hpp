//
//  PstLine.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/2/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef PstLine_hpp
#define PstLine_hpp

#include <vector>
#include <string>
#include "RMeth.hpp"
#include "PstSection.hpp"
#include <array>

class PstLine {
public:
    std::string line_code;
    int v;                // value reduced to a small integer for lookups
    std::string value;
    rmeth method;
    int bytes = standard_rmeth_size[method];
    std::array<std::string, 3> lca;   // line_code_aliases
    
    PstLine(PstSection subsection) : line_code(subsection.name), method(subsection.splits.front().method), bytes(subsection.splits.front().bytes), lca(subsection.lca) {}
    PstLine(std::string lc, rmeth rm, int v,     std::string value, std::string al) : line_code(lc), method(rm), v(v),         value(value), lca{al} { lca[0] = al; }
    PstLine(std::string lc, rmeth rm, int bytes, std::string value)                 : line_code(lc), method(rm), bytes(bytes), value(value) {}
    PstLine(const PstLine & pl2) = default;
    void read_binary (std::ifstream &in, std::vector<PstLine> & features);
    void read_binary_world_map (std::ifstream &in, std::vector<PstLine> & features);
    void read_binary (std::ifstream &in);
    void write_text (std::ofstream &out);
    void write_binary (std::ofstream &out);
    void expand_map_value();
    void update_map_value(int column, std::string value);
    std::string get_comment();
    std::string get_translation();
};


int read_int(std::ifstream & in);
enum translatable : char;

// Public routines
void check_for_specials(std::ifstream &in, std::ofstream &out, std::string line_code);
void augment_decoder_groups();

// Utilities?
int index_from_linecode (std::string);
int suffix_from_linecode (std::string);

// Stubs for routines called by get_translation
std::string translate(translatable t, const PstLine &);
std::string simple_translate (translatable t, int as_int);
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

#endif /* PstLine_hpp */
