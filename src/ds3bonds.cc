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
#include "ds3bonds.hh"

/*Initializes the bonds structure*/
DS3Bonds::DS3Bonds()
{
    this->natoms=0;
    this->nbonds=0;
}

/*Removes any existing bonds, and sets up the structure for adding bonds to
  the given data set
  ds3: The data set new bonds may be added to*/
int ds3BondsReset(DS3Bonds *_this,DataSet3D *_ds3)
{
    long    natoms;
    natoms=_ds3!=NULL?(long)_ds3->npoints:0;
    if (natoms < 2)
	_this->bonds.clear();
    else
	_this->bonds.resize((natoms - 1) * (natoms - 2));
    _this->nbonds=0;
    _this->natoms=natoms;
    return 1;
}

/*Sets the size of a bond
  from: The index of the atom the bond is from
  to:   The index of the atom the bond is to, which must be larger than from
  sz:   The size of the bound. If this is not positive, no bond is drawn*/
void ds3BondsSet(DS3Bonds *_this,long _from,long _to,double _sz)
{
    long i;
    i=_DSBondIdx(_from,_to,_this->natoms);
    if (_this->bonds[i]>0&&_sz<=0)_this->nbonds--;
    else if (_this->bonds[i]<=0&&_sz>0)_this->nbonds++;
    _this->bonds[i]=_sz;
}

/*Removes a bond, if it exists
  from: The index of the atom the bond is from
  to:   The index of the atom the bond is to, which must be larger than from*/
void ds3BondsDel(DS3Bonds *_this,long _from,long _to)
{
    ds3BondsSet(_this,_from,_to,0);
}

/*Gets the size of an existing bond
  from: The index of the atom the bond is from
  to:   The index of the atom the bond is to, which must be larger than from
  Return: The size of the bond, or a non-positive number if the bond doesn't
           exist*/
double ds3BondsGet(DS3Bonds *_this,long _from,long _to)
{
    return _this->bonds[_DSBondIdx(_from,_to,_this->natoms)];
}

/*Displays the bonds*/
static void ds3ViewBondsPeerDisplay(DS3ViewComp *_this,
                                    const GLWCallbacks *_cb)
{
    if (_this->ds3view->bonds.nbonds>0) {
        GLUquadricObj *q;
        q=gluNewQuadric();
        if (q!=NULL) {
            static const GLfloat COLOR[4]={1.F,1.F,1.F,1.F};
            DS3View  *view;
            DS3Bonds *bonds;
            int       x[3];
            int       x0[3];
            int       x1[3];
            long      i;
            long      j;
            long      k;
            int       l;
            view=_this->ds3view;
            bonds=&view->bonds;
            for (l=0; l<3; l++) {
                x0[l]=(int)floor(view->box[0][l])-1;
                x1[l]=(int)ceil(view->box[1][l]);
            }
            glPushAttrib(GL_CURRENT_BIT|GL_ENABLE_BIT|GL_LIGHTING_BIT);
            glMaterialfv(GL_FRONT,GL_SPECULAR,COLOR);
            glMateriali(GL_FRONT,GL_SHININESS,16);
            glEnable(GL_LIGHTING);
            glEnable(GL_COLOR_MATERIAL);
            for (x[X]=x0[X]; x[X]<x1[X]; x[X]++) {
                for (x[Y]=x0[Y]; x[Y]<x1[Y]; x[Y]++) {
                    for (x[Z]=x0[Z]; x[Z]<x1[Z]; x[Z]++) {
                        for (i=0,j=0; j<bonds->natoms-1; j++) {
                            if (ds3ViewGetPointVisible(view,j)) {
                                for (k=j+1; k<bonds->natoms; k++,i++) {
                                    if (ds3ViewGetPointVisible(view,k)&&bonds->bonds[i]>0) {
                                        Vect3d p0;
                                        Vect3d p1;
                                        Vect3d q0;
                                        Vect3d q1;
                                        double d;
                                        vectSet3d(q0,view->ds3->points[j].pos[X]+x[X],
                                                  view->ds3->points[j].pos[Y]+x[Y],
                                                  view->ds3->points[j].pos[Z]+x[Z]);
                                        vectSet3d(q1,view->ds3->points[k].pos[X]+x[X],
                                                  view->ds3->points[k].pos[Y]+x[Y],
                                                  view->ds3->points[k].pos[Z]+x[Z]);
                                        for (l=0; l<3; l++) {
                                            d=q0[l]-q1[l];
                                            if (d<-0.5)q0[l]+=1;
                                            else if (d>0.5)q1[l]+=1;
                                        }
                                        for (l=0; l<3; l++) {           /*Make sure points fall within our box*/
                                            if ((q0[l] < view->box[0][l] && q1[l] < view->box[0][l]) ||
                                                    (q0[l] > view->box[1][l] && q1[l]>view->box[1][l])) break;
                                            if ((q0[l] < view->box[0][l] && q1[l] >= view->box[0][l]) ||
                                                    (q0[l] >= view->box[0][l] && q1[l] < view->box[0][l])) {
                                                glEnable(GL_CLIP_PLANE0+(l<<1));
                                            } else glDisable(GL_CLIP_PLANE0+(l<<1));
                                            if ((q0[l] < view->box[1][l] && q1[l] >= view->box[1][l]) ||
                                                    (q0[l] >= view->box[1][l] && q1[l] < view->box[1][l])) {
                                                glEnable(GL_CLIP_PLANE0+(l<<1)+1);
                                            } else glDisable(GL_CLIP_PLANE0+(l<<1)+1);
                                        }
                                        if (l!=3)continue;
                                        for (l=0; l<3; l++) {        /*Transform points into world-coordinates*/
					    Eigen::Vector3d tmp = view->ds3->basis.col(l).cast<double>();
					    p0[l] = vectDot3d(tmp.data(), q0);
					    p1[l] = vectDot3d(tmp.data(), q0);
                                        }
                                        vectSub3d(q0,p1,p0);
                                        d=vectMag2_3d(q0);
                                        if (d>1E-100) {
                                            GLWcolor c;
                                            double   r;
                                            int      detail;
                                            int      err;
                                            c=GLW_COLOR_WHITE;
                                            r=bonds->bonds[i]*view->point_r;
                                            detail=(int)(view->super.bounds.h*r/(view->offs>1E-4?view->offs:1));
                                            if (detail<8)detail=8;
                                            if (view->track_lbf==j&&view->track_lbt==k&&
                                                    glwCompIsCapturing(&_this->super)) {
                                                glDisable(GL_LIGHT0);
                                                glEnable(GL_LIGHT1);
                                            }
                                            if (view->track_sbf==j&&view->track_sbt==k) {
                                                GLWcolor fc;
                                                if (glwCompIsFocused(&_this->super)) {
                                                    fc=glwColorBlend(view->super.forec,DS3V_FOCUS_COLOR);
                                                } else fc=view->super.forec;
                                                c=glwColorBlend(c,fc);
                                                r+=0.05*view->point_r;
                                            }
                                            d=sqrt(d);
                                            vectMul3d(q0,q0,1/d);
                                            glwColor(c);
                                            glPushMatrix();
                                            glTranslated(p0[X],p0[Y],p0[Z]);
                                            glRotated(atan2(q0[X],q0[Z])*(180/M_PI),0,1,0);
                                            glRotated(-atan2(q0[Y],sqrt(q0[Z]*q0[Z]+q0[X]*q0[X]))*(180/M_PI),
                                                      1,0,0);
                                            gluCylinder(q,r,r,d,detail,1);
                                            glPopMatrix();
                                            err=glGetError();
                                            if (view->track_lbf==j&&view->track_lbt==k&&
                                                    glwCompIsCapturing(&_this->super)) {
                                                glDisable(GL_LIGHT1);
                                                glEnable(GL_LIGHT0);
                                            }
                                        }
                                    }
                                }
                            } else i+=bonds->natoms-1-j;
                        }
                    }
                }
            }
            glPopAttrib();
            gluDeleteQuadric(q);
        }
    }
}

/*Redraws the bonds to reflect keyboard focus, and makes sure a bond is
  selected when we receive focus*/
static void ds3ViewBondsPeerFocus(DS3ViewComp *_this,GLWCallbacks *_cb,
                                  int _s)
{
    DS3View *view;
    glwCompSuperFocus(&_this->super,_cb,_s);
    view=_this->ds3view;
    if (_s) {
        if (view->track_sbf<0&&view->ds3!=NULL&&view->ds3->npoints>1) {
            long j;
            long k;
            ds3ViewSetSelectedBond(view,-1-view->track_sbf,view->track_sbt);
            if (!ds3ViewGetSelectedBond(view,&j,&k)) {
                DS3Bonds *bonds;
                int       i;
                bonds=&view->bonds;
                for (i=0,j=0; j<bonds->natoms-1; j++) {
                    if (ds3ViewGetPointVisible(view,j)) {
                        for (k=j+1; k<bonds->natoms; k++,i++) {
                            if (ds3ViewGetPointVisible(view,k)&&bonds->bonds[i]>0) {
                                ds3ViewSetSelectedBond(view,j,k);
                                return;
                            }
                        }
                    } else i+=bonds->natoms-1-j;
                }
            }
        }
    } else glwCompRepaint(&view->super,0);
}

/*Tracks the current bond the mouse is over*/
static void ds3ViewBondsPeerEntry(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                  int _s)
{
    DS3View *view;
    view=_this->ds3view;
    glwCompRepaint(&view->super,0);
    if (_s) {
        view->track_lbf=view->track_mbf;
        view->track_lbt=view->track_mbt;
    } else view->track_lbf=-1;
}

/*Keyboard manipulation of the bonds*/
static int ds3ViewBondsPeerKeyboard(DS3ViewComp *_this,
                                    const GLWCallbacks *_cb,
                                    unsigned char _k,int _x,int _y)
{
    if (_k==0x08||_k==0x7F) {
        DS3View *view;
        long     bf;
        long     bt;
        view=_this->ds3view;
        if (ds3ViewGetSelectedBond(view,&bf,&bt))ds3ViewDelBond(view,bf,bt);
        return -1;
    }
    return glwCompSuperKeyboard(&_this->super,_cb,_k,_x,_y);
}

/*Keyboard manipulation of the bonds*/
static int ds3ViewBondsPeerSpecial(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                   int _k,int _x,int _y)
{
    DS3View *view;
    int      ret;
    int      d;
    long     bf;
    long    bt;
    view=_this->ds3view;
    ret=-1;
    d=0;
    switch (_k) {
    case GLUT_KEY_UP       :
    case GLUT_KEY_RIGHT    :
        d=1;
        break;
    case GLUT_KEY_DOWN     :
    case GLUT_KEY_LEFT     :
        d=-1;
        break;
    case GLUT_KEY_PAGE_UP  : {
        if (ds3ViewGetSelectedBond(view,&bf,&bt)) {
            double r;
            r=ds3ViewGetBond(view,bf,bt)+0.10;
            if (r>0.5)r=0.5;
            ds3ViewSetBond(view,bf,bt,r);
        }
    }
    break;
    case GLUT_KEY_PAGE_DOWN: {
        if (ds3ViewGetSelectedBond(view,&bf,&bt)) {
            double r;
            r=ds3ViewGetBond(view,bf,bt)-0.10;
            if (r<0.1)r=0.1;
            ds3ViewSetBond(view,bf,bt,r);
        }
    }
    break;
    default                :
        ret=0;
    }
    if (ret<0) {
        if (d&&ds3ViewGetSelectedBond(view,&bf,&bt)) {
            long j;
            long k;
            int  n;
            if (d<0) {
                n=-d;
                d=-1;
            } else {
                n=d;
                d=1;
            }
            j=bf;
            k=bt;
            n++;
            for (;;) {
                if (ds3ViewGetPointVisible(view,j)) {
                    do {
                        if (ds3ViewGetPointVisible(view,k)&&
                                view->bonds.bonds[_DSBondIdx(j,k,view->bonds.natoms)]>0) {
                            n--;
                            if (!n)break;
                        }
                        k=k+d;
                        if (j==bf&&k==bt) {
                            n=-1;
                            break;
                        }
                    } while (k>j&&k<view->bonds.natoms);
                    if (n<=0)break;
                } else if (j==bf)break;
                j=j+d;
                if (j>=view->bonds.natoms-1)j=0;
                else if (j<0)j=view->bonds.natoms-2;
                if (d>0)k=j+1;
                else k=view->bonds.natoms-1;
            }
            if (!n)ds3ViewSetSelectedBond(view,j,k);
        }
    } else ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    return ret;
}

/*Mouse manipulation of the bonds*/
static int ds3ViewBondsPeerMouse(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                 int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0) {
        DS3View *view;
        view=_this->ds3view;
        if (_s) {
            ds3ViewSetSelectedBond(view,view->track_mbf,view->track_mbt);
        }
        ret=-1;
    }
    return ret;
}

/*Tracks the current bond the mouse is over*/
static int ds3ViewBondsPeerPassiveMotion(DS3ViewComp *_this,
        const GLWCallbacks *_cb,
        int _x,int _y)
{
    int ret;
    ret=glwCompSuperPassiveMotion(&_this->super,_cb,_x,_y);
    if (ret>=0) {
        DS3View *view;
        view=_this->ds3view;
        if (view->track_mbf!=view->track_lbf||view->track_mbt!=view->track_lbt) {
            glwCompRepaint(&view->super,0);
            view->track_lbf=view->track_mbf;
            view->track_lbt=view->track_mbt;
        }
        ret=1;
    }
    return ret;
}


const GLWCallbacks DS3_VIEW_BNDS_CALLBACKS= {
    &GLW_COMPONENT_CALLBACKS,
    NULL,
    (GLWDisplayFunc)ds3ViewBondsPeerDisplay,
    NULL,
    NULL,
    NULL,
    (GLWFocusFunc)ds3ViewBondsPeerFocus,
    NULL,
    (GLWEntryFunc)ds3ViewBondsPeerEntry,
    (GLWKeyboardFunc)ds3ViewBondsPeerKeyboard,
    (GLWSpecialFunc)ds3ViewBondsPeerSpecial,
    (GLWMouseFunc)ds3ViewBondsPeerMouse,
    NULL,
    (GLWMotionFunc)ds3ViewBondsPeerPassiveMotion
};
