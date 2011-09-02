/*C++2011 RAII wrapper around C stdio FILE
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

#include "file.hh"
#include <cstring>

File::File(const char* name, const char* mode)
{
    f = std::fopen(name, mode);
}

File::~File()
{
    if (f)
        fclose(f);
}

// fgets style function that fills a string up to the next newline or
// EOF. Returns true if something was read, false otherwise.
bool File::fgets(std::string& line)
{
    char buffer[1024];
    line.clear();
    do {
        if (!std::fgets(buffer, sizeof(buffer), this->f))
            return !line.empty();

        line.append(buffer);
    } while (!std::strchr(buffer, '\n'));
    return true;
}
