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
#include "glw.hh"
#pragma hdrstop
#if !defined(_glwgridb_C)
# define _glwgridb_C (1)

/*The only provided layout manager (other than the component-specific ones).
  This arranges components in a grid, and uses certain rules specified by
  the component's contraints member to allocate space to grid cells, and
  position the components in those cells*/

static void glwGBLInvalidate(GLWGridBagLayout *_this,GLWComponent *_comp){
 free(_this->cache);
 _this->cache=NULL;
 free(_this->min_w);
 _this->min_w=NULL;
 free(_this->min_h);
 _this->min_h=NULL;
 free(_this->pre_w);
 _this->pre_w=NULL;
 free(_this->pre_h);
 _this->pre_h=NULL;
 free(_this->weight_x);
 _this->weight_x=NULL;
 free(_this->weight_y);
 _this->weight_y=NULL;
 free(_this->adj_w);
 _this->adj_w=NULL;
 free(_this->adj_h);
 _this->adj_h=NULL;
 _this->comp=_comp;
 _this->valid=0;}

static int glwGBLCalcLayout(GLWGridBagLayout *_this,GLWComponent *_comp){
 if(_this->validating)return 0;
 if(_comp!=_this->comp)glwGBLInvalidate(_this,_comp);
 if(!_this->valid){
  GLWComponent **comps;
  size_t         i;
  int            cur_r;
  int            cur_c;
  CDynArray      x_max;
  CDynArray      y_max;
  int           *xs_w;
  int           *xs_h;
  int            next;
  _this->validating=1;
  _DAInit(&x_max,0,int);
  _DAInit(&y_max,0,int);
  _this->cache=(GLWGBLCInfo *)malloc(_comp->comps.size*sizeof(GLWGBLCInfo));
  if(_this->cache==NULL)goto fail;
  comps=(GLWComponent **)_DAGetAt(&_comp->comps,0,GLWComponent *);
  _this->w=_this->h=0;
  cur_r=cur_c=-1;
  /*Pass 1: figure out dimensions of the layout grid*/
  for(i=0;(size_t)i<_comp->comps.size;i++)if(glwCompIsVisible(comps[i])){
   int cur_x;
   int cur_y;
   int cur_w;
   int cur_h;
   int px;
   int py;
   int j;
   cur_x=comps[i]->constraints.gridx;
   cur_y=comps[i]->constraints.gridy;
   cur_w=comps[i]->constraints.gridw;
   cur_h=comps[i]->constraints.gridh;
   if(cur_w<=0)cur_w=1;
   if(cur_h<=0)cur_h=1;
   if(cur_x<0&&cur_y<0){
    if(cur_r>=0)cur_y=cur_r;
    else if(cur_c>=0)cur_x=cur_c;
    else cur_y=0;}
   if(cur_x<0){
    px=0;
    for(j=cur_y;j<cur_y+cur_h&&(size_t)j<x_max.size;j++){
     if(*_DAGetAt(&x_max,j,int)>px)px=*_DAGetAt(&x_max,j,int);}
    cur_x=px-cur_x-1;
    if(cur_x<0)cur_x=0;}
   else if(cur_y<0){
    py=0;
    for(j=cur_x;j<cur_x+cur_w&&(size_t)j<y_max.size;j++){
     if(*_DAGetAt(&y_max,j,int)>py)py=*_DAGetAt(&y_max,j,int);}
    cur_y=py-cur_y-1;
    if(cur_y<0)cur_y=0;}
   px=0;
   while(x_max.size<(size_t)cur_x){
    if(!daInsTail(&x_max,&px))goto fail;}
   while(y_max.size<(size_t)cur_y){
    if(!daInsTail(&y_max,&px))goto fail;}
   px=cur_x+cur_w;
   if(_this->w<px)_this->w=px;
   py=cur_y+cur_h;
   if(_this->h<py)_this->h=py;
   for(j=cur_y;j<py;j++){
    if((size_t)j<x_max.size)daSetAt(&x_max,j,&px);
    else if(!daInsTail(&x_max,&px))goto fail;}
   for(j=cur_x;j<px;j++){
    if((size_t)j<y_max.size)daSetAt(&y_max,j,&py);
    else if(!daInsTail(&y_max,&py))goto fail;}
   /*Cache minimum sizes while we're here*/
   glwCompGetMinSize(comps[i],&_this->cache[i].min_w,&_this->cache[i].min_h);
   glwCompGetPreSize(comps[i],&_this->cache[i].pre_w,&_this->cache[i].pre_h);
   glwCompGetMaxSize(comps[i],&_this->cache[i].max_w,&_this->cache[i].max_h);
   if(_this->cache[i].min_w<0)_this->cache[i].min_w=0;
   if(_this->cache[i].min_h<0)_this->cache[i].min_h=0;
   if(_this->cache[i].pre_w<0)_this->cache[i].pre_w=0;
   if(_this->cache[i].pre_h<0)_this->cache[i].pre_h=0;
   if(comps[i]->constraints.gridw<=0&&
      comps[i]->constraints.gridh<=0){
    cur_r=cur_c=-1;}
   if(comps[i]->constraints.gridh<=0&&cur_r<0){
    cur_c=cur_x+cur_w;}
   else if(comps[i]->constraints.gridw<=0&&cur_c<0){
    cur_r=cur_y+cur_h;} }
  /*Pass 2: Position relative components*/
  cur_r=cur_c=-1;
  daSetSize(&x_max,0);
  daSetSize(&y_max,0);
  for(i=0;(size_t)i<_comp->comps.size;i++)if(glwCompIsVisible(comps[i])){
   int cur_x;
   int cur_y;
   int cur_w;
   int cur_h;
   int px;
   int py;
   int j;
   cur_x=comps[i]->constraints.gridx;
   cur_y=comps[i]->constraints.gridy;
   cur_w=comps[i]->constraints.gridw;
   cur_h=comps[i]->constraints.gridh;
   if(cur_x<0&&cur_y<0){
    if(cur_r>=0)cur_y=cur_r;
    else if(cur_c>=0)cur_x=cur_c;
    else cur_y=0;}
   if(cur_x<0){
    if(cur_h<=0){
     cur_h+=_this->h-cur_y;
     if(cur_h<1)cur_h=1;}
    px=0;
    for(j=cur_y;j<cur_y+cur_h&&(size_t)j<x_max.size;j++){
     if(px<*_DAGetAt(&x_max,j,int))px=*_DAGetAt(&x_max,j,int);}
    cur_x=px-cur_x-1;
    if(cur_x<0)cur_x=0;}
   else if(cur_y<0){
    if(cur_w<=0){
     cur_w+=_this->w-cur_x;
     if(cur_w<1)cur_w=1;}
    py=0;
    for(j=cur_x;j<cur_x+cur_w&&(size_t)j<y_max.size;j++){
     if(py<*_DAGetAt(&y_max,j,int))py=*_DAGetAt(&y_max,j,int);}
    cur_y=py-cur_y-1;
    if(cur_y<0)cur_y=0;}
   if(cur_w<=0){
    cur_w+=_this->w-cur_x;
    if(cur_w<1)cur_w=1;}
   if(cur_h<=0){
    cur_h+=_this->h-cur_y;
    if(cur_h<1)cur_h=1;}
   px=0;
   while(x_max.size<(size_t)cur_x){
    if(!daInsTail(&x_max,&px))goto fail;}
   while(y_max.size<(size_t)cur_y){
    if(!daInsTail(&y_max,&px))goto fail;}
   px=cur_x+cur_w;
   py=cur_y+cur_h;
   for(j=cur_y;j<py;j++){
    if((size_t)j<x_max.size)daSetAt(&x_max,j,&px);
    else if(!daInsTail(&x_max,&px))goto fail;}
   for(j=cur_x;j<px;j++){
    if((size_t)j<y_max.size)daSetAt(&y_max,j,&py);
    else if(!daInsTail(&y_max,&py))goto fail;}
   if(comps[i]->constraints.gridw<=0&&comps[i]->constraints.gridh<=0){
    cur_r=cur_c=-1;}
   if(comps[i]->constraints.gridh<=0&&cur_r<=0)cur_c=cur_x+cur_w;
   else if(comps[i]->constraints.gridw<=0&&cur_c<=0)cur_r=cur_y+cur_h;
   _this->cache[i].x=cur_x;
   _this->cache[i].y=cur_y;
   _this->cache[i].w=cur_w;
   _this->cache[i].h=cur_h;}
  _this->min_w=(int *)calloc(_this->w,sizeof(int));
  if(_this->min_w==NULL)goto fail;
  _this->min_h=(int *)calloc(_this->h,sizeof(int));
  if(_this->min_h==NULL)goto fail;
  _this->pre_w=(int *)calloc(_this->w,sizeof(int));
  if(_this->pre_w==NULL)goto fail;
  _this->pre_h=(int *)calloc(_this->h,sizeof(int));
  if(_this->pre_h==NULL)goto fail;
  _this->weight_x=(double *)calloc(_this->w,sizeof(double));
  if(_this->weight_x==NULL)goto fail;
  _this->weight_y=(double *)calloc(_this->h,sizeof(double));
  if(_this->weight_y==NULL)goto fail;
  /*Pass 3: distribute weights*/
  next=INT_MAX;
  for(i=1;i!=INT_MAX;i=(size_t)next,next=INT_MAX){
   size_t j;
   for(j=0;j<_comp->comps.size;j++)if(glwCompIsVisible(comps[j])){
    if((size_t)_this->cache[j].w==i){
     int    k;
     int    px;
     double weightd;
     double tweight;
     px=_this->cache[j].x+_this->cache[j].w;
     weightd=comps[j]->constraints.weightx;
     tweight=0;
     for(k=_this->cache[j].x;k<px;k++)tweight+=_this->weight_x[k];
     weightd-=tweight;
     if(weightd>0){
      double weight;
      weight=tweight;
      for(k=_this->cache[j].x;weight>0&&k<px;k++){
       double wt=_this->weight_x[k];
       double dx=(wt*weightd)/weight;
       _this->weight_x[k]+=dx;
       weightd-=dx;
       weight-=wt;}
      _this->weight_x[px-1]+=weightd;} }
    else if((size_t)_this->cache[j].w>i&&_this->cache[j].w<next){
     next=_this->cache[j].w;}
    if((size_t)_this->cache[j].h==i){
     int    k;
     int    py;
     double weightd;
     double tweight;
     py=_this->cache[j].y+_this->cache[j].h;
     weightd=comps[j]->constraints.weighty;
     tweight=0;
     for(k=_this->cache[j].y;k<py;k++)tweight+=_this->weight_y[k];
     weightd-=tweight;
     if(weightd>0){
      double weight;
      weight=tweight;
      for(k=_this->cache[j].y;weight>0&&k<py;k++){
       double wt=_this->weight_y[k];
       double dy=(wt*weightd)/weight;
       _this->weight_y[k]+=dy;
       weightd-=dy;
       weight-=wt;}
      _this->weight_y[py-1]+=weightd;} }
    else if((size_t)_this->cache[j].h>i&&_this->cache[j].h<next){
     next=_this->cache[j].h;} } }
  /*Pass 4: distribute minimum widths and heights*/
  next=INT_MAX;
  for(i=1;i!=INT_MAX;i=(size_t)next,next=INT_MAX){
   size_t j;
   for(j=0;j<_comp->comps.size;j++)if(glwCompIsVisible(comps[j])){
    if((size_t)_this->cache[j].w==i){
     int    k;
     int    px;
     int    pixeld;
     double tweight;
     px=_this->cache[j].x+_this->cache[j].w;
     tweight=0;
     for(k=_this->cache[j].x;k<px;k++)tweight+=_this->weight_x[k];
     pixeld=_this->cache[j].min_w+comps[j]->constraints.insets.l+
            comps[j]->constraints.insets.r;
     for(k=_this->cache[j].x;k<px;k++)pixeld-=_this->min_w[k];
     if(pixeld>0){
      double weight;
      weight=tweight;
      for(k=_this->cache[j].x;weight>0&&k<px;k++){
       double wt=_this->weight_x[k];
       int    dx=(int)(wt*(double)pixeld/weight);
       _this->min_w[k]+=dx;
       pixeld-=dx;
       weight-=wt;}
      _this->min_w[px-1]+=pixeld;}
     pixeld=_this->cache[j].pre_w+comps[j]->constraints.insets.l+
            comps[j]->constraints.insets.r;
     for(k=_this->cache[j].x;k<px;k++)pixeld-=_this->pre_w[k];
     if(pixeld>0){
      double weight;
      weight=tweight;
      for(k=_this->cache[j].x;weight>0&&k<px;k++){
       double wt=_this->weight_x[k];
       int    dx=(int)(wt*(double)pixeld/weight);
       _this->pre_w[k]+=dx;
       pixeld-=dx;
       weight-=wt;}
      _this->pre_w[px-1]+=pixeld;} }
    else if((size_t)_this->cache[j].w>i&&_this->cache[j].w<next){
     next=_this->cache[j].w;}
    if((size_t)_this->cache[j].h==i){
     int    k;
     int    py;
     int    pixeld;
     double tweight;
     py=_this->cache[j].y+_this->cache[j].h;
     tweight=0;
     for(k=_this->cache[j].y;k<py;k++)tweight+=_this->weight_y[k];
     pixeld=_this->cache[j].min_h+comps[j]->constraints.insets.b+
            comps[j]->constraints.insets.t;
     for(k=_this->cache[j].y;k<py;k++)pixeld-=_this->min_h[k];
     if(pixeld>0){
      double weight;
      weight=tweight;
      for(k=_this->cache[j].y;weight>0&&k<py;k++){
       double wt=_this->weight_y[k];
       int    dy=(int)(wt*(double)pixeld/weight);
       _this->min_h[k]+=dy;
       pixeld-=dy;
       weight-=wt;}
      _this->min_h[py-1]+=pixeld;}
     pixeld=_this->cache[j].pre_h+comps[j]->constraints.insets.b+
            comps[j]->constraints.insets.t;
     for(k=_this->cache[j].y;k<py;k++)pixeld-=_this->pre_h[k];
     if(pixeld>0){
      double weight;
      weight=tweight;
      for(k=_this->cache[j].y;weight>0&&k<py;k++){
       double wt=_this->weight_y[k];
       int    dy=(int)(wt*(double)pixeld/weight);
       _this->pre_h[k]+=dy;
       pixeld-=dy;
       weight-=wt;}
      _this->pre_h[py-1]+=pixeld;} }
    else if((size_t)_this->cache[j].h>i&&_this->cache[j].h<next){
     next=_this->cache[j].h;} } }
  /*Pass 5: compress minimum widths and heights*/
  xs_w=_DAGetAt(&y_max,0,int);
  xs_h=_DAGetAt(&x_max,0,int);
  for(;;){
   size_t j;
   int    k;
   for(k=0;k<_this->w;k++)xs_w[k]=-1;
   for(j=0;j<_comp->comps.size;j++)if(glwCompIsVisible(comps[j])){
    int px;
    int pixeld;
    px=_this->cache[j].x+_this->cache[j].w;
    pixeld=-_this->cache[j].min_w-comps[j]->constraints.insets.l-
           comps[j]->constraints.insets.r;
    for(k=_this->cache[j].x;k<px;k++)pixeld+=_this->min_w[k];
    if(pixeld>=0)for(k=_this->cache[j].x;k<px;k++){
     if(xs_w[k]<0||xs_w[k]>pixeld)xs_w[k]=pixeld;} }
   for(k=0;k<_this->w;k++)if(xs_w[k]>0){
    _this->min_w[k]-=xs_w[k];
    break;}
   if(k==_this->w)break;}
  for(;;){
   size_t j;
   int    k;
   for(k=0;k<_this->h;k++)xs_h[k]=-1;
   for(j=0;j<_comp->comps.size;j++)if(glwCompIsVisible(comps[j])){
    int py;
    int pixeld;
    py=_this->cache[j].y+_this->cache[j].h;
    pixeld=-_this->cache[j].min_h-comps[j]->constraints.insets.t-
           comps[j]->constraints.insets.b;
    for(k=_this->cache[j].y;k<py;k++)pixeld+=_this->min_h[k];
    if(pixeld>=0)for(k=_this->cache[j].y;k<py;k++){
     if(xs_h[k]<0||xs_h[k]>pixeld)xs_h[k]=pixeld;} }
   for(k=0;k<_this->h;k++)if(xs_h[k]>0){
    _this->min_h[k]-=xs_h[k];
    break;}
   if(k==_this->h)break;}
  for(;;){
   size_t j;
   int    k;
   for(k=0;k<_this->w;k++)xs_w[k]=-1;
   for(j=0;j<_comp->comps.size;j++)if(glwCompIsVisible(comps[j])){
    int px;
    int pixeld;
    px=_this->cache[j].x+_this->cache[j].w;
    pixeld=-_this->cache[j].pre_w-comps[j]->constraints.insets.l-
           comps[j]->constraints.insets.r;
    for(k=_this->cache[j].x;k<px;k++)pixeld+=_this->pre_w[k];
    if(pixeld>=0)for(k=_this->cache[j].x;k<px;k++){
     if(xs_w[k]<0||xs_w[k]>pixeld)xs_w[k]=pixeld;} }
   for(k=0;k<_this->w;k++)if(xs_w[k]>0){
    _this->pre_w[k]-=xs_w[k];
    break;}
   if(k==_this->w)break;}
  for(;;){
   size_t j;
   int    k;
   for(k=0;k<_this->h;k++)xs_h[k]=-1;
   for(j=0;j<_comp->comps.size;j++)if(glwCompIsVisible(comps[j])){
    int py;
    int pixeld;
    py=_this->cache[j].y+_this->cache[j].h;
    pixeld=-_this->cache[j].pre_h-comps[j]->constraints.insets.t-
           comps[j]->constraints.insets.b;;
    for(k=_this->cache[j].y;k<py;k++)pixeld+=_this->pre_h[k];
    if(pixeld>=0)for(k=_this->cache[j].y;k<py;k++){
     if(xs_h[k]<0||xs_h[k]>pixeld)xs_h[k]=pixeld;} }
   for(k=0;k<_this->h;k++)if(xs_h[k]>0){
    _this->pre_h[k]-=xs_h[k];
    break;}
   if(k==_this->h)break;}
  daDstr(&x_max);
  daDstr(&y_max);
  _this->validating=0;
  _this->valid=1;
  return 1;
fail:
  free(_this->cache);
  _this->cache=NULL;
  free(_this->min_w);
  _this->min_w=NULL;
  free(_this->min_h);
  _this->min_h=NULL;
  free(_this->pre_w);
  _this->pre_w=NULL;
  free(_this->pre_h);
  _this->pre_h=NULL;
  free(_this->weight_x);
  _this->weight_x=NULL;
  free(_this->weight_y);
  _this->weight_y=NULL;
  daDstr(&x_max);
  daDstr(&y_max);
  _this->validating=0;
  return 0;}
 return 1;}

static void glwGBLMinSize(GLWGridBagLayout *_this,GLWComponent *_comp,
                          int *_w,int *_h){
 if(glwGBLCalcLayout(_this,_comp)){
  if(_w!=NULL){
   int i;
   *_w=0;
   for(i=0;i<_this->w;i++)*_w+=_this->min_w[i];}
  if(_h!=NULL){
   int i;
   *_h=0;
   for(i=0;i<_this->h;i++)*_h+=_this->min_h[i];} }
 else{
  if(_w!=NULL)*_w=-1;
  if(_h!=NULL)*_h=-1;} }

static void glwGBLPreSize(GLWGridBagLayout *_this,GLWComponent *_comp,
                          int *_w,int *_h){
 if(glwGBLCalcLayout(_this,_comp)){
  if(_w!=NULL){
   int i;
   *_w=0;
   for(i=0;i<_this->w;i++)*_w+=_this->pre_w[i];}
  if(_h!=NULL){
   int i;
   *_h=0;
   for(i=0;i<_this->h;i++)*_h+=_this->pre_h[i];} }
 else{
  if(_w!=NULL)*_w=-1;
  if(_h!=NULL)*_h=-1;} }

static void glwGBLAdjust(GLWGridBagLayout *_this,GLWComponent *_comp,int _i,
                         int *_x,int *_y,int *_w,int *_h){
 GLWConstraints *c;
 c=&(*_DAGetAt(&_comp->comps,_i,GLWComponent *))->constraints;
 *_x+=c->insets.l;
 *_w-=c->insets.l+c->insets.r;
 *_y+=c->insets.b;
 *_h-=c->insets.b+c->insets.t;
 if(!(c->fill&GLWC_HORIZONTAL)&&*_w>_this->cache[_i].pre_w){
  *_x+=(int)((*_w-_this->cache[_i].pre_w)*c->alignx);
  *_w=_this->cache[_i].pre_w;}
 else if(_this->cache[_i].max_w>=0&&*_w>_this->cache[_i].max_w){
  *_x+=(int)((*_w-_this->cache[_i].max_w)*c->alignx);
  *_w=_this->cache[_i].max_w;}
 if(!(c->fill&GLWC_VERTICAL)&&*_h>_this->cache[_i].pre_h){
  *_y+=(int)((*_h-_this->cache[_i].pre_h)*c->aligny);
  *_h=_this->cache[_i].pre_h;}
 else if(_this->cache[_i].max_h>=0&&*_h>_this->cache[_i].max_h){
  *_y+=(int)((*_h-_this->cache[_i].max_h)*c->aligny);
  *_h=_this->cache[_i].max_h;} }

static void glwGBLLayout(GLWGridBagLayout *_this,GLWComponent *_comp){
 if(_comp->comps.size){
  GLWComponent **comps;
  int            i;
  int            w;
  int            h;
  int            dw;
  int            dh;
  int           *min_w;
  int           *min_h;
  glwGBLPreSize(_this,_comp,&w,&h);
  if(w>_comp->bounds.w||h>_comp->bounds.h){
   glwGBLMinSize(_this,_comp,&w,&h);
   min_w=_this->min_w;
   min_h=_this->min_h;}
  else{
   min_w=_this->pre_w;
   min_h=_this->pre_h;}
  dw=_comp->bounds.w-w;
  if(dw){
   double weight;
   if(_this->adj_w==NULL){
    _this->adj_w=(int *)calloc(_this->w,sizeof(int));
    if(_this->adj_w==NULL)return;}
   weight=0;
   for(i=0;i<_this->w;i++)weight+=_this->weight_x[i];
   if(weight>0){
    for(i=0;i<_this->w;i++){
     int dx=(int)((double)dw*_this->weight_x[i]/weight);
     if(min_w[i]+dx<0)dx=-min_w[i];
     _this->adj_w[i]=dx;
     w+=dx;} }
   else if(_this->w>0){
    int dx=dw/_this->w;
    for(i=0;i<_this->w;i++){
     if(min_w[i]+dx<0)dx=-min_w[i];
     _this->adj_w[i]=dx;
     w+=dx;} }
   dw=_comp->bounds.w-w;}
  dh=_comp->bounds.h-h;
  if(dh){
   double weight;
   if(_this->adj_h==NULL){
    _this->adj_h=(int *)calloc(_this->h,sizeof(int));
    if(_this->adj_h==NULL)return;}
   weight=0;
   for(i=0;i<_this->h;i++)weight+=_this->weight_y[i];
   if(weight>0){
    for(i=0;i<_this->h;i++){
     int dy=(int)((double)dh*_this->weight_y[i]/weight);
     if(min_h[i]+dy<0)dy=-min_h[i];
     _this->adj_h[i]=dy;
     h+=dy;} }
   else if(_this->h>0){
    int dy=dh/_this->h;
    for(i=0;i<_this->h;i++){
     if(min_h[i]+dy<0)dy=-min_h[i];
     _this->adj_h[i]=dy;
     h+=dy;} }
   dh=_comp->bounds.h-h;}
  _this->start_x=(int)(dw*_comp->constraints.alignx);
  _this->start_y=_comp->bounds.h-(int)(dh*(1-_comp->constraints.aligny));
  comps=_DAGetAt(&_comp->comps,0,GLWComponent *);
  for(i=0;(size_t)i<_comp->comps.size;i++)if(glwCompIsVisible(comps[i])){
   int j;
   int cx;
   int cy;
   int cw;
   int ch;
   cx=_this->start_x;
   for(j=0;j<_this->cache[i].x;j++){
    cx+=min_w[j];
    if(_this->adj_w!=NULL)cx+=_this->adj_w[j];}
   cy=_this->start_y;
   for(j=0;j<_this->cache[i].y;j++){
    cy-=min_h[j];
    if(_this->adj_h!=NULL)cy-=_this->adj_h[j];}
   cw=0;
   for(j=_this->cache[i].x;j<_this->cache[i].x+_this->cache[i].w;j++){
    cw+=min_w[j];
    if(_this->adj_w!=NULL)cw+=_this->adj_w[j];}
   ch=0;
   for(j=_this->cache[i].y;j<_this->cache[i].y+_this->cache[i].h;j++){
    ch+=min_h[j];
    if(_this->adj_h!=NULL)ch+=_this->adj_h[j];}
   cy-=ch;
   glwGBLAdjust(_this,_comp,i,&cx,&cy,&cw,&ch);
   if(cw<0||ch<0)glwCompSetBounds(comps[i],0,0,0,0);
   else glwCompSetBounds(comps[i],cx,cy,cw,ch);} } }


GLWGridBagLayout *glwGridBagLayoutAlloc(void){
 GLWGridBagLayout *this_;
 this_=(GLWGridBagLayout *)malloc(sizeof(GLWGridBagLayout));
 if(this_!=NULL)glwGridBagLayoutInit(this_);
 return this_;}

void glwGridBagLayoutInit(GLWGridBagLayout *_this){
 _this->super.layout=(GLWLayoutFunc)glwGBLLayout;
 _this->super.invalidate=(GLWLayoutFunc)glwGBLInvalidate;
 _this->super.min_size=(GLWLayoutSizeFunc)glwGBLMinSize;
 _this->super.pre_size=(GLWLayoutSizeFunc)glwGBLPreSize;
 _this->super.max_size=NULL;
 _this->super.dispose=(GLWLayoutFunc)glwGridBagLayoutFree;
 _this->min_w=NULL;
 _this->min_h=NULL;
 _this->pre_w=NULL;
 _this->pre_h=NULL;
 _this->adj_w=NULL;
 _this->adj_h=NULL;
 _this->weight_x=NULL;
 _this->weight_y=NULL;
 _this->cache=NULL;
 _this->comp=NULL;
 _this->valid=0;}

void glwGridBagLayoutDstr(GLWGridBagLayout *_this){
 glwGBLInvalidate(_this,NULL);}

void glwGridBagLayoutFree(GLWGridBagLayout *_this){
 glwGridBagLayoutDstr(_this);
 free(_this);}

#endif                                                           /*_glwgridb_C*/
