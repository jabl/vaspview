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
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glw.hh"

/*A tabbed pane component. Instead of adding children directly, components are
  added as tabs. Only the currently active tab's component is ever actually a
  child of this component. If the currently active tab does not have a child,
  whatever component was last active remains in the pane.*/

# define GLW_TABBED_PANE_INSET   (4)
# define GLW_TABBED_PANE_OVERLAY (2)


static int glwTabbedPaneCalcMaxTabHeight(GLWTabbedPane *_this)
{
    return glwFontGetHeight(_this->super.font)+(GLW_TABBED_PANE_INSET<<1);
}

static int glwTabbedPaneCalcTabAreaHeight(GLWTabbedPane *_this,int _cols)
{
	if (_cols<0)_cols=(int)_this->runs.size();
    if (_cols)
    {
        return (_this->max_tab_h-GLW_TABBED_PANE_OVERLAY)*_cols+
               GLW_TABBED_PANE_OVERLAY;
    }
    return 0;
}

static int glwTabbedPaneCalcMaxTabWidth(GLWTabbedPane *_this)
{
	int w = 0;
	for (auto it = _this->tabs.begin(); it != _this->tabs.end(); ++it)
	{
		GLWTabPage& tp = *it;
		int tw = glwFontGetStringWidth(_this->super.font, 
					       tp.title.c_str());
		if (tw > w) w = tw;
	}
	w += GLW_TABBED_PANE_INSET<<1;
	return w;
}

static int glwTabbedPaneCalcTabAreaWidth(GLWTabbedPane *_this,int _rows)
{
	if (_rows<0)_rows=(int)_this->runs.size();
    if (_rows)
    {
        return (_this->max_tab_w-GLW_TABBED_PANE_OVERLAY)*_rows+
               GLW_TABBED_PANE_OVERLAY;
    }
    return 0;
}

static void glwTabbedPaneLayout(GLWLayoutManager *_this,
                                GLWTabbedPane *_tabpn)
{
    GLWComponent **children;
    GLWComponent  *selc;
    GLWTabPage    *tabs;
    GLWRect       *tabr;
    int            i;
    int            x;
    int            y;
    int            r;
    int            vert;
    if (!_tabpn->max_tab_w)_tabpn->max_tab_w=glwTabbedPaneCalcMaxTabWidth(_tabpn);
    if (!_tabpn->max_tab_h)
    {
        _tabpn->max_tab_h=glwTabbedPaneCalcMaxTabHeight(_tabpn);
    }
    if (!_tabpn->tabs.size()||_tabpn->valid)return;
    vert=_tabpn->tplc==GLWC_WEST||_tabpn->tplc==GLWC_EAST;
    switch (_tabpn->tplc)
    {
    case GLWC_WEST :
    {
        x=0;
        y=_tabpn->super.bounds.h-_tabpn->max_tab_h;
        r=0;
    }
    break;
    case GLWC_EAST :
    {
        x=_tabpn->super.bounds.w-_tabpn->max_tab_w;
        y=_tabpn->super.bounds.h-_tabpn->max_tab_h;
        r=0;
    }
    break;
    case GLWC_SOUTH:
    {
        x=0;
        y=0;
        r=_tabpn->super.bounds.w;
    }
    break;
    /*case GLWC_NORTH:*/
    default        :
    {
        x=0;
        y=_tabpn->super.bounds.h-_tabpn->max_tab_h;
        r=_tabpn->super.bounds.w;
    }
    }
    _tabpn->runs.clear();
    tabs = &_tabpn->tabs[0];
    for (i=0; (size_t)i<_tabpn->tabs.size(); i++)
    {
        tabr=&tabs[i].bounds;
        if (vert)
        {
            tabr->w=_tabpn->max_tab_w;
            tabr->h=_tabpn->max_tab_h;
            tabr->x=x;
            if (i>0)
            {
                tabr->y=tabs[i-1].bounds.y-tabr->h;
                if (tabr->y<r)
                {
			_tabpn->runs.push_back(i);
			tabr->y=y;
                }
            }
            else
            {
		    _tabpn->runs.push_back(i);
		    tabr->y=y;
            }
        }
        else
        {
            tabr->w=glwFontGetStringWidth(_tabpn->super.font,
                                          tabs[i].title.c_str())
                    + (GLW_TABBED_PANE_INSET<<1);
            tabr->h=_tabpn->max_tab_h;
            if (i>0)
            {
                tabr->x=tabs[i-1].bounds.x+tabs[i-1].bounds.w;
                if (tabr->x+tabr->w>r)
                {
			_tabpn->runs.push_back(i);
			tabr->x=x;
                }
            }
            else
            {
		    _tabpn->runs.push_back(i);
		    tabr->x=x;
            }
            tabr->y=y;
        }
        tabs[i].run = (int)_tabpn->runs.size() - 1;
    }
    if (_tabpn->runs.size() > 1)
    {
        double weight;
        weight=1.25;
        for (i = (int)_tabpn->runs.size() - 1; ;)
        {
            int last;
            last=((size_t)i+1<_tabpn->runs.size()?_tabpn->runs[i+1]:
                  (int)_tabpn->tabs.size())-1;
            if (vert)
            {
                if (tabs[last].bounds.y-r>_tabpn->max_tab_h*weight*2)
                {
                    int j;
                    j = --_tabpn->runs[i];
                    tabs[j].bounds.y=y;
                    tabs[j].run=i;
                    for (j++; j<=last; j++)
                    {
                        tabs[j].bounds.y=tabs[j-1].bounds.y-tabs[j].bounds.h;
                    }
                }
                else break;
            }
            else
            {
                if (r-tabs[last].bounds.x-tabs[last].bounds.h>_tabpn->max_tab_w*weight)
                {
                    int j;
                    j = --_tabpn->runs[i];
                    tabs[j].bounds.x=x;
                    tabs[j].run=i;
                    for (j++; j<=last; j++)
                    {
                        tabs[j].bounds.x=tabs[j-1].bounds.x+tabs[j-1].bounds.w;
                    }
                }
                else break;
            }
            if (i>1)i--;
            else
            {
                weight+=0.25;
                i = (int)_tabpn->runs.size() - 1;
            }
        }
        if (_tabpn->seld>=0&&tabs[_tabpn->seld].run!=0)
        {
            for (i=0; i<tabs[_tabpn->seld].run; i++)
            {
                size_t j;
                int    t;
                t = _tabpn->runs[0];
                for (j=1; j < _tabpn->runs.size(); j++)
                {
                    _tabpn->runs[j-1] = _tabpn->runs[j];
                }
                _tabpn->runs[_tabpn->runs.size() - 1] = t;
            }
        }
    }
    for (i=(int)_tabpn->runs.size(); i-->0;)
    {
        int start;
        int end;
        start = _tabpn->runs[i];
        end = _tabpn->runs[i + 1 < _tabpn->runs.size() ? i + 1 : 0];
        if (!end)end=_tabpn->tabs.size();
        if (vert)
        {
            int j;
            for (j=start; j<end; j++)
            {
                tabs[j].bounds.x=x;
                tabs[j].run=i;
            }
            if (_tabpn->runs.size()>1)
            {
                int h;
                h=tabs[start].bounds.y+tabs[start].bounds.h-tabs[end-1].bounds.y;
                if (h>0)
                {
                    double dh;
                    dh=(tabs[end-1].bounds.y-r)/(double)h;
                    for (j=end; j-->start;)
                    {
                        if (j+1<end)tabs[j].bounds.y=tabs[j+1].bounds.y+tabs[j+1].bounds.h;
                        else tabs[j].bounds.y=r;
                        tabs[j].bounds.h+=(int)(tabs[j].bounds.h*dh+0.5);
                    }
                    tabs[start].bounds.h=y+_tabpn->max_tab_h-tabs[start].bounds.y;
                }
            }
            if (_tabpn->tplc==GLWC_EAST)x-=_tabpn->max_tab_w-GLW_TABBED_PANE_OVERLAY;
            else x+=_tabpn->max_tab_w-GLW_TABBED_PANE_OVERLAY;
        }
        else
        {
            int j;
            for (j=start; j<end; j++)
            {
                tabs[j].bounds.y=y;
                tabs[j].run=i;
            }
            if (_tabpn->runs.size() > 1)
            {
                int w;
                w=tabs[end-1].bounds.x+tabs[end-1].bounds.w-tabs[start].bounds.x;
                if (w>0)
                {
                    double dw;
                    dw=(r-w-tabs[start].bounds.x)/(double)w;
                    for (j=start; j<end; j++)
                    {
                        if (j>start)tabs[j].bounds.x=tabs[j-1].bounds.x+tabs[j-1].bounds.w;
                        tabs[j].bounds.w+=(int)(tabs[j].bounds.w*dw+0.5);
                    }
                    tabs[end-1].bounds.w=r-tabs[end-1].bounds.x;
                }
            }
            if (_tabpn->tplc==GLWC_SOUTH)y+=_tabpn->max_tab_h-GLW_TABBED_PANE_OVERLAY;
            else y-=_tabpn->max_tab_h-GLW_TABBED_PANE_OVERLAY;
        }
    }
    if (_tabpn->seld>=0)
    {
        tabr=&tabs[_tabpn->seld].bounds;
        if (tabr->x>GLW_TABBED_PANE_INSET>>1)
        {
            tabr->x-=GLW_TABBED_PANE_INSET>>1;
            tabr->w+=GLW_TABBED_PANE_INSET>>1;
        }
        if (tabr->x+tabr->w+(GLW_TABBED_PANE_INSET>>1)<_tabpn->super.bounds.w)
        {
            tabr->w+=GLW_TABBED_PANE_INSET>>1;
        }
        if (tabr->y>GLW_TABBED_PANE_INSET>>1)
        {
            tabr->y-=GLW_TABBED_PANE_INSET>>1;
            tabr->h+=GLW_TABBED_PANE_INSET>>1;
        }
        if (tabr->y+tabr->h+(GLW_TABBED_PANE_INSET>>1)<_tabpn->super.bounds.h)
        {
            tabr->h+=GLW_TABBED_PANE_INSET>>1;
        }
    }
    switch (_tabpn->tplc)
    {
    case GLWC_WEST :
    {
        _tabpn->area.x=glwTabbedPaneCalcTabAreaWidth(_tabpn,-1);
        _tabpn->area.y=0;
        _tabpn->area.w=_tabpn->super.bounds.w-_tabpn->area.x;
        _tabpn->area.h=_tabpn->super.bounds.h;
    }
    break;
    case GLWC_EAST :
    {
        _tabpn->area.x=0;
        _tabpn->area.y=0;
        _tabpn->area.w=_tabpn->super.bounds.w-
                       glwTabbedPaneCalcTabAreaWidth(_tabpn,-1);
        _tabpn->area.h=_tabpn->super.bounds.h;
    }
    break;
    case GLWC_SOUTH:
    {
        _tabpn->area.x=0;
        _tabpn->area.y=glwTabbedPaneCalcTabAreaHeight(_tabpn,-1);
        _tabpn->area.w=_tabpn->super.bounds.w;
        _tabpn->area.h=_tabpn->super.bounds.h-_tabpn->area.y;
    }
    break;
    /*case GLWC_NORTH:*/
    default        :
    {
        _tabpn->area.x=0;
        _tabpn->area.y=0;
        _tabpn->area.w=_tabpn->super.bounds.w;
        _tabpn->area.h=_tabpn->super.bounds.h-
                       glwTabbedPaneCalcTabAreaHeight(_tabpn,-1);
    }
    }
    children=_DAGetAt(&_tabpn->super.comps,0,GLWComponent *);
    if (_tabpn->seld>=0)selc=tabs[_tabpn->seld].comp;
    else selc=NULL;
    for (i=(int)_tabpn->super.comps.size; i-->0;)
    {
        GLWConstraints *c;
        int             x;
        int             y;
        int             w;
        int             h;
        int             pre_w;
        int             pre_h;
        int             max_w;
        int             max_h;
        if (children[i]==selc)glwCompVisibility(children[i],1);
        else if (selc!=NULL)glwCompVisibility(children[i],0);
        c=&children[i]->constraints;
        x=_tabpn->area.x+GLW_TABBED_PANE_INSET;
        y=_tabpn->area.y+GLW_TABBED_PANE_INSET;
        w=_tabpn->area.w-(GLW_TABBED_PANE_INSET<<1);
        h=_tabpn->area.h-(GLW_TABBED_PANE_INSET<<1);
        x+=c->insets.l;
        y+=c->insets.b;
        w-=c->insets.l+c->insets.r;
        h-=c->insets.b+c->insets.t;
        glwCompGetPreSize(children[i],&pre_w,&pre_h);
        glwCompGetMaxSize(children[i],&max_w,&max_h);
        if (!(c->fill&GLWC_HORIZONTAL)&&w>pre_w)
        {
            x+=(int)((w-pre_w)*c->alignx);
            w=pre_w;
        }
        else if (max_w>=0&&w>max_w)
        {
            x+=(int)((w-max_w)*c->alignx);
            w=max_w;
        }
        if (!(c->fill&GLWC_VERTICAL)&&h>pre_h)
        {
            y+=(int)((h-pre_h)*c->aligny);
            h=pre_h;
        }
        else if (max_h>=0&&h>max_h)
        {
            y+=(int)((h-max_h)*c->aligny);
            h=max_h;
        }
        glwCompSetBounds(children[i],x,y,w,h);
    }
    _tabpn->valid=1;
}

static void glwTabbedPaneLayoutInvalidate(GLWLayoutManager *_this,
        GLWTabbedPane *_tabpn)
{
    _tabpn->valid=0;
    _tabpn->max_tab_w=0;
    _tabpn->max_tab_h=0;
}

static void glwTabbedPaneLayoutSize(GLWLayoutManager *_this,
                                    GLWTabbedPane *_tabpn,int *_w,int *_h,
                                    int _min)
{
    GLWTabPage *tabs;
    size_t      i;
    int         w;
    int         h;
    w=h=0;
    tabs = &_tabpn->tabs[0];
    for (i=_tabpn->tabs.size(); i-->0;)
    {
        int tw;
        int th;
        if (tabs[i].comp!=NULL)
        {
            if (_min)
            {
                glwCompGetMinSize(tabs[i].comp,_w==NULL?NULL:&tw,_h==NULL?NULL:&th);
            }
            else
            {
                glwCompGetPreSize(tabs[i].comp,_w==NULL?NULL:&tw,_h==NULL?NULL:&th);
            }
            tw+=tabs[i].comp->constraints.insets.l+tabs[i].comp->constraints.insets.r;
            th+=tabs[i].comp->constraints.insets.t+tabs[i].comp->constraints.insets.b;
            if (w<tw)w=tw;
            if (h<th)h=th;
        }
    }
    w+=GLW_TABBED_PANE_INSET<<1;
    h+=GLW_TABBED_PANE_INSET<<1;
    switch (_tabpn->tplc)
    {
    case GLWC_WEST:
    case GLWC_EAST:
	if (_tabpn->tabs.size() > 0)
        {
            if (!_tabpn->max_tab_h)
            {
                _tabpn->max_tab_h=glwTabbedPaneCalcMaxTabHeight(_tabpn);
            }
            if (_tabpn->max_tab_h>h)h=_tabpn->max_tab_h;
            if (_w!=NULL)
            {
                int y;
                int cols;
                y=h/_tabpn->max_tab_h;
                cols = ((int)_tabpn->tabs.size() + y - 1) / y;
                if (!_tabpn->max_tab_w)
                {
                    _tabpn->max_tab_w=glwTabbedPaneCalcMaxTabWidth(_tabpn);
                }
                w+=glwTabbedPaneCalcTabAreaWidth(_tabpn,cols);
            }
        }
        break;
        /*case GLWC_SOUTH:*/
        /*case GLWC_NORTH:*/
    default       :
	if (_tabpn->tabs.size() > 0)
        {
            int rows;
            int x;
            x=0;
            rows=1;
            for (i=0; i < _tabpn->tabs.size(); i++)
            {
                int tw;
                tw=glwFontGetStringWidth(_tabpn->super.font,
                                         tabs[i].title.c_str()) +
                   (GLW_TABBED_PANE_INSET<<1);
                if (tw>w)w=tw;
                if (x+tw>w)
                {
                    rows++;
                    x=tw;
                }
                else x+=tw;
            }
            if (_h!=NULL)
            {
                if (!_tabpn->max_tab_h)
                {
                    _tabpn->max_tab_h=glwTabbedPaneCalcMaxTabHeight(_tabpn);
                }
                h+=glwTabbedPaneCalcTabAreaHeight(_tabpn,rows);
            }
        }
    }
    if (_w!=NULL)*_w=w;
    if (_h!=NULL)*_h=h;
}

static void glwTabbedPaneLayoutMinSize(GLWLayoutManager *_this,
                                       GLWTabbedPane *_tabpn,int *_w,int *_h)
{
    glwTabbedPaneLayoutSize(_this,_tabpn,_w,_h,1);
}

static void glwTabbedPaneLayoutPreSize(GLWLayoutManager *_this,
                                       GLWTabbedPane *_tabpn,int *_w,int *_h)
{
    glwTabbedPaneLayoutSize(_this,_tabpn,_w,_h,0);
}

static GLWLayoutManager glw_tabbed_pane_layout=
{
    (GLWLayoutFunc)glwTabbedPaneLayout,
    (GLWLayoutFunc)glwTabbedPaneLayoutInvalidate,
    (GLWLayoutSizeFunc)glwTabbedPaneLayoutMinSize,
    (GLWLayoutSizeFunc)glwTabbedPaneLayoutPreSize,
    NULL
};

static int glwTabbedPaneGetTabAt(GLWTabbedPane *_this,int _x,int _y)
{
    GLWTabPage *tabs;
    size_t      i;
    tabs = &_this->tabs[0];
    for (i=0; i<_this->tabs.size(); i++)
    {
        if (tabs[i].bounds.x<=_x&&_x<tabs[i].bounds.x+tabs[i].bounds.w&&
                tabs[i].bounds.y<=_y&&_y<tabs[i].bounds.y+tabs[i].bounds.h)
        {
            return (int)i;
        }
    }
    return -1;
}

static void glwTabbedPaneSelNext(GLWTabbedPane *_this)
{
    GLWTabPage *tabs;
    int         i;
    int         j;
    i = _this->seld < 0 ? (int)_this->tabs.size() : _this->seld;
    j=i;
    tabs = &_this->tabs[0];
    for (;;)
    {
        if (++j==i)break;
        if (j >= (int)_this->tabs.size()) j -= (int)_this->tabs.size();
        if (j==i)break;
        if (tabs[j].comp==NULL||glwCompIsEnabled(tabs[j].comp))
        {
            glwTabbedPaneSetSelectedIdx(_this,j);
            break;
        }
    }
}

static void glwTabbedPaneSelPrev(GLWTabbedPane *_this)
{
    GLWTabPage *tabs;
    int         i;
    int         j;
    i=_this->seld<0?-1:_this->seld;
    j=i;
    tabs = &_this->tabs[0];
    for (;;)
    {
        j--;
        if (j==i)break;
        if (j<0)j+=(int)_this->tabs.size();
        if (j==i)break;
        if (tabs[j].comp==NULL||glwCompIsEnabled(tabs[j].comp))
        {
            glwTabbedPaneSetSelectedIdx(_this,j);
            break;
        }
    }
}

static void glwTabbedPaneSelAdj(GLWTabbedPane *_this,int _fwd)
{
    GLWTabPage *tabs;
    int         run;
    int         dy;
    int         dx;
    int         x;
    int         y;
    int         i;
    int         j;
    tabs = &_this->tabs[0];
    if (_this->seld<0)
    {
        run=0;
        x=y=0;
    }
    else
    {
        run=tabs[_this->seld].run;
        x=tabs[_this->seld].bounds.x+(tabs[_this->seld].bounds.w>>1);
        y=tabs[_this->seld].bounds.y+(tabs[_this->seld].bounds.h>>1);
    }
    switch (_this->tplc)
    {
    case GLWC_WEST :
    {
        dy=0;
        if (run<=0)
        {
            dx=_fwd?_this->max_tab_w-glwTabbedPaneCalcTabAreaWidth(_this,-1):
               -_this->max_tab_w;
        }
        else if ((size_t)run + 1 >= _this->runs.size())
        {
            dx=_fwd?_this->max_tab_w:
               glwTabbedPaneCalcTabAreaWidth(_this,-1)-_this->max_tab_w;
        }
        else dx=_fwd?_this->max_tab_w:-_this->max_tab_w;
    }
    break;
    case GLWC_EAST :
    {
        dy=0;
        if (run<=0)
        {
            dx=_fwd?_this->max_tab_w:
               glwTabbedPaneCalcTabAreaWidth(_this,-1)-_this->max_tab_w;
        }
        else if ((size_t)run+1 >= _this->runs.size())
        {
            dx=_fwd?_this->max_tab_w-glwTabbedPaneCalcTabAreaWidth(_this,-1):
               -_this->max_tab_w;
        }
        else dx=_fwd?_this->max_tab_w:-_this->max_tab_w;
    }
    break;
    case GLWC_SOUTH:
    {
        dx=0;
        if (run<=0)
        {
            dy=_fwd?-_this->max_tab_h:
               _this->max_tab_h-glwTabbedPaneCalcTabAreaHeight(_this,-1);
        }
        else if ((size_t)run+1 >= _this->runs.size())
        {
            dy=_fwd?_this->max_tab_h:
               glwTabbedPaneCalcTabAreaHeight(_this,-1)-_this->max_tab_h;
        }
        else dy=_fwd?-_this->max_tab_h:_this->max_tab_h;
    }
    break;
    /*case GLWC_NORTH:*/
    default        :
    {
        dx=0;
        if (run<=0)
        {
            dy=_fwd?_this->max_tab_h:
               glwTabbedPaneCalcTabAreaHeight(_this,-1)-_this->max_tab_h;
        }
        else if ((size_t)run + 1 >= _this->runs.size())
        {
            dy=_fwd?-_this->max_tab_h:
               _this->max_tab_h-glwTabbedPaneCalcTabAreaHeight(_this,-1);
        }
        else dy=_fwd?-_this->max_tab_h:_this->max_tab_h;
    }
    }
    i=_this->seld<0?(int)_this->tabs.size():_this->seld;
    j=glwTabbedPaneGetTabAt(_this,x+dx,y+dy);
    if (j>=0)for (;;)
        {
            if (tabs[j].comp==NULL||glwCompIsEnabled(tabs[j].comp))
            {
                glwTabbedPaneSetSelectedIdx(_this,j);
                break;
            }
            if (++j==i)break;
            if (j>=(int)_this->tabs.size())j-=(int)_this->tabs.size();
            if (j==i)break;
        }
}

static void glwTabbedPaneMoveSeld(GLWTabbedPane *_this,int _dir)
{
    switch (_this->tplc)
    {
    case GLWC_WEST:
    case GLWC_EAST:
        switch (_dir)
        {
        case GLWC_WEST :
            glwTabbedPaneSelAdj(_this,0);
            break;
        case GLWC_EAST :
            glwTabbedPaneSelAdj(_this,1);
            break;
        case GLWC_SOUTH:
            glwTabbedPaneSelNext(_this);
            break;
        case GLWC_NORTH:
            glwTabbedPaneSelPrev(_this);
        }
        break;
        /*case GLWC_SOUTH:*/
        /*case GLWC_NORTH:*/
    default       :
        switch (_dir)
        {
        case GLWC_WEST :
            glwTabbedPaneSelPrev(_this);
            break;
        case GLWC_EAST :
            glwTabbedPaneSelNext(_this);
            break;
        case GLWC_SOUTH:
            glwTabbedPaneSelAdj(_this,1);
            break;
        case GLWC_NORTH:
            glwTabbedPaneSelAdj(_this,0);
        }
    }
}

static void glwTabbedPaneDisplayTab(GLWTabbedPane *_this,int _i)
{
    int      x;
    int      y;
    int      w;
    int      h;
    double   tx;
    double   ty;
    GLWcolor fc;
    GLWcolor hc;
    GLWcolor sc;
    GLWTabPage *tab;
    tab = &_this->tabs[_i];
    if (glwCompIsEnabled(&_this->super)&&
            (tab->comp==NULL||glwCompIsEnabled(tab->comp)))fc=tab->forec;
    else fc=glwColorBlend(tab->forec,tab->backc);
    hc=glwColorLighten(tab->backc);
    sc=glwColorDarken(tab->backc);
    x=tab->bounds.x;
    y=tab->bounds.y;
    w=tab->bounds.w;
    h=tab->bounds.h;
    glwColor(tab->backc);
    switch (_this->tplc)
    {
    case GLWC_WEST :
    {
        glRecti(x+1,y+h-1,x+w-1,y);
        glBegin(GL_LINES);
        glwColor(hc);
        glVertex2i(x,y+2);
        glVertex2i(x,y+h-3);
        glVertex2i(x,y+h-3);
        glVertex2i(x+2,y+h-1);
        glVertex2i(x+2,y+h-1);
        glVertex2i(x+w-1,y+h-1);
        glwColor(sc);
        glVertex2i(x+1,y+1);
        glVertex2i(x+2,y);
        glVertex2i(x+2,y);
        glVertex2i(x+w-1,y);
        glEnd();
    }
    break;
    case GLWC_EAST :
    {
        glRecti(x,y+h-1,x+w-2,y);
        glBegin(GL_LINES);
        glwColor(hc);
        glVertex2i(x,y+h-1);
        glVertex2i(x+w-3,y+h-1);
        glVertex2i(x+w-3,y+h-1);
        glVertex2i(x+w-2,y+h-2);
        glwColor(sc);
        glVertex2i(x+w-1,y+h-3);
        glVertex2i(x+w-1,y+2);
        glVertex2i(x+w-1,y+2);
        glVertex2i(x+w-3,y);
        glVertex2i(x+w-3,y);
        glVertex2i(x,y);
        glEnd();
    }
    break;
    case GLWC_SOUTH:
    {
        glRecti(x,y+h-1,x+w-1,y+1);
        glBegin(GL_LINES);
        glwColor(hc);
        glVertex2i(x,y+h-1);
        glVertex2i(x,y+2);
        glVertex2i(x,y+2);
        glVertex2i(x+1,y+1);
        glwColor(sc);
        glVertex2i(x+2,y);
        glVertex2i(x+w-3,y);
        glVertex2i(x+w-3,y);
        glVertex2i(x+w-1,y+2);
        glVertex2i(x+w-1,y+2);
        glVertex2i(x+w-1,y+h-1);
        glEnd();
    }
    break;
    /*case GLWC_NORTH:*/
    default        :
    {
        glRecti(x,y+h-2,x+w-1,y);
        glBegin(GL_LINES);
        glwColor(hc);
        glVertex2i(x,y);
        glVertex2i(x,y+h-3);
        glVertex2i(x,y+h-3);
        glVertex2i(x+2,y+h-1);
        glVertex2i(x+2,y+h-1);
        glVertex2i(x+w-3,y+h-1);
        glwColor(sc);
        glVertex2i(x+w-2,y+h-2);
        glVertex2i(x+w-1,y+h-3);
        glVertex2i(x+w-1,y+h-3);
        glVertex2i(x+w-1,y);
        glEnd();
    }
    break;
    }
    w=glwFontGetStringWidth(_this->super.font, tab->title.c_str());
    h=glwFontGetHeight(_this->super.font);
    tx=x+0.5*(tab->bounds.w-w);
    ty=y+0.5*(tab->bounds.h-h);
    switch (_this->tplc)
    {
    case GLWC_WEST :
        _this->seld==_i?tx--:tx++;
        break;
    case GLWC_EAST :
        _this->seld==_i?tx++:tx--;
        break;
    case GLWC_SOUTH:
        _this->seld==_i?ty--:ty++;
        break;
        /*case GLWC_NORTH:*/
    default        :
        _this->seld==_i?ty++:ty--;
        break;
    }
    if (glwCompIsFocused(&_this->super)&&_this->seld==_i)
    {
        glLineStipple(2,0x5555);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_LOOP);
        glwColor(glwColorBlend(fc,_this->super.backc));
        glVertex2d(tx-1,ty-1);
        glVertex2d(tx-1,ty+h);
        glVertex2d(tx+w,ty+h);
        glVertex2d(tx+w,ty-1);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
    }
    glwColor(fc);
    ty+=glwFontGetDescent(_this->super.font);
    glwFontDrawString(_this->super.font, tab->title.c_str(), tx, ty);
}

static void glwTabbedPanePeerDisplay(GLWTabbedPane *_this,GLWCallbacks *_cb)
{
    GLWcolor  fc;
    GLWcolor  hc;
    GLWcolor  sc;
    size_t    i;
    int       x;
    int       y;
    int       w;
    int       h;
    GLWRect  *selr;
    glwCompSuperDisplay(&_this->super,_cb);
    if (glwCompIsEnabled(&_this->super))fc=_this->super.forec;
    else fc=glwColorBlend(_this->super.forec,_this->super.backc);
    hc=glwColorLighten(_this->super.backc);
    sc=glwColorDarken(_this->super.backc);
    for (i=_this->runs.size(); i-->0;)
    {
        int start;
        int end;
        int j;
        start = _this->runs[i];
        end = _this->runs[(size_t)i + 1 < _this->runs.size() ? i + 1 : 0];
        if (!end)end=_this->tabs.size();
        for (j=start; j<end; j++)glwTabbedPaneDisplayTab(_this,j);
    }
    if (_this->seld>=0 && _this->tabs[_this->seld].run == 0)
    {
        glwTabbedPaneDisplayTab(_this,_this->seld);
    }
    if (_this->seld<0)selr=NULL;
    else selr = &_this->tabs[_this->seld].bounds;
    x=_this->area.x;
    y=_this->area.y;
    w=_this->area.w;
    h=_this->area.h;
    glBegin(GL_LINES);
    glwColor(hc);
    if (_this->tplc!=GLWC_WEST&&_this->tplc!=GLWC_EAST&&_this->tplc!=GLWC_SOUTH&&
            selr!=NULL&&selr->y-1<=y+h-1)
    {
        if (x<selr->x)
        {
            glVertex2i(x,y+h-1);
            glVertex2i(selr->x-1,y+h-1);
        }
        if (selr->x+selr->w<x+w-2)
        {
            glVertex2i(selr->x+selr->w,y+h-1);
            glVertex2i(x+w-2,y+h-1);
        }
    }
    else
    {
        glVertex2i(x,y+h-1);
        glVertex2i(x+w-2,y+h-1);
    }
    glwColor(hc);
    if (_this->tplc==GLWC_WEST&&selr!=NULL&&selr->x+selr->w+1>=x)
    {
        if (y+h-1>selr->y+selr->h)
        {
            glVertex2i(x,y+h-1);
            glVertex2i(x,selr->y+selr->h);
        }
        if (selr->y-1>y+1)
        {
            glVertex2i(x,selr->y-1);
            glVertex2i(x,y+1);
        }
    }
    else
    {
        glVertex2i(x,y+h-1);
        glVertex2i(x,y);
    }
    glwColor(sc);
    if (_this->tplc==GLWC_SOUTH&&selr!=NULL&&selr->y+selr->h>=y)
    {
        if (x+1<selr->x)
        {
            glVertex2i(x+1,y);
            glVertex2i(selr->x-1,y);
        }
        if (selr->x+selr->w<x+w-2)
        {
            glVertex2i(selr->x+selr->w,y);
            glVertex2i(x+w-1,y);
        }
    }
    else
    {
        glVertex2i(x+1,y);
        glVertex2i(x+w-1,y);
    }
    if (_this->tplc==GLWC_EAST&&selr!=NULL&&selr->x-1<=x+w)
    {
        if (y+h-1>selr->y+selr->h)
        {
            glVertex2i(x+w-1,y+h-1);
            glVertex2i(x+w-1,selr->y+selr->h);
        }
        if (selr->y-1>y+1)
        {
            glVertex2i(x+w-1,selr->y-1);
            glVertex2i(x+w-1,y+1);
        }
    }
    else
    {
        glVertex2i(x+w-1,y+h-1);
        glVertex2i(x+w-1,y);
    }
    glEnd();
}

static void glwTabbedPanePeerEnable(GLWTabbedPane *_this,GLWCallbacks *_cb,
                                    int _s)
{
    glwCompSuperEnable(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static void glwTabbedPanePeerFocus(GLWTabbedPane *_this,GLWCallbacks *_cb,
                                   int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static int glwTabbedPanePeerSpecial(GLWTabbedPane *_this,GLWCallbacks *_cb,
                                    int _k,int _x,int _y)
{
    int ret;
    if (glwCompIsFocused(&_this->super))
    {
        ret=-1;
        switch (_k)
        {
        case GLUT_KEY_LEFT :
            glwTabbedPaneMoveSeld(_this,GLWC_WEST);
            break;
        case GLUT_KEY_RIGHT:
            glwTabbedPaneMoveSeld(_this,GLWC_EAST);
            break;
        case GLUT_KEY_DOWN :
            glwTabbedPaneMoveSeld(_this,GLWC_SOUTH);
            break;
        case GLUT_KEY_UP   :
            glwTabbedPaneMoveSeld(_this,GLWC_NORTH);
            break;
        case GLUT_KEY_HOME :
            glwTabbedPaneSetSelectedIdx(_this,0);
            break;
        case GLUT_KEY_END  :
        {
		glwTabbedPaneSetSelectedIdx(_this,(int)_this->tabs.size()-1);
		break;
        }
        default            :
            ret=0;
        }
    }
    else ret=0;
    if (ret>=0)ret=glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
    return ret;
}

static int glwTabbedPanePeerMouse(GLWTabbedPane *_this,GLWCallbacks *_cb,
                                  int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0&&_s)
    {
        glwTabbedPaneSetSelectedIdx(_this,glwTabbedPaneGetTabAt(_this,_x,_y));
        ret=1;
    }
    return ret;
}

static void glwTabbedPanePeerDispose(GLWTabbedPane *_this,GLWCallbacks *_cb)
{
    glwTabbedPaneDelAll(_this);
    glwCompSuperDispose(&_this->super,_cb);
}

const GLWCallbacks GLW_TABBED_PANE_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwTabbedPanePeerDispose,
    (GLWDisplayFunc)glwTabbedPanePeerDisplay,
    NULL,
    NULL,
    (GLWEnableFunc)glwTabbedPanePeerEnable,
    (GLWFocusFunc)glwTabbedPanePeerFocus,
    NULL,
    NULL,
    NULL,
    (GLWSpecialFunc)glwTabbedPanePeerSpecial,
    (GLWMouseFunc)glwTabbedPanePeerMouse,
    NULL,
    NULL
};


GLWTabbedPane::GLWTabbedPane()
{
    this->super.callbacks=&GLW_TABBED_PANE_CALLBACKS;
    glwCompSetFocusable(&this->super,1);
    glwCompSetLayout(&this->super,&glw_tabbed_pane_layout);
    this->tplc=GLWC_NORTH;
    this->seld=-1;
    this->max_tab_w=0;
    this->max_tab_h=0;
    this->valid=0;
    glwRectInit(&this->area,0,0,0,0);
}


int glwTabbedPaneGetPlacement(GLWTabbedPane *_this)
{
    return _this->tplc;
}

void glwTabbedPaneSetPlacement(GLWTabbedPane *_this,int _tplc)
{
    if (_this->tplc!=_tplc)
    {
        _this->tplc=_tplc;
        glwCompRevalidate(&_this->super);
    }
}

int glwTabbedPaneGetSelectedIdx(GLWTabbedPane *_this)
{
    return _this->seld;
}

void glwTabbedPaneSetSelectedIdx(GLWTabbedPane *_this,int _idx)
{
	if (_idx >= 0 && _idx < (int)_this->tabs.size() && _idx != _this->seld)
	{
		_this->seld=_idx;
		glwCompRevalidate(&_this->super);
	}
}

GLWComponent *glwTabbedPaneGetSelectedComp(GLWTabbedPane *_this)
{
    if (_this->seld<0)return NULL;
    return _this->tabs[_this->seld].comp;
}

void glwTabbedPaneSetSelectedComp(GLWTabbedPane *_this,GLWComponent *_comp)
{
    if (_comp!=NULL)
    {
        GLWTabPage* tabs;
        size_t      i;
        tabs = &_this->tabs[0];
        for (i=_this->tabs.size(); i-->0;)if (tabs[i].comp==_comp)
            {
                glwTabbedPaneSetSelectedIdx(_this,(int)i);
                break;
            }
    }
}

int glwTabbedPaneAdd(GLWTabbedPane *_this,GLWComponent *_comp,
                     const char *_title,int _idx)
{
    GLWTabPage tab;
    if (_idx<0||(size_t)_idx>_this->tabs.size()) _idx = _this->tabs.size();
    if (_title!=NULL)
	    tab.title = _title;
    glwRectInit(&tab.bounds,0,0,0,0);
    tab.backc=_this->super.backc;
    tab.forec=_this->super.forec;
    tab.run=0;
    tab.comp=_comp;
    auto ind = _this->tabs.begin() + _idx;
    _this->tabs.insert(ind, tab);
    if (_comp==NULL||glwCompAdd(&_this->super,_comp,-1))
    {
            if (_comp!=NULL)glwCompVisibility(_comp,0);
            else glwCompRevalidate(&_this->super);
            if (_this->seld<0)glwTabbedPaneSetSelectedIdx(_this,0);
            else if (_this->seld>=_idx)_this->seld++;
            return 1;
    }
    _this->tabs.erase(ind);
    return 0;
}

int glwTabbedPaneDel(GLWTabbedPane *_this,GLWComponent *_comp)
{
	for (size_t i = 0; i < _this->tabs.size(); ++i)
	{
		if (_this->tabs[i].comp == _comp)
		{
			glwTabbedPaneDelAt(_this,(int)i);
			return 1;
		}
        }
	return 0;
}

void glwTabbedPaneDelAt(GLWTabbedPane *_this,int _idx)
{
    GLWTabPage& tab = _this->tabs[_idx];
    if (tab.comp==NULL||!glwCompDel(&_this->super,tab.comp))
    {
        glwCompRevalidate(&_this->super);
    }
    _this->tabs.erase(_this->tabs.begin() + _idx);
    if (_this->seld >= (int)_this->tabs.size()) _this->seld--;
}

void glwTabbedPaneDelAll(GLWTabbedPane *_this)
{
	while (_this->tabs.size() > 0)
		glwTabbedPaneDelAt(_this, (int)_this->tabs.size() - 1);
}

int glwTabbedPaneGetTabCount(GLWTabbedPane *_this)
{
	return (int)_this->tabs.size();
}

int glwTabbedPaneGetRunCount(GLWTabbedPane *_this)
{
    glwCompValidate(&_this->super);
    return (int)_this->runs.size();
}

GLWcolor glwTabbedPaneGetBackColorAt(GLWTabbedPane *_this,int _idx)
{
    return _this->tabs[_idx].backc;
}

void glwTabbedPaneSetBackColorAt(GLWTabbedPane *_this,int _idx,GLWcolor _c)
{
    GLWTabPage& tab = _this->tabs[_idx];
    if (tab.backc != _c)
    {
        tab.backc = _c;
        glwCompRepaint(&_this->super,0);
    }
}

GLWcolor glwTabbedPaneGetForeColorAt(GLWTabbedPane *_this,int _idx)
{
    return _this->tabs[_idx].forec;
}

void glwTabbedPaneSetForeColorAt(GLWTabbedPane *_this,int _idx,GLWcolor _c)
{
    GLWTabPage& tab = _this->tabs[_idx];
    if (tab.forec != _c)
    {
        tab.forec = _c;
        glwCompRepaint(&_this->super,0);
    }
}

const char *glwTabbedPaneGetTitleAt(GLWTabbedPane *_this,int _idx)
{
	return _this->tabs[_idx].title.c_str();
}

int glwTabbedPaneSetTitleAt(GLWTabbedPane *_this,int _idx,const char *_s)
{
    GLWTabPage& tab = _this->tabs[_idx];
    if (_s == NULL)
	    tab.title.clear();
    else
	    tab.title = _s;
    glwCompRevalidate(&_this->super);
    return 1;
}

int glwTabbedPaneAddTitleAt(GLWTabbedPane *_this,int _idx,const char *_s)
{
    GLWTabPage& tab = _this->tabs[_idx];
    if (tab.title.size() <= 1)
	    return glwTabbedPaneSetTitleAt(_this,_idx,_s);
    else if (_s!=NULL)
    {
	    tab.title.append(_s);
            glwCompRevalidate(&_this->super);
            return 1;
    }
    else return 1;
}
