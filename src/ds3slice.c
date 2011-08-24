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
#include "ds3view.h"
#include "ds3slice.h"
#pragma hdrstop
#if !defined(_ds3slice_C)
# define _ds3slice_C (1)

/*NOTE: The 2D slice extraction is the only operation that prevents the
  data set from being repeated an arbitrary amount in each direction. If we
  could rely on 3D texturing being present, the only limitation would be
  framerate (which can get quite low with detailed iso-surfaces). All other
  code assumes the bounds of the clip box may be anything*/

/*This creates a palette for the data set using the current color scale. The
  palette is used by the color table 2D textures are constructed from, and
  by the 3D texture if our version of OpenGL supports paletted textures*/
static void ds3SlicePalette(DS3Slice *_this,DS3View *_view){
 int i;
 for(i=0;i<=UCHAR_MAX;i++){
  GLWcolor c;
  c=dsColorScale(_view->cs,i/(double)UCHAR_MAX);
  _this->ctable[i][0]=(GLubyte)(c&0xFF);
  _this->ctable[i][1]=(GLubyte)(c>>8&0xFF);
  _this->ctable[i][2]=(GLubyte)(c>>16&0xFF);
  _this->ctable[i][3]=(GLubyte)(c>>24&0xFF);} }

/*This creates a color table from the data set. The color table is a 3D array
  of indices into a 256 palette, one for each value in the data set. The data
  is mapped into integers between 0 and 255 inclusive using the current data
  scaling function*/
static int ds3SliceColor(DS3Slice *_this,DS3View *_view){
 if(!_view->c_valid){
  double        *data;
  unsigned char *cdata;
  size_t         size;
  size_t         i;
  size=_view->ds3->density[X]*_view->ds3->density[Y]*_view->ds3->density[Z];
  if(_this->cdata==NULL){
   cdata=(unsigned char *)malloc(size*sizeof(unsigned char));
   if(cdata==NULL)return 0;
   _this->cdata=cdata;}
  else cdata=_this->cdata;
  data=_view->ds3->data;
  for(i=0;i<size;i++){
   int c;
   c=(int)(dsScale(_view->ds,data[i])*(UCHAR_MAX+1));
   if(c>UCHAR_MAX)cdata[i]=UCHAR_MAX;
   else cdata[i]=(unsigned char)c;}
  ds3SlicePalette(_this,_view);
  _view->c_valid=1;}
 return 1;}

/*This routine extracts a 2D slice from the 3D texture (since we have to use
  OpenGL 1.1 which does not support 3D texturing), and applies it to a square
  passing through the data cube*/
static int ds3SliceMake(DS3Slice *_this,DS3View *_view){
 typedef GLubyte gl_ubyte_4[4];
 GLubyte       *txtr;
 gl_ubyte_4    *ctbl;
 unsigned char *cdat;
 double         d;
 int            x0i;
 unsigned       x0f;
 int            y0i;
 unsigned       y0f;
 int            z0i;
 unsigned       z0f;
 int            dixi;
 unsigned       dixf;
 int            diyi;
 unsigned       diyf;
 int            dizi;
 unsigned       dizf;
 int            djxi;
 unsigned       djxf;
 int            djyi;
 unsigned       djyf;
 int            djzi;
 unsigned       djzf;
 int            xi;
 unsigned       xf;
 int            yi;
 unsigned       yf;
 int            zi;
 unsigned       zf;
 long           xdim;
 long           ydim;
 long           zdim;
 long           zoff;
 long           toff;
 int            i;
 int            j;
 int            k;
 if(!ds3SliceColor(_this,_view))return 0;
 if(_this->txtr==NULL){
  _this->txtr=(GLubyte *)malloc(_this->t_sz*_this->t_sz*4*sizeof(GLubyte));
  if(_this->txtr==NULL)return 0;}
 xdim=_view->ds3->density[X];
 ydim=_view->ds3->density[Y];
 zdim=_view->ds3->density[Z];
 d=xdim*(_view->strans[X][W]-3*(_view->strans[X][X]+_view->strans[X][Y]));
 x0i=(int)d;
 if(x0i>d)x0i--;
 x0f=(unsigned)((d-x0i)*UINT_MAX);
 d=ydim*(_view->strans[Y][W]-3*(_view->strans[Y][X]+_view->strans[Y][Y]));
 y0i=(int)d;
 if(y0i>d)y0i--;
 y0f=(unsigned)((d-y0i)*UINT_MAX);
 d=zdim*(_view->strans[Z][W]-3*(_view->strans[Z][X]+_view->strans[Z][Y]));
 z0i=(int)d;
 if(z0i>d)z0i--;
 z0f=(unsigned)((d-z0i)*UINT_MAX);
 d=xdim*_view->strans[X][Y]*6/(_this->t_sz-1);
 dixi=(int)d;
 if(dixi>d)dixi--;
 dixf=(unsigned)((d-dixi)*UINT_MAX);
 d=ydim*_view->strans[Y][Y]*6/(_this->t_sz-1);
 diyi=(int)d;
 if(diyi>d)diyi--;
 diyf=(unsigned)((d-diyi)*UINT_MAX);
 d=zdim*_view->strans[Z][Y]*6/(_this->t_sz-1);
 dizi=(int)d;
 if(dizi>d)dizi--;
 dizf=(unsigned)((d-dizi)*UINT_MAX);
 d=xdim*_view->strans[X][X]*6/(_this->t_sz-1);
 djxi=(int)d;
 if(djxi>d)djxi--;
 djxf=(unsigned)((d-djxi)*UINT_MAX);
 d=ydim*_view->strans[Y][X]*6/(_this->t_sz-1);
 djyi=(int)d;
 if(djyi>d)djyi--;
 djyf=(unsigned)((d-djyi)*UINT_MAX);
 d=zdim*_view->strans[Z][X]*6/(_this->t_sz-1);
 djzi=(int)d;
 if(djzi>d)djzi--;
 djzf=(unsigned)((d-djzi)*UINT_MAX);
 zoff=xdim*ydim;
 toff=zoff*zdim;
 txtr=_this->txtr;
 ctbl=_this->ctable;
 cdat=_this->cdata;
 for(k=0,i=_this->t_sz;;){
  xi=x0i;
  xf=x0f;
  yi=y0i;
  yf=y0f;
  zi=z0i;
  zf=z0f;
  for(j=_this->t_sz;;){
   int  x;
   int  y;
   int  z;
   int  m;
   long l;
   long o[3][2];
   int  c[8];
   int  xm;
   int  ym;
   int  zm;
   /*Clamp the values in range, so mip-maps work properly*/
   /*x=xi<-1?-1:xi>=xdim?xdim-1:xi;
   y=yi<-1?-1:yi>=ydim?ydim-1:yi;
   z=zi<-1?-1:zi>=zdim?zdim-1:zi;
   o[0][0]=x>=0?0:1;
   o[0][1]=x<xdim-1?1:0;
   o[1][0]=y>=0?0:xdim;
   o[1][1]=y<ydim-1?xdim:0;
   o[2][0]=z>=0?0:zoff;
   o[2][1]=z<zdim-1?zoff:0;*/
   /*Wrap values around, so wrapping clip-box works properly*/
   if(xi<0){
    x=-xi%xdim;
    if(x)x=xdim-x;}
   else x=xi%xdim;
   if(yi<0){
    y=-yi%ydim;
    if(y)y=ydim-y;}
   else y=yi%ydim;
   if(zi<0){
    z=-zi%zdim;
    if(z)z=zdim-z;}
   else z=zi%zdim;
   o[0][0]=0;
   o[0][1]=x<xdim-1?1:1-xdim;
   o[1][0]=0;
   o[1][1]=y<ydim-1?xdim:xdim-zoff;
   o[2][0]=0;
   o[2][1]=z<zdim-1?zoff:zoff-toff;
   l=x+xdim*(y+ydim*z);
   for(m=0;m<8;m++)c[m]=cdat[l+o[0][m&1]+o[1][(m&2)>>1]+o[2][(m&4)>>2]];
   xm=(xf>>8)+1>>1;
   ym=(yf>>8)+1>>1;
   zm=(zf>>8)+1>>1;
   for(m=0;m<4;m++){
    int d[4];
# define DS3V_PREC_SHIFT (sizeof(unsigned)*CHAR_BIT-9)
    d[0]=ctbl[c[0]][m]+(zm*(ctbl[c[4]][m]-ctbl[c[0]][m])>>DS3V_PREC_SHIFT);
    d[1]=ctbl[c[1]][m]+(zm*(ctbl[c[5]][m]-ctbl[c[1]][m])>>DS3V_PREC_SHIFT);
    d[2]=ctbl[c[2]][m]+(zm*(ctbl[c[6]][m]-ctbl[c[2]][m])>>DS3V_PREC_SHIFT);
    d[3]=ctbl[c[3]][m]+(zm*(ctbl[c[7]][m]-ctbl[c[3]][m])>>DS3V_PREC_SHIFT);
    d[0]=d[0]+(ym*(d[2]-d[0])>>DS3V_PREC_SHIFT);
    d[1]=d[1]+(ym*(d[3]-d[1])>>DS3V_PREC_SHIFT);
    txtr[k++]=d[0]+(xm*(d[1]-d[0])>>DS3V_PREC_SHIFT);}
# undef DS3V_PREC_SHIFT
   if(--j<=0)break;
   xi+=djxi;
   xf+=djxf;
   if(xf<djxf)xi++;
   yi+=djyi;
   yf+=djyf;
   if(yf<djyf)yi++;
   zi+=djzi;
   zf+=djzf;
   if(zf<djzf)zi++;}
  if(--i<=0)break;
  x0i+=dixi;
  x0f+=dixf;
  if(x0f<dixf)x0i++;
  y0i+=diyi;
  y0f+=diyf;
  if(y0f<diyf)y0i++;
  z0i+=dizi;
  z0f+=dizf;
  if(z0f<dizf)z0i++;}
 if(!_this->t_id)glGenTextures(1,&_this->t_id);
 glBindTexture(GL_TEXTURE_2D,_this->t_id);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,_this->t_sz,_this->t_sz,
                   GL_RGBA,GL_UNSIGNED_BYTE,txtr);
 /*glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_this->t_sz,_this->t_sz,
                0,GL_RGBA,GL_UNSIGNED_BYTE,txtr);*/
 /*WORKAROUND: In MesaGL 3.0: glPushAttrib()/glPopAttrib() does not save the
   current texture's reference count, so if you change it, you'll screw it up.
   This causes problems in our display function, if we leave it set here, or
   in ds3SliceMakeFast(). Technically 0 may not have been the current texture,
   but it will be for us.*/
 glBindTexture(GL_TEXTURE_2D,0);
 _view->t_valid=1;
 if(_this->i_id){
  glwCompDelTimer(&_view->super,_this->i_id);
  _this->i_id=0;
  glwCompRepaint(&_view->super,0);}
 return 1;}

/*The above routine is somewhat slow, espcially since for average datasets we
  can be generating a meg of texture or more, so we use this version, which
  creates a texture at least one-fourth the size and without any sub-texeling
  or mip-maps, and schedule the above function to run when we're idle. That
  way, scrolling the slice around is not so slow, but we still get all the
  detail of the above function when we're done*/
static int ds3SliceMakeFast(DS3Slice *_this,DS3View *_view){
 typedef GLubyte gl_ubyte_4[4];
 GLubyte       *txtr;
 gl_ubyte_4    *ctbl;
 unsigned char *cdat;
 double         d;
 int            x0i;
 unsigned       x0f;
 int            y0i;
 unsigned       y0f;
 int            z0i;
 unsigned       z0f;
 int            dixi;
 unsigned       dixf;
 int            diyi;
 unsigned       diyf;
 int            dizi;
 unsigned       dizf;
 int            djxi;
 unsigned       djxf;
 int            djyi;
 unsigned       djyf;
 int            djzi;
 unsigned       djzf;
 int            xi;
 unsigned       xf;
 int            yi;
 unsigned       yf;
 int            zi;
 unsigned       zf;
 long           xdim;
 long           ydim;
 long           zdim;
 long           zoff;
 long           toff;
 int            i;
 int            j;
 int            k;
 int            scale;
 if(!ds3SliceColor(_this,_view))return 0;
 if(_this->txtr==NULL){
  _this->txtr=(GLubyte *)malloc(_this->t_sz*_this->t_sz*4*sizeof(GLubyte));
  if(_this->txtr==NULL)return 0;}
 d=sqrt(_view->super.bounds.w*_view->super.bounds.h)*_view->offs;
 if(_view->zoom>1E-16)d/=_view->zoom;
 for(scale=_this->t_sz>>1;scale>>1>d;scale>>=1);
 xdim=_view->ds3->density[X];
 ydim=_view->ds3->density[Y];
 zdim=_view->ds3->density[Z];
 d=xdim*(_view->strans[X][W]-3*(_view->strans[X][X]+_view->strans[X][Y]))+0.5;
 x0i=(int)d;
 if(x0i>d)x0i--;
 x0f=(unsigned)((d-x0i)*UINT_MAX);
 d=ydim*(_view->strans[Y][W]-3*(_view->strans[Y][X]+_view->strans[Y][Y]))+0.5;
 y0i=(int)d;
 if(y0i>d)y0i--;
 y0f=(unsigned)((d-y0i)*UINT_MAX);
 d=zdim*(_view->strans[Z][W]-3*(_view->strans[Z][X]+_view->strans[Z][Y]))+0.5;
 z0i=(int)d;
 if(z0i>d)z0i--;
 z0f=(unsigned)((d-z0i)*UINT_MAX);
 d=xdim*_view->strans[X][Y]*6/(scale-1);
 dixi=(int)d;
 if(dixi>d)dixi--;
 dixf=(unsigned)((d-dixi)*UINT_MAX);
 d=ydim*_view->strans[Y][Y]*6/(scale-1);
 diyi=(int)d;
 if(diyi>d)diyi--;
 diyf=(unsigned)((d-diyi)*UINT_MAX);
 d=zdim*_view->strans[Z][Y]*6/(scale-1);
 dizi=(int)d;
 if(dizi>d)dizi--;
 dizf=(unsigned)((d-dizi)*UINT_MAX);
 d=xdim*_view->strans[X][X]*6/(scale-1);
 djxi=(int)d;
 if(djxi>d)djxi--;
 djxf=(unsigned)((d-djxi)*UINT_MAX);
 d=ydim*_view->strans[Y][X]*6/(scale-1);
 djyi=(int)d;
 if(djyi>d)djyi--;
 djyf=(unsigned)((d-djyi)*UINT_MAX);
 d=zdim*_view->strans[Z][X]*6/(scale-1);
 djzi=(int)d;
 if(djzi>d)djzi--;
 djzf=(unsigned)((d-djzi)*UINT_MAX);
 zoff=xdim*ydim;
 toff=zoff*zdim;
 txtr=_this->txtr;
 ctbl=_this->ctable;
 cdat=_this->cdata;
 for(k=0,i=scale;;){
  xi=x0i;
  xf=x0f;
  yi=y0i;
  yf=y0f;
  zi=z0i;
  zf=z0f;
  for(j=scale;;){
   int      x;
   int      y;
   int      z;
   GLubyte *c;
   /*Wrap values around, so wrapping clip-box works properly*/
   if(xi<0){
    x=-xi%xdim;
    if(x)x=xdim-x;}
   else x=xi%xdim;
   if(yi<0){
    y=-yi%ydim;
    if(y)y=ydim-y;}
   else y=yi%ydim;
   if(zi<0){
    z=-zi%zdim;
    if(z)z=zdim-z;}
   else z=zi%zdim;
   c=ctbl[cdat[x+xdim*(y+ydim*z)]];
   txtr[k++]=c[0];
   txtr[k++]=c[1];
   txtr[k++]=c[2];
   txtr[k++]=c[3];
   if(--j<=0)break;
   xi+=djxi;
   xf+=djxf;
   if(xf<djxf)xi++;
   yi+=djyi;
   yf+=djyf;
   if(yf<djyf)yi++;
   zi+=djzi;
   zf+=djzf;
   if(zf<djzf)zi++;}
  if(--i<=0)break;
  x0i+=dixi;
  x0f+=dixf;
  if(x0f<dixf)x0i++;
  y0i+=diyi;
  y0f+=diyf;
  if(y0f<diyf)y0i++;
  z0i+=dizi;
  z0f+=dizf;
  if(z0f<dizf)z0i++;}
 if(!_this->t_id)glGenTextures(1,&_this->t_id);
 glBindTexture(GL_TEXTURE_2D,_this->t_id);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,scale,scale,
              0,GL_RGBA,GL_UNSIGNED_BYTE,txtr);
 /*gluBuild2DMipmaps(GL_TEXTURE_2D,4,scale,scale,
                   GL_RGBA,GL_UNSIGNED_BYTE,txtr);*/
 glBindTexture(GL_TEXTURE_2D,0);
 if(_this->i_id)glwCompDelTimer(&_view->super,_this->i_id);
 _this->i_id=glwCompAddTimer(&_view->super,
                             (GLWActionFunc)ds3SliceMake,_this,1000);
 _view->t_valid=1;
 return 1;}

/*Creates a 3D texture, if our version of OpenGL supports it (1.2 or later, or
  one that supports EXT_texture3d). Also, 256 colors is deemed to be enough,
  so paletted textures are used if supported (1.2 or later, or
  EXT_paletted_texture) to save on memory and processing time. This is much
  more advantageous than a 2D texture, since we do not have to create a 2D
  texture in software every time the slice moves (expensive, since the
  texture is nine times the size required to fill a unit box to account for
  repeating, and must be even larger to account for the power of two texture
  sizes required by OpenGL), and the 3D texturing may be hardware accelerated.*/
# if defined(GL_EXT_texture3d)
static int ds3SliceTexture3D(DS3Slice *_this,DS3View *_view){
 GLint m;
 glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE,&m);
 if(m>1)m>>=1;
 if(_view->ds3!=NULL){
  GLsizei   d[3];
  GLsizei   w[3];
  int       ws[3];
  int       i;
  GLsizei   j[3];
  GLsizei   k[3];
  GLint     o[3];
  size_t    l;
  size_t    n;
  GLint     lod;
  GLubyte  *txtr;
  double    x[3];
  double    xm[3];
  double    dx[3];
  double   *data;
  double    v[4];
  GLWcolor  c;
  for(i=0;i<3;i++){
   d[i]=_view->ds3->density[i];
   for(w[i]=1,ws[i]=0;w[i]<d[i]&&w[i]<(GLsizei)m;w[i]<<=1,ws[i]++);
   dx[i]=d[i]/(double)w[i];}
#  if defined(GL_EXT_paletted_texture)
  if(has_gl_ext_paletted_texture){
   txtr=(GLubyte *)malloc(sizeof(GLubyte)*w[X]*w[Y]*w[Z]);}
  else
#  endif
  txtr=(GLubyte *)malloc(4*sizeof(GLubyte)*w[X]*w[Y]*w[Z]);
  if(txtr==NULL)return 0;
  data=_view->ds3->data;
  if(!_this->t_id)glGenTextures(1,&_this->t_id);
  glBindTexture(GL_TEXTURE_3D,_this->t_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_REPEAT);
#  if defined(GL_EXT_paletted_texture)
  /*If we have paletted textures available, use those to reduce memory and
    CPU consumption to 1/4 of that for an RGBA texture*/
  if(has_gl_ext_paletted_texture){
   if(!_view->c_valid){
    ds3SlicePalette(_this,_view);
    _view->c_valid=1;}
   glColorTableEXT(GL_TEXTURE_3D,4,UCHAR_MAX+1,GL_RGBA,
                   GL_UNSIGNED_BYTE,_this->ctable);
   /*Create the full-sized texture*/
   for(n=0,j[Z]=0,x[Z]=0;j[Z]<w[Z];j[Z]++,x[Z]+=dx[Z]){
    k[Z]=(GLsizei)x[Z];
    o[Z]=(k[Z]+1>=d[Z]?-(GLint)k[Z]:1)*d[X]*d[Y];
    xm[Z]=x[Z]-k[Z];
    for(j[Y]=0,x[Y]=0;j[Y]<w[Y];j[Y]++,x[Y]+=dx[Y]){
     k[Y]=(GLsizei)x[Y];
     o[Y]=(k[Y]+1>=d[Y]?-(GLint)k[Y]:1)*d[X];
     xm[Y]=x[Y]-k[Y];
     for(j[X]=0,x[X]=0;j[X]<w[X];j[X]++,x[X]+=dx[X],n++){
      k[X]=(GLsizei)x[X];
      o[X]=k[X]+1>=d[X]?-(GLint)k[X]:1;
      xm[X]=x[X]-k[X];
      l=k[X]+d[X]*(k[Y]+d[Y]*k[Z]);
      v[0]=data[l]+xm[Z]*(data[l+o[Z]]-data[l]);
      v[1]=data[l+o[X]]+xm[Z]*(data[l+o[X]+o[Z]]-data[l+o[X]]);
      v[2]=data[l+o[Y]]+xm[Z]*(data[l+o[Y]+o[Z]]-data[l+o[Y]]);
      v[3]=data[l+o[X]+o[Y]]+xm[Z]*(data[l+o[X]+o[Y]+o[Z]]-data[l+o[X]+o[Y]]);
      v[0]+=xm[Y]*(v[2]-v[0]);
      v[1]+=xm[Y]*(v[3]-v[1]);
      c=(int)(dsScale(_view->ds,v[0]+xm[X]*(v[1]-v[0]))*256);
      if(c>255)txtr[n]=255;
      else txtr[n]=(GLubyte)c;} } }
   glTexImage3D(GL_TEXTURE_3D,0,GL_COLOR_INDEX8_EXT,w[X],w[Y],w[Z],
                0,GL_COLOR_INDEX,GL_UNSIGNED_BYTE,txtr);
   /*Create mip-maps*/
   for(lod=1;ws[X]||ws[Y]||ws[Z];lod++){
    o[X]=ws[X]?1:0;
    o[Y]=ws[Y]?w[X]:0;
    o[Z]=ws[Z]?w[X]<<ws[Y]:0;
    for(i=0;i<3;i++){
     if(ws[i]){
      ws[i]--;
      w[i]>>=1; 
      j[i]=1;}
     else j[i]=0;}
    for(k[Z]=0;k[Z]<w[Z];k[Z]++){
     for(k[Y]=0;k[Y]<w[Y];k[Y]++){
      for(k[X]=0;k[X]<w[X];k[X]++){
       int c;
       l=k[X]+(k[Y]+(k[Z]<<ws[Y]+j[Z])<<ws[X]+j[Y])<<j[X];
       c=txtr[l];
       c+=txtr[l+o[X]];
       c+=txtr[l+o[Y]];
       c+=txtr[l+o[Y]+o[X]];
       c+=txtr[l+o[Z]];
       c+=txtr[l+o[Z]+o[X]];
       c+=txtr[l+o[Z]+o[Y]];
       c+=txtr[l+o[Z]+o[Y]+o[X]];
       txtr[k[X]+(k[Y]+(k[Z]<<ws[Y])<<ws[X])]=(GLubyte)(c>>3);} } }
    glTexImage3D(GL_TEXTURE_3D,lod,GL_COLOR_INDEX8_EXT,w[X],w[Y],w[Z],
                 0,GL_COLOR_INDEX,GL_UNSIGNED_BYTE,txtr);} }
  else
#  endif
  /*Create the full-sized texture*/
  {for(n=0,j[Z]=0,x[Z]=0;j[Z]<w[Z];j[Z]++,x[Z]+=dx[Z]){
    k[Z]=(GLsizei)x[Z];
    o[Z]=(k[Z]+1>=d[Z]?-(GLint)k[Z]:1)*d[X]*d[Y];
    xm[Z]=x[Z]-k[Z];
    for(j[Y]=0,x[Y]=0;j[Y]<w[Y];j[Y]++,x[Y]+=dx[Y]){
     k[Y]=(GLsizei)x[Y];
     o[Y]=(k[Y]+1>=d[Y]?-(GLint)k[Y]:1)*d[X];
     xm[Y]=x[Y]-k[Y];
     for(j[X]=0,x[X]=0;j[X]<w[X];j[X]++,x[X]+=dx[X]){
      k[X]=(GLsizei)x[X];
      o[X]=k[X]+1>=d[X]?-(GLint)k[X]:1;
      xm[X]=x[X]-k[X];
      l=k[X]+d[X]*(k[Y]+d[Y]*k[Z]);
      v[0]=data[l]+xm[Z]*(data[l+o[Z]]-data[l]);
      v[1]=data[l+o[X]]+xm[Z]*(data[l+o[X]+o[Z]]-data[l+o[X]]);
      v[2]=data[l+o[Y]]+xm[Z]*(data[l+o[Y]+o[Z]]-data[l+o[Y]]);
      v[3]=data[l+o[X]+o[Y]]+xm[Z]*(data[l+o[X]+o[Y]+o[Z]]-data[l+o[X]+o[Y]]);
      v[0]+=xm[Y]*(v[2]-v[0]);
      v[1]+=xm[Y]*(v[3]-v[1]);
      c=dsColorScale(_view->cs,dsScale(_view->ds,v[0]+xm[X]*(v[1]-v[0])));
      txtr[n++]=(GLubyte)(c&0xFF);
      txtr[n++]=(GLubyte)(c>>8&0xFF);
      txtr[n++]=(GLubyte)(c>>16&0xFF);
      txtr[n++]=(GLubyte)(c>>24&0xFF);} } }
   glTexImage3D(GL_TEXTURE_3D,0,GL_RGBA,w[X],w[Y],w[Z],
                0,GL_RGBA,GL_UNSIGNED_BYTE,txtr);
   /*Create mip-maps*/
   for(lod=1;ws[X]||ws[Y]||ws[Z];lod++){
    o[X]=ws[X]?4:0;
    o[Y]=ws[Y]?w[X]<<2:0;
    o[Z]=ws[Z]?w[X]<<ws[Y]+2:0;
    for(i=0;i<3;i++){
     if(ws[i]){
      ws[i]--;
      w[i]>>=1; 
      j[i]=1;}
     else j[i]=0;}
    for(k[Z]=0;k[Z]<w[Z];k[Z]++){
     for(k[Y]=0;k[Y]<w[Y];k[Y]++){
      for(k[X]=0;k[X]<w[X];k[X]++){
       int c[4];
       l=k[X]+(k[Y]+(k[Z]<<ws[Y]+j[Z])<<ws[X]+j[Y])<<j[X]+2;
       for(i=0;i<4;i++){
        c[i]=txtr[l+i];
        c[i]+=txtr[l+o[X]];
        c[i]+=txtr[l+o[Y]+i];
        c[i]+=txtr[l+o[Y]+o[X]+i];
        c[i]+=txtr[l+o[Z]+i];
        c[i]+=txtr[l+o[Z]+o[X]+i];
        c[i]+=txtr[l+o[Z]+o[Y]+i];
        c[i]+=txtr[l+o[Z]+o[Y]+o[X]+i];}
       l=k[X]+(k[Y]+(k[Z]<<ws[Y])<<ws[X])<<2;
       for(i=0;i<4;i++)txtr[l+i]=(GLubyte)(c[i]>>3);} } }
    glTexImage3D(GL_TEXTURE_3D,lod,GL_RGBA,w[X],w[Y],w[Z],
                 0,GL_RGBA,GL_UNSIGNED_BYTE,txtr);} }
  glBindTexture(GL_TEXTURE_3D,0);
  free(txtr);}
 _view->t_valid=1;
 return 1;}
# endif

typedef struct DS3SliceVertex DS3SliceVertex;

struct DS3SliceVertex{
 Vect3d p;
 double tx;
 double ty;};

/*Clips the slice polygon against one of the planes of the bounding box
  slice:  A list of vertices in the slice polygon
  nverts: The number of vertices in the list
  plane:  The plane equation to clip against
  Return: The number of vertices in the new slice polygon*/
static int ds3ViewSliceClipPlane(DS3SliceVertex _slice[16],int _nverts,
                                 double _plane[4]){
 DS3SliceVertex slice[16];
 int            ret;
 int            i;
 int            j;
 double         d0;
 double         d1;
 ret=0;
 d0=vectDot3d(_slice[0].p,_plane)+_plane[W];
 for(i=0;i<_nverts;i++){
  j=i+1;
  if(j>=_nverts)j=0;
  d1=vectDot3d(_slice[j].p,_plane)+_plane[W];
  if(d0>=0)*(slice+ret++)=*(_slice+i);
  if(d0>0&&d1<0||d0<0&&d1>0){
   Vect3d dp;
   double t;
   vectSub3d(dp,_slice[j].p,_slice[i].p);
   t=(-_plane[W]-vectDot3d(_slice[i].p,_plane))/vectDot3d(dp,_plane);
   vectMul3d(slice[ret].p,dp,t);
   vectAdd3d(slice[ret].p,slice[ret].p,_slice[i].p);
   slice[ret].tx=_slice[i].tx+t*(_slice[j].tx-_slice[i].tx);
   slice[ret].ty=_slice[i].ty+t*(_slice[j].ty-_slice[i].ty);
   ret++;}
  d0=d1;}
 memcpy(_slice,slice,sizeof(DS3SliceVertex)*ret);
 return ret;}

/*Creates a slice polygon clipped against all the planes in the viewer's
  bounding box
  slice: A list of vertices to store the slice polygon in
  Return: The number of vertices in the slice polygon*/
static int ds3ViewSliceClip(DS3View *_this,DS3SliceVertex _slice[16]){
 double plane[4];
 int    nverts;
 int    i;
 int    j;
 for(i=0;i<4;i++){
  Vect3d p;
  vectSet3d(p,-3+6*(i>>1),-3+6*((i&1)^(i>>1)),0);
  for(j=0;j<3;j++){
   _slice[i].p[j]=vectDot3d(_this->strans[j],p)+_this->strans[j][W];}
  _slice[i].tx=i>>1;
  _slice[i].ty=(i&1)^(i>>1);}
 nverts=4;
 plane[Y]=plane[Z]=0;
 for(i=0;i<3;i++){
  plane[i]=1;
  plane[W]=-_this->box[0][i]+1E-4;
  nverts=ds3ViewSliceClipPlane(_slice,nverts,plane);
  plane[i]=-1;
  plane[W]=_this->box[1][i]+1E-4;
  nverts=ds3ViewSliceClipPlane(_slice,nverts,plane);
  plane[i]=0;}
 return nverts;}

/*Draws the slice*/
static void ds3ViewSlicePeerDisplay(DS3ViewComp *_this,
                                    const GLWCallbacks *_cb){
 DS3View       *view;
 view=_this->ds3view;
 /*If we can, use a 3D texture for the slice*/
# if defined(GL_EXT_texture3d)
#  if !defined(GL_VERSION_1_2)
 if(has_gl_ext_texture3d)
#  endif
 {if(view->t_valid||ds3SliceTexture3D(&view->slice,view)){
   DS3SliceVertex slice[16];
   int            nverts;
   int            i;
   nverts=ds3ViewSliceClip(view,slice);
   if(nverts){
    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
    glPushMatrix();
    glMultMatrixd(view->basis);
    if(glwCompIsFocused(&_this->super)){
     glLineWidth(2);
     glwColor(glwColorBlend(view->super.forec,DS3V_FOCUS_COLOR));
     glBegin(GL_LINE_LOOP);
     for(i=0;i<nverts;i++)glVertex3dv(slice[i].p);
     glEnd();
     glLineWidth(1);}
    glBindTexture(GL_TEXTURE_3D,view->slice.t_id);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
    glEnable(GL_TEXTURE_3D);
    glBegin(GL_POLYGON);
    for(i=0;i<nverts;i++){
     glTexCoord3dv(slice[i].p);
     glVertex3dv(slice[i].p);}
    glEnd();
    glBindTexture(GL_TEXTURE_3D,0);
    glPopMatrix();
    glPopAttrib();} } }
#  if !defined(GL_VERSION_1_2)
 else
#  endif
# endif
 /*If we can't use 3D texturing, fall back on extracting 2D textures (slow)*/
# if !defined(GL_EXT_texture3d)||!defined(GL_VERSION_1_2)
 {if(view->t_valid||ds3SliceMakeFast(&view->slice,view)){
   int i;
   glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
   glPushMatrix();
   glMultMatrixd(view->basis);
   glBindTexture(GL_TEXTURE_2D,view->slice.t_id);
   glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
   if(glwCompIsFocused(&_this->super)){
    DS3SliceVertex slice[16];
    int            nverts;
    nverts=ds3ViewSliceClip(view,slice);
    if(nverts){
     glLineWidth(2);
     glwColor(glwColorBlend(view->super.forec,DS3V_FOCUS_COLOR));
     glBegin(GL_LINE_LOOP);
     for(i=0;i<nverts;i++)glVertex3dv(slice[i].p);
     glEnd();
     glLineWidth(1);
     glEnable(GL_TEXTURE_2D);
     glBegin(GL_POLYGON);
     for(i=0;i<nverts;i++){
      glTexCoord2d(slice[i].tx,slice[i].ty);
      glVertex3dv(slice[i].p);}
     glEnd();} }
   else{
    for(i=0;i<6;i++)glEnable(GL_CLIP_PLANE0+i);
    glTranslated(0.5,0.5,0.5);
    glRotated(view->slice_p,1,0,0);
    glRotated(view->slice_t,0,1,0);
    glTranslated(0,0,view->slice_d);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2i(0,0);
    glVertex2i(-3,-3);
    glTexCoord2i(0,1);
    glVertex2i(-3,3);
    glTexCoord2i(1,1);
    glVertex2i(3,3);
    glTexCoord2i(1,0);
    glVertex2i(3,-3);
    glEnd();}
   glBindTexture(GL_TEXTURE_2D,0);
   glPopMatrix();
   glPopAttrib();} }
# endif
 }

/*Gets a unit ray from the center of slice rotation in the direction the mouse
  is pointing given the un-projected ray of the current mouse position
  track: The unit vector returned
  p0:    The origin of the unprojected ray
  p1:    The endpoint of the unprojected ray*/
static void ds3ViewGetSliceTrackCoords(DS3View *_this,Vect3d _track,
                                       const Vect3d _p0,const Vect3d _p1){
 Vect3d p0;
 Vect3d p1;
 Vect3d dp;
 double a,b,c,d,r;
 r=_this->track_rd;
 r=r>1E-16?1/r:1;
 vectSub3d(p0,_p0,_this->ds3->center);
 vectMul3d(p0,p0,r);
 vectSub3d(p1,_p1,_this->ds3->center);
 vectMul3d(p1,p1,r);
 vectSub3d(dp,p1,p0);
 a=vectMag2_3d(dp);
 b=vectDot3d(dp,p0);
 c=vectMag2_3d(p0)-1;
 d=b*b-a*c;
 if(d<0){
  switch(_this->proj){
   case DS3V_PROJECT_ORTHOGRAPHIC:{
    int i;
    b=-a*(c+1);
    c=0;
    for(i=0;i<3;i++)c+=p0[i]*dp[i];
    c*=c;
    d=b*b-4*a*c;
    if(d<0||fabs(c)<1E-100){
     vectSet3d(_track,0,0,1);
     return;}
    d=(-b+sqrt(d))/c;
    vectMul3d(p0,p0,d);
    b=vectDot3d(dp,p0);
    d=0;}break;
   /*case DS3V_PROJECT_PERSPECTIVE :*/
   default                       :{
    int i;
    int j;
    c+=1;
    b=-b-c;
    a=0;
    for(i=0;i<3;i++){
     static const int next[3]={1,2,0};
     j=next[i];
     a+=p1[i]*p1[i];
     d=p0[i]*p1[j]-p0[j]*p1[i];
     a-=d*d;}
    d=b*b-a*c;
    if(d<0||fabs(a)<1E-100){
     vectSet3d(_track,0,0,1);
     return;}
    d=(-b-sqrt(d))/a;
    vectMul3d(p1,p1,d);
    vectSub3d(dp,p1,p0);
    a=vectMag2_3d(dp);
    b=vectDot3d(dp,p0);
    d=0;} } }
 if(fabs(a)<1E-100)vectSet3d(_track,0,0,1);
 else{
  d=(-b+_this->track_rt*sqrt(d))/a;
  vectMul3d(_track,dp,d);
  vectAdd3d(_track,_track,p0);} }

/*Composes an additional rotation onto the given slice orientation, and sets
  the current slice orientation to it
  t:   The original slice theta
  p:   The original slice phi
  rot: The rotation to compose onto these angles*/
static void ds3ViewSlicePostComposeRot(DS3View *_this,double _t,
                                       double _p,double _rot[3][3]){
 double cp;
 double sp;
 double ct;
 double st;
 double r[3][3];
 double s[3][3];
 int    i,j,k;
 cp=cos(_p*(M_PI/180));
 sp=sin(_p*(M_PI/180));
 ct=cos(_t*(M_PI/180));
 st=sin(_t*(M_PI/180));
 r[X][X]=ct;
 r[X][Y]=0;
 r[X][Z]=st;
 r[Y][X]=sp*st;
 r[Y][Y]=cp;
 r[Y][Z]=-sp*ct;
 r[Z][X]=-cp*st;
 r[Z][Y]=sp;
 r[Z][Z]=cp*ct;
 for(i=0;i<3;i++)for(j=0;j<3;j++){
  s[i][j]=0;
  for(k=0;k<3;k++)s[i][j]+=_rot[i][k]*r[k][j];}
 st=s[X][Z];
 if(st<-1)st=1;
 else if(st>1)st=1;
 _t=asin(st)*(180/M_PI);
 if(_t<1E-8){
  if(_t<-1E-4)_t+=360;
  else _t=0;}
 if((fabs(_this->slice_t-_t)>90&&fabs(_this->slice_t-_t+360)>90&&
     fabs(_this->slice_t-_t-360)>90)){
  _t=180-_t;
  if(_t<1E-8){
   if(_t<-1E-4)_t+=360;
   else _t=0;} }
 ct=cos(_t*M_PI/180);
 if(fabs(ct)<1E-8){
  sp=s[Y][X]/st;
  cp=-s[Z][X]/st;}
 else{
  sp=-s[Y][Z]/ct;
  cp=s[Z][Z]/ct;}
 _p=atan2(sp,cp)*(180/M_PI);
 if(_p<1E-8){
  if(_p<-1E-4)_p+=360;
  else _p=0;}
 ds3ViewSetSlice(_this,_t,_p,_this->slice_d);}

/*Redraws the slice to reflect keyboard focus*/
static void ds3ViewSlicePeerFocus(DS3ViewComp *_this,GLWCallbacks *_cb,
                                  int _s){
 glwCompSuperFocus(&_this->super,_cb,_s);
 glwCompRepaint(&_this->super,0);}

/*Keyboard manipulation of the slice*/
static int ds3ViewSlicePeerSpecial(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                   int _k,int _x,int _y){
 DS3View *view;
 int      ret;
 view=_this->ds3view;
 ret=-1;
 switch(_k){
  case GLUT_KEY_LEFT     :{
   ds3ViewSetSlice(view,view->slice_t-5,view->slice_p,view->slice_d);}break;
  case GLUT_KEY_RIGHT    :{
   ds3ViewSetSlice(view,view->slice_t+5,view->slice_p,view->slice_d);}break;
  case GLUT_KEY_UP       :{
   ds3ViewSetSlice(view,view->slice_t,view->slice_p-5,view->slice_d);}break;
  case GLUT_KEY_DOWN     :{
   ds3ViewSetSlice(view,view->slice_t,view->slice_p+5,view->slice_d);}break;
  case GLUT_KEY_PAGE_DOWN:{
   ds3ViewSetSlice(view,view->slice_t,view->slice_p,view->slice_d-0.05);}break;
  case GLUT_KEY_PAGE_UP  :{
   ds3ViewSetSlice(view,view->slice_t,view->slice_p,view->slice_d+0.05);}break;
  case GLUT_KEY_HOME     :{
   ds3ViewSetSlice(view,0,0,0);}break;
  case GLUT_KEY_END      :{
   ds3ViewAlignOrientation(view);}break;
  default                :ret=0;}
 if(ret>=0)ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
 return ret;}
   
/*Mouse manipulation of the slice*/
static int ds3ViewSlicePeerMouse(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                 int _b,int _s,int _x,int _y){
 int ret;
 ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
 if(ret>=0&&_b==GLUT_LEFT_BUTTON&&_s){
  DS3View *view;
  Vect3d   p;
  Vect3d   q;
  view=_this->ds3view;
  vectSub3d(view->track_an,view->track_pt,view->ds3->center);
  view->track_rd=vectMag3d(view->track_an);
  if(view->track_rd<1E-16)vectSet3d(view->track_an,0,0,1);
  else vectMul3d(view->track_an,view->track_an,1/view->track_rd);
  view->track_rt=-1;
  ds3ViewGetSliceTrackCoords(view,p,view->track_p0,view->track_p1);
  vectSub3d(p,p,view->track_an);
  view->track_rt=1;
  ds3ViewGetSliceTrackCoords(view,q,view->track_p0,view->track_p1);
  vectSub3d(q,q,view->track_an);
  if(vectMag2_3d(p)<vectMag2_3d(q))view->track_rt=-1;
  view->track_p=view->slice_t;
  view->track_y=view->slice_p;
  return 1;}
 return ret;}

/*Dragging the slice orientation around*/
static int ds3ViewSlicePeerMotion(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                  int _x,int _y){
 int ret;
 ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
 if(ret>=0&&(_this->super.mouse_b&1<<GLUT_LEFT_BUTTON)){
  DS3View *view;
  Vect3d   p;
  Vect3d   axis;
  double   qs;
  view=_this->ds3view;
  ds3ViewGetUnprojRay(view,_x,_y,view->track_p0,view->track_p1);
  ds3ViewGetSliceTrackCoords(view,p,view->track_p0,view->track_p1);
  vectCross3d(axis,view->track_an,p);
  qs=vectMag3d(axis);
  if(qs<1E-8){
   ds3ViewSetSlice(view,view->track_p,view->track_y,view->slice_d);}
  else{
   double qc;
   double q[3][3];
   int    i,j;
   vectMul3d(axis,axis,1/qs);
   if(qs>1)qs=1;
   qc=sqrt(1-qs*qs);
   if(vectDot3d(view->track_an,p)<0)qc=-qc;
   for(i=0;i<3;i++){
    for(j=0;j<3;j++)q[i][j]=axis[i]*axis[j]*(1-qc);
    q[i][i]+=qc;}
   vectMul3d(axis,axis,qs);
   q[1][2]-=axis[0];
   q[2][1]+=axis[0];
   q[2][0]-=axis[1];
   q[0][2]+=axis[1];
   q[0][1]-=axis[2];
   q[1][0]+=axis[2];
   ds3ViewSlicePostComposeRot(view,view->track_p,view->track_y,q);}
  return 1;}
 return ret;}


const GLWCallbacks DS3_VIEW_SLICE_CALLBACKS={
 &GLW_COMPONENT_CALLBACKS,
 NULL,
 (GLWDisplayFunc)ds3ViewSlicePeerDisplay,
 NULL,
 NULL,
 NULL,
 (GLWFocusFunc)ds3ViewSlicePeerFocus,
 NULL,
 NULL,
 NULL,
 (GLWSpecialFunc)ds3ViewSlicePeerSpecial,
 (GLWMouseFunc)ds3ViewSlicePeerMouse,
 (GLWMotionFunc)ds3ViewSlicePeerMotion,
 NULL};

/*Initializes the slice data
  dens: The dimensions of the data set, or NULL if there is no data set*/
void ds3SliceInit(DS3Slice *_this,size_t _dens[3]){
 int    s;
 GLint  m;
 double d;
 if(_dens!=NULL){
  for(s=0,d=0;s<3;s++)d+=_dens[s]*_dens[s];
  d=sqrt(d);}
 else d=sqrt(3);
 d*=6;
 glGetIntegerv(GL_MAX_TEXTURE_SIZE,&m);
 m>>=1;
 if(m<=d)s=m;
 else for(s=8;s<d;s<<=1);
 _this->t_sz=s;
 _this->i_id=0;
 _this->t_id=0;
 _this->txtr=NULL;
 _this->cdata=NULL;}

/*Destroys the current slice, freeing any memory for cached data
  view: The view component to remove a callback timer from, if necessary*/
void ds3SliceDstr(DS3Slice *_this,DS3View *_view){
 if(_this->i_id){
  glwCompDelTimer(&_view->super,_this->i_id);
  _this->i_id=0;}
 if(_this->t_id)glDeleteTextures(1,&_this->t_id);
 _this->t_id=0;
 free(_this->txtr);
 _this->txtr=NULL;
 free(_this->cdata);
 _this->cdata=NULL;}

#endif                                                           /*_ds3slice_C*/