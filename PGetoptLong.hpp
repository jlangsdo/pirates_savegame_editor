//
//  PGetoptLong.hpp
//
//  Created by Langsdorf on 5/20/19.
//  Copyright © 2019 Langsdorf. All rights reserved.
//

#ifndef PGetoptLong_hpp
#define PGetoptLong_hpp
#include <map>

std::map<std::string, std::string> PGetOptions(int argc, char**argv, std::vector<std::string> switches);

/* PGetOptions parses command line options.
 
 Inputs: argc and argv (just as they are passed to main), and a vector of switches to look for.
 Output: map of found switches to their (string) values.
 
 Switches can be flags:
 "foo"   "-foo"   "--foo"  "+foo"  are all equivalent, and will return a value of "1"
 A flag switch can also end with a !, which allows you to call the switch with "no" in front to get the value set to "0"
"foo!" means that it will accept -foo and -nofoo, to get values for "foo" of "1" and "0" respectively.
 
 Switches can call for arguments.
 Arguments are of types i (int), f (float), or s(string)
 Arguments of types i or f are checked against a regex to see if they are probably OK, but are returned as strings.
 Arguments can be reqeuired (=) or optional (:)
 Optional arguments of types i or f default to "0", optional strings return "". The default is given if the argument was optional
 and the next argument in argv was parsed as a switch, or there was no next argument in argv.
 "size=i"   "scale:f"   "log:s"

 Switches can have alternate names, and non-conflicting abbreviations are automatically generated.
 The return key is the first of the alternatives, unabbreviated.
 "--pack=i"          will work with  -p 5, and will return {"pack", "5"}
 "try|do|work_on"    will work with -try, -do, or -work, and will return {"try", "1"}
 "pack=i", "park=i"  will not parse switch -pa.
 
 The special switch "--" says that all remaining arguments should not be parsed.
 All arguments which are not parsed are joined with spaces (in order) and returned as the value for the "--" switch.
 
 */

#endif /* PGetoptLong_hpp */
