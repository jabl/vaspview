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
#pragma hdrstop
#if !defined(_ds3view_C)
# define _ds3view_C (1)

typedef struct DS3ViewParams{
 double box[2][3];
 Vect3d cntr;
 double zoom;}DS3ViewParams;

static int  ds3ViewGetClipBox(DS3View *_this,int _x0,int _y0,int _x1,int _y1,
                              Vect3d _box[2]);
static int  ds3ViewGetRayLineISect(DS3View *_this,Vect3d _p,double *_t,
                                   const Vect3d _p0,const Vect3d _p1,
                                   const Vect3d _q0,const Vect3d _q1,
                                   double _e);
static int  ds3ViewGetAxesPoint(DS3View *_this,Vect3d _p,double *_t,
                                const Vect3d _p0,const Vect3d _p1);
static int  ds3ViewRaySphereISect(DS3View *_this,Vect3d _p,double *_t,
                                  const Vect3d _p0,const Vect3d _p1,
                                  const Vect3d _c,double _r);
static int  ds3ViewGetPointsPoint(DS3View *_this,Vect3d _p,double *_t,
                                  const Vect3d _p0,const Vect3d _p1);
# if defined(__DS3_ADD_BONDS__)
static int  ds3ViewGetBondsPoint(DS3View *_this,Vect3d _p,double *_t,
                                 const Vect3d _p0,const Vect3d _p1);
# endif
static int  ds3ViewGetSlicePoint(DS3View *_this,Vect3d _p,double *_t,
                                 const Vect3d _p0,const Vect3d _p1);
static void ds3ViewTransferCapture(DS3View *_this,int _x,int _y);

static DS3ViewComp *ds3ViewCompAlloc(DS3View *_ds3view);



/*Expands the yaw, pitch, and roll angles into a full 3x3 rotation matrix.
  This is equal to
  [[cos(r),-sin(r),0],   [[ cos(p),0,sin(p)],   [[1,  0   ,   0   ],
   [sin(r), cos(r),0], *  [   0   ,1,  0   ], *  [0,cos(y),-sin(y)],
   [  0   ,   0   ,1]]    [-sin(p),0,cos(p)]]    [0,sin(y), cos(y)]]
  And multiplication by a column vector on the right will perform the
  rotation.*/
void ds3ViewExpandRot(double _y,double _p,double _r,double _rot[3][3]){
 double sx,cx,sy,cy,sz,cz,szsy,czsy;
 sx=sin(_y*(M_PI/180));
 cx=cos(_y*(M_PI/180));
 sy=sin(_p*(M_PI/180));
 cy=cos(_p*(M_PI/180));
 sz=sin(_r*(M_PI/180));
 cz=cos(_r*(M_PI/180));
 szsy=sz*sy;
 czsy=cz*sy;
 _rot[0][0]=cz*cy;
 _rot[0][1]=czsy*sx-sz*cx;
 _rot[0][2]=czsy*cx+sz*sx;
 _rot[1][0]=sz*cy;
 _rot[1][1]=szsy*sx+cz*cx;
 _rot[1][2]=szsy*cx-cz*sx;
 _rot[2][0]=-sy;
 _rot[2][1]=cy*sx;
 _rot[2][2]=cy*cx;}


static void ds3ViewLayout(GLWLayoutManager *_this,DS3View *_ds3view){
 GLWRect *b;
 b=&_ds3view->super.bounds;
 glwCompSetBounds(&_ds3view->cm_axes->super,0,0,b->w,b->h);
 glwCompSetBounds(&_ds3view->cm_box->super,0,0,b->w,b->h);
 glwCompSetBounds(&_ds3view->cm_slice->super,0,0,b->w,b->h);
 glwCompSetBounds(&_ds3view->cm_iso->super,0,0,b->w,b->h);
 glwCompSetBounds(&_ds3view->cm_pts->super,0,0,b->w,b->h);}


static GLWLayoutManager ds3_view_layout={
 (GLWLayoutFunc)ds3ViewLayout,
 NULL,
 NULL,
 NULL,
 NULL};

/*The general strategy here is to clip lines of the box against each plane
  defined by the rectangle drawn by the user. All the intersections points
  are collected, and the closest one used to shrink a dimension of the box,
  and this is repeated until the box cannot be clipped by any more planes. If
  at any time the box falls entirely outside of the rectangle, we return
  failure. I used to try to do this in the correct order initially, instead
  of generating all the points and seeing which was closest, but it didn't
  work very well. Once an intersection is resolved, all the other
  intersections which are still in the box could be saved, but it seems more
  work to check to see if we already have them than to just go ahead and
  generate them again.*/
static int ds3ViewGetClipBox(DS3View *_this,int _x0,int _y0,int _x1,int _y1,
                             Vect3d _box[2]){
 typedef struct DS3EdgeIsect{
  int    idx;
  int    axs;
  int    pln;
  int    dir;
  double t;}DS3EdgeIsect;
 DS3EdgeIsect isects[32];
 int          nisects;
 double       plane[4][4];
 int          ff[6];
 Vect3d       p0[4];
 Vect3d       p1[4];
 Vect3d       b[8];
 Vect3d       z;
 Vect3d       eye;
 double       d[4][8];
 double       id;
 int          i;
 int          j;
 int          k;
 int          l;
 if(_x0==_x1||_y0==_y1)return 0;
 if(_x0>_x1){
  i=_x0;
  _x0=_x1;
  _x1=i;}
 if(_y0>_y1){
  i=_y0;
  _y0=_y1;
  _y1=i;}
 for(i=0;i<8;i++){
  for(j=0;j<3;j++)b[i][j]=_this->box[i&1<<j?1:0][j];}
 /*We only clip sides of the box that are facing the viewer, so back-transform
   the eyepoint, and figure out which planes are on which side of it*/
 vectMul3d(p0[0],_this->rot[Z],_this->zoom);
 vectAdd3d(p0[0],p0[0],_this->cntr);
 for(i=0;i<3;i++){
  eye[i]=vectDot3d(p0[0],_this->basinv[i]);
  z[i]=vectDot3d(_this->rot[Z],_this->basinv[i]);}
 switch(_this->proj){
  case DS3V_PROJECT_ORTHOGRAPHIC:{
   for(i=0;i<3;i++){
    p0[0][i]=vectDot3d(_this->cntr,_this->basinv[i]);
    ff[i<<1]=eye[i]<p0[0][i];
    ff[(i<<1)+1]=eye[i]>p0[0][i];} }break;
  /*case DS3V_PROJECT_PERSPECTIVE :*/
  default                       :{
   for(i=0;i<3;i++){
    ff[i<<1]=eye[i]<b[0][i];
    ff[(i<<1)+1]=eye[i]>b[7][i];} }break;}
 /*Set up the clip planes*/
 ds3ViewGetUnprojRay(_this,_x0,_y0,p0[0],p1[0]);
 ds3ViewGetUnprojRay(_this,_x0,_y1,p0[1],p1[1]);
 ds3ViewGetUnprojRay(_this,_x1,_y1,p0[2],p1[2]);
 ds3ViewGetUnprojRay(_this,_x1,_y0,p0[3],p1[3]);
 for(i=0;i<4;i++){
  Vect3d p;
  vectSet3dv(p,p0[i]);
  for(j=0;j<3;j++)p0[i][j]=vectDot3d(_this->basinv[j],p);
  vectSet3dv(p,p1[i]);
  for(j=0;j<3;j++)p1[i][j]=vectDot3d(_this->basinv[j],p);}
 for(i=0;i<4;i++){
  Vect3d d0;
  Vect3d d1;
  j=i+1&3;
  vectSub3d(d0,p1[i],p0[i]);
  vectSub3d(d1,p1[j],p0[i]);
  vectCross3d(plane[i],d0,d1);
  plane[i][W]=-vectDot3d(p0[i],plane[i]);}
 for(k=0;k<4;k++)for(i=0;i<8;i++)d[k][i]=vectDot3d(b[i],plane[k])+plane[k][W];
 for(;;){
  for(k=0;k<4;k++){   /*Make sure the clip box is never completely outside the*/
   for(i=0;i<8;i++)if(d[k][i]>=0)break;         /*clip rectangle. If so, abort*/
   if(i>=8)return 0;}
  /*Generate intersection points*/
  nisects=0;
  for(i=0;i<7;i++)for(j=0;j<3;j++)if(!(i&1<<j)){
   for(l=0;l<3;l++)if(l!=j&&ff[(l<<1)+((i&1<<l)?1:0)])break;
   if(l>=3)continue;
   for(k=0;k<4;k++){
    if(d[k][i]<-1E-8&&d[k][i|1<<j]>1E-8&&fabs(plane[k][j])>=1E-16){
     double t;
     t=b[i][j]-d[k][i]/plane[k][j];
     if(b[0][j]>=t||t>b[7][j]){
      t=b[i|1<<j][j]-d[k][i|1<<j]/plane[k][j];}
     if(b[0][j]<t&&t<=b[7][j]){
      isects[nisects].idx=i;
      isects[nisects].axs=j;
      isects[nisects].pln=k;
      isects[nisects].dir=0;
      isects[nisects++].t=t;} }
    else if(d[k][i]>1E-8&&d[k][i|1<<j]<-1E-8&&fabs(plane[k][j])>=1E-16){
     double t;
     t=b[i][j]-d[k][i]/plane[k][j];
     if(b[7][j]<=t||t<b[0][j]){
      t=b[i|1<<j][j]-d[k][i|1<<j]/plane[k][j];}
     if(b[7][j]>t&&t>=b[0][j]){
      isects[nisects].idx=i;
      isects[nisects].axs=j;
      isects[nisects].pln=k;
      isects[nisects].dir=1;
      isects[nisects++].t=t;} } } }
  /*Pick intersection point closest to the eye*/
  l=-1;
  for(i=0;i<nisects;i++){
   DS3EdgeIsect *s;
   Vect3d        p;
   double        t;
   s=&isects[i];
   vectSet3dv(p,b[s->idx|s->dir<<s->axs]);
   p[s->axs]=s->t;
   vectSub3d(p,p,eye);
   t=vectDot3d(z,p);
   if(l<0||t>id){
    l=i;
    id=t;} }
  /*Do the intersection*/
  if(l>=0){
   DS3EdgeIsect *s;
   s=&isects[l];
   for(i=0;i<8;i++)if((i&1<<s->axs?1:0)==s->dir){
    b[i][s->axs]=s->t;
    for(k=0;k<4;k++)d[k][i]=vectDot3d(b[i],plane[k])+plane[k][W];}
   /*if(_this->proj!=DS3V_PROJECT_ORTHOGRAPHIC){
    if(!s->dir)ff[s->axs<<1]=eye[s->axs]<b[0][s->axs];
    else ff[(s->axs<<1)+s->dir]=eye[s->axs]>b[7][s->axs];}*/}
  else break;}
 vectSet3dv(_box[0],b[0]);
 vectSet3dv(_box[1],b[7]);
 return 1;}

static void ds3ViewPeerDisplayChildren(DS3View *_this,
                                       const GLWCallbacks *_cb){
 GLWComponent *p;
 int           dx,dy;
 for(dx=0,p=&_this->super;p!=NULL;p=p->parent)dx+=p->bounds.x;
 for(dy=0,p=&_this->super;p!=NULL;p=p->parent)dy+=p->bounds.y;
 if(_this->ds3!=NULL&&_this->super.bounds.w>0&&_this->super.bounds.h>0){
  static const GLfloat LIGHT0_A[4]={0.15F,0.15F,0.15F,1.F};
  static const GLfloat LIGHT0_D[4]={0.65F,0.65F,0.65F,1.F};
  static const GLfloat LIGHT0_S[4]={0.25F,0.25F,0.25F,1.F};
  static const GLfloat LIGHT1_A[4]={0.575F,0.575F,0.575F,1.F};
  static const GLfloat LIGHT1_D[4]={0.825F,0.825F,0.825F,1.F};
  static const GLfloat LIGHT1_S[4]={0.625F,0.625F,0.625F,1.F};
  static const GLfloat LIGHT_P[4]={-1.F,1.F,0.F,0.F};
  GLdouble aspect;
  GLdouble zoom;
  GLdouble offs;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glScissor(dx,dy,_this->super.bounds.w,_this->super.bounds.h);
  glEnable(GL_SCISSOR_TEST);
  glViewport(dx,dy,_this->super.bounds.w,_this->super.bounds.h);
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);            /*Set up the viewing transformation:*/
  glLoadIdentity();
  aspect=_this->super.bounds.w*DS3V_ASPECT/_this->super.bounds.h;
  zoom=(6.0/256)*(_this->offs>_this->zoom?_this->offs:_this->zoom);
  offs=_this->zoom<1E-4?1E-4:_this->zoom;
  switch(_this->proj){
   case DS3V_PROJECT_ORTHOGRAPHIC:{
    if(aspect>=1){
     glOrtho(-offs*aspect,offs*aspect,-offs,offs,zoom,256*zoom);}
    else{
     glOrtho(-offs,offs,-offs/aspect,offs/aspect,zoom,256*zoom);} }break;
   /*case DS3V_PROJECT_PERSPECTIVE :*/
   default                       :{
    if(aspect>=1)glFrustum(-zoom*aspect,zoom*aspect,-zoom,zoom,zoom,256*zoom);
    else glFrustum(-zoom,zoom,-zoom/aspect,zoom/aspect,zoom,256*zoom);}break;}
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  if(_this->draw_points||_this->draw_iso){           /*These items are lighted*/
   glLightfv(GL_LIGHT0,GL_AMBIENT,LIGHT0_A);
   glLightfv(GL_LIGHT0,GL_DIFFUSE,LIGHT0_D);
   glLightfv(GL_LIGHT0,GL_SPECULAR,LIGHT0_S);
   glLightfv(GL_LIGHT0,GL_POSITION,LIGHT_P);
   if(_this->track_mp>=0&&glwCompIsCapturing(&_this->cm_pts->super)
# if defined(__DS3_ADD_BONDS__)
      ||_this->track_mbf>=0&&glwCompIsCapturing(&_this->cm_bnds->super)
# endif
      ){
    glLightfv(GL_LIGHT1,GL_AMBIENT,LIGHT1_A);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,LIGHT1_D);
    glLightfv(GL_LIGHT1,GL_SPECULAR,LIGHT1_S);
    glLightfv(GL_LIGHT1,GL_POSITION,LIGHT_P);}
   glEnable(GL_LIGHT0);}
  glTranslated(0,0,-_this->zoom);
  glRotated(_this->roll,0,0,1);
  glRotated(_this->pitch,0,1,0);
  glRotated(_this->yaw,1,0,0);
  glTranslated(-_this->cntr[0],-_this->cntr[1],-_this->cntr[2]);
# if defined(__DS3_ADD_BONDS__)
  if(_this->draw_slice||_this->draw_iso||_this->draw_points){
# else
  if(_this->draw_slice||_this->draw_iso){            /*These items are clipped*/
# endif
   GLdouble plane[4];
   int      i;
   glPushMatrix();
   glMultMatrixd(_this->basis);
   plane[Y]=plane[Z]=0;
   for(i=0;i<3;i++){
    plane[i]=1;
    plane[W]=-_this->box[0][i]+1E-4;
    glClipPlane(GL_CLIP_PLANE0+(i<<1),plane);
    plane[i]=-1;
    plane[W]=_this->box[1][i]+1E-4;
    glClipPlane(GL_CLIP_PLANE0+(i<<1)+1,plane);
    plane[i]=0;}
   glPopMatrix();}
  if(_this->draw_coords){                                          /*Draw axes*/
   glwCompDisplay(&_this->cm_axes->super);}
  if(_this->draw_points){                                    /*Draw the points*/
# if defined(__DS3_ADD_BONDS__)
   glwCompDisplay(&_this->cm_bnds->super);
# endif
   glwCompDisplay(&_this->cm_pts->super);}
  if(_this->draw_slice){                                 /*Draw the data slice*/
   glwCompDisplay(&_this->cm_slice->super);}
  if(_this->draw_iso){                                  /*Draw the iso-surface*/
   glwCompDisplay(&_this->cm_iso->super);}
  if(_this->draw_coords){                                  /*Draw bounding box*/
   glwCompDisplay(&_this->cm_box->super);}
  if(_this->track_cb){                           /*Draw the projected clip box*/
   Vect3d box[2];
   if(ds3ViewGetClipBox(_this,_this->track_cx,_this->track_cy,
                        _this->track_mx,_this->track_my,box)){
    GLWcolor c;
    int      i;
    int      j;
    int      k;
    c=glwColorBlend(_this->super.forec,DS3V_FOCUS_COLOR);
    glwColor(c);
    glPushMatrix();
    glMultMatrixd(_this->basis);
    glBegin(GL_LINES);
    for(i=0;i<7;i++)for(j=0;j<3;j++)if(!(i&1<<j)){
     Vect3d p0;
     Vect3d p1;
     for(k=0;k<3;k++){
      if(j!=k)p0[k]=p1[k]=box[(i&1<<k)?1:0][k];
      else{
       p0[k]=_this->box[0][k];
       p1[k]=_this->box[1][k];} }
     glVertex3dv(p0);
     glVertex3dv(p1);}
    glEnd();
    glwColor(c&0x3FFFFFFF);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    for(i=0;i<6;i++){
     int xbit;
     int ybit;
     int zbit;
     zbit=1<<(i>>1);
     zbit=zbit<<3|zbit;
     xbit=zbit>>1&7;
     ybit=zbit>>2&7;
     zbit=(i&1)?(zbit&7):0;
     glVertex3d(box[zbit&1][X],box[(zbit&2)>>1][Y],box[(zbit&4)>>2][Z]);
     zbit|=xbit;
     glVertex3d(box[zbit&1][X],box[(zbit&2)>>1][Y],box[(zbit&4)>>2][Z]);
     zbit|=ybit;
     glVertex3d(box[zbit&1][X],box[(zbit&2)>>1][Y],box[(zbit&4)>>2][Z]);
     zbit^=xbit;
     glVertex3d(box[zbit&1][X],box[(zbit&2)>>1][Y],box[(zbit&4)>>2][Z]);}
    glEnd();
    glPopMatrix();} }
  glPopAttrib();}
 if(_this->track_cb){                                      /*Draw the clip box*/
  glPushAttrib(GL_SCISSOR_BIT|GL_CURRENT_BIT|
               GL_VIEWPORT_BIT|GL_TRANSFORM_BIT);
  glScissor(dx,dy,_this->super.bounds.w,_this->super.bounds.h);
  glEnable(GL_SCISSOR_TEST);
  glViewport(dx,dy,_this->super.bounds.w,_this->super.bounds.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0-1E-4,_this->super.bounds.w-1E-4,
             0-1E-4,_this->super.bounds.h-1E-4);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glwColor(_this->super.forec);
  glBegin(GL_LINE_LOOP);
  glVertex2i(_this->track_mx,_this->track_my);
  glVertex2i(_this->track_mx,_this->track_cy);
  glVertex2i(_this->track_cx,_this->track_cy);
  glVertex2i(_this->track_cx,_this->track_my);
  glEnd();
  glPopAttrib();} }

/*Gets the intersection between a ray and a line segment. The ray must
  be non-zero, and is specified by two points, p0 and p1. The line is
  defined by endpoints q0 and q1. The lines must come within e of each
  other for them to be considered intersecting. The returned intersection
  point is always the point on the line segment that is closest to the
  ray. If there is more than one such point, the one closest to the
  origin of the ray is returned.
  p:  The returned point of intersection
  t:  The parametric position on the ray of the point of intersection
  p0: The origin of the ray
  p1: A point along the ray
  q0: The start of the line segment
  q1: The end of the line segment
  e:  The maximum distance to the line still considered an intersection
  Return: 0 iff there was no point of intersection*/
static int ds3ViewGetRayLineISect(DS3View *_this,Vect3d _p,double *_t,
                                  const Vect3d _p0,const Vect3d _p1,
                                  const Vect3d _q0,const Vect3d _q1,
                                  double _e){
 Vect3d dp;
 Vect3d dq;
 Vect3d d0;
 Vect3d p;
 double a,b,c,d,e,f,s,t;
 vectSub3d(dp,_p1,_p0);
 vectSub3d(dq,_q1,_q0);
 a=vectMag2_3d(dp);
 b=vectDot3d(dp,dq);
 c=vectMag2_3d(dq);
 vectSub3d(d0,_q0,_p0);
 e=vectDot3d(d0,dp);
 f=vectDot3d(d0,dq);
 d=a*c-b*b;
 s=c*e-b*f;
 t=b*e-a*f;
 if(d<0){
  d=-d;
  s=-s;
  t=-t;}
 if(d<1E-100){                /*Degenerate case: try endpoints of line segment*/
  Vect3d d1;
  /*a is the length^2 of the ray: always >> 0*/
  s=-e/a;
  if(s<0)s=0;
  t=(b-e)/a;
  if(t<0)t=0;
  vectMul3d(p,dp,s);
  vectAdd3d(p,p,_p0);
  vectSub3d(d0,p,_q0);
  vectMul3d(p,dp,t);
  vectAdd3d(p,p,_p0);
  vectSub3d(d1,p,_q1);
  if(vectMag2_3d(d0)<_e*_e){
   if(vectMag2_3d(d1)<_e*_e&&t<s){
    vectSet3dv(_p,_q1);
    *_t=t;}
   else{
    vectSet3dv(_p,_q0);
    *_t=s;}
   return 1;}
  else if(vectMag2_3d(d1)<_e*_e){
   vectSet3dv(_p,_q1);
   *_t=t;
   return 1;} }
 else{                                                  /*Exactly one solution*/
  if(s<0)s=0;
  else s/=d;
  if(t<0)t=0;
  else if(t>d)t=1;
  else t/=d;
  vectMul3d(p,dp,s);
  vectAdd3d(p,p,_p0);
  vectMul3d(_p,dq,t);
  vectAdd3d(_p,_p,_q0);
  vectSub3d(p,p,_p);
  if(vectMag2_3d(p)<_e*_e){
   *_t=s;
   return 1;} }
 return 0;}

/*Gets the closest intersection point (the one with the smallest t) with the
  mouse position and the coordinate axes (and clip box). The mouse position
  ray must be non-zero
  p:  The returned intersection point
  t:  The parametric position on the ray of the point of intersection
  p0: The origin of the unprojected mouse ray
  p1: A point along the unprojected mouse ray
  Return: 0 iff there was no point of intersection*/
static int ds3ViewGetAxesPoint(DS3View *_this,Vect3d _p,double *_t,
                               const Vect3d _p0,const Vect3d _p1){
 int    ret;
 ret=0;
 if(_this->ds3!=NULL){
  double e;
  e=_this->super.bounds.w*_this->super.bounds.h;
  if(e>0){
   Vect3d p;
   Vect3d o[2][3];
   Vect3d b[8];
   double t;
   int    i;
   int    j;
   int    k;
   e=64*sqrt(1/e)*_this->zoom;
   if(_this->offs>1E-16)e/=_this->offs;
   for(i=0;i<2;i++)for(j=0;j<3;j++){
    for(k=0;k<3;k++)o[i][j][k]=_this->ds3->basis[k][j]*_this->box[i][j];}
   for(i=0;i<8;i++){
    vectSet3d(b[i],0,0,0);
    for(j=0;j<3;j++)vectAdd3d(b[i],b[i],o[(i&1<<j)?1:0][j]);}
   for(i=0,k=3;i<7;i++)for(j=1;j<8;j<<=1)if(!(i&j)){
    if(ds3ViewGetRayLineISect(_this,p,&t,_p0,_p1,b[i],b[i|j],e)){
     if(!ret||t<*_t){
      _this->track_ax=k;
      *_t=t;
      vectSet3dv(_p,p);
      ret=1;} }
    k++;}
   for(j=0;j<3;j++)if(_this->box[0][j]>0)vectSet3d(o[0][j],0,0,0);
   for(j=0;j<3;j++)if(_this->box[1][j]<1){
    for(k=0;k<3;k++)o[1][j][k]=_this->ds3->basis[k][j];}
   for(j=0;j<3;j++){
    if(ds3ViewGetRayLineISect(_this,p,&t,_p0,_p1,o[0][j],o[1][j],e)){
     if(!ret||t<=*_t){
      _this->track_ax=j;
      *_t=t;
      vectSet3dv(_p,p);
      ret=1;} } } } }
 if(!ret)_this->track_ax=-1;
 return ret;}

/*Gets the closest intersection point (the one with the smallest t) between
  a ray and a sphere. The ray must be non-zero.
  p:  The point of intersection
  t:  The parametric position on the ray of the point of intersection
  p0: The origin of the ray
  p1: A point along the ray
  c:  The center of the sphere
  r:  The radius of the sphere
  Return: 0 iff there was no intersection point*/
static int ds3ViewRaySphereISect(DS3View *_this,Vect3d _p,double *_t,
                                 const Vect3d _p0,const Vect3d _p1,
                                 const Vect3d _c,double _r){
 Vect3d p0;
 Vect3d dp;
 double a,b,c,d;
 vectSub3d(dp,_p1,_p0);
 vectSub3d(p0,_p0,_c);
 a=vectMag2_3d(dp);
 b=vectDot3d(dp,p0);
 c=vectMag2_3d(p0)-_r*_r;
 d=b*b-a*c;
 if(d<0)return 0;
 *_t=(-b-sqrt(d))/a;
 if(*_t<0)return 0;
 vectMul3d(_p,dp,*_t);
 vectAdd3d(_p,_p,_p0);
 return 1;}

/*Gets the closest intersection point (the one with the smallest t) with the
  mouse position and any of the atoms that are currently visible. The mouse
  position ray must be non-zero
  p:  The returned intersection point
  t:  The parametric position on the ray of the point of intersection
  p0: The origin of the unprojected mouse ray
  p1: A point along the unprojected mouse ray
  Return: 0 iff there was no point of intersection*/
static int ds3ViewGetPointsPoint(DS3View *_this,Vect3d _p,double *_t,
                                 const Vect3d _p0,const Vect3d _p1){
 int    ret;
 ret=0;
 if(_this->ds3!=NULL){
  int    x[3];
  int    x0[3];
  int    x1[3];
  size_t i;
  int    j;
  for(j=0;j<3;j++){
   x0[j]=(int)floor(_this->box[0][j]);
   x1[j]=(int)ceil(_this->box[1][j]);}
  for(x[X]=x0[X];x[X]<x1[X];x[X]++){
   for(x[Y]=x0[Y];x[Y]<x1[Y];x[Y]++){
    for(x[Z]=x0[Z];x[Z]<x1[Z];x[Z]++){
     for(i=0;i<_this->ds3->npoints;i++){
      if(ds3ViewGetPointVisible(_this,(long)i)||
         (size_t)_this->track_sp==i&&glwCompIsFocused(&_this->cm_pts->super)){
       Vect3d p;
       Vect3d q;
       double t;
       for(j=0;j<3;j++){                /*Make sure point falls within our box*/
        double d;
        d=_this->ds3->points[i].pos[j]+x[j];
        if(d<_this->box[0][j]||d>_this->box[1][j])break;}
       if(j!=3)continue;
       /*It would probably be faster to back-transform the ray into data-set
         coordinates, but the basis matrix is not guaranteed to be invertible*/
       vectSet3d(q,_this->ds3->points[i].pos[X]+x[X],
                 _this->ds3->points[i].pos[Y]+x[Y],
                 _this->ds3->points[i].pos[Z]+x[Z]);
       for(j=0;j<3;j++)p[j]=vectDot3d(_this->ds3->basis[j],q);
       if(ds3ViewRaySphereISect(_this,p,&t,_p0,_p1,p,_this->point_r)){
        if(!ret||t<*_t){
         _this->track_mp=(long)i;
         vectSet3dv(_p,p);
         *_t=t;
         ret=1;} } } } } } } }
 if(!ret)_this->track_mp=-1;
 return ret;}

# if defined(__DS3_ADD_BONDS__)
/*Gets the closest intersection point (the one with the smallest t) with the
  mouse position and any of the bonds that are currently visible. The mouse
  position ray must be non-zero
  p:  The returned intersection point
  t:  The parametric position on the ray of the point of intersection
  p0: The origin of the unprojected mouse ray
  p1: A point along the unprojected mouse ray
  Return: 0 iff there was no point of intersection*/
static int ds3ViewGetBondsPoint(DS3View *_this,Vect3d _p,double *_t,
                                const Vect3d _p0,const Vect3d _p1){
 int ret;
 ret=0;
 if(_this->bonds.nbonds>0){
  double e;
  e=_this->super.bounds.w*_this->super.bounds.h;
  if(e>0){
   DS3Bonds *bonds;
   int       x[3];
   int       x0[3];
   int       x1[3];
   long      i;
   long      j;
   long      k;
   int       l;
   e=32*sqrt(1/e);
   bonds=&_this->bonds;
   for(l=0;l<3;l++){
    x0[l]=(int)floor(_this->box[0][l])-1;
    x1[l]=(int)ceil(_this->box[1][l]);}
   for(x[X]=x0[X];x[X]<x1[X];x[X]++){
    for(x[Y]=x0[Y];x[Y]<x1[Y];x[Y]++){
     for(x[Z]=x0[Z];x[Z]<x1[Z];x[Z]++){
      for(i=0,j=0;j<bonds->natoms-1;j++){
       if(ds3ViewGetPointVisible(_this,j)){
        for(k=j+1;k<bonds->natoms;k++,i++){
         if(ds3ViewGetPointVisible(_this,k)&&bonds->bonds[i]>0){
          Vect3d   p0;
          Vect3d   p1;
          Vect3d   q0;
          Vect3d   q1;
          Vect3d   p;
          double   t;
          double   d;
          vectSet3d(q0,_this->ds3->points[j].pos[X]+x[X],
                    _this->ds3->points[j].pos[Y]+x[Y],
                    _this->ds3->points[j].pos[Z]+x[Z]);
          vectSet3d(q1,_this->ds3->points[k].pos[X]+x[X],
                    _this->ds3->points[k].pos[Y]+x[Y],
                    _this->ds3->points[k].pos[Z]+x[Z]);
          for(l=0;l<3;l++){
           d=q0[l]-q1[l];
           if(d<-0.5)q0[l]+=1;
           else if(d>0.5)q1[l]+=1;}
          for(l=0;l<3;l++){        /*Clip the ray against our actual box bounds*/
           if(q0[l]<_this->box[0][l]){
            double t;
            if(q1[l]<_this->box[0][l])break;
            t=(_this->box[0][l]-q0[l])/(q1[l]-q0[l]);
            if(t<0||t>1)t=1;
            vectSub3d(p0,q1,q0);
            vectMul3d(p0,p0,t);
            vectAdd3d(q0,q0,p0);}
           if(q0[l]>_this->box[1][l]){
            double t;
            if(q1[l]>_this->box[1][l])break;
            t=(_this->box[1][l]-q0[l])/(q1[l]-q0[l]);
            if(t<0||t>1)t=1;
            vectSub3d(p0,q1,q0);
            vectMul3d(p0,p0,t);
            vectAdd3d(q0,q0,p0);}
           if(q1[l]<_this->box[0][l]){
            double t;
            if(q0[l]<_this->box[0][l])break;
            t=(_this->box[0][l]-q1[l])/(q0[l]-q1[l]);
            if(t<0||t>1)t=1;
            vectSub3d(p0,q0,q1);
            vectMul3d(p0,p0,t);
            vectAdd3d(q1,q1,p0);}
           if(q1[l]>_this->box[1][l]){
            double t;
            if(q0[l]>_this->box[1][l])break;
            t=(_this->box[1][l]-q1[l])/(q0[l]-q1[l]);
            if(t<0||t>1)t=1;
            vectSub3d(p0,q0,q1);
            vectMul3d(p0,p0,t);
            vectAdd3d(q1,q1,p0);} }
          if(l!=3)continue;
          for(l=0;l<3;l++){           /*Transform points into world-coordinates*/
           p0[l]=vectDot3d(_this->ds3->basis[l],q0);
           p1[l]=vectDot3d(_this->ds3->basis[l],q1);}
          if(ds3ViewGetRayLineISect(_this,p,&t,_p0,_p1,p0,p1,
                                    bonds->bonds[i]*_this->point_r+e)){
           if(!ret||t<*_t){
            _this->track_mbf=j;
            _this->track_mbt=k;
            *_t=t;
            vectSet3dv(_p,p);
            ret=1;} } } } }
       else i+=bonds->natoms-1-j;} } } } } }
 if(!ret)_this->track_mbf=_this->track_mbt=-1;
 return ret;}
# endif

/*Gets the closest intersection point (the one with the smallest t) with the
  mouse position and the slice. The mouse position ray must be non-zero
  p:  The returned intersection point
  t:  The parametric position on the ray of the point of intersection
  p0: The origin of the unprojected mouse ray
  p1: A point along the unprojected mouse ray
  Return: 0 iff there was no point of intersection*/
static int ds3ViewGetSlicePoint(DS3View *_this,Vect3d _p,double *_t,
                                const Vect3d _p0,const Vect3d _p1){
 int ret;
 ret=0;
 if(_this->ds3!=NULL){
  Vect3d dp;
  Vect3d m;
  Vect3d n;
  double d;
  double a;
  int    i;
  int    j;
  m[X]=_this->strans[X][Z];
  m[Y]=_this->strans[Y][Z];
  m[Z]=_this->strans[Z][Z];
  for(i=0;i<3;i++){
   n[i]=0;
   for(j=0;j<3;j++)n[i]+=m[j]*_this->basinv[j][i];}
  d=(m[X]+m[Y]+m[Z])*0.5+_this->slice_d;
  vectSub3d(dp,_p1,_p0);
  a=vectDot3d(dp,n);
  if(fabs(a)>=1E-16){
   *_t=(d-vectDot3d(_p0,n))/a;
   vectMul3d(_p,dp,*_t);
   vectAdd3d(_p,_p,_p0);
   for(i=0;i<3;i++)m[i]=vectDot3d(_this->basinv[i],_p);
   for(i=0;i<3;i++)if(m[i]<_this->box[0][i]||m[i]>_this->box[1][i])break;
   if(i==3){
    long x[3];
    for(i=0;i<3;i++){
     x[i]=(long)fmod(m[i]*_this->ds3->density[i],_this->ds3->density[i]);
     if(x[i]<0)x[i]+=_this->ds3->density[i];}
    if(x[X]!=_this->track_dx||x[Y]!=_this->track_dy||x[Z]!=_this->track_dz){
     _this->track_dx=x[X];
     _this->track_dy=x[Y];
     _this->track_dz=x[Z];
     if(_this->data_changed_func!=NULL){
      _this->data_changed_func(_this->data_changed_ctx,
                               &_this->super);} }
    ret=1;} } }
 if(!ret){
  if(_this->track_dx>=0){
   _this->track_dx=-1;
   if(_this->data_changed_func!=NULL){
    _this->data_changed_func(_this->data_changed_ctx,&_this->super);} } }
 return ret;}

/*Converts the mouse position into a ray in world coordinates, and then
  intersects it with all the currently visible objects. The closest object
  that the ray intersects (if any) is given mouse capture
  x: The x-coordinate of the current mouse position
  y: The y-coordinate of the current mouse position*/
static void ds3ViewTransferCapture(DS3View *_this,int _x,int _y){
 Vect3d        p;
 double        t;
 GLWComponent *cap;
 cap=NULL;
 if(_this->ds3!=NULL){
  ds3ViewGetUnprojRay(_this,_x,_y,_this->track_p0,_this->track_p1);
  if(_this->draw_coords&&
     ds3ViewGetAxesPoint(_this,_this->track_pt,&_this->track_t,
                         _this->track_p0,_this->track_p1)){
   cap=&_this->cm_axes->super;}
  if(_this->draw_points&&
     ds3ViewGetPointsPoint(_this,p,&t,_this->track_p0,_this->track_p1)){
   if(cap==NULL||t<_this->track_t){
    cap=&_this->cm_pts->super;
    vectSet3dv(_this->track_pt,p);
    _this->track_t=t;} }
# if defined(__DS3_ADD_BONDS__)
  if(_this->draw_points&&
     ds3ViewGetBondsPoint(_this,p,&t,_this->track_p0,_this->track_p1)){
   if(cap==NULL||t<_this->track_t){
    cap=&_this->cm_bnds->super;
    vectSet3dv(_this->track_pt,p);
    _this->track_t=t;} }
# endif
  if(_this->draw_slice&&
     ds3ViewGetSlicePoint(_this,p,&t,_this->track_p0,_this->track_p1)){
   if(cap==NULL||t<_this->track_t){
    cap=&_this->cm_slice->super;
    vectSet3dv(_this->track_pt,p);
    _this->track_t=t;} } }
 glwCompRequestCapture(&_this->super,cap);}

static int ds3ViewPeerMouse(DS3View *_this,const GLWCallbacks *_cb,
                            int _b,int _s,int _x,int _y){
 ds3ViewTransferCapture(_this,_x,_y);
 if(_s)_this->super.mouse_b|=1<<_b;
 else _this->super.mouse_b&=~(1<<_b);
 if(_this->super.capture!=NULL){
  return glwCompMouse(_this->super.capture,_b,_s,_x,_y);}
 else if(_this->ds3!=NULL){
  if(_s){
   if(_this->track_sp>=0)ds3ViewSetSelectedPoint(_this,-1-_this->track_sp);
# if defined(__DS3_ADD_BONDS__)
   if(_this->track_sbf>=0){
    ds3ViewSetSelectedBond(_this,-1-_this->track_sbf,_this->track_sbt);}
# endif
   glwCompRequestFocus(&_this->cm_axes->super);}
  if(_b==GLUT_LEFT_BUTTON){
   if(_s){
    _this->track_cb=1;
    _this->track_mx=_this->track_cx=_x;
    _this->track_my=_this->track_cy=_y;}
   else{
    Vect3d box[2];
    _this->track_cb=0;
    glwCompRepaint(&_this->super,0);
    if(ds3ViewGetClipBox(_this,_this->track_cx,_this->track_cy,_x,_y,box)){
     DS3ViewParams view;
     vectSet3dv(view.box[0],_this->box[0]);
     vectSet3dv(view.box[1],_this->box[1]);
     vectSet3dv(view.cntr,_this->cntr);
     view.zoom=_this->zoom;
     if((box[0][X]!=_this->box[0][X]||
         box[0][Y]!=_this->box[0][Y]||
         box[0][Z]!=_this->box[0][Z]||
         box[1][X]!=_this->box[1][X]||
         box[1][Y]!=_this->box[1][Y]||
         box[1][Z]!=_this->box[1][Z])&&
       daInsTail(&_this->view_stack,&view)){
      Vect3d p,q,c;
      double d,e;
      int    i,j;
      vectSub3d(q,_this->box[1],_this->box[0]);
      ds3ViewSetBox(_this,box[0][X],box[0][Y],box[0][Z],
                    box[1][X],box[1][Y],box[1][Z]);
      vectAdd3d(p,box[0],box[1]);
      vectMul3d(p,p,0.5);
      for(i=0;i<3;i++){
       c[i]=0;
       for(j=0;j<3;j++)c[i]+=_this->ds3->basis[j][i]*p[j];}
      ds3ViewSetCenter(_this,c[X],c[Y],c[Z]);
      vectSub3d(p,box[1],box[0]);
      d=vectMag2_3d(p);
      e=vectMag2_3d(q);
      if(e>1E-100)d/=e;
      else d*=1.0/3;
      ds3ViewSetZoom(_this,sqrt(d)*_this->zoom);} } } }
  else if(_b==GLUT_RIGHT_BUTTON&&_s){
   if(_this->view_stack.size>0){
    DS3ViewParams *p;
    p=_DAGetAt(&_this->view_stack,_this->view_stack.size-1,DS3ViewParams);
    ds3ViewSetBox(_this,p->box[0][X],p->box[0][Y],p->box[0][Z],
                  p->box[1][X],p->box[1][Y],p->box[1][Z]);
    ds3ViewSetCenter(_this,p->cntr[X],p->cntr[Y],p->cntr[Z]);
    ds3ViewSetZoom(_this,p->zoom);
    daDelTail(&_this->view_stack);}
   else{
    ds3ViewSetBox(_this,0,0,0,1,1,1);
    ds3ViewSetCenter(_this,_this->ds3->center[X],
                     _this->ds3->center[Y],_this->ds3->center[Z]);
    ds3ViewSetZoom(_this,_this->offs);} }
  else return 1;
  return -1;}
 return 0;}

static int ds3ViewPeerMotion(DS3View *_this,const GLWCallbacks *_cb,
                             int _x,int _y){
 int ret;
 ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
 if(ret>=0&&_this->track_cb){
  _this->track_mx=_x;
  _this->track_my=_y;
  glwCompRepaint(&_this->super,0);
  ret=-1;}
 return ret;}

static int ds3ViewPeerPassiveMotion(DS3View *_this,const GLWCallbacks *_cb,
                                    int _x,int _y){
 ds3ViewTransferCapture(_this,_x,_y);
 if(_this->super.capture!=NULL){
  return glwCompPassiveMotion(_this->super.capture,_x,_y);}
 return 0;}

static void ds3ViewPeerDispose(DS3View *_this,const GLWCallbacks *_cb){
 _this->ds3=NULL;
# if defined(__DS3_ADD_BONDS__)
 ds3BondsDstr(&_this->bonds);
# endif
 ds3SliceDstr(&_this->slice,_this);
 ds3IsoDstr(&_this->iso);
 daDstr(&_this->view_stack);}


const GLWCallbacks DS3_VIEW_CALLBACKS={
 &GLW_COMPONENT_CALLBACKS,
 (GLWDisposeFunc)ds3ViewPeerDispose,
 NULL,
 (GLWDisplayFunc)ds3ViewPeerDisplayChildren,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 NULL,
 (GLWMouseFunc)ds3ViewPeerMouse,
 (GLWMotionFunc)ds3ViewPeerMotion,
 (GLWMotionFunc)ds3ViewPeerPassiveMotion};

static DS3ViewComp *ds3ViewCompAlloc(DS3View *_ds3view){
 DS3ViewComp *this_;
 this_=(DS3ViewComp *)malloc(sizeof(DS3ViewComp));
 if(this_!=NULL){
  glwCompInit(&this_->super);
  this_->ds3view=_ds3view;
  return this_;}
 return NULL;}


DS3View *ds3ViewAlloc(void){
 DS3View *this_;
 this_=(DS3View *)malloc(sizeof(DS3View));
 if(this_!=NULL){
  if(ds3ViewInit(this_)){
   return this_;}
  free(this_);}
 return NULL;}

int ds3ViewInit(DS3View *_this){
 glwCompInit(&_this->super);
 _this->cm_axes=ds3ViewCompAlloc(_this);
 _this->cm_box=ds3ViewCompAlloc(_this);
 _this->cm_pts=ds3ViewCompAlloc(_this);
# if defined(__DS3_ADD_BONDS__)
 _this->cm_bnds=ds3ViewCompAlloc(_this);
# endif
 _this->cm_slice=ds3ViewCompAlloc(_this);
 _this->cm_iso=ds3ViewCompAlloc(_this);
 if(_this->cm_axes!=NULL&&_this->cm_box!=NULL&&
# if defined(__DS3_ADD_BONDS__)
    _this->cm_bnds!=NULL&&
# endif
    _this->cm_pts!=NULL&&_this->cm_slice!=NULL&&_this->cm_iso!=NULL){
  if(glwCompAdd(&_this->super,&_this->cm_axes->super,-1)&&
     glwCompAdd(&_this->super,&_this->cm_box->super,-1)&&
     glwCompAdd(&_this->super,&_this->cm_slice->super,-1)&&
     glwCompAdd(&_this->super,&_this->cm_iso->super,-1)&&
# if defined(__DS3_ADD_BONDS__)
     glwCompAdd(&_this->super,&_this->cm_pts->super,-1)&&
     glwCompAdd(&_this->super,&_this->cm_bnds->super,-1)){
# else
     glwCompAdd(&_this->super,&_this->cm_pts->super,-1)){
# endif
   _this->super.callbacks=&DS3_VIEW_CALLBACKS;
   glwCompSetBackColor(&_this->super,GLW_COLOR_BLACK);
   glwCompSetForeColor(&_this->super,GLW_COLOR_WHITE);
   glwCompSetCursor(&_this->super,GLUT_CURSOR_CROSSHAIR);
   glwCompSetLayout(&_this->super,&ds3_view_layout);
   _this->cm_axes->super.callbacks=&DS3_VIEW_AXES_CALLBACKS;
   _this->cm_box->super.callbacks=&DS3_VIEW_BOX_CALLBACKS;
   _this->cm_pts->super.callbacks=&DS3_VIEW_PTS_CALLBACKS;
# if defined(__DS3_ADD_BONDS__)
   _this->cm_bnds->super.callbacks=&DS3_VIEW_BNDS_CALLBACKS;
# endif
   _this->cm_iso->super.callbacks=&DS3_VIEW_ISO_CALLBACKS;
   _this->cm_slice->super.callbacks=&DS3_VIEW_SLICE_CALLBACKS;
   /*Win32 version of GLUT does not support these cursors!*/
   /*glwCompSetCursor(&_this->cm_axes->super,GLUT_CURSOR_CYCLE);*/
   /*glwCompSetCursor(&_this->cm_pts->super,GLUT_CURSOR_INFO);*/
   _this->data_changed_func=NULL;
   _this->data_changed_ctx=NULL;
   _this->slice_changed_func=NULL;
   _this->slice_changed_ctx=NULL;
   _this->ornt_changed_func=NULL;
   _this->ornt_changed_ctx=NULL;
   _this->zoom_changed_func=NULL;
   _this->zoom_changed_ctx=NULL;
   _this->cntr_changed_func=NULL;
   _this->cntr_changed_ctx=NULL;
   _this->box_changed_func=NULL;
   _this->box_changed_ctx=NULL;
   _this->point_changed_func=NULL;
   _this->point_changed_ctx=NULL;
# if defined(__DS3_ADD_BONDS__)
   _this->bond_changed_func=NULL;
   _this->bond_changed_ctx=NULL;
#  if defined(__DS3_SAVE_BONDS__)
   _this->bonds_changed_func=NULL;
   _this->bonds_changed_ctx=NULL;
#  endif
# endif
   ds3SliceInit(&_this->slice,NULL);
   ds3IsoInit(&_this->iso,NULL);
# if defined(__DS3_ADD_BONDS__)
   ds3BondsInit(&_this->bonds);
# endif
   _DAInit(&_this->view_stack,0,DS3ViewParams);
   _this->zoom=0;
   _this->yaw=1;
   _this->slice_t=1;
   _this->track_cb=0;
   _this->track_pl=5;
   _DAInit(&_this->draw_point,0,int);
   _this->draw_coords=1;
   _this->draw_points=1;
   _this->draw_slice=1;
   _this->draw_iso=1;
   _this->proj=DS3V_PROJECT_PERSPECTIVE;
   _this->ds=&DS_LINEAR_SCALE_IDENTITY.super;
   ds3ViewSetColorScale(_this,NULL);
   ds3ViewSetDataSet(_this,NULL);
   ds3ViewSetPointR(_this,0.03);
   ds3ViewSetSlice(_this,0,0,0);
   ds3ViewSetIso(_this,0.5,2);
   return 1;}
  glwCompDelAll(&_this->super);}
 glwCompFree(&_this->cm_axes->super);
 glwCompFree(&_this->cm_box->super);
 glwCompFree(&_this->cm_pts->super);
# if defined(__DS3_ADD_BONDS__)
 glwCompFree(&_this->cm_bnds->super);
# endif
 glwCompFree(&_this->cm_slice->super);
 glwCompFree(&_this->cm_iso->super);
 return 0;}

void ds3ViewDstr(DS3View *_this){
 glwCompDstr(&_this->super);}

void ds3ViewFree(DS3View *_this){
 glwCompFree(&_this->super);}


/*Gets a ray from the eyepoint through the projection plane at the given
  pixel coordinates. The ray is then transformed back into world
  coordinates. p0 contains the eyepoint, and p1 is on the forward
  extension of the ray. The two are always different.*/
void ds3ViewGetUnprojRay(DS3View *_this,int _x,int _y,Vect3d _p0,Vect3d _p1){
 Vect3d p0;
 Vect3d p1;
 int    i,j;
 if(_this->super.bounds.w>0&&_this->super.bounds.h>0){
  double aspect;
  double dx;
  double dy;
  aspect=_this->super.bounds.w*DS3V_ASPECT/_this->super.bounds.h;
  dx=2*(_x+0.5)/_this->super.bounds.w-1;
  dy=2*(_y+0.5)/_this->super.bounds.h-1;
  if(aspect>=1)dx*=aspect;
  else dy/=aspect;
  switch(_this->proj){
   case DS3V_PROJECT_ORTHOGRAPHIC:{
    dx*=_this->zoom;
    dy*=_this->zoom;
    vectSet3d(p0,dx,dy,_this->zoom);
    vectSet3d(p1,dx,dy,_this->zoom-1);}break;
   /*case DS3V_PROJECT_PERSEPCTIVE :*/
   default                       :{
    vectSet3d(p0,0,0,_this->zoom);
    vectSet3d(p1,dx,dy,_this->zoom-1);} } }
 else{
  vectSet3d(p0,0,0,_this->zoom);
  vectSet3d(p1,0,0,_this->zoom-1);}
 for(i=0;i<3;i++){
  _p1[i]=_p0[i]=0;
  for(j=0;j<3;j++){
   _p0[i]+=_this->rot[j][i]*p0[j];
   _p1[i]+=_this->rot[j][i]*p1[j];}
  _p0[i]+=_this->cntr[i];
  _p1[i]+=_this->cntr[i];} }

void ds3ViewSetDataChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->data_changed_func=_func;}

void ds3ViewSetDataChangedCtx(DS3View *_this,void *_ctx){
 _this->data_changed_ctx=_ctx;}

void ds3ViewSetSliceChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->slice_changed_func=_func;}

void ds3ViewSetSliceChangedCtx(DS3View *_this,void *_ctx){
 _this->slice_changed_ctx=_ctx;}

void ds3ViewSetOrientationChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->ornt_changed_func=_func;}

void ds3ViewSetOrientationChangedCtx(DS3View *_this,void *_ctx){
 _this->ornt_changed_ctx=_ctx;}

void ds3ViewSetZoomChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->zoom_changed_func=_func;}

void ds3ViewSetZoomChangedCtx(DS3View *_this,void *_ctx){
 _this->zoom_changed_ctx=_ctx;}

void ds3ViewSetCenterChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->cntr_changed_func=_func;}

void ds3ViewSetCenterChangedCtx(DS3View *_this,void *_ctx){
 _this->cntr_changed_ctx=_ctx;}

void ds3ViewSetBoxChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->box_changed_func=_func;}

void ds3ViewSetBoxChangedCtx(DS3View *_this,void *_ctx){
 _this->box_changed_ctx=_ctx;}

void ds3ViewSetPointChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->point_changed_func=_func;}

void ds3ViewSetPointChangedCtx(DS3View *_this,void *_ctx){
 _this->point_changed_ctx=_ctx;}

# if defined(__DS3_ADD_BONDS__)
void ds3ViewSetBondChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->bond_changed_func=_func;}

void ds3ViewSetBondChangedCtx(DS3View *_this,void *_ctx){
 _this->bond_changed_ctx=_ctx;}

#  if defined(__DS3_SAVE_BONDS__)
void ds3ViewSetBondsChangedFunc(DS3View *_this,GLWActionFunc _func){
 _this->bonds_changed_func=_func;}

void ds3ViewSetBondsChangedCtx(DS3View *_this,void *_ctx){
 _this->bonds_changed_ctx=_ctx;}
#  endif
# endif

void ds3ViewSetColorScale(DS3View *_this,const DSColorScale *_cs){
 _this->cs=_cs!=NULL?_cs:&DS_RAINBOW_SCALE;
 /*TODO: if we have textured palettes, only update the palette*/
 _this->c_valid=0;
 _this->t_valid=0;
 if(_this->draw_points||_this->draw_slice)glwCompRepaint(&_this->super,0);}

void ds3ViewSetDataScale(DS3View *_this,const DSDataScale *_ds){
 _this->ds=_ds!=NULL?_ds:&DS_LINEAR_SCALE_IDENTITY.super;
 _this->c_valid=0;
 _this->t_valid=0;
 if(_this->draw_slice)glwCompRepaint(&_this->super,0);}

int ds3ViewSetDataSet(DS3View *_this,DataSet3D *_ds3){
 int    i;
 int    j;
 /*Free existing slice texture*/
 ds3SliceDstr(&_this->slice,_this);
 _this->t_valid=0;
 _this->c_valid=0;
 /*Free existing iso-surface*/
 ds3IsoDstr(&_this->iso);
 _this->s_valid=0;
 /*Do the only things that can fail:*/
 if(_ds3!=NULL){
# if defined(__DS3_ADD_BONDS__)
  if(!ds3BondsReset(&_this->bonds,_ds3))return 0;
# endif
  if(!daSetSize(&_this->draw_point,_ds3->npoints))return 0;}
 /*Set up parameters for new data set*/
 _this->ds3=_ds3;
 /*Invert basis matrix (needed for correct iso-surface drawing order)*/
 if(_ds3==NULL)for(i=0;i<3;i++)for(j=0;j<3;j++)_this->basinv[i][j]=i==j;
 else dsMatrix3x3Inv(_ds3->basis,_this->basinv);
 if(_ds3!=NULL){
  size_t k;
  _this->offs=vectMag2_3d(_ds3->center);
  for(i=0;i<3;i++){
   Vect3d diff;
   double d;
   for(j=0;j<3;j++)diff[j]=_ds3->basis[j][i]-_ds3->center[j];
   d=vectMag2_3d(diff);
   if(d>_this->offs)_this->offs=d;}
  _this->offs=2*sqrt(_this->offs);
  ds3SliceInit(&_this->slice,_ds3->density);
  ds3IsoInit(&_this->iso,_ds3->density);
  for(i=0;i<3;i++){
   for(j=0;j<3;j++)_this->basis[(j<<2)+i]=_ds3->basis[i][j];
   _this->basis[(i<<2)+3]=0;
   _this->basis[(3<<2)+i]=0;}
  _this->basis[15]=1;
  i=1;
  for(k=0;k<_ds3->npoints;k++)_DASetAt(&_this->draw_point,k,&i,int);
  ds3ViewSetCenter(_this,_ds3->center[X],_ds3->center[Y],_ds3->center[Z]);}
 else{
  _this->offs=1;
  for(i=0;i<4;i++)for(j=0;j<4;j++)_this->basis[(i<<2)+j]=i==j;
  daSetSize(&_this->draw_point,0);
  ds3ViewSetCenter(_this,0.5,0.5,0.5);}
 ds3ViewSetZoom(_this,_this->offs);
 ds3ViewSetOrientation(_this,0,0,0);
 ds3ViewSetBox(_this,0,0,0,1,1,1);
 if(_this->track_dx>=0){
  _this->track_dx=-1;
  if(_this->data_changed_func!=NULL){
   _this->data_changed_func(_this->data_changed_ctx,&_this->super);} }
 _this->track_lx=-1;
 _this->track_lp=-1;
 ds3ViewSetSelectedPoint(_this,-1);
# if defined(__DS3_ADD_BONDS__)
 ds3ViewSetSelectedBond(_this,-1,-1);
# endif
 daSetSize(&_this->view_stack,0);
 glwCompSetFocusable(&_this->cm_axes->super,_ds3!=NULL);
 glwCompSetFocusable(&_this->cm_box->super,_ds3!=NULL);
 glwCompSetFocusable(&_this->cm_pts->super,_ds3!=NULL&&_ds3->npoints>0);
# if defined(__DS3_ADD_BONDS__)
 glwCompSetFocusable(&_this->cm_bnds->super,0);
# endif
 glwCompSetFocusable(&_this->cm_slice->super,_ds3!=NULL);
 glwCompRevalidate(&_this->super);
 return 1;}

void ds3ViewSetPointR(DS3View *_this,double _r){
 if(_r<0)_r=0;
 else if(_r>0.1*_this->offs)_r=0.1*_this->offs;
 if(_r!=_this->point_r){
  _this->point_r=_r;
  if(_this->draw_points)glwCompRepaint(&_this->super,0);} }

void ds3ViewSetSlice(DS3View *_this,double _t,double _p,double _d){
 double st;
 double ct;
 double sp;
 double cp;
 double a;
 double b;
 Vect3d n;
 int    i;
 /*Keep angles within 0 to 360*/
 if(_t<0||_t>360){
  _t=fmod(_t,360);
  if(_t<0)_t+=360;}
 if(_p<0||_p>360){
  _p=fmod(_p,360);
  if(_p<0)_p+=360;}
 /*Clamp plane offset to stay in cube*/
 st=sin(_t*M_PI/180);
 ct=cos(_t*M_PI/180);
 sp=sin(_p*M_PI/180);
 cp=cos(_p*M_PI/180);
 vectSet3d(n,st,-sp*ct,cp*ct);
 a=b=0;
 for(i=0;i<8;i++){
  Vect3d p;
  int    j;
  double d;
  for(j=0;j<3;j++)p[j]=_this->box[i&1<<j?1:0][j]-0.5;
  d=vectDot3d(n,p);
  if(d<a)a=d;
  if(d>b)b=d;}
 if(_d<a)_d=a;
 else if(_d>b)_d=b;
 if(_t!=_this->slice_t||_p!=_this->slice_p||_d!=_this->slice_d){
  _this->slice_t=_t;
  _this->slice_p=_p;
  _this->slice_d=_d;
# if defined(GL_EXT_texture3d)
#  if !defined(GL_VERSION_1_2)
  if(has_gl_ext_texture3d)_this->t_valid=0;
#  endif
# else
  _this->t_valid=0;
# endif
  _this->strans[X][X]=ct;
  _this->strans[X][Y]=0;
  _this->strans[X][Z]=n[X];
  _this->strans[X][W]=0.5+n[X]*_d;
  _this->strans[Y][X]=sp*st;
  _this->strans[Y][Y]=cp;
  _this->strans[Y][Z]=n[Y];
  _this->strans[Y][W]=0.5+n[Y]*_d;
  _this->strans[Z][X]=-cp*st;
  _this->strans[Z][Y]=sp;
  _this->strans[Z][Z]=n[Z];
  _this->strans[Z][W]=0.5+n[Z]*_d;
  if(_this->slice_changed_func!=NULL){
   _this->slice_changed_func(_this->slice_changed_ctx,&_this->super);}
  if(_this->draw_slice)glwCompRepaint(&_this->super,0);} }

void ds3ViewSetIso(DS3View *_this,double _v,int _d){
 if(_this->ds3!=NULL){
  if(_v>_this->ds3->max)_v=_this->ds3->max;
  if(_v<_this->ds3->min)_v=_this->ds3->min;}
 else{
  if(_v>1)_v=1;
  else if(_v<0)_v=0;}
 if(_d<1)_d=1;
 if(_v!=_this->iso_v||_d!=_this->iso_d){
  _this->iso_v=_v;
  _this->iso_d=_d;
  _this->s_valid=0;
  if(_this->draw_iso)glwCompRepaint(&_this->super,0);} }

void ds3ViewSetDrawCoordS(DS3View *_this,int _b){
 if(_this->draw_coords!=(_b?1U:0U)){
  _this->draw_coords=_b?1:0;
  glwCompVisibility(&_this->cm_axes->super,_b);
  glwCompVisibility(&_this->cm_box->super,_b);
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetDrawPoints(DS3View *_this,int _b){
 if(_this->draw_points!=(_b?1U:0U)){
  _this->draw_points=_b?1:0;
  glwCompVisibility(&_this->cm_pts->super,_b);
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetDrawSlice(DS3View *_this,int _b){
 if(_this->draw_slice!=(_b?1U:0U)){
  _this->draw_slice=_b?1:0;
  glwCompVisibility(&_this->cm_slice->super,_b);
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetDrawIso(DS3View *_this,int _b){
 if(_this->draw_iso!=(_b?1U:0U)){
  _this->draw_iso=_b?1:0;
  glwCompVisibility(&_this->cm_iso->super,_b);
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetOrientation(DS3View *_this,double _y,double _p,double _r){
 if(_y<0||_y>360){
  _y=fmod(_y,360);
  if(_y<0)_y+=360;}
 if(_p<0||_p>360){
  _p=fmod(_p,360);
  if(_p<0)_p+=360;}
 if(_r<0||_r>360){
  _r=fmod(_r,360);
  if(_r<0)_r+=360;}
 if(_this->yaw!=_y||_this->pitch!=_p||_this->roll!=_r){
  _this->yaw=_y;
  _this->pitch=_p;
  _this->roll=_r;
  ds3ViewExpandRot(_this->yaw,_this->pitch,_this->roll,_this->rot);
  if(_this->ornt_changed_func!=NULL){
   _this->ornt_changed_func(_this->ornt_changed_ctx,&_this->super);}
  glwCompRepaint(&_this->super,0);} }

void ds3ViewAlignOrientation(DS3View *_this){
 Vect3d m;
 Vect3d n;
 double d;
 double cx,sx,cy,sy;
 double x,y;
 int    i;
 int    j;
 m[X]=_this->strans[X][Z];
 m[Y]=_this->strans[Y][Z];
 m[Z]=_this->strans[Z][Z];
 for(i=0;i<3;i++){
  n[i]=0;
  for(j=0;j<3;j++)n[i]+=m[j]*_this->basinv[j][i];}
 d=vectMag2_3d(n);
 if(d<1E-100)vectSet3d(n,0,0,1);
 else vectMul3d(n,n,1/sqrt(d));
 sy=-n[X];
 y=asin(sy)*(180/M_PI);
 if(y<0){
  if(y<-1E-4)y+=360;
  else y=0;}
 if((fabs(_this->pitch-y)>90&&fabs(_this->pitch-y+360)>90&&
     fabs(_this->pitch-y-360)>90)){
  y=180-y;
  if(y<0){
   if(y<-1E-4)y+=360;
   else y=0;} }
 cy=cos(y*M_PI/180);
 if(fabs(cy)<1E-8)x=_this->yaw;
 else{
  sx=n[Y]/cy;
  cx=n[Z]/cy;
  x=atan2(sx,cx)*(180/M_PI);
  if(x<0){
   if(x<-1E-4)x+=360;
   else x=0;} }
 ds3ViewSetOrientation(_this,x,y,_this->roll);}

void ds3ViewSetProjectionType(DS3View *_this,int _t){
 if(_t!=_this->proj){
  _this->proj=_t;
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetZoom(DS3View *_this,double _zoom){
 if(_zoom<0)_zoom=0;
 if(_this->zoom!=_zoom){
  _this->zoom=_zoom;
  if(_this->zoom_changed_func!=NULL){
   _this->zoom_changed_func(_this->zoom_changed_ctx,&_this->super);}
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetCenter(DS3View *_this,double _x,double _y,double _z){
 if(_this->ds3!=NULL){
  if(_x<-2*_this->ds3->center[0])_x=-2*_this->ds3->center[0];
  else if(_x>4*_this->ds3->center[1])_x=4*_this->ds3->center[0];
  if(_y<-2*_this->ds3->center[1])_y=-2*_this->ds3->center[1];
  else if(_y>4*_this->ds3->center[1])_y=4*_this->ds3->center[1];
  if(_z<-2*_this->ds3->center[2])_z=-2*_this->ds3->center[2];
  else if(_z>4*_this->ds3->center[2])_z=4*_this->ds3->center[2];}
 else{
  if(_x<-1)_x=-1;
  else if(_x>2)_x=2;
  if(_y<-1)_y=-1;
  else if(_y>2)_y=2;
  if(_x<-1)_z=-1;
  else if(_z>2)_z=2;}
 if(_x!=_this->cntr[0]||_y!=_this->cntr[1]||_z!=_this->cntr[2]){
  vectSet3d(_this->cntr,_x,_y,_z);
  if(_this->cntr_changed_func!=NULL){
   _this->cntr_changed_func(_this->cntr_changed_ctx,&_this->super);}
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetBox(DS3View *_this,double _minx,double _miny,double _minz,
                   double _maxx,double _maxy,double _maxz){
 if(_minx<-1)_minx=-1;
 else if(_minx>2)_minx=2;
 if(_maxx<_minx)_maxx=_minx;
 else if(_maxx>2)_maxx=2;
 if(_miny<-1)_miny=-1;
 else if(_miny>2)_miny=2;
 if(_maxy<_miny)_maxy=_miny;
 else if(_maxy>2)_maxy=2;
 if(_minz<-1)_minz=-1;
 else if(_minz>2)_minz=2;
 if(_maxz<_minz)_maxz=_minz;
 else if(_maxz>2)_maxz=2;
 if(_minx!=_this->box[0][X]||_maxx!=_this->box[1][X]||
    _miny!=_this->box[0][Y]||_maxy!=_this->box[1][Y]||
    _minz!=_this->box[0][Z]||_maxz!=_this->box[1][Z]){
  vectSet3d(_this->box[0],_minx,_miny,_minz);
  vectSet3d(_this->box[1],_maxx,_maxy,_maxz);
  if(_this->box_changed_func!=NULL){
   _this->box_changed_func(_this->box_changed_ctx,&_this->super);}
  glwCompRepaint(&_this->super,0);} }

void ds3ViewSetPointVisible(DS3View *_this,long _pt,int _v){
 _v=_v?1:0;
 if(_pt>=0&&_this->ds3!=NULL&&(size_t)_pt<_this->ds3->npoints&&
    *_DAGetAt(&_this->draw_point,_pt,int)!=_v){
  _DASetAt(&_this->draw_point,_pt,&_v,int);
  if(_this->draw_points)glwCompRepaint(&_this->super,0);
  if(_this->track_sp==_pt&&_this->point_changed_func!=NULL){
   _this->point_changed_func(_this->point_changed_ctx,&_this->super);} } }

void ds3ViewSetSelectedPoint(DS3View *_this,long _pt){
 if(_this->ds3==NULL||_pt>=(long)_this->ds3->npoints)_pt=-1;
 if(_pt!=_this->track_sp){
  _this->track_sp=_pt;
  glwCompRepaint(&_this->super,0);
  if(_this->point_changed_func!=NULL){
   _this->point_changed_func(_this->point_changed_ctx,&_this->super);} } }

# if defined(__DS3_ADD_BONDS__)
void ds3ViewSetBond(DS3View *_this,long _from,long _to,double _sz){
 if(_from>=0&&_to>=0&&_from!=_to&&_this->ds3!=NULL){
  if(_from>_to){
   long t;
   t=_from;
   _from=_to;
   _to=t;}
  if((size_t)_from<_this->ds3->npoints-1&&(size_t)_to<_this->ds3->npoints&&
     ds3BondsGet(&_this->bonds,_from,_to)!=_sz){
   ds3BondsSet(&_this->bonds,_from,_to,_sz);
   glwCompSetFocusable(&_this->cm_bnds->super,_this->bonds.nbonds>0);
   glwCompRepaint(&_this->super,0);
#  if defined(__DS3_SAVE_BONDS__)
   if(_this->bonds_changed_func!=NULL){
    _this->bonds_changed_func(_this->bonds_changed_ctx,&_this->super);}
#  endif
   if(_this->track_sbf==_from&&_this->track_sbt==_to&&
      _this->bond_changed_func!=NULL){
    _this->bond_changed_func(_this->bond_changed_ctx,&_this->super);}
   if(_this->track_sbf<0)ds3ViewSetSelectedBond(_this,_from,_to);} } }

void ds3ViewDelBond(DS3View *_this,long _from,long _to){
 ds3ViewSetBond(_this,_from,_to,0);}

void ds3ViewSetSelectedBond(DS3View *_this,long _from,long _to){
 if(_this->ds3==NULL||_this->ds3->npoints<2)_from=_to=-1;
 else{
  if(_from>=0||_to<0||-1-_from>=_to||(size_t)_to>=_this->ds3->npoints){
   if(_from<0||_to<0||_from==_to)_from=_to=-1;
   else{
    if(_from>_to){
     long t;
     t=_from;
     _from=_to;
     _to=t;}
    if((size_t)_from>=_this->ds3->npoints-1||
       (size_t)_to>=_this->ds3->npoints||
       ds3BondsGet(&_this->bonds,_from,_to)<=0){
     _from=_to=-1;} } } }
 if(_from!=_this->track_sbf||_to!=_this->track_sbt){
  _this->track_sbf=_from;
  _this->track_sbt=_to;
  glwCompRepaint(&_this->super,0);
  if(_this->bond_changed_func!=NULL){
   _this->bond_changed_func(_this->bond_changed_ctx,&_this->super);} } }
# endif

int ds3ViewGetData(DS3View *_this,long *_x,long *_y,long *_z){
 if(_this->track_dx>=0){
  *_x=_this->track_dx;
  *_y=_this->track_dy;
  *_z=_this->track_dz;
  return 1;}
 return 0;}

int ds3ViewGetPointVisible(DS3View *_this,long _pt){
 if(_pt>=0&&_this->ds3!=NULL&&(size_t)_pt<_this->ds3->npoints){
  return *_DAGetAt(&_this->draw_point,_pt,int);}
 return 0;}
 
long ds3ViewGetSelectedPoint(DS3View *_this){
 return _this->track_sp;}

# if defined(__DS3_ADD_BONDS__)
double ds3ViewGetBond(DS3View *_this,long _from,long _to){
 if(_from>=0&&_to>=0&&_from!=_to&&_this->ds3!=NULL){
  if(_from>_to){
   long t;
   t=_from;
   _from=_to;
   _to=t;}
  if((size_t)_from<_this->ds3->npoints-1&&(size_t)_to<_this->ds3->npoints){
   return ds3BondsGet(&_this->bonds,_from,_to);} }
 return 0;}

int ds3ViewGetSelectedBond(DS3View *_this,long *_from,long *_to){
 if(_this->track_sbf<0||_this->track_sbt<0)return 0;
 else{
  *_from=_this->track_sbf;
  *_to=_this->track_sbt;
  return 1;} }
# endif

#endif                                                            /*_ds3view_C*/
