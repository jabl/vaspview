/*GL Widget Set - simple, portable OpenGL/GLUT widget set
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
#include <stdio.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glw.hh"

/*A slider component. Sliders may be horizontal or vertical, and may contain
  major and minor tick marks, and labels for specific values. A slider value
  may only be an integer, and any conversion to real values must be done by
  the application. Mouse tracking may be snapped to tick marks or the discrete
  values within a certain pixel radius (or always)*/

# define GLW_SLIDER_THUMB_WIDTH  (9)
# define GLW_SLIDER_THUMB_HEIGHT (12)
# define GLW_SLIDER_INSET        (4)
# define GLW_SLIDER_TICK_LENGTH  (6)
# define GLW_SLIDER_MIN_LENGTH   (32)
# define GLW_SLIDER_PRE_LENGTH   (192)

static int glwSliderGetXPos(GLWSlider *_this,int _val)
{
    int    range;
    double ppv;
    int    ret;
    range=_this->max-_this->min;
    if (!range)range=1;
    ppv=_this->track_rect.w/(double)range;
    ret=_this->track_rect.x+(int)(0.5+ppv*(_val-_this->min));
    if (ret>=_this->track_rect.x+_this->track_rect.w)
    {
        ret=_this->track_rect.x+_this->track_rect.w-1;
    }
    if (ret<_this->track_rect.x)ret=_this->track_rect.x;
    return ret;
}

static int glwSliderGetYPos(GLWSlider *_this,int _val)
{
    int    range;
    double ppv;
    int    ret;
    range=_this->max-_this->min;
    if (!range)range=1;
    ppv=_this->track_rect.h/range;
    ret=_this->track_rect.y+_this->track_rect.h-(int)(0.5+ppv*(_val-_this->min));
    if (ret>=_this->track_rect.y+_this->track_rect.h)
    {
        ret=_this->track_rect.y+_this->track_rect.h-1;
    }
    if (ret<_this->track_rect.y)ret=_this->track_rect.y;
    return ret;
}

static int glwSliderGetXValue(GLWSlider *_this,int _x)
{
    if (_x<=_this->track_rect.x)return _this->min;
    else if (_x>=_this->track_rect.x+_this->track_rect.w)return _this->max;
    else
    {
        double vpp;
        int    range;
        _x-=_this->track_rect.x;
        range=_this->track_rect.w;
        if (range<=0)range=1;
        vpp=(_this->max-_this->min)/(double)range;
        return (int)floor(_this->min+_x*vpp+0.5);
    }
}

static int glwSliderGetYValue(GLWSlider *_this,int _y)
{
    if (_y<=_this->track_rect.y)return _this->max;
    else if (_y>=_this->track_rect.y+_this->track_rect.h)return _this->min;
    else
    {
        double vpp;
        int    range;
        _y-=_this->track_rect.y;
        range=_this->track_rect.h;
        if (range<=0)range=1;
        vpp=(_this->min-_this->max)/(double)range;
        return (int)floor(_this->max+_y*vpp+0.5);
    }
}

static void glwSliderUpdateThumb(GLWSlider *_this)
{
    if (_this->ornt==GLWC_VERTICAL)
    {
        _this->thumb_rect.y=glwSliderGetYPos(_this,_this->val)-
                            (_this->thumb_rect.h>>1);
    }
    else
    {
        _this->thumb_rect.x=glwSliderGetXPos(_this,_this->val)-
                            (_this->thumb_rect.w>>1);
    }
    glwCompRepaint(&_this->super,0);
}

static void glwSliderMoveThumb(GLWSlider *_this,int _x,int _y)
{
    if (_this->ornt==GLWC_VERTICAL)
    {
        _this->thumb_rect.y=_y-_this->thumb_offs;
        if (_this->thumb_rect.y+(_this->thumb_rect.h>>1)>=
                _this->track_rect.y+_this->track_rect.h)
        {
            _this->thumb_rect.y=_this->track_rect.y+_this->track_rect.h-1-
                                (_this->thumb_rect.h>>1);
        }
        if (_this->thumb_rect.y+(_this->thumb_rect.h>>1)<_this->track_rect.y)
        {
            _this->thumb_rect.y=_this->track_rect.y-(_this->thumb_rect.h>>1);
        }
        _this->thumb_offs=_this->thumb_rect.h>>1;
    }
    else
    {
        _this->thumb_rect.x=_x-_this->thumb_offs;
        if (_this->thumb_rect.x+(_this->thumb_rect.w>>1)>=
                _this->track_rect.x+_this->track_rect.w)
        {
            _this->thumb_rect.x=_this->track_rect.x+_this->track_rect.w-1-
                                (_this->thumb_rect.w>>1);
        }
        if (_this->thumb_rect.x+(_this->thumb_rect.w>>1)<_this->track_rect.x)
        {
            _this->thumb_rect.x=_this->track_rect.x-(_this->thumb_rect.w>>1);
        }
        _this->thumb_offs=_this->thumb_rect.w>>1;
    }
    glwCompRepaint(&_this->super,0);
}

static int glwSliderCheckSnap(GLWSlider *_this,int _int,int _offs)
{
    int a;
    int b;
    int x;
    int u;
    int v;
    int d;
    x=_this->val-_this->min-_offs;
    if (x>=0)
    {
        a=_this->val-(x%_int);
        b=a+_int;
    }
    else
    {
        a=_this->val+(-x%_int);
        b=a-_int;
    }
    if (_this->ornt==GLWC_VERTICAL)
    {
        u=glwSliderGetYPos(_this,a);
        v=glwSliderGetYPos(_this,b);
        x=_this->thumb_rect.y+(_this->thumb_rect.h>>1);
    }
    else
    {
        u=glwSliderGetXPos(_this,a);
        v=glwSliderGetXPos(_this,b);
        x=_this->thumb_rect.x+(_this->thumb_rect.w>>1);
    }
    if (u>v)
    {
        d=a;
        a=b;
        b=d;
        d=u;
        u=v;
        v=u;
    }
    if ((_this->snap==GLWC_SNAP_ALWAYS||x-u<=_this->snap)&&v-x>=x-u)
    {
        glwSliderSetVal(_this,a,_this->ext);
        return 1;
    }
    else if (_this->snap==GLWC_SNAP_ALWAYS||v-x<=_this->snap)
    {
        glwSliderSetVal(_this,b,_this->ext);
        return 1;
    }
    return 0;
}

static void glwSliderSnap(GLWSlider *_this)
{
    if (_this->snap!=GLWC_SNAP_NEVER)
    {
        if (_this->major_ticks<=0||
                !glwSliderCheckSnap(_this,_this->major_ticks,_this->major_offs))
        {
            if (_this->minor_ticks<=0||
                    !glwSliderCheckSnap(_this,_this->minor_ticks,_this->minor_offs))
            {
                if (!glwSliderCheckSnap(_this,1,0))return;
            }
        }
        glwSliderUpdateThumb(_this);
    }
}

static void glwSliderLayout(GLWLayoutManager *_this,GLWSlider *_slider)
{
    if (_slider->ornt==GLWC_VERTICAL)
    {
        int           h;
        _slider->thumb_rect.w=GLW_SLIDER_THUMB_HEIGHT;
        _slider->thumb_rect.h=GLW_SLIDER_THUMB_WIDTH;
        if (_slider->labels.size() > 0 && _slider->center_labels)
        {
            _slider->track_offs=(glwFontGetHeight(_slider->super.font)>>1)+
                                GLW_SLIDER_INSET;
        }
        else _slider->track_offs=GLW_SLIDER_INSET;
        h=_slider->thumb_rect.h>>1;
        if (h>_slider->track_offs)_slider->track_offs=h;
        _slider->track_rect.x=GLW_SLIDER_INSET;
        _slider->track_rect.y=_slider->track_offs;
        _slider->track_rect.w=_slider->thumb_rect.w;
        _slider->track_rect.h=_slider->super.bounds.h-(_slider->track_offs<<1);
        _slider->tick_rect.x=_slider->track_rect.x+_slider->track_rect.w;
        _slider->tick_rect.y=_slider->track_rect.y;
        if (_slider->major_ticks<=0&&_slider->minor_ticks<=0)_slider->tick_rect.w=0;
        else _slider->tick_rect.w=GLW_SLIDER_TICK_LENGTH;
        _slider->tick_rect.h=_slider->track_rect.h;
        _slider->label_rect.x=_slider->tick_rect.x+_slider->tick_rect.w;
        _slider->label_rect.y=_slider->tick_rect.y-_slider->track_offs+
                              GLW_SLIDER_INSET;
        _slider->label_rect.w=0;
	for (auto it = _slider->labels.begin(); 
	     it != _slider->labels.end(); ++it)
        {
		std::string& lblp = (*it).second;
		int w = glwFontGetStringWidth(_slider->super.font, lblp.c_str());
	    if (w>_slider->label_rect.w)_slider->label_rect.w=w;
        }
        _slider->label_rect.h=_slider->tick_rect.h+
                              (_slider->track_offs-GLW_SLIDER_INSET<<1);
        _slider->thumb_rect.x=_slider->track_rect.x;
        _slider->thumb_rect.y=glwSliderGetYPos(_slider,_slider->val)-
                              (_slider->thumb_rect.h+1>>1);
    }
    else
    {
        int w;
        _slider->thumb_rect.w=GLW_SLIDER_THUMB_WIDTH;
        _slider->thumb_rect.h=GLW_SLIDER_THUMB_HEIGHT;
        if (!_slider->labels.empty() && _slider->center_labels)
        {
		auto it = _slider->labels.find(_slider->label_lo);
		if (it != _slider->labels.end())
		{
			std::string& lblp = (*it).second;
			_slider->track_offs=(glwFontGetStringWidth(_slider->super.font,lblp.c_str())>>1)+
                                    GLW_SLIDER_INSET;
		}
		it = _slider->labels.find(_slider->label_hi);
		if (it != _slider->labels.end())
		{
			std::string& lblp= (*it).second;
			w=(glwFontGetStringWidth(_slider->super.font,lblp.c_str())>>1)+GLW_SLIDER_INSET;
		}
            if (w>_slider->track_offs)_slider->track_offs=w;
        }
        else _slider->track_offs=GLW_SLIDER_INSET;
        w=_slider->thumb_rect.w>>1;
        if (w>_slider->track_offs)_slider->track_offs=w;
        _slider->track_rect.x=_slider->track_offs;
        _slider->track_rect.y=_slider->super.bounds.h-GLW_SLIDER_INSET-
                              _slider->thumb_rect.h;
        _slider->track_rect.w=_slider->super.bounds.w-(_slider->track_offs<<1);
        _slider->track_rect.h=_slider->thumb_rect.h;
        _slider->tick_rect.x=_slider->track_rect.x;
        _slider->tick_rect.y=_slider->track_rect.y;
        _slider->tick_rect.w=_slider->track_rect.w;
        if (_slider->major_ticks<=0&&_slider->minor_ticks<=0)_slider->tick_rect.h=0;
        else _slider->tick_rect.h=GLW_SLIDER_TICK_LENGTH;
        _slider->tick_rect.y-=_slider->tick_rect.h;
        _slider->label_rect.x=_slider->tick_rect.x-_slider->track_offs+
                              GLW_SLIDER_INSET;
        _slider->label_rect.y=_slider->tick_rect.y;
        _slider->label_rect.w=_slider->tick_rect.w+
                              (_slider->track_offs-GLW_SLIDER_INSET<<1);
        if (_slider->labels.size() > 0)
        {
            _slider->label_rect.h=glwFontGetHeight(_slider->super.font);
        }
        else _slider->label_rect.h=0;
        _slider->label_rect.y-=_slider->label_rect.h;
        _slider->thumb_rect.x=glwSliderGetXPos(_slider,_slider->val)-
                              (_slider->thumb_rect.w>>1);
        _slider->thumb_rect.y=_slider->track_rect.y;
    }
}

static void glwSliderLayoutMinSize(GLWLayoutManager *_this,GLWSlider *_slider,
                                   int *_w,int *_h)
{
    int w;
    int h;
    if (!_slider->super.valid)glwSliderLayout(_this,_slider);
    if (_slider->ornt==GLWC_VERTICAL)
    {
        w=(GLW_SLIDER_INSET<<1)+_slider->track_rect.w+
          _slider->tick_rect.w+_slider->label_rect.w;
        h=(_slider->track_offs<<1)+GLW_SLIDER_MIN_LENGTH;
    }
    else
    {
        w=(_slider->track_offs<<1)+GLW_SLIDER_MIN_LENGTH;
        h=(GLW_SLIDER_INSET<<1)+_slider->track_rect.h+
          _slider->tick_rect.h+_slider->label_rect.h;
    }
    if (_w!=NULL)*_w=w;
    if (_h!=NULL)*_h=h;
}

static void glwSliderLayoutPreSize(GLWLayoutManager *_this,GLWSlider *_slider,
                                   int *_w,int *_h)
{
    int w;
    int h;
    if (!_slider->super.valid)glwSliderLayout(_this,_slider);
    if (_slider->ornt==GLWC_VERTICAL)
    {
        w=(GLW_SLIDER_INSET<<1)+_slider->track_rect.w+
          _slider->tick_rect.w+_slider->label_rect.w;
        h=(_slider->track_offs<<1)+GLW_SLIDER_PRE_LENGTH;
    }
    else
    {
        w=(_slider->track_offs<<1)+GLW_SLIDER_PRE_LENGTH;
        h=(GLW_SLIDER_INSET<<1)+_slider->track_rect.h+
          _slider->tick_rect.h+_slider->label_rect.h;
    }
    if (_w!=NULL)*_w=w;
    if (_h!=NULL)*_h=h;
}


static GLWLayoutManager glw_slider_layout=
{
    (GLWLayoutFunc)glwSliderLayout,
    NULL,
    (GLWLayoutSizeFunc)glwSliderLayoutMinSize,
    (GLWLayoutSizeFunc)glwSliderLayoutPreSize,
    NULL,
    NULL
};


void glwSliderPeerDisplay(GLWSlider *_this,GLWCallbacks *_cb)
{
    int fc;
    int bc;
    int sc;
    int hc;
    glwCompSuperDisplay(&_this->super,_cb);
    if (glwCompIsEnabled(&_this->super))fc=_this->super.forec;
    else fc=glwColorBlend(_this->super.forec,_this->super.backc);
    bc=_this->super.backc;
    sc=glwColorDarken(bc);
    hc=glwColorLighten(bc);
    if (_this->ornt==GLWC_VERTICAL)
    {
        int cx,cy,ch;
        cx=_this->track_rect.x+(_this->track_rect.w>>1)-2;   /*Draw the slider track*/
        cy=_this->track_offs;
        ch=_this->track_rect.h;
        glBegin(GL_LINES);
        glwColor(sc);
        glVertex2i(cx,cy+ch);
        glVertex2i(cx,cy+1);
        glVertex2i(cx+1,cy+ch);
        glVertex2i(cx+2,cy+ch);
        glwColor(hc);
        glVertex2i(cx+3,cy+ch);
        glVertex2i(cx+3,cy);
        glVertex2i(cx,cy);
        glVertex2i(cx+3,cy);
        glwColor(fc);
        glVertex2i(cx+1,cy+ch-1);
        glVertex2i(cx+1,cy+2);
        cx=_this->tick_rect.x;
        if (_this->minor_ticks>0)                        /*Draw the minor tick marks*/
        {
            int v;
            glBegin(GL_LINES);
            glwColor(fc);
            v=_this->minor_offs;
            if (v<0)
            {
                v=-(-v%_this->minor_ticks);
                if (v)v=_this->minor_ticks-v;
            }
            else v=v%_this->minor_ticks;
            if (_this->min<=_this->max)
            {
                for (v+=_this->min; v<=_this->max; v+=_this->minor_ticks)
                {
                    cy=glwSliderGetYPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx+(_this->tick_rect.w>>1)-1,cy);
                }
            }
            else
            {
                for (v=_this->min-v; v>=_this->max; v-=_this->minor_ticks)
                {
                    cy=glwSliderGetYPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx+(_this->tick_rect.h>>1)-1,cy);
                }
            }
            glEnd();
        }
        if (_this->major_ticks>0)                        /*Draw the major tick marks*/
        {
            int v;
            glBegin(GL_LINES);
            glwColor(fc);
            v=_this->major_offs;
            if (v<0)
            {
                v=-(-v%_this->major_ticks);
                if (v)v=_this->major_ticks-v;
            }
            else v=v%_this->major_ticks;
            if (_this->min<=_this->max)
            {
                for (v=+_this->min; v<=_this->max; v+=_this->major_ticks)
                {
                    cy=glwSliderGetYPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx+_this->tick_rect.w-2,cy);
                }
            }
            else
            {
                for (v=_this->min-v; v>=_this->max; v-=_this->major_ticks)
                {
                    cy=glwSliderGetYPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx+_this->tick_rect.w-2,cy);
                }
            }
            glEnd();
        }
        if (_this->labels.size() > 0)                                  /*Draw the labels*/
        {
            double          dy;
            int             ch;
            ch=glwFontGetHeight(_this->super.font);
            cx=_this->label_rect.x+GLW_SLIDER_INSET;
            dy=glwFontGetDescent(_this->super.font);
            //hiInit(&ci,&_this->labels);
            //while (hiInc(&ci))
	    for (auto it = _this->labels.begin(); 
		 it != _this->labels.end(); ++it)
            {
		    int vp= (*it).first;
		    std::string& lblp = (*it).second;
                    double cy;
                    cy=glwSliderGetYPos(_this,vp)-0.5*ch;
                    if (cy<_this->label_rect.y)cy=_this->label_rect.y;
                    if (cy+ch>_this->label_rect.y+_this->label_rect.h)
                    {
                        cy=_this->label_rect.y+_this->label_rect.h-ch;
                    }
                    glwFontDrawString(_this->super.font,lblp.c_str(),cx,cy+dy);
	    }
        }
    }
    else
    {
        int cx,cy,cw;
        cx=_this->track_offs;                                /*Draw the slider track*/
        cy=_this->track_rect.y+(_this->track_rect.h+1>>1)+2;
        cw=_this->track_rect.w;
        glBegin(GL_LINES);
        glwColor(sc);
        glVertex2i(cx,cy);
        glVertex2i(cx+cw-1,cy);
        glVertex2i(cx,cy-1);
        glVertex2i(cx,cy-2);
        glwColor(hc);
        glVertex2i(cx,cy-3);
        glVertex2i(cx+cw,cy-3);
        glVertex2i(cx+cw,cy);
        glVertex2i(cx+cw,cy-3);
        glwColor(fc);
        glVertex2i(cx+1,cy-1);
        glVertex2i(cx+cw-2,cy-1);
        glEnd();
        cy=_this->tick_rect.y+_this->tick_rect.h;
        if (_this->minor_ticks>0)                        /*Draw the minor tick marks*/
        {
            int v;
            glBegin(GL_LINES);
            glwColor(fc);
            v=_this->minor_offs;
            if (v<0)
            {
                v=-(-v%_this->minor_ticks);
                if (v)v=_this->minor_ticks-v;
            }
            else v=v%_this->minor_ticks;
            if (_this->min<=_this->max)
            {
                for (v+=_this->min; v<=_this->max; v+=_this->minor_ticks)
                {
                    cx=glwSliderGetXPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx,cy-(_this->tick_rect.h>>1)+1);
                }
            }
            else
            {
                for (v=_this->min-v; v>=_this->max; v-=_this->minor_ticks)
                {
                    cx=glwSliderGetXPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx,cy-(_this->tick_rect.h>>1)+1);
                }
            }
            glEnd();
        }
        if (_this->major_ticks>0)                        /*Draw the major tick marks*/
        {
            int v;
            glBegin(GL_LINES);
            glwColor(fc);
            v=_this->major_offs;
            if (v<0)
            {
                v=-(-v%_this->major_ticks);
                if (v)v=_this->major_ticks-v;
            }
            else v=v%_this->major_ticks;
            if (_this->min<=_this->max)
            {
                for (v+=_this->min; v<=_this->max; v+=_this->major_ticks)
                {
                    cx=glwSliderGetXPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx,cy-_this->tick_rect.h+2);
                }
            }
            else
            {
                for (v=_this->min-v; v>=_this->max; v-=_this->major_ticks)
                {
                    cx=glwSliderGetXPos(_this,v);
                    glVertex2i(cx,cy);
                    glVertex2i(cx,cy-_this->tick_rect.h+2);
                }
            }
            glEnd();
        }
        if (_this->labels.size() > 0)                                  /*Draw the labels*/
        {
            cy=_this->label_rect.y+glwFontGetDescent(_this->super.font);
            //hiInit(&ci,&_this->labels);
            //while (hiInc(&ci))
	    for (auto it = _this->labels.begin(); 
		 it != _this->labels.end(); ++it)
            {
		    int vp= (*it).first;
		    std::string& lblp = (*it).second;
                    double cx;
                    double cw;
                    cw=glwFontGetStringWidth(_this->super.font,lblp.c_str());
                    cx=glwSliderGetXPos(_this,vp)-0.5*cw;
                    if (cx+cw>_this->label_rect.x+_this->label_rect.w)
                    {
                        cx=_this->label_rect.x+_this->label_rect.w-cw;
                    }
                    if (cx<_this->label_rect.x)cx=_this->label_rect.x;
                    glwFontDrawString(_this->super.font,lblp.c_str(),cx,cy);
	    }
        }
    }
    if (glwCompIsFocused(&_this->super))               /*Draw the focus rectangle*/
    {
        glLineStipple(2,0x5555);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_LOOP);
        glwColor(glwColorBlend(fc,bc));
        glVertex2d(GLW_SLIDER_INSET>>1,GLW_SLIDER_INSET>>1);
        glVertex2d(_this->super.bounds.w-(GLW_SLIDER_INSET>>1)-1,
                   GLW_SLIDER_INSET>>1);
        glVertex2d(_this->super.bounds.w-(GLW_SLIDER_INSET>>1)-1,
                   _this->super.bounds.h-(GLW_SLIDER_INSET>>1)-1);
        glVertex2d(GLW_SLIDER_INSET>>1,
                   _this->super.bounds.h-(GLW_SLIDER_INSET>>1)-1);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }
    if (!glwCompIsEnabled(&_this->super))
    {
        bc=fc;
        sc=glwColorDarken(bc);
        hc=glwColorLighten(bc);
    }
    /*glEnable(GL_LINE_SMOOTH);*/
    if (_this->major_ticks<=0&&_this->minor_ticks<=0)    /*Draw un-oriented thumb*/
    {
        GLWRect *r;
        r=&_this->thumb_rect;
        glwColor(bc);
        glRecti(r->x,r->y,r->x+r->w,r->y+r->h);
        glBegin(GL_LINES);
        glwColor(sc);
        glVertex2i(r->x,r->y);
        glVertex2i(r->x+r->w-1,r->y);
        glVertex2i(r->x+r->w-1,r->y+r->h-1);
        glVertex2i(r->x+r->w-1,r->y);
        glwColor(hc);
        glVertex2i(r->x,r->y+r->h-1);
        glVertex2i(r->x,r->y+1);
        glVertex2i(r->x+1,r->y+r->h-1);
        glVertex2i(r->x+r->w-1,r->y+r->h-1);
        glEnd();
    }
    else if (_this->ornt==GLWC_VERTICAL)              /*Draw right-pointing thumb*/
    {
        GLWRect *r;
        double   ch;
        r=&_this->thumb_rect;
        ch=r->h*0.5;
        glwColor(bc);
        glBegin(GL_POLYGON);
        glVertex2i(r->x,r->y+r->h-1);
        glVertex2d(r->x+r->w-ch-1,r->y+r->h-1);
        glVertex2d(r->x+r->w-1,r->y+ch);
        glVertex2d(r->x+r->w-ch-1,r->y+1);
        glVertex2i(r->x+1,r->y+1);
        glEnd();
        glBegin(GL_LINES);
        glwColor(sc);
        glVertex2i(r->x,r->y);
        glVertex2d(r->x+r->w-ch-1,r->y);
        glVertex2d(r->x+r->w-ch-1,r->y);
        glVertex2d(r->x+r->w-1,r->y+ch);
        glwColor(hc);
        glVertex2i(r->x,r->y+r->h-1);
        glVertex2i(r->x,r->y+1);
        glVertex2i(r->x+1,r->y+r->h-1);
        glVertex2d(r->x+r->w-ch-1,r->y+r->h-1);
        glVertex2d(r->x+r->w-ch-1,r->y+r->h-1);
        glVertex2d(r->x+r->w-1,r->y+ch+1);
        glEnd();
    }
    else                                               /*Draw down-pointing thumb*/
    {
        GLWRect *r;
        double   cw;
        r=&_this->thumb_rect;
        cw=r->w*0.5;
        glwColor(bc);
        glBegin(GL_POLYGON);
        glVertex2i(r->x,r->y+r->h-2);
        glVertex2d(r->x,r->y+cw-1);
        glVertex2d(r->x+cw,r->y);
        glVertex2d(r->x+r->w-1,r->y+cw-1);
        glVertex2i(r->x+r->w-1,r->y+r->h-2);
        glEnd();
        glBegin(GL_LINES);
        glwColor(sc);
        glVertex2i(r->x+r->w-1,r->y+r->h-1);
        glVertex2d(r->x+r->w-1,r->y+cw-1);
        glVertex2d(r->x+r->w-1,r->y+cw-1);
        glVertex2d(r->x+cw,r->y);
        glwColor(hc);
        glVertex2i(r->x,r->y+r->h-1);
        glVertex2i(r->x+r->w-1,r->y+r->h-1);
        glVertex2i(r->x,r->y+r->h-1);
        glVertex2d(r->x,r->y+cw);
        glVertex2d(r->x,r->y+cw);
        glVertex2d(r->x+cw-1,r->y+1);
        glEnd();
    }
}

static void glwSliderPeerEnable(GLWSlider *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperEnable(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static void glwSliderPeerFocus(GLWSlider *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static void glwSliderScrollByBlock(GLWSlider *_this,int _dir)
{
    int amt;
    if (_this->major_ticks>0)amt=_this->major_ticks;
    else if (_this->minor_ticks>0)amt=_this->minor_ticks;
    else
    {
        amt=_this->min<=_this->max?_this->max-_this->min:_this->min-_this->max;
        amt/=10;
        if (amt<=0)amt=1;
    }
    glwSliderSetVal(_this,_this->val+amt*_dir,_this->ext);
}

static int glwSliderPeerSpecial(GLWSlider *_this,GLWCallbacks *_cb,
                                int _k,int _x,int _y)
{
    int ret;
    if (!_this->super.mouse_b)
    {
        int dir;
        dir=_this->min<=_this->max?-1:1;
        ret=-1;
        switch (_k)
        {
        case GLUT_KEY_LEFT     :
        case GLUT_KEY_DOWN     :
        {
            glwSliderSetVal(_this,_this->val+dir,_this->ext);
        }
        break;
        case GLUT_KEY_RIGHT    :
        case GLUT_KEY_UP       :
        {
            glwSliderSetVal(_this,_this->val-dir,_this->ext);
        }
        break;
        case GLUT_KEY_PAGE_DOWN:
            glwSliderScrollByBlock(_this,dir);
            break;
        case GLUT_KEY_PAGE_UP  :
            glwSliderScrollByBlock(_this,-dir);
            break;
        case GLUT_KEY_HOME     :
            glwSliderSetVal(_this,_this->min,_this->ext);
            break;
        case GLUT_KEY_END      :
            glwSliderSetVal(_this,_this->max,_this->ext);
            break;
        default                :
            ret=0;
        }
    }
    else ret=0;
    if (!ret)ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    return ret;
}

static int glwSliderPeerMouse(GLWSlider *_this,GLWCallbacks *_cb,
                              int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0)
    {
        if (_s)
        {
            if (_this->thumb_rect.x<=_x&&_this->thumb_rect.x+_this->thumb_rect.w>_x&&
                    _this->thumb_rect.y<=_y&&_this->thumb_rect.y+_this->thumb_rect.h>_y)
            {
                if (_this->ornt==GLWC_VERTICAL)_this->thumb_offs=_y-_this->thumb_rect.y;
                else _this->thumb_offs=_x-_this->thumb_rect.x;
            }
            else
            {
                if (_this->ornt==GLWC_VERTICAL)
                {
                    int dir;
                    if (_this->thumb_rect.y<_y)dir=_this->min<=_this->max?1:-1;
                    else dir=_this->min<=_this->max?-1:1;
                    glwSliderSetVal(_this,glwSliderGetYValue(_this,_y),_this->ext);
                    _this->thumb_offs=_this->thumb_rect.h>>1;
                }
                else
                {
                    int dir;
                    if (_this->thumb_rect.x<_x)dir=_this->min<=_this->max?-1:1;
                    else dir=_this->min<=_this->max?1:-1;
                    glwSliderSetVal(_this,glwSliderGetXValue(_this,_x),_this->ext);
                    _this->thumb_offs=_this->thumb_rect.w>>1;
                }
                glwSliderMoveThumb(_this,_x,_y);
            }
        }
        glwSliderSnap(_this);
        ret=-1;
    }
    return ret;
}

static int glwSliderPeerMotion(GLWSlider *_this,GLWCallbacks *_cb,
                               int _x,int _y)
{
    int ret;
    ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
    if (ret>=0)
    {
        if (_this->ornt==GLWC_VERTICAL)
        {
            glwSliderSetVal(_this,glwSliderGetYValue(_this,_y-_this->thumb_offs+
                            (_this->thumb_rect.h>>1)),
                            _this->ext);
        }
        else
        {
            glwSliderSetVal(_this,glwSliderGetXValue(_this,_x-_this->thumb_offs+
                            (_this->thumb_rect.w>>1)),
                            _this->ext);
        }
        glwSliderMoveThumb(_this,_x,_y);
        glwSliderSnap(_this);
        ret=-1;
    }
    return ret;
}

static void glwSliderPeerDispose(GLWSlider *_this,GLWCallbacks *_cb)
{
    glwCompSuperDispose(&_this->super,_cb);
    _this->labels.clear();
}


const GLWCallbacks GLW_SLIDER_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwSliderPeerDispose,
    (GLWDisplayFunc)glwSliderPeerDisplay,
    NULL,
    NULL,
    (GLWEnableFunc)glwSliderPeerEnable,
    (GLWFocusFunc)glwSliderPeerFocus,
    NULL,
    NULL,
    NULL,
    (GLWSpecialFunc)glwSliderPeerSpecial,
    (GLWMouseFunc)glwSliderPeerMouse,
    (GLWMotionFunc)glwSliderPeerMotion,
    NULL
};


GLWSlider *glwSliderAlloc(int _min,int _max,int _val,int _ext)
{
	// Must be allocated by new in order to run constructors for labels map
	// TODO: Fix  destruction to use delete instead of free()
	GLWSlider *this_ = new GLWSlider();
    if (this_!=NULL)glwSliderInit(this_,_min,_max,_val,_ext);
    return this_;
}

void glwSliderInit(GLWSlider *_this,int _min,int _max,int _val,int _ext)
{
    glwCompInit(&_this->super);
    _this->super.callbacks=&GLW_SLIDER_CALLBACKS;
    glwCompSetLayout(&_this->super,&glw_slider_layout);
    glwCompSetFocusable(&_this->super,1);
    _this->changed=NULL;
    _this->changed_ctx=NULL;
    _this->major_ticks=0;
    _this->major_offs=0;
    _this->minor_ticks=0;
    _this->minor_offs=0;
    _this->ornt=GLWC_HORIZONTAL;
    _this->snap=GLWC_SNAP_NEVER;
    _this->center_labels=0;
    _this->min=_min;
    _this->max=_max;
    glwSliderSetVal(_this,_val,_ext);
}

void glwSliderDstr(GLWSlider *_this)
{
    glwCompDstr(&_this->super);
}

void glwSliderFree(GLWSlider *_this)
{
    glwCompFree(&_this->super);
}


int glwSliderIsCenteringLabels(GLWSlider *_this)
{
    return _this->center_labels;
}

void glwSliderSetCenteringLabels(GLWSlider *_this,int _b)
{
    if (_this->center_labels!=(_b?1U:0U))
    {
        _this->center_labels=_b?1:0;
        if (_this->labels.size() > 0) glwCompRepaint(&_this->super,0);
    }
}

int glwSliderGetMin(GLWSlider *_this)
{
    return _this->min;
}

int glwSliderGetMax(GLWSlider *_this)
{
    return _this->max;
}

void glwSliderSetRange(GLWSlider *_this,int _min,int _max)
{
    if (_this->min!=_min||_this->max!=_max)
    {
        _this->min=_min;
        _this->max=_max;
        glwSliderSetVal(_this,_this->val,_this->ext);
        glwSliderUpdateThumb(_this);
    }
}

int glwSliderGetVal(GLWSlider *_this)
{
    return _this->val;
}

int glwSliderGetExt(GLWSlider *_this)
{
    return _this->ext;
}

void glwSliderSetVal(GLWSlider *_this,int _val,int _ext)
{
    if (_this->min<=_this->max)
    {
        if (_ext>=0)
        {
            if (_val+_ext>_this->max)_val=_this->max-_ext;
            if (_val<_this->min)_val=_this->min;
            if (_val+_ext>_this->max)_ext=_this->max-_val;
        }
        else
        {
            if (_val+_ext<_this->min)_val=_this->min-_ext;
            if (_val>_this->max)_val=_this->max;
            if (_val+_ext<_this->min)_ext=_this->min-_val;
        }
    }
    else
    {
        if (_ext>=0)
        {
            if (_val+_ext>_this->min)_val=_this->min-_ext;
            if (_val<_this->max)_val=_this->max;
            if (_val+_ext>_this->min)_ext=_this->min-_val;
        }
        else
        {
            if (_val+_ext<_this->max)_val=_this->max-_ext;
            if (_val>_this->min)_val=_this->min;
            if (_val+_ext<_this->max)_ext=_this->max-_val;
        }
    }
    if (_val!=_this->val||_ext!=_this->ext)
    {
        _this->val=_val;
        _this->ext=_ext;
        if (_this->changed!=NULL)_this->changed(_this->changed_ctx,&_this->super);
        glwSliderUpdateThumb(_this);
    }
}

int glwSliderGetMajorTickSpacing(GLWSlider *_this)
{
    return _this->major_ticks;
}

void glwSliderSetMajorTickSpacing(GLWSlider *_this,int _s)
{
    if (_s!=_this->major_ticks)
    {
        _this->major_ticks=_s;
        glwCompRevalidate(&_this->super);
    }
}

int glwSliderGetMajorTickOffset(GLWSlider *_this)
{
    return _this->major_offs;
}

void glwSliderSetMajorTickOffset(GLWSlider *_this,int _o)
{
    if (_o!=_this->major_offs)
    {
        _this->major_offs=_o;
        glwCompRepaint(&_this->super,0);
    }
}

int glwSliderGetMinorTickSpacing(GLWSlider *_this)
{
    return _this->minor_ticks;
}

void glwSliderSetMinorTickSpacing(GLWSlider *_this,int _s)
{
    if (_s!=_this->minor_ticks)
    {
        _this->minor_ticks=_s;
        glwCompRevalidate(&_this->super);
    }
}

int glwSliderGetMinorTickOffset(GLWSlider *_this)
{
    return _this->minor_offs;
}

void glwSliderSetMinorTickOffset(GLWSlider *_this,int _o)
{
    if (_o!=_this->minor_offs)
    {
        _this->minor_offs=_o;
        glwCompRepaint(&_this->super,0);
    }
}

int glwSliderGetSnap(GLWSlider *_this)
{
    return _this->snap;
}

void glwSliderSetSnap(GLWSlider *_this,int _s)
{
    _this->snap=_s;
}

int glwSliderGetOrientation(GLWSlider *_this)
{
    return _this->ornt;
}

void glwSliderSetOrientation(GLWSlider *_this,int _o)
{
    if (_o!=_this->ornt)
    {
        _this->ornt=_o;
        glwCompRevalidate(&_this->super);
    }
}

int glwSliderAddLabel(GLWSlider *_this,int _val,const char *_label)
{
    if (_label==NULL)return glwSliderDelLabel(_this,_val);
    else
    {
	    _this->labels.erase(_val);
	    std::string tmp(_label);
	    _this->labels.insert(std::pair<int, std::string>(_val, tmp));
        // size_t   len;
        // char    *lbl;
        // char   **lblp;
        // len=strlen(_label)+1;
        // lbl=(char *)malloc(len*sizeof(char));
        // lblp=static_cast<char**>(htGet(&_this->labels,&_val));
        // if (lblp!=NULL)
        // {
        //     free(*lblp);
        //     *lblp=lbl;
        // }
        // else if (htIns(&_this->labels,&_val,&lbl)==NULL)
        // {
        //     free(lbl);
        //     return 0;
        // }
	    if (_this->labels.size() == 1||_this->label_lo>_val)_this->label_lo=_val;
	    if (_this->labels.size() == 1||_this->label_hi<_val)_this->label_hi=_val;
        //memcpy(lbl,_label,len);
        glwCompRevalidate(&_this->super);
        return 1;
    }
}

int glwSliderDelLabel(GLWSlider *_this,int _val)
{
	_this->labels.erase(_val);
        if (_val==_this->label_lo||_val==_this->label_hi)
        {
            _this->label_lo=INT_MAX;
            _this->label_hi=INT_MIN;
	    for (auto it = _this->labels.begin(); 
		 it != _this->labels.end(); ++it)
            {
		    int v= (*it).first;
                    if (v < _this->label_lo) _this->label_lo=v;
                    if (v > _this->label_hi) _this->label_hi=v;
	    }
        }
        glwCompRevalidate(&_this->super);
        return 1;
}

int glwSliderMakeLabels(GLWSlider *_this,int _start,int _inc)
{
    int  i;
    int  s;
    if (!_inc)return 0;
    if (_inc>0)
    {
        if (_this->min<_this->max)
        {
            s=_this->max;
            if (_start<_this->min)_start+=(_this->min-_start+_inc-1)/_inc*_inc;
        }
        else
        {
            s=_this->min;
            if (_start<_this->max)_start+=(_this->max-_start+_inc-1)/_inc*_inc;
        }
    }
    else
    {
        if (_this->min<_this->max)
        {
            s=_this->min;
            if (_start>_this->max)_start-=(_start-_this->max+_inc-1)/_inc*_inc;
        }
        else
        {
            s=_this->max;
            if (_start>_this->max)_start-=(_start-_this->max+_inc-1)/_inc*_inc;
        }
        i=s;
        s=_start;
        _start=i;
        _inc=-_inc;
    }
    for (i=_start; i<=s; i+=_inc)
    {
        char n[32];
        sprintf(n,"%i",i);
        if (!glwSliderAddLabel(_this,i,n))
        {
            int j;
            for (j=_start; j<i; j+=_inc)glwSliderDelLabel(_this,j);
            return 0;
        }
    }
    return 1;
}

GLWActionFunc glwSliderGetChangedFunc(GLWSlider *_this)
{
    return _this->changed;
}

void glwSliderSetChangedFunc(GLWSlider *_this,GLWActionFunc _func)
{
    _this->changed=_func;
}

void *glwSliderGetChangedCtx(GLWSlider *_this)
{
    return _this->changed_ctx;
}

void glwSliderSetChangedCtx(GLWSlider *_this,void *_ctx)
{
    _this->changed_ctx=_ctx;
}
