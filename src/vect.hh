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
#include <math.h>
#if !defined(_vect_H)
# define _vect_H (1)

# define X 0
# define Y 1
# define Z 2
# define W 3

# if defined(__SAVE_DIVS__)
#  define _VectDiv3(_this,_v,_c,_typ)                                         \
 _typ c=1/_typ;                                                               \
 _this[0]=_v[0]*_c;                                                           \
 _this[1]=_v[1]*_c;                                                           \
 _this[2]=_v[2]*_c;
# else
#  define _VectDiv3(_this,_v,_c,_typ)                                         \
 _this[0]=_v[0]/_c;                                                           \
 _this[1]=_v[1]/_c;                                                           \
 _this[2]=_v[2]/_c;
# endif

# define _DeclareTypedVect3(_typ,_sfx)                                        \
 typedef _typ Vect3##_sfx[3];                                                 \
       int   vectIsZero3##_sfx(const Vect3##_sfx _this);                      \
       void  vectSet3##_sfx(Vect3##_sfx _this,_typ _x,_typ _y,_typ _z);       \
       void  vectSet3##_sfx##v(Vect3##_sfx _this,const Vect3##_sfx _v);       \
       _typ  vectMag2_3##_sfx(const Vect3##_sfx _this);                       \
       _typ  vectMag3##_sfx(const Vect3##_sfx _this);                         \
       void  vectNeg3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v);          \
       void  vectMul3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,_typ _c);  \
       void  vectDiv3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,_typ _c);  \
       void  vectAdd3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _u,           \
                            const Vect3##_sfx _v);                            \
       void  vectAddS3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,          \
                             _typ _c);                                        \
       void  vectSub3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _u,           \
                            const Vect3##_sfx _v);                            \
       void  vectSubS3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,          \
                             _typ _c);                                        \
       void  vectNorm3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v);         \
       _typ  vectDot3##_sfx(const Vect3##_sfx _u,const Vect3##_sfx _v);       \
       _typ  vectMCross2_3##_sfx(const Vect3##_sfx _u,const Vect3##_sfx _v);  \
       _typ  vectMCross3##_sfx(const Vect3##_sfx _u,const Vect3##_sfx _v);    \
       void  vectCross3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _u,         \
                              const Vect3##_sfx _v);

# define _DefineTypedVect3(_typ,_sfx)                                         \
 int vectIsZero3##_sfx(const Vect3##_sfx _this){                              \
  return !_this[0]&&!_this[1]&&!_this[2];}                                    \
 void vectSet3##_sfx(Vect3##_sfx _this,_typ _x,_typ _y,_typ _z){              \
  _this[0]=_x;                                                                \
  _this[1]=_y;                                                                \
  _this[2]=_z;}                                                               \
 void vectSet3##_sfx##v(Vect3##_sfx _this,const Vect3##_sfx _v){              \
  _this[0]=_v[0];                                                             \
  _this[1]=_v[1];                                                             \
  _this[2]=_v[2];}                                                            \
 _typ vectMag2_3##_sfx(const Vect3##_sfx _this){                              \
  return vectDot3##_sfx(_this,_this);}                                        \
 _typ vectMag3##_sfx(const Vect3##_sfx _this){                                \
  return (_typ)sqrt(vectMag2_3##_sfx(_this));}                                \
 void vectNeg3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v){                 \
  _this[0]=-_v[0];                                                            \
  _this[1]=-_v[1];                                                            \
  _this[2]=-_v[2];}                                                           \
 void vectMul3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,_typ _c){         \
  _this[0]=_v[0]*_c;                                                          \
  _this[1]=_v[1]*_c;                                                          \
  _this[2]=_v[2]*_c;}                                                         \
 void vectDiv3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,_typ _c){         \
  _VectDiv3(_this,_v,_c,_typ);}                                               \
 void vectAdd3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _u,                  \
                     const Vect3##_sfx _v){                                   \
  _this[0]=_u[0]+_v[0];                                                       \
  _this[1]=_u[1]+_v[1];                                                       \
  _this[2]=_u[2]+_v[2];}                                                      \
 void vectAddS3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,_typ _c){        \
  _this[0]=_v[0]+_c;                                                          \
  _this[1]=_v[1]+_c;                                                          \
  _this[2]=_v[2]+_c;}                                                         \
 void vectSub3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _u,                  \
                     const Vect3##_sfx _v){                                   \
  _this[0]=_u[0]-_v[0];                                                       \
  _this[1]=_u[1]-_v[1];                                                       \
  _this[2]=_u[2]-_v[2];}                                                      \
 void vectSubS3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v,_typ _c){        \
  _this[0]=_v[0]-_c;                                                          \
  _this[1]=_v[1]-_c;                                                          \
  _this[2]=_v[2]-_c;}                                                         \
 void vectNorm3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _v){                \
  vectDiv3##_sfx(_this,_v,vectMag3##_sfx(_v));}                               \
 _typ vectDot3##_sfx(const Vect3##_sfx _u,const Vect3##_sfx _v){              \
  return _u[0]*_v[0]+_u[1]*_v[1]+_u[2]*_v[2];}                                \
 _typ vectMCross2_3##_sfx(const Vect3##_sfx _u,const Vect3##_sfx _v){         \
  Vect3##_sfx temp;                                                           \
  vectCross3##_sfx(temp,_u,_v);                                               \
  return vectMag2_3##_sfx(temp);}                                             \
 _typ vectMCross3##_sfx(const Vect3##_sfx _u,const Vect3##_sfx _v){           \
  return (_typ)sqrt(vectMCross2_3##_sfx(_u,_v));}                             \
 void vectCross3##_sfx(Vect3##_sfx _this,const Vect3##_sfx _u,                \
                       const Vect3##_sfx _v){                                 \
  _typ a,b;                                                                   \
  a=_u[1]*_v[2]-_u[2]*_v[1];                                                  \
  b=_u[2]*_v[0]-_u[0]*_v[2];                                                  \
  _this[2]=_u[0]*_v[1]-_u[1]*_v[0];                                           \
  _this[1]=b;                                                                 \
  _this[0]=a;}

# endif                                                              /*_vect_H*/