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
#if !defined(_ds3iso_H)
# define _ds3iso_H (1)

typedef struct DS3IsoVertex  DS3IsoVertex;
typedef struct DS3IsoOctNode DS3IsoOctNode;
typedef struct DS3IsoOctLeaf DS3IsoOctLeaf;
typedef struct DS3IsoSurface DS3IsoSurface;



# define DS3V_NO_EDGE  (-1L)
# define DS3V_NO_CHILD (-1L)

struct DS3IsoVertex
{
    Vect3d vert;
    Vect3d norm;
};



struct DS3IsoOctNode
{
    long node[8];
};



struct DS3IsoOctLeaf
{
    GLint nverts;
    GLint verts[15];
};



struct DS3IsoSurface
{
    CDynArray verts;
    CDynArray nodes;
    CDynArray leafs;
    long      dim;
    long      stp;
};


void ds3IsoInit(DS3IsoSurface *_this,size_t _dens[3]);
void ds3IsoDstr(DS3IsoSurface *_this);

int ds3IsoMake(DS3IsoSurface *_this,DataSet3D *_ds3,double _v,int _d);


extern const GLWCallbacks DS3_VIEW_ISO_CALLBACKS;

#endif                                                             /*_ds3iso_H*/
