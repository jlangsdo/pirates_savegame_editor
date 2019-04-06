//
//  ReadMethod.hpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/6/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef ReadMethod_hpp
#define ReadMethod_hpp

#include <stdio.h>
#include "Pirates.hpp"
#include <iostream>
#include <vector>

class ReadMethod {
public:
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) = 0;
    virtual std::string typecode(int) = 0;
    virtual const int standard_size() { return 0; }
    virtual const bool is_world_map() { return false; }
private:
};

class rmBULK : public ReadMethod {
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    std::string typecode(int sz) override { return "H" + std::to_string(sz);}
    virtual bool check_against_zero() { return false; }
};
class rmZERO : public rmBULK {
    std::string typecode(int sz) override { return "x" + std::to_string(sz);}
    bool check_against_zero() override { return true; }
};

class rmMAP : public ReadMethod { // World Maps
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    const bool is_world_map() override { return true; }
    virtual const bool throw_away_string() { return false; }
    virtual const char land() { return -1; }
    virtual const char max_sea() { return 0; }
};

class rmSMAP : public rmMAP { // Specific examples of world maps:
    std::string typecode(int sz) { return "m" + std::to_string(sz);}
    const bool throw_away_string() { return true; }
};
class rmFMAP : public rmMAP {
    std::string typecode(int sz) { return "M" + std::to_string(sz);}
};
class rmCMAP : public rmMAP {
    std::string typecode(int sz) { return "MM" + std::to_string(sz);}
    const char max_sea() { return 4; }
    const char land() { return 9; }
};

class rmNUMBER : public ReadMethod {
public:
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual const bool write_hex() { return false; }
};

class rmINT : public rmNUMBER {
    virtual std::string typecode(int sz) { return "V4"; }
    virtual const int standard_size() { return 4; }
};
class rmmFLOAT : public rmINT {
    // an mFLOAT is printed divided by 1000.
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual std::string typecode(int sz) override { return "g4"; }
};
class rmuFLOAT : public rmINT {
    // a uFLOAT is printed divided by 1,000,000
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual std::string typecode(int sz) override { return "G4"; }
};
class rmHEX : public rmINT {
    // A rmHEX is an rmINT that returns its value in hex.
    virtual std::string typecode(int sz) override { return "h4"; }
    const bool write_hex() override { return true; }
};
class rmSHORT : public rmNUMBER {
    std::string typecode(int sz) { return "s2"; }
    const int standard_size() { return 2; }
};
class rmCHAR : public rmNUMBER {
    virtual std::string typecode(int sz) { return "C1"; }
    virtual const int standard_size() { return 1; }
};
class rmLCHAR : public rmCHAR {
    std::string typecode(int sz) { return "c1"; }
};
class rmBINARY : public rmCHAR {
    // A rmBINARY is a rmCHAR that returns its value as a string of 1s and 0s.
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    std::string typecode(int sz) override { return "B1"; }
};

class rmTEXT0 : public ReadMethod {
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual std::string typecode(int) override { return "t0"; }
    virtual const int padding() { return 0; }
};
class rmTEXT8 : public rmTEXT0 {
    // Text string followed by 8 bytes of 0.
    const int padding() override { return 8; }
    std::string typecode(int) override { return "t8"; }
};


#endif /* ReadMethod_hpp */
