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

#ifndef DS3_HH
#define DS3_HH

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glw.hh"
#include "vect.hh"

# if !defined(M_PI)
#  define M_PI (3.141592653589793238462643)
# endif

# if !defined(M_E)
#  define M_E  (2.71828182845904523536)
# endif

# if !defined(GL_VERSION_1_1)
#  error "OpenGL version 1.1 or later is required"
# endif

# if GLUT_API_VERSION<3
#  error "GLUT version 3 or later is required"
# endif

#if defined(GL_EXT_paletted_texture)
extern int has_gl_ext_paletted_texture;
#if defined(_WIN32)&&!defined(MESA)
extern PFNGLCOLORTABLEEXTPROC               gl_color_table_ext;
extern PFNGLCOLORSUBTABLEEXTPROC            gl_color_sub_table_ext;
extern PFNGLGETCOLORTABLEEXTPROC            gl_get_color_table_ext;
extern PFNGLGETCOLORTABLEPARAMETERIVEXTPROC gl_get_color_table_parameterfv_ext;
extern PFNGLGETCOLORTABLEPARAMETERFVEXTPROC gl_get_color_table_parameteriv_ext;
#define glColorTableEXT               (*gl_color_table_ext)
#   define glColorSubTableEXT            (*gl_color_sub_table_ext)
#   define glGetColorTableEXT            (*gl_get_color_table_ext)
#   define glGetColorTableParameterfvEXT (*gl_get_color_table_parameterfv_ext)
#   define glGetColorTableParameterivEXT (*gl_get_color_table_parameteriv_ext)
#  endif
# endif

/*Some systems claim to support 3D textures, but I cannot get them to work
   (even the Mesa sample program stex3d does not work correctly on my Linux
   laptop, for example, and I've reports of failures on SGI machines as well).
  Until I can figure out what I'm doing wrong, __DS3_NO_3D_TEXTURES__ can be
   defined to prevent the use of this feature.
  It really is a much better way of doing things, though, so you should leave
   it enabled unless you have problems (e.g., your texture is always just a
   solid color).*/
# if !defined(__DS3_NO_3D_TEXTURES__)
#  if defined(GL_VERSION_1_2)         /*3D texturing is standard in OpenGL 1.2*/
#   if !defined(GL_EXT_texture3d)
#    define GL_EXT_texture3d (1)               /*Pretend we have the extension*/
#   endif
#  elif defined(GL_EXT_texture3d) /*If we have the extension, define 1.2 names*/
extern int has_gl_ext_texture3d;
#   if !defined(GL_PACK_SKIP_IMAGES)
#    define GL_PACK_SKIP_IMAGES    GL_PACK_SKIP_IMAGES_EXT
#   endif
#   if !defined(GL_PACK_IMAGE_HEIGHT)
#    define GL_PACK_IMAGE_HEIGHT   GL_PACK_IMAGE_HEIGHT_EXT
#   endif
#   if !defined(GL_UNPACK_SKIP_IMAGES)
#    define GL_UNPACK_SKIP_IMAGES  GL_UNPACK_SKIP_IMAGES_EXT
#   endif
#   if !defined(GL_UNPACK_IMAGE_HEIGHT)
#    define GL_UNPACK_IMAGE_HEIGHT GL_UNPACK_IMAGE_HEIGHT_EXT
#   endif
#   if !defined(GL_TEXTURE_3D)
#    define GL_TEXTURE_3D          GL_TEXTURE_3D_EXT
#   endif
#   if !defined(GL_PROXY_TEXTURE_3D)
#    define GL_PROXY_TEXTURE_3D    GL_PROXY_TEXTURE_3D_EXT
#   endif
#   if !defined(GL_TEXTURE_DEPTH)
#    define GL_TEXTURE_DEPTH       GL_TEXTURE_DEPTH_EXT
#   endif
#   if !defined(GL_TEXTURE_WRAP_R)
#    define GL_TEXTURE_WRAP_R      GL_TEXTURE_WRAP_R_EXT
#   endif
#   if !defined(GL_MAX_3D_TEXTURE_SIZE)
#    define GL_MAX_3D_TEXTURE_SIZE GL_MAX_3D_TEXTURE_SIZE_EXT
#   endif
#   if !defined(GL_TEXTURE_3D_BINDING)
#    define GL_TEXTURE_3D_BINDING  GL_TEXTURE_3D_BINDING_EXT
#   endif
#   if !defined(glTexImage3D)
#    define glTexImage3D           glTexImage3DEXT
#   endif
#   if !defined(glTexSubImage3D)
#    define glTexSubImage3D        glTexSubImage3DEXT
#   endif
#   if !defined(glCopyTexSubImage3D)
#    define glCopyTexSubImage3D    glCopyTexSubImage3DEXT
#   endif
#   if defined(_WIN32)&&!defined(MESA)
extern PFNGLTEXIMAGE3DEXTPROC        gl_tex_image3d_ext;
extern PFNGLTEXSUBIMAGE3DEXTPROC     gl_tex_sub_image3d_ext;
extern PFNGLCOPYTEXSUBIMAGE3DEXTPROC gl_copy_tex_sub_image3d_ext;
#    define glTexImage3D        (*gl_tex_image3d_ext)
#    define glTexSubImage3D     (*gl_tex_sub_image3d_ext)
#    define glCopyTexSubImage3D (*gl_copy_tex_image3d_ext)
#   endif
#  endif
# endif

/*Undefine this macro to disable the ability to add bonds between atoms*/
# define __DS3_ADD_BONDS__ (1)
/*Undefine this macro to disable saving bond information between sessions*/
# define __DS3_SAVE_BONDS__ (1)

_DeclareTypedVect3(double,d)
typedef struct DSPoint3D  DSPoint3D;
typedef struct DataSet3D  DataSet3D;

typedef struct DSColorScale  DSColorScale;
typedef struct DSDataScale   DSDataScale;
typedef struct DSLinearScale DSLinearScale;
typedef struct DSLogScale    DSLogScale;

typedef int      (*DS3ReadFunc)(void *_this);
typedef GLWcolor (*DSColorScaleFunc)(const DSColorScale *_this,double _c);
typedef double   (*DSScaleFunc)(const DSDataScale *_this,double _data);
typedef double   (*DSUnscaleFunc)(const DSDataScale *_this,double _v);



struct DSPoint3D
{
    Vect3d   pos;
    int      typ;
    double   col;
};



# define _DS3Index(_this,_x,_y,_z)                                            \
 ((_x)+(_this)->density[0]*((_y)+(_this)->density[1]*(_z)))

struct DataSet3D
{
    char           *name;                                  /*Name of the data set*/
    char           *label[4];                      /*Label of three axes and data*/
    char           *units[4];                     /*Units for three axes and data*/
    Vect3d          basis[3];                         /*Basis vectors for lattice*/
    Vect3d          center;                               /*Center of the lattice*/
    size_t          npoints;                                   /*Number of points*/
    DSPoint3D      *points;                                     /*Discrete points*/
    /*DSLine3D       *lines;*/                                 /*Lines between points*/
    size_t          density[3];       /*Dimensions of packed array of data values*/
    double         *data;                               /*3D packed array of data*/
    double          min;                                     /*Minimum data value*/
    double          max;
};                                   /*Maximum data value*/


void ds3Init(DataSet3D *_this);
void ds3Dstr(DataSet3D *_this);



struct DSColorScale
{
    DSColorScaleFunc scale;
};


# define dsColorScale(_this,_v)                                               \
 (((const DSColorScale *)(_this))->scale((const DSColorScale *)(_this),(_v)))


extern const DSColorScale DS_GRAY_SCALE;



# define DS_RAINBOW_MIN_HUE (240)
# define DS_RAINBOW_MAX_HUE (0)

extern const DSColorScale DS_RAINBOW_SCALE;



struct DSDataScale
{
    DSScaleFunc   scale;
    DSUnscaleFunc unscale;
};


# define dsScale(_this,_data)                                                 \
 (((const DSDataScale *)(_this))->scale((const DSDataScale *)(_this),(_data)))
# define dsUnscale(_this,_data)                                               \
 (((const DSDataScale *)(_this))->unscale((const DSDataScale *)(_this),       \
                                          (_data)))



struct DSLinearScale
{
    DSDataScale super;
    double      mul;
    double      offs;
};


void dsLinearScaleInit(DSLinearScale *_this,double _min,double _max);


extern const DSLinearScale DS_LINEAR_SCALE_IDENTITY;



struct DSLogScale
{
    DSDataScale super;
    double      mul;
    double      offs;
};


void dsLogScaleInit(DSLogScale *_this,double _min,double _max);



/*Inverts a 3x3 matrix. Returns the pseudoinverse iff the matrix was not
  invertible*/
void dsMatrix3x3Inv(const double _m[3][3],double _i[3][3]);

/*Pseudoinverts a 3x3 matrix.*/
void dsMatrix3x3PInv(const double _m[3][3],double _i[3][3]);

#endif                                                                /*_ds3_H*/
