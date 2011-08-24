/*GL Widget Set - simple, portable OpenGL/GLUT widget set
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
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glw.h"
#pragma hdrstop
#if !defined(_glwframe_C)
# define _glwframe_C (1)

/*The highest-level component.*/

CHashTable glw_frame_table;

static void glwFramePeerDispose(GLWFrame *_this,const GLWCallbacks *_cb){
 glwCompSuperDispose(&_this->super,_cb);
 daDstr(&_this->title);
 htDel(&glw_frame_table,&_this->super.wid,NULL);
 glutDestroyWindow(_this->super.wid);}

static void glwFramePeerEntry(GLWFrame *_this,const GLWCallbacks *_cb,int _s){
 GLWComponent *focus;
 glwCompSuperEntry(&_this->super,_cb,_s);
 focus=_this->super.focus!=NULL?_this->super.focus:&_this->super;
 if(glwCompIsFocusable(focus))glwCompFocus(focus,_s);}

static void glwFrameGlutCursor(GLWFrame *_this){
 GLWComponent *c;
 for(c=&_this->super;c->capture!=NULL;c=c->capture);
 for(;c->parent!=NULL&&c->cursor==GLUT_CURSOR_INHERIT;c=c->parent);
 glutSetCursor(c->cursor);}

static void glwFrameGlutDisplay(void){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   int minw;
   int minh;
   int maxw;
   int maxh;
   int w;
   int h;
   glwCompGetMinSize(&(*framep)->super,&minw,&minh);
   glwCompGetMaxSize(&(*framep)->super,&maxw,&maxh);
   w=(*framep)->super.bounds.w;
   h=(*framep)->super.bounds.h;
   if(maxw>=0&&w>maxw)w=maxw;
   if(maxh>=0&&h>maxh)h=maxh;
   if(minw>=0&&w<minw)w=minw;
   if(minh>=0&&h<minh)h=minh;
   if(w!=(*framep)->super.bounds.w||h!=(*framep)->super.bounds.h){
    glutReshapeWindow(w,h);}
   else{
    glwCompValidate(&(*framep)->super);
    glDrawBuffer(GL_BACK);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glViewport(0,0,(*framep)->super.bounds.w,(*framep)->super.bounds.h);
    glwCompDisplay(&(*framep)->super);
    glPopAttrib();
    glwCompDisplayChildren(&(*framep)->super);
    glutSwapBuffers();} } } }

static void glwFrameGlutReshape(int _w,int _h){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompSetBounds(&(*framep)->super,(*framep)->super.bounds.x,
                    (*framep)->super.bounds.y,_w,_h);} } }

static void glwFrameGlutVisibility(int _state){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompVisibility(&(*framep)->super,_state==GLUT_VISIBLE?1:0);} } }

static void glwFrameGlutEntry(int _state){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompEntry(&(*framep)->super,_state==GLUT_ENTERED?1:0);} } }

static void glwFrameGlutKeyboard(unsigned char _key,int _x,int _y){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompKeyboard(&(*framep)->super,_key,
                   _x,(*framep)->super.bounds.h-_y);
   glwFrameGlutCursor(*framep);} } }

static void glwFrameGlutSpecial(int _key,int _x,int _y){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompSpecial(&(*framep)->super,_key,
                  _x,(*framep)->super.bounds.h-_y);
   glwFrameGlutCursor(*framep);} } }

static void glwFrameGlutMouse(int _but,int _state,int _x,int _y){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompMouse(&(*framep)->super,_but,_state==GLUT_DOWN?1:0,
                _x,(*framep)->super.bounds.h-_y);
   glwFrameGlutCursor(*framep);} } }

static void glwFrameGlutMotion(int _x,int _y){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompMotion(&(*framep)->super,
                 _x,(*framep)->super.bounds.h-_y);
   glwFrameGlutCursor(*framep);} } }

static void glwFrameGlutPassiveMotion(int _x,int _y){
 int wid;
 wid=glutGetWindow();
 if(wid!=0){
  GLWFrame **framep=(GLWFrame **)htGet(&glw_frame_table,&wid);
  if(framep!=NULL&&*framep!=NULL){
   glwCompPassiveMotion(&(*framep)->super,
                        _x,(*framep)->super.bounds.h-_y);
   glwFrameGlutCursor(*framep);} } }


const GLWCallbacks GLW_FRAME_CALLBACKS={
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
 NULL};


GLWFrame *glwFrameAlloc(const char *_title){
 GLWFrame *this;
 this=(GLWFrame *)malloc(sizeof(GLWFrame));
 if(this!=NULL){
  if(glwFrameInit(this,_title)){
   return this;}
  free(this);}
 return NULL;}

int glwFrameInit(GLWFrame *_this,const char *_title){
 int wid;
 wid=glutCreateWindow(_title);
 if(wid!=0){
  if(htIns(&glw_frame_table,&wid,&_this)!=NULL){
   glwCompInit(&_this->super);
   _DAInit(&_this->title,0,char);
   if(daInsArrayHead(&_this->title,_title,strlen(_title)+1)){
    _this->super.wid=wid;
    _this->super.callbacks=&GLW_FRAME_CALLBACKS;
    glwCompSetCursor(&_this->super,GLUT_CURSOR_RIGHT_ARROW);
    glutDisplayFunc(glwFrameGlutDisplay);
    glutReshapeFunc(glwFrameGlutReshape);
    glutVisibilityFunc(glwFrameGlutVisibility);
    glutEntryFunc(glwFrameGlutEntry);
    glutKeyboardFunc(glwFrameGlutKeyboard);
    glutSpecialFunc(glwFrameGlutSpecial);
    glutMouseFunc(glwFrameGlutMouse);
    glutMotionFunc(glwFrameGlutMotion);
    glutPassiveMotionFunc(glwFrameGlutPassiveMotion);
    return 1;}
   daDstr(&_this->title);
   glwCompDstr(&_this->super);
   htDel(&glw_frame_table,&wid,NULL);}
  glutDestroyWindow(wid);}
 return 0;}

void glwFrameDstr(GLWFrame *_this){
 glwCompDstr(&_this->super);}

void glwFrameFree(GLWFrame *_this){
 glwCompFree(&_this->super);}


int glwFrameSetTitle(GLWFrame *_this,const char *_title){
 size_t len;
 len=strlen(_title)+1;
 if(daSetSize(&_this->title,len)){
  int wid;
  memcpy(_DAGetAt(&_this->title,0,char),_title,len);
  wid=glutGetWindow();
  if(wid!=_this->super.wid)glutSetWindow(_this->super.wid);
  glutSetWindowTitle(_title);
  glutSetIconTitle(_title);
  if(wid!=_this->super.wid)glutSetWindow(wid);
  return 1;}
 return 0;}

const char *glwFrameGetTitle(GLWFrame *_this){
 return _DAGetAt(&_this->title,0,char);}

void glwFrameShow(GLWFrame *_this){
 int wid;
 wid=glutGetWindow();
 if(wid!=_this->super.wid)glutSetWindow(_this->super.wid);
 glutShowWindow();
 if(wid!=_this->super.wid)glutSetWindow(wid);}

void glwFrameHide(GLWFrame *_this){
 int wid;
 wid=glutGetWindow();
 if(wid!=_this->super.wid)glutSetWindow(_this->super.wid);
 glutHideWindow();
 if(wid!=_this->super.wid)glutSetWindow(wid);}

void glwFramePack(GLWFrame *_this){
 int prew;
 int preh;
 glwCompGetPreSize(&_this->super,&prew,&preh);
 if(prew>=0||preh>=0){
  int wid;
  if(prew<0)prew=_this->super.bounds.w;
  if(preh<0)preh=_this->super.bounds.h;
  wid=glutGetWindow();
  if(wid!=_this->super.wid)glutSetWindow(_this->super.wid);
  glutReshapeWindow(prew,preh);
  if(wid!=_this->super.wid)glutSetWindow(wid);} }

#endif                                                           /*_glwframe_C*/