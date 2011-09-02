/*Some C++ string utilities
  Copyright (C) 2011  Janne Blomqvist

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*/

#include "strutil.hh"
#include <cctype>

// Remove trailing spaces (as determined by std::isspace)
std::string& trim(std::string& str)
{
    size_t found = str.length();
    for (std::string::reverse_iterator rit = str.rbegin();
            rit != str.rend(); ++rit) {
        if (std::isspace(*rit))
            found--;
        else
            break;
    }
    str.erase(found);
    return str;
}

// Left adjust a string
std::string& adjustl(std::string& str)
{
    size_t found = 0;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (std::isspace(*it))
            found++;
        else
            break;
    }
    str.erase(0, found);
    return str;
}

// Trim a string at both ends
std::string& trimlr(std::string& str)
{
    return trim(adjustl(str));
}
