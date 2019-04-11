//
//  PiratesFiles.hpp
//  
//
//  Created by Langsdorf on 4/10/19.
//

#ifndef PiratesFiles_hpp
#define PiratesFiles_hpp

#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>

// These are the routines called in main();
void print_help();
void print_advanced_help();
void set_up_decoding();
void unpack(std::string afile);
void pack(std::string afile);
void testpack(std::string afile);
std::vector<std::string> find_pg_files();
std::vector<std::string> split_by_commas(std::string);
void comparePg(std::string afile);
void splice(std::string infile, std::string donor, std::string outfiles,
            std::string splice, std::string clone, std::string set, bool do_auto, std::string notfiles);


// These are used internally.
void pack(std::string afile, std::string suffix);
std::string find_file(std::string game, std::string suffix);

void compare_binary_files(std::string file1, std::string file2);
void test_file_to_test(std::string save_dir, std::string file_to_test);
std::string find_file(std::string dir, std::string file, std::string suffix);

#endif /* PiratesFiles_hpp */
