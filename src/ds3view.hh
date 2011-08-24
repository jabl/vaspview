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

#if !defined(_ds3view_H)
# define _ds3view_H (1)

#include "ds3.hh"

typedef struct DS3View     DS3View;
typedef struct DS3ViewComp DS3ViewComp;



# include "ds3coord.hh"
# include "ds3pts.hh"
# if defined(__DS3_ADD_BONDS__)
#  include "ds3bonds.hh"
# endif
# include "ds3slice.hh"
# include "ds3iso.hh"



# define DS3V_CAPTURE_COLOR GLW_COLOR_RED
# define DS3V_FOCUS_COLOR   GLW_COLOR_GREEN

# define DS3V_ASPECT (1.0)

# define DS3V_PROJECT_PERSPECTIVE  (0)
# define DS3V_PROJECT_ORTHOGRAPHIC (1)

struct DS3ViewComp
{
    GLWComponent  super;
    DS3View      *ds3view;
};



struct DS3View
{
    GLWComponent   super;
    DS3ViewComp   *cm_axes;
    DS3ViewComp   *cm_box;
    DS3ViewComp   *cm_pts;
# if defined(__DS3_ADD_BONDS__)
    DS3ViewComp   *cm_bnds;
# endif
    DS3ViewComp   *cm_slice;
    DS3ViewComp   *cm_iso;
    GLWActionFunc  data_changed_func;
    void          *data_changed_ctx;
    GLWActionFunc  slice_changed_func;
    void          *slice_changed_ctx;
    GLWActionFunc  ornt_changed_func;
    void          *ornt_changed_ctx;
    GLWActionFunc  zoom_changed_func;
    void          *zoom_changed_ctx;
    GLWActionFunc  cntr_changed_func;
    void          *cntr_changed_ctx;
    GLWActionFunc  box_changed_func;
    void          *box_changed_ctx;
    GLWActionFunc  point_changed_func;
    void          *point_changed_ctx;
# if defined(__DS3_ADD_BONDS__)
    GLWActionFunc  bond_changed_func;
    void          *bond_changed_ctx;
#  if defined(__DS3_SAVE_BONDS__)
    GLWActionFunc  bonds_changed_func;
    void          *bonds_changed_ctx;
#  endif
# endif
    DataSet3D     *ds3;
    const DSColorScale  *cs;             /*Color scale for data->color conversion*/
    const DSDataScale   *ds;       /*Data scale for data<->[0,1] interval mapping*/
    /*Display structures*/
# if defined(__DS3_ADD_BONDS__)
    DS3Bonds       bonds;                             /*Bonds between atoms*/
# endif
    DS3Slice       slice;
    DS3IsoSurface  iso;
    /*Cached information*/
    double         offs;              /*Radius of smallest enclosing sphere*/
    double         rot[3][3]; /*Rotation matrix for orientation around cntr*/
    double         basis[16];                   /*GL-formatted basis matrix*/
    double         basinv[3][3];                    /*Inverted basis matrix*/
    double         strans[3][4];              /*Slice transformation matrix*/
    CDynArray      view_stack;                           /*Saved clip boxes*/
    /*Viewing parameters*/
    double         point_r;                                  /*Point radius*/
    double         slice_t;                         /*Slice angles/distance*/
    double         slice_p;
    double         slice_d;
    double         iso_v;                        /*Iso-surface value/detail*/
    int            iso_d;
    /*Position/orientation parameters*/
    int            proj;                                  /*Projection type*/
    double         zoom;                   /*Distance from point to look at*/
    Vect3d         cntr;                                 /*Point to look at*/
    double         roll;                        /*Orientation around center*/
    double         pitch;
    double         yaw;
    double         box[2][3];                                    /*Clip box*/
    /*Mouse/keyboard tracking info*/
    int            track_cb;                  /*Are we tracking a clip box?*/
    int            track_cx;             /*Pixel coordinates of mouse click*/
    int            track_cy;
    int            track_mx;  /*Pixel coordinates of current mouse position*/
    int            track_my;
    Vect3d         track_p0;                      /*Ray from mouse position*/
    Vect3d         track_p1;                          /*(world coordinates)*/
    Vect3d         track_pt; /*Point mouse is currently (world-coordinates)*/
    double         track_t; /*Distance along projection ray to intersection*/
    Vect3d         track_an;/*Point mouse clicked (unit vector from center)*/
    double         track_rd;     /*Distance from center where mouse clicked*/
    int            track_rt;    /*Which half of the sphere is being tracked*/
    double         track_r;                  /*Orientation at time of click*/
    double         track_p;
    double         track_y;
    int            track_ax;                           /*Axis mouse is over*/
    int            track_lx;                     /*Last axis mouse was over*/
    int            track_pl;                  /*Plane keyboard has selected*/
    long           track_mp;                           /*Atom mouse is over*/
    long           track_lp;                     /*Last atom mouse was over*/
    long           track_sp;                                /*Selected atom*/
# if defined(__DS3_ADD_BONDS__)
    long           track_mbf;                   /*Bond mouse is over (from)*/
    long           track_mbt;                     /*Bond mouse is over (to)*/
    long           track_lbf;             /*Last bond mouse was over (from)*/
    long           track_lbt;               /*Last bond mouse was over (to)*/
    long           track_sbf;                        /*Selected bond (from)*/
    long           track_sbt;                          /*Selected bond (to)*/
# endif
    long           track_dx;            /*Data-value on slice mouse is over*/
    long           track_dy;
    long           track_dz;
    /*Which items to draw*/
    CDynArray      draw_point;                 /*Whether to draw each point*/
    unsigned       draw_coords:1;   /*Whether to draw the coordinate system*/
    unsigned       draw_points:1;       /*Whether or not to draw any points*/
    unsigned       draw_slice:1;         /*Whether or not to draw the slice*/
    unsigned       draw_iso:1;     /*Whether or not to draw the iso-surface*/
    /*Which display structures are valid*/
    unsigned       c_valid:1;
    unsigned       t_valid:1;
    unsigned       s_valid:1;
};


void     ds3ViewExpandRot(double _y,double _p,double _r,double _rot[3][3]);

DS3View *ds3ViewAlloc(void);
int      ds3ViewInit(DS3View *_this);
void     ds3ViewDstr(DS3View *_this);
void     ds3ViewFree(DS3View *_this);

void     ds3ViewGetUnprojRay(DS3View *_this,int _x,int _y,
                             Vect3d _p0,Vect3d _p1);
void     ds3ViewSetDataChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetDataChangedCtx(DS3View *_this,void *_ctx);
void     ds3ViewSetSliceChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetSliceChangedCtx(DS3View *_this,void *_ctx);
void     ds3ViewSetOrientationChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetOrientationChangedCtx(DS3View *_this,void *_ctx);
void     ds3ViewSetZoomChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetZoomChangedCtx(DS3View *_this,void *_ctx);
void     ds3ViewSetCenterChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetCenterChangedCtx(DS3View *_this,void *_ctx);
void     ds3ViewSetBoxChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetBoxChangedCtx(DS3View *_this,void *_ctx);
void     ds3ViewSetPointChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetPointChangedCtx(DS3View *_this,void *_ctx);
# if defined(__DS3_ADD_BONDS__)
void     ds3ViewSetBondChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetBondChangedCtx(DS3View *_this,void *_ctx);
#  if defined(__DS3_SAVE_BONDS__)
void     ds3ViewSetBondsChangedFunc(DS3View *_this,GLWActionFunc _func);
void     ds3ViewSetBondsChangedCtx(DS3View *_this,void *_ctx);
#  endif
# endif

void     ds3ViewSetColorScale(DS3View *_this,const DSColorScale *_cs);
void     ds3ViewSetDataScale(DS3View *_this,const DSDataScale *_ds);
int      ds3ViewSetDataSet(DS3View *_this,DataSet3D *_ds3);
void     ds3ViewSetPointR(DS3View *_this,double _r);
void     ds3ViewSetSlice(DS3View *_this,double _t,double _p,double _d);
void     ds3ViewSetIso(DS3View *_this,double _v,int _d);
void     ds3ViewSetDrawCoordS(DS3View *_this,int _b);
void     ds3ViewSetDrawPoints(DS3View *_this,int _b);
void     ds3ViewSetDrawSlice(DS3View *_this,int _b);
void     ds3ViewSetDrawIso(DS3View *_this,int _b);
void     ds3ViewSetOrientation(DS3View *_this,double _y,double _p,double _r);
void     ds3ViewAlignOrientation(DS3View *_this);
void     ds3ViewSetProjectionType(DS3View *_this,int _t);
void     ds3ViewSetZoom(DS3View *_this,double _zoom);
void     ds3ViewSetCenter(DS3View *_this,double _x,double _y,double _z);
void     ds3ViewSetBox(DS3View *_this,double _minx,double _miny,double _minz,
                       double _maxx,double _maxy,double _maxz);
void     ds3ViewSetPointVisible(DS3View *_this,long _pt,int _v);
void     ds3ViewSetSelectedPoint(DS3View *_this,long _pt);
# if defined(__DS3_ADD_BONDS__)
void     ds3ViewSetBond(DS3View *_this,long _from,long _to,double _sz);
void     ds3ViewDelBond(DS3View *_this,long _from,long _to);
void     ds3ViewSetSelectedBond(DS3View *_this,long _from,long _to);
# endif

int      ds3ViewGetData(DS3View *_this,long *_x,long *_y,long *_z);
int      ds3ViewGetPointVisible(DS3View *_this,long _pt);
long     ds3ViewGetSelectedPoint(DS3View *_this);
# if defined(__DS3_ADD_BONDS__)
double   ds3ViewGetBond(DS3View *_this,long _from,long _to);
int      ds3ViewGetSelectedBond(DS3View *_this,long *_from,long *_to);
# endif


extern const GLWCallbacks DS3_VIEW_CALLBACKS;

#endif                                                            /*_ds3view_H*/
