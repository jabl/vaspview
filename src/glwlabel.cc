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

#include "glinc.hh"
#include "glw.hh"

/*A static, one line label component*/

# define GLW_LABEL_INSET (2)

static void glwLabelLayoutMinSize(GLWLayoutManager *_this,GLWLabel *_label,
                                  int *_w,int *_h)
{
    if (_w!=NULL) {
        *_w=glwFontGetStringWidth(_label->super.font,glwLabelGetLabel(_label))+
            (GLW_LABEL_INSET<<1);
    }
    if (_h!=NULL) {
        *_h=glwFontGetHeight(_label->super.font)+(GLW_LABEL_INSET<<1);
    }
}

static GLWLayoutManager glw_label_layout= {
    NULL,
    NULL,
    (GLWLayoutSizeFunc)glwLabelLayoutMinSize,
    NULL,
    NULL
};

static void glwLabelPeerDisplay(GLWLabel *_this,GLWCallbacks *_cb)
{
    GLWcolor  fc;
    int       w;
    int       h;
    double    x;
    double    y;
    glwCompSuperDisplay(&_this->super,_cb);
    if (glwCompIsEnabled(&_this->super))fc=_this->super.forec;
    else fc=glwColorBlend(_this->super.forec,_this->super.backc);
    w=glwFontGetStringWidth(_this->super.font, _this->label);
    h=glwFontGetHeight(_this->super.font);
    x=_this->super.bounds.w-w-(GLW_LABEL_INSET<<1);
    y=_this->super.bounds.h-h-(GLW_LABEL_INSET<<1);
    x*=_this->super.constraints.alignx;
    y*=_this->super.constraints.aligny;
    x+=GLW_LABEL_INSET;
    y+=GLW_LABEL_INSET;
    /*Draw focus rectangle if we have focus*/
    if (glwCompIsFocused(&_this->super)) {
        glLineStipple(2,0x5555);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_LOOP);
        glwColor(glwColorBlend(fc,_this->super.backc));
        glVertex2d(x-1,y-1);
        glVertex2d(x-1,y+h);
        glVertex2d(x+w,y+h);
        glVertex2d(x+w,y-1);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }
    y+=glwFontGetDescent(_this->super.font);
    glwColor(fc);
    glwFontDrawString(_this->super.font, _this->label.c_str(), x, y);
}

static void glwLabelPeerEnable(GLWLabel *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperEnable(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static void glwLabelPeerDispose(GLWLabel *_this,GLWCallbacks *_cb)
{
    glwCompSuperDispose(&_this->super,_cb);
}

const GLWCallbacks GLW_LABEL_CALLBACKS= {
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwLabelPeerDispose,
    (GLWDisplayFunc)glwLabelPeerDisplay,
    NULL,
    NULL,
    (GLWEnableFunc)glwLabelPeerEnable,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

GLWLabel::GLWLabel(const std::string& label)
{
    glwLabelSetLabel(this, label);
    this->super.callbacks=&GLW_LABEL_CALLBACKS;
    glwCompSetAlignX(&this->super,0);
    glwCompSetLayout(&this->super,&glw_label_layout);
    return;
}

const char *glwLabelGetLabel(GLWLabel *_this)
{
    return _this->label.c_str();
}

int glwLabelSetLabel(GLWLabel *_this, const std::string& label)
{
    _this->label = label;
    glwCompRevalidate(&_this->super);
    return 1;
}

int glwLabelAddLabel(GLWLabel *_this, const std::string& label)
{
    _this->label.append(label);
    glwCompRevalidate(&_this->super);
    return 1;
}
