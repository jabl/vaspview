/*VASP Data Viewer - Views 3d data sets of molecular charge distribution
  Copyright (C) 1999-2001 Timothy B. Terriberry
  (mailto:tterribe@users.sourceforge.net)
  2011 Janne Blomqvist

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

#if !defined(_ds3slice_H)
# define _ds3slice_H (1)

#include "ds3.hh"
#include "ds3view.hh"

struct DS3Slice {
    GLuint         t_id;       /*ID of the slice texture*/
};

void ds3SliceInit(DS3Slice *_this,size_t _dens[3]);
void ds3SliceDstr(DS3Slice *_this,DS3View *_view);

extern const GLWCallbacks DS3_VIEW_SLICE_CALLBACKS;

#endif                                                           /*_ds3slice_H*/
