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
#if !defined(_glwlabel_C)
# define _glwlabel_C (1)

/*A static, one line label component*/

# define GLW_LABEL_INSET (2)

static void glwLabelLayoutMinSize(GLWLayoutManager *_this,GLWLabel *_label,
                                  int *_w,int *_h){
 if(_w!=NULL){
  *_w=glwFontGetStringWidth(_label->super.font,glwLabelGetLabel(_label))+
      (GLW_LABEL_INSET<<1);}
 if(_h!=NULL){
  *_h=glwFontGetHeight(_label->super.font)+(GLW_LABEL_INSET<<1);} }
 
static GLWLayoutManager glw_label_layout={
 NULL,
 NULL,
 (GLWLayoutSizeFunc)glwLabelLayoutMinSize,
 NULL,
 NULL,
 NULL};

static void glwLabelPeerDisplay(GLWLabel *_this,GLWCallbacks *_cb){
 GLWcolor  fc;
 int       w;
 int       h;
 double    x;
 double    y;
 char     *label;
 glwCompSuperDisplay(&_this->super,_cb);
 if(glwCompIsEnabled(&_this->super))fc=_this->super.forec;
 else fc=glwColorBlend(_this->super.forec,_this->super.backc);
 label=_DAGetAt(&_this->label,0,char);
 w=glwFontGetStringWidth(_this->super.font,label);
 h=glwFontGetHeight(_this->super.font);
 x=_this->super.bounds.w-w-(GLW_LABEL_INSET<<1);
 y=_this->super.bounds.h-h-(GLW_LABEL_INSET<<1);
 x*=_this->super.constraints.alignx;
 y*=_this->super.constraints.aligny;
 x+=GLW_LABEL_INSET;
 y+=GLW_LABEL_INSET;
 /*Draw focus rectangle if we have focus*/
 if(glwCompIsFocused(&_this->super)){
  glLineStipple(2,0x5555);
  glEnable(GL_LINE_STIPPLE);
  glBegin(GL_LINE_LOOP);
  glwColor(glwColorBlend(fc,_this->super.backc));
  glVertex2d(x-1,y-1);
  glVertex2d(x-1,y+h);
  glVertex2d(x+w,y+h);
  glVertex2d(x+w,y-1);
  glEnd();
  glDisable(GL_LINE_STIPPLE);}
 y+=glwFontGetDescent(_this->super.font);
 glwColor(fc);
 glwFontDrawString(_this->super.font,label,x,y);}

static void glwLabelPeerEnable(GLWLabel *_this,GLWCallbacks *_cb,int _s){
 glwCompSuperEnable(&_this->super,_cb,_s);
 glwCompRepaint(&_this->super,0);}

static void glwLabelPeerDispose(GLWLabel *_this,GLWCallbacks *_cb){
 glwCompSuperDispose(&_this->super,_cb);
 daDstr(&_this->label);}

const GLWCallbacks GLW_LABEL_CALLBACKS={
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
 NULL};

GLWLabel *glwLabelAlloc(const char *_label){
 GLWLabel *this_;
 this_=(GLWLabel *)malloc(sizeof(GLWLabel));
 if(this_!=NULL){
  if(glwLabelInit(this_,_label)){
   return this_;}
  free(this_);}
 return NULL;}

int glwLabelInit(GLWLabel *_this,const char *_label){
 glwCompInit(&_this->super);
 _DAInit(&_this->label,0,char);
 if(glwLabelSetLabel(_this,_label)){
  _this->super.callbacks=&GLW_LABEL_CALLBACKS;
  glwCompSetAlignX(&_this->super,0);
  glwCompSetLayout(&_this->super,&glw_label_layout);
  return 1;}
 daDstr(&_this->label);
 glwCompDstr(&_this->super);
 return 0;}

void glwLabelDstr(GLWLabel *_this){
 glwCompDstr(&_this->super);}

void glwLabelFree(GLWLabel *_this){
 glwCompFree(&_this->super);}


const char *glwLabelGetLabel(GLWLabel *_this){
 return _DAGetAt(&_this->label,0,char);}

int glwLabelSetLabel(GLWLabel *_this,const char *_label){
 if(_label==NULL){
  if(daSetSize(&_this->label,1)){
   *_DAGetAt(&_this->label,0,char)='\0';
   glwCompRevalidate(&_this->super);
   return 1;} }
 else{
  size_t len;
  len=strlen(_label)+1;
  if(daSetSize(&_this->label,len)){
   memcpy(_DAGetAt(&_this->label,0,char),_label,len);
   glwCompRevalidate(&_this->super);
   return 1;} }
 return 0;}

int glwLabelAddLabel(GLWLabel *_this,const char *_label){
 if(_this->label.size<=1)return glwLabelSetLabel(_this,_label);
 else if(_label!=NULL){
  size_t len;
  len=strlen(_label);
  if(daInsArrayBefore(&_this->label,_this->label.size-1,_label,len)){
   glwCompRevalidate(&_this->super);
   return 1;} }
 else return 1;
 return 0;}

#endif                                                           /*_glwlabel_C*/
