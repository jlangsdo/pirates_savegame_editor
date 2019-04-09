//
//  Rmeth.cpp
//  pirates_savegame_editor
//
//  Created by Langsdorf on 4/9/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#include "RMeth.hpp"
#include <string>
#include <stdio.h>
#include <boost/bimap.hpp>
#include <boost/assign/list_inserter.hpp> // for 'insert()'
using namespace std;
using namespace boost::assign; // bring 'insert()' into scope


rmethstrings char_for_method;
void set_up_rmeth() {
    insert( char_for_method )
    (TEXT,"t") (INT, "V") (HEX, "h") (BINARY, "B") (SHORT, "s") (CHAR, "C") (FMAP, "M") (BULK, "H")
    (ZERO,"x") (uFLOAT,"G") (mFLOAT, "g")(LCHAR, "c") (SMAP, "m") (CMAP, "MM") (FEATURE, "F");
}
