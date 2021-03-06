/*GL Widget Set - simple, portable OpenGL/GLUT widget set
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
#include "glw.hh"

/*The only provided layout manager (other than the component-specific ones).
  This arranges components in a grid, and uses certain rules specified by
  the component's contraints member to allocate space to grid cells, and
  position the components in those cells*/

static void glwGBLInvalidate(GLWGridBagLayout *_this,GLWComponent *_comp)
{
    _this->cache.clear();
    _this->min_w.clear();
    _this->min_h.clear();
    _this->pre_w.clear();
    _this->pre_h.clear();
    _this->weight_x.clear();
    _this->weight_y.clear();
    _this->adj_w.clear();
    _this->adj_h.clear();
    _this->comp=_comp;
    _this->valid=0;
}

static int glwGBLCalcLayout(GLWGridBagLayout *_this,GLWComponent *_comp)
{
    if (_this->validating)return 0;
    if (_comp!=_this->comp)glwGBLInvalidate(_this,_comp);
    if (!_this->valid) {
        size_t         i;
        int            cur_r;
        int            cur_c;
        std::vector<int>      x_max;
        std::vector<int>      y_max;
        int           *xs_w;
        int           *xs_h;
        int            next;
        _this->validating=1;
        _this->cache.resize(_comp->comps.size());
        _this->w=_this->h=0;
        cur_r=cur_c=-1;
        /*Pass 1: figure out dimensions of the layout grid*/
        for (i=0; (size_t)i<_comp->comps.size(); i++)if (glwCompIsVisible(_comp->comps[i])) {
                int cur_x;
                int cur_y;
                int cur_w;
                int cur_h;
                int px;
                int py;
                int j;
                cur_x=_comp->comps[i]->constraints.gridx;
                cur_y=_comp->comps[i]->constraints.gridy;
                cur_w=_comp->comps[i]->constraints.gridw;
                cur_h=_comp->comps[i]->constraints.gridh;
                if (cur_w<=0)cur_w=1;
                if (cur_h<=0)cur_h=1;
                if (cur_x<0&&cur_y<0) {
                    if (cur_r>=0)cur_y=cur_r;
                    else if (cur_c>=0)cur_x=cur_c;
                    else cur_y=0;
                }
                if (cur_x<0) {
                    px=0;
                    for (j = cur_y; j < cur_y + cur_h
                            && (size_t)j < x_max.size(); j++) {
                        if (x_max[j] > px) px = x_max[j];
                    }
                    cur_x=px-cur_x-1;
                    if (cur_x<0)cur_x=0;
                } else if (cur_y<0) {
                    py=0;
                    for (j = cur_x; j < cur_x + cur_w
                            && (size_t)j < y_max.size(); j++) {
                        if (y_max[j] > py) py = y_max[j];
                    }
                    cur_y=py-cur_y-1;
                    if (cur_y<0)cur_y=0;
                }
                px=0;
                while (x_max.size() < (size_t)cur_x) {
                    x_max.push_back(px);
                }
                while (y_max.size() < (size_t)cur_y) {
                    y_max.push_back(px);
                }
                px=cur_x+cur_w;
                if (_this->w<px)_this->w=px;
                py=cur_y+cur_h;
                if (_this->h<py)_this->h=py;
                for (j=cur_y; j<py; j++) {
                    if ((size_t)j < x_max.size()) x_max[j] = px;
                    else x_max.push_back(px);
                }
                for (j=cur_x; j<px; j++) {
                    if ((size_t)j < y_max.size()) y_max[j] = py;
                    else y_max.push_back(py);
                }
                /*Cache minimum sizes while we're here*/
                glwCompGetMinSize(_comp->comps[i],&_this->cache[i].min_w,&_this->cache[i].min_h);
                glwCompGetPreSize(_comp->comps[i],&_this->cache[i].pre_w,&_this->cache[i].pre_h);
                glwCompGetMaxSize(_comp->comps[i],&_this->cache[i].max_w,&_this->cache[i].max_h);
                if (_this->cache[i].min_w<0)_this->cache[i].min_w=0;
                if (_this->cache[i].min_h<0)_this->cache[i].min_h=0;
                if (_this->cache[i].pre_w<0)_this->cache[i].pre_w=0;
                if (_this->cache[i].pre_h<0)_this->cache[i].pre_h=0;
                if (_comp->comps[i]->constraints.gridw<=0&&
                        _comp->comps[i]->constraints.gridh<=0) {
                    cur_r=cur_c=-1;
                }
                if (_comp->comps[i]->constraints.gridh<=0&&cur_r<0) {
                    cur_c=cur_x+cur_w;
                } else if (_comp->comps[i]->constraints.gridw<=0&&cur_c<0) {
                    cur_r=cur_y+cur_h;
                }
            }
        /*Pass 2: Position relative components*/
        cur_r=cur_c=-1;
        x_max.clear();
        y_max.clear();
        for (i=0; (size_t)i<_comp->comps.size(); i++)if (glwCompIsVisible(_comp->comps[i])) {
                int cur_x;
                int cur_y;
                int cur_w;
                int cur_h;
                int px;
                int py;
                int j;
                cur_x=_comp->comps[i]->constraints.gridx;
                cur_y=_comp->comps[i]->constraints.gridy;
                cur_w=_comp->comps[i]->constraints.gridw;
                cur_h=_comp->comps[i]->constraints.gridh;
                if (cur_x<0&&cur_y<0) {
                    if (cur_r>=0)cur_y=cur_r;
                    else if (cur_c>=0)cur_x=cur_c;
                    else cur_y=0;
                }
                if (cur_x<0) {
                    if (cur_h<=0) {
                        cur_h+=_this->h-cur_y;
                        if (cur_h<1)cur_h=1;
                    }
                    px=0;
                    for (j = cur_y; j < cur_y + cur_h
                            && (size_t)j < x_max.size(); j++) {
                        if (px < x_max[j]) px = x_max[j];
                    }
                    cur_x=px-cur_x-1;
                    if (cur_x<0)cur_x=0;
                } else if (cur_y<0) {
                    if (cur_w<=0) {
                        cur_w+=_this->w-cur_x;
                        if (cur_w<1)cur_w=1;
                    }
                    py=0;
                    for (j = cur_x; j < cur_x + cur_w
                            && (size_t)j < y_max.size(); j++) {
                        if (py < y_max[j]) py = y_max[j];
                    }
                    cur_y=py-cur_y-1;
                    if (cur_y<0)cur_y=0;
                }
                if (cur_w<=0) {
                    cur_w+=_this->w-cur_x;
                    if (cur_w<1)cur_w=1;
                }
                if (cur_h<=0) {
                    cur_h+=_this->h-cur_y;
                    if (cur_h<1)cur_h=1;
                }
                px=0;
                while (x_max.size() < (size_t)cur_x) {
                    x_max.push_back(px);
                }
                while (y_max.size() < (size_t)cur_y) {
                    y_max.push_back(px);
                }
                px=cur_x+cur_w;
                py=cur_y+cur_h;
                for (j=cur_y; j<py; j++) {
                    if ((size_t)j < x_max.size()) x_max[j] = px;
                    else x_max.push_back(px);
                }
                for (j=cur_x; j<px; j++) {
                    if ((size_t)j < y_max.size()) y_max[j] = py;
                    else y_max.push_back(py);
                }
                if (_comp->comps[i]->constraints.gridw<=0&&_comp->comps[i]->constraints.gridh<=0) {
                    cur_r=cur_c=-1;
                }
                if (_comp->comps[i]->constraints.gridh<=0&&cur_r<=0)cur_c=cur_x+cur_w;
                else if (_comp->comps[i]->constraints.gridw<=0&&cur_c<=0)cur_r=cur_y+cur_h;
                _this->cache[i].x=cur_x;
                _this->cache[i].y=cur_y;
                _this->cache[i].w=cur_w;
                _this->cache[i].h=cur_h;
            }
        _this->min_w.resize(_this->w);
        _this->min_h.resize(_this->h);
        _this->pre_w.resize(_this->w);
        _this->pre_h.resize(_this->h);
        _this->weight_x.resize(_this->w);
        _this->weight_y.resize(_this->h);
        /*Pass 3: distribute weights*/
        next=INT_MAX;
        for (i=1; i!=INT_MAX; i=(size_t)next,next=INT_MAX) {
            size_t j;
            for (j=0; j<_comp->comps.size(); j++)if (glwCompIsVisible(_comp->comps[j])) {
                    if ((size_t)_this->cache[j].w==i) {
                        int    k;
                        int    px;
                        double weightd;
                        double tweight;
                        px=_this->cache[j].x+_this->cache[j].w;
                        weightd=_comp->comps[j]->constraints.weightx;
                        tweight=0;
                        for (k=_this->cache[j].x; k<px; k++)tweight+=_this->weight_x[k];
                        weightd-=tweight;
                        if (weightd>0) {
                            double weight;
                            weight=tweight;
                            for (k=_this->cache[j].x; weight>0&&k<px; k++) {
                                double wt=_this->weight_x[k];
                                double dx=(wt*weightd)/weight;
                                _this->weight_x[k]+=dx;
                                weightd-=dx;
                                weight-=wt;
                            }
                            _this->weight_x[px-1]+=weightd;
                        }
                    } else if ((size_t)_this->cache[j].w>i&&_this->cache[j].w<next) {
                        next=_this->cache[j].w;
                    }
                    if ((size_t)_this->cache[j].h==i) {
                        int    k;
                        int    py;
                        double weightd;
                        double tweight;
                        py=_this->cache[j].y+_this->cache[j].h;
                        weightd=_comp->comps[j]->constraints.weighty;
                        tweight=0;
                        for (k=_this->cache[j].y; k<py; k++)tweight+=_this->weight_y[k];
                        weightd-=tweight;
                        if (weightd>0) {
                            double weight;
                            weight=tweight;
                            for (k=_this->cache[j].y; weight>0&&k<py; k++) {
                                double wt=_this->weight_y[k];
                                double dy=(wt*weightd)/weight;
                                _this->weight_y[k]+=dy;
                                weightd-=dy;
                                weight-=wt;
                            }
                            _this->weight_y[py-1]+=weightd;
                        }
                    } else if ((size_t)_this->cache[j].h>i&&_this->cache[j].h<next) {
                        next=_this->cache[j].h;
                    }
                }
        }
        /*Pass 4: distribute minimum widths and heights*/
        next=INT_MAX;
        for (i=1; i!=INT_MAX; i=(size_t)next,next=INT_MAX) {
            size_t j;
            for (j=0; j<_comp->comps.size(); j++)if (glwCompIsVisible(_comp->comps[j])) {
                    if ((size_t)_this->cache[j].w==i) {
                        int    k;
                        int    px;
                        int    pixeld;
                        double tweight;
                        px=_this->cache[j].x+_this->cache[j].w;
                        tweight=0;
                        for (k=_this->cache[j].x; k<px; k++)tweight+=_this->weight_x[k];
                        pixeld=_this->cache[j].min_w+_comp->comps[j]->constraints.insets.l+
                               _comp->comps[j]->constraints.insets.r;
                        for (k=_this->cache[j].x; k<px; k++)pixeld-=_this->min_w[k];
                        if (pixeld>0) {
                            double weight;
                            weight=tweight;
                            for (k=_this->cache[j].x; weight>0&&k<px; k++) {
                                double wt=_this->weight_x[k];
                                int    dx=(int)(wt*(double)pixeld/weight);
                                _this->min_w[k]+=dx;
                                pixeld-=dx;
                                weight-=wt;
                            }
                            _this->min_w[px-1]+=pixeld;
                        }
                        pixeld=_this->cache[j].pre_w+_comp->comps[j]->constraints.insets.l+
                               _comp->comps[j]->constraints.insets.r;
                        for (k=_this->cache[j].x; k<px; k++)pixeld-=_this->pre_w[k];
                        if (pixeld>0) {
                            double weight;
                            weight=tweight;
                            for (k=_this->cache[j].x; weight>0&&k<px; k++) {
                                double wt=_this->weight_x[k];
                                int    dx=(int)(wt*(double)pixeld/weight);
                                _this->pre_w[k]+=dx;
                                pixeld-=dx;
                                weight-=wt;
                            }
                            _this->pre_w[px-1]+=pixeld;
                        }
                    } else if ((size_t)_this->cache[j].w>i&&_this->cache[j].w<next) {
                        next=_this->cache[j].w;
                    }
                    if ((size_t)_this->cache[j].h==i) {
                        int    k;
                        int    py;
                        int    pixeld;
                        double tweight;
                        py=_this->cache[j].y+_this->cache[j].h;
                        tweight=0;
                        for (k=_this->cache[j].y; k<py; k++)tweight+=_this->weight_y[k];
                        pixeld=_this->cache[j].min_h+_comp->comps[j]->constraints.insets.b+
                               _comp->comps[j]->constraints.insets.t;
                        for (k=_this->cache[j].y; k<py; k++)pixeld-=_this->min_h[k];
                        if (pixeld>0) {
                            double weight;
                            weight=tweight;
                            for (k=_this->cache[j].y; weight>0&&k<py; k++) {
                                double wt=_this->weight_y[k];
                                int    dy=(int)(wt*(double)pixeld/weight);
                                _this->min_h[k]+=dy;
                                pixeld-=dy;
                                weight-=wt;
                            }
                            _this->min_h[py-1]+=pixeld;
                        }
                        pixeld=_this->cache[j].pre_h+_comp->comps[j]->constraints.insets.b+
                               _comp->comps[j]->constraints.insets.t;
                        for (k=_this->cache[j].y; k<py; k++)pixeld-=_this->pre_h[k];
                        if (pixeld>0) {
                            double weight;
                            weight=tweight;
                            for (k=_this->cache[j].y; weight>0&&k<py; k++) {
                                double wt=_this->weight_y[k];
                                int    dy=(int)(wt*(double)pixeld/weight);
                                _this->pre_h[k]+=dy;
                                pixeld-=dy;
                                weight-=wt;
                            }
                            _this->pre_h[py-1]+=pixeld;
                        }
                    } else if ((size_t)_this->cache[j].h>i&&_this->cache[j].h<next) {
                        next=_this->cache[j].h;
                    }
                }
        }
        /*Pass 5: compress minimum widths and heights*/
        xs_w = &y_max[0];
        xs_h = &x_max[0];
        for (;;) {
            size_t j;
            int    k;
            for (k=0; k<_this->w; k++)xs_w[k]=-1;
            for (j=0; j<_comp->comps.size(); j++)if (glwCompIsVisible(_comp->comps[j])) {
                    int px;
                    int pixeld;
                    px=_this->cache[j].x+_this->cache[j].w;
                    pixeld=-_this->cache[j].min_w-_comp->comps[j]->constraints.insets.l-
                           _comp->comps[j]->constraints.insets.r;
                    for (k=_this->cache[j].x; k<px; k++)pixeld+=_this->min_w[k];
                    if (pixeld>=0)for (k=_this->cache[j].x; k<px; k++) {
                            if (xs_w[k]<0||xs_w[k]>pixeld)xs_w[k]=pixeld;
                        }
                }
            for (k=0; k<_this->w; k++)if (xs_w[k]>0) {
                    _this->min_w[k]-=xs_w[k];
                    break;
                }
            if (k==_this->w)break;
        }
        for (;;) {
            size_t j;
            int    k;
            for (k=0; k<_this->h; k++)xs_h[k]=-1;
            for (j=0; j<_comp->comps.size(); j++)if (glwCompIsVisible(_comp->comps[j])) {
                    int py;
                    int pixeld;
                    py=_this->cache[j].y+_this->cache[j].h;
                    pixeld=-_this->cache[j].min_h-_comp->comps[j]->constraints.insets.t-
                           _comp->comps[j]->constraints.insets.b;
                    for (k=_this->cache[j].y; k<py; k++)pixeld+=_this->min_h[k];
                    if (pixeld>=0)for (k=_this->cache[j].y; k<py; k++) {
                            if (xs_h[k]<0||xs_h[k]>pixeld)xs_h[k]=pixeld;
                        }
                }
            for (k=0; k<_this->h; k++)if (xs_h[k]>0) {
                    _this->min_h[k]-=xs_h[k];
                    break;
                }
            if (k==_this->h)break;
        }
        for (;;) {
            size_t j;
            int    k;
            for (k=0; k<_this->w; k++)xs_w[k]=-1;
            for (j=0; j<_comp->comps.size(); j++)if (glwCompIsVisible(_comp->comps[j])) {
                    int px;
                    int pixeld;
                    px=_this->cache[j].x+_this->cache[j].w;
                    pixeld=-_this->cache[j].pre_w-_comp->comps[j]->constraints.insets.l-
                           _comp->comps[j]->constraints.insets.r;
                    for (k=_this->cache[j].x; k<px; k++)pixeld+=_this->pre_w[k];
                    if (pixeld>=0)for (k=_this->cache[j].x; k<px; k++) {
                            if (xs_w[k]<0||xs_w[k]>pixeld)xs_w[k]=pixeld;
                        }
                }
            for (k=0; k<_this->w; k++)if (xs_w[k]>0) {
                    _this->pre_w[k]-=xs_w[k];
                    break;
                }
            if (k==_this->w)break;
        }
        for (;;) {
            size_t j;
            int    k;
            for (k=0; k<_this->h; k++)xs_h[k]=-1;
            for (j=0; j<_comp->comps.size(); j++)if (glwCompIsVisible(_comp->comps[j])) {
                    int py;
                    int pixeld;
                    py=_this->cache[j].y+_this->cache[j].h;
                    pixeld=-_this->cache[j].pre_h-_comp->comps[j]->constraints.insets.t-
                           _comp->comps[j]->constraints.insets.b;;
                    for (k=_this->cache[j].y; k<py; k++)pixeld+=_this->pre_h[k];
                    if (pixeld>=0)for (k=_this->cache[j].y; k<py; k++) {
                            if (xs_h[k]<0||xs_h[k]>pixeld)xs_h[k]=pixeld;
                        }
                }
            for (k=0; k<_this->h; k++)if (xs_h[k]>0) {
                    _this->pre_h[k]-=xs_h[k];
                    break;
                }
            if (k==_this->h)break;
        }
        _this->validating=0;
        _this->valid=1;
        return 1;
    }
    return 1;
}

static void glwGBLMinSize(GLWGridBagLayout *_this,GLWComponent *_comp,
                          int *_w,int *_h)
{
    if (glwGBLCalcLayout(_this,_comp)) {
        if (_w!=NULL) {
            int i;
            *_w=0;
            for (i=0; i<_this->w; i++)*_w+=_this->min_w[i];
        }
        if (_h!=NULL) {
            int i;
            *_h=0;
            for (i=0; i<_this->h; i++)*_h+=_this->min_h[i];
        }
    } else {
        if (_w!=NULL)*_w=-1;
        if (_h!=NULL)*_h=-1;
    }
}

static void glwGBLPreSize(GLWGridBagLayout *_this,GLWComponent *_comp,
                          int *_w,int *_h)
{
    if (glwGBLCalcLayout(_this,_comp)) {
        if (_w!=NULL) {
            int i;
            *_w=0;
            for (i=0; i<_this->w; i++)*_w+=_this->pre_w[i];
        }
        if (_h!=NULL) {
            int i;
            *_h=0;
            for (i=0; i<_this->h; i++)*_h+=_this->pre_h[i];
        }
    } else {
        if (_w!=NULL)*_w=-1;
        if (_h!=NULL)*_h=-1;
    }
}

static void glwGBLAdjust(GLWGridBagLayout *_this,GLWComponent *_comp,int _i,
                         int *_x,int *_y,int *_w,int *_h)
{
    GLWConstraints *c;
    c = &_comp->comps[_i]->constraints;
    *_x+=c->insets.l;
    *_w-=c->insets.l+c->insets.r;
    *_y+=c->insets.b;
    *_h-=c->insets.b+c->insets.t;
    if (!(c->fill&GLWC_HORIZONTAL)&&*_w>_this->cache[_i].pre_w) {
        *_x+=(int)((*_w-_this->cache[_i].pre_w)*c->alignx);
        *_w=_this->cache[_i].pre_w;
    } else if (_this->cache[_i].max_w>=0&&*_w>_this->cache[_i].max_w) {
        *_x+=(int)((*_w-_this->cache[_i].max_w)*c->alignx);
        *_w=_this->cache[_i].max_w;
    }
    if (!(c->fill&GLWC_VERTICAL)&&*_h>_this->cache[_i].pre_h) {
        *_y+=(int)((*_h-_this->cache[_i].pre_h)*c->aligny);
        *_h=_this->cache[_i].pre_h;
    } else if (_this->cache[_i].max_h>=0&&*_h>_this->cache[_i].max_h) {
        *_y+=(int)((*_h-_this->cache[_i].max_h)*c->aligny);
        *_h=_this->cache[_i].max_h;
    }
}

static void glwGBLLayout(GLWGridBagLayout *_this,GLWComponent *_comp)
{
    if (_comp->comps.size()) {
        int            i;
        int            w;
        int            h;
        int            dw;
        int            dh;
	std::vector<int> min_w;
	std::vector<int> min_h;
        glwGBLPreSize(_this,_comp,&w,&h);
        if (w>_comp->bounds.w||h>_comp->bounds.h) {
            glwGBLMinSize(_this,_comp,&w,&h);
            min_w=_this->min_w;
            min_h=_this->min_h;
        } else {
            min_w=_this->pre_w;
            min_h=_this->pre_h;
        }
        dw=_comp->bounds.w-w;
        if (dw) {
            double weight;
            if (_this->adj_w.empty()) {
                _this->adj_w.resize(_this->w);
            }
            weight=0;
            for (i=0; i<_this->w; i++)weight+=_this->weight_x[i];
            if (weight>0) {
                for (i=0; i<_this->w; i++) {
                    int dx=(int)((double)dw*_this->weight_x[i]/weight);
                    if (min_w[i]+dx<0)dx=-min_w[i];
                    _this->adj_w[i]=dx;
                    w+=dx;
                }
            } else if (_this->w>0) {
                int dx=dw/_this->w;
                for (i=0; i<_this->w; i++) {
                    if (min_w[i]+dx<0)dx=-min_w[i];
                    _this->adj_w[i]=dx;
                    w+=dx;
                }
            }
            dw=_comp->bounds.w-w;
        }
        dh=_comp->bounds.h-h;
        if (dh) {
            double weight;
            if (_this->adj_h.empty()) {
                _this->adj_h.resize(_this->h);
            }
            weight=0;
            for (i=0; i<_this->h; i++)weight+=_this->weight_y[i];
            if (weight>0) {
                for (i=0; i<_this->h; i++) {
                    int dy=(int)((double)dh*_this->weight_y[i]/weight);
                    if (min_h[i]+dy<0)dy=-min_h[i];
                    _this->adj_h[i]=dy;
                    h+=dy;
                }
            } else if (_this->h>0) {
                int dy=dh/_this->h;
                for (i=0; i<_this->h; i++) {
                    if (min_h[i]+dy<0)dy=-min_h[i];
                    _this->adj_h[i]=dy;
                    h+=dy;
                }
            }
            dh=_comp->bounds.h-h;
        }
        _this->start_x=(int)(dw*_comp->constraints.alignx);
        _this->start_y=_comp->bounds.h-(int)(dh*(1-_comp->constraints.aligny));
        for (i=0; (size_t)i<_comp->comps.size(); i++)if (glwCompIsVisible(_comp->comps[i])) {
                int j;
                int cx;
                int cy;
                int cw;
                int ch;
                cx=_this->start_x;
                for (j=0; j<_this->cache[i].x; j++) {
                    cx+=min_w[j];
                    if (!_this->adj_w.empty()) cx += _this->adj_w[j];
                }
                cy=_this->start_y;
                for (j=0; j<_this->cache[i].y; j++) {
                    cy-=min_h[j];
                    if (!_this->adj_h.empty()) cy -= _this->adj_h[j];
                }
                cw=0;
                for (j=_this->cache[i].x; j<_this->cache[i].x+_this->cache[i].w; j++) {
                    cw+=min_w[j];
                    if (!_this->adj_w.empty()) cw += _this->adj_w[j];
                }
                ch=0;
                for (j=_this->cache[i].y; j<_this->cache[i].y+_this->cache[i].h; j++) {
                    ch+=min_h[j];
                    if (!_this->adj_h.empty()) ch += _this->adj_h[j];
                }
                cy-=ch;
                glwGBLAdjust(_this,_comp,i,&cx,&cy,&cw,&ch);
                if (cw<0||ch<0)glwCompSetBounds(_comp->comps[i],0,0,0,0);
                else glwCompSetBounds(_comp->comps[i],cx,cy,cw,ch);
            }
    }
}


GLWGridBagLayout::GLWGridBagLayout()
{
    this->super.layout = (GLWLayoutFunc)glwGBLLayout;
    this->super.invalidate = (GLWLayoutFunc)glwGBLInvalidate;
    this->super.min_size = (GLWLayoutSizeFunc)glwGBLMinSize;
    this->super.pre_size = (GLWLayoutSizeFunc)glwGBLPreSize;
    this->super.max_size = NULL;
    this->comp = NULL;
    this->valid = 0;
    this->validating = 0;
}

GLWGridBagLayout::~GLWGridBagLayout()
{
    glwGBLInvalidate(this, NULL);
}

