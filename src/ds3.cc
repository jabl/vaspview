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
#include "ds3viewr.h"
#pragma hdrstop
#if !defined(_ds3_C)
# define _ds3_C (1)

/*Initializes the data set to default values*/
void ds3Init(DataSet3D *_this){
 _this->name=NULL;
 _this->label[0]=_this->label[1]=_this->label[2]=_this->label[3]=NULL;
 _this->units[0]=_this->units[1]=_this->units[2]=_this->units[3]=NULL;
 vectSet3d(_this->basis[0],1,0,0);
 vectSet3d(_this->basis[1],0,1,0);
 vectSet3d(_this->basis[2],0,0,1);
 vectSet3d(_this->center,0.5,0.5,0.5);
 _this->npoints=0;
 _this->points=NULL;
 _this->density[0]=_this->density[1]=_this->density[2]=0;
 _this->data=NULL;
 _this->min=0;
 _this->max=1;}

/*Frees the memory used by the data set*/
void ds3Dstr(DataSet3D *_this){
 int i;
 free(_this->name);
 for(i=0;i<4;i++)free(_this->label[i]);
 for(i=0;i<4;i++)free(_this->units[i]);
 free(_this->points);
 free(_this->data);}



/*A color scale from black to white*/
static GLWcolor dsGrayScale(const DSColorScale *_this,double _c){
 GLWcolor ret;
 ret=(GLWcolor)(_c*0xFF);
 if(ret>0xFF)ret=0xFF;
 return 0xFF000000|ret<<16|ret<<8|ret;}

const DSColorScale DS_GRAY_SCALE={dsGrayScale};



/*
# define CSR_RLUM    (0.299)
# define CSR_GLUM    (0.587)
# define CSR_BLUM    (0.114)

static double csRainbowGetLuminance(double _hue){
 int    h;
 double flr;
 double red,grn,blu;
 h=(int)_hue;
 _hue*=(1/60.0);
 flr=_hue-h
 switch((h&Integer.MAX_VALUE)%6){
  case 0 :{red=1.0;grn=flr;blu=0.0;}break;
  case 1 :{red=1-flr;grn=1.0;blu=0;}break;
  case 2 :{red=0.0;grn=1.0;blu=flr;}break;
  case 3 :{red=0;grn=1-flr;blu=1.0;}break;
  case 4 :{red=flr;grn=0.0;blu=1.0;}break;
  case 5 :{red=1.0;grn=0;blu=1-flr;}break;
  default:{red=grn=blu=0.0;} }
 return red*CSR_RLUM+grn*CSR_GLUM+blu*CSR_BLUM;}

# define CSR_MIN_LUM (csRainbowGetLuminance(CSR_MIN_HUE))
# define CSR_MAX_LUM (csRainbowGetLuminance(CSR_MAX_HUE))
*/

/*A color scale with maximum brightness, minimum saturation, and varying hue,
  from blue to red*/
static GLWcolor dsRainbowScale(const DSColorScale *_this,double _c){
 double hue;
 int    h;
 double flr;
 double red,grn,blu;
 /*double lum,tgt_lum;*/
 hue=(DS_RAINBOW_MIN_HUE+(DS_RAINBOW_MAX_HUE-DS_RAINBOW_MIN_HUE)*_c)*(1.0/60);
 h=(int)floor(hue);
 flr=hue-h;
 switch((h&INT_MAX)%6){
  case 0 :{red=1.0;grn=flr;blu=0.0;}break;
  case 1 :{red=1-flr;grn=1.0;blu=0;}break;
  case 2 :{red=0.0;grn=1.0;blu=flr;}break;
  case 3 :{red=0;grn=1-flr;blu=1.0;}break;
  case 4 :{red=flr;grn=0.0;blu=1.0;}break;
  case 5 :{red=1.0;grn=0;blu=1-flr;}break;
  default:{red=grn=blu=0.0;} }
 /*
 lum=red*CSR_RLUM+grn*CSR_GLUM+blu*CSR_BLUM;
 tgt_lum=CSR_MIN_LUM+(CSR_MAX_LUM-CSR_MIN_LUM)*_pos;
 if(lum>tgt_lum){
  double r=Math.sqrt(tgt_lum/lum);
  red=red*r;
  grn=grn*r;
  blu=blu*r;}*/
 return 0xFF000000|((int)(blu*0xFF)<<16)|
        ((int)(grn*0xFF)<<8)|(int)(red*0xFF);}

const DSColorScale DS_RAINBOW_SCALE={dsRainbowScale};



/*A data scale that maps data linearly so that the minimum is 0 and the
  maximum is 1*/
static double dsLinearScale(const DSLinearScale *_this,double _data){
 return (_data+_this->offs)*_this->mul;}

static double dsLinearUnscale(const DSLinearScale *_this,double _v){
 return _v/_this->mul-_this->offs;}


void dsLinearScaleInit(DSLinearScale *_this,double _min,double _max){
 _this->super.scale=(DSScaleFunc)dsLinearScale;
 _this->super.unscale=(DSUnscaleFunc)dsLinearUnscale;
 _this->offs=-_min;
 _this->mul=fabs(_max-_min)>1E-100?1/(_max-_min):1;}


const DSLinearScale DS_LINEAR_SCALE_IDENTITY={
 (DSScaleFunc)dsLinearScale,(DSUnscaleFunc)dsLinearUnscale,1,0};



/*A data scale that maps data linearly so that the minimum is 1 and the
  maximum is e^2, and then takes half the log of that (so it is in the range
  0 to 1)*/
static double dsLogScale(const DSLogScale *_this,double _data){
 return 0.5*log(1+(_data+_this->offs)*_this->mul);}

static double dsLogUnscale(const DSLogScale *_this,double _v){
 return (exp(2*_v)-1)/_this->mul-_this->offs;}


void dsLogScaleInit(DSLogScale *_this,double _min,double _max){
 _this->super.scale=(DSScaleFunc)dsLogScale;
 _this->super.unscale=(DSUnscaleFunc)dsLogUnscale;
 _this->offs=-_min;
 _this->mul=(fabs(_max-_min)>1E-100?1/(_max-_min):1)*(M_E*M_E-1);}



/*Inverts a 3x3 matrix. Returns the pseudoinvers iff the matrix was not
  invertible*/
void dsMatrix3x3Inv(const double _m[3][3],double _i[3][3]){
 double det;
 _i[0][0]=_m[1][1]*_m[2][2]-_m[1][2]*_m[2][1];
 _i[1][0]=_m[1][2]*_m[2][0]-_m[1][0]*_m[2][2];
 _i[2][0]=_m[1][0]*_m[2][1]-_m[1][1]*_m[2][0];
 det=_m[0][0]*_i[0][0]+_m[0][1]*_i[1][0]+_m[0][2]*_i[2][0];
 if(fabs(det)<1E-100)dsMatrix3x3PInv(_m,_i);
 else{
  det=1/det;
  _i[0][0]*=det;
  _i[1][0]*=det;
  _i[2][0]*=det;
  _i[0][1]=det*(_m[2][1]*_m[0][2]-_m[2][2]*_m[0][1]);
  _i[1][1]=det*(_m[2][2]*_m[0][0]-_m[2][0]*_m[0][2]);
  _i[2][1]=det*(_m[2][0]*_m[0][1]-_m[2][1]*_m[0][0]);
  _i[0][2]=det*(_m[0][1]*_m[1][2]-_m[0][2]*_m[1][1]);
  _i[1][2]=det*(_m[0][2]*_m[1][0]-_m[0][0]*_m[1][2]);
  _i[2][2]=det*(_m[0][0]*_m[1][1]-_m[0][1]*_m[1][0]);} }

/*Calculates the inverse of a 1x1 or 2x2 matrix. If the matrix is too close to
  singular, the identity is returned*/
static void dsMatrix2x2HInv(const double _m[3][3],double _i[2][2],int _h){
 switch(_h){
  case 1:{
   if(fabs(_m[0][0])<1E-100)_i[0][0]=1;
   else _i[0][0]=1/_m[0][0];}break;
  case 2:{
   double d;
   d=_m[0][0]*_m[1][1]-_m[0][1]*_m[1][0];
   if(fabs(d)<1E-100){
    _i[0][0]=_i[1][1]=1;
    _i[0][1]=_i[1][0]=0;}
   else{
    _i[0][0]=_m[1][1]/d;
    _i[0][1]=-_m[0][1]/d;
    _i[1][0]=-_m[1][0]/d;
    _i[1][1]=_m[0][0]/d;}break;} } }

/*Pseudoinverts a 3x3 matrix.*/
void dsMatrix3x3PInv(const double _m[3][3],double _i[3][3]){
 double lu[3][3];
 double x[3][2];
 double y[3][2];
 double xtxi[2][2];
 double ytyi[2][2];
 double a[3][3];
 int    p[3]={0,1,2};
 int    h;
 int    i;
 int    j;
 int    k;
 /*First step: compute an LUP decomposition*/
 memcpy(lu[0],_m[0],sizeof(lu));
 for(h=k=0;k<3;k++){
  double v;
  int l;
  v=0;
  for(i=h;i<3;i++){
   double aluik;
   aluik=fabs(lu[i][k]);
   if(aluik>v){
    v=aluik;
    l=i;} }
  if(v>1E-8){
   int t;
   t=p[h];
   p[h]=p[l];
   p[l]=t;
   for(i=0;i<3;i++){
    double d;
    d=lu[h][i];
    lu[h][i]=lu[l][i];
    lu[l][i]=d;}
   for(i=h+1;i<3;i++){
    lu[i][k]/=lu[h][k];
    for(j=k+1;j<3;j++)lu[i][j]-=lu[i][k]*lu[h][j];}
   h++;} }
 /*At this point, if
               0,        j>i
    L[i][j] == 1,        j==i
               lu[i][j], j<i
   and
               lu[i][j], j>=i
    U[i][j] == 0,        j<i
   and
               0, j!=p[i]
    P[i][j] == 1, j==p[i]
            *           *
   then m==P LU, where P  means P conjugate-transpose.
   Furthermore, h is the rank of U (and m)*/
 switch(h){
  case 0:{
   for(i=0;i<3;i++)for(j=0;j<3;j++)_i[i][j]=0;
   return;}
  case 3:{                                              /*Should be impossible*/
   for(i=0;i<3;i++)for(j=0;j<3;j++)_i[i][j]=i==j;
   return;} }
 for(j=0;j<h;j++){
  for(i=0;i<j;i++)x[p[i]][j]=0;
  x[p[i++]][j]=1;
  for(;i<3;i++)x[p[i]][j]=lu[i][j];}
 for(i=0;i<h;i++){
  for(j=0;j<i;j++)y[j][i]=0;
  for(;j<3;j++)y[j][i]=lu[i][j];}
 /*At this point,
        *
   m==xy , where x and y are 3 by h matrices, a full-rank decomposition*/
 for(i=0;i<h;i++)for(j=0;j<h;j++){
  a[i][j]=0;
  for(k=0;k<3;k++)a[i][j]+=x[k][i]*x[k][j];}
 dsMatrix2x2HInv(a,xtxi,h);
 /*              *
   xtxi is now (x x)^-1*/
 for(i=0;i<h;i++)for(j=0;j<h;j++){
  a[i][j]=0;
  for(k=0;k<3;k++)a[i][j]+=y[k][i]*y[k][j];}
 dsMatrix2x2HInv(a,ytyi,h);
 /*              *
   ytyi is now (y y)^-1*/
 for(i=0;i<3;i++)for(j=0;j<h;j++){
  _i[i][j]=0;
  for(k=0;k<h;k++)_i[i][j]+=y[i][k]*ytyi[k][j];}
 for(i=0;i<3;i++)for(j=0;j<h;j++){
  a[i][j]=0;
  for(k=0;k<h;k++)a[i][j]+=_i[i][j]*xtxi[k][j];}
 for(i=0;i<3;i++)for(j=0;j<3;j++){
  _i[i][j]=0;
  for(k=0;k<h;k++)_i[i][j]+=a[i][k]*x[j][k];} }
 /*            *        *
   i is now y(y y)^-1 (x x)^-1 x, the pseudoinverse*/



/*Win32 systems allow multiple OpenGL implementations to simultaneously
  co-exist, which may or may not support different extensions in different
  ways. Hence a function pointer has to be dynamically retrieved for any
  function defined by an extension, for each rendering context. We use only
  one rendering context, but more importantly one pixel format (function
  pointers are guaranteed to be the same for the same pixel format), so we
  are safe loading these once, after creating a context. BTW: this seems
  really stupid to me, since all the other OpenGL functions can be implemeted
  differently for different pixel formats already, I fail to see what is
  preventing extensions functions from being added in the exact same way. Just
  more Microsoft stupidity, if you ask me. Mesa just exports the damn
  functions from the .dll (though it would be nice for compatibility reasons
  if they also supported wglGetProcAddress()). Sigh, only under Windows...*/
# if defined(GL_EXT_texture3d)&&!defined(GL_VERSION_1_2)
int                        has_gl_ext_texture3d;
#  if defined(_WIN32)&&!defined(MESA)
glTexImage3DEXTFunc        gl_tex_image3d_ext;
glTexSubImage3DEXTFunc     gl_tex_sub_image3d_ext;
glCopyTexSubImage3DEXTFunc gl_copy_tex_sub_image3d_ext;
#  endif
# endif
# if defined(GL_EXT_paletted_texture)
int                               has_gl_ext_paletted_texture;
#  if defined(_WIN32)&&!defined(MESA)
/*
glColorTableEXTFunc               gl_color_table_ext;
glColorSubTableEXTFunc            gl_color_sub_table_ext;
glGetColorTableEXTFunc            gl_get_color_table_ext;
glGetColorTableParameterfvEXTFunc gl_get_color_table_parameterfv_ext;
glGetColorTableParameterivEXTFunc gl_get_color_table_parameteriv_ext;
*/
PFNGLCOLORTABLEEXTPROC               gl_color_table_ext;
PFNGLCOLORSUBTABLEEXTPROC            gl_color_sub_table_ext;
PFNGLGETCOLORTABLEEXTPROC            gl_get_color_table_ext;
PFNGLGETCOLORTABLEPARAMETERIVEXTPROC gl_get_color_table_parameterfv_ext;
PFNGLGETCOLORTABLEPARAMETERFVEXTPROC gl_get_color_table_parameteriv_ext;
#  endif
# endif


int main(int _argc,char **_argv){
 DS3Viewer ds3v;
 glwInit(&_argc,_argv);
 if(ds3ViewerInit(&ds3v)){
  /*At this point, we've created a window, and so should have a current
    rendering context: test for GL extensions*/
# if defined(GL_EXT_texture3d)&&!defined(GL_VERSION_1_2)
  has_gl_ext_texture3d=glutExtensionSupported("GL_EXT_texture_3d");
#  if defined(_WIN32)&&!defined(MESA)
  if(has_gl_ext_texture3d){
   gl_tex_image3d_ext=(PFNGLTEXIMAGE3DEXTPROC)
   wglGetProcAddress("glTexImage3DEXT");
   gl_tex_sub_image3d_ext=(PFNGLTEXSUBIMAGE3DEXTPROC)
   wglGetProcAddress("glTexSubImage3DEXT");
   gl_copy_tex_sub_image3d_ext=(PFNGLCOPYTEXSUBIMAGE3DEXTPROC)
   wglGetProcAddress("glCopyTexSubImage3DEXT");
   if(gl_tex_image3d_ext==NULL||gl_tex_sub_image3d_ext==NULL||
      gl_copy_tex_sub_image3d_ext==NULL){
    has_gl_ext_texture3d=0;} }
#  endif
# endif
# if defined(GL_EXT_paletted_texture)
  has_gl_ext_paletted_texture=
  glutExtensionSupported("GL_EXT_paletted_texture");
#  if defined(_WIN32)&&!defined(MESA)
  if(has_gl_ext_paletted_texture){
   gl_color_table_ext=(PFNGLCOLORTABLEEXTPROC)wglGetProcAddress("glColorTableEXT");
   gl_color_sub_table_ext=(PFNGLCOLORSUBTABLEEXTPROC)
   wglGetProcAddress("glColorSubTableEXT");
   gl_get_color_table_ext=(PFNGLGETCOLORTABLEEXTPROC)
   wglGetProcAddress("glGetColorTableEXT");
   gl_get_color_table_parameterfv_ext=(PFNGLGETCOLORTABLEPARAMETERIVEXTPROC)
   wglGetProcAddress("glGetColorTableParameterfvEXT");
   gl_get_color_table_parameteriv_ext=(PFNGLGETCOLORTABLEPARAMETERFVEXTPROC)
   wglGetProcAddress("glGetColorTableParameterivEXT");
   if(gl_color_table_ext==NULL||gl_color_sub_table_ext==NULL||
      gl_get_color_table_ext==NULL||gl_get_color_table_parameterfv_ext==NULL||
      gl_get_color_table_parameteriv_ext==NULL){
    has_gl_ext_paletted_texture=0;} }
#  endif
# endif
  if(_argc>1)ds3ViewerOpenFile(&ds3v,_argv[1]);
  glutMainLoop();}
 else fprintf(stderr,"Could not initialize UI.\n");
 return 1;}

#endif                                                                /*_ds3_C*/