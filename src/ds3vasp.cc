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
#include "ds3vasp.hh"
#pragma hdrstop
#if !defined(_ds3vasp_C)
# define _ds3vasp_C (1)

/*Reads the VASP header, and sets up for reading the data asynchronously*/
int ds3VaspReaderInit(DS3VaspReader *_this,DataSet3D *_ds3,FILE *_in)
{
    CDynArray      line;
    CDynArray      points;
    char          *p;
    size_t         npoints;
    double         d;
    double         scale;
    int            i,j;
    size_t         k;
    unsigned long  l;
    int            ret;
    ds3Init(_ds3);
    _ds3->name=NULL;
    _ds3->points=NULL;
    _ds3->data=NULL;
    _DAInit(&line,0,char);
    _DAInit(&points,0,unsigned long);
    /*Read the data set name*/
    if (!daFGetS(&line,_in))goto err;
    daTrimWS(&line);
    daTrimToSize(&line);
    _ds3->name=line.data;
    line.data=NULL;
    line.cap=line.size=0;
    /*Read the "Universal scaling factor"*/
    if (fscanf(_in,"%lf",&scale)<1)goto err;
    /*Read the basis for the box*/
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)if (fscanf(_in,"%lf",_ds3->basis[j]+i)<1)goto err;
    }
    for (i=0; i<3; i++)vectMul3d(_ds3->basis[i],_ds3->basis[i],scale);
    if (fscanf(_in," ")<0)goto err;
    vectSet3d(_ds3->center,0,0,0);
    for (i=0; i<3; i++)vectAdd3d(_ds3->center,_ds3->center,_ds3->basis[i]);
    vectMul3d(_ds3->center,_ds3->center,0.5);
    /*Read the atom types*/
    if (!daFGetS(&line,_in))goto err;
    p=_DAGetAt(&line,0,char);
    for (npoints=0; *p!='\0';)
    {
        char          *e;
        unsigned long  u;
        u=strtoul(p,&e,0);
        if (e==p||*e!='\0'&&!isspace((unsigned char)*e))goto err;
        if (u!=0&&!daInsTail(&points,&u))goto err;
        npoints+=u;
        p=e;
    }
    _ds3->points=(DSPoint3D *)malloc(npoints*sizeof(DSPoint3D));
    if (_ds3->points==NULL)goto err;
    _ds3->npoints=npoints;
    /*Read in the atom positions*/
    /*First line specifies the coordinate mode*/
    if (fscanf(_in," ")<0||!daFGetS(&line,_in)||line.size<1)goto err;
    d=points.size>1?1.0/(points.size-1):1;
    /*Only the first (non-whitespace) character of the line matters*/
    switch (tolower(_DAGetAt(&line,0,unsigned char)[0]))
    {
    case 'd':                                                         /*"Direct"*/
    {
        /*Direct coordinates are fractions of the lattice vectors*/
        for (k=0,i=0; k<points.size; k++)
        {
            double col;
            col=k*d;
            for (l=*_DAGetAt(&points,k,unsigned long); l-->0; i++)
            {
                for (j=0; j<3; j++)if (fscanf(_in,"%lf",_ds3->points[i].pos+j)<1)goto err;
                _ds3->points[i].typ=(int)k;
                _ds3->points[i].col=col;
            }
        }
    }
    break;
    case 'c':                                                           /*"Cart"*/
    {
        /*Cart coordinates are regular cartesian, but should be scaled by the
          universal scaling factor. We convert them to Direct coordinates.*/
        Vect3d basinv[3];
        dsMatrix3x3Inv(_ds3->basis,basinv);
        for (k=0,i=0; k<points.size; k++)
        {
            double col;
            col=k*d;
            for (l=*_DAGetAt(&points,k,unsigned long); l-->0; i++)
            {
                Vect3d pos;
                for (j=0; j<3; j++)if (fscanf(_in,"%lf",&pos[j])<1)goto err;
                for (j=0; j<3; j++)_ds3->points[i].pos[j]=scale*vectDot3d(pos,basinv[j]);
                _ds3->points[i].typ=(int)k;
                _ds3->points[i].col=col;
            }
        }
    }
    break;
    default :
        goto err;
    }
    /*Read in data dimensions*/
    for (i=0; i<3; i++)
    {
        if (fscanf(_in,"%lu",&l)<1)goto err;
        _ds3->density[i]=(size_t)l;
    }
    /*Read in the data*/
    _this->ds3=_ds3;
    _this->in=_in;
    _this->npoints=_ds3->density[0]*_ds3->density[1]*_ds3->density[2];
    _ds3->data=(double *)malloc(_this->npoints*sizeof(double));
    if (_ds3->data==NULL)goto err;
    if (_this->npoints)
    {
        if (fscanf(_in,"%lf",_ds3->data)<1)goto err;
        _ds3->min=_ds3->max=_ds3->data[0];
        _this->k=1;
        ret=1;
    }
    else
    {
        _ds3->min=_ds3->max=0;
        _this->k=0;
        ret=-1;
    }
    goto done;
err:
    ds3Dstr(_ds3);
    if (!errno)errno=EINVAL;
    ret=0;
done:
    daDstr(&line);
    daDstr(&points);
    return ret;
}

/*Reads a block of data from the VASP file*/
int ds3VaspReaderRead(DS3VaspReader *_this)
{
    double *data;
    double  min;
    double  max;
    size_t  k;
    size_t  s;
    int     ret;
    data=_this->ds3->data;
    min=_this->ds3->min;
    max=_this->ds3->max;
    if (_this->npoints-_this->k>DS3_VASP_BLOCK_SIZE)
    {
        s=_this->k+DS3_VASP_BLOCK_SIZE;
        ret=s*100/_this->npoints+1;
    }
    else
    {
        s=_this->npoints;
        ret=-1;
    }
    for (k=_this->k; k<s; k++)
    {
        if (fscanf(_this->in,"%lf",data+k)<1)
        {
            if (!errno)errno=ENOMEM;
            ds3Dstr(_this->ds3);
            ret=0;
            break;
        }
        if (data[k]<min)min=data[k];
        else if (data[k]>max)max=data[k];
    }
    _this->k=k;
    _this->ds3->min=min;
    _this->ds3->max=max;
    return ret;
}

int ds3VaspReaderCancel(DS3VaspReader *_this)
{
    ds3Dstr(_this->ds3);
    return 1;
}

#endif                                                            /*_ds3vasp_C*/
