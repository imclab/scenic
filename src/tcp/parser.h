/* 
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/** \file 
 *      
 *      Command parser functions used in ipcp protocol
 *      
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <map>
#include "gutil/strIntFloat.h"

/// TODO place in namespace?

/// returns a string with " and \ escaped
std::string strEsq(const std::string& str);

/// returns a string with chars " and \ with escape removed 
std::string strUnEsq(const std::string& str);

/// returns the position of the trailing quote in a string
/// ignores escaped version
int get_end_of_quoted_string(const std::string& str);

/// builds a string of command: key=value pairs
/// TODO make map const
bool stringify(std::map<std::string,StrIntFloat>& cmd_map, std::string& str);

/// fills a map of key=value pairs with a special key "command" from a string
/// of type specified above
/// TODO handle extraneous white space
bool tokenize(const std::string& str, std::map<std::string,StrIntFloat>& cmd_map);

#endif

