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

/*Checkbox component*/

# if !defined(M_PI)
#  define M_PI (3.141592653589793238462643)
# endif

# define GLW_CHECK_BOX_INSET (2)

GLWCheckBoxGroup::GLWCheckBoxGroup(void)
{
    this->seld=-1;
    this->changed=NULL;
    this->changed_ctx=NULL;
}

GLWCheckBoxGroup::~GLWCheckBoxGroup()
{
    this->changed = NULL;
    for (auto it = cbs.begin(); it != cbs.end(); ++it)
	    glwCheckBoxSetGroup(*it, NULL);
}

static int glwCheckBoxGroupAdd(GLWCheckBoxGroup* _this, GLWCheckBox* cb)
{
	_this->cbs.push_back(cb);
	return 1;
}

static int glwCheckBoxGroupDel(GLWCheckBoxGroup* _this, GLWCheckBox* cb)
{
	int i = 0;
	for (auto it = _this->cbs.begin(); it != _this->cbs.end(); ++it)
	{
		if (*it == cb)
		{
			_this->cbs.erase(it);
			if (i == _this->seld)
			{
				_this->seld=-1;
				if (_this->changed != NULL)
					_this->changed(_this->changed_ctx, 
						       NULL);
			}
			else if ((int)i<_this->seld)_this->seld--;
			return 1;
		}
		++i;
	}
	return 0;
}

int glwCheckBoxGroupGetSelectedIdx(GLWCheckBoxGroup *_this)
{
    return _this->seld;
}

void glwCheckBoxGroupSetSelectedIdx(GLWCheckBoxGroup* _this, int ii)
{
	if (ii >= 0 && (size_t)ii < _this->cbs.size())
	{
		glwCheckBoxSetState(_this->cbs[ii], 1);
	}
}

void glwCheckBoxGroupSelectNext(GLWCheckBoxGroup *_this)
{
	if ((size_t)_this->seld + 1 >= _this->cbs.size())
		glwCheckBoxGroupSetSelectedIdx(_this, 0);
	else 
		glwCheckBoxGroupSetSelectedIdx(_this, _this->seld + 1);
}

void glwCheckBoxGroupSelectPrev(GLWCheckBoxGroup *_this)
{
    if (_this->seld - 1 < 0)
	    glwCheckBoxGroupSetSelectedIdx(_this, (int)_this->cbs.size() - 1);
    else 
	    glwCheckBoxGroupSetSelectedIdx(_this, _this->seld - 1);
}

GLWActionFunc glwCheckBoxGroupGetChangedFunc(GLWCheckBoxGroup *_this)
{
    return _this->changed;
}

void glwCheckBoxGroupSetChangedFunc(GLWCheckBoxGroup *_this,
                                    GLWActionFunc _func)
{
    _this->changed=_func;
}

void *glwCheckBoxGroupGetChangedCtx(GLWCheckBoxGroup *_this)
{
    return _this->changed_ctx;
}

void glwCheckBoxGroupSetChangedCtx(GLWCheckBoxGroup *_this,void *_ctx)
{
    _this->changed_ctx=_ctx;
}



static void glwCheckBoxLayoutMinSize(GLWLayoutManager *_this,GLWCheckBox *_cb,
                                     int *_w,int *_h)
{
    int h;
    h=glwFontGetHeight(_cb->super.font);
    if (_w!=NULL)
    {
	    *_w = glwFontGetStringWidth(_cb->super.font, _cb->label.c_str())
		    + h + (GLW_CHECK_BOX_INSET<<2);
    }
    if (_h!=NULL)*_h=h+(GLW_CHECK_BOX_INSET<<1);
}

static GLWLayoutManager glw_check_box_layout=
{
    NULL,
    NULL,
    (GLWLayoutSizeFunc)glwCheckBoxLayoutMinSize,
    NULL,
    NULL
};

static void glwCheckBoxPeerDisplay(GLWCheckBox *_this,GLWCallbacks *_cb)
{
    double  y;
    int     w;
    int     h;
    int     fc;
    int     bc;
    glwCompSuperDisplay(&_this->super,_cb);
    /*Calculate text position and dimensions*/
    w = glwFontGetStringWidth(_this->super.font, _this->label.c_str());
    h=glwFontGetHeight(_this->super.font);
    y=(_this->super.bounds.h-h)*0.5;
    if (glwCompIsEnabled(&_this->super))
    {
        /*Draw focus rectangle if we have focus*/
        if (glwCompIsFocused(&_this->super))
        {
            glLineStipple(2,0x5555);
            glEnable(GL_LINE_STIPPLE);
            glBegin(GL_LINE_LOOP);
            glwColor(glwColorBlend(_this->super.forec,_this->super.backc));
            glVertex2d(h+GLW_CHECK_BOX_INSET*3-1,y-1);
            glVertex2d(h+GLW_CHECK_BOX_INSET*3-1,y+h);
            glVertex2d(h+GLW_CHECK_BOX_INSET*3+w,y+h);
            glVertex2d(h+GLW_CHECK_BOX_INSET*3+w,y-1);
            glEnd();
            glDisable(GL_LINE_STIPPLE);
        }
        glwColor(_this->super.forec);
        bc=glwColorBlend(_this->super.backc,glwColorInvert(_this->super.forec));
        fc=_this->super.forec;
    }
    else
    {
        glwColor(glwColorBlend(_this->super.forec,_this->super.backc));
        bc=glwColorBlend(_this->super.backc,_this->super.forec);
        fc=glwColorBlend(_this->super.forec,bc);
    }
    glwFontDrawString(_this->super.font, _this->label.c_str(), 
		      h + GLW_CHECK_BOX_INSET * 3,
                      y+glwFontGetDescent(_this->super.font));
    if (_this->group==NULL)                                  /*Draw the check box*/
    {
        glwColor(bc);
        glRectd(GLW_CHECK_BOX_INSET,y,h+GLW_CHECK_BOX_INSET,y+h);
        if (_this->state^_this->down)
        {
            glBegin(GL_LINES);
            glwColor(fc);
            glVertex2d(GLW_CHECK_BOX_INSET+1,y+2);
            glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+h-1);
            glVertex2d(GLW_CHECK_BOX_INSET+1,y+1);
            glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+h-2);
            glVertex2d(GLW_CHECK_BOX_INSET+2,y+1);
            glVertex2d(GLW_CHECK_BOX_INSET+h-1,y+h-2);
            glVertex2d(GLW_CHECK_BOX_INSET+1,y+h-1);
            glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+2);
            glVertex2d(GLW_CHECK_BOX_INSET+1,y+h-2);
            glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+1);
            glVertex2d(GLW_CHECK_BOX_INSET+0,y+h-2);
            glVertex2d(GLW_CHECK_BOX_INSET+h-3,y+1);
            glEnd();
        }
        glBegin(GL_LINES);
        glwColor(glwColorDarken(bc));
        glVertex2d(GLW_CHECK_BOX_INSET,y);
        glVertex2d(GLW_CHECK_BOX_INSET,y+h-1);
        glVertex2d(GLW_CHECK_BOX_INSET,y+h-1);
        glVertex2d(GLW_CHECK_BOX_INSET+h-1,y+h-1);
        glVertex2d(GLW_CHECK_BOX_INSET+1,y+1);
        glVertex2d(GLW_CHECK_BOX_INSET+1,y+h-2);
        glVertex2d(GLW_CHECK_BOX_INSET+1,y+h-2);
        glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+h-2);
        glwColor(glwColorLighten(bc));
        glVertex2d(GLW_CHECK_BOX_INSET+h-1,y+h-1);
        glVertex2d(GLW_CHECK_BOX_INSET+h-1,y);
        glVertex2d(GLW_CHECK_BOX_INSET+h-1,y);
        glVertex2d(GLW_CHECK_BOX_INSET+1,y);
        glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+h-2);
        glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+1);
        glVertex2d(GLW_CHECK_BOX_INSET+h-2,y+1);
        glVertex2d(GLW_CHECK_BOX_INSET+2,y+1);
        glEnd();
    }
    else                                                  /*Draw the radio button*/
    {
        double r;
        double m;
        int    detail;
        int    i;
        detail=h>>1;
        if (detail<4)detail=4;
        else if (detail&1)detail++;
        m=2*M_PI/detail;
        glMatrixMode(GL_MODELVIEW);
        glTranslated(GLW_CHECK_BOX_INSET+0.5*h,y+0.5*h,0);
        r=0.5*h;
        glwColor(bc);
        glBegin(GL_POLYGON);
        for (i=0; i<detail; i++)glVertex2d(r*cos(i*m),r*sin(i*m));
        glEnd();
        if (_this->state^_this->down)
        {
            r-=3;
            glwColor(fc);
            glBegin(GL_POLYGON);
            for (i=0; i<detail; i++)glVertex2d(r*cos(i*m),r*sin(i*m));
            glEnd();
            r+=3;
        }
        glBegin(GL_LINE_STRIP);
        glwColor(glwColorDarken(bc));
        for (i=0; i<=detail>>1; i++)
        {
            glVertex2d(r*cos(i*m+M_PI*0.25),r*sin(i*m+M_PI*0.25));
        }
        glwColor(glwColorLighten(bc));
        for (i=detail>>1; i<=detail; i++)
        {
            glVertex2d(r*cos(i*m+M_PI*0.25),r*sin(i*m+M_PI*0.25));
        }
        glEnd();
    }
}

static void glwCheckBoxPeerEnable(GLWCheckBox *_this,GLWCallbacks *_cb,
                                  int _s)
{
    glwCompSuperEnable(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static void glwCheckBoxPeerFocus(GLWCheckBox *_this,GLWCallbacks *_cb,int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwCompRepaint(&_this->super,0);
}

static int glwCheckBoxPeerKeyboard(GLWCheckBox *_this,const GLWCallbacks *_cb,
                                   unsigned char _k,int _x,int _y)
{
    if (_k==' '&&!_this->super.mouse_b)
    {
        glwCheckBoxSetState(_this,_this->state?0:1);
        glwCompRepaint(&_this->super,0);
        return -1;
    }
    return glwCompSuperKeyboard(&_this->super,_cb,_k,_x,_y);
}

static int glwCheckBoxPeerSpecial(GLWCheckBox *_this,const GLWCallbacks *_cb,
                                  int _k,int _x,int _y)
{
    if (!_this->super.mouse_b)
    {
        if (_this->group!=NULL)
        {
            int           i;
	    for (auto it = _this->group->cbs.begin(), i = 0; 
		 it != _this->group->cbs.end(); ++it, ++i)
		    if (*it == _this) break;
            switch (_k)
            {
            case GLUT_KEY_UP   :
            case GLUT_KEY_LEFT :
            {
                if (i>0)i--;
                else i = (int)_this->group->cbs.size() - 1;
                if (i>=0)
                {
                    if (_this->state)
			    glwCheckBoxSetState(_this->group->cbs[i], 1);
                    glwCompRequestFocus(&_this->group->cbs[i]->super);
                }
                return -1;
            }
            case GLUT_KEY_DOWN :
            case GLUT_KEY_RIGHT:
            {
		    if ((size_t)i + 1 < _this->group->cbs.size()) i++;
		    else i = 0;
		    if ((size_t)i < _this->group->cbs.size())
		    {
			    if (_this->state)
				    glwCheckBoxSetState(_this->group->cbs[i],
							1);
			    glwCompRequestFocus(&_this->group->cbs[i]->super);
		    }
		    return -1;
            }
            }
        }
        else switch (_k)
            {
            case GLUT_KEY_UP   :
            case GLUT_KEY_LEFT :
            {
                glwCompPrevFocus(&_this->super);
                return -1;
            }
            case GLUT_KEY_DOWN :
            case GLUT_KEY_RIGHT:
            {
                glwCompNextFocus(&_this->super);
                return -1;
            }
            }
    }
    return glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
}

static int glwCheckBoxPeerMouse(GLWCheckBox *_this,const GLWCallbacks *_cb,
                                int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0)
    {
        int rp;
        rp=0;
        if (_x<0||_x>_this->super.bounds.w||_y<0||_y>_this->super.bounds.h)
        {
            ret=-1;
            if (_this->down)
            {
                _this->down=0;
                rp=1;
            }
        }
        else if (!_s&&_this->down&&!_this->super.mouse_b)
        {
            ret=-1;
            _this->down=0;
            glwCheckBoxSetState(_this,_this->state?0:1);
            rp=1;
        }
        else if (_s||_this->super.mouse_b)
        {
            ret=-1;
            if (!_this->down)
            {
                _this->down=1;
                rp=1;
            }
        }
        if (rp)glwCompRepaint(&_this->super,0);
    }
    return ret;
}

static int glwCheckBoxPeerMotion(GLWCheckBox *_this,const GLWCallbacks *_cb,
                                 int _x,int _y)
{
    int ret;
    ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
    if (ret>=0)
    {
        int rp;
        rp=0;
        if (_x<0||_x>_this->super.bounds.w||_y<0||_y>_this->super.bounds.h)
        {
            ret=1;
            if (_this->down)
            {
                _this->down=0;
                rp=1;
            }
        }
        else if (_this->super.mouse_b)
        {
            ret=1;
            if (!_this->down)
            {
                _this->down=1;
                rp=1;
            }
        }
        if (rp)glwCompRepaint(&_this->super,0);
    }
    return ret;
}

static void glwCheckBoxPeerDispose(GLWCheckBox *_this,GLWCallbacks *_cb)
{
    glwCompSuperDispose(&_this->super,_cb);
    glwCheckBoxSetGroup(_this,NULL);
}


const GLWCallbacks GLW_CHECK_BOX_CALLBACKS=
{
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwCheckBoxPeerDispose,
    (GLWDisplayFunc)glwCheckBoxPeerDisplay,
    NULL,
    NULL,
    (GLWEnableFunc)glwCheckBoxPeerEnable,
    (GLWFocusFunc)glwCheckBoxPeerFocus,
    NULL,
    NULL,
    (GLWKeyboardFunc)glwCheckBoxPeerKeyboard,
    (GLWSpecialFunc)glwCheckBoxPeerSpecial,
    (GLWMouseFunc)glwCheckBoxPeerMouse,
    (GLWMotionFunc)glwCheckBoxPeerMotion,
    NULL
};

GLWCheckBox::GLWCheckBox(const char* label, int _state,
			 GLWCheckBoxGroup* _group)
{
	this->changed=NULL;
	this->changed_ctx=NULL;
	glwCheckBoxSetLabel(this, label);
        this->super.callbacks=&GLW_CHECK_BOX_CALLBACKS;
        glwCompSetLayout(&this->super,&glw_check_box_layout);
        glwCompSetFocusable(&this->super,1);
        glwCompSetAlignX(&this->super,0);
        this->state=_state?1:0;
        this->down=0;
        this->group=NULL;
	if (_group != NULL)
		glwCheckBoxSetGroup(this, _group);
}

int glwCheckBoxGetState(GLWCheckBox *_this)
{
    return _this->state;
}

void glwCheckBoxSetState(GLWCheckBox *_this,int _state)
{
    if (_this->state!=(_state?1U:0U))
    {
        if (_this->group!=NULL)
        {
            int idx;
            idx=_this->group->seld;
            if (_state&&_this->group->seld>=0)     /*Unselect a previously selected box*/
            {
                _this->group->seld=-1;
                glwCheckBoxSetState(_this->group->cbs[idx], 0);
            }
            if (_state)
            {
                if (_this->group->seld<0)              /*If nothing is selected, select us*/
                {
			int ii;
			for (auto it = _this->group->cbs.begin(), ii = 0;
			     it != _this->group->cbs.end(); ++it, ++ii)
				if (*it == _this)
				{
					_this->group->seld = (int)ii;
					break;
				}
			if (_this->group->seld<0) _state=0;
                }        /*We're not in the group! (BAD)*/
                /*If we tried to unselect a previously selected checkbox, and something
                  has remained selected, allow it to override the request to select this
                  checkbox*/
                else return;
            }
            else if (_this->group->seld>=0)_state=1;     /*Can't deselect selected item*/
            if (_this->state!=(_state?1U:0U))
            {
                _this->state=_state?1:0;
                if (_this->changed!=NULL)
                {
                    _this->changed(_this->changed_ctx,&_this->super);
                }
            }
            if (idx!=_this->group->seld&&_this->group->changed!=NULL)
            {
                _this->group->changed(_this->group->changed_ctx,&_this->super);
            }
        }
        else
        {
            _this->state=_state?1:0;
            if (_this->changed!=NULL)
            {
                _this->changed(_this->changed_ctx,&_this->super);
            }
        }
    }
}

const char *glwCheckBoxGetLabel(GLWCheckBox *_this)
{
	return _this->label.c_str();
}

int glwCheckBoxSetLabel(GLWCheckBox* _this, const char* label)
{
    if (label==NULL)
	    _this->label.clear();
    else
	    _this->label = label;
    glwCompRevalidate(&_this->super);
    return 1;
}

int glwCheckBoxAddLabel(GLWCheckBox* _this,const char* label)
{
	if (_this->label.size() <= 1)
		return glwCheckBoxSetLabel(_this, label);
	else if (label != NULL)
	{
		_this->label.append(label);
                glwCompRevalidate(&_this->super);
                return 1;
	}
        else return 1;
}

GLWCheckBoxGroup *glwCheckBoxGetGroup(GLWCheckBox *_this)
{
    return _this->group;
}

int glwCheckBoxSetGroup(GLWCheckBox *_this,GLWCheckBoxGroup *_group)
{
    if (_this->group!=NULL)glwCheckBoxGroupDel(_this->group,_this);
    _this->group=_group;
    if (_group!=NULL)
    {
        if (!glwCheckBoxGroupAdd(_this->group,_this))return 0;
        if (_this->state)
        {
            _this->state=0;
            glwCheckBoxSetState(_this,1);
            if (!_this->state)
            {
                _this->state=1;
                glwCheckBoxSetState(_this,0);
            }
        }
    }
    return 1;
}

GLWActionFunc glwCheckBoxGetChangedFunc(GLWCheckBox *_this)
{
    return _this->changed;
}

void glwCheckBoxSetChangedFunc(GLWCheckBox *_this,GLWActionFunc _func)
{
    _this->changed=_func;
}

void *glwCheckBoxGetChangedCtx(GLWCheckBox *_this)
{
    return _this->changed_ctx;
}

void glwCheckBoxSetChangedCtx(GLWCheckBox *_this,void *_ctx)
{
    _this->changed_ctx=_ctx;
}

