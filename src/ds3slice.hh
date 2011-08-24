/*VASP Data Viewer - Views 3d data sets of molecular charge distribution
  Copyright (C) 1999-2001 Timothy B. Terriberry
  (mailto:tterribe@users.sourceforge.net)

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
#include "ds3.hh"
#include "ds3view.hh"
#if !defined(_ds3slice_H)
# define _ds3slice_H (1)

typedef struct DS3Slice DS3Slice;



struct DS3Slice
{
    int            t_sz;                              /*Size of the slice texture*/
    int            i_id;              /*Timer ID for making the high-detail slice*/
    GLuint         t_id;                                /*ID of the slice texture*/
    GLubyte       *txtr;                                    /*Texture data buffer*/
    unsigned char *cdata;                      /*3D packed array of color indices*/
    GLubyte        ctable[UCHAR_MAX+1][4];
};                        /*Color table*/

void ds3SliceInit(DS3Slice *_this,size_t _dens[3]);
void ds3SliceDstr(DS3Slice *_this,DS3View *_view);

extern const GLWCallbacks DS3_VIEW_SLICE_CALLBACKS;

#endif                                                           /*_ds3slice_H*/
