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

#include <map>

/*The highest-level component.*/

// Ordered semantics not needed, so C++2011 unordered_map would be
// fine.
std::map<int, GLWFrame*> glw_frame_table;
typedef std::map<int, GLWFrame*>::iterator frt_itr;

static void glwFramePeerDispose(GLWFrame *_this,const GLWCallbacks *_cb)
{
    glwCompSuperDispose(&_this->super,_cb);
    glw_frame_table.erase(_this->super.wid);
    glutDestroyWindow(_this->super.wid);
}

static void glwFramePeerEntry(GLWFrame *_this,const GLWCallbacks *_cb,int _s)
{
    GLWComponent *focus;
    glwCompSuperEntry(&_this->super,_cb,_s);
    focus=_this->super.focus!=NULL?_this->super.focus:&_this->super;
    if (glwCompIsFocusable(focus))glwCompFocus(focus,_s);
}

static void glwFrameGlutCursor(GLWFrame *_this)
{
    GLWComponent *c;
    for (c=&_this->super; c->capture!=NULL; c=c->capture);
    for (; c->parent!=NULL&&c->cursor==GLUT_CURSOR_INHERIT; c=c->parent);
    glutSetCursor(c->cursor);
}

static void glwFrameGlutDisplay(void)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
        if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            int minw;
            int minh;
            int maxw;
            int maxh;
            int w;
            int h;
            glwCompGetMinSize(&framep->super,&minw,&minh);
            glwCompGetMaxSize(&framep->super,&maxw,&maxh);
            w=framep->super.bounds.w;
            h=framep->super.bounds.h;
            if (maxw>=0&&w>maxw)w=maxw;
            if (maxh>=0&&h>maxh)h=maxh;
            if (minw>=0&&w<minw)w=minw;
            if (minh>=0&&h<minh)h=minh;
            if (w!=framep->super.bounds.w||h!=framep->super.bounds.h)
            {
                glutReshapeWindow(w,h);
            }
            else
            {
                glwCompValidate(&framep->super);
                glDrawBuffer(GL_BACK);
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glViewport(0,0,framep->super.bounds.w,framep->super.bounds.h);
                glwCompDisplay(&framep->super);
                glPopAttrib();
                glwCompDisplayChildren(&framep->super);
                glutSwapBuffers();
            }
        }
    }
}

static void glwFrameGlutReshape(int _w,int _h)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
        if (it != glw_frame_table.end())
        {
	    GLWFrame * framep = (*it).second;
            glwCompSetBounds(&framep->super,framep->super.bounds.x,
                             framep->super.bounds.y,_w,_h);
        }
    }
}

static void glwFrameGlutVisibility(int _state)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompVisibility(&framep->super,_state==GLUT_VISIBLE?1:0);
        }
    }
}

static void glwFrameGlutEntry(int _state)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompEntry(&framep->super,_state==GLUT_ENTERED?1:0);
        }
    }
}

static void glwFrameGlutKeyboard(unsigned char _key,int _x,int _y)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompKeyboard(&framep->super,_key,
                            _x,framep->super.bounds.h-_y);
            glwFrameGlutCursor(framep);
        }
    }
}

static void glwFrameGlutSpecial(int _key,int _x,int _y)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompSpecial(&framep->super,_key,
                           _x,framep->super.bounds.h-_y);
            glwFrameGlutCursor(framep);
        }
    }
}

static void glwFrameGlutMouse(int _but,int _state,int _x,int _y)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompMouse(&framep->super,_but,_state==GLUT_DOWN?1:0,
                         _x,framep->super.bounds.h-_y);
            glwFrameGlutCursor(framep);
        }
    }
}

static void glwFrameGlutMotion(int _x,int _y)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompMotion(&framep->super,
                          _x,framep->super.bounds.h-_y);
            glwFrameGlutCursor(framep);
        }
    }
}

static void glwFrameGlutPassiveMotion(int _x,int _y)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=0)
    {
	frt_itr it = glw_frame_table.find(wid);
	if (it != glw_frame_table.end())
        {
	    GLWFrame *framep = (*it).second;
            glwCompPassiveMotion(&framep->super,
                                 _x,framep->super.bounds.h-_y);
            glwFrameGlutCursor(framep);
        }
    }
}


const GLWCallbacks GLW_FRAME_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwFramePeerDispose,
    NULL,
    NULL,
    NULL/*(GLWValidateFunc)glwFramePeerValidate*/,
    NULL,
    NULL,
    NULL,
    (GLWEntryFunc)glwFramePeerEntry,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


GLWFrame::GLWFrame(const char* title)
{
	int wid;
	wid=glutCreateWindow(title);
	if (wid!=0)
	{
		glw_frame_table.insert(std::pair<int, GLWFrame*>(wid, this));
		this->title = title;
		this->super.wid=wid;
		this->super.callbacks=&GLW_FRAME_CALLBACKS;
		glwCompSetCursor(&this->super,GLUT_CURSOR_RIGHT_ARROW);
		glutDisplayFunc(glwFrameGlutDisplay);
		glutReshapeFunc(glwFrameGlutReshape);
		glutVisibilityFunc(glwFrameGlutVisibility);
		glutEntryFunc(glwFrameGlutEntry);
		glutKeyboardFunc(glwFrameGlutKeyboard);
		glutSpecialFunc(glwFrameGlutSpecial);
		glutMouseFunc(glwFrameGlutMouse);
		glutMotionFunc(glwFrameGlutMotion);
		glutPassiveMotionFunc(glwFrameGlutPassiveMotion);
		return;
	}
	glw_frame_table.erase(wid);
	glutDestroyWindow(wid);
}

void glwFrameShow(GLWFrame *_this)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=_this->super.wid)glutSetWindow(_this->super.wid);
    glutShowWindow();
    if (wid!=_this->super.wid)glutSetWindow(wid);
}

void glwFrameHide(GLWFrame *_this)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=_this->super.wid)glutSetWindow(_this->super.wid);
    glutHideWindow();
    if (wid!=_this->super.wid)glutSetWindow(wid);
}

void glwFramePack(GLWFrame *_this)
{
    int prew;
    int preh;
    glwCompGetPreSize(&_this->super,&prew,&preh);
    if (prew>=0||preh>=0)
    {
        int wid;
        if (prew<0)prew=_this->super.bounds.w;
        if (preh<0)preh=_this->super.bounds.h;
        wid=glutGetWindow();
        if (wid!=_this->super.wid)glutSetWindow(_this->super.wid);
        glutReshapeWindow(prew,preh);
        if (wid!=_this->super.wid)glutSetWindow(wid);
    }
}
