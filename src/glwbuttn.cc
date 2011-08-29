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
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glw.hh"

/*Button component*/

static void glwButtonLayoutMinSize(GLWLayoutManager *_this,GLWButton *_button,
                                   int *_w,int *_h)
{
    if (_w!=NULL)
    {
        *_w=glwFontGetStringWidth(_button->super.font,
                                  _button->label.c_str()) + 8;
    }
    if (_h!=NULL)*_h=glwFontGetHeight(_button->super.font)+8;
}

static GLWLayoutManager glw_button_layout=
{
    NULL,
    NULL,
    (GLWLayoutSizeFunc)glwButtonLayoutMinSize,
    NULL,
    NULL
};

static void glwButtonPeerDisplay(GLWButton *_this,GLWCallbacks *_cb)
{
    double  x;
    double  y;
    int     w;
    int     h;
    int     c1;
    int     c2;
    glwCompSuperDisplay(&_this->super,_cb);
    /*Calculate text position and dimensions*/
    w=glwFontGetStringWidth(_this->super.font, _this->label.c_str());
    h=glwFontGetHeight(_this->super.font);
    x=(_this->super.bounds.w-w)*0.5;
    y=(_this->super.bounds.h-h)*0.5;
    if (glwCompIsEnabled(&_this->super))
    {
        /*Draw focus rectangle if we have focus*/
        if (glwCompIsFocused(&_this->super))
        {
            glLineStipple(2,0x5555);
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINE_LOOP);
            glwColor(glwColorBlend(_this->super.forec,_this->super.backc));
            glVertex2d(x-1,y-1);
            glVertex2d(x-1,y+h);
            glVertex2d(x+w,y+h);
            glVertex2d(x+w,y-1);
            glEnd();
            glDisable(GL_LINE_STIPPLE);
        }
        /*If the button is down, shift the text and change border colors*/
        if (_this->down)
        {
            x++;
            y--;
            c1=glwColorDarken(_this->super.backc);
            c2=glwColorLighten(_this->super.backc);
        }
        else
        {
            c1=glwColorLighten(_this->super.backc);
            c2=glwColorDarken(_this->super.backc);
        }
        glwColor(_this->super.forec);
    }
    else
    {
        c1=glwColorLighten(_this->super.backc);
        c2=glwColorDarken(_this->super.backc);
        glwColor(glwColorBlend(_this->super.forec,_this->super.backc));
    }
    y+=glwFontGetDescent(_this->super.font);
    glwFontDrawString(_this->super.font, _this->label.c_str(), x, y);
    /*Draw the border, using the appropriate colors to make the button appear
      raised or lowered*/
    glBegin(GL_LINES);
    glwColor(c1);
    glVertex2i(0,0);
    glVertex2i(0,_this->super.bounds.h-1);
    glVertex2i(0,_this->super.bounds.h-1);
    glVertex2i(_this->super.bounds.w-2,_this->super.bounds.h-1);
    glVertex2i(1,1);
    glVertex2i(1,_this->super.bounds.h-2);
    glVertex2i(1,_this->super.bounds.h-2);
    glVertex2i(_this->super.bounds.w-3,_this->super.bounds.h-2);
    glwColor(c2);
    glVertex2i(_this->super.bounds.w-1,_this->super.bounds.h-1);
    glVertex2i(_this->super.bounds.w-1,0);
    glVertex2i(_this->super.bounds.w-1,0);
    glVertex2i(1,0);
    glVertex2i(_this->super.bounds.w-2,_this->super.bounds.h-2);
    glVertex2i(_this->super.bounds.w-2,1);
    glVertex2i(_this->super.bounds.w-2,1);
    glVertex2i(2,1);
    glEnd();
    /*If we were pressed with the keyboard, redisplay ourselves up*/
    if (_this->down&&_this->release)
    {
        _this->down=0;
        _this->release=0;
        glwCompRepaint(&_this->super,100);
    }
}

static void glwButtonPeerEnable(GLWButton *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperEnable(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static void glwButtonPeerFocus(GLWButton *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static int glwButtonPeerKeyboard(GLWButton *_this,const GLWCallbacks *_cb,
                                 unsigned char _k,int _x,int _y)
{
    if (_k==' '&&!_this->super.mouse_b)
    {
        _this->down=1;
        _this->release=1;
        glwCompRepaint(&_this->super,0);
        if (_this->pressed!=NULL)_this->pressed(_this->pressed_ctx,&_this->super);
        return -1;
    }
    return glwCompSuperKeyboard(&_this->super,_cb,_k,_x,_y);
}

static int glwButtonPeerMouse(GLWButton *_this,const GLWCallbacks *_cb,
                              int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0)
    {
        int rp;
        rp=0;
        if (_x<0||_x>_this->super.bounds.w||_y<0||_y>_this->super.bounds.h)
        {
            ret=-1;
            if (_this->down)
            {
                _this->down=0;
                rp=1;
            }
        }
        else if (!_s&&_this->down&&!_this->super.mouse_b)
        {
            ret=-1;
            _this->down=0;
            if (_this->pressed!=NULL)_this->pressed(_this->pressed_ctx,&_this->super);
            rp=1;
        }
        else if (_s||_this->super.mouse_b)
        {
            ret=-1;
            if (!_this->down)
            {
                _this->down=1;
                rp=1;
            }
            _this->release=0;
        }
        if (rp)glwCompRepaint(&_this->super,0);
    }
    return ret;
}

static int glwButtonPeerMotion(GLWButton *_this,const GLWCallbacks *_cb,
                               int _x,int _y)
{
    int ret;
    ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
    if (ret>=0)
    {
        int rp;
        rp=0;
        if (_x<0||_x>_this->super.bounds.w||_y<0||_y>_this->super.bounds.h)
        {
            ret=1;
            if (_this->down)
            {
                _this->down=0;
                rp=1;
            }
        }
        else if (_this->super.mouse_b)
        {
            ret=1;
            if (!_this->down)
            {
                _this->down=1;
                _this->release=0;
                rp=1;
            }
        }
        if (rp)glwCompRepaint(&_this->super,0);
    }
    return ret;
}

static void glwButtonPeerDispose(GLWButton *_this,GLWCallbacks *_cb)
{
    glwCompSuperDispose(&_this->super,_cb);
}


const GLWCallbacks GLW_BUTTON_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwButtonPeerDispose,
    (GLWDisplayFunc)glwButtonPeerDisplay,
    NULL,
    NULL,
    (GLWEnableFunc)glwButtonPeerEnable,
    (GLWFocusFunc)glwButtonPeerFocus,
    NULL,
    NULL,
    (GLWKeyboardFunc)glwButtonPeerKeyboard,
    NULL,
    (GLWMouseFunc)glwButtonPeerMouse,
    (GLWMotionFunc)glwButtonPeerMotion,
    NULL
};

GLWButton::GLWButton(const char* label)
{
	this->pressed=NULL;
	this->pressed_ctx=NULL;
	glwButtonSetLabel(this, label);
	this->super.callbacks=&GLW_BUTTON_CALLBACKS;
        glwCompSetLayout(&this->super,&glw_button_layout);
        glwCompSetFocusable(&this->super,1);
        this->down=0;
}

const char *glwButtonGetLabel(GLWButton *_this)
{
	return _this->label.c_str();
}

int glwButtonSetLabel(GLWButton* _this, const char* label)
{
	if (label==NULL)
		_this->label.clear();
	else
		_this->label = label;
	glwCompRevalidate(&_this->super);
	return 1;
}

int glwButtonAddLabel(GLWButton* _this, const char* label)
{
	if (_this->label.size() <= 1)
		return glwButtonSetLabel(_this, label);
	else if (label != NULL)
        {
		_this->label.append(label);
		glwCompRevalidate(&_this->super);
		return 1;
        }
	else return 1;
}

GLWActionFunc glwButtonGetPressedFunc(GLWButton *_this)
{
    return _this->pressed;
}

void glwButtonSetPressedFunc(GLWButton *_this,GLWActionFunc _func)
{
    _this->pressed=_func;
}

void *glwButtonGetPressedCtx(GLWButton *_this)
{
    return _this->pressed_ctx;
}

void glwButtonSetPressedCtx(GLWButton *_this,void *_ctx)
{
    _this->pressed_ctx=_ctx;
}
