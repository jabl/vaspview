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
#include "ds3pts.hh"


/*Draws the atoms*/
static void ds3ViewPointsPeerDisplay(DS3ViewComp *_this,
                                     const GLWCallbacks *_cb)
{
    DS3View       *view;
    size_t         i;
    GLUquadricObj *q;
    view=_this->ds3view;
    q=gluNewQuadric();
    if (q!=NULL)
    {
        static const GLfloat COLOR[4]={1.F,1.F,1.F,1.F};
        int x[3];
        int x0[3];
        int x1[3];
        int j;
        for (j=0; j<3; j++)
        {
            x0[j]=(int)floor(view->box[0][j]);
            x1[j]=(int)ceil(view->box[1][j]);
        }
        glPushAttrib(GL_LIGHTING);
        glMaterialfv(GL_FRONT,GL_SPECULAR,COLOR);
        glMateriali(GL_FRONT,GL_SHININESS,16);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        for (x[X]=x0[X]; x[X]<x1[X]; x[X]++)
        {
            for (x[Y]=x0[Y]; x[Y]<x1[Y]; x[Y]++)
            {
                for (x[Z]=x0[Z]; x[Z]<x1[Z]; x[Z]++)
                {
                    for (i=0; i<view->ds3->npoints; i++)
                    {
			    if ((ds3ViewGetPointVisible(view,(long)i)) ||
                                ((size_t)view->track_sp == i && glwCompIsFocused(&_this->super)))
                        {
                            GLWcolor c;
                            Vect3d   p;
                            Vect3d   o;
                            double   r;
                            int      detail;
                            for (j=0; j<3; j++)                /*Make sure point falls within our box*/
                            {
                                double d;
                                d=view->ds3->points[i].pos[j]+x[j];
                                if (d<view->box[0][j]||d>view->box[1][j])break;
                            }
                            if (j!=3)continue;
                            c=dsColorScale(view->cs,view->ds3->points[i].col);
                            r=view->point_r;
                            detail=(int)(view->super.bounds.h*r/(view->offs>1E-4?view->offs:1));
                            if (detail<8)detail=8;
                            if (view->track_mp==(long)i&&glwCompIsCapturing(&_this->super))
                            {
                                glDisable(GL_LIGHT0);
                                glEnable(GL_LIGHT1);
                            }
                            if ((size_t)view->track_sp==i)
                            {
                                GLWcolor fc;
                                if (glwCompIsFocused(&_this->super))
                                {
                                    fc=glwColorBlend(view->super.forec,DS3V_FOCUS_COLOR);
                                }
                                else fc=view->super.forec;
                                c=glwColorBlend(c,fc);
                                r+=0.005*view->offs;
                                if (!ds3ViewGetPointVisible(view,(long)i))
                                {
                                    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                                    detail>>=2;
                                    if (detail<6)detail=6;
                                }
                            }
                            glwColor(c);
                            glPushMatrix();
                            /*We must multiply the basis out oursevles, because we only want to
                              scale the center of the sphere along the basis vectors, not each
                              point on the sphere (which would make ovals)*/
                            vectSet3d(o,view->ds3->points[i].pos[X]+x[X],
                                      view->ds3->points[i].pos[Y]+x[Y],
                                      view->ds3->points[i].pos[Z]+x[Z]);
                            for (j=0; j<3; j++)p[j]=vectDot3d(view->ds3->basis[j],o);
                            glTranslated(p[X],p[Y],p[Z]);
                            gluSphere(q,r,detail,detail);
                            glPopMatrix();
                            if (view->track_mp==(long)i&&glwCompIsCapturing(&_this->super))
                            {
                                glDisable(GL_LIGHT1);
                                glEnable(GL_LIGHT0);
                            }
                            if ((size_t)view->track_sp==i&&!ds3ViewGetPointVisible(view,(long)i))
                            {
                                glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
                            }
                        }
                    }
                }
            }
        }
        glPopAttrib();
        gluDeleteQuadric(q);
    }
}

/*Draws the atoms to reflect keyboard focus. Also makes sure there is a
  currently selected atom when we gain focus*/
static void ds3ViewPointsPeerFocus(DS3ViewComp *_this,GLWCallbacks *_cb,
                                   int _s)
{
    DS3View *view;
    glwCompSuperFocus(&_this->super,_cb,_s);
    view=_this->ds3view;
    if (_s)
    {
        if (view->track_sp<0&&view->ds3!=NULL&&view->ds3->npoints>0)
        {
            ds3ViewSetSelectedPoint(view,-1-view->track_sp);
        }
        else glwCompRepaint(&view->super,0);
    }
}

/*Tracks the current atom the mouse is over*/
static void ds3ViewPointsPeerEntry(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                   int _s)
{
    glwCompRepaint(&_this->ds3view->super,0);
    if (_s)_this->ds3view->track_lp=_this->ds3view->track_mp;
    else _this->ds3view->track_lp=-1;
}

/*Keyboard manipulation of the atoms*/
static int ds3ViewPointsPeerKeyboard(DS3ViewComp *_this,
                                     const GLWCallbacks *_cb,
                                     unsigned char _k,int _x,int _y)
{
    DS3View *view;
    view=_this->ds3view;
    if (view->ds3!=NULL&&view->ds3->npoints>0&&_k==' ')
    {
        long l;
        l=ds3ViewGetSelectedPoint(view);
        ds3ViewSetPointVisible(view,l,!ds3ViewGetPointVisible(view,l));
        return -1;
    }
    return glwCompSuperKeyboard(&_this->super,_cb,_k,_x,_y);
}

/*Keyboard manipulation of the atoms*/
static int ds3ViewPointsPeerSpecial(DS3ViewComp *_this,
                                    const GLWCallbacks *_cb,
                                    int _k,int _x,int _y)
{
    DS3View *view;
    int      ret;
    view=_this->ds3view;
    if (view->ds3!=NULL&&view->ds3->npoints>0)
    {
        ret=-1;
        switch (_k)
        {
        case GLUT_KEY_UP       :
        case GLUT_KEY_RIGHT    :
        {
            long l;
            l=ds3ViewGetSelectedPoint(view);
            if (l<0)l=0;
            else l=(l+1)%(long)view->ds3->npoints;
            ds3ViewSetSelectedPoint(view,l);
        }
        break;
        case GLUT_KEY_DOWN     :
        case GLUT_KEY_LEFT     :
        {
            long l;
            l=ds3ViewGetSelectedPoint(view);
            if (l<0)l=(long)view->ds3->npoints-1;
            else l=(l+(long)view->ds3->npoints-1)%(long)view->ds3->npoints;
            ds3ViewSetSelectedPoint(view,l);
        }
        break;
        case GLUT_KEY_PAGE_UP  :
        {
            long l;
            long m;
            if (view->ds3->npoints>10)m=10;
            else if (view->ds3->npoints>5)m=5;
            else m=1;
            l=ds3ViewGetSelectedPoint(view);
            if (l<0)l=0;
            else l=(l+m)%(long)view->ds3->npoints;
            ds3ViewSetSelectedPoint(view,l);
        }
        break;
        case GLUT_KEY_PAGE_DOWN:
        {
            long l;
            long m;
            if (view->ds3->npoints>10)m=10;
            else if (view->ds3->npoints>5)m=5;
            else m=1;
            l=ds3ViewGetSelectedPoint(view);
            if (l<0)l=(long)view->ds3->npoints-1;
            else l=(l+(long)view->ds3->npoints-m)%(long)view->ds3->npoints;
            ds3ViewSetSelectedPoint(view,l);
        }
        break;
        case GLUT_KEY_HOME     :
        {
            if (!(glutGetModifiers()&GLUT_ACTIVE_CTRL))ds3ViewSetSelectedPoint(view,0);
            else
            {
                long l;
                l=ds3ViewGetSelectedPoint(view);
                if (l>=0)
                {
                    Vect3d p;
                    int    i;
                    for (i=0; i<3; i++)
                    {
                        p[i]=vectDot3d(view->ds3->points[l].pos,view->ds3->basis[i]);
                    }
                    ds3ViewSetCenter(view,p[X],p[Y],p[Z]);
                    ds3ViewSetZoom(view,view->zoom*0.5);
                }
            }
        }
        break;
        case GLUT_KEY_END      :
        {
            ds3ViewSetSelectedPoint(view,(long)view->ds3->npoints-1);
        }
        break;
        default                :
            ret=0;
            break;
        }
    }
    else ret=0;
    if (ret>=0)ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    return ret;
}

/*Mouse manipulation of the atoms and bond creation*/
static int ds3ViewPointsPeerMouse(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                  int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0)
    {
        DS3View *view;
        view=_this->ds3view;
        if (_s)
        {
# if defined(__DS3_ADD_BONDS__)
            if (_b==GLUT_LEFT_BUTTON&&(glutGetModifiers()&GLUT_ACTIVE_CTRL))
            {
                if (ds3ViewGetBond(view,view->track_sp,view->track_mp)<=0)
                {
                    ds3ViewSetSelectedBond(view,view->track_sp,view->track_mp);
                    ds3ViewSetBond(view,view->track_sp,view->track_mp,0.1);
                }
                return -1;
            }
# endif
            ds3ViewSetSelectedPoint(view,view->track_mp);
            if (_b==GLUT_RIGHT_BUTTON)
            {
                ds3ViewSetPointVisible(view,view->track_mp,
                                       !ds3ViewGetPointVisible(view,view->track_mp));
            }
        }
        ret=-1;
    }
    return ret;
}

/*Tracks the current atom the mouse is over*/
static int ds3ViewPointsPeerPassiveMotion(DS3ViewComp *_this,
        const GLWCallbacks *_cb,
        int _x,int _y)
{
    int ret;
    ret=glwCompSuperPassiveMotion(&_this->super,_cb,_x,_y);
    if (ret>=0)
    {
        DS3View *view;
        view=_this->ds3view;
        if (view->track_mp!=view->track_lp)
        {
            glwCompRepaint(&view->super,0);
            view->track_lp=view->track_mp;
        }
        ret=1;
    }
    return ret;
}

const GLWCallbacks DS3_VIEW_PTS_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    NULL,
    (GLWDisplayFunc)ds3ViewPointsPeerDisplay,
    NULL,
    NULL,
    NULL,
    (GLWFocusFunc)ds3ViewPointsPeerFocus,
    NULL,
    (GLWEntryFunc)ds3ViewPointsPeerEntry,
    (GLWKeyboardFunc)ds3ViewPointsPeerKeyboard,
    (GLWSpecialFunc)ds3ViewPointsPeerSpecial,
    (GLWMouseFunc)ds3ViewPointsPeerMouse,
    NULL,
    (GLWMotionFunc)ds3ViewPointsPeerPassiveMotion
};
