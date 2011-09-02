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

#ifndef FILE_HH
#define FILE_HH

#include <cstdio>
#include <string>

class File {
public :
    File(const char* name, const char* mode);
    ~File();
    bool fgets(std::string& line);
    std::FILE* f;
private:
    File();
    File& operator=(const File&);
    File(const File&);
};

#endif
