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
#include "ds3iso.hh"


/*When generating the iso-surface, we cannot simply make a list of triangles
  to draw, for two reasons. One, the surface is translucent (so that it cannot
  hide parts of itself from the viewer, concealing its underlying structure),
  so it must be drawn from back to front order. Secondly, because the portion
  of the dataset being viewed can wrap around, we may have to draw the surface
  up to 27 times. In order to avoid putting the whole surface into the GL
  rendering pipeline 27 times and have the majority of it clipped away (after
  being transformed and lit--the most CPU-intensive operations, even with
  hardware acceleration), we'd like to only draw those parts which actually
  fall within the clip box. So, we use an oct-tree to partition the surface.
  Then, when rendering it, we can only traverse those portions of the tree
  that fall within the clip box, and we can do it in an order so that the
  surface is rendered back to front. We use split planes that are aligned on
  integers in data-set coordinates, and the sizes of all oct-tree nodes are
  powers of two . This is slightly wasteful when the dimensions of the
  data-set are not powers of two, or when the detail level of the iso-surface
  is not a power of two, but the increased simplicity in the code makes it
  worth it both in coding and execution time*/

/*Resets the iso-surface for creating a new surface at the specified detail
  level
  d: The new detail level*/
static void ds3IsoReset(DS3IsoSurface *_this,long _d)
{
    daSetSize(&_this->verts,0);
    daSetSize(&_this->nodes,0);
    daSetSize(&_this->leafs,0);
    while (_this->dim<=_d)_this->dim<<=1;
    _this->stp=_d;
}

/*Adds an oct-tree node to the iso-surface
  x,y,z: The index of the center of the node*/
static long ds3IsoAddNode(DS3IsoSurface *_this)
{
    long ret;
    if (daSetSize(&_this->nodes,_this->nodes.size+1))
    {
        DS3IsoOctNode *node;
        int            i;
        ret=(long)_this->nodes.size-1;
        node=_DAGetAt(&_this->nodes,ret,DS3IsoOctNode);
        for (i=0; i<8; i++)node->node[i]=DS3V_NO_CHILD;
    }
    else ret=DS3V_NO_CHILD;
    return ret;
}

/*Adds a leaf node to the iso-surface
  cv:   An array of indices into the vertex table for each edge of the cube
  tris: A negative terminated list of the edges that compose the triangles
  Return: The index of the leaf created, or DS3V_NO_CHILD if space for the
           leaf could not be allocated*/
static long ds3IsoAddLeaf(DS3IsoSurface *_this,long _cv[12],
                          const int _tris[16])
{
    long ret;
    int  i;
    for (i=0; _tris[i]>=0; i++);
    ret=(long)_this->leafs.size;
    if (daSetSize(&_this->leafs,_this->leafs.size+i+1))
    {
        DS3IsoOctLeaf *leaf;
        leaf=(DS3IsoOctLeaf *)_DAGetAt(&_this->leafs,ret,GLint);
        leaf->nverts=i;
        while (i-->0)leaf->verts[i]=(GLint)_cv[_tris[i]];
    }
    else ret=DS3V_NO_CHILD;
    return ret;
}

/*Adds a set of triangles to the iso surface.
  x:    The index of the lower-left corner of the cube the triangles go in
  cv:   An array of indices into the vertex table for each edge of the cube
  tris: A negative terminated list of the edges that compose the triangles*/
static int ds3IsoAddTris(DS3IsoSurface *_this,long _x[3],
                         long _cv[12],const int _tris[16])
{
    DS3IsoOctNode *nodes;
    long           node;
    long           offs;
    int            idx;
    int            i;
    offs=_this->dim>>1;
    if (_this->nodes.size==0&&ds3IsoAddNode(_this)<0)return 0;
    nodes=_DAGetAt(&_this->nodes,0,DS3IsoOctNode);
    for (node=0; offs>_this->stp; offs>>=1)
    {
        for (i=idx=0; i<3; i++)if (_x[i]&offs)idx|=1<<i;
        if (nodes[node].node[idx]==DS3V_NO_CHILD)
        {
            long n;
            n=ds3IsoAddNode(_this);
            if (n==DS3V_NO_CHILD)return 0;
            nodes=_DAGetAt(&_this->nodes,0,DS3IsoOctNode);
            nodes[node].node[idx]=n;
        }
        node=nodes[node].node[idx];
    }
    for (i=idx=0; i<3; i++)if (_x[i]&offs)idx|=1<<i;
    if (nodes[node].node[idx]==DS3V_NO_CHILD)
    {
        long n;
        n=ds3IsoAddLeaf(_this,_cv,_tris);
        if (n==DS3V_NO_CHILD)return 0;
        nodes[node].node[idx]=n;
    }
    else return 0;
    return 1;
}

/*Multiplies all the vertices in the iso-surface by the data set's basis
  vectors. This allows us to unitize the normals (since all the remaining
  transformations do not involve scales), and thus saves on square roots in
  successive frames*/
static void ds3IsoXForm(DS3IsoSurface *_this,DataSet3D *_ds3)
{
    DS3IsoVertex *verts;
    long          i;
    verts=_DAGetAt(&_this->verts,0,DS3IsoVertex);
    for (i=(long)_this->verts.size; i-->0;)
    {
        Vect3d p;
        double m;
        int    j;
        for (j=0; j<3; j++)p[j]=vectDot3d(_ds3->basis[j],verts[i].vert);
        vectSet3dv(verts[i].vert,p);
        for (j=0; j<3; j++)p[j]=vectDot3d(_ds3->basis[j],verts[i].norm);
        m=vectMag2_3d(p);
        if (m<1E-100)vectSet3d(verts[i].norm,0,0,1);
        else vectMul3d(verts[i].norm,p,1/sqrt(m));
    }
}

/*This holds information used while traversing the oct-tree to draw the
  surface, which keeps from cluttering the stack*/
typedef struct DS3IsoDrawCtx
{
    DS3IsoSurface *iso;                                 /*The iso-surface to draw*/
    long           cntr[3];       /*The data-set coordinates of the center of the
                                                         current oct-tree node*/
    Vect3d         box[2];                 /*The clip box in data-set coordinates*/
    Vect3d         eye;
} DS3IsoDrawCtx; /*The eye position in data-set coordinates*/

/*Draws a list of triangles stored in the leaf of an oct-tree node. Also
  checks to make sure this leaf at least partially intersects the clip box.
  Technically, we could see exactly which clip planes bisect this leaf, and
  only enable those planes, but right now we always keep them all enabled*/
static void ds3ViewIsoDrawLeaf(DS3IsoDrawCtx *_this,long _leaf,long _offs)
{
    DS3IsoOctLeaf *leaf;
    int            i;
    for (i=0; i<3; i++)
    {
        if (_this->cntr[i]+_offs<_this->box[0][i]||
                _this->cntr[i]-_offs>_this->box[1][i])return;
    }
    leaf=(DS3IsoOctLeaf *)_DAGetAt(&_this->iso->leafs,_leaf,GLint);
    glDrawElements(GL_TRIANGLES,leaf->nverts,GL_UNSIGNED_INT,leaf->verts);
}

/*This sets up the parameters for the below function*/
/*{int           i;
 double        hlen;
 for(i=0;i<3;i++){
  ds3view->light[i]=(ds3view->rot[1][i]-ds3view->rot[0][i])*0.707106781186547524401;
  ds3view->eye[i]=ds3view->zoom*(ds3view->hvec[i]=ds3view->rot[2][i]);
  ds3view->hvec[i]+=ds3view->light[i];
  ds3view->eye[i]+=ds3view->cntr[i];}
 hlen=vectMag2_3d(ds3view->hvec);
 if(hlen>1E-16)vectMul3d(ds3view->hvec,ds3view->hvec,1/sqrt(hlen));}*/

/*Draw a list of triangles, doing the lighting ourselves instead of letting GL do it.
  This is equivalent to a two-sided light model, no local viewer, light 0 enabled,
  all material parameters <1,1,1,0.75>, except the back specular, which is <0,0,0,1>,
  and a shininess of 16. Because we can special case a great deal, this should be
  faster than any non-hardware accelerated lighting
static void ds3ViewIsoDrawLitLeaf(DS3View *_this,long _leaf){
 DS3IsoOctLeaf *leaf;
 DS3IsoVertex  *verts;
 GLint          i;
 verts=_DAGetAt(&_this->iso.verts,0,DS3IsoVertex);
 leaf=(DS3IsoOctLeaf *)_DAGetAt(&_this->iso.leafs,_leaf,GLint);
 for(i=0;i<leaf->nverts;i+=3){
  Vect3d d0;
  Vect3d d1;
  Vect3d n;
  GLint  j;
  vectSub3d(d0,verts[leaf->verts[i]].vert,verts[leaf->verts[i+1]].vert);
  vectSub3d(d1,verts[leaf->verts[i+2]].vert,verts[leaf->verts[i+1]].vert);
  vectCross3d(n,d0,d1);
  if(vectDot3d(n,_this->eye)>=vectDot3d(n,verts[leaf->verts[i]].vert)){
   for(j=i+3;j-->i;){
    double lum;
    double ndl;
    double ndh;
    lum=0.2+0.15;
    ndl=vectDot3d(_this->light,verts[leaf->verts[j]].norm);
    if(ndl>0)lum+=0.65*ndl;
    ndh=vectDot3d(_this->hvec,verts[leaf->verts[j]].norm);
    if(ndh>0){
     ndh*=ndh;
     ndh*=ndh;
     ndh*=ndh;
     ndh*=ndh;
     lum+=0.5*ndh;}
    glColor4d(lum,lum,lum,0.75);
    glArrayElement(leaf->verts[j]);} }
  else{
   for(j=i+3;j-->i;){
    double lum;
    double ndl;
    lum=0.2+0.15;
    ndl=vectDot3d(_this->light,verts[leaf->verts[j]].norm);
    if(ndl<0)lum-=0.65*ndl;
    glColor4d(lum,lum,lum,0.75);
    glArrayElement(leaf->verts[j]);} } } }*/

/*Draws all the children of an oct-tree node which fall at least partially
  within the current clip box. Exact clipping is handled by GL. Children are
  drawn back-to-front using the data-set coordinates of the current eye
  position.*/
static void ds3ViewIsoDrawNode(DS3IsoDrawCtx *_this,long _node,long _offs)
{
    DS3IsoOctNode *node;
    int            i;
    int            j;
    int            k;
    int            idx;
    for (i=0; i<3; i++)
    {
        if (_this->cntr[i]+_offs<_this->box[0][i]+1E-4||
                _this->cntr[i]-_offs>_this->box[1][i]-1E-4)return;
    }
    node=_DAGetAt(&_this->iso->nodes,_node,DS3IsoOctNode);
    for (idx=j=0; j<3; j++)if (_this->eye[j]<_this->cntr[j])idx|=1<<j;
    if (_offs>_this->iso->stp)
    {
        _offs>>=1;
        for (i=0; i<8; i++)
        {
            k=i^idx;
            if (node->node[k]!=DS3V_NO_CHILD)
            {
                for (j=0; j<3; j++)
                {
                    if (k&1<<j)_this->cntr[j]+=_offs;
                    else _this->cntr[j]-=_offs;
                }
                ds3ViewIsoDrawNode(_this,node->node[k],_offs);
                for (j=0; j<3; j++)
                {
                    if (k&1<<j)_this->cntr[j]-=_offs;
                    else _this->cntr[j]+=_offs;
                }
            }
        }
    }
    else
    {
        _offs>>=1;
        for (i=0; i<8; i++)
        {
            k=i^idx;
            if (node->node[k]!=DS3V_NO_CHILD)
            {
                for (j=0; j<3; j++)
                {
                    if (k&1<<j)_this->cntr[j]+=_offs;
                    else _this->cntr[j]-=_offs;
                }
                ds3ViewIsoDrawLeaf(_this,node->node[k],_offs);
                for (j=0; j<3; j++)
                {
                    if (k&1<<j)_this->cntr[j]-=_offs;
                    else _this->cntr[j]+=_offs;
                }
            }
        }
    }
}

/*Draws the entire iso-surface (clipped against the current clip box), tiling
  the region described by box with it. The separate instances of the surface
  are drawn in back-to-front order, using the data-set coordinates of the
  current eye position*/
static void ds3ViewIsoDrawTree(DS3View *_this,DS3IsoDrawCtx *_ctx,
                               int _box[2][3])
{
    Vect3d p;
    int    i;
    int    j;
    /*If we're not in a 1x1x1 box, split it up and recurse*/
    for (i=0; i<3; i++)if (_box[0][i]+1<_box[1][i])
        {
            int m;
            if (_ctx->eye[i]<(_box[0][i]+1)*_this->ds3->density[i])
            {
                _box[0][i]+=1;
                ds3ViewIsoDrawTree(_this,_ctx,_box);
                _box[0][i]-=1;
                m=_box[1][i];
                _box[1][i]=_box[0][i]+1;
                ds3ViewIsoDrawTree(_this,_ctx,_box);
                _box[1][i]=m;
            }
            else
            {
                m=_box[1][i];
                _box[1][i]=_box[0][i]+1;
                ds3ViewIsoDrawTree(_this,_ctx,_box);
                _box[1][i]=m;
                _box[0][i]+=1;
                ds3ViewIsoDrawTree(_this,_ctx,_box);
                _box[0][i]-=1;
            }
            return;
        }
    /*We are in a 1x1x1 box: draw a copy of the iso-surface*/
    glPushMatrix();
    vectSet3d(p,0,0,0);
    for (i=0; i<3; i++)
    {
        for (j=0; j<3; j++)p[j]+=_this->ds3->basis[j][i]*_box[0][i];
        _ctx->cntr[i]=(_ctx->iso->dim>>1)+_box[0][i]*_this->ds3->density[i];
    }
    glTranslated(p[X],p[Y],p[Z]);
    ds3ViewIsoDrawNode(_ctx,0,_ctx->iso->dim>>1);
    glPopMatrix();
}

/*Draws the iso-surface*/
static void ds3ViewIsoPeerDisplay(DS3ViewComp *_this,const GLWCallbacks *_cb)
{
    static const GLfloat COLOR[4]={1.F,1.F,1.F,.75F};
    DS3View       *view;
    DS3IsoVertex  *verts;
    DS3IsoDrawCtx  ctx;
    Vect3d         p;
    int            i;
    int            j;
    int            box[2][3];
    view=_this->ds3view;
    /*Create the iso-surface if we don't already have one*/
    if (!view->s_valid)
    {
        if (!ds3IsoMake(&view->iso,view->ds3,view->iso_v,view->iso_d))
        {
            return;
        }
        view->s_valid=1;
    }
    if (view->iso.nodes.size<=0)return;
    ctx.iso=&view->iso;
    /*Scale the viewable box into data-set coordinates*/
    for (i=0; i<2; i++)for (j=0; j<3; j++)
        {
            ctx.box[i][j]=view->box[i][j]*view->ds3->density[j];
        }
    /*Find the location of the eye in data-set coordinates*/
    switch (view->proj)
    {
    case DS3V_PROJECT_ORTHOGRAPHIC:
    {
        /*Approximate being really, really, far away*/
        vectMul3d(p,view->rot[2],(1+view->zoom)*65536);
    }
    break;
    /*case DS3V_PROJECT_PERSPECTIVE :*/
    default                       :
    {
        vectMul3d(p,view->rot[2],view->zoom);
    }
    }
    vectAdd3d(p,p,view->cntr);
    for (i=0; i<3; i++)
    {
        ctx.eye[i]=vectDot3d(p,view->basinv[i]);
        ctx.eye[i]*=view->ds3->density[i];
    }
    /*Figure out what region to tile with surfaces*/
    for (i=0; i<3; i++)
    {
        box[0][i]=(int)floor(ctx.box[0][i]/view->ds3->density[i]);
        box[1][i]=(int)ceil(ctx.box[1][i]/view->ds3->density[i]);
    }
    /*Set up OpenGL parameters*/
    verts=_DAGetAt(&view->iso.verts,0,DS3IsoVertex);
    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_CURRENT_BIT|
                 GL_ENABLE_BIT|GL_LIGHTING_BIT);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    for (i=0; i<6; i++)glEnable(GL_CLIP_PLANE0+i);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
    /*glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);*/
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,COLOR);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,COLOR);
    glMaterialfv(GL_FRONT,GL_SPECULAR,COLOR);
    glMateriali(GL_FRONT,GL_SHININESS,16);
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glVertexPointer(3,GL_DOUBLE,sizeof(DS3IsoVertex),verts[0].vert);
    glNormalPointer(GL_DOUBLE,sizeof(DS3IsoVertex),verts[0].norm);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    ds3ViewIsoDrawTree(view,&ctx,box);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopClientAttrib();
    glPopAttrib();
}

const GLWCallbacks DS3_VIEW_ISO_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    NULL,
    (GLWDisplayFunc)ds3ViewIsoPeerDisplay,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


/*Initializes the iso-surface structure
  dens: The dimensions of the data set to create surfaces for, or NULL if no
         data set will be used*/
void ds3IsoInit(DS3IsoSurface *_this,size_t _dens[3])
{
    int i;
    _DAInit(&_this->verts,0,DS3IsoVertex);
    _DAInit(&_this->nodes,0,DS3IsoOctNode);
    _DAInit(&_this->leafs,0,GLint);
    _this->dim=2;
    if (_dens!=NULL)for (i=0; i<3; i++)
        {
            for (; (size_t)_this->dim<_dens[i]; _this->dim<<=1);
        }
}

/*Frees the memory used by the iso-surface structure*/
void ds3IsoDstr(DS3IsoSurface *_this)
{
    daDstr(&_this->verts);
    daDstr(&_this->nodes);
    daDstr(&_this->leafs);
}


/*Creates an iso-surface for the given data set using the given data value
  and detail level. Every 'd' values from the data set are used to create
  the surface, so large values of 'd' will reduce the number of polygons in
  the surface and the number of data values examined (d=2 produces a surface
  1/8 the complexity of d=1)
  ds3: The data set to create an iso-surface from
  v:   The value of the data along the surface
  d:   The detail level of the surface*/
int ds3IsoMake(DS3IsoSurface *_this,DataSet3D *_ds3,double _v,int _d)
{
    /*These tables for computing the Marching Cubes algorithm are from
      http://www.swin.edu.au/astronomy/pbourke/modelling/polygonise/
      (formerly http://www.mhri.edu.au/~pdb/modelling/polygonise/) by Paul Bourke,
      based on code by Cory Gene Bloyd.
      The indexing of vertices and edges in a cube are defined as:
                 4-----4-----5
                /|          /|
               7 |         5 |
              /  8        /  9
          i  7---+-6-----6   |
             |   |       |   |
             | k 0-----0-+---1
            11  /       10  /
             | 3         | 1
             |/          |/
          O  3-----2-----2  j
      This is somewhat awkward, but the tables were originally set up for the
      axes in a different order. The order was modified so that the first four
      corners corresponded to the lower i, and the next for the upper i, so that
      a bit shift could be used to save four compares per iteration. This required
      renaming a lot of things to avoid changing the actual tables, and hence the
      awkwardness.*/
    static const int EDGE_TABLE[256]=
    {
        0x000,0x109,0x203,0x30A,0x406,0x50F,0x605,0x70C,
        0x80C,0x905,0xA0F,0xB06,0xC0A,0xD03,0xE09,0xF00,
        0x190,0x099,0x393,0x29A,0x596,0x49F,0x795,0x69C,
        0x99C,0x895,0xB9F,0xA96,0xD9A,0xC93,0xF99,0xE90,
        0x230,0x339,0x033,0x13A,0x636,0x73F,0x435,0x53C,
        0xA3C,0xB35,0x83F,0x936,0xE3A,0xF33,0xC39,0xD30,
        0x3A0,0x2A9,0x1A3,0x0AA,0x7A6,0x6AF,0x5A5,0x4AC,
        0xBAC,0xAA5,0x9AF,0x8A6,0xFAA,0xEA3,0xDA9,0xCA0,
        0x460,0x569,0x663,0x76A,0x066,0x16F,0x265,0x36C,
        0xC6C,0xD65,0xE6F,0xF66,0x86A,0x963,0xA69,0xB60,
        0x5F0,0x4F9,0x7F3,0x6FA,0x1F6,0x0FF,0x3F5,0x2FC,
        0xDFC,0xCF5,0xFFF,0xEF6,0x9FA,0x8F3,0xBF9,0xAF0,
        0x650,0x759,0x453,0x55A,0x256,0x35F,0x055,0x15C,
        0xE5C,0xF55,0xC5F,0xD56,0xA5A,0xB53,0x859,0x950,
        0x7C0,0x6C9,0x5C3,0x4CA,0x3C6,0x2CF,0x1C5,0x0CC,
        0xFCC,0xEC5,0xDCF,0xCC6,0xBCA,0xAC3,0x9C9,0x8C0,
        0x8C0,0x9C9,0xAC3,0xBCA,0xCC6,0xDCF,0xEC5,0xFCC,
        0x0CC,0x1C5,0x2CF,0x3C6,0x4CA,0x5C3,0x6C9,0x7C0,
        0x950,0x859,0xB53,0xA5A,0xD56,0xC5F,0xF55,0xE5C,
        0x15C,0x055,0x35F,0x256,0x55A,0x453,0x759,0x650,
        0xAF0,0xBF9,0x8F3,0x9FA,0xEF6,0xFFF,0xCF5,0xDFC,
        0x2FC,0x3F5,0x0FF,0x1F6,0x6FA,0x7F3,0x4F9,0x5F0,
        0xB60,0xA69,0x963,0x86A,0xF66,0xE6F,0xD65,0xC6C,
        0x36C,0x265,0x16F,0x066,0x76A,0x663,0x569,0x460,
        0xCA0,0xDA9,0xEA3,0xFAA,0x8A6,0x9AF,0xAA5,0xBAC,
        0x4AC,0x5A5,0x6AF,0x7A6,0x0AA,0x1A3,0x2A9,0x3A0,
        0xD30,0xC39,0xF33,0xE3A,0x936,0x83F,0xB35,0xA3C,
        0x53C,0x435,0x73F,0x636,0x13A,0x033,0x339,0x230,
        0xE90,0xF99,0xC93,0xD9A,0xA96,0xB9F,0x895,0x99C,
        0x69C,0x795,0x49F,0x596,0x29A,0x393,0x099,0x190,
        0xF00,0xE09,0xD03,0xC0A,0xB06,0xA0F,0x905,0x80C,
        0x70C,0x605,0x50F,0x406,0x30A,0x203,0x109,0x000
    };
    static const int TRI_TABLE[256][16]=
    {
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 1, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 8, 3, 9, 8, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 3, 1, 2,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 9, 2,10, 0, 2, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 2, 8, 3, 2,10, 8,10, 9, 8,-1,-1,-1,-1,-1,-1,-1},
        { 3,11, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0,11, 2, 8,11, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 9, 0, 2, 3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1,11, 2, 1, 9,11, 9, 8,11,-1,-1,-1,-1,-1,-1,-1},
        { 3,10, 1,11,10, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0,10, 1, 0, 8,10, 8,11,10,-1,-1,-1,-1,-1,-1,-1},
        { 3, 9, 0, 3,11, 9,11,10, 9,-1,-1,-1,-1,-1,-1,-1},
        { 9, 8,10,10, 8,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 7, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 3, 0, 7, 3, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 1, 9, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 1, 9, 4, 7, 1, 7, 3, 1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,10, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 3, 4, 7, 3, 0, 4, 1, 2,10,-1,-1,-1,-1,-1,-1,-1},
        { 9, 2,10, 9, 0, 2, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1},
        { 2,10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4,-1,-1,-1,-1},
        { 8, 4, 7, 3,11, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {11, 4, 7,11, 2, 4, 2, 0, 4,-1,-1,-1,-1,-1,-1,-1},
        { 9, 0, 1, 8, 4, 7, 2, 3,11,-1,-1,-1,-1,-1,-1,-1},
        { 4, 7,11, 9, 4,11, 9,11, 2, 9, 2, 1,-1,-1,-1,-1},
        { 3,10, 1, 3,11,10, 7, 8, 4,-1,-1,-1,-1,-1,-1,-1},
        { 1,11,10, 1, 4,11, 1, 0, 4, 7,11, 4,-1,-1,-1,-1},
        { 4, 7, 8, 9, 0,11, 9,11,10,11, 0, 3,-1,-1,-1,-1},
        { 4, 7,11, 4,11, 9, 9,11,10,-1,-1,-1,-1,-1,-1,-1},
        { 9, 5, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 9, 5, 4, 0, 8, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 5, 4, 1, 5, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 8, 5, 4, 8, 3, 5, 3, 1, 5,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,10, 9, 5, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 3, 0, 8, 1, 2,10, 4, 9, 5,-1,-1,-1,-1,-1,-1,-1},
        { 5, 2,10, 5, 4, 2, 4, 0, 2,-1,-1,-1,-1,-1,-1,-1},
        { 2,10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8,-1,-1,-1,-1},
        { 9, 5, 4, 2, 3,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0,11, 2, 0, 8,11, 4, 9, 5,-1,-1,-1,-1,-1,-1,-1},
        { 0, 5, 4, 0, 1, 5, 2, 3,11,-1,-1,-1,-1,-1,-1,-1},
        { 2, 1, 5, 2, 5, 8, 2, 8,11, 4, 8, 5,-1,-1,-1,-1},
        {10, 3,11,10, 1, 3, 9, 5, 4,-1,-1,-1,-1,-1,-1,-1},
        { 4, 9, 5, 0, 8, 1, 8,10, 1, 8,11,10,-1,-1,-1,-1},
        { 5, 4, 0, 5, 0,11, 5,11,10,11, 0, 3,-1,-1,-1,-1},
        { 5, 4, 8, 5, 8,10,10, 8,11,-1,-1,-1,-1,-1,-1,-1},
        { 9, 7, 8, 5, 7, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 9, 3, 0, 9, 5, 3, 5, 7, 3,-1,-1,-1,-1,-1,-1,-1},
        { 0, 7, 8, 0, 1, 7, 1, 5, 7,-1,-1,-1,-1,-1,-1,-1},
        { 1, 5, 3, 3, 5, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 9, 7, 8, 9, 5, 7,10, 1, 2,-1,-1,-1,-1,-1,-1,-1},
        {10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3,-1,-1,-1,-1},
        { 8, 0, 2, 8, 2, 5, 8, 5, 7,10, 5, 2,-1,-1,-1,-1},
        { 2,10, 5, 2, 5, 3, 3, 5, 7,-1,-1,-1,-1,-1,-1,-1},
        { 7, 9, 5, 7, 8, 9, 3,11, 2,-1,-1,-1,-1,-1,-1,-1},
        { 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7,11,-1,-1,-1,-1},
        { 2, 3,11, 0, 1, 8, 1, 7, 8, 1, 5, 7,-1,-1,-1,-1},
        {11, 2, 1,11, 1, 7, 7, 1, 5,-1,-1,-1,-1,-1,-1,-1},
        { 9, 5, 8, 8, 5, 7,10, 1, 3,10, 3,11,-1,-1,-1,-1},
        { 5, 7, 0, 5, 0, 9, 7,11, 0, 1, 0,10,11,10, 0,-1},
        {11,10, 0,11, 0, 3,10, 5, 0, 8, 0, 7, 5, 7, 0,-1},
        {11,10, 5, 7,11, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {10, 6, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 3, 5,10, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 9, 0, 1, 5,10, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 8, 3, 1, 9, 8, 5,10, 6,-1,-1,-1,-1,-1,-1,-1},
        { 1, 6, 5, 2, 6, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 6, 5, 1, 2, 6, 3, 0, 8,-1,-1,-1,-1,-1,-1,-1},
        { 9, 6, 5, 9, 0, 6, 0, 2, 6,-1,-1,-1,-1,-1,-1,-1},
        { 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8,-1,-1,-1,-1},
        { 2, 3,11,10, 6, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {11, 0, 8,11, 2, 0,10, 6, 5,-1,-1,-1,-1,-1,-1,-1},
        { 0, 1, 9, 2, 3,11, 5,10, 6,-1,-1,-1,-1,-1,-1,-1},
        { 5,10, 6, 1, 9, 2, 9,11, 2, 9, 8,11,-1,-1,-1,-1},
        { 6, 3,11, 6, 5, 3, 5, 1, 3,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8,11, 0,11, 5, 0, 5, 1, 5,11, 6,-1,-1,-1,-1},
        { 3,11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9,-1,-1,-1,-1},
        { 6, 5, 9, 6, 9,11,11, 9, 8,-1,-1,-1,-1,-1,-1,-1},
        { 5,10, 6, 4, 7, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 3, 0, 4, 7, 3, 6, 5,10,-1,-1,-1,-1,-1,-1,-1},
        { 1, 9, 0, 5,10, 6, 8, 4, 7,-1,-1,-1,-1,-1,-1,-1},
        {10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4,-1,-1,-1,-1},
        { 6, 1, 2, 6, 5, 1, 4, 7, 8,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7,-1,-1,-1,-1},
        { 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6,-1,-1,-1,-1},
        { 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9,-1},
        { 3,11, 2, 7, 8, 4,10, 6, 5,-1,-1,-1,-1,-1,-1,-1},
        { 5,10, 6, 4, 7, 2, 4, 2, 0, 2, 7,11,-1,-1,-1,-1},
        { 0, 1, 9, 4, 7, 8, 2, 3,11, 5,10, 6,-1,-1,-1,-1},
        { 9, 2, 1, 9,11, 2, 9, 4,11, 7,11, 4, 5,10, 6,-1},
        { 8, 4, 7, 3,11, 5, 3, 5, 1, 5,11, 6,-1,-1,-1,-1},
        { 5, 1,11, 5,11, 6, 1, 0,11, 7,11, 4, 0, 4,11,-1},
        { 0, 5, 9, 0, 6, 5, 0, 3, 6,11, 6, 3, 8, 4, 7,-1},
        { 6, 5, 9, 6, 9,11, 4, 7, 9, 7,11, 9,-1,-1,-1,-1},
        {10, 4, 9, 6, 4,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4,10, 6, 4, 9,10, 0, 8, 3,-1,-1,-1,-1,-1,-1,-1},
        {10, 0, 1,10, 6, 0, 6, 4, 0,-1,-1,-1,-1,-1,-1,-1},
        { 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1,10,-1,-1,-1,-1},
        { 1, 4, 9, 1, 2, 4, 2, 6, 4,-1,-1,-1,-1,-1,-1,-1},
        { 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4,-1,-1,-1,-1},
        { 0, 2, 4, 4, 2, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 8, 3, 2, 8, 2, 4, 4, 2, 6,-1,-1,-1,-1,-1,-1,-1},
        {10, 4, 9,10, 6, 4,11, 2, 3,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 2, 2, 8,11, 4, 9,10, 4,10, 6,-1,-1,-1,-1},
        { 3,11, 2, 0, 1, 6, 0, 6, 4, 6, 1,10,-1,-1,-1,-1},
        { 6, 4, 1, 6, 1,10, 4, 8, 1, 2, 1,11, 8,11, 1,-1},
        { 9, 6, 4, 9, 3, 6, 9, 1, 3,11, 6, 3,-1,-1,-1,-1},
        { 8,11, 1, 8, 1, 0,11, 6, 1, 9, 1, 4, 6, 4, 1,-1},
        { 3,11, 6, 3, 6, 0, 0, 6, 4,-1,-1,-1,-1,-1,-1,-1},
        { 6, 4, 8,11, 6, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 7,10, 6, 7, 8,10, 8, 9,10,-1,-1,-1,-1,-1,-1,-1},
        { 0, 7, 3, 0,10, 7, 0, 9,10, 6, 7,10,-1,-1,-1,-1},
        {10, 6, 7, 1,10, 7, 1, 7, 8, 1, 8, 0,-1,-1,-1,-1},
        {10, 6, 7,10, 7, 1, 1, 7, 3,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7,-1,-1,-1,-1},
        { 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9,-1},
        { 7, 8, 0, 7, 0, 6, 6, 0, 2,-1,-1,-1,-1,-1,-1,-1},
        { 7, 3, 2, 6, 7, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 2, 3,11,10, 6, 8,10, 8, 9, 8, 6, 7,-1,-1,-1,-1},
        { 2, 0, 7, 2, 7,11, 0, 9, 7, 6, 7,10, 9,10, 7,-1},
        { 1, 8, 0, 1, 7, 8, 1,10, 7, 6, 7,10, 2, 3,11,-1},
        {11, 2, 1,11, 1, 7,10, 6, 1, 6, 7, 1,-1,-1,-1,-1},
        { 8, 9, 6, 8, 6, 7, 9, 1, 6,11, 6, 3, 1, 3, 6,-1},
        { 0, 9, 1,11, 6, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 7, 8, 0, 7, 0, 6, 3,11, 0,11, 6, 0,-1,-1,-1,-1},
        { 7,11, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 7, 6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 3, 0, 8,11, 7, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 1, 9,11, 7, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 8, 1, 9, 8, 3, 1,11, 7, 6,-1,-1,-1,-1,-1,-1,-1},
        {10, 1, 2, 6,11, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,10, 3, 0, 8, 6,11, 7,-1,-1,-1,-1,-1,-1,-1},
        { 2, 9, 0, 2,10, 9, 6,11, 7,-1,-1,-1,-1,-1,-1,-1},
        { 6,11, 7, 2,10, 3,10, 8, 3,10, 9, 8,-1,-1,-1,-1},
        { 7, 2, 3, 6, 2, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 7, 0, 8, 7, 6, 0, 6, 2, 0,-1,-1,-1,-1,-1,-1,-1},
        { 2, 7, 6, 2, 3, 7, 0, 1, 9,-1,-1,-1,-1,-1,-1,-1},
        { 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6,-1,-1,-1,-1},
        {10, 7, 6,10, 1, 7, 1, 3, 7,-1,-1,-1,-1,-1,-1,-1},
        {10, 7, 6, 1, 7,10, 1, 8, 7, 1, 0, 8,-1,-1,-1,-1},
        { 0, 3, 7, 0, 7,10, 0,10, 9, 6,10, 7,-1,-1,-1,-1},
        { 7, 6,10, 7,10, 8, 8,10, 9,-1,-1,-1,-1,-1,-1,-1},
        { 6, 8, 4,11, 8, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 3, 6,11, 3, 0, 6, 0, 4, 6,-1,-1,-1,-1,-1,-1,-1},
        { 8, 6,11, 8, 4, 6, 9, 0, 1,-1,-1,-1,-1,-1,-1,-1},
        { 9, 4, 6, 9, 6, 3, 9, 3, 1,11, 3, 6,-1,-1,-1,-1},
        { 6, 8, 4, 6,11, 8, 2,10, 1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,10, 3, 0,11, 0, 6,11, 0, 4, 6,-1,-1,-1,-1},
        { 4,11, 8, 4, 6,11, 0, 2, 9, 2,10, 9,-1,-1,-1,-1},
        {10, 9, 3,10, 3, 2, 9, 4, 3,11, 3, 6, 4, 6, 3,-1},
        { 8, 2, 3, 8, 4, 2, 4, 6, 2,-1,-1,-1,-1,-1,-1,-1},
        { 0, 4, 2, 4, 6, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8,-1,-1,-1,-1},
        { 1, 9, 4, 1, 4, 2, 2, 4, 6,-1,-1,-1,-1,-1,-1,-1},
        { 8, 1, 3, 8, 6, 1, 8, 4, 6, 6,10, 1,-1,-1,-1,-1},
        {10, 1, 0,10, 0, 6, 6, 0, 4,-1,-1,-1,-1,-1,-1,-1},
        { 4, 6, 3, 4, 3, 8, 6,10, 3, 0, 3, 9,10, 9, 3,-1},
        {10, 9, 4, 6,10, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 9, 5, 7, 6,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 3, 4, 9, 5,11, 7, 6,-1,-1,-1,-1,-1,-1,-1},
        { 5, 0, 1, 5, 4, 0, 7, 6,11,-1,-1,-1,-1,-1,-1,-1},
        {11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5,-1,-1,-1,-1},
        { 9, 5, 4,10, 1, 2, 7, 6,11,-1,-1,-1,-1,-1,-1,-1},
        { 6,11, 7, 1, 2,10, 0, 8, 3, 4, 9, 5,-1,-1,-1,-1},
        { 7, 6,11, 5, 4,10, 4, 2,10, 4, 0, 2,-1,-1,-1,-1},
        { 3, 4, 8, 3, 5, 4, 3, 2, 5,10, 5, 2,11, 7, 6,-1},
        { 7, 2, 3, 7, 6, 2, 5, 4, 9,-1,-1,-1,-1,-1,-1,-1},
        { 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7,-1,-1,-1,-1},
        { 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0,-1,-1,-1,-1},
        { 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8,-1},
        { 9, 5, 4,10, 1, 6, 1, 7, 6, 1, 3, 7,-1,-1,-1,-1},
        { 1, 6,10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4,-1},
        { 4, 0,10, 4,10, 5, 0, 3,10, 6,10, 7, 3, 7,10,-1},
        { 7, 6,10, 7,10, 8, 5, 4,10, 4, 8,10,-1,-1,-1,-1},
        { 6, 9, 5, 6,11, 9,11, 8, 9,-1,-1,-1,-1,-1,-1,-1},
        { 3, 6,11, 0, 6, 3, 0, 5, 6, 0, 9, 5,-1,-1,-1,-1},
        { 0,11, 8, 0, 5,11, 0, 1, 5, 5, 6,11,-1,-1,-1,-1},
        { 6,11, 3, 6, 3, 5, 5, 3, 1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,10, 9, 5,11, 9,11, 8,11, 5, 6,-1,-1,-1,-1},
        { 0,11, 3, 0, 6,11, 0, 9, 6, 5, 6, 9, 1, 2,10,-1},
        {11, 8, 5,11, 5, 6, 8, 0, 5,10, 5, 2, 0, 2, 5,-1},
        { 6,11, 3, 6, 3, 5, 2,10, 3,10, 5, 3,-1,-1,-1,-1},
        { 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2,-1,-1,-1,-1},
        { 9, 5, 6, 9, 6, 0, 0, 6, 2,-1,-1,-1,-1,-1,-1,-1},
        { 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8,-1},
        { 1, 5, 6, 2, 1, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 3, 6, 1, 6,10, 3, 8, 6, 5, 6, 9, 8, 9, 6,-1},
        {10, 1, 0,10, 0, 6, 9, 5, 0, 5, 6, 0,-1,-1,-1,-1},
        { 0, 3, 8, 5, 6,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {10, 5, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {11, 5,10, 7, 5,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {11, 5,10,11, 7, 5, 8, 3, 0,-1,-1,-1,-1,-1,-1,-1},
        { 5,11, 7, 5,10,11, 1, 9, 0,-1,-1,-1,-1,-1,-1,-1},
        {10, 7, 5,10,11, 7, 9, 8, 1, 8, 3, 1,-1,-1,-1,-1},
        {11, 1, 2,11, 7, 1, 7, 5, 1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2,11,-1,-1,-1,-1},
        { 9, 7, 5, 9, 2, 7, 9, 0, 2, 2,11, 7,-1,-1,-1,-1},
        { 7, 5, 2, 7, 2,11, 5, 9, 2, 3, 2, 8, 9, 8, 2,-1},
        { 2, 5,10, 2, 3, 5, 3, 7, 5,-1,-1,-1,-1,-1,-1,-1},
        { 8, 2, 0, 8, 5, 2, 8, 7, 5,10, 2, 5,-1,-1,-1,-1},
        { 9, 0, 1, 5,10, 3, 5, 3, 7, 3,10, 2,-1,-1,-1,-1},
        { 9, 8, 2, 9, 2, 1, 8, 7, 2,10, 2, 5, 7, 5, 2,-1},
        { 1, 3, 5, 3, 7, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 7, 0, 7, 1, 1, 7, 5,-1,-1,-1,-1,-1,-1,-1},
        { 9, 0, 3, 9, 3, 5, 5, 3, 7,-1,-1,-1,-1,-1,-1,-1},
        { 9, 8, 7, 5, 9, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 5, 8, 4, 5,10, 8,10,11, 8,-1,-1,-1,-1,-1,-1,-1},
        { 5, 0, 4, 5,11, 0, 5,10,11,11, 3, 0,-1,-1,-1,-1},
        { 0, 1, 9, 8, 4,10, 8,10,11,10, 4, 5,-1,-1,-1,-1},
        {10,11, 4,10, 4, 5,11, 3, 4, 9, 4, 1, 3, 1, 4,-1},
        { 2, 5, 1, 2, 8, 5, 2,11, 8, 4, 5, 8,-1,-1,-1,-1},
        { 0, 4,11, 0,11, 3, 4, 5,11, 2,11, 1, 5, 1,11,-1},
        { 0, 2, 5, 0, 5, 9, 2,11, 5, 4, 5, 8,11, 8, 5,-1},
        { 9, 4, 5, 2,11, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 2, 5,10, 3, 5, 2, 3, 4, 5, 3, 8, 4,-1,-1,-1,-1},
        { 5,10, 2, 5, 2, 4, 4, 2, 0,-1,-1,-1,-1,-1,-1,-1},
        { 3,10, 2, 3, 5,10, 3, 8, 5, 4, 5, 8, 0, 1, 9,-1},
        { 5,10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2,-1,-1,-1,-1},
        { 8, 4, 5, 8, 5, 3, 3, 5, 1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 4, 5, 1, 0, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5,-1,-1,-1,-1},
        { 9, 4, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4,11, 7, 4, 9,11, 9,10,11,-1,-1,-1,-1,-1,-1,-1},
        { 0, 8, 3, 4, 9, 7, 9,11, 7, 9,10,11,-1,-1,-1,-1},
        { 1,10,11, 1,11, 4, 1, 4, 0, 7, 4,11,-1,-1,-1,-1},
        { 3, 1, 4, 3, 4, 8, 1,10, 4, 7, 4,11,10,11, 4,-1},
        { 4,11, 7, 9,11, 4, 9, 2,11, 9, 1, 2,-1,-1,-1,-1},
        { 9, 7, 4, 9,11, 7, 9, 1,11, 2,11, 1, 0, 8, 3,-1},
        {11, 7, 4,11, 4, 2, 2, 4, 0,-1,-1,-1,-1,-1,-1,-1},
        {11, 7, 4,11, 4, 2, 8, 3, 4, 3, 2, 4,-1,-1,-1,-1},
        { 2, 9,10, 2, 7, 9, 2, 3, 7, 7, 4, 9,-1,-1,-1,-1},
        { 9,10, 7, 9, 7, 4,10, 2, 7, 8, 7, 0, 2, 0, 7,-1},
        { 3, 7,10, 3,10, 2, 7, 4,10, 1,10, 0, 4, 0,10,-1},
        { 1,10, 2, 8, 7, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 9, 1, 4, 1, 7, 7, 1, 3,-1,-1,-1,-1,-1,-1,-1},
        { 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1,-1,-1,-1,-1},
        { 4, 0, 3, 7, 4, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 4, 8, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 9,10, 8,10,11, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 3, 0, 9, 3, 9,11,11, 9,10,-1,-1,-1,-1,-1,-1,-1},
        { 0, 1,10, 0,10, 8, 8,10,11,-1,-1,-1,-1,-1,-1,-1},
        { 3, 1,10,11, 3,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 2,11, 1,11, 9, 9,11, 8,-1,-1,-1,-1,-1,-1,-1},
        { 3, 0, 9, 3, 9,11, 1, 2, 9, 2,11, 9,-1,-1,-1,-1},
        { 0, 2,11, 8, 0,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 3, 2,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 2, 3, 8, 2, 8,10,10, 8, 9,-1,-1,-1,-1,-1,-1,-1},
        { 9,10, 2, 0, 9, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 2, 3, 8, 2, 8,10, 0, 1, 8, 1,10, 8,-1,-1,-1,-1},
        { 1,10, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 1, 3, 8, 9, 1, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 9, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        { 0, 3, 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
    };
    int            i;
    long           x[2][3];
    long           dx[3];
    long           dim[3];
    long           off[3];
    long           step[3];
    double        *data;
    long          *edges;
    size_t         sz;
    for (i=0; i<3; i++)
    {
        dim[i]=(long)_ds3->density[i];
        if (dim[i]<=_d)return 0;
    }
    for (dx[0]=_d,i=1; i<3; i++)dx[i]=dx[i-1]*dim[i-1];
    for (off[0]=1,i=1; i<3; i++)off[i]=off[i-1]*dim[i-1];
    for (i=0; i<3; i++)step[i]=(dim[i]+_d-1)/_d;
    sz=(size_t)(step[0]*step[1])*12*sizeof(long);
    edges=(long *)malloc(sz);
    if (edges!=NULL)
    {
        ds3IsoReset(_this,_d);
        memset(edges,0xFF/*DS3V_NO_EDGE*/,sz);
        data = &_ds3->data[0];
        for (x[0][Z]=0; x[0][Z]<dim[Z]; x[0][Z]+=_d)
        {
            long o[3];
            if (x[0][Z]+_d<dim[Z])
            {
                o[Z]=dx[Z];
                x[1][Z]=x[0][Z]+_d;
            }
            else
            {
                o[Z]=-x[0][Z]*off[Z];
                x[1][Z]=0;
            }
            for (x[0][Y]=0; x[0][Y]<dim[1]; x[0][Y]+=_d)
            {
                size_t l;
                size_t e;
                int    idx;
                l=dim[X]*(x[0][Y]+dim[Y]*x[0][Z]);
                e=12*step[X]*(x[0][Y]/_d);
                if (x[0][Y]+_d<dim[Y])
                {
                    o[Y]=dx[Y];
                    x[1][Y]=x[0][Y]+_d;
                }
                else
                {
                    o[Y]=-x[0][Y]*off[Y];
                    x[1][Y]=0;
                }
                idx=0;
                if (data[l+o[Z]]<_v)idx|=0x01;
                if (data[l+o[Y]+o[Z]]<_v)idx|=0x02;
                if (data[l+o[Y]]<_v)idx|=0x04;
                if (data[l]<_v)idx|=0x08;
                for (x[0][X]=0; x[0][X]<dim[X]; x[0][X]+=_d)
                {
                    int   isect;
                    const int  *tris;
                    int   b;
                    long  cv[12];
                    if (x[0][X]+_d<dim[X])
                    {
                        o[X]=dx[X];
                        x[1][X]=x[0][X]+_d;
                    }
                    else
                    {
                        o[X]=-x[0][X]/**off[X]*/;
                        x[1][X]=0;
                    }
                    if (data[l+o[X]+o[Z]]<_v)idx|=0x10;
                    if (data[l+o[X]+o[Y]+o[Z]]<_v)idx|=0x20;
                    if (data[l+o[X]+o[Y]]<_v)idx|=0x40;
                    if (data[l+o[X]]<_v)idx|=0x80;
                    /*Find out which edges are intersected:*/
                    isect=EDGE_TABLE[idx];
                    for (b=0; b<12; b++)
                    {
                        if (isect&1<<b)
                        {
                            if (edges[e+b]==DS3V_NO_EDGE)          /*Compute the edge intersection:*/
                            {
                                static const int F[3][12]=
                                {
                                    {0,0,0,0,1,1,1,1,0,0,0,0},
                                    {0,1,0,0,0,1,0,0,0,1,1,0},
                                    {1,0,0,0,1,0,0,0,1,1,0,0}
                                };
                                static const int T[3][12]=
                                {
                                    {0,0,0,0,1,1,1,1,1,1,1,1},
                                    {1,1,1,0,1,1,1,0,0,1,1,0},
                                    {1,1,0,1,1,1,0,1,1,1,0,0}
                                };
                                long          vf;
                                long          vt;
                                long          fx[3];
                                DS3IsoVertex *vert;
                                double        d;
                                if (!daSetSize(&_this->verts,_this->verts.size+1))
                                {
                                    free(edges);
                                    ds3IsoDstr(_this);
                                    return 0;
                                }
                                for (vf=vt=l,i=0; i<3; i++)
                                {
                                    if (F[i][b])
                                    {
                                        vf+=o[i];
                                        fx[i]=x[1][i]?x[1][i]:dim[i];
                                    }
                                    else fx[i]=x[0][i];
                                    if (T[i][b])vt+=o[i];
                                }
                                d=(data[vf]-_v)/(data[vf]-data[vt]);
                                if (d<0)d=0;
                                else if (d>1)d=1;
                                vert=_DAGetAt(&_this->verts,_this->verts.size-1,DS3IsoVertex);
                                for (i=0; i<3; i++)
                                {
                                    double nf;
                                    double nt;
                                    vert->vert[i]=fx[i];
                                    if (F[i][b]!=T[i][b])
                                    {
                                        vert->vert[i]+=d*((T[i][b]?(x[1][i]?x[1][i]:dim[i]):x[0][i])-
                                                                  fx[i]);
                                    }
                                    vert->vert[i]/=dim[i];
                                    nf=0.5*(data[vf+(x[F[i][b]][i]+_d<dim[i]?dx[i]:-x[F[i][b]][i]*off[i])]-
                                            data[vf+(x[F[i][b]][i]>=_d?-dx[i]:(step[i]-1)*dx[i])]);
                                    nt=0.5*(data[vt+(x[T[i][b]][i]+_d<dim[i]?dx[i]:-x[T[i][b]][i]*off[i])]-
                                            data[vt+(x[T[i][b]][i]>=_d?-dx[i]:(step[i]-1)*dx[i])]);
                                    vert->norm[i]=(nf+d*(nt-nf))/dim[i];
                                }
                                edges[e+b]=cv[b]=(long)(_this->verts.size-1);
                            }
                            else cv[b]=edges[e+b];
                        }                  /*Reuse old edge intersections*/
                        else cv[b]=DS3V_NO_EDGE;
                    }
                    if (x[1][0])                      /*Propagate edge intersections forward:*/
            {
                        e+=12;
                        edges[e+0]=cv[4];
                        edges[e+1]=cv[5];
                        edges[e+2]=cv[6];
                        edges[e+3]=cv[7];
                        e-=12;
                    }
                    if (x[1][1])                     /*Propagate edge intersections sideways:*/
                    {
                        e+=12*step[0];
                        edges[e+3]=cv[1];
                        edges[e+8]=cv[9];
                        edges[e+7]=cv[5];
                        edges[e+11]=cv[10];
                        e-=12*step[0];
                    }
                    /*Draw the triangles for the current edge intersections:*/
                    tris=TRI_TABLE[idx];
                    if (tris[0]>=0&&!ds3IsoAddTris(_this,x[0],cv,tris))
                    {
                        free(edges);
                        ds3IsoDstr(_this);
                        return 0;
                    }
                    l+=dx[0];
                    e+=12;
                    idx>>=4;
                }
            }
            if (x[1][2])                             /*Propagate edge intersections up:*/
            {
                size_t e;
                size_t m;
                m=12*step[0]*step[1];
                for (e=0; e<m; e+=12)
                {
                    edges[e+11]=edges[e+8];
                    edges[e+10]=edges[e+9];
                    edges[e+9]=DS3V_NO_EDGE;
                    edges[e+8]=DS3V_NO_EDGE;
                    edges[e+7]=DS3V_NO_EDGE;
                    edges[e+6]=edges[e+4];
                    edges[e+5]=DS3V_NO_EDGE;
                    edges[e+4]=DS3V_NO_EDGE;
                    edges[e+3]=DS3V_NO_EDGE;
                    edges[e+2]=edges[e+0];
                    edges[e+1]=DS3V_NO_EDGE;
                    edges[e+0]=DS3V_NO_EDGE;
                }
            }
        }
        ds3IsoXForm(_this,_ds3);
        free(edges);
        return 1;
    }
    return 0;
}
