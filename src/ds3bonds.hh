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

#ifndef DS3BONDS_HH
#define DS3BONDS_HH

#include "ds3.hh"

typedef struct DS3Bonds DS3Bonds;



struct DS3Bonds
{
    double *bonds;
    long    natoms;
    long    nbonds;
};


# define _DSBondIdx(_i,_j,_n)                                                 \
 ((_i)*(_n)-((_i)*((_i)+1)>>1)+(_j)-(_i)-1)

void   ds3BondsInit(DS3Bonds *_this);
void   ds3BondsDstr(DS3Bonds *_this);

int    ds3BondsReset(DS3Bonds *_this,DataSet3D *_ds3);
void   ds3BondsSet(DS3Bonds *_this,long _from,long _to,double _sz);
void   ds3BondsDel(DS3Bonds *_this,long _from,long _to);
double ds3BondsGet(DS3Bonds *_this,long _from,long _to);


extern const GLWCallbacks DS3_VIEW_BNDS_CALLBACKS;

#endif                                                           /*_ds3bonds_H*/
