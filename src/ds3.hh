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
#include <GL/glew.h>
#include <GL/glut.h>
#include "glw.hh"
#include "vect.hh"
#include <string>
#include <vector>

# if !defined(M_PI)
#  define M_PI (3.141592653589793238462643)
# endif

# if !defined(M_E)
#  define M_E  (2.71828182845904523536)
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

class DataSet3D
{
public:
	DataSet3D();
	~DataSet3D();

	// C++11 version of making private copy constructor and
	// assignment operator.
	DataSet3D& operator=(const DataSet3D&) = delete;
	DataSet3D(const DataSet3D&) = delete;

	std::string name;                  /*Name of the data set*/
    char           *label[4];                      /*Label of three axes and data*/
    char           *units[4];                     /*Units for three axes and data*/
    Vect3d          basis[3];                         /*Basis vectors for lattice*/
    Vect3d          center;                               /*Center of the lattice*/
    size_t          npoints;                                   /*Number of points*/
	std::vector<DSPoint3D> points;     /*Discrete points*/
    /*DSLine3D       *lines;*/                                 /*Lines between points*/
    size_t          density[3];       /*Dimensions of packed array of data values*/
	std::vector<double> data;          /*3D packed array of data*/
    double          min;                                     /*Minimum data value*/
    double          max;
};                                   /*Maximum data value*/


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
