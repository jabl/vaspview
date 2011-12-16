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
#include "ds3vasp.hh"
#include "strutil.hh"
#include <vector>

/*Reads the VASP header, and sets up for reading the data asynchronously*/
DS3VaspReader::DS3VaspReader(const char* file_name, const char* mode)
        : file(file_name, mode), ds3(new DataSet3D())
{
    std::string    str;
    std::vector<int>    atypes;  // Atom types
    char          *p;
    size_t         npoints;
    double         d;
    double         scale;
    int            i,j;
    size_t         k;
    unsigned long  l;

    if (file.f == NULL)
        return;
    if (!file.fgets(ds3->name)) goto err;
    trimlr(ds3->name);
    /*Read the "Universal scaling factor"*/
    if (fscanf(file.f,"%lf",&scale)<1)goto err;
    /*Read the basis for the box*/
    for (i=0; i<3; i++) {
        for (j=0; j<3; j++)
            if (fscanf(file.f,"%f", &ds3->basis(j, i)) < 1)
                goto err;
    }
    ds3->basis *= scale;
    if (fscanf(file.f," ")<0)goto err;
    ds3->center.setZero();
    for (i=0; i<3; i++) ds3->center += ds3->basis.col(i);
    ds3->center *= 0.5f;
    /*Read the atom types*/
    if (!file.fgets(str)) goto err;
    trimlr(str);
    p = &str[0];
    for (npoints=0; *p!='\0';) {
        char          *e;
        unsigned long  u;
        u=strtoul(p,&e,0);
        if (e == p || (*e != '\0' && !isspace((unsigned char)*e))) goto err;
        if (u != 0) atypes.push_back(u);
        npoints+=u;
        p=e;
    }
    ds3->points.resize(npoints);
    ds3->npoints=npoints;
    /*Read in the atom positions*/
    /*First line specifies the coordinate mode*/
    if (!file.fgets(str)) goto err;
    adjustl(str);
    d = atypes.size() > 1 ? 1.0/(atypes.size() - 1) : 1;
    /*Only the first (non-whitespace) character of the line matters*/
    switch (tolower(str[0])) {
    case 'd': {                                                       /*"Direct"*/
        /*Direct coordinates are fractions of the lattice vectors*/
        for (k = 0, i = 0; k < atypes.size(); k++) {
            double col;
            col=k*d;
            for (int ll = atypes[k]; ll-- > 0; i++) {
                for (j=0; j<3; j++)if (fscanf(file.f,"%lf",ds3->points[i].pos+j)<1)goto err;
                ds3->points[i].typ=(int)k;
                ds3->points[i].col=col;
            }
        }
    }
    break;
    case 'c': {                                                         /*"Cart"*/
        /*Cart coordinates are regular cartesian, but should be scaled by the
          universal scaling factor. We convert them to Direct coordinates.*/
	Eigen::Matrix3f basinv = ds3->basis.inverse();
        for (k = 0,i = 0; k < atypes.size(); k++) {
            double col;
            col=k*d;
            for (int ll = atypes[k]; ll-- > 0; i++) {
		Eigen::Vector3f pos;
                for (j=0; j<3; j++)if (fscanf(file.f,"%f",&pos[j])<1)goto err;
                for (j=0; j<3; j++) 
		    ds3->points[i].pos[j] = scale * pos.dot(basinv.col(j));
                ds3->points[i].typ=(int)k;
                ds3->points[i].col=col;
            }
        }
    }
    break;
    default :
        goto err;
    }
    /*Read in data dimensions*/
    for (i=0; i<3; i++) {
        if (fscanf(file.f,"%lu",&l)<1)goto err;
        ds3->density[i]=(size_t)l;
    }
    /*Read in the data*/
    this->npoints = ds3->density[0]*ds3->density[1]*ds3->density[2];
    ds3->data.reserve(this->npoints);
    if (this->npoints) {
        double val;
        if (fscanf(file.f, "%lf", &val) < 1) goto err;
        ds3->data.push_back(val);
        ds3->min = ds3->max = ds3->data[0];
        this->k = 1;
    } else {
        ds3->min = ds3->max = 0;
        this->k = 0;
    }
    return;
err:
    delete ds3;
    ds3 = NULL;
    if (!errno)errno=EINVAL;
    throw "Error initializing vasp reader\n";
}

DS3VaspReader::~DS3VaspReader() {
    delete ds3;
}

/*Reads a block of data from the VASP file*/
int DS3VaspReader::read()
{
    double  val;
    double  min;
    double  max;
    size_t  k;
    size_t  s;
    int     ret;
    min = this->ds3->min;
    max= this->ds3->max;
    if (this->npoints - this->k > DS3_VASP_BLOCK_SIZE) {
        s = this->k + DS3_VASP_BLOCK_SIZE;
        ret = s * 100 / this->npoints + 1;
    } else {
        s = this->npoints;
        ret=-1;
    }
    for (k=this->k; k<s; k++) {
        if (fscanf(this->file.f, "%lf", &val) < 1) {
            delete ds3;
	    ds3 = NULL;
            ret=0;
            break;
        }
        this->ds3->data.push_back(val);
        if (ds3->data[k] < min) min = ds3->data[k];
        else if (ds3->data[k] > max) max = ds3->data[k];
    }
    this->k=k;
    this->ds3->min=min;
    this->ds3->max=max;
    return ret;
}

int DS3VaspReader::cancel()
{
    fprintf(stderr, "Canceling chgcar read\n");
    delete ds3;
    ds3 = NULL;
    return 1;
}

DataSet3D* DS3VaspReader::release_ds3()
{
    DataSet3D* tmp = ds3;
    ds3 = NULL;
    return tmp;
}
