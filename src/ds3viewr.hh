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

#if !defined(_ds3viewr_H)
# define _ds3viewr_H (1)

#include "ds3.hh"
#include "ds3legnd.hh"
#include "ds3view.hh"
#include "ds3vasp.hh"

# define DS3V_ISO_V_RES (200)
# define DS3V_SLIDER_SMALL_WIDTH (100)

class DS3Viewer {
public:
    DS3Viewer();
    ~DS3Viewer();

    GLWFrame          frame;
    DS3View*          ds3view;
    GLWButton         bn_open;
    GLWTextField      tf_file;
    GLWLabel          lb_data_set;
    GLWTabbedPane     tp_ctrl;
    DSColorLegend     legend;
    GLWLabel          lb_status;
    GLWCheckBox       cb_draw_slice;
    GLWLabel          lb_datax;
    GLWLabel          lb_datay;
    GLWLabel          lb_dataz;
    GLWLabel          lb_datav;
    GLWTextField      tf_slice_t;
    GLWSlider*        sl_slice_t;
    GLWTextField      tf_slice_p;
    GLWSlider        *sl_slice_p;
    GLWTextField      tf_slice_d;
    GLWSlider        *sl_slice_d;
    GLWCheckBox       cb_draw_iso;
    GLWTextField      tf_iso_v;
    GLWSlider        *sl_iso_v;
    /*GLWTextField     *tf_iso_d;*/
    GLWSlider        *sl_iso_d;
    GLWCheckBox       cb_draw_points;
    GLWTextField      tf_point_r;
    GLWSlider        *sl_point_r;
    GLWTextField      tf_point_s;
    GLWLabel          lb_point_t;
    GLWLabel          lb_point_l;
    GLWButton         bn_point_c;
    GLWButton         bn_point_v;
    GLWButton         bn_point_sa;
# if defined(__DS3_ADD_BONDS__)
    GLWTextField      tf_bond_f;
    GLWTextField      tf_bond_t;
    GLWTextField      tf_bond_s;
    GLWSlider         sl_bond_s;
    GLWButton         bn_bond_a;
    GLWButton         bn_bond_d;
# endif
    GLWTextField     tf_minx;
    GLWSlider        *sl_minx;
    GLWTextField     tf_maxx;
    GLWSlider        *sl_maxx;
    GLWTextField     tf_miny;
    GLWSlider        *sl_miny;
    GLWTextField     tf_maxy;
    GLWSlider        *sl_maxy;
    GLWTextField     tf_minz;
    GLWSlider        *sl_minz;
    GLWTextField     tf_maxz;
    GLWSlider        *sl_maxz;
    GLWTextField     *tf_zoom;
    GLWSlider        *sl_zoom;
    GLWTextField     *tf_ornt_y;
    GLWSlider        *sl_ornt_y;
    GLWTextField     *tf_ornt_p;
    GLWSlider        *sl_ornt_p;
    GLWTextField     *tf_ornt_r;
    GLWSlider        *sl_ornt_r;
    GLWTextField     *tf_cntr_x;
    GLWSlider        *sl_cntr_x;
    GLWTextField     *tf_cntr_y;
    GLWSlider        *sl_cntr_y;
    GLWTextField     *tf_cntr_z;
    GLWSlider        *sl_cntr_z;
    GLWCheckBox      *cb_draw_coords;
    GLWCheckBoxGroup  cg_scale;
    GLWCheckBox      *cb_scale_linear;
    GLWCheckBox      *cb_scale_log;
    GLWCheckBoxGroup  cg_color;
    GLWCheckBox      *cb_color_rainbow;
    GLWCheckBox      *cb_color_grayscale;
    GLWCheckBoxGroup  cg_backc;
    GLWCheckBox      *cb_backc_black;
    GLWCheckBox      *cb_backc_white;
    GLWCheckBoxGroup  cg_projt;
    GLWCheckBox      *cb_projt_persp;
    GLWCheckBox      cb_projt_ortho;
    DSLinearScale     scale_linear;
    DSLogScale        scale_log;
    DataSet3D*        ds3;
# if defined(__DS3_ADD_BONDS__)&&defined(__DS3_SAVE_BONDS__)
    std::string       bond_name;
# endif
    std::string       read_name;
    DS3VaspReader*    reader;
    int               read_prog;
    int               read_id;
private:
    // Forbid copying
    DS3Viewer& operator=(const DS3Viewer&);
    DS3Viewer(const DS3Viewer&);
};

void ds3ViewerUpdatePointRLabels(DS3Viewer *_this);
void ds3ViewerUpdateIsoVLabels(DS3Viewer *_this);
void ds3ViewerUpdateZoomLabels(DS3Viewer *_this);
void ds3ViewerSetPointR(DS3Viewer *_this,double _r);
void ds3ViewerSetSlice(DS3Viewer *_this,double _t,double _p,double _d);
void ds3ViewerSetIso(DS3Viewer *_this,double _v,int _d);
void ds3ViewerSetZoom(DS3Viewer *_this,double _z);
void ds3ViewerSetOrientation(DS3Viewer *_this,double _y,double _p,double _r);
void ds3ViewerSetCenter(DS3Viewer *_this,double _x,double _y,double _z);
void ds3ViewerSetBox(DS3Viewer *_this,double _minx,double _miny,double _minz,
                     double _maxx,double _maxy,double _maxz);
void ds3ViewerSetSelectedPoint(DS3Viewer *_this,long _pt);
void ds3ViewerSetPointVisible(DS3Viewer *_this,int _v);
# if defined(__DS3_ADD_BONDS__)
void ds3ViewerSetSelectedBond(DS3Viewer *_this,long _f,long _t);
void ds3ViewerSetBond(DS3Viewer *_this,double _sz);
# endif
void ds3ViewerOpenFile(DS3Viewer *_this,const char *_file);

#endif                                                           /*_ds3viewr_H*/
