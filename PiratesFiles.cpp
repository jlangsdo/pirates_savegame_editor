//
//  PiratesFiles.cpp
//  
//
//  Created by Langsdorf on 4/10/19.
//
// This module has the file handling routines so the calls in main() can be as straightforward as possible.


#include "PiratesFiles.hpp"
#include "PstFile.hpp"
#include "ship_names.hpp"
#include "PstLine.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include "boost/filesystem.hpp"

// Filename suffixes
const std::string pg_suffix   = "pirates_savegame";
const std::string pst_suffix  = "pst";
const std::string test_suffix = pg_suffix + ".test";

extern std::string save_dir;

using namespace std;

void print_help() {
    
    const string help_message = R"USAGE(
    
    Basic switches:
    -dir <>         Directory to find pirates_savegame files
    -unpack <>      pirates_savegame file(s) to be unpacked to a pst
    -pack <>        pst file(s) to pack into pirates_savegame
    -advanced_help  More information, including splicing switches
    
    The pst file is a text file which contains all of the information
    required to rewrite the original pirates savegame.
    
    After running -unpack on a pirates_savegame file,
    you can edit the pst file in any text editor,
    then run -pack to rebuild the pirates_savegame file.
    Leave off the file extension.
    
    The advanced switches give you ways to splice parts of one
    pst file into another without a text editor.
    
    )USAGE";
    cout << help_message;
    exit(0);
}

vector<std::string> split_by_commas(std::string arg) {
    vector<std::string> results;
    string temp;
    std::istringstream tokenStream(arg);
    while(std::getline(tokenStream, temp, ',')) {
        results.push_back(temp);
    }
    return results;
}

void print_advanced_help() {
    const string advanced_help_message = R"AUSAGE(
    
    The advanced switches have not been implemented yet.
    
    
    )AUSAGE";
    cout << advanced_help_message;
    exit(0);
    
}

void set_up_decoding() {
    // Initialization of some variables that is done at runtime, mainly for unpack().
    set_up_rmeth();
    augment_decoder_groups();
    load_pirate_shipnames();
}

void comparePg(std::string afile) {
    string file1 = find_file(afile, pg_suffix);
    ifstream fs1 = ifstream(file1, ios::binary);   // Looks like opend()
    if (! fs1.is_open()) {
        cerr << "Failed to read from " << file1 << "\n";
        exit(1);
    }
    
    string file2 = find_file(afile, test_suffix);
    ifstream fs2 = ifstream(file2, ios::binary);
    if (! fs2.is_open()) {
        cerr << "Failed to read from " << file2 << "\n";
        exit(1);
    }
    cout << "Comparing " << file1 << " to " << file2 << "\n";
    compare_binary_filestreams(fs1,fs2); // Aborts on failure.
    fs1.close();
    fs2.close();
    cout << "PASS!\n";
    std::remove(file2.c_str());
}

string find_file(string game, string suffix) {
    // Remove suffix.
    game = regex_replace(game, regex("\\.[^\\/]+$"), "");
    
    // Look for the game with the right suffix. It should either have
    // come with a full path, or be in the save_dir.
    const string possible_files[] = {
        game + "." + suffix,
        save_dir + "/" + game + "." + suffix,
    };
    for (auto afile : possible_files) {
        ifstream f(afile.c_str());
        if (f.good()) {
            return afile;
        }
    }
    cout << "Could not find file " << game << "\n";
    cout << "Looked for: \n";
    for (auto afile : possible_files) {
        cout << "  '" << afile << "'\n";
    }
    exit(1);
}

void unpack(std::string afile) {
    string pg_file = find_file(afile, pg_suffix);
    ifstream pg_in = ifstream(pg_file, ios::binary);
    if (! pg_in.is_open()) {
        cerr << "Failed to read from " << pg_file << "\n";
        exit(1);
    }
    cout << "Unpacking " << pg_file << "\n";
    
    string pst_file = regex_replace(pg_file, regex(pg_suffix + "$"), pst_suffix);
    ofstream pst_out = ofstream(pst_file);
    if (! pst_out.is_open()) {
        cerr << "Failed to write to " << pst_file << "\n";
        pg_in.close();
        exit(1);
    }
    cout << "Writing " << pst_file << "\n\n";
    
    unpackPst(pg_in, pst_out);
    pg_in.close();
    pst_out.close();
}

void testpack(string afile) {  pack(afile, test_suffix); }
void pack(string afile)     {  pack(afile, pg_suffix); }
void pack(string afile, string out_suffix) {
    PstFile myPst(afile, pst_suffix);
    string pg_file = regex_replace(myPst.filename, regex(pst_suffix + "$"), out_suffix);
    
    ofstream pg_out = ofstream(pg_file);
    if (! pg_out.is_open()) throw runtime_error("Failed to write_to " + pg_file);
    cout << "Writing " << pg_file << "\n\n";
    
    myPst.write_pg(pg_out);
}

std::vector<std::string> find_pg_files() {
    using namespace boost::filesystem;
    using namespace boost;
    vector<std::string> results;
    path dir_path(save_dir);
    if (is_directory(dir_path)) {
        for (directory_entry& x : directory_iterator(dir_path)) {
            string afile = x.path().string();
            if (regex_search(afile, regex("." + pg_suffix + "$"))) {
                results.push_back(afile);
            }
        }
    }
    return results;
}

void splice(std::string infile, std::string donor, std::string outfiles,
            std::string splices, std::string clone, std::string set, bool do_auto,
            std::string notfiles, std::string suffix) {
    PstFile inPst(infile);
 
    enum splice_source { UNSET, DONOR, SET, CLONE };
    splice_source source = UNSET;
    
    if (donor.length() > 0 ) { source = DONOR; }
    PstFile donorPst(donor); // This will be blank if there is no donor.
    
    if (set.length() > 0) {
        if (source != UNSET) { throw invalid_argument("Do not use -donor and -set together"); }
        source = SET;
    }
    auto all_sets = split_by_commas(set);
    auto all_sets_count = all_sets.size();
    
    PstFile clonePst();
    map<std::string, vector<regex> > clone_by_section;
    map<std::string, vector<unique_ptr<PstLine> > > clone_PstLines;
    auto all_clones = split_by_commas(clone);
    if (clone.length() > 0) {
        if (source != UNSET) { throw invalid_argument("Do not use -clone with -donor or -set"); }
        source = CLONE;
        
        // Populate the clonePst();
        // This requires the same code as the main splice loop below.... Hmmm.
        
        for (size_t i=0; i<all_clones.size(); i++) {
            auto aclone = all_clones[i];
            
            // Take the splice string and cut it into a section and a linecode.
            auto first_underscore = aclone.find('_');
            string section_name = aclone.substr(0,first_underscore);
            // The line_code may be blank.
            string line_code = first_underscore == string::npos ? "" : aclone.substr(first_underscore, string::npos);
            // The clone line_code may not contain _x as a wildcard.
            // line_code = regex_replace(line_code, regex("_x"), "_\\d+");
            // Processing the -splice string values into regular expressions to be checked against
            // every line_code within a section.
            clone_by_section[section_name].emplace_back(regex(line_code));
            clone_by_section[section_name].emplace_back(regex(line_code + "_.*"));
        }
        
    
        for (auto section : section_vector) {
            if (clone_by_section.count(section.name)) {
                for (auto && line_pair : inPst.data[section.name]) {
                    for (auto clone_line : clone_by_section[section.name]) {
                        if (regex_match(line_pair.second->line_code, clone_line)) {
                            clone_PstLines[section.name].emplace_back(std::make_unique<PstLine>(*inPst.data[section.name][line_pair.first]));
                        }
                    }
                }
            }
        }
    }
    
   //  if (source == UNSET) throw invalid_argument("-splice requires exactly one of -donor, -set, or -clone as a source");
    
        
    auto all_splices = split_by_commas(splices);
    auto all_outfiles = split_by_commas(outfiles);
    auto outfile_count = all_outfiles.size();
    
    for (auto oi=0; oi<outfile_count; ++oi) {
        auto afile = all_outfiles[oi];
        string outfile = save_dir + "/" + afile + "." + suffix;
        
        // If there is just one outfile, apply all splices to it.
        // If there are multiple outfiles, parcel out the splices, but everyone gets at least one.
        map<std::string, vector<regex> > splice_by_section;
        if (all_splices.size()) { // If there are no splices, then this is just a pack.
            for (auto i=oi % all_splices.size(); i<all_splices.size(); i += outfile_count) {
                auto asplice = all_splices[i];
                
                // Take the splice string and cut it into a section and a linecode.
                auto first_underscore = asplice.find('_');
                string section_name = asplice.substr(0,first_underscore);
                // The line_code may be blank.
                string line_code = first_underscore == string::npos ? "" : asplice.substr(first_underscore, string::npos);
                // The line_code may contain _x as a wildcard.
                line_code = regex_replace(line_code, regex("_x"), "_\\d+");
                // Processing the -splice string values into regular expressions to be checked against
                // every line_code within a section.
                splice_by_section[section_name].emplace_back(regex(line_code));
                splice_by_section[section_name].emplace_back(regex(line_code + "_.*"));
            }
        }
        size_t set_count = oi;
        PstFile outPst;
        for (auto section : section_vector) {
            for (auto && line_pair : inPst.data[section.name]) {
                bool did_splice_this_line = false;
                if (splice_by_section.count(section.name)) {
                    for (auto splice_line : splice_by_section[section.name]) {
                        if (regex_match(line_pair.second->line_code, splice_line)) {
                            switch (source) {
                                case DONOR:
                                    // Donor is required to have an exactly matching line_code. Take the line from the donor.
                                    outPst.data[section.name].emplace(line_pair.first, std::make_unique<PstLine>(*donorPst.data[section.name][line_pair.first]));
                                    did_splice_this_line = true;
                                    break;
                                case SET:
                                    // Set uses the line_code from inPst, but changes the value.
                                    outPst.data[section.name].emplace(line_pair.first, std::make_unique<PstLine>(*inPst.data[section.name][line_pair.first]));
                                    outPst.data[section.name][line_pair.first]->value = all_sets[set_count];
                                    set_count = (set_count + outfile_count) % all_sets.size();
                                    did_splice_this_line = true;
                                    break;
                                case CLONE:
                                    // For a clone, clonePst has been assembled and we expect that it will have just the right line to fit.
                                    outPst.data[section.name].emplace(line_pair.first, std::make_unique<PstLine>(*clone_PstLines[section.name][set_count]));
                                    //Making sure it fits here.
                                    if (outPst.data[section.name][line_pair.first]->bytes != inPst.data[section.name][line_pair.first]->bytes ||
                                        outPst.data[section.name][line_pair.first]->method != inPst.data[section.name][line_pair.first]->method )
                                        throw invalid_argument("Problem with splice from " + section.name + outPst.data[section.name][line_pair.first]->line_code +
                                                               " into " + section.name + inPst.data[section.name][line_pair.first]->line_code);
                                    set_count = (set_count + outfile_count) % clone_PstLines[section.name].size();
                                    did_splice_this_line = true;
                                    break;
                                default: ;
                                    // Case UNSET : nothing to do here.
                            }
                            break;
                        }
                    }
                }
                if (! did_splice_this_line ) {
                    outPst.data[section.name].emplace(line_pair.first, std::make_unique<PstLine>(*line_pair.second));
                }
            }
        }
    
        ofstream pg_out(outfile);
        if (! pg_out.is_open()) throw runtime_error("Failed to write_to " + outfile);
        cout << "Writing " << outfile << "\n\n";
        outPst.write_pg(pg_out);
        unpack(outfile);
    }
}
