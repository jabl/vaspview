/*VASP Data Viewer - Views 3d data sets of molecular charge distribution
  Copyright (C) 1999-2001 Timothy B. Terriberry
  (mailto:tterribe@users.sourceforge.net)
  2011 Janne Blomqvist

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
#include "ds3viewr.hh"
#include <cassert>

int limit_texture3D_mipmap_level = INT_MAX;
bool use_vbo;

/*Initializes the data set to default values*/
DataSet3D::DataSet3D()
{
    vectSet3d(this->basis[0],1,0,0);
    vectSet3d(this->basis[1],0,1,0);
    vectSet3d(this->basis[2],0,0,1);
    vectSet3d(this->center,0.5,0.5,0.5);
    this->npoints=0;
    this->density[0]=this->density[1]=this->density[2]=0;
    this->min=0;
    this->max=1;
}


/*A color scale from black to white*/
static GLWcolor dsGrayScale(const DSColorScale *_this,double _c)
{
    GLWcolor ret;
    ret=(GLWcolor)(_c*0xFF);
    if (ret>0xFF)ret=0xFF;
    return 0xFF000000|ret<<16|ret<<8|ret;
}

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
static GLWcolor dsRainbowScale(const DSColorScale *_this,double _c)
{
    double hue;
    int    h;
    double flr;
    double red,grn,blu;
    /*double lum,tgt_lum;*/
    hue=(DS_RAINBOW_MIN_HUE+(DS_RAINBOW_MAX_HUE-DS_RAINBOW_MIN_HUE)*_c)*(1.0/60);
    assert(hue >= 0. && hue <= INT_MAX);
    // Should be flr = hue - floor(hue), and h = (int) floor(hue)
    // Avoiding the floor function improves performance and is ok as
    // long as hue > 0.
    //flr = floor(hue);
    h = static_cast<int>(hue);
    flr = hue - h;
    switch ((h&INT_MAX)%6) {
    case 0 : {
        red=1.0;
        grn=flr;
        blu=0.0;
    }
    break;
    case 1 : {
        red=1-flr;
        grn=1.0;
        blu=0;
    }
    break;
    case 2 : {
        red=0.0;
        grn=1.0;
        blu=flr;
    }
    break;
    case 3 : {
        red=0;
        grn=1-flr;
        blu=1.0;
    }
    break;
    case 4 : {
        red=flr;
        grn=0.0;
        blu=1.0;
    }
    break;
    case 5 : {
        red=1.0;
        grn=0;
        blu=1-flr;
    }
    break;
    default: {
        red=grn=blu=0.0;
    }
    }
    /*
    lum=red*CSR_RLUM+grn*CSR_GLUM+blu*CSR_BLUM;
    tgt_lum=CSR_MIN_LUM+(CSR_MAX_LUM-CSR_MIN_LUM)*_pos;
    if(lum>tgt_lum){
     double r=Math.sqrt(tgt_lum/lum);
     red=red*r;
     grn=grn*r;
     blu=blu*r;}*/
    return 0xFF000000|((int)(blu*0xFF)<<16)|
           ((int)(grn*0xFF)<<8)|(int)(red*0xFF);
}

const DSColorScale DS_RAINBOW_SCALE={dsRainbowScale};



/*A data scale that maps data linearly so that the minimum is 0 and the
  maximum is 1*/
static double dsLinearScale(const DSLinearScale *_this,double _data)
{
    return (_data+_this->offs)*_this->mul;
}

static double dsLinearUnscale(const DSLinearScale *_this,double _v)
{
    return _v/_this->mul-_this->offs;
}


void dsLinearScaleInit(DSLinearScale *_this,double _min,double _max)
{
    _this->super.scale=(DSScaleFunc)dsLinearScale;
    _this->super.unscale=(DSUnscaleFunc)dsLinearUnscale;
    _this->offs=-_min;
    _this->mul=fabs(_max-_min)>1E-100?1/(_max-_min):1;
}


const DSLinearScale DS_LINEAR_SCALE_IDENTITY= {
    {
        (DSScaleFunc)dsLinearScale,
        (DSUnscaleFunc)dsLinearUnscale
    },
    1,
    0
};



/*A data scale that maps data linearly so that the minimum is 1 and the
  maximum is e^2, and then takes half the log of that (so it is in the range
  0 to 1)*/
static double dsLogScale(const DSLogScale *_this,double _data)
{
    return 0.5*log(1+(_data+_this->offs)*_this->mul);
}

static double dsLogUnscale(const DSLogScale *_this,double _v)
{
    return (exp(2*_v)-1)/_this->mul-_this->offs;
}


void dsLogScaleInit(DSLogScale *_this,double _min,double _max)
{
    _this->super.scale=(DSScaleFunc)dsLogScale;
    _this->super.unscale=(DSUnscaleFunc)dsLogUnscale;
    _this->offs=-_min;
    _this->mul=(fabs(_max-_min)>1E-100?1/(_max-_min):1)*(M_E*M_E-1);
}



/*Inverts a 3x3 matrix. Returns the pseudoinvers iff the matrix was not
  invertible*/
void dsMatrix3x3Inv(const double _m[3][3],double _i[3][3])
{
    double det;
    _i[0][0]=_m[1][1]*_m[2][2]-_m[1][2]*_m[2][1];
    _i[1][0]=_m[1][2]*_m[2][0]-_m[1][0]*_m[2][2];
    _i[2][0]=_m[1][0]*_m[2][1]-_m[1][1]*_m[2][0];
    det=_m[0][0]*_i[0][0]+_m[0][1]*_i[1][0]+_m[0][2]*_i[2][0];
    if (fabs(det)<1E-100)dsMatrix3x3PInv(_m,_i);
    else {
        det=1/det;
        _i[0][0]*=det;
        _i[1][0]*=det;
        _i[2][0]*=det;
        _i[0][1]=det*(_m[2][1]*_m[0][2]-_m[2][2]*_m[0][1]);
        _i[1][1]=det*(_m[2][2]*_m[0][0]-_m[2][0]*_m[0][2]);
        _i[2][1]=det*(_m[2][0]*_m[0][1]-_m[2][1]*_m[0][0]);
        _i[0][2]=det*(_m[0][1]*_m[1][2]-_m[0][2]*_m[1][1]);
        _i[1][2]=det*(_m[0][2]*_m[1][0]-_m[0][0]*_m[1][2]);
        _i[2][2]=det*(_m[0][0]*_m[1][1]-_m[0][1]*_m[1][0]);
    }
}

/*Calculates the inverse of a 1x1 or 2x2 matrix. If the matrix is too close to
  singular, the identity is returned*/
static void dsMatrix2x2HInv(const double _m[3][3],double _i[2][2],int _h)
{
    switch (_h) {
    case 1: {
        if (fabs(_m[0][0])<1E-100)_i[0][0]=1;
        else _i[0][0]=1/_m[0][0];
    }
    break;
    case 2: {
        double d;
        d=_m[0][0]*_m[1][1]-_m[0][1]*_m[1][0];
        if (fabs(d)<1E-100) {
            _i[0][0]=_i[1][1]=1;
            _i[0][1]=_i[1][0]=0;
        } else {
            _i[0][0]=_m[1][1]/d;
            _i[0][1]=-_m[0][1]/d;
            _i[1][0]=-_m[1][0]/d;
            _i[1][1]=_m[0][0]/d;
        }
        break;
    }
    }
}

/*Pseudoinverts a 3x3 matrix.*/
void dsMatrix3x3PInv(const double _m[3][3],double _i[3][3])
{
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
    for (h=k=0; k<3; k++) {
        double v;
        int l = 0;
        v=0;
        for (i=h; i<3; i++) {
            double aluik;
            aluik=fabs(lu[i][k]);
            if (aluik>v) {
                v=aluik;
                l=i;
            }
        }
        if (v>1E-8) {
            int t;
            t=p[h];
            p[h]=p[l];
            p[l]=t;
            for (i=0; i<3; i++) {
                double d;
                d=lu[h][i];
                lu[h][i]=lu[l][i];
                lu[l][i]=d;
            }
            for (i=h+1; i<3; i++) {
                lu[i][k]/=lu[h][k];
                for (j=k+1; j<3; j++)lu[i][j]-=lu[i][k]*lu[h][j];
            }
            h++;
        }
    }
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
    switch (h) {
    case 0: {
        for (i=0; i<3; i++)for (j=0; j<3; j++)_i[i][j]=0;
        return;
    }
    case 3: {                                             /*Should be impossible*/
        for (i=0; i<3; i++)for (j=0; j<3; j++)_i[i][j]=i==j;
        return;
    }
    }
    for (j=0; j<h; j++) {
        for (i=0; i<j; i++)x[p[i]][j]=0;
        x[p[i++]][j]=1;
        for (; i<3; i++)x[p[i]][j]=lu[i][j];
    }
    for (i=0; i<h; i++) {
        for (j=0; j<i; j++)y[j][i]=0;
        for (; j<3; j++)y[j][i]=lu[i][j];
    }
    /*At this point,
           *
      m==xy , where x and y are 3 by h matrices, a full-rank decomposition*/
    for (i=0; i<h; i++)for (j=0; j<h; j++) {
            a[i][j]=0;
            for (k=0; k<3; k++)a[i][j]+=x[k][i]*x[k][j];
        }
    dsMatrix2x2HInv(a,xtxi,h);
    /*              *
      xtxi is now (x x)^-1*/
    for (i=0; i<h; i++)for (j=0; j<h; j++) {
            a[i][j]=0;
            for (k=0; k<3; k++)a[i][j]+=y[k][i]*y[k][j];
        }
    dsMatrix2x2HInv(a,ytyi,h);
    /*              *
      ytyi is now (y y)^-1*/
    for (i=0; i<3; i++)for (j=0; j<h; j++) {
            _i[i][j]=0;
            for (k=0; k<h; k++)_i[i][j]+=y[i][k]*ytyi[k][j];
        }
    for (i=0; i<3; i++)for (j=0; j<h; j++) {
            a[i][j]=0;
            for (k=0; k<h; k++)a[i][j]+=_i[i][j]*xtxi[k][j];
        }
    for (i=0; i<3; i++)for (j=0; j<3; j++) {
            _i[i][j]=0;
            for (k=0; k<h; k++)_i[i][j]+=a[i][k]*x[j][k];
        }
}
/*            *        *
  i is now y(y y)^-1 (x x)^-1 x, the pseudoinverse*/


int main(int _argc,char **_argv)
{
    glwInit(&_argc,_argv);

    // Allocate DS3Viewer on heap and DON'T delete it, as destruction
    // is messed up and will crash the program.
    DS3Viewer* ds3v = new DS3Viewer();

    /*At this point, we've created a window, and so should have a current
      rendering context: test for GL extensions*/
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 1;
    }
#ifndef NDEBUG
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
    if (!GLEW_VERSION_1_2) {
        printf("Sorry, OpenGL 1.2 is the minimum required version, and it appears you don't have it. Bye!\n");
	return 1;
    }
    std::string rr = (const char*) glGetString(GL_RENDERER);
    if (rr.find("Mesa") != std::string::npos &&
	rr.find("Intel") != std::string::npos) {
	printf("Disabling 3D texture mipmapping on renderer: %s\n",
	       rr.c_str());
	limit_texture3D_mipmap_level = 0;
    } else if (rr.find("Mesa") != std::string::npos
	       && (rr.find("R200") != std::string::npos 
		   || rr.find("R300") != std::string::npos)) {
	printf("Limiting 3D texture mipmap levels to 7 on renderer: %s\n",
	       rr.c_str());
	printf("See https://bugs.freedesktop.org/show_bug.cgi?id=28284\n");
	limit_texture3D_mipmap_level = 7;
    }
    if (GLEW_ARB_vertex_buffer_object) {
        printf("ARB_vertex_buffer_object extension available, will use VBO's for rendering isosurfaces.\n");
        use_vbo = true;
    }
    if (_argc > 1) ds3ViewerOpenFile(ds3v, _argv[1]);
    glutMainLoop();
}
