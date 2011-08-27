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

// Remove trailing spaces (as determined by std::isspace)
std::string trim(std::string& str)
{
    size_t found;
    found = str.find_last_not_of(std::isspace);
    if (found != std::string::npos)
        str.erase(found+1);
    else
        str.clear();            // str is all whitespace

    return str;
}

// Left adjust a string
std::string adjustl(std::string& str)
{
	size_t found;
	found = str.find_first_not_of(std::isspace);
	if (found != std::string::npos)
		str.erase(0, found);
	else
		str.clear();

	return str;
}

// Trim a string at both ends
std::string trimlr(std::string& str)
{
	return trim(adjustl(str));
}
