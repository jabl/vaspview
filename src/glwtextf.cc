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

/*Text field component. Although text may be selected, no clipboard or drag
  and drop capabilities are supported.*/

# define GLW_TEXT_FIELD_PAD   (12)
# define GLW_TEXT_FIELD_INSET (4)

static void glwTextFieldLayoutMinSize(GLWLayoutManager *_this,
                                      GLWTextField *_tf,int *_w,int *_h)
{
    if (_w!=NULL) {
        *_w=GLW_TEXT_FIELD_PAD+(GLW_TEXT_FIELD_INSET<<1);
        if (_tf->cols>0)*_w+=glwFontGetWidth(_tf->super.font,'0')*_tf->cols;
        else if (_tf->echo) {
            if (_tf->text.size() > 0) {
                *_w += glwFontGetWidth(_tf->super.font, _tf->echo)
                       * (_tf->text.size() - 1);
            }
        } else {
            *_w += glwFontGetStringWidth(_tf->super.font, _tf->text);
        }
    }
    if (_h!=NULL)*_h=glwFontGetHeight(_tf->super.font)+(GLW_TEXT_FIELD_INSET<<1);
}

static GLWLayoutManager glw_text_field_layout= {
    NULL,
    NULL,
    (GLWLayoutSizeFunc)glwTextFieldLayoutMinSize,
    NULL,
    NULL
};


static void glwTextFieldFixOffset(GLWTextField *_this)
{
    int fix;
    /*Figure out if we can see the character at the current caret position*/
    if (_this->offs<0)_this->offs=0;
    if (_this->offs>=_this->carp)fix=1;
    else if ((size_t)_this->offs + 1 < _this->text.size()) {
        int            i;
        int            w;
        w=_this->super.bounds.w-(GLW_TEXT_FIELD_INSET<<1);
        for (i = _this->offs; (size_t)i < _this->text.size() && w > 0; i++) {
            w -= glwFontGetWidth(_this->super.font, _this->echo ? _this->echo
                                 : _this->text[i]);
        }
        i--;
        if (i<_this->offs)i=_this->offs;
        if (i<_this->carp)fix=1;
        else fix=0;
    } else fix=0;
    /*Try to position caret in the middle of the text field*/
    if (fix) {
        int            i;
        int            w;
        w=(_this->super.bounds.w>>1)-GLW_TEXT_FIELD_INSET;
        for (i=_this->carp; i>0&&w>0; i--) {
            w -= glwFontGetWidth(_this->super.font, _this->echo ? _this->echo
                                 : _this->text[i]);
        }
        i++;
        if (i>_this->carp)i=_this->carp;
        _this->offs=i;
    }
}

static int glwTextFieldGetPosAt(GLWTextField *_this,int _x)
{
    int            ret;
    int            x;
    int            w;
    x=4;
    ret=_this->offs-1;
    if (ret < -1) ret = 1;
    w=0;
    while (_x+w>=x) {
        ret++;
        if ((size_t)ret > _this->text.size()) break;
        w = glwFontGetWidth(_this->super.font, _this->echo ? _this->echo
                            :_this->text[ret]);
        x+=w;
        w>>=1;
        if (x>_this->super.bounds.w-GLW_TEXT_FIELD_INSET)break;
    }
    if ((size_t)ret > _this->text.size())
        ret = (int)_this->text.size();
    if (ret<0)ret=0;
    return ret;
}


static void glwTextFieldBlink(void *_ctx,GLWTextField *_this)
{
    _this->blink_timer=0;
    _this->caret=_this->caret?0:1;
    glwCompRepaint(&_this->super,0);
}

static void glwTextFieldSetBlink(GLWTextField *_this)
{
    if (!_this->blink_timer) {
        _this->blink_timer=glwCompAddTimer(&_this->super,
                                           (GLWActionFunc)glwTextFieldBlink,NULL,
                                           500);
    }
}

static void glwTextFieldResetBlink(GLWTextField *_this)
{
    _this->caret=1;
    if (_this->blink_timer) {
        glwCompDelTimer(&_this->super,_this->blink_timer);
        _this->blink_timer=0;
    }
    glwCompRepaint(&_this->super,0);
}

static void glwTextFieldPeerDisplay(GLWTextField *_this,GLWCallbacks *_cb)
{
    double    y;
    double    yb;
    int       h;
    int       se;
    GLWcolor  c1;
    GLWcolor  c2;
    GLWcolor  bc;
    GLWcolor  fc;

    glwCompSuperDisplay(&_this->super,_cb);
    /*Calculate text position and dimensions*/
    std::string text;
    /*Position text if it is offset*/
    if (_this->offs > 0 && (size_t)_this->offs < _this->text.size()) {
        text = _this->text.substr(_this->offs);
    } else
        text = _this->text;
    /*w=glwFontGetStringWidth(_this->super.font,text);*/
    h=glwFontGetHeight(_this->super.font);
    y=(_this->super.bounds.h-h)*0.5;
    yb=y+glwFontGetDescent(_this->super.font);
    /*Figure out foreground, background, and border colors*/
    if (glwTextFieldIsEditable(_this)) {
        bc=glwColorBlend(_this->super.backc,glwColorInvert(_this->super.forec));
    } else bc=_this->super.backc;
    if (glwCompIsEnabled(&_this->super))fc=_this->super.forec;
    else {
        fc=glwColorBlend(_this->super.forec,_this->super.backc);
        bc=glwColorBlend(bc,fc);
    }
    c1=glwColorDarken(bc);
    c2=glwColorLighten(bc);
    glwColor(bc);
    glRecti(0,0,_this->super.bounds.w,_this->super.bounds.h);
    /*Draw the text*/
    glwColor(fc);
    if (_this->echo) {
        int i;
        int x;
        for (x=GLW_TEXT_FIELD_INSET,i=0;
                (size_t)i < _this->text.size() && x < _this->super.bounds.w;
                i++) {
            x+=glwFontDrawChar(_this->super.font,_this->echo,x,yb);
        }
    }

    else {
        glwFontDrawString(_this->super.font, text.c_str(),
                          GLW_TEXT_FIELD_INSET, yb);
    }
    if (glwCompIsFocused(&_this->super)) {
        /*Draw the selected portion*/
        se=_this->sele;
        if ((size_t)se<_this->text.size()) {
            int ss;
            ss=_this->sels;
            if (_this->offs>0) {
                ss-=_this->offs;
                se-=_this->offs;
            }
            if (ss<0)ss=0;
            if (se<0)se=0;
            if (ss<se) {
                int  i;
                int  x;
                int  w;
                for (x=GLW_TEXT_FIELD_INSET,i=0; i<ss; i++) {
                    x+=glwFontGetWidth(_this->super.font,
                                       _this->echo?_this->echo:(unsigned char)text[i]);
                }
                for (w=0; i<se; i++) {
                    w+=glwFontGetWidth(_this->super.font,
                                       _this->echo?_this->echo:(unsigned char)text[i]);
                }
                glRecti(x,0,x+w,_this->super.bounds.h);
                glwColor(bc);
                for (i=ss; i<se; i++) {
                    x+=glwFontDrawChar(_this->super.font,
                                       _this->echo?_this->echo:text[i],x,yb);
                }
                if (_this->carp<_this->sels||_this->carp>=_this->sele)glwColor(fc);
            }
        }
        /*Draw caret (cursor)*/
        if (_this->caret) {
            int       carp;
            carp=_this->carp;
            if ((size_t)_this->carp <= _this->text.size()) {
                if (_this->offs>0)carp-=_this->offs;
                if (carp>=0) {
                    int       w;
                    for (w=GLW_TEXT_FIELD_INSET; carp-->0;) {
                        w+=glwFontGetWidth(_this->super.font,
                                           _this->echo?_this->echo:(unsigned char)text[carp]);
                    }
                    glBegin(GL_LINES);
                    glVertex2d(w,y);
                    glVertex2d(w,y+h);
                    glEnd();
                }
            }
        }
        glwTextFieldSetBlink(_this);
    }
    /*Draw the border, using the appropriate colors to make us appear lowered*/
    glBegin(GL_LINES);
    glwColor(c1);
    glVertex2i(0,0);
    glVertex2i(0,_this->super.bounds.h-1);
    glVertex2i(0,_this->super.bounds.h-1);
    glVertex2i(_this->super.bounds.w-1,_this->super.bounds.h-1);
    glVertex2i(1,1);
    glVertex2i(1,_this->super.bounds.h-2);
    glVertex2i(1,_this->super.bounds.h-2);
    glVertex2i(_this->super.bounds.w-2,_this->super.bounds.h-2);
    glwColor(c2);
    glVertex2i(_this->super.bounds.w,_this->super.bounds.h-1);
    glVertex2i(_this->super.bounds.w,0);
    glVertex2i(_this->super.bounds.w,0);
    glVertex2i(1,0);
    glVertex2i(_this->super.bounds.w-1,_this->super.bounds.h-2);
    glVertex2i(_this->super.bounds.w-1,1);
    glVertex2i(_this->super.bounds.w-1,1);
    glVertex2i(2,1);
    glwColor(bc);
    glVertex2i(_this->super.bounds.w-2,2);
    glVertex2i(_this->super.bounds.w-2,_this->super.bounds.h-3);
    glVertex2i(_this->super.bounds.w-3,2);
    glVertex2i(_this->super.bounds.w-3,_this->super.bounds.h-3);
    glEnd();
}

static void glwTextFieldPeerEnable(GLWTextField *_this,GLWCallbacks *_cb,
                                   int _s)
{
    glwCompSuperEnable(&_this->super,_cb,_s);
    glwTextFieldResetBlink(_this);
}

static void glwTextFieldPeerFocus(GLWTextField *_this,GLWCallbacks *_cb,
                                  int _s)
{
    glwCompSuperFocus(&_this->super,_cb,_s);
    glwTextFieldResetBlink(_this);
    if (_s) glwTextFieldSelect(_this, 0, (int)_this->text.size());
}

static int glwTextFieldPeerKeyboard(GLWTextField *_this,
                                    const GLWCallbacks *_cb,
                                    unsigned char _k,int _x,int _y)
{
    int mods;
    mods=glutGetModifiers();
    if (!(mods&(GLUT_ACTIVE_CTRL|GLUT_ACTIVE_ALT))&&(_k==0x08||_k>=' ')) {
        if (!_this->super.mouse_b&&glwTextFieldIsEditable(_this)) {
            int   ss;
            int   se;
            ss=glwTextFieldGetSelectionStart(_this);
            se=glwTextFieldGetSelectionEnd(_this);
            if (ss < 0 || se < 0 || ss >= se
                    || (size_t)se > _this->text.size())
                ss=se=0;
            if (se - ss != 1 || _this->text[ss] != (char)_k) {      /*If there will be a net change...*/
                if (ss<se) {                             /*Any selection will get replaced*/
                    _this->carp=ss;
                    _this->text.erase(ss, se - ss);
                    _this->sels=_this->sele=-1;
                }
                if (_k==0x08) {                                                /*Backspace*/
                    if (ss >= se && _this->carp > 0
                            && (size_t)_this->carp <= _this->text.size()) {
                        size_t pos = --_this->carp;
                        _this->text.erase(pos, 1);
                    }
                } else if (_k==0x7F) {                                             /*Delete*/
                    if (ss >= se && _this->carp >= 0
                            && (size_t)_this->carp < _this->text.size()) {
                        _this->text.erase(_this->carp, 1);
                    }
                } else {
                    _this->text.insert(_this->carp, 1, _k);
                    _this->carp++;
                }
                glwTextFieldFixOffset(_this);
                glwTextFieldResetBlink(_this);
                if (_this->changed!=NULL)_this->changed(_this->changed_ctx,&_this->super);
            } else {
                _this->carp=se;
                _this->sels=_this->sele=-1;
            }
            _this->mark=-1;
        }
        return 1;
    } else if (_k=='\r'&&!mods) {
        if (_this->action!=NULL)_this->action(_this->action_ctx,&_this->super);
        return 1;
    } else return glwCompSuperKeyboard(&_this->super,_cb,_k,_x,_y);
}

static int glwTextFieldPeerSpecial(GLWTextField *_this,
                                   const GLWCallbacks *_cb,
                                   int _k,int _x,int _y)
{
    int mods;
    mods=glutGetModifiers();
    if (!(mods&GLUT_ACTIVE_ALT)&&
            (_k==GLUT_KEY_LEFT||_k==GLUT_KEY_RIGHT||
             _k==GLUT_KEY_HOME||_k==GLUT_KEY_END)) {
        if (!_this->super.mouse_b) {
            int carp;
            carp=_this->carp;
            if (mods&GLUT_ACTIVE_SHIFT) {
                if (_this->mark<0)_this->mark=carp;
            } else _this->sels=_this->sele=_this->mark=-1;
            switch (_k) {
            case GLUT_KEY_LEFT: {
                if (_this->carp>0)_this->carp--;
                if ((mods&GLUT_ACTIVE_CTRL)&&_this->carp>0&&
                        (size_t)_this->carp + 1 < _this->text.size()
                        && !_this->echo) {
                    std::string& text = _this->text;
                    if (isalnum(text[_this->carp])||text[_this->carp]=='_') {
                        while (_this->carp>0&&(isalnum(text[_this->carp-1])||
                                               text[_this->carp-1]=='_')) {
                            _this->carp--;
                        }
                    } else while (_this->carp>0&&!isalnum(text[_this->carp-1])&&
                                      text[_this->carp-1]!='_') {
                            _this->carp--;
                        }
                }
            }
            break;
            case GLUT_KEY_RIGHT: {
                if ((size_t)_this->carp < _this->text.size())
                    _this->carp++;
                if ((mods&GLUT_ACTIVE_CTRL)&&_this->carp>0&&
                        (size_t)_this->carp < _this->text.size()
                        && !_this->echo) {
                    std::string& text = _this->text;
                    if (isalnum(text[_this->carp-1])||text[_this->carp-1]=='_') {
                        while ((size_t)_this->carp < _this->text.size()
                                &&
                                (isalnum(text[_this->carp])
                                 || text[_this->carp] == '_')) {
                            _this->carp++;
                        }
                    } else while ((size_t)_this->carp < _this->text.size()
                                      &&
                                      !isalnum(text[_this->carp])
                                      && text[_this->carp]!='_') {
                            _this->carp++;
                        }
                }
            }
            break;
            case GLUT_KEY_HOME: {
                _this->carp=0;
                if ((mods&GLUT_ACTIVE_CTRL)&&!_this->echo) {
                    while ((size_t)_this->carp < _this->text.size()
                            && isspace(_this->text[_this->carp]))
                        _this->carp++;
                }
            }
            break;
            case GLUT_KEY_END : {
                _this->carp = (int)_this->text.size();
                if (_this->carp<0)_this->carp=0;
                if ((mods&GLUT_ACTIVE_CTRL)&&!_this->echo) {
                    while (_this->carp > 0
                            && isspace(_this->text[_this->carp - 1])) {
                        _this->carp--;
                    }
                }
            }
            break;
            }
            if (_this->carp!=carp) {
                if (mods&GLUT_ACTIVE_SHIFT) {
                    if (_this->carp<=_this->mark) {
                        _this->sels=_this->carp;
                        _this->sele=_this->mark;
                    } else {
                        _this->sels=_this->mark;
                        _this->sele=_this->carp;
                    }
                }
                glwTextFieldFixOffset(_this);
                glwTextFieldResetBlink(_this);
            }
        }
        return 1;
    } else return glwCompSuperSpecial(&_this->super,_cb,_k,_x,_y);
}

static int glwTextFieldPeerMouse(GLWTextField *_this,const GLWCallbacks *_cb,
                                 int _b,int _s,int _x,int _y)
{
    int ret;
    ret=glwCompSuperMouse(&_this->super,_cb,_b,_s,_x,_y);
    if (ret>=0&&_b==GLUT_LEFT_BUTTON) {
        int mark;
        mark=glwTextFieldGetPosAt(_this,_x);
        if (_s)_this->mark=mark;
        if (mark<=_this->mark) {
            _this->sels=mark;
            _this->sele=_this->mark;
        } else {
            _this->sels=_this->mark;
            _this->sele=mark;
        }
        _this->carp=mark;
        glwTextFieldFixOffset(_this);
        glwTextFieldResetBlink(_this);
        ret=1;
    }
    return ret;
}

static int glwTextFieldPeerMotion(GLWTextField *_this,const GLWCallbacks *_cb,
                                  int _x,int _y)
{
    int ret;
    ret=glwCompSuperMotion(&_this->super,_cb,_x,_y);
    if (ret>=0&&(_this->super.mouse_b&1<<GLUT_LEFT_BUTTON)) {
        int mark;
        mark=glwTextFieldGetPosAt(_this,_x);
        if (mark<=_this->mark) {
            _this->sels=mark;
            _this->sele=_this->mark;
        } else {
            _this->sels=_this->mark;
            _this->sele=mark;
        }
        _this->carp=mark;
        glwTextFieldFixOffset(_this);
        glwTextFieldResetBlink(_this);
        ret=1;
    }
    return ret;
}

static void glwTextFieldPeerDispose(GLWTextField *_this,GLWCallbacks *_cb)
{
    glwCompSuperDispose(&_this->super,_cb);
}


const GLWCallbacks GLW_TEXT_FIELD_CALLBACKS= {
    &GLW_COMPONENT_CALLBACKS,
    (GLWDisposeFunc)glwTextFieldPeerDispose,
    (GLWDisplayFunc)glwTextFieldPeerDisplay,
    NULL,
    NULL,
    (GLWEnableFunc)glwTextFieldPeerEnable,
    (GLWFocusFunc)glwTextFieldPeerFocus,
    NULL/*(GLWVisibilityFunc)glwTextFieldPeerVisibility*/,
    NULL,
    (GLWKeyboardFunc)glwTextFieldPeerKeyboard,
    (GLWSpecialFunc)glwTextFieldPeerSpecial,
    (GLWMouseFunc)glwTextFieldPeerMouse,
    (GLWMotionFunc)glwTextFieldPeerMotion,
    NULL
};


GLWTextField::GLWTextField(const std::string& text, int _cols)
{
    this->changed=NULL;
    this->changed_ctx=NULL;
    this->action=NULL;
    this->action_ctx=NULL;
    this->blink_timer=0;
    this->offs = this->carp = 0;
    this->mark = this->sels = this->sele = -1;
    glwTextFieldSetText(this, text);
    this->super.callbacks=&GLW_TEXT_FIELD_CALLBACKS;
    glwCompSetFont(&this->super,glwFontGet(GLW_FONT_FIXED,0));
    glwCompSetCursor(&this->super,GLUT_CURSOR_TEXT);
    glwCompSetLayout(&this->super,&glw_text_field_layout);
    glwCompSetFocusable(&this->super,1);
    this->cols=_cols;
    this->echo=0;
    this->editable=1;
}

int glwTextFieldIsEditable(GLWTextField *_this)
{
    return _this->editable;
}

void glwTextFieldSetEditable(GLWTextField *_this,int _b)
{
    if ((_this->editable && !_b) || (!_this->editable && _b)) {
        _this->editable=_b?1:0;
        glwCompRepaint(&_this->super,0);
    }
}

const char* glwTextFieldGetText(GLWTextField *_this)
{
    return _this->text.c_str();
}

int glwTextFieldSetText(GLWTextField *_this, const std::string& text)
{
    if (_this->text != text) {
        _this->text = text;
        _this->offs=_this->carp=0;
        _this->mark=_this->sels=_this->sele=-1;
        glwCompRevalidate(&_this->super);
        if (_this->changed != NULL)
            _this->changed(_this->changed_ctx, &_this->super);
        return 1;
    }
    return 0;
}

int glwTextFieldAddText(GLWTextField *_this, const std::string& text)
{
    if (!text.empty()) {
        _this->text.append(text);
        _this->offs=_this->carp=0;
        _this->mark=_this->sels=_this->sele=-1;
        glwCompRevalidate(&_this->super);
        if (_this->changed != NULL)
            _this->changed(_this->changed_ctx,&_this->super);
        return 1;
    }
    return 0;
}

const char *glwTextFieldGetSelectedText(GLWTextField *_this)
{
    size_t len;
    if (_this->sels<0||_this->sele<0||
            _this->sels >= _this->sele
            ||(size_t)_this->sele + 1 >= _this->text.size())
        len=0;
    else len=_this->sele-_this->sels;
    _this->seld = _this->text.substr(_this->sels, len);
    return _this->seld.c_str();
}

int glwTextFieldGetCaretPos(GLWTextField *_this)
{
    return _this->carp;
}

void glwTextFieldSetCaretPos(GLWTextField *_this,int _carp)
{
    if ((size_t)_carp > _this->text.size())
        _carp = (int)_this->text.size();
    if (_carp<0)_carp=0;
    if (_carp!=_this->carp) {
        _this->carp=_carp;
        glwTextFieldFixOffset(_this);
        glwTextFieldResetBlink(_this);
    }
}

int glwTextFieldGetSelectionStart(GLWTextField *_this)
{
    return _this->sels;
}

void glwTextFieldSetSelectionStart(GLWTextField *_this,int _sels)
{
    if ((size_t)_sels > _this->text.size())
        _sels = (int)_this->text.size();
    if (_sels<0)_sels=0;
    if (_sels!=_this->sels) {
        _this->sels=_sels;
        glwCompRepaint(&_this->super,0);
    }
}

int glwTextFieldGetSelectionEnd(GLWTextField *_this)
{
    return _this->sele;
}

void glwTextFieldSetSelectionEnd(GLWTextField *_this,int _sele)
{
    if ((size_t)_sele > _this->text.size())
        _sele = (int)_this->text.size();
    if (_sele<0)_sele=0;
    if (_sele!=_this->sele) {
        _this->sele=_sele;
        glwCompRepaint(&_this->super,0);
    }
}

void glwTextFieldSelect(GLWTextField *_this,int _sels,int _sele)
{
    if ((size_t)_sels > _this->text.size())
        _sels = (int)_this->text.size();
    if (_sels<0)_sels=0;
    if ((size_t)_sele > _this->text.size())
        _sele = (int)_this->text.size();
    if (_sele<0)_sele=0;
    if (_sels!=_this->sels||_sele!=_this->sele) {
        _this->sels=_sels;
        _this->sele=_sele;
        glwCompRepaint(&_this->super,0);
    }
}

int glwTextFieldGetCols(GLWTextField *_this)
{
    return _this->cols;
}

void glwTextFieldSetCols(GLWTextField *_this,int _cols)
{
    _this->cols=_cols;
}

int glwTextFieldGetEchoChar(GLWTextField *_this)
{
    return _this->echo;
}

void glwTextFieldSetEchoChar(GLWTextField *_this,int _echo)
{
    _this->echo=_echo;
}

GLWActionFunc glwTextFieldGetActionFunc(GLWTextField *_this)
{
    return _this->action;
}

void glwTextFieldSetActionFunc(GLWTextField *_this,GLWActionFunc _func)
{
    _this->action=_func;
}

void *glwTextFieldGetActionCtx(GLWTextField *_this)
{
    return _this->action_ctx;
}

void glwTextFieldSetActionCtx(GLWTextField *_this,void *_ctx)
{
    _this->action_ctx=_ctx;
}

GLWActionFunc glwTextFieldGetChangedFunc(GLWTextField *_this)
{
    return _this->changed;
}

void glwTextFieldSetChangedFunc(GLWTextField *_this,GLWActionFunc _func)
{
    _this->changed=_func;
}

void *glwTextFieldGetChangedCtx(GLWTextField *_this)
{
    return _this->changed_ctx;
}

void glwTextFieldSetChangedCtx(GLWTextField *_this,void *_ctx)
{
    _this->changed_ctx=_ctx;
}
