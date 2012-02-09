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
#include "ds3view.hh"
#include "ds3slice.hh"

extern int limit_texture3D_mipmap_level;

/*NOTE: No operation prevents the data set from being repeated an
  arbitrary amount in each direction. As we can rely on 3D texturing
  being present, the only limitation is the be framerate (which can
  get quite low with detailed iso-surfaces). All code assumes the
  bounds of the clip box may be anything*/


/*Creates a 3D texture (requires OpenGL 1.2).  This is much more
  advantageous than a 2D texture, since we do not have to create a 2D
  texture in software every time the slice moves (expensive, since the
  texture is nine times the size required to fill a unit box to
  account for repeating, and must be even larger to account for the
  power of two texture sizes required by OpenGL), and the 3D texturing
  may be hardware accelerated.*/
static int ds3SliceTexture3D(DS3Slice *_this,DS3View *_view)
{
    GLint m;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE,&m);
#ifndef NDEBUG
    printf("Max 3D texture size: %d\n", m);
#endif
    //if (m>1)m>>=1;  // Reduce texture size to conserve memory?
    if (_view->ds3!=NULL) {
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
	std::vector<GLubyte>  txtr;
        double    x[3];
        double    xm[3];
        double    dx[3];
        double   *data;
        double    v[4];
        GLWcolor  c;
        for (i=0; i<3; i++) {
            d[i]=_view->ds3->density[i];
            for (w[i]=1,ws[i]=0; w[i]<d[i]&&w[i]<(GLsizei)m; w[i]<<=1,ws[i]++);
            dx[i]=d[i]/(double)w[i];
        }
#ifndef NDEBUG
        printf("calculated texture size: x=%d, y=%d, z=%d\n", w[X], w[Y], w[Z]);
#endif
	txtr.reserve(4 * w[X] * w[Y] * w[Z]);
        data = &_view->ds3->data[0];
        if (!_this->t_id)glGenTextures(1,&_this->t_id);
        glBindTexture(GL_TEXTURE_3D,_this->t_id);
        glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_REPEAT);
	/*Create the full-sized texture*/
	for (n = 0,j[Z] = 0,x[Z] = 0; j[Z] < w[Z]; j[Z]++, x[Z] += dx[Z]) {
	    k[Z] = (GLsizei)x[Z];
	    o[Z] = (k[Z] + 1 >= d[Z] ? -(GLint)k[Z] : 1) * d[X] * d[Y];
	    xm[Z] = x[Z] - k[Z];
	    for (j[Y] = 0, x[Y] = 0; j[Y] < w[Y]; j[Y]++, x[Y] += dx[Y]) {
		k[Y] = (GLsizei)x[Y];
		o[Y] = (k[Y] + 1 >= d[Y] ? -(GLint)k[Y] : 1) * d[X];
		xm[Y] = x[Y] - k[Y];
		for (j[X] = 0, x[X] = 0; j[X] < w[X]; j[X]++, x[X] += dx[X]) {
		    k[X] = (GLsizei)x[X];
		    o[X] = k[X] + 1 >= d[X] ? -(GLint)k[X] : 1;
		    xm[X] = x[X] - k[X];
		    l = k[X] + d[X] * (k[Y] + d[Y] * k[Z]);
		    v[0] = data[l] + xm[Z] * (data[l + o[Z]] - data[l]);
		    v[1] = data[l + o[X]] + xm[Z] * (data[l + o[X] + o[Z]] 
						     - data[l + o[X]]);
		    v[2] = data[l + o[Y]] + xm[Z] * (data[l + o[Y] + o[Z]] 
						     - data[l + o[Y]]);
		    v[3] = data[l + o[X] + o[Y]] + xm[Z] 
			* (data[l + o[X] + o[Y] + o[Z]] 
			   - data[l + o[X] + o[Y]]);
		    v[0] += xm[Y] * (v[2] - v[0]);
		    v[1] += xm[Y] * (v[3] - v[1]);
		    c = dsColorScale(_view->cs, 
				     dsScale(_view->ds, v[0] 
					     + xm[X] * (v[1] - v[0])));
		    txtr.push_back((GLubyte)(c&0xFF));
		    txtr.push_back((GLubyte)(c>>8&0xFF));
		    txtr.push_back((GLubyte)(c>>16&0xFF));
		    txtr.push_back((GLubyte)(c>>24&0xFF));
		    n += 4;
		}
	    }
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, w[X], w[Y], w[Z],
		     0, GL_RGBA, GL_UNSIGNED_BYTE, &txtr[0]);
	/*Create mip-maps*/
	for (lod = 1; (ws[X] || ws[Y] || ws[Z])
		 && lod <= limit_texture3D_mipmap_level; lod++) {
	    o[X] = ws[X] ? 4 : 0;
	    o[Y] = ws[Y] ? (w[X] << 2) : 0;
	    o[Z] = ws[Z] ? w[X] << (ws[Y] + 2) : 0;
	    for (i = 0; i < 3; i++) {
		if (ws[i]) {
		    ws[i]--;
		    w[i]>>=1;
		    j[i] = 1;
		} else j[i] = 0;
	    }
	    for (k[Z] = 0; k[Z] < w[Z]; k[Z]++) {
		for (k[Y] = 0; k[Y] < w[Y]; k[Y]++) {
		    for (k[X] = 0; k[X] < w[X]; k[X]++) {
			int c[4];
			l = (k[X] + ((k[Y] + (k[Z] << (ws[Y] + j[Z])))
				     << (ws[X] + j[Y]))) << (j[X] + 2);
			for (i = 0; i<4; i++) {
			    c[i] = txtr[l + i];
			    c[i] += txtr[l + o[X]];
			    c[i] += txtr[l + o[Y] + i];
			    c[i] += txtr[l + o[Y] + o[X] + i];
			    c[i] += txtr[l + o[Z] + i];
			    c[i] += txtr[l + o[Z] + o[X] + i];
			    c[i] += txtr[l + o[Z] + o[Y] + i];
			    c[i] += txtr[l + o[Z] + o[Y] + o[X] + i];
			}
			l = (k[X] + ((k[Y] + (k[Z] << ws[Y])) << ws[X]))
			    << 2;
			for (i = 0; i < 4; i++) 
			    txtr[l + i] = (GLubyte)(c[i]>>3);
		    }
		}
	    }
#ifndef NDEBUG
	    printf("creating mipmap level %d with size: x=%d, y=%d, z=%d\n", lod, w[X], w[Y], w[Z]);
#endif
	    glTexImage3D(GL_TEXTURE_3D, lod, GL_RGBA, w[X], w[Y], w[Z],
			 0, GL_RGBA, GL_UNSIGNED_BYTE, &txtr[0]);
	}
	glBindTexture(GL_TEXTURE_3D, 0);
	_view->t_valid = 1;
    }
    return 1;
}

typedef struct DS3SliceVertex DS3SliceVertex;

struct DS3SliceVertex {
    Eigen::Vector3f p;
    double tx;
    double ty;
};

/*Clips the slice polygon against one of the planes of the bounding box
  slice:  A list of vertices in the slice polygon
  nverts: The number of vertices in the list
  plane:  The plane equation to clip against
  Return: The number of vertices in the new slice polygon*/
static int ds3ViewSliceClipPlane(DS3SliceVertex _slice[16],int _nverts,
                                 double _plane[4])
{
    DS3SliceVertex slice[16];
    int            ret;
    int            i;
    int            j;
    double         d0;
    double         d1;
    ret=0;
    d0=vectDot3d(_slice[0].p,_plane)+_plane[W];
    for (i=0; i<_nverts; i++) {
        j=i+1;
        if (j>=_nverts)j=0;
        d1=vectDot3d(_slice[j].p,_plane)+_plane[W];
        if (d0>=0)*(slice+ret++)=*(_slice+i);
        if (((d0 > 0) && (d1 < 0))||(d0 < 0 && (d1 > 0))) {
            Eigen::Vector3f dp;
            double t;
            dp = _slice[j].p - _slice[i].p;
            t=(-_plane[W]-vectDot3d(_slice[i].p,_plane))/vectDot3d(dp,_plane);
            vectMul3d(slice[ret].p,dp,t);
            vectAdd3d(slice[ret].p,slice[ret].p,_slice[i].p);
            slice[ret].tx=_slice[i].tx+t*(_slice[j].tx-_slice[i].tx);
            slice[ret].ty=_slice[i].ty+t*(_slice[j].ty-_slice[i].ty);
            ret++;
        }
        d0=d1;
    }
    memcpy(_slice,slice,sizeof(DS3SliceVertex)*ret);
    return ret;
}

/*Creates a slice polygon clipped against all the planes in the viewer's
  bounding box
  slice: A list of vertices to store the slice polygon in
  Return: The number of vertices in the slice polygon*/
static int ds3ViewSliceClip(DS3View *_this,DS3SliceVertex _slice[16])
{
    double plane[4];
    int    nverts;
    int    i;
    int    j;
    for (i=0; i<4; i++) {
        Eigen::Vector3f p(-3+6*(i>>1), -3+6*((i&1)^(i>>1)), 0);
        for (j=0; j<3; j++) {
            _slice[i].p[j]=vectDot3d(_this->strans[j],p)+_this->strans[j][W];
        }
        _slice[i].tx=i>>1;
        _slice[i].ty=(i&1)^(i>>1);
    }
    nverts=4;
    plane[Y]=plane[Z]=0;
    for (i=0; i<3; i++) {
        plane[i]=1;
        plane[W]=-_this->box[0][i]+1E-4;
        nverts=ds3ViewSliceClipPlane(_slice,nverts,plane);
        plane[i]=-1;
        plane[W]=_this->box[1][i]+1E-4;
        nverts=ds3ViewSliceClipPlane(_slice,nverts,plane);
        plane[i]=0;
    }
    return nverts;
}

/*Draws the slice*/
static void ds3ViewSlicePeerDisplay(DS3ViewComp *_this,
                                    const GLWCallbacks *_cb)
{
    DS3View       *view;
    view=_this->ds3view;
    if (view->t_valid || ds3SliceTexture3D(&view->slice, view)) {
	DS3SliceVertex slice[16];
	int            nverts;
	int            i;
	nverts = ds3ViewSliceClip(view, slice);
	if (nverts) {
	    glPushAttrib(GL_TEXTURE_BIT|GL_ENABLE_BIT);
	    glPushMatrix();
	    glMultMatrixd(view->basis);
	    if (glwCompIsFocused(&_this->super)) {
		glLineWidth(2);
		glwColor(glwColorBlend(view->super.forec, DS3V_FOCUS_COLOR));
		glBegin(GL_LINE_LOOP);
		for (i = 0; i < nverts; i++) glVertex3dv(slice[i].p);
		glEnd();
		glLineWidth(1);
	    }
	    glBindTexture(GL_TEXTURE_3D, view->slice.t_id);
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    glEnable(GL_TEXTURE_3D);
	    glBegin(GL_POLYGON);
	    for (i = 0; i < nverts; i++) {
		glTexCoord3dv(slice[i].p);
		glVertex3dv(slice[i].p);
	    }
	    glEnd();
	    glBindTexture(GL_TEXTURE_3D, 0);
	    glPopMatrix();
	    glPopAttrib();
	}
    }
}

/*Gets a unit ray from the center of slice rotation in the direction the mouse
  is pointing given the un-projected ray of the current mouse position
  track: The unit vector returned
  p0:    The origin of the unprojected ray
  p1:    The endpoint of the unprojected ray*/
static void ds3ViewGetSliceTrackCoords(DS3View *_this, 
				       Eigen::Vector3f& _track,
                                       const Eigen::Vector3f _p0,
				       const Eigen::Vector3f _p1)
{
    Eigen::Vector3f p0;
    Eigen::Vector3f p1;
    Eigen::Vector3f dp;
    double a,b,c,d,r;
    r=_this->track_rd;
    r=r>1E-16?1/r:1;
    Eigen::Vector3f center = _this->ds3->center;
    p0 = _p0 - center;
    p0 *= r;
    p1 = _p1 - center;
    p1 *= r;
    dp = p1 - p0;
    a = dp.squaredNorm();
    b = dp.dot(p0);
    c = p0.squaredNorm() - 1;
    d=b*b-a*c;
    if (d<0) {
        switch (_this->proj) {
        case DS3V_PROJECT_ORTHOGRAPHIC: {
            int i;
            b=-a*(c+1);
            c=0;
            for (i=0; i<3; i++)c+=p0[i]*dp[i];
            c*=c;
            d=b*b-4*a*c;
            if (d<0||fabs(c)<1E-100) {
                _track << 0,0,1;
                return;
            }
            d=(-b+sqrt(d))/c;
	    p0 *= d;
	    b = dp.dot(p0);
            d=0;
        }
        break;
        /*case DS3V_PROJECT_PERSPECTIVE :*/
        default                       : {
            int i;
            int j;
            c+=1;
            b=-b-c;
            a=0;
            for (i=0; i<3; i++) {
                static const int next[3]={1,2,0};
                j=next[i];
                a+=p1[i]*p1[i];
                d=p0[i]*p1[j]-p0[j]*p1[i];
                a-=d*d;
            }
            d=b*b-a*c;
            if (d<0||fabs(a)<1E-100) {
                _track << 0, 0, 1;
                return;
            }
            d=(-b-sqrt(d))/a;
	    p1 *= d;
	    dp = p1 - p0;
	    a = dp.squaredNorm();
	    b = dp.dot(p0);
            d=0;
        }
        }
    }
    if (fabs(a)<1E-100) 
	_track << 0, 0, 1;
    else {
        d=(-b+_this->track_rt*sqrt(d))/a;
        _track = dp * d + p0;
    }
}

/*Composes an additional rotation onto the given slice orientation, and sets
  the current slice orientation to it
  t:   The original slice theta
  p:   The original slice phi
  rot: The rotation to compose onto these angles*/
static void ds3ViewSlicePostComposeRot(DS3View *_this,double _t,
                                       double _p,double _rot[3][3])
{
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
    for (i=0; i<3; i++)for (j=0; j<3; j++) {
            s[i][j]=0;
            for (k=0; k<3; k++)s[i][j]+=_rot[i][k]*r[k][j];
        }
    st=s[X][Z];
    if (st<-1)st=1;
    else if (st>1)st=1;
    _t=asin(st)*(180/M_PI);
    if (_t<1E-8) {
        if (_t<-1E-4)_t+=360;
        else _t=0;
    }
    if ((fabs(_this->slice_t-_t)>90&&fabs(_this->slice_t-_t+360)>90&&
            fabs(_this->slice_t-_t-360)>90)) {
        _t=180-_t;
        if (_t<1E-8) {
            if (_t<-1E-4)_t+=360;
            else _t=0;
        }
    }
    ct=cos(_t*M_PI/180);
    if (fabs(ct)<1E-8) {
        sp=s[Y][X]/st;
        cp=-s[Z][X]/st;
    } else {
        sp=-s[Y][Z]/ct;
        cp=s[Z][Z]/ct;
    }
    _p=atan2(sp,cp)*(180/M_PI);
    if (_p<1E-8) {
        if (_p<-1E-4)_p+=360;
        else _p=0;
    }
    ds3ViewSetSlice(_this,_t,_p,_this->slice_d);
}

/*Redraws the slice to reflect keyboard focus*/
static void ds3ViewSlicePeerFocus(DS3ViewComp *_this,GLWCallbacks *_cb,
                                  int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

/*Keyboard manipulation of the slice*/
static int ds3ViewSlicePeerSpecial(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                   int _k,int _x,int _y)
{
    DS3View *view;
    int      ret;
    view=_this->ds3view;
    ret=-1;
    switch (_k) {
    case GLUT_KEY_LEFT     : {
        ds3ViewSetSlice(view,view->slice_t-5,view->slice_p,view->slice_d);
    }
    break;
    case GLUT_KEY_RIGHT    : {
        ds3ViewSetSlice(view,view->slice_t+5,view->slice_p,view->slice_d);
    }
    break;
    case GLUT_KEY_UP       : {
        ds3ViewSetSlice(view,view->slice_t,view->slice_p-5,view->slice_d);
    }
    break;
    case GLUT_KEY_DOWN     : {
        ds3ViewSetSlice(view,view->slice_t,view->slice_p+5,view->slice_d);
    }
    break;
    case GLUT_KEY_PAGE_DOWN: {
        ds3ViewSetSlice(view,view->slice_t,view->slice_p,view->slice_d-0.05);
    }
    break;
    case GLUT_KEY_PAGE_UP  : {
        ds3ViewSetSlice(view,view->slice_t,view->slice_p,view->slice_d+0.05);
    }
    break;
    case GLUT_KEY_HOME     : {
        ds3ViewSetSlice(view,0,0,0);
    }
    break;
    case GLUT_KEY_END      : {
        ds3ViewAlignOrientation(view);
    }
    break;
    default                :
        ret=0;
    }
    if (ret>=0)ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    return ret;
}

/*Mouse manipulation of the slice*/
static int ds3ViewSlicePeerMouse(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                 int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0&&_b==GLUT_LEFT_BUTTON&&_s) {
        DS3View *view;
        Eigen::Vector3f   p;
        Eigen::Vector3f   q;
        view=_this->ds3view;
	Eigen::Vector3f cent = view->ds3->center;
        view->track_an = view->track_pt - cent;
        view->track_rd = view->track_an.norm();
        if (view->track_rd<1E-16)
	    view->track_an << 0, 0, 1;
        else view->track_an *= 1/view->track_rd;
        view->track_rt=-1;
        ds3ViewGetSliceTrackCoords(view,p,view->track_p0,view->track_p1);
	p -= view->track_an;
        view->track_rt=1;
        ds3ViewGetSliceTrackCoords(view,q,view->track_p0,view->track_p1);
        q -= view->track_an;
        if (p.squaredNorm() < q.squaredNorm())
	    view->track_rt = -1;
        view->track_p=view->slice_t;
        view->track_y=view->slice_p;
        return 1;
    }
    return ret;
}

/*Dragging the slice orientation around*/
static int ds3ViewSlicePeerMotion(DS3ViewComp *_this,const GLWCallbacks *_cb,
                                  int _x,int _y)
{
    int ret;
    ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
    if (ret>=0&&(_this->super.mouse_b&1<<GLUT_LEFT_BUTTON)) {
        DS3View *view;
        Eigen::Vector3f   p;
        Eigen::Vector3f   axis;
        float qs;
        view=_this->ds3view;
        ds3ViewGetUnprojRay(view,_x,_y,view->track_p0,view->track_p1);
        ds3ViewGetSliceTrackCoords(view,p,view->track_p0,view->track_p1);
        axis = view->track_an.cross(p);
        qs = axis.norm();
        if (qs<1E-8) {
            ds3ViewSetSlice(view,view->track_p,view->track_y,view->slice_d);
        } else {
            double qc;
            double q[3][3];
            int    i,j;
            axis *= 1/qs;
            if (qs>1)qs=1;
            qc=sqrt(1-qs*qs);
            if (view->track_an.dot(p) < 0) 
		qc = -qc;
            for (i=0; i<3; i++) {
                for (j=0; j<3; j++)q[i][j]=axis[i]*axis[j]*(1-qc);
                q[i][i]+=qc;
            }
            axis *= qs;
            q[1][2]-=axis[0];
            q[2][1]+=axis[0];
            q[2][0]-=axis[1];
            q[0][2]+=axis[1];
            q[0][1]-=axis[2];
            q[1][0]+=axis[2];
            ds3ViewSlicePostComposeRot(view,view->track_p,view->track_y,q);
        }
        return 1;
    }
    return ret;
}


const GLWCallbacks DS3_VIEW_SLICE_CALLBACKS= {
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
    NULL
};

/*Initializes the slice data
  dens: The dimensions of the data set, or NULL if there is no data set*/
void ds3SliceInit(DS3Slice *_this,size_t _dens[3])
{
    int    s;
    double d;
    if (_dens!=NULL) {
        for (s=0,d=0; s<3; s++)d+=_dens[s]*_dens[s];
        d=sqrt(d);
    } else d=sqrt(3);
    d*=6;
    _this->t_id=0;
}

/*Destroys the current slice, freeing texture memory on the GPU.  */
void ds3SliceDstr(DS3Slice *_this,DS3View *_view)
{
    if (_this->t_id)glDeleteTextures(1,&_this->t_id);
    _this->t_id=0;
}
