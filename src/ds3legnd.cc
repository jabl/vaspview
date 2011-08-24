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
#include "ds3.h"
#include "ds3legnd.h"
#pragma hdrstop
#if !defined(_ds3legnd_C)
# define _ds3legnd_C (1)

/*Draws the color scale for the color legend*/
static void dsColorLegendScalePeerDisplay(GLWComponent *_this,
                                          GLWCallbacks *_cb){
 typedef GLubyte GLubyte4[4];
 glwCompSuperDisplay(_this,_cb);
 if(_this->parent!=NULL&&_this->bounds.w>1){
  GLubyte4 *ctable;
  int       i;
  ctable=((DSColorLegend *)_this->parent)->ctable;
  glBegin(GL_LINES);
  for(i=0;i<_this->bounds.w;i++){
   glColor4ubv(ctable[i*UCHAR_MAX/_this->bounds.w]);
   glVertex2i(i,0);
   glVertex2i(i,_this->bounds.h-1);}
  glEnd();} }


static const GLWCallbacks DS_COLOR_LEGEND_SCALE_CALLBACKS={
 &GLW_COMPONENT_CALLBACKS,
 NULL,
 (GLWDisplayFunc)dsColorLegendScalePeerDisplay};
 


DSColorLegend *dsColorLegendAlloc(void){
 DSColorLegend *this_;
 this_=(DSColorLegend *)malloc(sizeof(DSColorLegend));
 if(this_!=NULL){
  if(dsColorLegendInit(this_))return this_;
  free(this_);}
 return NULL;}
 
int dsColorLegendInit(DSColorLegend *_this){
 GLWGridBagLayout *layout;
 glwCompInit(&_this->super);
 _this->cm_scale=glwCompAlloc();
 _this->lb_min=glwLabelAlloc("0");
 _this->lb_max=glwLabelAlloc("1");
 _this->lb_label=glwLabelAlloc(NULL);
 layout=glwGridBagLayoutAlloc();
 if(_this->cm_scale!=NULL&&_this->lb_min!=NULL&&
    _this->lb_max!=NULL&&_this->lb_label!=NULL&&layout!=NULL){
  _this->cm_scale->callbacks=&DS_COLOR_LEGEND_SCALE_CALLBACKS;
  if(glwCompAdd(&_this->super,_this->cm_scale,-1)&&
     glwCompAdd(&_this->super,&_this->lb_min->super,-1)&&
     glwCompAdd(&_this->super,&_this->lb_label->super,-1)&&
     glwCompAdd(&_this->super,&_this->lb_max->super,-1)){
   glwCompSetMinWidth(_this->cm_scale,64);
   glwCompSetMinHeight(_this->cm_scale,16);
   glwCompSetGridWidth(_this->cm_scale,GLWC_REMAINDER);
   glwCompSetWeightX(_this->cm_scale,1);
   glwCompSetWeightY(_this->cm_scale,1);
   glwCompSetFill(_this->cm_scale,GLWC_BOTH);
   glwCompSetAlignX(&_this->lb_label->super,0.5);
   glwCompSetAlignX(&_this->lb_max->super,1);
   glwCompSetLayout(&_this->super,&layout->super);
   dsColorLegendSetColorScale(_this,NULL);
   return 1;}
  glwCompDelAll(&_this->super);}
 glwCompFree(_this->cm_scale);
 glwLabelFree(_this->lb_min);
 glwLabelFree(_this->lb_max);
 glwLabelFree(_this->lb_label);
 glwGridBagLayoutFree(layout);}
  

void dsColorLegendDstr(DSColorLegend *_this){
 glwCompDstr(&_this->super);}

void dsColorLegendFree(DSColorLegend *_this){
 glwCompFree(&_this->super);}


/*Resets the color legend to use values from the given data set
  ds3: The data set to get labels, units, and range from*/
int dsColorLegendSetDataSet(DSColorLegend *_this,DataSet3D *_ds3){
 dsColorLegendSetRange(_this,_ds3->min,_ds3->max);
 if(_ds3->label[3]!=NULL&&_ds3->label[3][0]!='\0'){
  if(_ds3->units[3]!=NULL&&_ds3->units[3][0]!='\0'){
   CDynArray label;
   char      sp=' ';
   char      lp='(';
   char      rp=')';
   int       ret;
   _DAInit(&label,64,char);
   if(daInsArrayTail(&label,_ds3->label[3],strlen(_ds3->label[3]))&&
      daInsTail(&label,&sp)&&daInsTail(&label,&lp)&&
      daInsArrayTail(&label,_ds3->units[3],strlen(_ds3->units[3])+1)&&
      daInsTail(&label,&rp)){
    ret=dsColorLegendSetLabel(_this,_DAGetAt(&label,0,char));}
   else ret=0;
   daDstr(&label);
   return ret;}
  else return dsColorLegendSetLabel(_this,_ds3->label[3]);}
 else return dsColorLegendSetLabel(_this,_ds3->units[3]);}
   
/*Sets the color scale to use
  cs: The new color scale*/
void dsColorLegendSetColorScale(DSColorLegend *_this,const DSColorScale *_cs){
 int i;
 _this->cs=_cs!=NULL?_cs:&DS_RAINBOW_SCALE;
 for(i=0;i<=UCHAR_MAX;i++){
  GLWcolor c;
  c=dsColorScale(_this->cs,i/(double)UCHAR_MAX);
  _this->ctable[i][0]=(GLubyte)(c&0xFF);
  _this->ctable[i][1]=(GLubyte)(c>>8&0xFF);
  _this->ctable[i][2]=(GLubyte)(c>>16&0xFF);
  _this->ctable[i][3]=(GLubyte)(c>>24&0xFF);}
 glwCompRepaint(_this->cm_scale,0);}

/*Sets the data range to display
  min: The minimum data value
  max: The maximum data value*/
int dsColorLegendSetRange(DSColorLegend *_this,double _min,double _max){
 char text[32];
 int  ret;
 sprintf(text,"%0.5lg",_min);
 ret=glwLabelSetLabel(_this->lb_min,text);
 sprintf(text,"%0.5lg",_max);
 ret=glwLabelSetLabel(_this->lb_max,text)&&ret;
 return ret;}
 
/*Sets the label for data values
  label: The new label for data values*/
int dsColorLegendSetLabel(DSColorLegend *_this,const char *_label){
 return glwLabelSetLabel(_this->lb_label,_label);}

/*Adds to the label for data values
  label: The additional text to add to the label for data values*/
int dsColorLegendAddLabel(DSColorLegend *_this,const char *_label){
 return glwLabelAddLabel(_this->lb_label,_label);}

#endif                                                           /*_ds3legnd_C*/
