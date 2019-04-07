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
#include <string>

class ReadMethod {
    public:
    std::string line_code;
    int count_of_lines;
    int bytes_per_line;
    std::string value = "";    // value read from file
    int v = 0 ;                // value reduced to an integer

    ReadMethod(std::string lc , int col, int bpl) : line_code(lc), count_of_lines(col), bytes_per_line(bpl) {}
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) {}
    virtual std::string typecode(int) { return ""; };
    virtual const int standard_size() { return 0; }
    virtual const bool is_world_map() { return false; }
private:
};

class rmBULK : public ReadMethod {
public:
    rmBULK(std::string lc , int col, int bpl) : ReadMethod(lc, col, bpl) {}
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    std::string typecode(int sz) override { return "H" + std::to_string(sz);}
    virtual bool check_against_zero() { return false; }
};
class rmZERO : public rmBULK {
public:
    rmZERO(std::string lc , int col, int bpl) : rmBULK(lc, col, bpl) {}
    std::string typecode(int sz) override { return "x" + std::to_string(sz);}
    bool check_against_zero() override { return true; }
};

class rmMAP : public ReadMethod { // World Maps
public:
    rmMAP(std::string lc , int col, int bpl) : ReadMethod(lc, col, bpl) {}
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    const bool is_world_map() override { return true; }
    virtual const bool throw_away_string() { return false; }
    virtual const char land() { return -1; }
    virtual const char max_sea() { return 0; }
};

class rmSMAP : public rmMAP { // Specific examples of world maps:
public:
    rmSMAP(std::string lc , int col, int bpl) : rmMAP(lc, col, bpl) {}
    std::string typecode(int sz) { return "m" + std::to_string(sz);}
    const bool throw_away_string() { return true; }
};
class rmFMAP : public rmMAP {
public:
    rmFMAP(std::string lc , int col, int bpl) : rmMAP(lc, col, bpl) {}
    std::string typecode(int sz) { return "M" + std::to_string(sz);}
};
class rmCMAP : public rmMAP {
public:
    rmCMAP(std::string lc , int col, int bpl) : rmMAP(lc, col, bpl) {}
    std::string typecode(int sz) { return "MM" + std::to_string(sz);}
    const char max_sea() { return 4; }
    const char land() { return 9; }
};

class rmNUMBER : public ReadMethod {
public:
    rmNUMBER(std::string lc , int col, int bpl) : ReadMethod(lc, col, bpl) {}
    rmNUMBER(std::string lc) : ReadMethod(lc, 1, 1) {}
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual const bool write_hex() { return false; }
};

class rmINT : public rmNUMBER {
public:
    rmINT(std::string lc , int col, int bpl) : rmNUMBER(lc, col, bpl) {}
    rmINT(std::string lc) : rmNUMBER(lc, 1, 1) {}
    virtual std::string typecode(int sz) { return "V4"; }
    virtual const int standard_size() { return 4; }
};
class rmmFLOAT : public rmINT {
public:
    rmmFLOAT(std::string lc , int col, int bpl) : rmINT(lc, col, bpl) {}
    // an mFLOAT is printed divided by 1000.
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual std::string typecode(int sz) override { return "g4"; }
};
class rmuFLOAT : public rmINT {
public:
    rmuFLOAT(std::string lc , int col, int bpl) : rmINT(lc, col, bpl) {}
    // a uFLOAT is printed divided by 1,000,000
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual std::string typecode(int sz) override { return "G4"; }
};
class rmHEX : public rmINT {
public:
    rmHEX(std::string lc , int col, int bpl) : rmINT(lc, col, bpl) {}
    rmHEX(std::string lc) : rmINT(lc, 1, 1) {}
    // A rmHEX is an rmINT that returns its value in hex.
    virtual std::string typecode(int sz) override { return "h4"; }
    const bool write_hex() override { return true; }
};
class rmSHORT : public rmNUMBER {
public:
    rmSHORT(std::string lc , int col, int bpl) : rmNUMBER(lc, col, bpl) {}
    std::string typecode(int sz) { return "s2"; }
    const int standard_size() { return 2; }
};
class rmCHAR : public rmNUMBER {
public:
    rmCHAR(std::string lc , int col, int bpl) : rmNUMBER(lc, col, bpl) {}
    virtual std::string typecode(int sz) { return "C1"; }
    virtual const int standard_size() { return 1; }
};
class rmLCHAR : public rmCHAR {
public:
    rmLCHAR(std::string lc , int col, int bpl) : rmCHAR(lc, col, bpl) {}
    std::string typecode(int sz) { return "c1"; }
};
class rmBINARY : public rmCHAR {
public:
    rmBINARY(std::string lc , int col, int bpl) : rmCHAR(lc, col, bpl) {}
    // A rmBINARY is a rmCHAR that returns its value as a string of 1s and 0s.
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    std::string typecode(int sz) override { return "B1"; }
};

class rmTEXT0 : public ReadMethod {
public:
    rmTEXT0(std::string lc , int col, int bpl) : ReadMethod(lc, col, bpl) {}
    rmTEXT0(std::string lc) : ReadMethod(lc, 1, 1) {}
    virtual void read(std::ifstream &, info_for_line_decode &, int, std::vector<info_for_line_decode> &) override;
    virtual std::string typecode(int) override { return "t0"; }
    virtual const int padding() { return 0; }
};
class rmTEXT8 : public rmTEXT0 {
public:
    rmTEXT8(std::string lc , int col, int bpl) : rmTEXT0(lc, col, bpl) {}
    // Text string followed by 8 bytes of 0.
    const int padding() override { return 8; }
    std::string typecode(int) override { return "t8"; }
};


#endif /* ReadMethod_hpp */
