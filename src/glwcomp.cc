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

#include "glinc.hh"
#include "glw.hh"

#include <map>



/*Generic component. All components can contain other components. They receive
  events from their parent (translated into their coordinate system), which
  they dispatch to their children (though this can be overridden). Each
  component has at most one child receiving keyboard input, which is said to
  have focus. Each component has at most one child receiving mouse input
  (usually the first child under the mouse), which is said to have capture.
  Components have a layout manager, which controls the sizing and position of
  the component's children, and determines the space requirements for this
  component. They also have a callback vector, which receives events and
  processes them. In addition, they can register idle callbacks, or timer
  callbacks, which are called whenever the application is idle, or after an
  elapsed period of time, respectively.*/

typedef struct GLWTimerEntry {
    GLWComponent  *comp;
    void          *ctx;
    GLWActionFunc  func;
} GLWTimerEntry;

// These maps does not need ordering, so C++2011 unordered_map wuold
// be fine.
typedef std::map<int, GLWTimerEntry> timer_map;
typedef std::map<int, GLWTimerEntry>::iterator timer_itr;

static int glw_timer_id;
static int glw_idler_id;
static timer_map glw_timer_table;
static timer_map glw_idler_table;

static void glwCompGlutPostRedisplay(int _wid)
{
    int wid;
    wid=glutGetWindow();
    if (wid!=_wid)glutSetWindow(_wid);
    glutPostRedisplay();
    if (wid!=_wid)glutSetWindow(wid);
}

static void glwCompGlutTimer(int _id)
{
    timer_itr it = glw_timer_table.find(_id);
    if (it != glw_timer_table.end()) {
        GLWTimerEntry& timer = it->second;
        timer.func(timer.ctx,timer.comp);
        glutSetWindow(timer.comp->wid);
    }
    glw_timer_table.erase(_id);
}

static void glwCompGlutIdle(void)
{
    int clear = 1;
    // Need an unusual iteration structure since the current iterator
    // may be deleted.
    timer_itr it = glw_idler_table.begin();
    while (it != glw_idler_table.end()) {
        // Tricky part: idler is taken before incrementing
        // iterator, and before calling the functions below which
        // may delete the map element.
        GLWTimerEntry& idler = it->second;
        ++it;
        glutSetWindow(idler.comp->wid);
        idler.func(idler.ctx, idler.comp);
        clear=0;
    }
    if (clear)
        glutIdleFunc(NULL);
}

static void glwCompPeerDisplay(GLWComponent *_this,const GLWCallbacks *_cb)
{
    /*The default coordinate system is a 2D orthographic projection with the
      origin at 0,0 and the upper-right corner at the component's width and
      height*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0-1E-4,_this->bounds.w-1E-4,0-1E-4,_this->bounds.h-1E-4);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /*Clear the background*/
    glwColor(_this->backc);
    glRecti(0,0,_this->bounds.w-1,_this->bounds.h-1);
}

static void glwCompPeerDisplayChildren(GLWComponent *_this,
                                       const GLWCallbacks *_cb)
{
    if (_this->comps.size() > 0) {
        GLWComponent *p;
        int           dx,dy;
        size_t        i;
        for (dx=0,p=_this; p!=NULL; p=p->parent)dx+=p->bounds.x;
        for (dy=0,p=_this; p!=NULL; p=p->parent)dy+=p->bounds.y;
        for (i = _this->comps.size(); i-->0;) {
            GLWComponent *comp;
            comp = _this->comps[i];
            if (glwCompIsVisible(comp)) {
                glwCompValidate(comp);
                glPushAttrib(GL_ALL_ATTRIB_BITS);
                glScissor(dx+comp->bounds.x,dy+comp->bounds.y,
                          comp->bounds.w,comp->bounds.h);
                glEnable(GL_SCISSOR_TEST);
                glViewport(dx+comp->bounds.x,dy+comp->bounds.y,
                           comp->bounds.w,comp->bounds.h);
                glwCompDisplay(comp);
                glPopAttrib();
                glwCompDisplayChildren(comp);
            }
        }
    }
}

static void glwCompPeerValidate(GLWComponent *_this,const GLWCallbacks *_cb)
{
    glwCompLayout(_this);
}

static int glwCompPeerKeyboard(GLWComponent *_this,const GLWCallbacks *_cb,
                               unsigned char _k,int _x,int _y)
{
    GLWComponent *focus;
    glwCompTransferCapture(_this,_x,_y);
    if (_k=='\t') {
        int mods;
        mods=glutGetModifiers();
        /*A bug in GLUT can prevent any key code from being delivered at all when
           Shift+Tab is pressed.*/
        if (mods!=0/*==GLUT_ACTIVE_SHIFT*/) {
            glwCompPrevFocus(_this);
            return -1;
        } else if (mods==0) {
            glwCompNextFocus(_this);
            return -1;
        }
    }
    focus=_this->focus;
    if (focus!=NULL) {
        for (; focus->parent!=_this; focus=focus->parent);
        if (glwCompIsVisible(focus)&&glwCompIsEnabled(focus)) {
            return glwCompKeyboard(focus,_k,_x-focus->bounds.x,_y-focus->bounds.y);
        }
    }
    return 0;
}

static int glwCompPeerSpecial(GLWComponent *_this,const GLWCallbacks *_cb,
                              int _k,int _x,int _y)
{
    GLWComponent *focus;
    glwCompTransferCapture(_this,_x,_y);
    focus=_this->focus;
    if (focus!=NULL) {
        for (; focus->parent!=_this; focus=focus->parent);
        if (glwCompIsVisible(focus)&&glwCompIsEnabled(focus)) {
            return glwCompSpecial(focus,_k,_x-focus->bounds.x,_y-focus->bounds.y);
        }
    }
    return 0;
}

static int glwCompPeerMouse(GLWComponent *_this,const GLWCallbacks *_cb,
                            int _b,int _s,int _x,int _y)
{
    glwCompTransferCapture(_this,_x,_y);
    if (_s)_this->mouse_b|=1<<_b;
    else _this->mouse_b&=~(1<<_b);
    if (_this->capture!=NULL) {
        return glwCompMouse(_this->capture,_b,_s,_x-_this->capture->bounds.x,
                            _y-_this->capture->bounds.y);
    } else if (_s) {
        glwCompRequestFocus(_this);
        return 1;
    }
    return 0;
}

static int glwCompPeerMotion(GLWComponent *_this,const GLWCallbacks *_cb,
                             int _x,int _y)
{
    /*Mouse is down: can't transfer capture*/
    if (_this->capture!=NULL) {
        return glwCompMotion(_this->capture,_x-_this->capture->bounds.x,
                             _y-_this->capture->bounds.y);
    }
    return 0;
}

static int glwCompPeerPassiveMotion(GLWComponent *_this,
                                    const GLWCallbacks *_cb,int _x,int _y)
{
    glwCompTransferCapture(_this,_x,_y);
    if (_this->capture!=NULL) {
        return glwCompPassiveMotion(_this->capture,_x-_this->capture->bounds.x,
                                    _y-_this->capture->bounds.y);
    }
    return 0;
}


const GLWCallbacks GLW_COMPONENT_CALLBACKS= {
    NULL,
    NULL,
    glwCompPeerDisplay,
    glwCompPeerDisplayChildren,
    glwCompPeerValidate,
    NULL,
    NULL,
    NULL,
    NULL,
    glwCompPeerKeyboard,
    glwCompPeerSpecial,
    glwCompPeerMouse,
    glwCompPeerMotion,
    glwCompPeerPassiveMotion
};


GLWComponent::GLWComponent()
{
    this->parent=NULL;
    this->callbacks=&GLW_COMPONENT_CALLBACKS;
    glwRectInit(&this->bounds,0,0,0,0);
    this->constraints.minw=-1;
    this->constraints.minh=-1;
    this->constraints.prew=-1;
    this->constraints.preh=-1;
    this->constraints.maxw=-1;
    this->constraints.maxh=-1;
    glwInsetsInit(&this->constraints.insets,0,0,0,0);
    this->constraints.alignx=0.5;
    this->constraints.aligny=0.5;
    this->constraints.fill=GLWC_NONE;
    this->constraints.gridx=GLWC_RELATIVE;
    this->constraints.gridy=GLWC_RELATIVE;
    this->constraints.gridw=1;
    this->constraints.gridh=1;
    this->constraints.weightx=0;
    this->constraints.weighty=0;
    this->forec=GLW_COLOR_BLACK;
    this->backc=GLW_COLOR_LIGHT_GREY;
    this->font=glwFontGet(GLW_FONT_SERIF,0);
    this->cursor=GLUT_CURSOR_INHERIT;
    this->layout=NULL;
    this->focus=NULL;
    this->capture=NULL;
    this->mouse_b=0;
    this->wid=0;
    this->visible=0;
    this->enabled=1;
    this->valid=0;
    this->validate_root=0;
    this->focused=0;
    this->focusable=0;
}

GLWComponent::~GLWComponent()
{
    delete this->parent;
// The Components should be owned by their parents and be destroyed
// when going out of scope. this->comps is just a list of pointers to
// components without ownership.
    // for (auto it = this->comps.begin(); it != this->comps.end(); ++it)
    // {
    // 	delete *it;
    // }
    glwCompSetLayout(this,NULL);
    glwCompDispose(this);
}

int glwCompIsVisible(GLWComponent *_this)
{
    return _this->visible;
}

int glwCompIsShowing(GLWComponent *_this)
{
    return glwCompIsVisible(_this)&&_this->parent!=NULL&&
           glwCompIsShowing(_this->parent);
}

int glwCompIsEnabled(GLWComponent *_this)
{
    return _this->enabled;
}

int glwCompIsFocused(GLWComponent *_this)
{
    return _this->focused;
}

int glwCompIsFocusable(GLWComponent *_this)
{
    return _this->focusable;
}

int glwCompIsFocusTraversable(GLWComponent *_this)
{
    for (comps_itr it = _this->comps.begin(); it != _this->comps.end(); ++it) {
        GLWComponent* child = *it;
        if (child->focusable||glwCompIsFocusTraversable(child))
            return 1;
    }
    return 0;
}

int glwCompIsCapturing(GLWComponent *_this)
{
    return _this->parent==NULL||_this->parent->capture==_this;
}

void glwCompSetBounds(GLWComponent *_this,int _x,int _y,int _w,int _h)
{
    _this->bounds.x=_x;
    _this->bounds.y=_y;
    if (_w!=_this->bounds.w||_h!=_this->bounds.h) {
        _this->bounds.w=_w;
        _this->bounds.h=_h;
        glwCompRevalidate(_this);
    }
}

void glwCompSetForeColor(GLWComponent *_this,GLWcolor _c)
{
    if (_c!=_this->forec) {
        _this->forec=_c;
        glwCompRepaint(_this,0);
    }
}

void glwCompSetBackColor(GLWComponent *_this,GLWcolor _c)
{
    if (_c!=_this->backc) {
        _this->backc=_c;
        glwCompRepaint(_this,0);
    }
}

void glwCompSetFont(GLWComponent *_this,GLWfont _font)
{
    if (_font!=_this->font) {
        _this->font=_font;
        glwCompRevalidate(_this);
    }
}

void glwCompSetCursor(GLWComponent *_this,GLWcursor _cursor)
{
    if (_cursor!=_this->cursor) {
        GLWComponent *c;
        _this->cursor=_cursor;
        for (c=_this; c->parent!=NULL; c=c->parent) {
            if (c->parent->capture!=c)return;
        }
        for (c=_this; c->parent!=NULL&&c->cursor!=GLUT_CURSOR_INHERIT; c=c->parent);
        glutSetCursor(c->cursor);
    }
}

void glwCompSetFocusable(GLWComponent *_this,int _b)
{
    _this->focusable=_b?1:0;
    if (!_b&&glwCompIsFocused(_this))glwCompClearFocus(_this);
}

void glwCompSetMinWidth(GLWComponent *_this,int _min_w)
{
    if (_this->constraints.minw!=_min_w) {
        _this->constraints.minw=_min_w;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetMinHeight(GLWComponent *_this,int _min_h)
{
    if (_this->constraints.minh!=_min_h) {
        _this->constraints.minh=_min_h;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetMaxWidth(GLWComponent *_this,int _max_w)
{
    if (_this->constraints.maxw!=_max_w) {
        _this->constraints.maxw=_max_w;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetMaxHeight(GLWComponent *_this,int _max_h)
{
    if (_this->constraints.maxh!=_max_h) {
        _this->constraints.maxh=_max_h;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetPreWidth(GLWComponent *_this,int _pre_w)
{
    if (_this->constraints.prew!=_pre_w) {
        _this->constraints.prew=_pre_w;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetPreHeight(GLWComponent *_this,int _pre_h)
{
    if (_this->constraints.preh!=_pre_h) {
        _this->constraints.preh=_pre_h;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetInsets(GLWComponent *_this,int _t,int _b,int _l,int _r)
{
    if (_this->constraints.insets.t!=_t||
            _this->constraints.insets.b!=_b||
            _this->constraints.insets.l!=_l||
            _this->constraints.insets.r!=_r) {
        glwInsetsInit(&_this->constraints.insets,_t,_b,_l,_r);
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetAlignX(GLWComponent *_this,double _align_x)
{
    if (_align_x<0)_align_x=0;
    else if (_align_x>1)_align_x=1;
    if (_this->constraints.alignx!=_align_x) {
        _this->constraints.alignx=_align_x;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetAlignY(GLWComponent *_this,double _align_y)
{
    if (_align_y<0)_align_y=0;
    else if (_align_y>1)_align_y=1;
    if (_this->constraints.aligny!=_align_y) {
        _this->constraints.aligny=_align_y;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetFill(GLWComponent *_this,int _fill)
{
    if (_this->constraints.fill!=_fill) {
        _this->constraints.fill=_fill;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetGridX(GLWComponent *_this,int _grid_x)
{
    if (_this->constraints.gridx!=_grid_x) {
        _this->constraints.gridx=_grid_x;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetGridY(GLWComponent *_this,int _grid_y)
{
    if (_this->constraints.gridy!=_grid_y) {
        _this->constraints.gridy=_grid_y;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetGridWidth(GLWComponent *_this,int _grid_w)
{
    if (_this->constraints.gridw!=_grid_w) {
        _this->constraints.gridw=_grid_w;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetGridHeight(GLWComponent *_this,int _grid_h)
{
    if (_this->constraints.gridh!=_grid_h) {
        _this->constraints.gridh=_grid_h;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetWeightX(GLWComponent *_this,double _weight_x)
{
    if (_weight_x<0)_weight_x=0;
    if (_this->constraints.weightx!=_weight_x) {
        _this->constraints.weightx=_weight_x;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompSetWeightY(GLWComponent *_this,double _weight_y)
{
    if (_weight_y<0)_weight_y=0;
    if (_this->constraints.weighty!=_weight_y) {
        _this->constraints.weighty=_weight_y;
        if (_this->parent!=NULL)glwCompRevalidate(_this->parent);
    }
}

void glwCompGetMinSize(GLWComponent *_this,int *_w,int *_h)
{
    if (_w!=NULL) {
        *_w=_this->constraints.minw;
        if (*_w>=0)_w=NULL;
    }
    if (_h!=NULL) {
        *_h=_this->constraints.minh;
        if (*_h>=0)_h=NULL;
    }
    if ((_w!=NULL||_h!=NULL)&&_this->layout!=NULL&&_this->layout->min_size!=NULL) {
        _this->layout->min_size(_this->layout,_this,_w,_h);
    }
}

void glwCompGetMaxSize(GLWComponent *_this,int *_w,int *_h)
{
    if (_w!=NULL) {
        *_w=_this->constraints.maxw;
        if (*_w>=0)_w=NULL;
    }
    if (_h!=NULL) {
        *_h=_this->constraints.maxh;
        if (*_h>=0)_h=NULL;
    }
    if ((_w!=NULL||_h!=NULL)&&_this->layout!=NULL&&_this->layout->max_size!=NULL) {
        _this->layout->max_size(_this->layout,_this,_w,_h);
    }
}

void glwCompGetPreSize(GLWComponent *_this,int *_w,int *_h)
{
    int *w;
    int *h;
    int  minw;
    int  minh;
    int  maxw;
    int  maxh;
    if (_w!=NULL) {
        *_w=_this->constraints.prew;
        if (*_w>=0)w=NULL;
        else w=_w;
    } else w=NULL;
    if (_h!=NULL) {
        *_h=_this->constraints.preh;
        if (*_h>=0)h=NULL;
        else h=_h;
    } else h=NULL;
    if ((w!=NULL||h!=NULL)&&_this->layout!=NULL&&_this->layout->pre_size!=NULL) {
        _this->layout->pre_size(_this->layout,_this,w,h);
    }
    glwCompGetMinSize(_this,_w!=NULL?&minw:NULL,_h!=NULL?&minh:NULL);
    glwCompGetMaxSize(_this,_w!=NULL?&maxw:NULL,_h!=NULL?&maxh:NULL);
    if (_w!=NULL) {
        if ((*_w<0&&minw<0)|| (maxw >= 0 && maxw < *_w))
            *_w=maxw;
        if (*_w < 0|| (minw >= 0 && minw > *_w))
            *_w=minw;
    }
    if (_h!=NULL) {
        if ((*_h<0&&minh<0)|| (maxh >= 0 && maxh < *_h))
            *_h=maxh;
        if (*_h<0|| (minh >= 0 && minh > *_h))
            *_h=minh;
    }
}

void glwCompInvalidate(GLWComponent *_this)
{
    if (_this->valid) {
        _this->valid=0;
        if (_this->layout!=NULL&&_this->layout->invalidate!=NULL) {
            _this->layout->invalidate(_this->layout,_this);
        }
        if (_this->parent!=NULL&&!_this->parent->valid) {
            glwCompInvalidate(_this->parent);
        } else glwCompRepaint(_this,0);
    }
}

void glwCompRevalidate(GLWComponent *_this)
{
    if (_this->valid) {
        _this->valid=0;
        if (_this->layout!=NULL&&_this->layout->invalidate!=NULL) {
            _this->layout->invalidate(_this->layout,_this);
        }
        if (!_this->validate_root&&_this->parent!=NULL&&_this->parent->valid) {
            glwCompRevalidate(_this->parent);
        } else glwCompRepaint(_this,0);
    }
}

void glwCompRepaint(GLWComponent *_this,unsigned _millis)
{
    if (_this->wid!=0) {
        if (!_millis)glwCompGlutPostRedisplay(_this->wid);
        else glutTimerFunc(_millis,glwCompGlutPostRedisplay,_this->wid);
    }
}

int glwCompAdd(GLWComponent *_this,GLWComponent *_comp,int _idx)
{
    if (_idx <= (int)_this->comps.size()) {
        if (_idx < 0) _idx = (int)_this->comps.size();
        _this->comps.insert(_this->comps.begin() + _idx, _comp);
        if (_comp->parent!=NULL)glwCompDel(_this,_comp);
        else {
            glwCompClearFocus(_comp);
            glwCompClearCapture(_comp);
        }
        _comp->parent=_this;
        _comp->wid=_this->wid;
        if (!glwCompIsVisible(_comp))glwCompVisibility(_comp,1);
        glwCompInvalidate(_this);
        return 1;
    }
    return 0;
}

int glwCompDel(GLWComponent *_this,GLWComponent *_comp)
{
    for (comps_itr it = _this->comps.begin(); it != _this->comps.end(); ++it) {
        if (*it == _comp) {
            _this->comps.erase(it);
            _comp->parent=NULL;
            _comp->wid=0;
            if (glwCompIsVisible(_comp))
                glwCompVisibility(_comp, 0);
            if (_this->focus == _comp
                    || _this->focus == _comp->focus)
                glwCompClearFocus(_this);
            glwCompInvalidate(_this);
            return 1;
        }
    }
    return 0;
}

void glwCompDelAll(GLWComponent *_this)
{
    for (comps_itr it = _this->comps.begin(); it != _this->comps.end(); ++it) {
        GLWComponent* comp = *it;
        comp->parent=NULL;
        comp->wid=0;
        if (glwCompIsVisible(comp)) glwCompVisibility(comp, 0);
    }
    _this->comps.clear();
    if (_this->focus != NULL) glwCompClearFocus(_this);
    glwCompInvalidate(_this);
}

void glwCompSetLayout(GLWComponent *_this,GLWLayoutManager *_layout)
{
    if (_this->layout!=_layout) {
        if (_this->layout != NULL) {
            delete _this->layout;
        }
        _this->layout=_layout;
    }
    glwCompInvalidate(_this);
}

void glwCompLayout(GLWComponent *_this)
{
    if (_this->layout!=NULL&&_this->layout->layout!=NULL) {
        _this->layout->layout(_this->layout,_this);
    }
}

static void glwCompSetFocus(GLWComponent *_this,GLWComponent *_comp)
{
    _this->focus=_comp;
    if (_this->parent!=NULL)glwCompSetFocus(_this->parent,_comp);
}

static int glwCompNextFocusTree(GLWComponent *_this)
{
    size_t i,j;
    if (_this->focus==NULL)i=(size_t)-1;
    else {              /*Find the currently focused component (or it's ancestor)*/
        for (i=0;; i++) {
            GLWComponent *comp;
            if (i == _this->comps.size()) { /*This should never happen!*/
                _this->focus=NULL;
                return 0;
            }
            comp = _this->comps[i];
            /*Foud the currently focused component. If it has no children, we get to
              transfer, otherwise let it try to transfer to one of its children first*/
            if (comp==_this->focus) {
                if (!glwCompIsFocusTraversable(comp))break;
                if (glwCompNextFocusTree(comp))return 1;
                break;
            }
            /*We've found an ancestor of the currently focused component: let it
              try to transfer to one of its children first*/
            if (comp->focus==_this->focus) {
                if (!glwCompIsVisible(comp)||!glwCompIsEnabled(comp)) {
                    glwCompClearFocus(comp);
                } else if (glwCompNextFocusTree(comp))return 1;
                break;
            }
        }                                         /*No, we get to transfer*/
        _this->focus=NULL;
    }
    j=i;
    do {
        GLWComponent *comp;
        j++;
        if (j >= _this->comps.size()) {
            /*There were no more focusable components in our tree, let our parent
              try transferring focus to one of its descendants now*/
            if (_this->parent!=NULL||glwCompIsFocusable(_this)||
                    _this->comps.size() == 0) return 0;
            j=0;
        }
        comp = _this->comps[j];
        if (glwCompIsVisible(comp)&&glwCompIsEnabled(comp)) {
            /*Found a focusable component: propogate it up the tree*/
            if (glwCompIsFocusable(comp)) {
                glwCompSetFocus(_this,comp);
                return 1;
            }
            /*Found an ancestor of a focusable component: let it transfer focus*/
            else if (glwCompIsFocusTraversable(comp)) {
                if (glwCompNextFocusTree(comp))return 1;
            }
        }
    } while (j!=i);
    return 0;
}

static int glwCompPrevFocusTree(GLWComponent *_this)
{
    size_t i,j;
    if (_this->focus==NULL) {
        i=0;
        j = _this->comps.size();
    } else {             /*Find the currently focused component (or it's ancestor)*/
        for (i=0;; i++) {
            GLWComponent *comp;
            if (i == _this->comps.size()) {  /*This should never happen!*/
                _this->focus=NULL;
                return 0;
            }
            comp = _this->comps[i];
            /*Foud the currently focused component: we get to transfer*/
            if (comp==_this->focus)break;
            /*We've found an ancestor of the currently focused component: let it
              try to transfer to one of its children first. If it fails and is itself
              focusable, give it focus*/
            if (comp->focus==_this->focus) {
                if (!glwCompIsVisible(comp)||!glwCompIsEnabled(comp)) {
                    glwCompClearFocus(comp);
                } else if (glwCompPrevFocusTree(comp))return 1;
                if (glwCompIsFocusable(comp)) {
                    glwCompSetFocus(_this,comp);
                    return 1;
                }
                break;
            }
        }                                         /*No, we get to transfer*/
        _this->focus=NULL;
        j=i;
    }
    do {
        GLWComponent *comp;
        j--;
        if (j >= _this->comps.size()) {
            /*There were no more focusable components in our tree, let our parent
              try transferring focus to one of its descendants now*/
            if (_this->parent!=NULL||glwCompIsFocusable(_this)||
                    _this->comps.size() == 0) return 0;
            j = _this->comps.size() - 1;
        }
        comp = _this->comps[j];
        if (glwCompIsVisible(comp)&&glwCompIsEnabled(comp)) {
            /*Found an ancestor of a focusable component: let it transfer focus*/
            if (glwCompIsFocusTraversable(comp)) {
                if (glwCompPrevFocusTree(comp))return 1;
            }
            /*Found a focusable component: propogate it up the tree*/
            else if (glwCompIsFocusable(comp)) {
                glwCompSetFocus(_this,comp);
                return 1;
            }
        }
    } while (j!=i);
    return 0;
}

void glwCompNextFocus(GLWComponent *_this)
{
    if (_this->parent!=NULL)glwCompNextFocus(_this->parent);
    else {
        GLWComponent *old;
        GLWComponent *new_;
        old=_this->focus!=NULL?_this->focus:_this;
        glwCompNextFocusTree(_this);
        new_=_this->focus!=NULL?_this->focus:_this;
        if (old!=new_&&glwCompIsFocused(old))glwCompFocus(old,0);
        if (old!=new_&&glwCompIsFocusable(new_))glwCompFocus(new_,1);
    }
}

void glwCompPrevFocus(GLWComponent *_this)
{
    if (_this->parent!=NULL)glwCompNextFocus(_this->parent);
    else {
        GLWComponent *old;
        GLWComponent *new_;
        old=_this->focus!=NULL?_this->focus:_this;
        glwCompPrevFocusTree(_this);
        new_=_this->focus!=NULL?_this->focus:_this;
        if (old!=new_&&glwCompIsFocused(old))glwCompFocus(old,0);
        if (old!=new_&&glwCompIsFocusable(new_))glwCompFocus(new_,1);
    }
}

void glwCompClearFocus(GLWComponent *_this)
{
    if (_this->focus!=NULL) {
        size_t i;
        for (i = 0; i < _this->comps.size(); i++) {
            GLWComponent *comp;
            comp = _this->comps[i];
            if (_this->focus==comp||comp->focus!=NULL)glwCompClearFocus(comp);
        }
    } else if (_this->parent==NULL||_this->parent->focus==_this) {
        glwCompSetFocus(_this,NULL);
        if (glwCompIsFocused(_this))glwCompFocus(_this,0);
    }
}

void glwCompRequestFocus(GLWComponent *_this)
{
    if (glwCompIsVisible(_this)&&glwCompIsEnabled(_this)&&_this->parent!=NULL) {
        GLWComponent *root;
        GLWComponent *old;
        for (root=_this->parent; root->parent!=NULL; root=root->parent);
        old=root->focus!=NULL?root->focus:root;
        if (_this!=old) {
            glwCompClearFocus(root);
            if (glwCompIsFocusable(_this)) {
                glwCompSetFocus(_this->parent,_this);
                glwCompFocus(_this,1);
            }
        }
    }
}

void glwCompTransferCapture(GLWComponent *_this,int _x,int _y)
{
    GLWComponent *capture;
    capture=glwCompGetComponentAt(_this,_x,_y);
    glwCompRequestCapture(_this,capture);
}

void glwCompClearCapture(GLWComponent *_this)
{
    size_t i;
    for (i = 0; i < _this->comps.size(); i++) {
        glwCompClearCapture(_this->comps[i]);
    }
    if (_this->capture!=NULL) {
        glwCompEntry(_this->capture,0);
        _this->capture=NULL;
    }
    _this->mouse_b=0;
}

void glwCompRequestCapture(GLWComponent *_this,GLWComponent *_that)
{
    if (_that==NULL||_that->parent==_this) {
        if (_that!=NULL&&!glwCompIsEnabled(_that))_that=NULL;
        if (_this->capture!=_that&&!_this->mouse_b) {
            if (_this->capture!=NULL)glwCompEntry(_this->capture,0);
            _this->capture=_that;
            if (_that!=NULL)glwCompEntry(_that,1);
        }
    }
}

GLWComponent *glwCompGetComponentAt(GLWComponent *_this,int _x,int _y)
{
    /*Children do not extend beyond their parent's client rectangle*/
    if (_x<0||_x>=_this->bounds.w||_y<0||_y>=_this->bounds.h)return NULL;
    /*Check each child, in order. The first one hit that contains the point is
      returned (assumes higher components come first)*/
    for (comps_itr it = _this->comps.begin(); it != _this->comps.end(); ++it) {
        GLWComponent* child = *it;
        if (child!=NULL&&glwCompIsVisible(child)&&
                child->bounds.x<=_x&&
                child->bounds.x+child->bounds.w>_x&&
                child->bounds.y<=_y&&
                child->bounds.y+child->bounds.h>_y) {
            return child;
        }
    }
    /*No child contains this point*/
    return NULL;
}

GLWComponent *glwCompFindComponentAt(GLWComponent *_this,int _x,int _y)
{
    /*Children do not extend beyond their parent's client rectangle*/
    if (_x<0||_x>=_this->bounds.w||_y<0||_y>=_this->bounds.h)return _this;
    /*Check each child, in order. The first one hit that contains the point is
      returned (assumes higher components come first)*/
    for (comps_itr it = _this->comps.begin(); it != _this->comps.end(); ++it) {
        GLWComponent* child = *it;
        if (child!=NULL&&glwCompIsVisible(child)&&
                child->bounds.x<=_x&&
                child->bounds.x+child->bounds.w>_x&&
                child->bounds.y<=_y&&
                child->bounds.y+child->bounds.h>_y) {
            return glwCompFindComponentAt(child,_x-child->bounds.x,
                                          _y-child->bounds.y);
        }
    }
    /*No child contains this point, return ourselves*/
    return _this;
}

int glwCompAddTimer(GLWComponent *_this,GLWActionFunc _func,
                    void *_ctx,unsigned _millis)
{
    GLWTimerEntry timer;
    int           id;
    id=++glw_timer_id;
    if (!id)id=++glw_timer_id;
    while (glw_timer_table.find(id) != glw_timer_table.end())
        id = ++glw_timer_id;
    timer.comp=_this;
    timer.ctx=_ctx;
    timer.func=_func;
    glw_timer_table.insert(std::pair<int, GLWTimerEntry>(id, timer));
    _this->timers.push_back(id);
    glutTimerFunc(_millis,glwCompGlutTimer,id);
    return id;
}

int glwCompDelTimer(GLWComponent *_this, int id)
{
    for (vi_itr it = _this->timers.begin(); it != _this->timers.end(); ++it)
        if (*it == id) {
            _this->timers.erase(it);
            glw_timer_table.erase(id);
            return 1;
        }
    return 0;
}

GLWActionFunc glwCompGetTimerFunc(GLWComponent *_this,int _id)
{
    timer_itr it = glw_timer_table.find(_id);
    if (it != glw_timer_table.end()) {
        GLWTimerEntry& entry = (*it).second;
        return entry.func;
    }
    return NULL;
}

void *glwCompGetTimerCtx(GLWComponent *_this,int _id)
{
    timer_itr it = glw_timer_table.find(_id);
    if (it != glw_timer_table.end()) {
        GLWTimerEntry& entry = (*it).second;
        return entry.ctx;
    }
    return NULL;
}

int glwCompGetTimerCount(GLWComponent *_this)
{
    return (int)_this->timers.size();
}

int glwCompGetTimerID(GLWComponent *_this,int _idx)
{
    return _this->timers[_idx];
}

int glwCompAddIdler(GLWComponent *_this,GLWActionFunc _func,void *_ctx)
{
    GLWTimerEntry idler;
    int           id;
    id=++glw_idler_id;
    if (!id)
        id = ++glw_idler_id;
    while (glw_idler_table.find(id) != glw_idler_table.end())
        id = ++glw_idler_id;
    idler.comp=_this;
    idler.ctx=_ctx;
    idler.func=_func;
    glw_idler_table.insert(std::pair<int, GLWTimerEntry>(id, idler));
    _this->idlers.push_back(id);
    glutIdleFunc(glwCompGlutIdle);
    return id;
}

int glwCompDelIdler(GLWComponent *_this, int id)
{
    for (vi_itr it = _this->idlers.begin(); it != _this->idlers.end(); ++it)
        if (*it == id) {
            _this->idlers.erase(it);
            glw_idler_table.erase(id);
            if (glw_idler_table.empty())
                glutIdleFunc(NULL);
            return 1;
        }
    return 0;
}

GLWActionFunc glwCompGetIdlerFunc(GLWComponent *_this,int _id)
{
    timer_itr it = glw_idler_table.find(_id);
    if (it != glw_idler_table.end()) {
        GLWTimerEntry& entry = (*it).second;
        return entry.func;
    }
    return NULL;
}

void *glwCompGetIdlerCtx(GLWComponent *_this,int _id)
{
    timer_itr it = glw_idler_table.find(_id);
    if (it != glw_idler_table.end()) {
        GLWTimerEntry& entry = (*it).second;
        return entry.ctx;
    }
    return NULL;
}

int glwCompGetIdlerCount(GLWComponent *_this)
{
    return (int)_this->idlers.size();
}

int glwCompGetIdlerID(GLWComponent *_this,int _idx)
{
    return _this->idlers[_idx];
}

void glwCompDisplay(GLWComponent *_this)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->display==NULL; cb=cb->super);
    if (cb!=NULL)cb->display(_this,cb);
}

void glwCompDisplayChildren(GLWComponent *_this)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->display_children==NULL; cb=cb->super);
    if (cb!=NULL)cb->display_children(_this,cb);
}

void glwCompValidate(GLWComponent *_this)
{
    if (!_this->valid) {
        const GLWCallbacks *cb;
        for (cb=_this->callbacks; cb!=NULL&&cb->validate==NULL; cb=cb->super);
        if (cb!=NULL)cb->validate(_this,cb);
        for (comps_itr it = _this->comps.begin(); it != _this->comps.end(); ++it) {
            glwCompValidate(*it);
        }
        _this->valid=1;
    }
}

void glwCompEnable(GLWComponent *_this,int _s)
{
    const GLWCallbacks *cb;
    _this->enabled=_s?1:0;
    if (!_s) {
        glwCompClearFocus(_this);
        glwCompClearCapture(_this);
    }
    for (cb=_this->callbacks; cb!=NULL&&cb->enable==NULL; cb=cb->super);
    if (cb!=NULL)cb->enable(_this,cb,_s);
}

void glwCompFocus(GLWComponent *_this,int _s)
{
    const GLWCallbacks *cb;
    _this->focused=_s?1:0;
    for (cb=_this->callbacks; cb!=NULL&&cb->focus==NULL; cb=cb->super);
    if (cb!=NULL)cb->focus(_this,cb,_s);
}

void glwCompVisibility(GLWComponent *_this,int _s)
{
    const GLWCallbacks *cb;
    _this->visible=_s?1:0;
    if (!_s) {
        glwCompClearFocus(_this);
        glwCompClearCapture(_this);
    }
    for (cb=_this->callbacks; cb!=NULL&&cb->visibility==NULL; cb=cb->super);
    if (cb!=NULL)cb->visibility(_this,cb,_s);
    if (_this->parent!=NULL)glwCompInvalidate(_this->parent);
}

void glwCompEntry(GLWComponent *_this,int _s)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->entry==NULL; cb=cb->super);
    if (cb!=NULL)cb->entry(_this,cb,_s);
}

int glwCompKeyboard(GLWComponent *_this,unsigned char _k,int _x,int _y)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->keyboard==NULL; cb=cb->super);
    if (cb!=NULL) {
        GLWComponent *focus;
        focus=_this->focus!=NULL?_this->focus:_this;
        if (glwCompIsFocusable(focus)&&!glwCompIsFocused(focus)) {
            glwCompFocus(focus,1);
        }
        return cb->keyboard(_this,cb,_k,_x,_y);
    }
    return 0;
}

int glwCompSpecial(GLWComponent *_this,int _k,int _x,int _y)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->special==NULL; cb=cb->super);
    if (cb!=NULL) {
        GLWComponent *focus;
        focus=_this->focus!=NULL?_this->focus:_this;
        if (glwCompIsFocusable(focus)&&!glwCompIsFocused(focus)) {
            glwCompFocus(focus,1);
        }
        return cb->special(_this,cb,_k,_x,_y);
    }
    return 0;
}

int glwCompMouse(GLWComponent *_this,int _b,int _s,int _x,int _y)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->mouse==NULL; cb=cb->super);
    if (cb!=NULL)return cb->mouse(_this,cb,_b,_s,_x,_y);
    return 0;
}

int glwCompMotion(GLWComponent *_this,int _x,int _y)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->motion==NULL; cb=cb->super);
    if (cb!=NULL)return cb->motion(_this,cb,_x,_y);
    return 0;
}

int glwCompPassiveMotion(GLWComponent *_this,int _x,int _y)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->passive_motion==NULL; cb=cb->super);
    if (cb!=NULL)return cb->passive_motion(_this,cb,_x,_y);
    return 0;
}

void glwCompDispose(GLWComponent *_this)
{
    const GLWCallbacks *cb;
    for (cb=_this->callbacks; cb!=NULL&&cb->dispose==NULL; cb=cb->super);
    if (cb!=NULL)cb->dispose(_this,cb);
}

void glwCompSuperDisplay(GLWComponent *_this,const GLWCallbacks *_cb)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->display==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->display(_this,_cb);
}

void glwCompSuperDisplayChildren(GLWComponent *_this,const GLWCallbacks *_cb)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->display_children==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->display_children(_this,_cb);
}

void glwCompSuperValidate(GLWComponent *_this,const GLWCallbacks *_cb)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->validate==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->validate(_this,_cb);
}

void glwCompSuperEnable(GLWComponent *_this,const GLWCallbacks *_cb,int _s)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->enable==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->enable(_this,_cb,_s);
}

void glwCompSuperFocus(GLWComponent *_this,const GLWCallbacks *_cb,int _s)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->focus==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->focus(_this,_cb,_s);
}

void glwCompSuperVisibility(GLWComponent *_this,const GLWCallbacks *_cb,
                            int _s)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->visibility==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->visibility(_this,_cb,_s);
}

void glwCompSuperEntry(GLWComponent *_this,const GLWCallbacks *_cb,int _s)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->entry==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->entry(_this,_cb,_s);
}

int glwCompSuperKeyboard(GLWComponent *_this,const GLWCallbacks *_cb,
                         unsigned char _k,int _x,int _y)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->keyboard==NULL; _cb=_cb->super);
    if (_cb!=NULL)return _cb->keyboard(_this,_cb,_k,_x,_y);
    return 0;
}

int glwCompSuperSpecial(GLWComponent *_this,const GLWCallbacks *_cb,
                        int _k,int _x,int _y)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->special==NULL; _cb=_cb->super);
    if (_cb!=NULL)return _cb->special(_this,_cb,_k,_x,_y);
    return 0;
}

int glwCompSuperMouse(GLWComponent *_this,const GLWCallbacks *_cb,
                      int _b,int _s,int _x,int _y)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->mouse==NULL; _cb=_cb->super);
    if (_cb!=NULL)return _cb->mouse(_this,_cb,_b,_s,_x,_y);
    return 0;
}

int glwCompSuperMotion(GLWComponent *_this,const GLWCallbacks *_cb,
                       int _x,int _y)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->motion==NULL; _cb=_cb->super);
    if (_cb!=NULL)return _cb->motion(_this,_cb,_x,_y);
    return 0;
}

int glwCompSuperPassiveMotion(GLWComponent *_this,const GLWCallbacks *_cb,
                              int _x,int _y)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->passive_motion==NULL; _cb=_cb->super);
    if (_cb!=NULL)return _cb->passive_motion(_this,_cb,_x,_y);
    return 0;
}

void glwCompSuperDispose(GLWComponent *_this,const GLWCallbacks *_cb)
{
    for (_cb=_cb->super; _cb!=NULL&&_cb->dispose==NULL; _cb=_cb->super);
    if (_cb!=NULL)_cb->dispose(_this,_cb);
}
