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
#include "ds3coord.hh"

# define DS3V_UNIT_ANGLE (5)                    /*Amount to turn per keystroke*/
# define DS3V_UNIT_DIST  (0.05)                 /*Amount to move per keystroke*/

/*Draws the coordinate axes*/
static void ds3ViewAxesPeerDisplay(DS3ViewComp *_this,
                                   const GLWCallbacks *_cb)
{
    DS3View  *view;
    GLdouble  b[2][3];
    int       i;
    int       j;
    int       k;
    GLWcolor  c;
    GLWcolor  nc;
    GLWcolor  fc = GLW_COLOR_RED;
    view=_this->ds3view;
    for (i=0; i<3; i++)b[0][i]=view->box[0][i]<0?view->box[0][i]:0;
    for (i=0; i<3; i++)b[1][i]=view->box[1][i]>1?view->box[1][i]:1;
    glPushMatrix();
    glMultMatrixd(view->basis);
    /*This part is supposed to be in ds3ViewBoxPeerDisplay(), but is here for
      z-buffer accuracy concerns*/
    nc=glwColorBlend(view->super.forec,view->super.backc);
    if (glwCompIsFocused(&view->cm_box.super))
    {
        fc=glwColorBlend(nc,DS3V_FOCUS_COLOR);
    }
    glBegin(GL_LINES);                                       /*Draw the box lines*/
    for (i=0,k=3; i<7; i++)
    {
        int x[3];
        for (j=0; j<3; j++)x[j]=i&1<<j?1:0;
        for (j=0; j<3; j++)if (!x[j])
            {
                if (glwCompIsFocused(&view->cm_box.super)&&view->track_pl>>1!=j&&
                        x[view->track_pl>>1]==(view->track_pl&1))c=fc;
                else c=nc;
                if (view->track_ax==k&&glwCompIsCapturing(&view->cm_axes.super))
                {
                    glwColor(glwColorBlend(c,DS3V_CAPTURE_COLOR));
                }
                else glwColor(c);
                glVertex3d(view->box[x[X]][X],view->box[x[Y]][Y],view->box[x[Z]][Z]);
                x[j]=1;
                glVertex3d(view->box[x[X]][X],view->box[x[Y]][Y],view->box[x[Z]][Z]);
                x[j]=0;
                k++;
            }
    }
    glEnd();
    c=view->super.forec;
    if (glwCompIsFocused(&_this->super))c=glwColorBlend(c,DS3V_FOCUS_COLOR);
    glDepthFunc(GL_ALWAYS);
    glwColor(c);
    glBegin(GL_LINES);
    for (i=0; i<3; i++)                                          /*Draw each axis*/
    {
        static const int next[3]={1,2,0};
        Vect3d   d;
        vectSet3d(d,0,0,0);
        d[i]=b[0][i];
        if (view->track_ax==i&&glwCompIsCapturing(&_this->super))
        {
            glwColor(glwColorBlend(view->super.forec,DS3V_CAPTURE_COLOR));
        }
        glVertex3dv(d);
        d[i]=b[1][i];
        glVertex3dv(d);
        for (j=next[i]; j!=i; j=next[j])                          /*Add arrow heads*/
        {
            d[i]-=0.05;
            d[j]+=0.05;
            glVertex3dv(d);
            d[i]+=0.05;
            d[j]-=0.05;
            glVertex3dv(d);
        }
        if (view->track_ax==i&&glwCompIsCapturing(&_this->super))
        {
            glwColor(c);
        }
    }
    glEnd();
    glDepthFunc(GL_LESS);
    glPopMatrix();
}

static void ds3ViewGetTrackCoords(DS3View *_this,int _x,int _y,Vect3d _track)
{
    if (_this->super.bounds.w>0&&_this->super.bounds.h>0)
    {
        double aspect;
        double dx;
        double dy;
        aspect=_this->super.bounds.w*DS3V_ASPECT/_this->super.bounds.h;
        dx=2*(_x+0.5)/_this->super.bounds.w-1;
        dy=2*(_y+0.5)/_this->super.bounds.h-1;
        if (aspect>=1)dx*=aspect;
        else dy/=aspect;
        switch (_this->proj)
        {
        case DS3V_PROJECT_ORTHOGRAPHIC:
        {
            double d;
            dx*=_this->zoom;
            dy*=_this->zoom;
            if (_this->track_rd>1E-100)
            {
                dx/=_this->track_rd;
                dy/=_this->track_rd;
            }
            d=1-dx*dx-dy*dy;
            if (d<0)
            {
                d=sqrt(1-d);
                vectSet3d(_track,dx/d,dy/d,0);
            }
            else vectSet3d(_track,dx,dy,_this->track_rt*sqrt(d));
        }
        break;
        /*case DS3V_PROJECT_PERSPECTIVE :*/
        default                       :
        {
            double a,b,c,d,e,z;
            z=_this->track_rd>1E-100?_this->zoom/_this->track_rd:_this->zoom;
            e=dx*dx+dy*dy;
            a=e+1;
            b=e*z;
            c=b*z-1;
            b*=-2;
            d=b*b-4*a*c;
            if (d<0)
            {
                double f;
                f=1-z*z;
                if (f>=0||!e)vectSet3d(_track,0,0,1);
                else
                {
                    c=sqrt(-e*f)/(z*e);
                    vectSet3d(_track,dx*c,dy*c,1/z);
                }
            }
            else
            {
                d=(-b+_this->track_rt*sqrt(d))/(2*a);
                vectSet3d(_track,dx*(z-d),dy*(z-d),d);
            }
        }
        }
    }
    else vectSet3d(_track,0,0,1);
}

/*Composes an additional rotation onto the given view orientation, and sets
  the current orientation to the result
  y:   The original yaw
  p:   The original pitch
  r:   The original roll
  rot: The rotation to compose after the given orientation*/
static void ds3ViewPostComposeRot(DS3View *_this,double _y,
                                  double _p,double _r,double _rot[3][3])
{
    double r[3][3];
    double s[3][3];
    double sx,cx,sy,cy,sz,cz;
    double x,y,z;
    int    i,j,k;
    ds3ViewExpandRot(_y,_p,_r,r);
    for (i=0; i<3; i++)for (j=0; j<3; j++)
        {
            s[i][j]=0;
            for (k=0; k<3; k++)s[i][j]+=_rot[i][k]*r[k][j];
        }
    sy=-s[2][0];
    if (sy<-1)sy=-1;
    else if (sy>1)sy=1;
    y=asin(sy)*(180/M_PI);
    if (y<1E-8)
    {
        if (y<-1E-4)y+=360;
        else y=0;
    }
    if ((fabs(_this->pitch-y)>90&&fabs(_this->pitch-y+360)>90&&
            fabs(_this->pitch-y-360)>90))
    {
        y=180-y;
        if (y<1E-8)
        {
            if (y<-1E-4)y+=360;
            else y=0;
        }
    }
    cy=cos(y*M_PI/180);
    if (fabs(cy)<1E-8)
    {
        sz=sin(_this->roll*(M_PI/180));
        cz=cos(_this->roll*(M_PI/180));
        sx=sy*cz*s[0][1]+sz*s[0][2];
        cx=sy*cz*s[0][2]-sz*s[0][1];
        if (sx<-1)sx=-1;
        else if (sx>1)sx=1;
        if (cx<-1)cx=-1;
        else if (cx>1)cx=1;
        x=atan2(sx,cx)*(180/M_PI);
        if (x<1E-8)
        {
            if (x<-1E-4)x+=360;
            else x=0;
        }
        z=_this->roll;
    }
    else
    {
        sx=s[2][1]/cy;
        cx=s[2][2]/cy;
        sz=s[1][0]/cy;
        cz=s[0][0]/cy;
        x=atan2(sx,cx)*(180/M_PI);
        z=atan2(sz,cz)*(180/M_PI);
        if (x<1E-8)
        {
            if (x<-1E-4)x+=360;
            else x=0;
        }
        if (z<1E-8)
        {
            if (z<-1E-4)z+=360;
            else z=0;
        }
    }
    ds3ViewSetOrientation(_this,x,y,z);
}

/*Redraws the axes to reflect keyboard focus*/
static void ds3ViewAxesPeerFocus(DS3ViewComp *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->ds3view->super,0);
}

/*Tracks the axis the mouse is over*/
static void ds3ViewAxesPeerEntry(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                 int _s)
{
    if (_s)_this->ds3view->track_lx=_this->ds3view->track_ax;
    else _this->ds3view->track_lx=-1;
    glwCompRepaint(&_this->ds3view->super,0);
}

/*Keyboard manipulation of the view*/
static int ds3ViewAxesPeerSpecial(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                  int _k,int _x,int _y)
{
    DS3View *view;
    int      axis;
    int      mods;
    int      ret;
    view=_this->ds3view;
    ret=-1;
    mods=glutGetModifiers();
    if (!(mods&GLUT_ACTIVE_CTRL))switch (_k)
        {
        case GLUT_KEY_LEFT     :
            axis=-2;
            break;
        case GLUT_KEY_RIGHT    :
            axis=2;
            break;
        case GLUT_KEY_UP       :
            axis=-1;
            break;
        case GLUT_KEY_DOWN     :
            axis=1;
            break;
        case GLUT_KEY_PAGE_UP  :
        {
            ds3ViewSetZoom(view,view->zoom-DS3V_UNIT_DIST*view->offs);
            axis=0;
        }
        break;
        case GLUT_KEY_PAGE_DOWN:
        {
            ds3ViewSetZoom(view,view->zoom+DS3V_UNIT_DIST*view->offs);
            axis=0;
        }
        break;
        case GLUT_KEY_HOME     :
        {
            ds3ViewSetOrientation(view,0,0,0);
            axis=0;
        }
        break;
        default                :
            ret=0;
        }
    else ret=0;
    if (ret<0)
    {
        if (axis)
        {
            double a;
            double s;
            double c;
            double r[3][3];
            if (mods&GLUT_ACTIVE_SHIFT)axis=-(axis<<1);
            if (axis<0)
            {
                axis=-axis;
                a=-DS3V_UNIT_ANGLE*M_PI/180;
            }
            else a=DS3V_UNIT_ANGLE*M_PI/180;
            s=sin(a);
            c=cos(a);
            if (axis==1)
            {
                r[0][0]=1;
                r[0][1]=0;
                r[0][2]=0;
                r[1][0]=0;
                r[1][1]=c;
                r[1][2]=-s;
                r[2][0]=0;
                r[2][1]=s;
                r[2][2]=c;
            }
            else if (axis==2)
            {
                r[0][0]=c;
                r[0][1]=0;
                r[0][2]=s;
                r[1][0]=0;
                r[1][1]=1;
                r[1][2]=0;
                r[2][0]=-s;
                r[2][1]=0;
                r[2][2]=c;
            }
            else
            {
                r[0][0]=c;
                r[0][1]=-s;
                r[0][2]=0;
                r[1][0]=s;
                r[1][1]=c;
                r[1][2]=0;
                r[2][0]=0;
                r[2][1]=0;
                r[2][2]=1;
            }
            ds3ViewPostComposeRot(view,view->yaw,view->pitch,view->roll,r);
        }
    }
    else
    {
        ret=-1;
        switch (_k)
        {
        case GLUT_KEY_LEFT     :
            axis=1;
            break;
        case GLUT_KEY_RIGHT    :
            axis=-1;
            break;
        case GLUT_KEY_DOWN     :
            axis=2;
            break;
        case GLUT_KEY_UP       :
            axis=-2;
            break;
        case GLUT_KEY_PAGE_DOWN:
            axis=3;
            break;
        case GLUT_KEY_PAGE_UP  :
            axis=-3;
            break;
        case GLUT_KEY_HOME     :
        {
            ds3ViewSetZoom(view,view->offs);
            if (view->ds3!=NULL)
            {
                ds3ViewSetCenter(view,view->ds3->center[0],
                                 view->ds3->center[1],view->ds3->center[2]);
            }
            else ds3ViewSetCenter(view,0.5,0.5,0.5);
            axis=0;
        }
        break;
        case GLUT_KEY_END      :
        {
            ds3ViewAlignOrientation(view);
            axis=0;
        }
        break;
        default                :
            ret=0;
        }
        if (ret<0)
        {
            if (axis)
            {
                Vect3d c;
                double d;
                d=view->zoom<0.05*view->offs?0.05*view->offs:view->zoom;
                if (axis<0)vectMul3d(c,view->rot[-1-axis],-DS3V_UNIT_DIST*d);
                else vectMul3d(c,view->rot[axis-1],DS3V_UNIT_DIST*d);
                vectAdd3d(c,c,view->cntr);
                ds3ViewSetCenter(view,c[X],c[Y],c[Z]);
            }
        }
        else ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    }
    return ret;
}

/*Mouse manipulation of the view*/
static int ds3ViewAxesPeerMouse(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0&&_b==GLUT_LEFT_BUTTON&&_s)
    {
        DS3View *view;
        Vect3d   p;
        Vect3d   q;
        int      i;
        view=_this->ds3view;
        vectSub3d(p,view->track_pt,view->cntr);
        for (i=0; i<3; i++)view->track_an[i]=vectDot3d(view->rot[i],p);
        view->track_rd=vectMag3d(view->track_an);
        if (view->track_rd<1E-16)vectSet3d(view->track_an,0,0,1);
        else vectMul3d(view->track_an,view->track_an,1/view->track_rd);
        view->track_rt=-1;
        ds3ViewGetTrackCoords(view,_x,_y,p);
        vectSub3d(p,p,view->track_an);
        view->track_rt=1;
        ds3ViewGetTrackCoords(view,_x,_y,q);
        vectSub3d(q,q,view->track_an);
        if (vectMag2_3d(p)<vectMag2_3d(q))view->track_rt=-1;
        view->track_r=view->roll;
        view->track_p=view->pitch;
        view->track_y=view->yaw;
        return 1;
    }
    return ret;
}

/*Allows dragging the mouse to change the view orientation*/
static int ds3ViewAxesPeerMotion(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                 int _x,int _y)
{
    int ret;
    ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
    if (ret>=0&&(_this->super.mouse_b&1<<GLUT_LEFT_BUTTON))
    {
        DS3View *view;
        Vect3d   p;
        Vect3d   axis;
        double   qs;
        view=_this->ds3view;
        ds3ViewGetTrackCoords(view,_x,_y,p);
        vectCross3d(axis,view->track_an,p);
        qs=vectMag3d(axis);
        if (qs<1E-8)
        {
            ds3ViewSetOrientation(view,view->track_y,
                                  view->track_p,view->track_r);
        }
        else
        {
            double qc;
            double q[3][3];
            int    i,j;
            vectMul3d(axis,axis,1/qs);
            if (qs>1)qs=1;
            qc=sqrt(1-qs*qs);
            if (vectDot3d(view->track_an,p)<0)qc=-qc;
            for (i=0; i<3; i++)
            {
                for (j=0; j<3; j++)q[i][j]=axis[i]*axis[j]*(1-qc);
                q[i][i]+=qc;
            }
            vectMul3d(axis,axis,qs);
            q[1][2]-=axis[0];
            q[2][1]+=axis[0];
            q[2][0]-=axis[1];
            q[0][2]+=axis[1];
            q[0][1]-=axis[2];
            q[1][0]+=axis[2];
            ds3ViewPostComposeRot(view,view->track_y,view->track_p,
                                  view->track_r,q);
        }
        return 1;
    }
    return ret;
}

/*Tracks the current axis the mouse is over*/
static int ds3ViewAxesPeerPassiveMotion(DS3ViewComp *_this,
                                        const GLWCallbacks *_cb,
                                        int _x,int _y)
{
    int ret;
    ret=glwCompSuperPassiveMotion(&_this->super,_cb,_x,_y);
    if (ret>=0)
    {
        DS3View *view;
        view=_this->ds3view;
        if (view->track_ax!=view->track_lx)
        {
            glwCompRepaint(&view->super,0);
            view->track_lx=view->track_ax;
        }
        ret=1;
    }
    return ret;
}

/*Displays the clip box*/
static void ds3ViewBoxPeerDisplay(DS3ViewComp *_this,
                                  const GLWCallbacks *_cb)
{
    DS3View  *view;
    view=_this->ds3view;
    glPushMatrix();
    glMultMatrixd(view->basis);
    if (glwCompIsFocused(&_this->super))
    {
        int xbit;
        int ybit;
        int zbit;
        zbit=1<<(view->track_pl>>1);
        zbit=zbit<<3|zbit;
        xbit=zbit>>1&7;
        ybit=zbit>>2&7;
        zbit=(view->track_pl&1)?(zbit&7):0;
        glwColor(glwColorBlend(glwColorBlend(view->super.forec,view->super.backc),
                               DS3V_FOCUS_COLOR)&0x3FFFFFFF);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glBegin(GL_QUADS);
        glVertex3d(view->box[zbit&1][X],view->box[(zbit&2)>>1][Y],
                   view->box[(zbit&4)>>2][Z]);
        zbit|=xbit;
        glVertex3d(view->box[zbit&1][X],view->box[(zbit&2)>>1][Y],
                   view->box[(zbit&4)>>2][Z]);
        zbit|=ybit;
        glVertex3d(view->box[zbit&1][X],view->box[(zbit&2)>>1][Y],
                   view->box[(zbit&4)>>2][Z]);
        zbit^=xbit;
        glVertex3d(view->box[zbit&1][X],view->box[(zbit&2)>>1][Y],
                   view->box[(zbit&4)>>2][Z]);
        glEnd();
    }
    glPopMatrix();
}

/*Redraws the clip box to reflect keyboard focus*/
static void ds3ViewBoxPeerFocus(DS3ViewComp *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->ds3view->super,0);
}

/*Keyboard manipulation of the clip box*/
static int ds3ViewBoxPeerSpecial(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                 int _k,int _x,int _y)
{
    DS3View *view;
    double   dx;
    int      ret;
    view=_this->ds3view;
    ret=-1;
    dx=0;
    switch (_k)
    {
    case GLUT_KEY_LEFT     :
    {
        view->track_pl+=2;
        if (view->track_pl>=6)view->track_pl=7-view->track_pl;
        glwCompRepaint(&view->super,0);
    }
    break;
    case GLUT_KEY_RIGHT    :
    {
        view->track_pl-=2;
        if (view->track_pl<0)view->track_pl=3-view->track_pl;
        glwCompRepaint(&view->super,0);
    }
    break;
    case GLUT_KEY_UP       :
        dx=DS3V_UNIT_DIST;
        break;
    case GLUT_KEY_PAGE_UP  :
        dx=10*DS3V_UNIT_DIST;
        break;
    case GLUT_KEY_DOWN     :
        dx=-DS3V_UNIT_DIST;
        break;
    case GLUT_KEY_PAGE_DOWN:
        dx=-10*DS3V_UNIT_DIST;
        break;
    case GLUT_KEY_HOME     :
    {
        dx=(view->track_pl&1)-
           view->box[view->track_pl&1][view->track_pl>>1];
    }
    break;
    case GLUT_KEY_END      :
    {
        dx=(view->track_pl&1)*3-1-
           view->box[view->track_pl&1][view->track_pl>>1];
    }
    break;
    default                :
        ret=0;
        break;
    }
    if (ret<0)
    {
        if (dx)
        {
            double box[2][3];
            vectSet3dv(box[0],view->box[0]);
            vectSet3dv(box[1],view->box[1]);
            box[view->track_pl&1][view->track_pl>>1]+=dx;
            ds3ViewSetBox(view,box[0][X],box[0][Y],box[0][Z],
                          box[1][X],box[1][Y],box[1][Z]);
        }
    }
    else ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    return ret;
}

const GLWCallbacks DS3_VIEW_AXES_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    NULL,
    (GLWDisplayFunc)ds3ViewAxesPeerDisplay,
    NULL,
    NULL,
    NULL,
    (GLWFocusFunc)ds3ViewAxesPeerFocus,
    NULL,
    (GLWEntryFunc)ds3ViewAxesPeerEntry,
    NULL,
    (GLWSpecialFunc)ds3ViewAxesPeerSpecial,
    (GLWMouseFunc)ds3ViewAxesPeerMouse,
    (GLWMotionFunc)ds3ViewAxesPeerMotion,
    (GLWMotionFunc)ds3ViewAxesPeerPassiveMotion
};

const GLWCallbacks DS3_VIEW_BOX_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    NULL,
    (GLWDisplayFunc)ds3ViewBoxPeerDisplay,
    NULL,
    NULL,
    NULL,
    (GLWFocusFunc)ds3ViewBoxPeerFocus,
    NULL,
    NULL,
    NULL,
    (GLWSpecialFunc)ds3ViewBoxPeerSpecial,
    NULL,
    NULL,
    NULL
};
