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

#if !defined(_ds3iso_H)
# define _ds3iso_H (1)

#include "ds3.hh"


# define DS3V_NO_EDGE  (-1L)
# define DS3V_NO_CHILD (-1L)

struct DS3IsoVertex
{
    Vect3f vert;
    Vect3f norm;
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
    DS3IsoSurface();
    void clear();
    void init(size_t[3]);
    int isoMake(DataSet3D*, double, int);
    std::vector<DS3IsoVertex> verts;
    std::vector<DS3IsoOctNode> nodes;
    std::vector<DS3IsoOctLeaf> leafs;
    long      dim;
    long      stp;
private:
    DS3IsoSurface(const DS3IsoSurface&);
    DS3IsoSurface& operator=(const DS3IsoSurface&);
    void reset(long);
    long addNode();
    long addLeaf(long[12], const int[16]);
    int addTris(long[3], long[12], const int[16]);
    void xForm(DataSet3D*);
};


extern const GLWCallbacks DS3_VIEW_ISO_CALLBACKS;

#endif                                                             /*_ds3iso_H*/
