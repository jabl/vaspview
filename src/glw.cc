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

/*Initialization and a few miscellaneous functions (colors and fonts)*/

typedef struct GLWTimerEntry
{
    GLWComponent  *comp;
    void          *ctx;
    GLWActionFunc  func;
} GLWTimerEntry;


void glwInit(int *_argc,char **_argv)
{
    glutInit(_argc,_argv);
    glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
}



/*
int glwIsExtSupported(const char *_ext){
 const char *p;
 const char *e;
 size_t      l;
 if(_ext==NULL||strchr(_ext,' ')!=NULL)return 0;
 e=glGetString(GL_EXTENSIONS);
 l=strlen(_ext);
 for(p=e;;p=e+l){
  p=strstr(p,_ext);
  if(p==NULL)break;
  if((p==e||*(p-1)==' ')&&(p[l]=='\0'||p[l]==' '))return 1;}
 return 0;}
*/



void glwClearColor(GLWcolor _c)
{
    glClearColor((GLclampf)((_c&0xFF)*(1.0/0xFF)),
                 (GLclampf)((_c>>8&0xFF)*(1.0/0xFF)),
                 (GLclampf)((_c>>16&0xFF)*(1.0/0xFF)),
                 (GLclampf)((_c>>24&0xFF)*(1.0/0xFF)));
}

void glwColor(GLWcolor _c)
{
    glColor4ub((GLubyte)(_c&0xFF),
               (GLubyte)(_c>>8&0xFF),
               (GLubyte)(_c>>16&0xFF),
               (GLubyte)(_c>>24&0xFF));
}

GLWcolor glwColorDarken(GLWcolor _c)
{
    return (_c&0xFF000000)|
           (0x00FF0000&(GLWcolor)((_c&0x00FF0000)*0.7))|
           (0x0000FF00&(GLWcolor)((_c&0x0000FF00)*0.7))|
           (0x000000FF&(GLWcolor)((_c&0x000000FF)*0.7));
}

GLWcolor glwColorLighten(GLWcolor _c)
{
    GLWcolor r;
    GLWcolor g;
    GLWcolor b;
    r=_c&0xFF;
    g=_c>>8&0xFF;
    b=_c>>16&0xFF;
    r=(int)r*(1/0.7);
    g=(int)g*(1/0.7);
    b=(int)b*(1/0.7);
    if (r<3)r=3;
    else if (r>0xFF)r=0xFF;
    if (g<3)g=3;
    else if (g>0xFF)g=0xFF;
    if (b<3)b=3;
    else if (b>0xFF)b=0xFF;
    return (_c&0xFF000000)|r|g<<8|b<<16;
}

GLWcolor glwColorInvert(GLWcolor _c)
{
    return (_c&0xFF000000)|~(_c|0xFF000000);
}

GLWcolor glwColorBlend(GLWcolor _c1,GLWcolor _c2)
{
    GLWcolor d;
    GLWcolor ret;
    d=_c1&0xFEFEFEFF;
    ret=d+(_c2&0xFEFEFEFF);
    if (ret<d)ret=ret>>1|0x80000000;
    else ret>>=1;
    ret+=_c1&_c2&0x01010100;
    return ret;
}



static const GLWfont GLW_FONTS[7]={GLUT_BITMAP_8_BY_13,
                                   GLUT_BITMAP_9_BY_15,
                                   GLUT_BITMAP_TIMES_ROMAN_10,
                                   GLUT_BITMAP_TIMES_ROMAN_24,
                                   GLUT_BITMAP_HELVETICA_10,
                                   GLUT_BITMAP_HELVETICA_12,
                                   GLUT_BITMAP_HELVETICA_18
                                  };
static const int     GLW_FONT_FAMILIES[3]={0,2,4};
static const int     GLW_FONT_FAM_SIZES[3]={2,2,3};
static const int     GLW_FONT_SIZES[7]={13,15,10,24,10,12,18};
/*Just made these numbers up:*/
static const int     GLW_FONT_DESCENTS[7]={2,3,2,5,2,3,5};

GLWfont glwFontGet(int _family,int _size)
{
    int i;
    int j;
    int d;
    if (_family<0||_family>=3)return NULL;
    d=_size-GLW_FONT_SIZES[GLW_FONT_FAMILIES[_family]];
    if (d<0)d=-d;
    for (j=0,i=1; i<GLW_FONT_FAM_SIZES[_family]; i++)
    {
        int e;
        e=_size-GLW_FONT_SIZES[GLW_FONT_FAMILIES[_family]+i];
        if (e<0)e=-e;
        if (e<d)
        {
            j=i;
            d=e;
        }
    }
    return GLW_FONTS[GLW_FONT_FAMILIES[_family]+j];
}

int glwFontGetHeight(GLWfont _font)
{
    int i;
    for (i=0; i<7; i++)if (_font==GLW_FONTS[i])return GLW_FONT_SIZES[i];
    return 0;
}

int glwFontGetAscent(GLWfont _font)
{
    int i;
    for (i=0; i<7; i++)if (_font==GLW_FONTS[i])
        {
            return GLW_FONT_SIZES[i]-GLW_FONT_DESCENTS[i];
        }
    return 0;
}

int glwFontGetDescent(GLWfont _font)
{
    int i;
    for (i=0; i<7; i++)if (_font==GLW_FONTS[i])return GLW_FONT_DESCENTS[i];
    return 0;
}

int glwFontGetWidth(GLWfont _font,int _c)
{
    return glutBitmapWidth(_font,_c);
}

int glwFontGetStringWidth(GLWfont _font,const char *_s)
{
    size_t i;
    int    ret;
    ret=0;
    if (_s!=NULL)
    {
        for (i=0; _s[i]; i++)ret+=glutBitmapWidth(_font,(unsigned char)_s[i]);
    }
    return ret;
}

int glwFontDrawString(GLWfont _font,const char *_s,double _x,double _y)
{
    size_t i;
    int    ret;
    ret=0;
    if (_s!=NULL)
    {
        glPushAttrib(GL_CURRENT_BIT);
        glRasterPos2d(_x,_y);
        for (i=0; _s[i]; i++)
        {
            glutBitmapCharacter(_font,(unsigned char)_s[i]);
            ret+=glutBitmapWidth(_font,(unsigned char)_s[i]);
        }
        glPopAttrib();
    }
    return ret;
}

int glwFontDrawChar(GLWfont _font,int _c,double _x,double _y)
{
    double rp[4];
    glGetDoublev(GL_CURRENT_RASTER_POSITION,rp);
    glRasterPos2d(_x,_y);
    glutBitmapCharacter(_font,_c);
    glRasterPos4dv(rp);
    return glutBitmapWidth(_font,_c);
}



void glwRectInit(GLWRect *_this,int _x,int _y,int _w,int _h)
{
    _this->x=_x;
    _this->y=_y;
    _this->w=_w;
    _this->h=_h;
}



void glwInsetsInit(GLWInsets *_this,int _t,int _b,int _l,int _r)
{
    _this->t=_t;
    _this->b=_b;
    _this->l=_l;
    _this->r=_r;
}
