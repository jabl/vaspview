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
#include "ds3.hh"
#include "ds3vasp.hh"
#include "ds3legnd.hh"
#include "ds3view.hh"
#include "ds3viewr.hh"
#include "file.hh"
#include "strutil.hh"

static void ds3ViewerTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerTextSet(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerViewDataChanged(DS3Viewer *_this,DS3View *_view);
static void ds3ViewerViewSliceChanged(DS3Viewer *_this,DS3View *_view);
static void ds3ViewerViewZoomChanged(DS3Viewer *_this,DS3View *_view);
static void ds3ViewerViewOrientationChanged(DS3Viewer *_this,DS3View *_view);
static void ds3ViewerViewCenterChanged(DS3Viewer *_this,DS3View *_view);
static void ds3ViewerViewBoxChanged(DS3Viewer *_this,DS3View *_view);
static void ds3ViewerViewPointChanged(DS3Viewer *_this,GLWComponent *_c);
# if defined(__DS3_ADD_BONDS__)
static void ds3ViewerViewBondChanged(DS3Viewer *_this,GLWComponent *_c);
#  if defined(__DS3_SAVE_BONDS__)
static void ds3ViewerSaveBonds(DS3Viewer *_this);
static void ds3ViewerLoadBonds(DS3Viewer *_this);
#  endif
# endif
static void ds3ViewerFileTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerDrawSliceChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerSliceTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerSliceTSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerSlicePSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerSliceDSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerDrawIsoChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerIsoVTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerIsoVSliderChanged(DS3Viewer *_this,GLWComponent *_c);
/*static void ds3ViewerIsoDTextChanged(DS3Viewer *_this,GLWComponent *_c)*/
static void ds3ViewerIsoDSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerDrawCoordSChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerDrawPointsChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerPointRTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerPointRSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerPointSTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerPointVisible(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerPointShowAll(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerPointCenter(DS3Viewer *_this,GLWComponent *_c);
# if defined(__DS3_ADD_BONDS__)
static void ds3ViewerBondFTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerBondTTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerBondSTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerBondSSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerBondAdd(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerBondDel(DS3Viewer *_this,GLWComponent *_c);
# endif
static void ds3ViewerBoxTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerMinXSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerMinYSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerMinZSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerMaxXSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerMaxYSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerMaxZSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerZoomTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerZoomSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerOrntTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerOrntYSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerOrntPSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerOrntRSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerCntrTextChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerCntrXSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerCntrYSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerCntrZSliderChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerResetOrientation(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerResetPosition(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerAlignOrientation(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerScaleChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerColorChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerBackCChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerProjTChanged(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerFinishRead(DS3Viewer *_this);
static void ds3ViewerAsyncRead(DS3Viewer *_this,GLWComponent *_c);
static void ds3ViewerOpen(DS3Viewer *_this,GLWComponent *_c);



static void ds3ViewerTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    glwCompSetForeColor(_c,GLW_COLOR_RED);
}

static void ds3ViewerTextSet(DS3Viewer *_this,GLWComponent *_c)
{
    glwCompSetForeColor(_c,_this->frame->super.forec);
}

static void ds3ViewerViewDataChanged(DS3Viewer *_this,DS3View *_view)
{
    long x;
    long y;
    long z;
    if (ds3ViewGetData(_view,&x,&y,&z)) {
        char   text[32];
        double v;
        sprintf(text,"X: %li",x);
        glwLabelSetLabel(_this->lb_datax,text);
        sprintf(text,"Y: %li",y);
        glwLabelSetLabel(_this->lb_datay,text);
        sprintf(text,"Z: %li",z);
        glwLabelSetLabel(_this->lb_dataz,text);
        v=_this->ds3->data[_DS3Index(_this->ds3,x,y,z)];
        sprintf(text,"Value: %0.6g",v);
        glwLabelSetLabel(_this->lb_datav,text);
    } else {
        glwLabelSetLabel(_this->lb_datax,"X:");
        glwLabelSetLabel(_this->lb_datay,"Y:");
        glwLabelSetLabel(_this->lb_dataz,"Z:");
        glwLabelSetLabel(_this->lb_datav,"Value:");
    }
}

static void ds3ViewerViewSliceChanged(DS3Viewer *_this,DS3View *_view)
{
    ds3ViewerSetSlice(_this,_view->slice_t,_view->slice_p,_view->slice_d);
}

static void ds3ViewerViewZoomChanged(DS3Viewer *_this,DS3View *_view)
{
    ds3ViewerSetZoom(_this,_view->zoom/(_view->offs>1E-16?_view->offs:1));
}

static void ds3ViewerViewOrientationChanged(DS3Viewer *_this,DS3View *_view)
{
    ds3ViewerSetOrientation(_this,_view->yaw,_view->pitch,_view->roll);
}

static void ds3ViewerViewCenterChanged(DS3Viewer *_this,DS3View *_view)
{
    Vect3d cntr;
    int    i;
    for (i=0; i<3; i++)cntr[i]=vectDot3d(_view->basinv[i],_view->cntr);
    ds3ViewerSetCenter(_this,cntr[X],cntr[Y],cntr[Z]);
}

static void ds3ViewerViewBoxChanged(DS3Viewer *_this,DS3View *_view)
{
    ds3ViewerSetBox(_this,_view->box[0][X],_view->box[0][Y],_view->box[0][Z],
                    _view->box[1][X],_view->box[1][Y],_view->box[1][Z]);
}

static void ds3ViewerViewPointChanged(DS3Viewer *_this,GLWComponent *_c)
{
    long pt;
    pt=ds3ViewGetSelectedPoint(_this->ds3view);
    ds3ViewerSetSelectedPoint(_this,pt);
    if (pt>=0) {
        ds3ViewerSetPointVisible(_this,
                                 ds3ViewGetPointVisible(_this->ds3view,pt));
    }
}

# if defined(__DS3_ADD_BONDS__)
static void ds3ViewerViewBondChanged(DS3Viewer *_this,GLWComponent *_c)
{
    long bf;
    long bt;
    if (ds3ViewGetSelectedBond(_this->ds3view,&bf,&bt)) {
        ds3ViewerSetSelectedBond(_this,bf,bt);
        ds3ViewerSetBond(_this,ds3ViewGetBond(_this->ds3view,bf,bt));
    } else ds3ViewerSetSelectedBond(_this,-1,-1);
}

#  if defined(__DS3_SAVE_BONDS__)
static void ds3ViewerSaveBonds(DS3Viewer *_this)
{
    if (!_this->bond_name.empty()) {
        FILE *file;
        int   err;
        file=fopen(_this->bond_name.c_str(),"w");
        err=1;
        if (file!=NULL) {
            long bf;
            long bt = 0;  // Just to shut up "may be used uninitialized" warning
            if (fprintf(file,"#Bond information for \"%s\"\n",_this->ds3->name.c_str())>=0) {
                for (bf=0; (size_t)bf+1<_this->ds3->npoints; bf++) {
                    for (bt=bf+1; (size_t)bt<_this->ds3->npoints; bt++) {
                        double s;
                        s=ds3ViewGetBond(_this->ds3view,bf,bt);
                        if (s>0&&fprintf(file,"%li %li %0.9g\n",bf,bt,s*10)<0)break;
                    }
                }
                if ((size_t)bf+1==_this->ds3->npoints&&
                        (size_t)bt==_this->ds3->npoints) {
                    err=0;
                }
            }
            fclose(file);
        }
        if (err) {
            glwLabelSetLabel(_this->lb_status,
                             "Error writing bond information to \"");
            glwLabelAddLabel(_this->lb_status, _this->bond_name.c_str());
            glwLabelAddLabel(_this->lb_status,"\": ");
            glwLabelAddLabel(_this->lb_status,strerror(errno));
        }
    }
}

static void ds3ViewerLoadBonds(DS3Viewer *_this)
{
    if (!_this->bond_name.empty()) {
        File file(_this->bond_name.c_str(), "r");
        int   err;
        err=1;
        if (file.f != NULL) {
            std::string line;
            ds3ViewSetBondsChangedFunc(_this->ds3view,NULL);
            while (!feof(file.f)) {
                char *p;
                if (!file.fgets(line)) {
                    if (!ferror(file.f))errno=ENOMEM;
                    break;
                }
                trimlr(line);
                p = &line[0];
                if (p[0]!='#'&&p[0]!='\0') {
                    long   bf;
                    long   bt;
                    double s;
                    if (sscanf(p,"%li %li %lf",&bf,&bt,&s)<3) {
                        errno=EINVAL;
                        break;
                    }
                    ds3ViewSetBond(_this->ds3view,bf,bt,s*0.1);
                }
            }
            if (feof(file.f))err=0;
            ds3ViewSetBondsChangedFunc(_this->ds3view,
                                       (GLWActionFunc)ds3ViewerSaveBonds);
        } else if (errno==ENOENT)err=0;
        if (err) {
            glwLabelSetLabel(_this->lb_status,
                             "Error reading bond information from \"");
            glwLabelAddLabel(_this->lb_status, _this->bond_name.c_str());
            glwLabelAddLabel(_this->lb_status,"\": ");
            glwLabelAddLabel(_this->lb_status,strerror(errno));
        }
    }
}
#  endif
# endif

static void ds3ViewerFileTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char *text;
    glwCompSetForeColor(_c,_c->parent==NULL?GLW_COLOR_BLACK:_c->parent->forec);
    text = glwTextFieldGetText(&_this->tf_file);
    glwCompEnable(&_this->bn_open.super,text!=NULL&&text[0]!='\0');
}

static void ds3ViewerDrawSliceChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int b;
    b=glwCheckBoxGetState(_this->cb_draw_slice);
    ds3ViewSetDrawSlice(_this->ds3view,b);
    glwCompEnable(&_this->tf_slice_t->super,b);
    glwCompEnable(&_this->sl_slice_t->super,b);
    glwCompEnable(&_this->tf_slice_p->super,b);
    glwCompEnable(&_this->sl_slice_p->super,b);
    glwCompEnable(&_this->tf_slice_d->super,b);
    glwCompEnable(&_this->sl_slice_d->super,b);
}

static void ds3ViewerSliceTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  t;
    double  p;
    double  d;
    int     i;
    i=0;
    ds3ViewerTextSet(_this,&_this->tf_slice_t->super);
    text=glwTextFieldGetText(_this->tf_slice_t);
    t=strtod(text,&e);
    if (e==text||e[0]!='\0')t=_this->ds3view->slice_t;
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_slice_p->super);
    text=glwTextFieldGetText(_this->tf_slice_p);
    p=strtod(text,&e);
    if (e==text||e[0]!='\0')p=_this->ds3view->slice_p;
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_slice_d->super);
    text=glwTextFieldGetText(_this->tf_slice_d);
    d=strtod(text,&e);
    if (e==text||e[0]!='\0')d=_this->ds3view->slice_d;
    else i=1;
    if (i)ds3ViewerSetSlice(_this,t,p,d);
}

static void ds3ViewerSliceTSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v = _this->sl_slice_t->getVal();
    ds3ViewerSetSlice(_this,v,_this->ds3view->slice_p,_this->ds3view->slice_d);
}

static void ds3ViewerSlicePSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_slice_p);
    ds3ViewerSetSlice(_this,_this->ds3view->slice_t,v,_this->ds3view->slice_d);
}

static void ds3ViewerSliceDSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_slice_d);
    ds3ViewerSetSlice(_this,_this->ds3view->slice_t,_this->ds3view->slice_p,
                      v*0.01);
}

static void ds3ViewerDrawIsoChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int b;
    b=glwCheckBoxGetState(_this->cb_draw_iso);
    ds3ViewSetDrawIso(_this->ds3view,b);
    glwCompEnable(&_this->tf_iso_v->super,b);
    glwCompEnable(&_this->sl_iso_v->super,b);
    /*glwCompEnable(&_this->tf_iso_d->super,b);*/
    glwCompEnable(&_this->sl_iso_d->super,b);
}

static void ds3ViewerIsoVTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  v;
    ds3ViewerTextSet(_this,_c);
    text=glwTextFieldGetText(_this->tf_iso_v);
    v=strtod(text,&e);
    if (e!=text&&e[0]=='\0') {
        ds3ViewerSetIso(_this,v,_this->ds3view->iso_d);
    }
}

static void ds3ViewerIsoVSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_iso_v);
    ds3ViewerSetIso(_this,dsUnscale(_this->ds3view->ds,v/(double)DS3V_ISO_V_RES),
                    _this->ds3view->iso_d);
}

/*static void ds3ViewerIsoDTextChanged(DS3Viewer *_this,GLWComponent *_c){
 const char *text;
       char *e;
       long  d;
 ds3ViewerTextSet(_this,_c);
 text=glwTextFieldGetText(_this->tf_iso_d);
 d=strtol(text,&e,0);
 if(e!=text&&e[0]=='\0'){
  ds3ViewerSetIso(_this,_this->ds3.min+_this->ds3view->iso_v*
                        (_this->ds3.max-_this->ds3.min),d);} }*/

static void ds3ViewerIsoDSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_iso_d);
    ds3ViewerSetIso(_this,_this->ds3view->iso_v,v);
}

static void ds3ViewerDrawCoordSChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int b;
    b=glwCheckBoxGetState(_this->cb_draw_coords);
    ds3ViewSetDrawCoordS(_this->ds3view,b);
}

static void ds3ViewerDrawPointsChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int b;
    b=glwCheckBoxGetState(_this->cb_draw_points);
    ds3ViewSetDrawPoints(_this->ds3view,b);
# if defined(__DS3_ADD_BONDS__)
    glwCompEnable(&_this->tf_bond_f->super,b);
    glwCompEnable(&_this->tf_bond_t->super,b);
    glwCompEnable(&_this->tf_bond_s->super,b);
    glwCompEnable(&_this->sl_bond_s->super,b);
    glwCompEnable(&_this->bn_bond_a->super,b);
    glwCompEnable(&_this->bn_bond_d->super,b);
# endif
    glwCompEnable(&_this->tf_point_r->super,b);
    glwCompEnable(&_this->sl_point_r->super,b);
    glwCompEnable(&_this->tf_point_s->super,b);
    glwCompEnable(&_this->bn_point_v->super,b);
    glwCompEnable(&_this->bn_point_sa->super,b);
    glwCompEnable(&_this->bn_point_c->super,b);
}

static void ds3ViewerPointRTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  r;
    ds3ViewerTextSet(_this,_c);
    text=glwTextFieldGetText(_this->tf_point_r);
    r=strtod(text,&e);
    if (e!=text&&e[0]=='\0')ds3ViewerSetPointR(_this,r);
}

static void ds3ViewerPointRSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_point_r);
    ds3ViewerSetPointR(_this,v*0.001*_this->ds3view->offs);
}

static void ds3ViewerPointSTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char *text;
    char *e;
    long  l;
    ds3ViewerTextSet(_this,_c);
    text=glwTextFieldGetText(_this->tf_point_s);
    l=strtol(text,&e,0)-1;
    if (e!=text&&e[0]=='\0')ds3ViewerSetSelectedPoint(_this,l);
}

static void ds3ViewerPointVisible(DS3Viewer *_this,GLWComponent *_c)
{
    long l;
    ds3ViewerPointSTextChanged(_this,_c);
    l=ds3ViewGetSelectedPoint(_this->ds3view);
    ds3ViewSetPointVisible(_this->ds3view,l,
                           !ds3ViewGetPointVisible(_this->ds3view,l));
}

static void ds3ViewerPointShowAll(DS3Viewer *_this,GLWComponent *_c)
{
    long l;
    for (l=0; l<(long)_this->ds3->npoints; l++) {
        ds3ViewSetPointVisible(_this->ds3view,l,1);
    }
}

static void ds3ViewerPointCenter(DS3Viewer *_this,GLWComponent *_c)
{
    long l;
    ds3ViewerPointSTextChanged(_this,_c);
    l=ds3ViewGetSelectedPoint(_this->ds3view);
    if (l>=0) {
        ds3ViewerSetCenter(_this,_this->ds3->points[l].pos[X],
                           _this->ds3->points[l].pos[Y],
                           _this->ds3->points[l].pos[Z]);
        ds3ViewSetZoom(_this->ds3view,_this->ds3view->zoom*0.5);
    }
}

# if defined(__DS3_ADD_BONDS__)
static void ds3ViewerBondFTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char *text;
    char *e;
    long  f;
    ds3ViewerTextSet(_this,_c);
    text=glwTextFieldGetText(_this->tf_bond_f);
    f=strtol(text,&e,0)-1;
    if (e!=text&&e[0]=='\0') {
        long t;
        text=glwTextFieldGetText(_this->tf_bond_t);
        t=strtol(text,&e,0)-1;
        if (e!=text&&e[0]=='\0')ds3ViewerSetSelectedBond(_this,f,t);
    }
}

static void ds3ViewerBondTTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    ds3ViewerBondFTextChanged(_this,_c);
}

static void ds3ViewerBondSTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  r;
    ds3ViewerTextSet(_this,_c);
    text=glwTextFieldGetText(_this->tf_bond_s);
    r=strtod(text,&e);
    if (e!=text&&e[0]=='\0') {
        ds3ViewerBondFTextChanged(_this,_c);
        ds3ViewerSetBond(_this,r*0.1);
    }
}

static void ds3ViewerBondSSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_bond_s);
    ds3ViewerBondFTextChanged(_this,_c);
    ds3ViewerSetBond(_this,v*0.1);
}

static void ds3ViewerBondAdd(DS3Viewer *_this,GLWComponent *_c)
{
    const char *text;
    char *e;
    long  f;
    int   v;
    v=glwSliderGetVal(_this->sl_bond_s);
    text=glwTextFieldGetText(_this->tf_bond_f);
    f=strtol(text,&e,0)-1;
    if (e!=text&&e[0]=='\0') {
        long t;
        text=glwTextFieldGetText(_this->tf_bond_t);
        t=strtol(text,&e,0)-1;
        if (e!=text&&e[0]=='\0') {
            ds3ViewSetBond(_this->ds3view,f,t,v*0.1);
            ds3ViewSetSelectedBond(_this->ds3view,f,t);
        }
    }
}

static void ds3ViewerBondDel(DS3Viewer *_this,GLWComponent *_c)
{
    ds3ViewerBondFTextChanged(_this,_c);
    ds3ViewerSetBond(_this,0);
}
# endif

static void ds3ViewerBoxTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  minx;
    double  miny;
    double  minz;
    double  maxx;
    double  maxy;
    double  maxz;
    int     i;
    i=0;
    ds3ViewerTextSet(_this,&_this->tf_minx->super);
    text=glwTextFieldGetText(_this->tf_minx);
    minx=strtod(text,&e);
    if (e==text||e[0]!='\0')minx=_this->ds3view->box[0][X];
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_miny->super);
    text=glwTextFieldGetText(_this->tf_miny);
    miny=strtod(text,&e);
    if (e==text||e[0]!='\0')miny=_this->ds3view->box[0][Y];
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_minz->super);
    text=glwTextFieldGetText(_this->tf_minz);
    minz=strtod(text,&e);
    if (e==text||e[0]!='\0')minz=_this->ds3view->box[0][Z];
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_maxx->super);
    text=glwTextFieldGetText(_this->tf_maxx);
    maxx=strtod(text,&e);
    if (e==text||e[0]!='\0')maxx=_this->ds3view->box[1][X];
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_maxy->super);
    text=glwTextFieldGetText(_this->tf_maxy);
    maxy=strtod(text,&e);
    if (e==text||e[0]!='\0')maxy=_this->ds3view->box[1][Y];
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_maxz->super);
    text=glwTextFieldGetText(_this->tf_maxz);
    maxz=strtod(text,&e);
    if (e==text||e[0]!='\0')maxz=_this->ds3view->box[1][Z];
    else i=1;
    if (i)ds3ViewerSetBox(_this,minx,miny,minz,maxx,maxy,maxz);
}

static void ds3ViewerMinXSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    double v;
    v=glwSliderGetVal(_this->sl_minx)*0.01;
    if (v>_this->ds3view->box[1][X])v=_this->ds3view->box[1][X];
    ds3ViewerSetBox(_this,v,_this->ds3view->box[0][Y],
                    _this->ds3view->box[0][Z],_this->ds3view->box[1][X],
                    _this->ds3view->box[1][Y],_this->ds3view->box[1][Z]);
}

static void ds3ViewerMinYSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    double v;
    v=glwSliderGetVal(_this->sl_miny)*0.01;
    if (v>_this->ds3view->box[1][Y])v=_this->ds3view->box[1][Y];
    ds3ViewerSetBox(_this,_this->ds3view->box[0][X],v,
                    _this->ds3view->box[0][Z],_this->ds3view->box[1][X],
                    _this->ds3view->box[1][Y],_this->ds3view->box[1][Z]);
}

static void ds3ViewerMinZSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    double v;
    v=glwSliderGetVal(_this->sl_minz)*0.01;
    if (v>_this->ds3view->box[1][Z])v=_this->ds3view->box[1][Z];
    ds3ViewerSetBox(_this,_this->ds3view->box[0][X],_this->ds3view->box[0][Y],
                    v,_this->ds3view->box[1][X],
                    _this->ds3view->box[1][Y],_this->ds3view->box[1][Z]);
}

static void ds3ViewerMaxXSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_maxx);
    ds3ViewerSetBox(_this,_this->ds3view->box[0][X],_this->ds3view->box[0][Y],
                    _this->ds3view->box[0][Z],v*0.01,
                    _this->ds3view->box[1][Y],_this->ds3view->box[1][Z]);
}

static void ds3ViewerMaxYSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_maxy);
    ds3ViewerSetBox(_this,_this->ds3view->box[0][X],_this->ds3view->box[0][Y],
                    _this->ds3view->box[0][Z],_this->ds3view->box[1][X],
                    v*0.01,_this->ds3view->box[1][Z]);
}

static void ds3ViewerMaxZSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_maxz);
    ds3ViewerSetBox(_this,_this->ds3view->box[0][X],_this->ds3view->box[0][Y],
                    _this->ds3view->box[0][Z],_this->ds3view->box[1][X],
                    _this->ds3view->box[1][Y],v*0.01);
}

static void ds3ViewerZoomTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  r;
    ds3ViewerTextSet(_this,_c);
    text=glwTextFieldGetText(_this->tf_zoom);
    r=strtod(text,&e);
    if (e!=text&&e[0]=='\0')ds3ViewerSetZoom(_this,r);
}

static void ds3ViewerZoomSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_zoom);
    ds3ViewerSetZoom(_this,v*0.01);
}

static void ds3ViewerOrntTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  y;
    double  p;
    double  r;
    int     i;
    i=0;
    ds3ViewerTextSet(_this,&_this->tf_ornt_y->super);
    text=glwTextFieldGetText(_this->tf_ornt_y);
    y=strtod(text,&e);
    if (e==text||e[0]!='\0')y=_this->ds3view->yaw;
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_ornt_p->super);
    text=glwTextFieldGetText(_this->tf_ornt_p);
    p=strtod(text,&e);
    if (e==text||e[0]!='\0')p=_this->ds3view->pitch;
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_ornt_r->super);
    text=glwTextFieldGetText(_this->tf_ornt_r);
    r=strtod(text,&e);
    if (e==text||e[0]!='\0')r=_this->ds3view->roll;
    else i=1;
    if (i)ds3ViewerSetOrientation(_this,y,p,r);
}

static void ds3ViewerOrntYSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_ornt_y);
    ds3ViewerSetOrientation(_this,v,_this->ds3view->pitch,_this->ds3view->roll);
}

static void ds3ViewerOrntPSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_ornt_p);
    ds3ViewerSetOrientation(_this,_this->ds3view->yaw,v,_this->ds3view->roll);
}

static void ds3ViewerOrntRSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    int v;
    v=glwSliderGetVal(_this->sl_ornt_r);
    ds3ViewerSetOrientation(_this,_this->ds3view->yaw,_this->ds3view->pitch,v);
}

static void ds3ViewerCntrTextChanged(DS3Viewer *_this,GLWComponent *_c)
{
    const char   *text;
    char   *e;
    double  x;
    double  y;
    double  z;
    int     i;
    i=0;
    ds3ViewerTextSet(_this,&_this->tf_cntr_x->super);
    text=glwTextFieldGetText(_this->tf_cntr_x);
    x=strtod(text,&e);
    if (e==text||e[0]!='\0')x=vectDot3d(_this->ds3view->basinv[X],_this->ds3view->cntr);
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_cntr_y->super);
    text=glwTextFieldGetText(_this->tf_cntr_y);
    y=strtod(text,&e);
    if (e==text||e[0]!='\0')y=vectDot3d(_this->ds3view->basinv[Y],_this->ds3view->cntr);
    else i=1;
    ds3ViewerTextSet(_this,&_this->tf_cntr_z->super);
    text=glwTextFieldGetText(_this->tf_cntr_z);
    z=strtod(text,&e);
    if (e==text||e[0]!='\0')z=vectDot3d(_this->ds3view->basinv[Z],_this->ds3view->cntr);
    else i=1;
    if (i)ds3ViewerSetCenter(_this,x,y,z);
}

static void ds3ViewerCntrXSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    Vect3d cntr;
    cntr[X]=glwSliderGetVal(_this->sl_cntr_x)*0.01;
    cntr[Y]=vectDot3d(_this->ds3view->basinv[Y],_this->ds3view->cntr);
    cntr[Z]=vectDot3d(_this->ds3view->basinv[Z],_this->ds3view->cntr);
    ds3ViewerSetCenter(_this,cntr[X],cntr[Y],cntr[Z]);
}

static void ds3ViewerCntrYSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    Vect3d cntr;
    cntr[X]=vectDot3d(_this->ds3view->basinv[X],_this->ds3view->cntr);
    cntr[Y]=glwSliderGetVal(_this->sl_cntr_y)*0.01;
    cntr[Z]=vectDot3d(_this->ds3view->basinv[Z],_this->ds3view->cntr);
    ds3ViewerSetCenter(_this,cntr[X],cntr[Y],cntr[Z]);
}

static void ds3ViewerCntrZSliderChanged(DS3Viewer *_this,GLWComponent *_c)
{
    Vect3d cntr;
    cntr[X]=vectDot3d(_this->ds3view->basinv[X],_this->ds3view->cntr);
    cntr[Y]=vectDot3d(_this->ds3view->basinv[Y],_this->ds3view->cntr);
    cntr[Z]=glwSliderGetVal(_this->sl_cntr_z)*0.01;
    ds3ViewerSetCenter(_this,cntr[X],cntr[Y],cntr[Z]);
}

static void ds3ViewerResetOrientation(DS3Viewer *_this,GLWComponent *_c)
{
    ds3ViewerSetOrientation(_this,0,0,0);
}

static void ds3ViewerResetPosition(DS3Viewer *_this,GLWComponent *_c)
{
    ds3ViewerSetZoom(_this,1);
    ds3ViewerSetCenter(_this,0.5,0.5,0.5);
}

static void ds3ViewerAlignOrientation(DS3Viewer *_this,GLWComponent *_c)
{
    ds3ViewAlignOrientation(_this->ds3view);
}

static void ds3ViewerScaleChanged(DS3Viewer *_this,GLWComponent *_c)
{
    switch (glwCheckBoxGroupGetSelectedIdx(&_this->cg_scale)) {
    case 0:
        ds3ViewSetDataScale(_this->ds3view,&_this->scale_linear.super);
        break;
    case 1:
        ds3ViewSetDataScale(_this->ds3view,&_this->scale_log.super);
        break;
    }
    ds3ViewerUpdateIsoVLabels(_this);
    ds3ViewerSetIso(_this,_this->ds3view->iso_v,_this->ds3view->iso_d);
}

static void ds3ViewerColorChanged(DS3Viewer *_this,GLWComponent *_c)
{
    switch (glwCheckBoxGroupGetSelectedIdx(&_this->cg_color)) {
    case 0: {
        ds3ViewSetColorScale(_this->ds3view,&DS_RAINBOW_SCALE);
        dsColorLegendSetColorScale(_this->legend,&DS_RAINBOW_SCALE);
    }
    break;
    case 1: {
        ds3ViewSetColorScale(_this->ds3view,&DS_GRAY_SCALE);
        dsColorLegendSetColorScale(_this->legend,&DS_GRAY_SCALE);
    }
    break;
    }
}

static void ds3ViewerBackCChanged(DS3Viewer *_this,GLWComponent *_c)
{
    switch (glwCheckBoxGroupGetSelectedIdx(&_this->cg_backc)) {
    case 0: {
        glwCompSetBackColor(&_this->ds3view->super,GLW_COLOR_BLACK);
        glwCompSetForeColor(&_this->ds3view->super,GLW_COLOR_WHITE);
    }
    break;
    case 1: {
        glwCompSetBackColor(&_this->ds3view->super,GLW_COLOR_WHITE);
        glwCompSetForeColor(&_this->ds3view->super,GLW_COLOR_BLACK);
    }
    break;
    }
}

static void ds3ViewerProjTChanged(DS3Viewer *_this,GLWComponent *_c)
{
    switch (glwCheckBoxGroupGetSelectedIdx(&_this->cg_projt)) {
    case 0: {
        ds3ViewSetProjectionType(_this->ds3view,DS3V_PROJECT_PERSPECTIVE);
    }
    break;
    case 1: {
        ds3ViewSetProjectionType(_this->ds3view,
                                 DS3V_PROJECT_ORTHOGRAPHIC);
    }
    break;
    }
}

static void ds3ViewerFinishRead(DS3Viewer *_this)
{
    double iso_v;
    delete _this->ds3;
    _this->ds3 = _this->reader->release_ds3();
    iso_v=dsScale(_this->ds3view->ds,_this->ds3view->iso_v);
    if (ds3ViewSetDataSet(_this->ds3view, _this->ds3)) {
        dsLinearScaleInit(&_this->scale_linear, _this->ds3->min,
                          _this->ds3->max);
        dsLogScaleInit(&_this->scale_log,_this->ds3->min,_this->ds3->max);
        dsColorLegendSetDataSet(_this->legend, _this->ds3);
        ds3ViewerUpdatePointRLabels(_this);
        ds3ViewerUpdateIsoVLabels(_this);
        /*ds3ViewerUpdateZoomLabels(_this);*/
        ds3ViewerSetPointR(_this,glwSliderGetVal(_this->sl_point_r)*
                           0.001*_this->ds3view->offs);
        ds3ViewerSetSlice(_this,_this->ds3view->slice_t,_this->ds3view->slice_p,
                          _this->ds3view->slice_d);
        ds3ViewerSetIso(_this,dsUnscale(_this->ds3view->ds,iso_v),
                        _this->ds3view->iso_d);

        glwLabelSetLabel(_this->lb_data_set,"Data Set: ");
        glwLabelAddLabel(_this->lb_data_set,_this->ds3->name.c_str());
        glwLabelSetLabel(_this->lb_status,"\"");
        glwLabelAddLabel(_this->lb_status, _this->read_name.c_str());
        glwLabelAddLabel(_this->lb_status,"\" Loaded.");
# if defined(__DS3_ADD_BONDS__)&&defined(__DS3_SAVE_BONDS__)
	_this->bond_name = _this->read_name;
	_this->bond_name.append(".aux");
	ds3ViewerLoadBonds(_this);
#endif
    } else {
        ds3ViewSetDataSet(_this->ds3view,NULL);
        glwLabelSetLabel(_this->lb_status,strerror(ENOMEM));
    }
}

static void ds3ViewerAsyncRead(DS3Viewer *_this,GLWComponent *_c)
{
    int ret;
    ret=_this->reader->read();
    if (ret<=0) {
        glwCompDelIdler(&_this->frame->super,_this->read_id);
        _this->read_id=0;
        if (!ret) {
            if (!errno)errno=ENOMEM;
            glwLabelSetLabel(_this->lb_status,"Error reading \"");
            glwLabelAddLabel(_this->lb_status, _this->read_name.c_str());
            glwLabelAddLabel(_this->lb_status,"\": ");
            glwLabelAddLabel(_this->lb_status,strerror(errno));
        } else ds3ViewerFinishRead(_this);
	_this->read_name.clear();
        delete _this->reader;
    } else if (_this->read_prog!=--ret) {
        char text[32];
        _this->read_prog=ret;
        sprintf(text,"%i%%",ret);
        glwLabelSetLabel(_this->lb_status,"Loading \"");
        glwLabelAddLabel(_this->lb_status, _this->read_name.c_str());
        glwLabelAddLabel(_this->lb_status,"\"... ");
        glwLabelAddLabel(_this->lb_status,text);
    }
}

static void ds3ViewerOpen(DS3Viewer *_this,GLWComponent *_c)
{
    const char *file;
    file = glwTextFieldGetText(&_this->tf_file);
    ds3ViewerOpenFile(_this,file);
}

DS3Viewer::DS3Viewer() :
        frame(new GLWFrame("VASP Data Viewer")),
        ds3view(new DS3View()),
        bn_open("Open"),
        tf_file(NULL, 20),
        lb_data_set(new GLWLabel("Data Set: ")),
        tp_ctrl(new GLWTabbedPane()),
        legend(new DSColorLegend()),
        lb_status(new GLWLabel(NULL)),
        cb_draw_slice(new GLWCheckBox("Draw Slice",1,NULL)),
        lb_datax(new GLWLabel(NULL)),
        lb_datay(new GLWLabel(NULL)),
        lb_dataz(new GLWLabel(NULL)),
        lb_datav(new GLWLabel(NULL)),
        tf_slice_t(new GLWTextField(NULL,5)),
        tf_slice_p(new GLWTextField(NULL,5)),
        tf_slice_d(new GLWTextField(NULL,5)),
        cb_draw_iso(new GLWCheckBox("Draw Iso-Surface",1,NULL)),
        tf_iso_v(new GLWTextField(NULL,5)),
        cb_draw_points(new GLWCheckBox("Draw Atoms",1,NULL)),
        tf_point_r(new GLWTextField(NULL,5)),
        tf_point_s(new GLWTextField(NULL,5)),
        lb_point_t(new GLWLabel("Atom type:")),
        lb_point_l(new GLWLabel("Atom location:")),
        bn_point_c(new GLWButton("Look at Atom")),
        bn_point_v(new GLWButton("Hide Atom")),
        bn_point_sa(new GLWButton("Show All Atoms")),
#if defined(__DS3_ADD_BONDS__)
        tf_bond_f(new GLWTextField(NULL,5)),
        tf_bond_t(new GLWTextField(NULL,5)),
        tf_bond_s(new GLWTextField(NULL,5)),
        sl_bond_s(new GLWSlider(1,5,1,0)),
        bn_bond_a(new GLWButton("Add Bond")),
        bn_bond_d(new GLWButton("Delete Bond")),
#endif
        tf_minx(new GLWTextField(NULL,5)),
	cb_projt_ortho("Orthographic", 0, &this->cg_projt)
{
    GLWLabel     *lb_file;
    GLWLabel     *lb_slice_t;
    GLWLabel     *lb_slice_p;
    GLWLabel     *lb_slice_d;
    GLWLabel     *lb_iso_v;
    GLWLabel     *lb_iso_d;
    GLWLabel     *lb_point_r;
    GLWLabel     *lb_point_s;
# if defined(__DS3_ADD_BONDS__)
    GLWLabel     *lb_bond_f;
    GLWLabel     *lb_bond_t;
    GLWLabel     *lb_bond_s;
# endif
    GLWLabel     *lb_minx;
    GLWLabel     *lb_maxx;
    GLWLabel     *lb_miny;
    GLWLabel     *lb_maxy;
    GLWLabel     *lb_minz;
    GLWLabel     *lb_maxz;
    GLWLabel     *lb_ornt;
    GLWLabel     *lb_ornt_y;
    GLWLabel     *lb_ornt_p;
    GLWLabel     *lb_ornt_r;
    GLWLabel     *lb_zoom;
    GLWLabel     *lb_cntr;
    GLWLabel     *lb_cntr_x;
    GLWLabel     *lb_cntr_y;
    GLWLabel     *lb_cntr_z;
    GLWLabel     *lb_scale;
    GLWLabel     *lb_color;
    GLWLabel     *lb_backc;
    GLWLabel     *lb_projt;
    GLWButton    *bn_ornt;
    GLWButton    *bn_cntr;
    GLWButton    *bn_align;
    GLWComponent *cm_vals;
    GLWComponent *cm_data;
    GLWComponent *cm_strc;
    GLWComponent *cm_point_btns;
# if defined(__DS3_ADD_BONDS__)
    GLWComponent *cm_bond_btns;
# endif
    GLWComponent *cm_bnds;
    GLWComponent *cm_view;
    GLWComponent *cm_view_btns;
    GLWComponent *cm_opts;

    DS3Viewer* _this = this;
    _this->read_id=0;
    _this->ds3 = new DataSet3D();
    dsLinearScaleInit(&_this->scale_linear, _this->ds3->min, _this->ds3->max);
    dsLogScaleInit(&_this->scale_log, _this->ds3->min, _this->ds3->max);
    cm_vals = new GLWComponent();
    lb_file = new GLWLabel("Open File:");
    cm_data = new GLWComponent();
    cm_strc = new GLWComponent();
    cm_point_btns = new GLWComponent();
# if defined(__DS3_ADD_BONDS__)
    cm_bond_btns = new GLWComponent();
# endif
    cm_bnds = new GLWComponent();
    cm_view = new GLWComponent();
    cm_view_btns = new GLWComponent();
    cm_opts = new GLWComponent();
    _this->sl_slice_t = new GLWSlider(0,360,0,0);
    lb_slice_p = new GLWLabel("Polar slice angle:");
    _this->sl_slice_p= new GLWSlider(0,360,0,0);
    lb_slice_d = new GLWLabel("Slice offset:");
    _this->sl_slice_d= new GLWSlider(-260,260,0,0);
    lb_slice_t = new GLWLabel("Azimuthal slice angle:");
    lb_iso_v= new GLWLabel("Iso-surface value:");
    _this->sl_iso_v= new GLWSlider(0,DS3V_ISO_V_RES,DS3V_ISO_V_RES>>1,0);
    lb_iso_d = new GLWLabel("Iso-surface detail:");
    /*_this->tf_iso_d=glwTextFieldAlloc(NULL,5);*/
    _this->sl_iso_d= new GLWSlider(4,1,2,0);
    lb_point_r = new GLWLabel("Atom radius:");
    _this->sl_point_r= new GLWSlider(0,100,30,0);
    lb_point_s = new GLWLabel("Current Atom:");
# if defined(__DS3_ADD_BONDS__)
    lb_bond_f = new GLWLabel("Bond from:");
    lb_bond_t = new GLWLabel("Bond to:");
    lb_bond_s = new GLWLabel("Bond size:");
# endif
    lb_minx = new GLWLabel("Minimum X:");
    _this->sl_minx= new GLWSlider(-100,200,0,0);
    lb_maxx = new GLWLabel("Maximum X:");
    _this->tf_maxx = new GLWTextField(NULL,5);
    _this->sl_maxx= new GLWSlider(-100,200,100,0);
    lb_miny= new GLWLabel("Minimum Y:");
    _this->tf_miny = new GLWTextField(NULL,5);
    _this->sl_miny= new GLWSlider(-100,200,0,0);
    lb_maxy = new GLWLabel("Maximum Y:");
    _this->tf_maxy = new GLWTextField(NULL,5);
    _this->sl_maxy= new GLWSlider(-100,200,100,0);
    lb_minz = new GLWLabel("Minimum Z:");
    _this->tf_minz = new GLWTextField(NULL,5);
    _this->sl_minz= new GLWSlider(-100,200,0,0);
    lb_maxz = new GLWLabel("Maximum Z:");
    _this->tf_maxz = new GLWTextField(NULL,5);
    _this->sl_maxz= new GLWSlider(-100,200,100,0);
    _this->cb_draw_coords = new GLWCheckBox("Draw Coordinate System",1,NULL);
    lb_zoom = new GLWLabel("Zoom:");
    _this->tf_zoom = new GLWTextField(NULL,5);
    _this->sl_zoom= new GLWSlider(0,200,100,0);
    lb_ornt = new GLWLabel("Orientation:");
    lb_ornt_y = new GLWLabel("Yaw:");
    _this->tf_ornt_y = new GLWTextField(NULL,5);
    _this->sl_ornt_y= new GLWSlider(0,360,0,0);
    lb_ornt_p = new GLWLabel("Pitch:");
    _this->tf_ornt_p = new GLWTextField(NULL,5);
    _this->sl_ornt_p= new GLWSlider(0,360,0,0);
    lb_ornt_r = new GLWLabel("Roll:");
    _this->tf_ornt_r = new GLWTextField(NULL,5);
    _this->sl_ornt_r=new GLWSlider(0,360,0,0);
    lb_cntr = new GLWLabel("Look at:");
    lb_cntr_x = new GLWLabel("X:");
    _this->tf_cntr_x = new GLWTextField(NULL,5);
    _this->sl_cntr_x=new GLWSlider(-100,200,50,0);
    lb_cntr_y = new GLWLabel("Y:");
    _this->tf_cntr_y = new GLWTextField(NULL,5);
    _this->sl_cntr_y=new GLWSlider(-100,200,50,0);
    lb_cntr_z = new GLWLabel("Z:");
    _this->tf_cntr_z = new GLWTextField(NULL,5);
    _this->sl_cntr_z=new GLWSlider(-100,200,50,0);
    bn_ornt = new GLWButton("Reset Orientation");
    bn_cntr = new GLWButton("Reset Position");
    bn_align = new GLWButton("Align to Slice");
    lb_scale = new GLWLabel("Data scale type:");
    _this->cb_scale_linear= new GLWCheckBox("Linear",1,&_this->cg_scale);
    _this->cb_scale_log= new GLWCheckBox("Logarithmic",0,&_this->cg_scale);
    lb_color= new GLWLabel("Color scale type:");
    _this->cb_color_rainbow= new GLWCheckBox("Rainbow",1,&_this->cg_color);
    _this->cb_color_grayscale= new GLWCheckBox("Gray scale",0,&_this->cg_color);
    lb_backc= new GLWLabel("Background color:");
    _this->cb_backc_black= new GLWCheckBox("Black",1,&_this->cg_backc);
    _this->cb_backc_white= new GLWCheckBox("White",0,&_this->cg_backc);
    lb_projt= new GLWLabel("Projection type:");
    _this->cb_projt_persp= new GLWCheckBox("Perspective",1,&_this->cg_projt);

    if (_this->frame!=NULL&&_this->ds3view!=NULL&&cm_vals!=NULL&&lb_file!=NULL&&
            _this->lb_data_set!=NULL&&
            cm_data!=NULL&&cm_strc!=NULL&&cm_point_btns!=NULL&&
# if defined(__DS3_ADD_BONDS__)
            cm_bond_btns!=NULL&&
# endif
            cm_bnds!=NULL&&cm_view!=NULL&&cm_view_btns!=NULL&&cm_opts!=NULL&&
            _this->tp_ctrl!=NULL&&_this->legend!=NULL&&_this->lb_status!=NULL&&
            _this->lb_datax!=NULL&&_this->lb_datay!=NULL&&_this->lb_dataz!=NULL&&
            _this->lb_datav!=NULL&&_this->cb_draw_slice!=NULL&&lb_slice_t!=NULL&&
            _this->tf_slice_t!=NULL&&_this->sl_slice_t != NULL&&lb_slice_p!=NULL&&
            _this->tf_slice_p!=NULL&&_this->sl_slice_p!=NULL&&lb_slice_d!=NULL&&
            _this->tf_slice_d!=NULL&&_this->sl_slice_d!=NULL&&
            _this->cb_draw_iso!=NULL&&lb_iso_v!=NULL&&_this->tf_iso_v!=NULL&&
            _this->sl_iso_v!=NULL&&lb_iso_d!=NULL&&/*_this->tf_iso_d!=NULL&&*/
            _this->sl_iso_d!=NULL&&_this->cb_draw_points!=NULL&&lb_point_r!=NULL&&
            _this->tf_point_r!=NULL&&_this->sl_point_r!=NULL&&lb_point_s!=NULL&&
            _this->tf_point_s!=NULL&&_this->lb_point_t!=NULL&&
            _this->lb_point_l!=NULL&&_this->bn_point_c!=NULL&&
            _this->bn_point_v!=NULL&&_this->bn_point_sa!=NULL&&
# if defined(__DS3_ADD_BONDS__)
            lb_bond_f!=NULL&&_this->tf_bond_f!=NULL&&lb_bond_t!=NULL&&
            _this->tf_bond_t!=NULL&&lb_bond_s!=NULL&&_this->tf_bond_s!=NULL&&
            _this->sl_bond_s!=NULL&&_this->bn_bond_a!=NULL&&_this->bn_bond_d!=NULL&&
# endif
            lb_minx!=NULL&&_this->tf_minx!=NULL&&_this->sl_minx!=NULL&&lb_maxx!=NULL&&
            _this->tf_maxx!=NULL&&_this->sl_maxx!=NULL&&lb_miny!=NULL&&
            _this->tf_miny!=NULL&&_this->sl_miny!=NULL&&lb_maxy!=NULL&&
            _this->tf_maxy!=NULL&&_this->sl_maxy!=NULL&&lb_minz!=NULL&&
            _this->tf_minz!=NULL&&_this->sl_minz!=NULL&&lb_maxz!=NULL&&
            _this->tf_maxz!=NULL&&_this->sl_maxz!=NULL&&_this->cb_draw_coords!=NULL&&
            lb_zoom!=NULL&&_this->tf_zoom!=NULL&&_this->sl_zoom!=NULL&&
            lb_ornt!=NULL&&lb_ornt_y!=NULL&&_this->tf_ornt_y!=NULL&&
            _this->sl_ornt_y!=NULL&&lb_ornt_p!=NULL&&_this->tf_ornt_p!=NULL&&
            _this->sl_ornt_p!=NULL&&lb_ornt_r!=NULL&&_this->tf_ornt_r!=NULL&&
            _this->sl_ornt_r!=NULL&&lb_cntr!=NULL&&lb_cntr_x!=NULL&&
            _this->tf_cntr_x!=NULL&&_this->sl_cntr_x!=NULL&&lb_cntr_y!=NULL&&
            _this->tf_cntr_y!=NULL&&_this->sl_cntr_y!=NULL&&lb_cntr_z!=NULL&&
            _this->tf_cntr_z!=NULL&&_this->sl_cntr_z!=NULL&&bn_ornt!=NULL&&
            bn_cntr!=NULL&&bn_align!=NULL&&lb_scale!=NULL&&
            _this->cb_scale_linear!=NULL&&_this->cb_scale_log!=NULL&&lb_color!=NULL&&
            _this->cb_color_rainbow!=NULL&&_this->cb_color_grayscale!=NULL&&
            lb_backc!=NULL&&_this->cb_backc_black!=NULL&&_this->cb_backc_white!=NULL&&
            lb_projt!=NULL&&_this->cb_projt_persp!=NULL) {
        int i;
        ds3ViewSetDataScale(_this->ds3view,&_this->scale_linear.super);
        glwButtonSetPressedFunc(&_this->bn_open,(GLWActionFunc)ds3ViewerOpen);
        glwButtonSetPressedCtx(&_this->bn_open,_this);
        glwButtonSetPressedFunc(_this->bn_point_c,
                                (GLWActionFunc)ds3ViewerPointCenter);
        glwButtonSetPressedCtx(_this->bn_point_c,_this);
        glwButtonSetPressedFunc(_this->bn_point_v,
                                (GLWActionFunc)ds3ViewerPointVisible);
        glwButtonSetPressedCtx(_this->bn_point_v,_this);
        glwButtonSetPressedFunc(_this->bn_point_sa,
                                (GLWActionFunc)ds3ViewerPointShowAll);
        glwButtonSetPressedCtx(_this->bn_point_sa,_this);
        glwButtonSetPressedFunc(bn_ornt,(GLWActionFunc)ds3ViewerResetOrientation);
        glwButtonSetPressedCtx(bn_ornt,_this);
        glwButtonSetPressedFunc(bn_cntr,(GLWActionFunc)ds3ViewerResetPosition);
        glwButtonSetPressedCtx(bn_cntr,_this);
        glwButtonSetPressedFunc(bn_align,(GLWActionFunc)ds3ViewerAlignOrientation);
        glwButtonSetPressedCtx(bn_align,_this);
# if defined(__DS3_ADD_BONDS__)
        glwButtonSetPressedFunc(_this->bn_bond_a,(GLWActionFunc)ds3ViewerBondAdd);
        glwButtonSetPressedCtx(_this->bn_bond_a,_this);
        glwButtonSetPressedFunc(_this->bn_bond_d,(GLWActionFunc)ds3ViewerBondDel);
        glwButtonSetPressedCtx(_this->bn_bond_d,_this);
# endif
        ds3ViewSetDataChangedFunc(_this->ds3view,
                                  (GLWActionFunc)ds3ViewerViewDataChanged);
        ds3ViewSetDataChangedCtx(_this->ds3view,_this);
        ds3ViewSetSliceChangedFunc(_this->ds3view,
                                   (GLWActionFunc)ds3ViewerViewSliceChanged);
        ds3ViewSetSliceChangedCtx(_this->ds3view,_this);
        ds3ViewSetOrientationChangedFunc(_this->ds3view,(GLWActionFunc)
                                         ds3ViewerViewOrientationChanged);
        ds3ViewSetOrientationChangedCtx(_this->ds3view,_this);
        ds3ViewSetZoomChangedFunc(_this->ds3view,
                                  (GLWActionFunc)ds3ViewerViewZoomChanged);
        ds3ViewSetZoomChangedCtx(_this->ds3view,_this);
        ds3ViewSetCenterChangedFunc(_this->ds3view,
                                    (GLWActionFunc)ds3ViewerViewCenterChanged);
        ds3ViewSetCenterChangedCtx(_this->ds3view,_this);
        ds3ViewSetBoxChangedFunc(_this->ds3view,
                                 (GLWActionFunc)ds3ViewerViewBoxChanged);
        ds3ViewSetBoxChangedCtx(_this->ds3view,_this);
        ds3ViewSetPointChangedFunc(_this->ds3view,
                                   (GLWActionFunc)ds3ViewerViewPointChanged);
        ds3ViewSetPointChangedCtx(_this->ds3view,_this);
# if defined(__DS3_ADD_BONDS__)
        ds3ViewSetBondChangedFunc(_this->ds3view,
                                  (GLWActionFunc)ds3ViewerViewBondChanged);
        ds3ViewSetBondChangedCtx(_this->ds3view,_this);
#  if defined(__DS3_SAVE_BONDS__)
        ds3ViewSetBondsChangedFunc(_this->ds3view,
                                   (GLWActionFunc)ds3ViewerSaveBonds);
        ds3ViewSetBondsChangedCtx(_this->ds3view,_this);
#  endif
# endif
        glwCheckBoxSetChangedFunc(_this->cb_draw_coords,
                                  (GLWActionFunc)ds3ViewerDrawCoordSChanged);
        glwCheckBoxSetChangedCtx(_this->cb_draw_coords,_this);
        glwCheckBoxSetChangedFunc(_this->cb_draw_points,
                                  (GLWActionFunc)ds3ViewerDrawPointsChanged);
        glwCheckBoxSetChangedCtx(_this->cb_draw_points,_this);
        glwCheckBoxSetChangedFunc(_this->cb_draw_slice,
                                  (GLWActionFunc)ds3ViewerDrawSliceChanged);
        glwCheckBoxSetChangedCtx(_this->cb_draw_slice,_this);
        glwCheckBoxSetChangedFunc(_this->cb_draw_iso,
                                  (GLWActionFunc)ds3ViewerDrawIsoChanged);
        glwCheckBoxSetChangedCtx(_this->cb_draw_iso,_this);
        glwCheckBoxGroupSetChangedFunc(&_this->cg_scale,
                                       (GLWActionFunc)ds3ViewerScaleChanged);
        glwCheckBoxGroupSetChangedCtx(&_this->cg_scale,_this);
        glwCheckBoxGroupSetChangedFunc(&_this->cg_color,
                                       (GLWActionFunc)ds3ViewerColorChanged);
        glwCheckBoxGroupSetChangedCtx(&_this->cg_color,_this);
        glwCheckBoxGroupSetChangedFunc(&_this->cg_backc,
                                       (GLWActionFunc)ds3ViewerBackCChanged);
        glwCheckBoxGroupSetChangedCtx(&_this->cg_backc,_this);
        glwCheckBoxGroupSetChangedFunc(&_this->cg_projt,
                                       (GLWActionFunc)ds3ViewerProjTChanged);
        glwCheckBoxGroupSetChangedCtx(&_this->cg_projt,_this);
        glwTextFieldSetActionFunc(&_this->tf_file, 
				  (GLWActionFunc)ds3ViewerOpen);
        glwTextFieldSetActionCtx(&_this->tf_file, _this);
        glwTextFieldSetChangedFunc(&_this->tf_file,
                                   (GLWActionFunc)ds3ViewerFileTextChanged);
        glwTextFieldSetChangedCtx(&_this->tf_file, _this);
        glwTextFieldSetActionFunc(_this->tf_slice_t,
                                  (GLWActionFunc)ds3ViewerSliceTextChanged);
        glwTextFieldSetActionCtx(_this->tf_slice_t,_this);
        glwTextFieldSetChangedFunc(_this->tf_slice_t,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_slice_t,_this);
        glwTextFieldSetActionFunc(_this->tf_slice_p,
                                  (GLWActionFunc)ds3ViewerSliceTextChanged);
        glwTextFieldSetActionCtx(_this->tf_slice_p,_this);
        glwTextFieldSetChangedFunc(_this->tf_slice_p,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_slice_p,_this);
        glwTextFieldSetActionFunc(_this->tf_slice_d,
                                  (GLWActionFunc)ds3ViewerSliceTextChanged);
        glwTextFieldSetActionCtx(_this->tf_slice_d,_this);
        glwTextFieldSetChangedFunc(_this->tf_slice_d,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_slice_d,_this);
        glwTextFieldSetActionFunc(_this->tf_iso_v,
                                  (GLWActionFunc)ds3ViewerIsoVTextChanged);
        glwTextFieldSetActionCtx(_this->tf_iso_v,_this);
        glwTextFieldSetChangedFunc(_this->tf_iso_v,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_iso_v,_this);
        /*glwTextFieldSetActionFunc(_this->tf_iso_d,
                                   (GLWActionFunc)ds3ViewerIsoDTextChanged);
        glwTextFieldSetActionCtx(_this->tf_iso_d,_this);
        glwTextFieldSetChangedFunc(_this->tf_iso_d,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_iso_d,_this);*/
        glwTextFieldSetActionFunc(_this->tf_point_r,
                                  (GLWActionFunc)ds3ViewerPointRTextChanged);
        glwTextFieldSetActionCtx(_this->tf_point_r,_this);
        glwTextFieldSetChangedFunc(_this->tf_point_r,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_point_r,_this);
        glwTextFieldSetActionFunc(_this->tf_point_s,
                                  (GLWActionFunc)ds3ViewerPointSTextChanged);
        glwTextFieldSetActionCtx(_this->tf_point_s,_this);
        glwTextFieldSetChangedFunc(_this->tf_point_s,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_point_s,_this);
# if defined(__DS3_ADD_BONDS__)
        glwTextFieldSetActionFunc(_this->tf_bond_f,
                                  (GLWActionFunc)ds3ViewerBondFTextChanged);
        glwTextFieldSetActionCtx(_this->tf_bond_f,_this);
        glwTextFieldSetChangedFunc(_this->tf_bond_f,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_bond_f,_this);
        glwTextFieldSetActionFunc(_this->tf_bond_t,
                                  (GLWActionFunc)ds3ViewerBondTTextChanged);
        glwTextFieldSetActionCtx(_this->tf_bond_t,_this);
        glwTextFieldSetChangedFunc(_this->tf_bond_t,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_bond_t,_this);
        glwTextFieldSetActionFunc(_this->tf_bond_s,
                                  (GLWActionFunc)ds3ViewerBondSTextChanged);
        glwTextFieldSetActionCtx(_this->tf_bond_s,_this);
        glwTextFieldSetChangedFunc(_this->tf_bond_s,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_bond_s,_this);
# endif
        glwTextFieldSetActionFunc(_this->tf_minx,
                                  (GLWActionFunc)ds3ViewerBoxTextChanged);
        glwTextFieldSetActionCtx(_this->tf_minx,_this);
        glwTextFieldSetChangedFunc(_this->tf_minx,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_minx,_this);
        glwTextFieldSetActionFunc(_this->tf_miny,
                                  (GLWActionFunc)ds3ViewerBoxTextChanged);
        glwTextFieldSetActionCtx(_this->tf_miny,_this);
        glwTextFieldSetChangedFunc(_this->tf_miny,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_miny,_this);
        glwTextFieldSetActionFunc(_this->tf_minz,
                                  (GLWActionFunc)ds3ViewerBoxTextChanged);
        glwTextFieldSetActionCtx(_this->tf_minz,_this);
        glwTextFieldSetChangedFunc(_this->tf_minz,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_minz,_this);
        glwTextFieldSetActionFunc(_this->tf_maxx,
                                  (GLWActionFunc)ds3ViewerBoxTextChanged);
        glwTextFieldSetActionCtx(_this->tf_maxx,_this);
        glwTextFieldSetChangedFunc(_this->tf_maxx,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_maxx,_this);
        glwTextFieldSetActionFunc(_this->tf_maxy,
                                  (GLWActionFunc)ds3ViewerBoxTextChanged);
        glwTextFieldSetActionCtx(_this->tf_maxy,_this);
        glwTextFieldSetChangedFunc(_this->tf_maxy,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_maxy,_this);
        glwTextFieldSetActionFunc(_this->tf_maxz,
                                  (GLWActionFunc)ds3ViewerBoxTextChanged);
        glwTextFieldSetActionCtx(_this->tf_maxz,_this);
        glwTextFieldSetChangedFunc(_this->tf_maxz,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_maxz,_this);
        glwTextFieldSetActionFunc(_this->tf_zoom,
                                  (GLWActionFunc)ds3ViewerZoomTextChanged);
        glwTextFieldSetActionCtx(_this->tf_zoom,_this);
        glwTextFieldSetChangedFunc(_this->tf_zoom,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_zoom,_this);
        glwTextFieldSetActionFunc(_this->tf_ornt_y,
                                  (GLWActionFunc)ds3ViewerOrntTextChanged);
        glwTextFieldSetActionCtx(_this->tf_ornt_y,_this);
        glwTextFieldSetChangedFunc(_this->tf_ornt_y,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_ornt_y,_this);
        glwTextFieldSetActionFunc(_this->tf_ornt_p,
                                  (GLWActionFunc)ds3ViewerOrntTextChanged);
        glwTextFieldSetActionCtx(_this->tf_ornt_p,_this);
        glwTextFieldSetChangedFunc(_this->tf_ornt_p,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_ornt_p,_this);
        glwTextFieldSetActionFunc(_this->tf_ornt_r,
                                  (GLWActionFunc)ds3ViewerOrntTextChanged);
        glwTextFieldSetActionCtx(_this->tf_ornt_r,_this);
        glwTextFieldSetChangedFunc(_this->tf_ornt_r,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_ornt_r,_this);
        glwTextFieldSetActionFunc(_this->tf_cntr_x,
                                  (GLWActionFunc)ds3ViewerCntrTextChanged);
        glwTextFieldSetActionCtx(_this->tf_cntr_x,_this);
        glwTextFieldSetChangedFunc(_this->tf_cntr_x,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_cntr_x,_this);
        glwTextFieldSetActionFunc(_this->tf_cntr_y,
                                  (GLWActionFunc)ds3ViewerCntrTextChanged);
        glwTextFieldSetActionCtx(_this->tf_cntr_y,_this);
        glwTextFieldSetChangedFunc(_this->tf_cntr_y,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_cntr_y,_this);
        glwTextFieldSetActionFunc(_this->tf_cntr_z,
                                  (GLWActionFunc)ds3ViewerCntrTextChanged);
        glwTextFieldSetActionCtx(_this->tf_cntr_z,_this);
        glwTextFieldSetChangedFunc(_this->tf_cntr_z,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        glwTextFieldSetChangedCtx(_this->tf_cntr_z,_this);
        glwSliderSetChangedFunc(_this->sl_point_r,
                                (GLWActionFunc)ds3ViewerPointRSliderChanged);
        glwSliderSetChangedCtx(_this->sl_point_r,_this);
        glwSliderSetMajorTickSpacing(_this->sl_point_r,10);
        glwSliderSetMinorTickSpacing(_this->sl_point_r,5);
        ds3ViewerUpdatePointRLabels(_this);
        glwSliderSetSnap(_this->sl_point_r,1);
        _this->sl_slice_t->setChangedFunc((GLWActionFunc)ds3ViewerSliceTSliderChanged);
        _this->sl_slice_t->setChangedCtx(_this);
        _this->sl_slice_t->setMajorTickSpacing(90);
        _this->sl_slice_t->setMinorTickSpacing(15);
        _this->sl_slice_t->makeLabels(0, 90);
        _this->sl_slice_t->setSnap(1);
        glwSliderSetChangedFunc(_this->sl_slice_p,
                                (GLWActionFunc)ds3ViewerSlicePSliderChanged);
        glwSliderSetChangedCtx(_this->sl_slice_p,_this);
        glwSliderSetMajorTickSpacing(_this->sl_slice_p,90);
        glwSliderSetMinorTickSpacing(_this->sl_slice_p,15);
        glwSliderMakeLabels(_this->sl_slice_p,0,90);
        glwSliderSetSnap(_this->sl_slice_p,1);
        glwSliderSetChangedFunc(_this->sl_slice_d,
                                (GLWActionFunc)ds3ViewerSliceDSliderChanged);
        glwSliderSetChangedCtx(_this->sl_slice_d,_this);
        glwSliderSetMajorTickSpacing(_this->sl_slice_d,100);
        glwSliderSetMajorTickOffset(_this->sl_slice_d,60);
        glwSliderSetMinorTickSpacing(_this->sl_slice_d,25);
        glwSliderSetMinorTickOffset(_this->sl_slice_d,10);
        for (i=-2; i<=2; i++) {
            char text[32];
            sprintf(text,"%i",i);
            glwSliderAddLabel(_this->sl_slice_d,i*100,text);
        }
        glwSliderSetSnap(_this->sl_slice_d,1);
        glwSliderSetChangedFunc(_this->sl_iso_v,
                                (GLWActionFunc)ds3ViewerIsoVSliderChanged);
        glwSliderSetChangedCtx(_this->sl_iso_v,_this);
        glwSliderSetMajorTickSpacing(_this->sl_iso_v,DS3V_ISO_V_RES/10);
        glwSliderSetMinorTickSpacing(_this->sl_iso_v,DS3V_ISO_V_RES/40);
        ds3ViewerUpdateIsoVLabels(_this);
        glwSliderSetSnap(_this->sl_slice_p,1);
        glwSliderSetChangedFunc(_this->sl_iso_d,
                                (GLWActionFunc)ds3ViewerIsoDSliderChanged);
        glwSliderSetChangedCtx(_this->sl_iso_d,_this);
        glwSliderSetMajorTickSpacing(_this->sl_iso_d,1);
        glwSliderAddLabel(_this->sl_iso_d,1,"High");
        glwSliderAddLabel(_this->sl_iso_d,4,"Low");
        glwSliderSetSnap(_this->sl_iso_d,GLWC_SNAP_ALWAYS);
# if defined(__DS3_ADD_BONDS__)
        glwSliderSetChangedFunc(_this->sl_bond_s,
                                (GLWActionFunc)ds3ViewerBondSSliderChanged);
        glwSliderSetChangedCtx(_this->sl_bond_s,_this);
        glwSliderSetMajorTickSpacing(_this->sl_bond_s,1);
        glwSliderMakeLabels(_this->sl_bond_s,1,1);
        glwSliderSetSnap(_this->sl_bond_s,GLWC_SNAP_ALWAYS);
# endif
        glwSliderSetChangedFunc(_this->sl_minx,
                                (GLWActionFunc)ds3ViewerMinXSliderChanged);
        glwSliderSetChangedCtx(_this->sl_minx,_this);
        glwSliderSetMajorTickSpacing(_this->sl_minx,50);
        glwSliderSetMinorTickSpacing(_this->sl_minx,10);
        glwSliderSetSnap(_this->sl_minx,1);
        glwSliderSetChangedFunc(_this->sl_miny,
                                (GLWActionFunc)ds3ViewerMinYSliderChanged);
        glwSliderSetChangedCtx(_this->sl_miny,_this);
        glwSliderSetMajorTickSpacing(_this->sl_miny,50);
        glwSliderSetMinorTickSpacing(_this->sl_miny,10);
        glwSliderSetSnap(_this->sl_miny,1);
        glwSliderSetChangedFunc(_this->sl_minz,
                                (GLWActionFunc)ds3ViewerMinZSliderChanged);
        glwSliderSetChangedCtx(_this->sl_minz,_this);
        glwSliderSetMajorTickSpacing(_this->sl_minz,50);
        glwSliderSetMinorTickSpacing(_this->sl_minz,10);
        glwSliderSetSnap(_this->sl_minz,1);
        glwSliderSetChangedFunc(_this->sl_maxx,
                                (GLWActionFunc)ds3ViewerMaxXSliderChanged);
        glwSliderSetChangedCtx(_this->sl_maxx,_this);
        glwSliderSetMajorTickSpacing(_this->sl_maxx,50);
        glwSliderSetMinorTickSpacing(_this->sl_maxx,10);
        glwSliderSetSnap(_this->sl_maxx,1);
        glwSliderSetChangedFunc(_this->sl_maxy,
                                (GLWActionFunc)ds3ViewerMaxYSliderChanged);
        glwSliderSetChangedCtx(_this->sl_maxy,_this);
        glwSliderSetMajorTickSpacing(_this->sl_maxy,50);
        glwSliderSetMinorTickSpacing(_this->sl_maxy,10);
        glwSliderSetSnap(_this->sl_maxy,1);
        glwSliderSetChangedFunc(_this->sl_maxz,
                                (GLWActionFunc)ds3ViewerMaxZSliderChanged);
        glwSliderSetChangedCtx(_this->sl_maxz,_this);
        glwSliderSetMajorTickSpacing(_this->sl_maxz,50);
        glwSliderSetMinorTickSpacing(_this->sl_maxz,10);
        glwSliderSetSnap(_this->sl_maxz,1);
        for (i=-1; i<=2; i++) {
            char text[32];
            sprintf(text,"%i",i);
            glwSliderAddLabel(_this->sl_minx,i*100,text);
            glwSliderAddLabel(_this->sl_miny,i*100,text);
            glwSliderAddLabel(_this->sl_minz,i*100,text);
            glwSliderAddLabel(_this->sl_maxx,i*100,text);
            glwSliderAddLabel(_this->sl_maxy,i*100,text);
            glwSliderAddLabel(_this->sl_maxz,i*100,text);
        }
        glwSliderSetChangedFunc(_this->sl_zoom,
                                (GLWActionFunc)ds3ViewerZoomSliderChanged);
        glwSliderSetChangedCtx(_this->sl_zoom,_this);
        glwSliderSetMajorTickSpacing(_this->sl_zoom,50);
        glwSliderSetMinorTickSpacing(_this->sl_zoom,10);
        /*ds3ViewerUpdateZoomLabels(_this);*/
        for (i=0; i<=2; i++) {
            char text[32];
            sprintf(text,"%i",i);
            glwSliderAddLabel(_this->sl_zoom,i*100,text);
        }
        glwSliderSetSnap(_this->sl_zoom,1);
        glwSliderSetChangedFunc(_this->sl_ornt_y,
                                (GLWActionFunc)ds3ViewerOrntYSliderChanged);
        glwSliderSetChangedCtx(_this->sl_ornt_y,_this);
        glwSliderMakeLabels(_this->sl_ornt_y,0,360);
        glwSliderSetMajorTickSpacing(_this->sl_ornt_y,180);
        glwSliderSetMinorTickSpacing(_this->sl_ornt_y,30);
        glwSliderSetSnap(_this->sl_ornt_y,1);
        glwSliderSetChangedFunc(_this->sl_ornt_p,
                                (GLWActionFunc)ds3ViewerOrntPSliderChanged);
        glwSliderSetChangedCtx(_this->sl_ornt_p,_this);
        glwSliderMakeLabels(_this->sl_ornt_p,0,360);
        glwSliderSetMajorTickSpacing(_this->sl_ornt_p,180);
        glwSliderSetMinorTickSpacing(_this->sl_ornt_p,30);
        glwSliderSetSnap(_this->sl_ornt_p,1);
        glwSliderSetChangedFunc(_this->sl_ornt_r,
                                (GLWActionFunc)ds3ViewerOrntRSliderChanged);
        glwSliderSetChangedCtx(_this->sl_ornt_r,_this);
        glwSliderMakeLabels(_this->sl_ornt_r,0,360);
        glwSliderSetMajorTickSpacing(_this->sl_ornt_r,180);
        glwSliderSetMinorTickSpacing(_this->sl_ornt_r,30);
        glwSliderSetSnap(_this->sl_ornt_r,1);
        for (i=-1; i<=2; i++) {
            char text[32];
            sprintf(text,"%i",i);
            glwSliderAddLabel(_this->sl_cntr_x,i*100,text);
            glwSliderAddLabel(_this->sl_cntr_y,i*100,text);
            glwSliderAddLabel(_this->sl_cntr_z,i*100,text);
        }
        glwSliderSetChangedFunc(_this->sl_cntr_x,
                                (GLWActionFunc)ds3ViewerCntrXSliderChanged);
        glwSliderSetChangedCtx(_this->sl_cntr_x,_this);
        glwSliderSetMajorTickSpacing(_this->sl_cntr_x,100);
        glwSliderSetMinorTickSpacing(_this->sl_cntr_x,25);
        glwSliderSetSnap(_this->sl_cntr_x,1);
        glwSliderSetChangedFunc(_this->sl_cntr_y,
                                (GLWActionFunc)ds3ViewerCntrYSliderChanged);
        glwSliderSetChangedCtx(_this->sl_cntr_y,_this);
        glwSliderSetMajorTickSpacing(_this->sl_cntr_y,100);
        glwSliderSetMinorTickSpacing(_this->sl_cntr_y,25);
        glwSliderSetSnap(_this->sl_cntr_y,1);
        glwSliderSetChangedFunc(_this->sl_cntr_z,
                                (GLWActionFunc)ds3ViewerCntrZSliderChanged);
        glwSliderSetChangedCtx(_this->sl_cntr_z,_this);
        glwSliderSetMajorTickSpacing(_this->sl_cntr_z,100);
        glwSliderSetMinorTickSpacing(_this->sl_cntr_z,25);
        glwSliderSetSnap(_this->sl_cntr_z,1);
        glwCompSetLayout(&_this->frame->super, &(new GLWGridBagLayout())->super);
        glwCompSetMinWidth(&_this->ds3view->super,360);
        glwCompSetMinHeight(&_this->ds3view->super,360);
        glwCompSetInsets(&_this->ds3view->super,2,2,2,2);
        glwCompSetGridHeight(&_this->ds3view->super,4);
        glwCompSetFill(&_this->ds3view->super,GLWC_BOTH);
        glwCompSetWeightX(&_this->ds3view->super,1);
        glwCompSetWeightY(&_this->ds3view->super,1);
        glwCompSetInsets(&lb_file->super,2,2,2,2);
        glwCompSetGridWidth(&lb_file->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->tf_file.super, 2, 2, 2, 2);
        glwCompSetFill(&_this->tf_file.super, GLWC_HORIZONTAL);
        glwCompSetInsets(&_this->bn_open.super, 2, 2, 2, 2);
        glwCompSetGridWidth(&_this->bn_open.super, GLWC_REMAINDER);
        glwCompSetInsets(&_this->lb_data_set->super,0,0,2,2);
        glwCompSetGridWidth(&_this->lb_data_set->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->lb_data_set->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&_this->tp_ctrl->super,2,2,2,2);
        glwCompSetGridWidth(&_this->tp_ctrl->super,GLWC_REMAINDER);
        glwCompSetGridHeight(&_this->tp_ctrl->super,3);
        glwCompSetFill(&_this->tp_ctrl->super,GLWC_BOTH);
        glwCompSetInsets(&_this->legend->super,2,2,2,2);
        glwCompSetFill(&_this->legend->super,GLWC_HORIZONTAL);
        glwCompSetGridX(&_this->legend->super,0);
        glwCompSetGridY(&_this->legend->super,4);
        glwCompSetInsets(cm_vals,2,2,2,2);
        glwCompSetFill(cm_vals,GLWC_HORIZONTAL);
        glwCompSetGridX(cm_vals,0);
        glwCompSetGridY(cm_vals,5);
        glwCompSetInsets(&_this->lb_status->super,0,0,2,2);
        glwCompSetGridWidth(&_this->lb_status->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->lb_status->super,GLWC_HORIZONTAL);
        glwCompAdd(&_this->frame->super,&_this->ds3view->super,-1);
        glwCompAdd(&_this->frame->super,&lb_file->super,-1);
        glwCompAdd(&_this->frame->super, &_this->tf_file.super, -1);
        glwCompAdd(&_this->frame->super, &_this->bn_open.super, -1);
        glwCompAdd(&_this->frame->super,&_this->lb_data_set->super,-1);
        glwCompAdd(&_this->frame->super,&_this->tp_ctrl->super,-1);
        glwCompAdd(&_this->frame->super,&_this->legend->super,-1);
        glwCompAdd(&_this->frame->super,cm_vals,-1);
        glwCompAdd(&_this->frame->super,&_this->lb_status->super,-1);
        glwCompSetLayout(cm_vals,&(new GLWGridBagLayout())->super);
        glwCompSetInsets(&_this->lb_datax->super,2,2,2,2);
        glwCompSetPreWidth(&_this->lb_datax->super,80);
        glwCompSetInsets(&_this->lb_datay->super,2,2,2,2);
        glwCompSetPreWidth(&_this->lb_datay->super,80);
        glwCompSetInsets(&_this->lb_dataz->super,2,2,2,2);
        glwCompSetPreWidth(&_this->lb_dataz->super,80);
        glwCompSetInsets(&_this->lb_datav->super,2,2,2,2);
        glwCompSetPreWidth(&_this->lb_datav->super,80);
        glwCompAdd(cm_vals,&_this->lb_datax->super,-1);
        glwCompAdd(cm_vals,&_this->lb_datay->super,-1);
        glwCompAdd(cm_vals,&_this->lb_dataz->super,-1);
        glwCompAdd(cm_vals,&_this->lb_datav->super,-1);
        glwCompSetFill(cm_data,GLWC_HORIZONTAL);
        glwCompSetAlignY(cm_data,1);
        glwCompSetFill(cm_strc,GLWC_HORIZONTAL);
        glwCompSetAlignY(cm_strc,1);
        glwCompSetFill(cm_view,GLWC_HORIZONTAL);
        glwCompSetAlignY(cm_bnds,1);
        glwCompSetFill(cm_bnds,GLWC_HORIZONTAL);
        glwCompSetAlignY(cm_view,1);
        glwCompSetFill(cm_opts,GLWC_HORIZONTAL);
        glwCompSetAlignY(cm_opts,1);
        glwTabbedPaneAdd(_this->tp_ctrl,cm_data,"Data",-1);
        glwTabbedPaneAdd(_this->tp_ctrl,cm_strc,"Atoms",-1);
        glwTabbedPaneAdd(_this->tp_ctrl,cm_bnds,"Bounds",-1);
        glwTabbedPaneAdd(_this->tp_ctrl,cm_view,"View",-1);
        glwTabbedPaneAdd(_this->tp_ctrl,cm_opts,"Options",-1);
        glwCompSetLayout(cm_data,&(new GLWGridBagLayout())->super);
        glwCompSetInsets(&_this->cb_draw_slice->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_draw_slice->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_slice_t->super,0,0,10,2);
        glwCompSetWeightX(&lb_slice_t->super,1);
        glwCompSetInsets(&_this->tf_slice_t->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_slice_t->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_slice_t->super,1);
        glwCompSetInsets(&_this->sl_slice_t->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_slice_t->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_slice_t->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_slice_p->super,0,0,10,2);
        glwCompSetWeightX(&lb_slice_p->super,1);
        glwCompSetInsets(&_this->tf_slice_p->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_slice_p->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_slice_p->super,1);
        glwCompSetInsets(&_this->sl_slice_p->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_slice_p->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_slice_p->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_slice_d->super,0,0,10,2);
        glwCompSetWeightX(&lb_slice_d->super,1);
        glwCompSetInsets(&_this->tf_slice_d->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_slice_d->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_slice_d->super,1);
        glwCompSetInsets(&_this->sl_slice_d->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_slice_d->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_slice_d->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&_this->cb_draw_iso->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_draw_iso->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_iso_v->super,0,0,10,2);
        glwCompSetWeightX(&lb_iso_v->super,1);
        glwCompSetInsets(&_this->tf_iso_v->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_iso_v->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_iso_v->super,1);
        glwCompSetInsets(&_this->sl_iso_v->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_iso_v->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_iso_v->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_iso_d->super,0,0,10,2);
        /*glwCompSetInsets(&_this->tf_iso_d->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_iso_d->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_iso_d->super,1);*/
        glwCompSetGridWidth(&lb_iso_d->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->sl_iso_d->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_iso_d->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_iso_d->super,GLWC_HORIZONTAL);
        glwCompAdd(cm_data,&_this->cb_draw_slice->super,-1);
        glwCompAdd(cm_data,&lb_slice_t->super,-1);
        glwCompAdd(cm_data,&_this->tf_slice_t->super,-1);
        glwCompAdd(cm_data,&_this->sl_slice_t->super,-1);
        glwCompAdd(cm_data,&lb_slice_p->super,-1);
        glwCompAdd(cm_data,&_this->tf_slice_p->super,-1);
        glwCompAdd(cm_data,&_this->sl_slice_p->super,-1);
        glwCompAdd(cm_data,&lb_slice_d->super,-1);
        glwCompAdd(cm_data,&_this->tf_slice_d->super,-1);
        glwCompAdd(cm_data,&_this->sl_slice_d->super,-1);
        glwCompAdd(cm_data,&_this->cb_draw_iso->super,-1);
        glwCompAdd(cm_data,&lb_iso_v->super,-1);
        glwCompAdd(cm_data,&_this->tf_iso_v->super,-1);
        glwCompAdd(cm_data,&_this->sl_iso_v->super,-1);
        glwCompAdd(cm_data,&lb_iso_d->super,-1);
        /*glwCompAdd(cm_data,&_this->tf_iso_d->super,-1);*/
        glwCompAdd(cm_data,&_this->sl_iso_d->super,-1);
        glwCompSetLayout(cm_strc,&(new GLWGridBagLayout())->super);
        glwCompSetInsets(&_this->cb_draw_points->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_draw_points->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_point_r->super,0,0,10,2);
        glwCompSetWeightX(&lb_point_r->super,1);
        glwCompSetInsets(&_this->tf_point_r->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_point_r->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_point_r->super,1);
        glwCompSetInsets(&_this->sl_point_r->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_point_r->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_point_r->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_point_s->super,0,0,10,2);
        glwCompSetWeightX(&lb_point_s->super,1);
        glwCompSetInsets(&_this->tf_point_s->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_point_s->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_point_s->super,1);
        glwCompSetInsets(&_this->lb_point_t->super,2,2,10,2);
        glwCompSetGridWidth(&_this->lb_point_t->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->lb_point_l->super,2,2,10,2);
        glwCompSetGridWidth(&_this->lb_point_l->super,GLWC_REMAINDER);
        glwCompSetInsets(cm_point_btns,2,2,2,2);
        glwCompSetFill(cm_point_btns,GLWC_HORIZONTAL);
        glwCompSetGridWidth(cm_point_btns,GLWC_REMAINDER);
# if defined(__DS3_ADD_BONDS__)
        glwCompSetInsets(&lb_bond_f->super,0,0,10,2);
        glwCompSetWeightX(&lb_bond_f->super,1);
        glwCompSetInsets(&_this->tf_bond_f->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_bond_f->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_bond_f->super,1);
        glwCompSetInsets(&lb_bond_t->super,0,0,10,2);
        glwCompSetWeightX(&lb_bond_t->super,1);
        glwCompSetInsets(&_this->tf_bond_t->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_bond_t->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_bond_t->super,1);
        glwCompSetInsets(&lb_bond_s->super,0,0,10,2);
        glwCompSetWeightX(&lb_bond_s->super,1);
        glwCompSetInsets(&_this->tf_bond_s->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_bond_s->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_bond_s->super,1);
        glwCompSetInsets(&_this->sl_bond_s->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_bond_s->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_bond_s->super,GLWC_HORIZONTAL);
        glwCompSetInsets(cm_bond_btns,2,2,2,2);
        glwCompSetFill(cm_bond_btns,GLWC_HORIZONTAL);
        glwCompSetGridWidth(cm_bond_btns,GLWC_REMAINDER);
# endif
        glwCompAdd(cm_strc,&_this->cb_draw_points->super,-1);
        glwCompAdd(cm_strc,&lb_point_r->super,-1);
        glwCompAdd(cm_strc,&_this->tf_point_r->super,-1);
        glwCompAdd(cm_strc,&_this->sl_point_r->super,-1);
        glwCompAdd(cm_strc,&lb_point_s->super,-1);
        glwCompAdd(cm_strc,&_this->tf_point_s->super,-1);
        glwCompAdd(cm_strc,&_this->lb_point_t->super,-1);
        glwCompAdd(cm_strc,&_this->lb_point_l->super,-1);
        glwCompAdd(cm_strc,cm_point_btns,-1);
# if defined(__DS3_ADD_BONDS__)
        glwCompAdd(cm_strc,&lb_bond_f->super,-1);
        glwCompAdd(cm_strc,&_this->tf_bond_f->super,-1);
        glwCompAdd(cm_strc,&lb_bond_t->super,-1);
        glwCompAdd(cm_strc,&_this->tf_bond_t->super,-1);
        glwCompAdd(cm_strc,&lb_bond_s->super,-1);
        glwCompAdd(cm_strc,&_this->tf_bond_s->super,-1);
        glwCompAdd(cm_strc,&_this->sl_bond_s->super,-1);
        glwCompAdd(cm_strc,cm_bond_btns,-1);
# endif
        glwCompSetInsets(&_this->bn_point_c->super,0,0,2,2);
        glwCompSetAlignX(&_this->bn_point_c->super,1);
        glwCompSetInsets(&_this->bn_point_v->super,0,0,2,2);
        glwCompSetInsets(&_this->bn_point_sa->super,0,0,2,2);
        glwCompSetAlignX(&_this->bn_point_sa->super,0);
        glwCompSetGridWidth(&_this->bn_point_sa->super,GLWC_REMAINDER);
        glwCompSetLayout(cm_point_btns,&(new GLWGridBagLayout())->super);
        glwCompAdd(cm_point_btns,&_this->bn_point_c->super,-1);
        glwCompAdd(cm_point_btns,&_this->bn_point_v->super,-1);
        glwCompAdd(cm_point_btns,&_this->bn_point_sa->super,-1);
# if defined(__DS3_ADD_BONDS__)
        glwCompSetInsets(&_this->bn_bond_a->super,0,0,2,2);
        glwCompSetAlignX(&_this->bn_bond_a->super,1);
        glwCompSetInsets(&_this->bn_bond_d->super,0,0,2,2);
        glwCompSetAlignX(&_this->bn_bond_d->super,0);
        glwCompSetGridWidth(&_this->bn_bond_d->super,GLWC_REMAINDER);
        glwCompSetLayout(cm_bond_btns,&(new GLWGridBagLayout())->super);
        glwCompAdd(cm_bond_btns,&_this->bn_bond_a->super,-1);
        glwCompAdd(cm_bond_btns,&_this->bn_bond_d->super,-1);
# endif
        glwCompSetLayout(cm_bnds,&(new GLWGridBagLayout())->super);
        glwCompSetInsets(&lb_minx->super,0,0,2,2);
        glwCompSetWeightX(&lb_minx->super,1);
        glwCompSetInsets(&_this->tf_minx->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_minx->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_minx->super,1);
        glwCompSetInsets(&_this->sl_minx->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_minx->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_minx->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_miny->super,0,0,2,2);
        glwCompSetWeightX(&lb_miny->super,1);
        glwCompSetInsets(&_this->tf_miny->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_miny->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_miny->super,1);
        glwCompSetInsets(&_this->sl_miny->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_miny->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_miny->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_minz->super,0,0,2,2);
        glwCompSetWeightX(&lb_minz->super,1);
        glwCompSetInsets(&_this->tf_minz->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_minz->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_minz->super,1);
        glwCompSetInsets(&_this->sl_minz->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_minz->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_minz->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_maxx->super,0,0,2,2);
        glwCompSetWeightX(&lb_maxx->super,1);
        glwCompSetInsets(&_this->tf_maxx->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_maxx->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_maxx->super,1);
        glwCompSetInsets(&_this->sl_maxx->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_maxx->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_maxx->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_maxy->super,0,0,2,2);
        glwCompSetWeightX(&lb_maxy->super,1);
        glwCompSetInsets(&_this->tf_maxy->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_maxy->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_maxy->super,1);
        glwCompSetInsets(&_this->sl_maxy->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_maxy->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_maxy->super,GLWC_HORIZONTAL);
        glwCompSetInsets(&lb_maxz->super,0,0,2,2);
        glwCompSetWeightX(&lb_maxz->super,1);
        glwCompSetInsets(&_this->tf_maxz->super,0,0,2,2);
        glwCompSetGridWidth(&_this->tf_maxz->super,GLWC_REMAINDER);
        glwCompSetAlignX(&_this->tf_maxz->super,1);
        glwCompSetInsets(&_this->sl_maxz->super,0,0,10,2);
        glwCompSetGridWidth(&_this->sl_maxz->super,GLWC_REMAINDER);
        glwCompSetFill(&_this->sl_maxz->super,GLWC_HORIZONTAL);
        glwCompAdd(cm_bnds,&lb_minx->super,-1);
        glwCompAdd(cm_bnds,&_this->tf_minx->super,-1);
        glwCompAdd(cm_bnds,&_this->sl_minx->super,-1);
        glwCompAdd(cm_bnds,&lb_miny->super,-1);
        glwCompAdd(cm_bnds,&_this->tf_miny->super,-1);
        glwCompAdd(cm_bnds,&_this->sl_miny->super,-1);
        glwCompAdd(cm_bnds,&lb_minz->super,-1);
        glwCompAdd(cm_bnds,&_this->tf_minz->super,-1);
        glwCompAdd(cm_bnds,&_this->sl_minz->super,-1);
        glwCompAdd(cm_bnds,&lb_maxx->super,-1);
        glwCompAdd(cm_bnds,&_this->tf_maxx->super,-1);
        glwCompAdd(cm_bnds,&_this->sl_maxx->super,-1);
        glwCompAdd(cm_bnds,&lb_maxy->super,-1);
        glwCompAdd(cm_bnds,&_this->tf_maxy->super,-1);
        glwCompAdd(cm_bnds,&_this->sl_maxy->super,-1);
        glwCompAdd(cm_bnds,&lb_maxz->super,-1);
        glwCompAdd(cm_bnds,&_this->tf_maxz->super,-1);
        glwCompAdd(cm_bnds,&_this->sl_maxz->super,-1);
        glwCompSetLayout(cm_view,&(new GLWGridBagLayout())->super);
        glwCompSetInsets(&_this->cb_draw_coords->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_draw_coords->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_zoom->super,2,2,2,2);
        glwCompSetFill(&lb_zoom->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_zoom->super,1);
        glwCompSetInsets(&_this->tf_zoom->super,2,2,2,2);
        glwCompSetAlignX(&_this->tf_zoom->super,1);
        glwCompSetPreWidth(&_this->sl_zoom->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_zoom->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_zoom->super,1);
        glwCompSetGridWidth(&_this->sl_zoom->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_ornt->super,0,0,2,2);
        glwCompSetGridWidth(&lb_ornt->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_ornt_y->super,0,0,2,2);
        glwCompSetFill(&lb_ornt_y->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_ornt_y->super,1);
        glwCompSetInsets(&_this->tf_ornt_y->super,2,2,2,2);
        glwCompSetPreWidth(&_this->sl_ornt_y->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_ornt_y->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_ornt_y->super,1);
        glwCompSetGridWidth(&_this->sl_ornt_y->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_ornt_p->super,0,0,2,2);
        glwCompSetFill(&lb_ornt_p->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_ornt_p->super,1);
        glwCompSetInsets(&_this->tf_ornt_p->super,2,2,2,2);
        glwCompSetPreWidth(&_this->sl_ornt_p->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_ornt_p->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_ornt_p->super,1);
        glwCompSetGridWidth(&_this->sl_ornt_p->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_ornt_r->super,0,0,2,2);
        glwCompSetFill(&lb_ornt_r->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_ornt_r->super,1);
        glwCompSetInsets(&_this->tf_ornt_r->super,2,2,2,2);
        glwCompSetPreWidth(&_this->sl_ornt_r->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_ornt_r->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_ornt_r->super,1);
        glwCompSetGridWidth(&_this->sl_ornt_r->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_cntr->super,0,0,2,2);
        glwCompSetGridWidth(&lb_cntr->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_cntr_x->super,0,0,2,2);
        glwCompSetFill(&lb_cntr_x->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_cntr_x->super,1);
        glwCompSetInsets(&_this->tf_cntr_x->super,2,2,2,2);
        glwCompSetPreWidth(&_this->sl_cntr_x->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_cntr_x->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_cntr_x->super,1);
        glwCompSetGridWidth(&_this->sl_cntr_x->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_cntr_y->super,0,0,2,2);
        glwCompSetFill(&lb_cntr_y->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_cntr_y->super,1);
        glwCompSetInsets(&_this->tf_cntr_y->super,2,2,2,2);
        glwCompSetPreWidth(&_this->sl_cntr_y->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_cntr_y->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_cntr_y->super,1);
        glwCompSetGridWidth(&_this->sl_cntr_y->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_cntr_z->super,0,0,2,2);
        glwCompSetFill(&lb_cntr_z->super,GLWC_HORIZONTAL);
        glwCompSetWeightX(&lb_cntr_z->super,1);
        glwCompSetInsets(&_this->tf_cntr_z->super,2,2,2,2);
        glwCompSetPreWidth(&_this->sl_cntr_z->super,DS3V_SLIDER_SMALL_WIDTH);
        glwCompSetInsets(&_this->sl_cntr_z->super,0,0,2,2);
        glwCompSetWeightX(&_this->sl_cntr_z->super,1);
        glwCompSetGridWidth(&_this->sl_cntr_z->super,GLWC_REMAINDER);
        glwCompSetGridWidth(cm_view_btns,GLWC_REMAINDER);
        glwCompAdd(cm_view,&_this->cb_draw_coords->super,-1);
        glwCompAdd(cm_view,&lb_zoom->super,-1);
        glwCompAdd(cm_view,&_this->tf_zoom->super,-1);
        glwCompAdd(cm_view,&_this->sl_zoom->super,-1);
        glwCompAdd(cm_view,&lb_ornt->super,-1);
        glwCompAdd(cm_view,&lb_ornt_y->super,-1);
        glwCompAdd(cm_view,&_this->tf_ornt_y->super,-1);
        glwCompAdd(cm_view,&_this->sl_ornt_y->super,-1);
        glwCompAdd(cm_view,&lb_ornt_p->super,-1);
        glwCompAdd(cm_view,&_this->tf_ornt_p->super,-1);
        glwCompAdd(cm_view,&_this->sl_ornt_p->super,-1);
        glwCompAdd(cm_view,&lb_ornt_r->super,-1);
        glwCompAdd(cm_view,&_this->tf_ornt_r->super,-1);
        glwCompAdd(cm_view,&_this->sl_ornt_r->super,-1);
        glwCompAdd(cm_view,&lb_cntr->super,-1);
        glwCompAdd(cm_view,&lb_cntr_x->super,-1);
        glwCompAdd(cm_view,&_this->tf_cntr_x->super,-1);
        glwCompAdd(cm_view,&_this->sl_cntr_x->super,-1);
        glwCompAdd(cm_view,&lb_cntr_y->super,-1);
        glwCompAdd(cm_view,&_this->tf_cntr_y->super,-1);
        glwCompAdd(cm_view,&_this->sl_cntr_y->super,-1);
        glwCompAdd(cm_view,&lb_cntr_z->super,-1);
        glwCompAdd(cm_view,&_this->tf_cntr_z->super,-1);
        glwCompAdd(cm_view,&_this->sl_cntr_z->super,-1);
        glwCompAdd(cm_view,cm_view_btns,-1);
        glwCompSetInsets(&bn_ornt->super,2,2,2,2);
        glwCompSetAlignX(&bn_ornt->super,1);
        glwCompSetInsets(&bn_cntr->super,2,2,2,2);
        glwCompSetAlignX(&bn_cntr->super,0);
        glwCompSetGridWidth(&bn_cntr->super,GLWC_REMAINDER);
        glwCompSetInsets(&bn_align->super,2,2,2,2);
        glwCompSetGridWidth(&bn_align->super,GLWC_REMAINDER);
        glwCompSetLayout(cm_view_btns,&(new GLWGridBagLayout())->super);
        glwCompAdd(cm_view_btns,&bn_ornt->super,-1);
        glwCompAdd(cm_view_btns,&bn_cntr->super,-1);
        glwCompAdd(cm_view_btns,&bn_align->super,-1);
        glwCompSetLayout(cm_opts,&(new GLWGridBagLayout())->super);
        glwCompSetInsets(&lb_scale->super,0,0,2,2);
        glwCompSetGridWidth(&lb_scale->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->cb_scale_linear->super,0,0,2,2);
        glwCompSetInsets(&_this->cb_scale_log->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_scale_log->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_color->super,0,0,2,2);
        glwCompSetGridWidth(&lb_color->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->cb_color_rainbow->super,0,0,2,2);
        glwCompSetInsets(&_this->cb_color_grayscale->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_color_grayscale->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_backc->super,0,0,2,2);
        glwCompSetGridWidth(&lb_backc->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->cb_backc_black->super,0,0,2,2);
        glwCompSetInsets(&_this->cb_backc_white->super,0,0,2,2);
        glwCompSetGridWidth(&_this->cb_backc_white->super,GLWC_REMAINDER);
        glwCompSetInsets(&lb_projt->super,0,0,2,2);
        glwCompSetGridWidth(&lb_projt->super,GLWC_REMAINDER);
        glwCompSetInsets(&_this->cb_projt_persp->super,0,0,2,2);
        glwCompSetInsets(&_this->cb_projt_ortho.super, 0, 0, 2, 2);
        glwCompSetGridWidth(&_this->cb_projt_ortho.super, GLWC_REMAINDER);
        glwCompAdd(cm_opts,&lb_scale->super,-1);
        glwCompAdd(cm_opts,&_this->cb_scale_linear->super,-1);
        glwCompAdd(cm_opts,&_this->cb_scale_log->super,-1);
        glwCompAdd(cm_opts,&lb_color->super,-1);
        glwCompAdd(cm_opts,&_this->cb_color_rainbow->super,-1);
        glwCompAdd(cm_opts,&_this->cb_color_grayscale->super,-1);
        glwCompAdd(cm_opts,&lb_backc->super,-1);
        glwCompAdd(cm_opts,&_this->cb_backc_black->super,-1);
        glwCompAdd(cm_opts,&_this->cb_backc_white->super,-1);
        glwCompAdd(cm_opts,&lb_projt->super,-1);
        glwCompAdd(cm_opts,&_this->cb_projt_persp->super,-1);
        glwCompAdd(cm_opts,&_this->cb_projt_ortho.super, -1);
        ds3ViewerFileTextChanged(_this, &_this->tf_file.super);
        ds3ViewerViewDataChanged(_this,_this->ds3view);
        ds3ViewerSetPointR(_this,_this->ds3view->point_r);
        ds3ViewerSetSlice(_this,_this->ds3view->slice_t,_this->ds3view->slice_p,
                          _this->ds3view->slice_d);
        ds3ViewerSetIso(_this,_this->ds3view->iso_v,_this->ds3view->iso_d);
        ds3ViewerSetZoom(_this,1);
        ds3ViewerSetOrientation(_this,_this->ds3view->yaw,_this->ds3view->pitch,
                                _this->ds3view->roll);
        ds3ViewerSetCenter(_this,0.5,0.5,0.5);
        ds3ViewerSetBox(_this,_this->ds3view->box[0][X],_this->ds3view->box[0][Y],
                        _this->ds3view->box[0][Z],_this->ds3view->box[1][X],
                        _this->ds3view->box[1][Y],_this->ds3view->box[1][Z]);
        glwFramePack(_this->frame);
    } else {
        throw "DS3Viewer initialization failed\n";
    }
}

DS3Viewer::~DS3Viewer()
{
    printf("Destroying DS3Viewer\n");
    DS3Viewer* _this = this;
    delete (_this->cb_projt_persp);
//    delete (lb_projt);
    delete (_this->cb_backc_white);
    delete (_this->cb_backc_black);
//    delete (lb_backc);
    delete (_this->cb_color_grayscale);
    delete (_this->cb_color_rainbow);
//    delete (lb_color);
    delete (_this->cb_scale_log);
    delete (_this->cb_scale_linear);
    // delete (lb_scale);
    // glwButtonFree(bn_align);
    // glwButtonFree(bn_cntr);
    // glwButtonFree(bn_ornt);
    delete _this->sl_cntr_z;
    delete (_this->tf_cntr_z);
//    delete (lb_cntr_z);
    delete _this->sl_cntr_y;
    delete (_this->tf_cntr_y);
//    delete (lb_cntr_y);
    delete _this->sl_cntr_x;
    delete (_this->tf_cntr_x);
//    delete (lb_cntr_x);
//    delete (lb_cntr);
    delete _this->sl_ornt_r;
    delete (_this->tf_ornt_r);
//    delete (lb_ornt_r);
    delete _this->sl_ornt_p;
    delete (_this->tf_ornt_p);
//    delete (lb_ornt_p);
    delete _this->sl_ornt_y;
    delete (_this->tf_ornt_y);
//    delete (lb_ornt_y);
//    delete (lb_ornt);
    delete _this->sl_zoom;
    delete (_this->tf_zoom);
//    delete (lb_zoom);
    delete (_this->cb_draw_coords);
    delete _this->sl_maxz;
    delete (_this->tf_maxz);
//    delete (lb_maxz);
    delete _this->sl_minz;
    delete (_this->tf_minz);
//    delete (lb_minz);
    delete _this->sl_maxy;
    delete (_this->tf_maxy);
//    delete (lb_maxy);
    delete _this->sl_miny;
    delete (_this->tf_miny);
//    delete (lb_miny);
    delete _this->sl_maxx;
    delete (_this->tf_maxx);
//    delete (lb_maxx);
    delete _this->sl_minx;
    delete (_this->tf_minx);
//    delete (lb_minx);
# if defined(__DS3_ADD_BONDS__)
    delete (_this->bn_bond_d);
    delete (_this->bn_bond_a);
    delete _this->sl_bond_s;
    delete (_this->tf_bond_s);
//    delete (lb_bond_s);
    delete (_this->tf_bond_t);
//    delete (lb_bond_t);
    delete (_this->tf_bond_f);
//    delete (lb_bond_f);
# endif
    delete (_this->bn_point_sa);
    delete (_this->bn_point_v);
    delete (_this->bn_point_c);
    delete (_this->lb_point_l);
    delete (_this->lb_point_t);
    delete (_this->tf_point_s);
//    delete (lb_point_s);
    delete _this->sl_point_r;
    delete (_this->tf_point_r);
//    delete (lb_point_r);
    delete (_this->cb_draw_points);
    delete _this->sl_iso_d;
    /*delete (_this->tf_iso_d);*/
//    delete (lb_iso_d);
    delete _this->sl_iso_v;
    delete (_this->tf_iso_v);
//    delete (lb_iso_v);
    delete (_this->cb_draw_iso);
    delete _this->sl_slice_d;
    delete (_this->tf_slice_d);
//    delete (lb_slice_d);
    delete _this->sl_slice_p;
    delete (_this->tf_slice_p);
//    delete (lb_slice_p);
//    delete _this->sl_slice_t; // unique_ptr auto-delete?
    delete (_this->tf_slice_t);
//    delete (lb_slice_t);
    delete (_this->cb_draw_slice);
    delete (_this->lb_datav);
    delete (_this->lb_dataz);
    delete (_this->lb_datay);
    delete (_this->lb_datax);
    delete (_this->lb_status);
    delete _this->legend;
    delete _this->tp_ctrl;
//    glwCompFree(cm_opts);
//    glwCompFree(cm_view_btns);
//    glwCompFree(cm_view);
//    glwCompFree(cm_bnds);
# if defined(__DS3_ADD_BONDS__)
//    glwCompFree(cm_bond_btns);
# endif
//    glwCompFree(cm_point_btns);
//    glwCompFree(cm_strc);
//    glwCompFree(cm_data);
    delete (_this->lb_data_set);
//    delete (lb_file);
//    glwCompFree(cm_vals);
    delete _this->ds3view;
    delete _this->frame;
}

void ds3ViewerUpdatePointRLabels(DS3Viewer *_this)
{
    int i;
    for (i=0; i<=100; i+=50) {
        char text[32];
        sprintf(text,"%0.3g",i*0.001*_this->ds3view->offs);
        glwSliderAddLabel(_this->sl_point_r,i,text);
    }
}

void ds3ViewerUpdateIsoVLabels(DS3Viewer *_this)
{
    char text[32];
    sprintf(text,"%0.4g", _this->ds3->min);
    glwSliderAddLabel(_this->sl_iso_v,0,text);
    sprintf(text,"%0.4g",dsUnscale(_this->ds3view->ds,
                                   (DS3V_ISO_V_RES>>1)*(1.0/DS3V_ISO_V_RES)));
    glwSliderAddLabel(_this->sl_iso_v,DS3V_ISO_V_RES>>1,text);
    sprintf(text,"%0.4g", _this->ds3->max);
    glwSliderAddLabel(_this->sl_iso_v,DS3V_ISO_V_RES,text);
}

/*
void ds3ViewerUpdateZoomLabels(DS3Viewer *_this){
 char text[32];
 int  i;
 for(i=0;i<=200;i+=100){
  sprintf(text,"%0.4g",i*0.01*_this->ds3view->offs);
  glwSliderAddLabel(_this->sl_zoom,i,text);} }
*/

void ds3ViewerSetPointR(DS3Viewer *_this,double _r)
{
    double r;
    char   text[32];
    ds3ViewSetPointR(_this->ds3view,_r);
    _r=_this->ds3view->point_r;
    sprintf(text,"%0.3g",_r);
    glwTextFieldSetChangedFunc(_this->tf_point_r,
                               (GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_point_r,text);
    glwTextFieldSetChangedFunc(_this->tf_point_r,
                               (GLWActionFunc)ds3ViewerTextChanged);
    r=_r*1000;
    if (_this->ds3view->offs>1E-16)r/=_this->ds3view->offs;
    r+=0.5;
    glwSliderSetChangedFunc(_this->sl_point_r,NULL);
    glwSliderSetVal(_this->sl_point_r,(int)r,0);
    glwSliderSetChangedFunc(_this->sl_point_r,
                            (GLWActionFunc)ds3ViewerPointRSliderChanged);
}

void ds3ViewerSetSlice(DS3Viewer *_this,double _t,double _p,double _d)
{
    char   text[32];
    ds3ViewSetSliceChangedFunc(_this->ds3view,NULL);
    ds3ViewSetSlice(_this->ds3view,_t,_p,_d);
    ds3ViewSetSliceChangedFunc(_this->ds3view,
                               (GLWActionFunc)ds3ViewerViewSliceChanged);
    _t=_this->ds3view->slice_t;
    _p=_this->ds3view->slice_p;
    _d=_this->ds3view->slice_d;
    sprintf(text,"%0.3g",_t);
    glwTextFieldSetChangedFunc(_this->tf_slice_t,
                               (GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_slice_t,text);
    glwTextFieldSetChangedFunc(_this->tf_slice_t,
                               (GLWActionFunc)ds3ViewerTextChanged);
    _this->sl_slice_t->setChangedFunc(NULL);
    _this->sl_slice_t->setVal((int)(_t+0.5),0);
    _this->sl_slice_t->setChangedFunc(
        (GLWActionFunc)ds3ViewerSliceTSliderChanged);
    sprintf(text,"%0.3g",_p);
    glwTextFieldSetChangedFunc(_this->tf_slice_p,
                               (GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_slice_p,text);
    glwTextFieldSetChangedFunc(_this->tf_slice_p,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_slice_p,NULL);
    glwSliderSetVal(_this->sl_slice_p,(int)(_p+0.5),0);
    glwSliderSetChangedFunc(_this->sl_slice_p,
                            (GLWActionFunc)ds3ViewerSlicePSliderChanged);
    sprintf(text,"%0.3g",_d);
    glwTextFieldSetChangedFunc(_this->tf_slice_d,
                               (GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_slice_d,text);
    glwTextFieldSetChangedFunc(_this->tf_slice_d,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_slice_d,NULL);
    glwSliderSetVal(_this->sl_slice_d,(int)floor(_d*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_slice_d,
                            (GLWActionFunc)ds3ViewerSliceDSliderChanged);
}

void ds3ViewerSetIso(DS3Viewer *_this,double _v,int _d)
{
    char text[32];
    ds3ViewSetIso(_this->ds3view,_v,_d);
    _v=_this->ds3view->iso_v;
    _d=_this->ds3view->iso_d;
    sprintf(text,"%0.4g",_v);
    glwTextFieldSetChangedFunc(_this->tf_iso_v,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_iso_v,text);
    glwTextFieldSetChangedFunc(_this->tf_iso_v,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_iso_v,NULL);
    glwSliderSetVal(_this->sl_iso_v,
                    (int)floor(dsScale(_this->ds3view->ds,_v)*DS3V_ISO_V_RES+
                               0.5),0);
    glwSliderSetChangedFunc(_this->sl_iso_v,
                            (GLWActionFunc)ds3ViewerIsoVSliderChanged);
    /*sprintf(text,"%i",_d);*/
    /*glwTextFieldSetChangedFunc(_this->tf_iso_d,
                                 (GLWActionFunc)ds3ViewerTextSet);*/
    /*glwTextFieldSetText(_this->tf_iso_d,text);*/
    /*glwTextFieldSetChangedFunc(_this->tf_iso_d,
                                 (GLWActionFunc)ds3ViewerTextChanged);*/
    glwSliderSetChangedFunc(_this->sl_iso_d,NULL);
    glwSliderSetVal(_this->sl_iso_d,_d,0);
    glwSliderSetChangedFunc(_this->sl_iso_d,
                            (GLWActionFunc)ds3ViewerIsoDSliderChanged);
}

void ds3ViewerSetZoom(DS3Viewer *_this,double _z)
{
    char text[32];
    ds3ViewSetZoomChangedFunc(_this->ds3view,NULL);
    ds3ViewSetZoom(_this->ds3view,_z*_this->ds3view->offs);
    ds3ViewSetZoomChangedFunc(_this->ds3view,
                              (GLWActionFunc)ds3ViewerViewZoomChanged);
    _z=_this->ds3view->zoom;
    if (_this->ds3view->offs>1E-16)_z/=_this->ds3view->offs;
    sprintf(text,"%0.4g",_z);
    glwTextFieldSetChangedFunc(_this->tf_zoom,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_zoom,text);
    glwTextFieldSetChangedFunc(_this->tf_zoom,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_zoom,NULL);
    glwSliderSetVal(_this->sl_zoom,(int)(_z*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_zoom,
                            (GLWActionFunc)ds3ViewerZoomSliderChanged);
}

void ds3ViewerSetOrientation(DS3Viewer *_this,double _y,double _p,double _r)
{
    char text[32];
    ds3ViewSetOrientationChangedFunc(_this->ds3view,NULL);
    ds3ViewSetOrientation(_this->ds3view,_y,_p,_r);
    ds3ViewSetOrientationChangedFunc(_this->ds3view,(GLWActionFunc)
                                     ds3ViewerViewOrientationChanged);
    _y=_this->ds3view->yaw;
    _p=_this->ds3view->pitch;
    _r=_this->ds3view->roll;
    sprintf(text,"%0.3g",_y);
    glwTextFieldSetChangedFunc(_this->tf_ornt_y,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_ornt_y,text);
    glwTextFieldSetChangedFunc(_this->tf_ornt_y,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_ornt_y,NULL);
    glwSliderSetVal(_this->sl_ornt_y,(int)(_y+0.5),0);
    glwSliderSetChangedFunc(_this->sl_ornt_y,
                            (GLWActionFunc)ds3ViewerOrntYSliderChanged);
    sprintf(text,"%0.3g",_p);
    glwTextFieldSetChangedFunc(_this->tf_ornt_p,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_ornt_p,text);
    glwTextFieldSetChangedFunc(_this->tf_ornt_p,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_ornt_p,NULL);
    glwSliderSetVal(_this->sl_ornt_p,(int)(_p+0.5),0);
    glwSliderSetChangedFunc(_this->sl_ornt_p,
                            (GLWActionFunc)ds3ViewerOrntPSliderChanged);
    sprintf(text,"%0.3g",_r);
    glwTextFieldSetChangedFunc(_this->tf_ornt_r,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_ornt_r,text);
    glwTextFieldSetChangedFunc(_this->tf_ornt_r,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_ornt_r,NULL);
    glwSliderSetVal(_this->sl_ornt_r,(int)(_r+0.5),0);
    glwSliderSetChangedFunc(_this->sl_ornt_r,
                            (GLWActionFunc)ds3ViewerOrntRSliderChanged);
}

void ds3ViewerSetCenter(DS3Viewer *_this,double _x,double _y,double _z)
{
    char   text[32];
    Vect3d cntr;
    int    i;
    for (i=0; i<3; i++) {
        cntr[i] = _this->ds3->basis[i][X] * _x + _this->ds3->basis[i][Y] * _y
                  + _this->ds3->basis[i][Z] * _z;
    }
    ds3ViewSetCenterChangedFunc(_this->ds3view,NULL);
    ds3ViewSetCenter(_this->ds3view,cntr[X],cntr[Y],cntr[Z]);
    ds3ViewSetCenterChangedFunc(_this->ds3view,
                                (GLWActionFunc)ds3ViewerViewCenterChanged);
    _x=vectDot3d(_this->ds3view->basinv[X],_this->ds3view->cntr);
    _y=vectDot3d(_this->ds3view->basinv[Y],_this->ds3view->cntr);
    _z=vectDot3d(_this->ds3view->basinv[Z],_this->ds3view->cntr);
    sprintf(text,"%0.3g",_x);
    glwTextFieldSetChangedFunc(_this->tf_cntr_x,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_cntr_x,text);
    glwTextFieldSetChangedFunc(_this->tf_cntr_x,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_cntr_x,NULL);
    glwSliderSetVal(_this->sl_cntr_x,(int)floor(_x*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_cntr_x,
                            (GLWActionFunc)ds3ViewerCntrXSliderChanged);
    sprintf(text,"%0.3g",_y);
    glwTextFieldSetChangedFunc(_this->tf_cntr_y,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_cntr_y,text);
    glwTextFieldSetChangedFunc(_this->tf_cntr_x,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_cntr_y,NULL);
    glwSliderSetVal(_this->sl_cntr_y,(int)floor(_y*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_cntr_y,
                            (GLWActionFunc)ds3ViewerCntrYSliderChanged);
    sprintf(text,"%0.3g",_z);
    glwTextFieldSetChangedFunc(_this->tf_cntr_z,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_cntr_z,text);
    glwTextFieldSetChangedFunc(_this->tf_cntr_z,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_cntr_z,NULL);
    glwSliderSetVal(_this->sl_cntr_z,(int)floor(_z*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_cntr_z,
                            (GLWActionFunc)ds3ViewerCntrZSliderChanged);
}

void ds3ViewerSetBox(DS3Viewer *_this,double _minx,double _miny,double _minz,
                     double _maxx,double _maxy,double _maxz)
{
    char text[32];
    ds3ViewSetBoxChangedFunc(_this->ds3view,NULL);
    ds3ViewSetBox(_this->ds3view,_minx,_miny,_minz,_maxx,_maxy,_maxz);
    ds3ViewSetBoxChangedFunc(_this->ds3view,
                             (GLWActionFunc)ds3ViewerViewBoxChanged);
    _minx=_this->ds3view->box[0][X];
    _miny=_this->ds3view->box[0][Y];
    _minz=_this->ds3view->box[0][Z];
    _maxx=_this->ds3view->box[1][X];
    _maxy=_this->ds3view->box[1][Y];
    _maxz=_this->ds3view->box[1][Z];
    sprintf(text,"%0.3g",_minx);
    glwTextFieldSetChangedFunc(_this->tf_minx,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_minx,text);
    glwTextFieldSetChangedFunc(_this->tf_minx,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_minx,NULL);
    glwSliderSetVal(_this->sl_minx,(int)floor(_minx*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_minx,
                            (GLWActionFunc)ds3ViewerMinXSliderChanged);
    sprintf(text,"%0.3g",_miny);
    glwTextFieldSetChangedFunc(_this->tf_miny,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_miny,text);
    glwTextFieldSetChangedFunc(_this->tf_miny,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_miny,NULL);
    glwSliderSetVal(_this->sl_miny,(int)floor(_miny*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_miny,
                            (GLWActionFunc)ds3ViewerMinYSliderChanged);
    sprintf(text,"%0.3g",_minz);
    glwTextFieldSetChangedFunc(_this->tf_minz,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_minz,text);
    glwTextFieldSetChangedFunc(_this->tf_minz,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_minz,NULL);
    glwSliderSetVal(_this->sl_minz,(int)floor(_minz*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_minz,
                            (GLWActionFunc)ds3ViewerMinZSliderChanged);
    sprintf(text,"%0.3g",_maxx);
    glwTextFieldSetChangedFunc(_this->tf_maxx,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_maxx,text);
    glwTextFieldSetChangedFunc(_this->tf_maxx,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_maxx,NULL);
    glwSliderSetVal(_this->sl_maxx,(int)floor(_maxx*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_maxx,
                            (GLWActionFunc)ds3ViewerMaxXSliderChanged);
    sprintf(text,"%0.3g",_maxy);
    glwTextFieldSetChangedFunc(_this->tf_maxy,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_maxy,text);
    glwTextFieldSetChangedFunc(_this->tf_maxy,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_maxy,NULL);
    glwSliderSetVal(_this->sl_maxy,(int)floor(_maxy*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_maxy,
                            (GLWActionFunc)ds3ViewerMaxYSliderChanged);
    sprintf(text,"%0.3g",_maxz);
    glwTextFieldSetChangedFunc(_this->tf_maxz,(GLWActionFunc)ds3ViewerTextSet);
    glwTextFieldSetText(_this->tf_maxz,text);
    glwTextFieldSetChangedFunc(_this->tf_maxz,
                               (GLWActionFunc)ds3ViewerTextChanged);
    glwSliderSetChangedFunc(_this->sl_maxz,NULL);
    glwSliderSetVal(_this->sl_maxz,(int)floor(_maxz*100+0.5),0);
    glwSliderSetChangedFunc(_this->sl_maxz,
                            (GLWActionFunc)ds3ViewerMaxZSliderChanged);
}

void ds3ViewerSetSelectedPoint(DS3Viewer *_this,long _pt)
{
    char text[128];
    ds3ViewSetPointChangedFunc(_this->ds3view,NULL);
    ds3ViewSetSelectedPoint(_this->ds3view,_pt);
    ds3ViewSetPointChangedFunc(_this->ds3view,
                               (GLWActionFunc)ds3ViewerViewPointChanged);
    _pt=ds3ViewGetSelectedPoint(_this->ds3view);
    if (_pt>=0) {
        sprintf(text,"%li",_pt+1);
        glwTextFieldSetChangedFunc(_this->tf_point_s,
                                   (GLWActionFunc)ds3ViewerTextSet);
        glwTextFieldSetText(_this->tf_point_s,text);
        glwTextFieldSetChangedFunc(_this->tf_point_s,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        sprintf(text,"Atom type: %i",_this->ds3->points[_pt].typ+1);
        glwLabelSetLabel(_this->lb_point_t,text);
        sprintf(text,"Atom location: <%0.4g,%0.4g,%0.4g>",
                _this->ds3->points[_pt].pos[X], _this->ds3->points[_pt].pos[Y],
                _this->ds3->points[_pt].pos[Z]);
        glwLabelSetLabel(_this->lb_point_l,text);
        ds3ViewerSetPointVisible(_this,
                                 ds3ViewGetPointVisible(_this->ds3view,_pt));
    }
}

void ds3ViewerSetPointVisible(DS3Viewer *_this,int _v)
{
    long l;
    l=ds3ViewGetSelectedPoint(_this->ds3view);
    if (l>=0) {
        ds3ViewSetPointChangedFunc(_this->ds3view,NULL);
        ds3ViewSetPointVisible(_this->ds3view,l,_v);
        ds3ViewSetPointChangedFunc(_this->ds3view,
                                   (GLWActionFunc)ds3ViewerViewPointChanged);
        _v=ds3ViewGetPointVisible(_this->ds3view,l);
        if (_v)glwButtonSetLabel(_this->bn_point_v,"Hide Atom");
        else glwButtonSetLabel(_this->bn_point_v,"Show Atom");
    } else glwButtonSetLabel(_this->bn_point_v,"Hide Atom");
}

# if defined(__DS3_ADD_BONDS__)
void ds3ViewerSetSelectedBond(DS3Viewer *_this,long _f,long _t)
{
    ds3ViewSetBondChangedFunc(_this->ds3view,NULL);
    ds3ViewSetSelectedBond(_this->ds3view,_f,_t);
    ds3ViewSetBondChangedFunc(_this->ds3view,
                              (GLWActionFunc)ds3ViewerViewBondChanged);
    if (ds3ViewGetSelectedBond(_this->ds3view,&_f,&_t)) {
        char text[32];
        sprintf(text,"%li",_f+1);
        glwTextFieldSetChangedFunc(_this->tf_bond_f,
                                   (GLWActionFunc)ds3ViewerTextSet);
        glwTextFieldSetText(_this->tf_bond_f,text);
        glwTextFieldSetChangedFunc(_this->tf_bond_f,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        sprintf(text,"%li",_t+1);
        glwTextFieldSetChangedFunc(_this->tf_bond_t,
                                   (GLWActionFunc)ds3ViewerTextSet);
        glwTextFieldSetText(_this->tf_bond_t,text);
        glwTextFieldSetChangedFunc(_this->tf_bond_t,
                                   (GLWActionFunc)ds3ViewerTextChanged);
        ds3ViewerSetBond(_this,ds3ViewGetBond(_this->ds3view,_f,_t));
    }
}

void ds3ViewerSetBond(DS3Viewer *_this,double _sz)
{
    long f;
    long t;
    if (ds3ViewGetSelectedBond(_this->ds3view,&f,&t)) {
        ds3ViewSetBondChangedFunc(_this->ds3view,NULL);
        ds3ViewSetBond(_this->ds3view,f,t,_sz);
        ds3ViewSetBondChangedFunc(_this->ds3view,
                                  (GLWActionFunc)ds3ViewerViewBondChanged);
        _sz=ds3ViewGetBond(_this->ds3view,f,t);
        if (_sz>0) {
            char text[32];
            sprintf(text,"%0.4g",_sz*10);
            glwTextFieldSetChangedFunc(_this->tf_bond_s,
                                       (GLWActionFunc)ds3ViewerTextSet);
            glwTextFieldSetText(_this->tf_bond_s,text);
            glwTextFieldSetChangedFunc(_this->tf_bond_s,
                                       (GLWActionFunc)ds3ViewerTextChanged);
            glwSliderSetVal(_this->sl_bond_s,(int)(_sz*10+0.5),0);
        }
    }
}
# endif

void ds3ViewerOpenFile(DS3Viewer *_this,const char *_file)
{
    if (_this->read_id) {
        glwCompDelIdler(&_this->frame->super,_this->read_id);
        _this->read_id=0;
        _this->reader->cancel();
	_this->read_name.clear();
        delete _this->reader;
    }
    if (_file==NULL||_file[0]=='\0') {
        glwLabelSetLabel(_this->lb_status,"Please type a file name in the "
                         "\'Open File\' field.");
    } else {
        int    ret = 1;
        size_t name_sz;
        name_sz = (strlen(_file)+1)*sizeof(char);
        glwTextFieldSetText(&_this->tf_file, _file);
        _this->reader = new DS3VaspReader(_file, "r");
        if (_this->reader->file.f == NULL) {
            glwLabelSetLabel(_this->lb_status,"Could not open \"");
            glwLabelAddLabel(_this->lb_status,_file);
            glwLabelAddLabel(_this->lb_status,"\": ");
            glwLabelAddLabel(_this->lb_status,strerror(errno));
        } else {
            _this->read_prog = 0;
	    _this->read_name = _file;
            _this->read_id=glwCompAddIdler(&_this->frame->super,
                                           (GLWActionFunc)ds3ViewerAsyncRead,_this);
            if (!_this->read_id) {
                _this->reader->cancel();
                ret=0;
                errno=ENOMEM;
            }
        }
        if (ret<=0) {
            if (!ret) {
                if (!errno)errno=ENOMEM;
                glwLabelSetLabel(_this->lb_status,"Error reading \"");
                glwLabelAddLabel(_this->lb_status,_file);
                glwLabelAddLabel(_this->lb_status,"\": ");
                glwLabelAddLabel(_this->lb_status,strerror(errno));
            } else ds3ViewerFinishRead(_this);
	    _this->read_name.clear();
            delete _this->reader;
        }
    }
}
