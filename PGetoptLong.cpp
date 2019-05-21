//  PGetoptLong.cpp
//
//  Created by Langsdorf on 5/20/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//
// Adapted from Perl Getopt::Long by Johan Vromans
// The goal is to have the terse interface of the Perl Getopt::Long code for parsing switches to c++.

#include "PGetoptLong.hpp"
#include <regex>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <set>

using namespace std;

const set<string> allowed_endings = { "", "!", "!!", "=s", "=f", "=i", ":s", ":f", ":i" };

std::map<std::string, std::string> PGetOptions(int argc, char**argv, std::vector<std::string> switches) {
    
    map<std::string, std::string> opt;  // Output
    map <string, string> checker;       // Internal for checking
    map <string, string> alias;         // Internal for aliases

    // Process the switches to fill the local maps for checker and alias.
    for (auto kv2 : switches) {
        
        string kv = regex_replace(kv2, regex("^(--?|\\+)"), ""); // It is legal to define your switches with - in front, the way they are called.
        
        // Each switch starts with | alternated alternate names,
        // optionally followed by = or : followed by a type char,
        // or with !.
        // The first name is the "real" name
        // of the switch, that will be used in the response.
        const auto stopat = kv.find_first_of("|=:!");
        const string firstkey = kv.substr(0,stopat);
        
        // If there is a : or =, save that and what comes after it into
        // the checker.
        const auto end_index = kv.find_first_of("=:!");
        const string ending = (end_index == string::npos) ? "" : kv.substr(end_index);
        if (not allowed_endings.count(ending)) { throw logic_error("Invalid ending for " + kv); }
        if (ending == "!!") {  throw logic_error("Invalid ending for " + kv); } // !! is only legal if created internally.
        if (checker.count(firstkey)) { throw logic_error("Cannot define an option twice, as was done for -" + kv); }
        checker[firstkey] = ending;
        
        // If there were pipe alternated names, create aliases from each alternative
        // to the real name (firstkey).
        const string keys   = (end_index == string::npos) ? kv : kv.substr(0,end_index);
        stringstream keystream(keys);
        string akey;
        while (std::getline(keystream, akey, '|')) {
            alias[akey] = firstkey;
            // cout << firstkey << " <= " << akey << "\n";
            if (ending == "!") {
                alias["no" + akey] = "no" + firstkey;
                checker["no" + firstkey] = "!!";        // So "nofoo" is now a flag, but it gets a special checker.
                alias[akey] = firstkey;
                checker[firstkey] = "!";
            }
        }
    }
    // Adding abbreviations.
    // Step 1. For each switch, for all shortenings of the switch, add the target to the abbreviations map.
    map<string, set<string>> abbreviations;
    for (auto kv : alias) {
        const string fullswitch = kv.first;
        const string target = kv.second;
        
        for (int i = (int)fullswitch.length()-1; i>0; i--) {
            abbreviations[ fullswitch.substr(0,i) ].insert(target);
        }
    }
    // Step 2: For each potential abbreviation, if only one target was referenced, add it to the actual aliases.
    for (auto ks : abbreviations) {
        if (ks.second.size() == 1) {
            for (auto target : ks.second) {
                alias[ks.first] = target;
            }
        }
    }
    
    
    // Now process the argv list.
    
    // State machine:
    string find_value_for;
    bool value_required = false;
    bool value_optional = false;
    bool all_remaining_bypass = false;
    vector<string> unparsed_argv;
    
    for (int i = 1; i< argc; i++) { // Skips argv[0] as that is the script name.
        if (all_remaining_bypass) {
            unparsed_argv.push_back(argv[i]);
        } else {
            if (value_required) {
                opt[find_value_for] = argv[i];
                value_optional = false;
                value_required = false;
            } else { // i.e. If value is not required, then this argument may be parsed as a switch.
                const string thisarg = argv[i];
                if (thisarg == "--") {
                    all_remaining_bypass = true;
                    continue;
                }
                const string akey = regex_replace(thisarg, regex("^(--?|\\+)"), "");
                
                if (alias.count(akey) != 0) {
                    // Successfully parsed argument to be a switch.
                    if (opt.count(alias[akey])) {
                        throw std::invalid_argument("Cannot use the same flag twice for -" + alias[akey]);
                    }
                    if (checker[alias[akey]] == "" || checker[alias[akey]] == "!!" || checker[alias[akey]] == "!") {
                        // FLAG style switch.
                        value_optional = false;
                        value_required = false;
                        opt[alias[akey]] = "1";
                    } else {
                        // Non-flag style switch, need to look for an optional/required argument at the next position.
                        value_optional = true;
                        value_required = checker[alias[akey]].front() == '=';
                        find_value_for = alias[akey];
                        if (! value_required) { // If optional, put in a placeholder value for now.
                            if (checker[find_value_for] == ":s") {
                                opt[find_value_for] = "";
                            } else {
                                opt[find_value_for] = "0";
                            }
                        }
                    }
                } else {
                    // This argument is not a recognized switch after all. Either it is an optional value,
                    // or we leave it unparsed.
                    if (value_optional) {
                        opt[find_value_for] = argv[i];
                        value_optional = false;
                        value_required = false;
                    } else {
                        unparsed_argv.push_back(argv[i]);
                    }
                }
            }
        }
    }
    if (value_required) {
        throw std::invalid_argument("ERROR, still looking for a required value to go with switch -" + find_value_for);
    }
    
    // Run the checkers to make sure the values picked up seem valid.
    set<string> remove;
    for (const auto & [sw, value] : opt ) { 
        if (checker[sw] == "") {
            // Blank checker is a basic flag item.
            if (value != "1") { throw logic_error("Odd value for flag -" + sw); }
        } else if (checker[sw] == "!!") {
            // If we hit on a bang-bang, that means there was a -nofoo switch, so we should delete this and set the value for foo to 0.
            if (sw.substr(0,2) != "no") {
                throw logic_error("ARGH, confusion about !! usage with " + sw);
            } else {
                string truesw = sw.substr(2);
                if (opt.count(truesw)) { throw std::invalid_argument("Cannot use the same flag twice for -" + sw + " and -" + truesw); }
                opt[truesw] = "0";
                remove.insert(sw);
            }
        } else if (checker[sw] == "!") {
            // Bang checker can be set to 1 or 0.
            if ((value != "1") && (value != "0")) {
                throw logic_error("Odd value for flag -" + sw);
            }
        } else if (checker[sw] == ":s" || checker[sw] == "=s") {
            // It's a string, so always legal.
        } else if (checker[sw] == ":i" || checker[sw] == "=i") {
            if (! regex_match(value,regex("[-+]?[0-9][0-9]*"))) {
                throw std::invalid_argument("Bad non integer value " + value + " for -" + sw);
            }
        } else if (checker[sw] == ":f" || checker[sw] == "=f") {
            if (! regex_match(value, regex("[-+]?([0-9.])[0-9]*(.[0-9]+)?([eE][-+]?[0-9]+)?"))) {
                throw std::invalid_argument("Bad non floating point value " + value + " for -" + sw);
            }
        } else {
            // This was meant to be an exhaustive check on all allowed_endings
            throw logic_error("Missed case in ending checker parsing");
        }
    }
    // Remove any !! switches that were noticed during checking.
    for (auto sw : remove) {
        opt.erase(sw);
    }
    
    // Join any unparsed args into a single string to return with --.
    for (auto i = 0; i< unparsed_argv.size(); ++i) {
        opt["--"] += (i ? " " : "") + unparsed_argv[i];
    }
    
    return opt;
}
