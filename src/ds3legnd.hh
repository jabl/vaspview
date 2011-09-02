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

#if !defined(_ds3legnd_H)
# define _ds3legnd_H (1)

#include "ds3.hh"

typedef struct DSColorLegend DSColorLegend;



struct DSColorLegend {
    DSColorLegend();
    GLWComponent  super;
    const DSColorScale *cs;
    GLubyte       ctable[UCHAR_MAX+1][4];
    GLWComponent cm_scale;
    GLWLabel     lb_min;
    GLWLabel     lb_max;
    GLWLabel     lb_label;
};


int            dsColorLegendSetDataSet(DSColorLegend *_this,DataSet3D *_ds3);
void           dsColorLegendSetColorScale(DSColorLegend *_this,
        const DSColorScale *_cs);
int            dsColorLegendSetRange(DSColorLegend *_this,double _min,
                                     double _max);
int            dsColorLegendSetLabel(DSColorLegend *_this,const char *_label);
int            dsColorLegendAddLabel(DSColorLegend *_this,const char *_label);

#endif                                                           /*_ds3legnd_H*/
