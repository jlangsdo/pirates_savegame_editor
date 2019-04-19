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
    
    For manual splicing:
    -in <file> -out <files> -splice <regex>
    plus one of these: -set <value>, -clone <regex>, or -donor <file>
    
    The splicing operation loads in a pst file, changes some of the lines
    according to the switches, writes out an edited pirates_savegame file,
    then unpacks that file.
    
    The -splice target is a line_code, like Log_4_3.
    If you leave off part of the line_code, it will add all extensions,
    so -splice Ship_0 will splice all of the lines that start with Ship_0_.
    You can also use _x to as a wildcard number. T
    hese switches can take multiple comma separated arguments,
    so you can splice multiple groups of lines at a time.
    
    -clone takes the same style of regex as -splice.
        Use this to clone one Ship to another, or one CityInfo to another.
    -set lets you just set values. Be careful to make sure you put in a legal value.
    -donor says to look in the donor file for the identical line_codes there.
    
    If there are multiple comma separated -out files, then it splits up the splices
    to parcel them out to the different output files.
    It also splits up the -clone, -set, and -donor appropriately.
    
    For automatic splicing:
    -auto -in <file> -out <files> -donor <files> [-not <files>]
    
    This is designed to help you find which line_code corresponds to a feature
    which appears in the donor file(s) but not in the -in file
    (and also not in any of the -not files, if applicable).
    It finds all line_codes which could correspond to such a feature.
    If there is one output file, all candidate lines go there.
    If there are more output files than candidate lines,
        it puts all of the splices into the first output file,
        and then one into each of the others.
    If there are more splice candidates, then it sets up a binary search,
        by putting half of the splices into each output file,
        in such a way that testing each output file will have minimal redundancy.
    After checking which of those files have the feature,
    they can be fed back in using -donor and -not for another round
    to quickly narrow down the target line_code.
    
    For regression:
    -test <files>
        
    Unpacks the file, then repacks it, and then compares the resulting
    pirates_savegame file to the original. Detects cases where information has
    been lost in the unpack/pack combination.
        
    -sweep
        
    Runs -test on all available files in the directory given.
        
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
    string short_file1 = regex_replace(file1, regex(".*\\/"), "");
    string short_file2 = regex_replace(file2, regex(".*\\/"), "");
    cout << "Comparing " << short_file1 << " to " << short_file2 << "\n";
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

void unpack(std::string afile, std::string extra_text) {
    string pg_file = find_file(afile, pg_suffix);
    ifstream pg_in = ifstream(pg_file, ios::binary);
    if (! pg_in.is_open()) {
        cerr << "Failed to read from " << pg_file << "\n";
        exit(1);
    }
    string short_file = regex_replace(pg_file, regex(".*\\/"), "");
    cout << "Translated " << short_file;
    string pst_file = regex_replace(pg_file, regex(pg_suffix + "$"), pst_suffix);
    ofstream pst_out = ofstream(pst_file);
    if (! pst_out.is_open()) {
        cerr << "Failed to write to " << pst_file << "\n";
        pg_in.close();
        exit(1);
    }
    
    unpackPst(pg_in, pst_out); // BUG: Doesn't check that the file is eof
    pst_out << extra_text;
    pg_in.close();
    pst_out.close();
    
    short_file = regex_replace(pst_file, regex(".*\\/"), "");
    cout << " -> " << short_file << "\n";
}

void testpack(string afile) {  pack(afile, test_suffix); }
void pack(string afile)     {  pack(afile, pg_suffix); }
void pack(string afile, string out_suffix) {
    PstFile myPst(afile, pst_suffix);
    string pg_file = regex_replace(myPst.filename, regex(pst_suffix + "$"), out_suffix);
    string short_file = regex_replace(pg_file, regex(".*\\/"), "");
    ofstream pg_out = ofstream(pg_file);
    if (! pg_out.is_open()) throw runtime_error("Failed to write_to " + pg_file);
    cout << "Writing " << short_file << "\n";
    
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

static map<std::string, vector<std::regex> > regex_from_arg(std::vector<std::basic_string<char>, std::allocator<std::basic_string<char> > > &all_splices, int oi, unsigned long outfile_count) {
    
    // This extracted routine takes a list of items as could go to -splice or -clone
    // and returns a list of regex that matches. If the outfile_count is not 1, then it parcels them out
    map<std::string, vector<regex> > splice_by_section;
    if (all_splices.size() > 0) {
        for (auto i=oi % all_splices.size(); i<all_splices.size(); i += outfile_count) {
            auto asplice = all_splices[i];
            
            // Take the splice string and cut it into a section and a linecode.
            auto first_underscore = asplice.find('_');
            string section_name = asplice.substr(0,first_underscore);
            // The line_code may be blank.
            string line_code = first_underscore == string::npos ? "" : asplice.substr(first_underscore, string::npos);
            // The line_code may contain _x as a wildcard.
            line_code = regex_replace(line_code, regex("_x"), "_\\d+");
            // Processing the -splice string values into two regular expressions to be checked against
            // every line_code within a section.
            splice_by_section[section_name].emplace_back(regex(line_code));
            splice_by_section[section_name].emplace_back(regex(line_code + "_.*"));
        }
    }
    return splice_by_section;
}


void splice(std::string infile, std::string donor, std::string outfiles,
            std::string splices, std::string clone, std::string set, bool do_auto,
            std::string notfiles, std::string suffix) {
    
    PstFile inPst(infile);
    auto all_outfiles = split_by_commas(outfiles);
    auto outfile_count = all_outfiles.size();
    auto all_splices = split_by_commas(splices);
    
    // Set up source for replacement lines. Only one of these three will be active at a time (guaranteed by switch testing in main)
    PstFile donorPst(donor);
    auto all_clones  = split_by_commas(clone);
    auto all_sets = split_by_commas(set);
    
    for (auto oi=0; oi<outfile_count; ++oi) {
        auto afile = all_outfiles[oi];
        string outfile = save_dir + "/" + afile + "." + suffix;
        
        // If there is just one outfile, apply all splices to it.
        // If there are multiple outfiles, parcel out the splices, but everyone gets at least one.
        auto splice_by_section = regex_from_arg(all_splices, oi, outfile_count);
        
        // clone can be comma-separated, so parcel out the clone regex if it exists too.
        // Doing this here because it is once per file, not once per section.
        auto clone_by_section = regex_from_arg(all_clones, oi, outfile_count);
        
        // Scan through the file for lines that match a section splice regex. Leave those lines out
        // (noting their sortcodes) but duplicate the rest to outPst.
        PstFile outPst;
        for (auto section : section_vector) {
            vector<Sortcode> lines_to_splice;
            for (auto && [sortcode, aPstLine] : inPst[section]) {
                bool did_splice_this_line = false;
                if (splice_by_section.count(section.name)) {
                    for (auto splice_line : splice_by_section[section.name]) {
                        if (regex_match(aPstLine->line_code, splice_line)) {
                            lines_to_splice.push_back(sortcode);
                            did_splice_this_line = true;
                            break;
                        }
                    }
                }
                if (! did_splice_this_line ) {
                    outPst[section].emplace(sortcode, std::make_unique<PstLine>(*aPstLine));
                }
            }
            
            // Now, add in replacement lines from the desired source:
            if (splice_by_section.count(section.name)) {
                if (donor != "") {
                    // parse the donorPst and add in any lines that match the splice.
                    for (auto && [sortcode, aPstLine] : donorPst[section]) {
                        for (auto splice_line : splice_by_section[section.name]) {
                            if (regex_match(aPstLine->line_code, splice_line)) {
                                outPst[section].emplace(sortcode, std::make_unique<PstLine>(*aPstLine));
                                break;
                            }
                        }
                    }
                } else if (clone != ""){
                    
                    PstFile clonePst;  // This PstFile is just one section's worth of cloned lines.
                    for (auto && [sortcode, aPstLine] : inPst[section]) {
                        for (auto clone_line : clone_by_section[section.name]) {
                            if (regex_match(aPstLine->line_code, clone_line)) {
                                clonePst[section].emplace(sortcode, std::make_unique<PstLine>(*aPstLine));
                            }
                        }
                    }
                    
                    auto clone_iterator = clonePst[section].begin();
                    for (auto sortcode : lines_to_splice) {
                        outPst[section].emplace(sortcode, std::make_unique<PstLine>(*clone_iterator->second));
                        
                        //Make sure the replacement line is exactly of the same form as the line it replaced.
                        //This prevents splicing an INT in place of a SHORT.
                        if (outPst[section][sortcode]->bytes != inPst[section][sortcode]->bytes ||
                            outPst[section][sortcode]->method != inPst[section][sortcode]->method )
                            throw invalid_argument("Problem with splice from " + section.name + outPst[section][sortcode]->line_code +
                                                   " into " + section.name + inPst[section][sortcode]->line_code);
                        ++clone_iterator;
                        if (clone_iterator == clonePst[section].end()) {
                            clone_iterator = clonePst[section].begin();
                        }
                    }
                } else if (set != "") {
                    // For set, we add the original lines in that were skipped, then change their value.
                    // There is no type-checking on the value because that seems hard.
                    size_t set_count = oi;
                    for (auto sortcode : lines_to_splice) {
                        outPst[section].emplace(sortcode, std::make_unique<PstLine>(*inPst[section][sortcode]));
                        outPst[section][sortcode]->value = all_sets[set_count];
                        set_count = (set_count + outfile_count) % all_sets.size();
                    }
                }
            }
        }
        ofstream pg_out(outfile);
        if (! pg_out.is_open()) throw runtime_error("Failed to write_to " + outfile);
        string short_file = regex_replace(outfile, regex(".*\\/"), "");
        cout << "Writing " << short_file << "\n";
        outPst.write_pg(pg_out);
        unpack(outfile);
    }
}


void auto_splice(std::string infile, std::string donorfiles, std::string outfiles, std::string notfiles) {
    
    PstFile inPst(infile);
    auto all_outfiles = split_by_commas(outfiles);
    auto outfile_count = all_outfiles.size();
    
    auto all_donorfiles = split_by_commas(donorfiles);
    vector <std::unique_ptr<PstFile> > donorPst;
    for (auto afile : all_donorfiles) { donorPst.emplace_back(make_unique<PstFile>(afile)); }
    auto & oneDonor = donorPst.back();
    //donorPst.pop_back();
    
    auto all_notfiles = split_by_commas(notfiles);
    vector <std::unique_ptr<PstFile> > notPst;
    for (auto afile : all_notfiles) { notPst.emplace_back(make_unique<PstFile>(afile)); }
    
    // An auto-splice line comes from something observed in the donor files which is not in inPst or the notPst.
    //   - in the oneDonor, same value in other donors, different/missing in inPst and notPst
    //   - in inPst, not in any donors, same in notPst as in inPst - but for backward compatibility, these ARE NOT COUNTED.
    //
    map <std::string, set<Sortcode> >   splice_targets;
    vector<std::string> splice_lines;
    int splice_count = 0;
    for (auto section : section_vector) {
        for (auto && [sortcode, aPstLine] : (*oneDonor)[section]) {
            bool splice_it = true;
            auto value = aPstLine->value;
            
            if (inPst.matches(section,sortcode,value)) { splice_it = false; }
            for (auto && otherDonor : donorPst) {
                if (! (*otherDonor).matches(section,sortcode,value)) { splice_it = false; }
            }
            for (auto && otherNot : notPst) {
                if ((*otherNot).matches(section,sortcode,value)) { splice_it = false; }
                // backward compatibility: inPst must match the -not files for any line that will be spliced.
                if ((*otherNot)[section].count(sortcode) && inPst[section].count(sortcode)) {
                    string inVal = inPst[section][sortcode]->value;
                    if (! (*otherNot).matches(section,sortcode,inVal)) { splice_it = false; }
                }
            }
            if (splice_it) {
                splice_count++;
                splice_targets[section.name].insert(sortcode);
                splice_lines.emplace_back(section.name + aPstLine->line_code);
            }
        }
    }
    cout << "Total of " << splice_lines.size() << " candidate lines for auto-splicing\n";
    sort(splice_lines.begin(), splice_lines.end());
    
    // Now for the splice. We can splice just from the oneDonor (because all splice lines are the same
    // in all donors. Also, no need for regex, we have exact sortcodes / linecodes.
    for (auto oi=0; oi<outfile_count; ++oi) {
        auto afile = all_outfiles[oi];
        string outfile = save_dir + "/" + afile + "." + pg_suffix;
        
        enum auto_mode {ALL, ONE, HALF};
        auto_mode mode;
        if (outfile_count == 1) {
            // If we have only one outfile, then all splices go to that outfile.
            mode = ALL;
        } else if (outfile_count > splice_count) {
            // If we have very few candidate lines, put one in each outfile, but put them all in the first outfile.
            if (oi==0) mode = ALL;
            else mode = ONE;
            if (oi > splice_count) { break; }
        } else {
            // If we have many splice lines, split them up between the outfiles to enable a binary search.
            mode = HALF;
        }
        
        if (mode == ALL) {
            cout << "All lines test: " << afile << " <=";
            for (auto aline : splice_lines) { cout << aline << " "; }
            cout << "\n";
        }
        if (mode == ONE) {
            cout << "Single line test: " << afile << " <= " << splice_lines[oi-1] << "\n";
        }
        
        
        PstFile outPst;
        
        // Weird sort order inherited from perl version.
        int flip = pow(2, oi);
        bool include = true;
        int this_splice_count = 0;
        map<std::string, set <std::string > > splice_sub_lines;
        for (int i=0; i<splice_lines.size(); ++i) {
            if (mode==ALL || (mode==ONE && oi==i+1) || (mode==HALF && include) ) {
                size_t underscore = splice_lines[i].find("_",0);
                string asection = splice_lines[i].substr(0,underscore);
                string alinecode = splice_lines[i].substr(underscore, string::npos);
                splice_sub_lines[asection].emplace(alinecode);
                this_splice_count++;
            }
            if ((i+1) % flip == 0) { include = !include; }
        }
        cout << "auto-splicing " << this_splice_count << " lines\n";
        
        for (auto section : section_vector) {
            for (auto && [sortcode, aPstLine] : (inPst)[section]) {
                if (splice_sub_lines.count(section.name) &&
                    splice_sub_lines[section.name].count(aPstLine->line_code)) {
                    // All splices must exist in the donor (see above)
                    outPst[section].emplace(sortcode, std::make_unique<PstLine>(*(*oneDonor)[section][sortcode]));
                } else {
                    outPst[section].emplace(sortcode, std::make_unique<PstLine>(*inPst[section][sortcode]));
                }
            }
            // Also consider splice_targets that exist in oneDonor but not inPst.
            for (auto && [sortcode, aPstLine] : (*oneDonor)[section]) {
                if ( splice_targets.count(section.name) ) {
                    if (splice_sub_lines.count(section.name) &&
                        splice_sub_lines[section.name].count(aPstLine->line_code) &&
                        ! inPst[section].count(sortcode) ) {
                        outPst[section].emplace(sortcode, std::make_unique<PstLine>(*(*oneDonor)[section][sortcode]));
                    }
                }
            }
        }
        ofstream pg_out(outfile);
        if (! pg_out.is_open()) throw runtime_error("Failed to write_to " + outfile);
        string short_file = regex_replace(outfile, regex(".*\\/"), "");
        cout << "Writing " << short_file << "\n";
        outPst.write_pg(pg_out);
        unpack(outfile, "## Auto Spliced\n");
    }
}
