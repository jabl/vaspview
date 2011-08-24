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

#if !defined(_glw_H)
# define _glw_H (1)

#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <unordered_map>
#include <string>
#include "DynArray.hh"

/*This is a small, portable component library that is dependent only on OpenGL
  and GLUT. It is not very general purpose, and in order to be made so
  requires at least:
  1) An event queue
  2) Event listeners
  3) Overlay support (menus, choices, dialogs)
  4) Peer abtraction to limit GLUT dependency (outside of glwcomp, and
     possibly glwframe, no GLUT-specific code should be required)
  5) Macros to avoid casting derived structures
  The only components included are those required by this project. Note that
  this library does not support a multi-threading environtment*/

/*Common colors*/
# define GLW_COLOR_BLACK      (0xFF000000)
# define GLW_COLOR_DARK_GREY  (0xFF3F3F3F)
# define GLW_COLOR_GREY       (0xFF7F7F7F)
# define GLW_COLOR_LIGHT_GREY (0xFFBFBFBF)
# define GLW_COLOR_WHITE      (0xFFFFFFFF)
# define GLW_COLOR_RED        (0xFF0000FF)
# define GLW_COLOR_GREEN      (0xFF00FF00)
# define GLW_COLOR_BLUE       (0xFFFF0000)
/*Font families*/
# define GLW_FONT_FIXED      (0)
# define GLW_FONT_SERIF      (1)
# define GLW_FONT_SANS_SERIF (2)

typedef        unsigned long         GLWcolor;
typedef        void                 *GLWfont;
typedef        int                   GLWcursor;
typedef struct GLWRect               GLWRect;
typedef struct GLWInsets             GLWInsets;
typedef struct GLWCallbacks          GLWCallbacks;
typedef struct GLWConstraints        GLWConstraints;
typedef struct GLWLayoutManager      GLWLayoutManager;
typedef struct GLWGridBagLayout      GLWGridBagLayout;
typedef struct GLWComponent          GLWComponent;
typedef struct GLWFrame              GLWFrame;
typedef struct GLWLabel              GLWLabel;
typedef struct GLWButton             GLWButton;
typedef struct GLWTextField          GLWTextField;
typedef struct GLWCheckBox           GLWCheckBox;
typedef struct GLWCheckBoxGroup      GLWCheckBoxGroup;
typedef struct GLWSlider             GLWSlider;
typedef struct GLWTabbedPane         GLWTabbedPane;



struct GLWRect
{
    int x;
    int y;
    int w;
    int h;
};



struct GLWInsets
{
    int t;
    int b;
    int l;
    int r;
};



typedef void (*GLWActionFunc)(void *_ctx,GLWComponent *_this);



typedef void (*GLWDisplayFunc)(GLWComponent *_this,const GLWCallbacks *_cb);
typedef void (*GLWValidateFunc)(GLWComponent *_this,const GLWCallbacks *_cb);
typedef void (*GLWEnableFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                              int _s);
typedef void (*GLWFocusFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                             int _s);
typedef void (*GLWVisibilityFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                                  int _s);
typedef void (*GLWEntryFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                             int _s);
typedef int  (*GLWKeyboardFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                                unsigned char _k,int _x,int _y);
typedef int  (*GLWSpecialFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                               int _k,int _x,int _y);
typedef int  (*GLWMouseFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                             int _b,int _s,int _x,int _y);
typedef int  (*GLWMotionFunc)(GLWComponent *_this,const GLWCallbacks *_cb,
                              int _x,int _y);
typedef int  (*GLWDisposeFunc)(GLWComponent *_this,const GLWCallbacks *_cb);

struct GLWCallbacks
{
    const GLWCallbacks      *super;
    GLWDisposeFunc     dispose;
    GLWDisplayFunc     display;
    GLWDisplayFunc     display_children;
    GLWValidateFunc    validate;
    GLWEnableFunc      enable;
    GLWFocusFunc       focus;
    GLWVisibilityFunc  visibility;
    GLWEntryFunc       entry;
    GLWKeyboardFunc    keyboard;
    GLWSpecialFunc     special;
    GLWMouseFunc       mouse;
    GLWMotionFunc      motion;
    GLWMotionFunc      passive_motion;
};

# define GLWC_RELATIVE   (-1)
# define GLWC_REMAINDER  (0)
# define GLWC_NONE       (0)
# define GLWC_VERTICAL   (1)
# define GLWC_HORIZONTAL (2)
# define GLWC_BOTH       (3)
# define GLWC_CENTER     (0)
# define GLWC_TOP        (1)
# define GLWC_LEFT       (4)
# define GLWC_BOTTOM     (2)
# define GLWC_RIGHT      (8)
# define GLWC_NORTH      (1)
# define GLWC_NORTHEAST  (9)
# define GLWC_EAST       (8)
# define GLWC_SOUTHEAST  (10)
# define GLWC_SOUTH      (2)
# define GLWC_SOTHWEST   (6)
# define GLWC_WEST       (4)
# define GLWC_NORTHWEST  (5)

# define GLWC_SNAP_ALWAYS (-1)
# define GLWC_SNAP_NEVER  (0)

struct GLWConstraints
{
    int       minw;
    int       minh;
    int       prew;
    int       preh;
    int       maxw;
    int       maxh;
    GLWInsets insets;
    double    alignx;
    double    aligny;
    int       fill;
    int       gridx;
    int       gridy;
    int       gridw;
    int       gridh;
    double    weightx;
    double    weighty;
};

typedef void (*GLWLayoutFunc)(GLWLayoutManager *_this,GLWComponent *_comp);
typedef void (*GLWLayoutSizeFunc)(GLWLayoutManager *_this,GLWComponent *_comp,
                                  int *_w,int *_h);

struct GLWLayoutManager
{
    GLWLayoutFunc     layout;
    GLWLayoutFunc     invalidate;
    GLWLayoutSizeFunc min_size;
    GLWLayoutSizeFunc pre_size;
    GLWLayoutSizeFunc max_size;
    GLWLayoutFunc     dispose;
};



typedef struct GLWGBLCInfo
{
    int min_w;
    int min_h;
    int pre_w;
    int pre_h;
    int max_w;
    int max_h;
    int x;
    int y;
    int w;
    int h;
} GLWGBLCInfo;

struct GLWGridBagLayout
{
    GLWLayoutManager  super;
    int               w;
    int               h;
    int               start_x;
    int               start_y;
    int              *min_w;
    int              *min_h;
    int              *pre_w;
    int              *pre_h;
    int              *adj_w;
    int              *adj_h;
    double           *weight_x;
    double           *weight_y;
    GLWGBLCInfo      *cache;
    GLWComponent     *comp;
    unsigned          valid:1;
    unsigned          validating:1;
};


GLWGridBagLayout *glwGridBagLayoutAlloc(void);
void              glwGridBagLayoutInit(GLWGridBagLayout *_this);
void              glwGridBagLayoutDstr(GLWGridBagLayout *_this);
void              glwGridBagLayoutFree(GLWGridBagLayout *_this);



struct GLWComponent
{
    GLWComponent     *parent;
    const GLWCallbacks     *callbacks;
    GLWRect           bounds;
    GLWConstraints    constraints;
    GLWcolor          forec;
    GLWcolor          backc;
    GLWfont           font;
    GLWcursor         cursor;
    CDynArray         comps;
    CDynArray         timers;
    CDynArray         idlers;
    GLWLayoutManager *layout;
    GLWComponent     *focus;                 /*Currently focused descendant*/
    GLWComponent     *capture;           /*Current child with mouse capture*/
    unsigned          mouse_b;                         /*Mouse button state*/
    int               wid;                                 /*GLUT Window id*/
    unsigned          visible:1;
    unsigned          enabled:1;
    unsigned          valid:1;
    unsigned          validate_root:1;
    unsigned          focused:1;
    unsigned          focusable:1;
};


extern const GLWCallbacks GLW_COMPONENT_CALLBACKS;

void           glwCompInit(GLWComponent *_this);
GLWComponent  *glwCompAlloc(void);
void           glwCompDstr(GLWComponent *_this);
void           glwCompFree(GLWComponent *_this);

int            glwCompIsVisible(GLWComponent *_this);
int            glwCompIsShowing(GLWComponent *_this);
int            glwCompIsEnabled(GLWComponent *_this);
int            glwCompIsFocused(GLWComponent *_this);
int            glwCompIsFocusable(GLWComponent *_this);
int            glwCompIsFocusTraversable(GLWComponent *_this);
int            glwCompIsCapturing(GLWComponent *_this);
void           glwCompSetBounds(GLWComponent *_this,int _x,int _y,
                                int _w,int _h);
void           glwCompSetForeColor(GLWComponent *_this,GLWcolor _c);
void           glwCompSetBackColor(GLWComponent *_this,GLWcolor _c);
void           glwCompSetFont(GLWComponent *_this,GLWfont _font);
void           glwCompSetCursor(GLWComponent *_this,GLWcursor _cursor);
void           glwCompSetFocusable(GLWComponent *_this,int _b);

void           glwCompSetMinWidth(GLWComponent *_this,int _min_w);
void           glwCompSetMinHeight(GLWComponent *_this,int _min_h);
void           glwCompSetMaxWidth(GLWComponent *_this,int _max_w);
void           glwCompSetMaxHeight(GLWComponent *_this,int _max_h);
void           glwCompSetPreWidth(GLWComponent *_this,int _pre_w);
void           glwCompSetPreHeight(GLWComponent *_this,int _pre_h);
void           glwCompSetInsets(GLWComponent *_this,int _t,int _b,
                                int _l,int _r);
void           glwCompSetAlignX(GLWComponent *_this,double _align_x);
void           glwCompSetAlignY(GLWComponent *_this,double _align_y);
void           glwCompSetFill(GLWComponent *_this,int _fill);
void           glwCompSetGridX(GLWComponent *_this,int _grid_x);
void           glwCompSetGridY(GLWComponent *_this,int _grid_y);
void           glwCompSetGridWidth(GLWComponent *_this,int _grid_w);
void           glwCompSetGridHeight(GLWComponent *_this,int _grid_h);
void           glwCompSetWeightX(GLWComponent *_this,double _weight_x);
void           glwCompSetWeightY(GLWComponent *_this,double _weight_y);

void           glwCompGetMinSize(GLWComponent *_this,int *_w,int *_h);
void           glwCompGetMaxSize(GLWComponent *_this,int *_w,int *_h);
void           glwCompGetPreSize(GLWComponent *_this,int *_w,int *_h);
void           glwCompInvalidate(GLWComponent *_this);
void           glwCompRevalidate(GLWComponent *_this);
void           glwCompRepaint(GLWComponent *_this,unsigned _millis);
int            glwCompAdd(GLWComponent *_this,GLWComponent *_comp,int _idx);
int            glwCompDel(GLWComponent *_this,GLWComponent *_comp);
void           glwCompDelAll(GLWComponent *_this);
void           glwCompSetLayout(GLWComponent *_this,GLWLayoutManager *_layout);
void           glwCompLayout(GLWComponent *_this);
void           glwCompNextFocus(GLWComponent *_this);
void           glwCompPrevFocus(GLWComponent *_this);
void           glwCompClearFocus(GLWComponent *_this);
void           glwCompRequestFocus(GLWComponent *_this);
void           glwCompTransferCapture(GLWComponent *_this,int _x,int _y);
void           glwCompClearCapture(GLWComponent *_this);
void           glwCompRequestCapture(GLWComponent *_this,GLWComponent *_that);
GLWComponent  *glwCompGetComponentAt(GLWComponent *_this,int _x,int _y);
GLWComponent  *glwCompFindComponentAt(GLWComponent *_this,int _x,int _y);

int            glwCompAddTimer(GLWComponent *_this,GLWActionFunc _func,
                               void *_ctx,unsigned _millis);
int            glwCompDelTimer(GLWComponent *_this,int _id);
GLWActionFunc  glwCompGetTimerFunc(GLWComponent *_this,int _id);
void          *glwCompGetTimerCtx(GLWComponent *_this,int _id);
int            glwCompGetTimerCount(GLWComponent *_this);
int            glwCompGetTimerID(GLWComponent *_this,int _idx);
int            glwCompAddIdler(GLWComponent *_this,GLWActionFunc _func,
                               void *_ctx);
int            glwCompDelIdler(GLWComponent *_this,int _id);
GLWActionFunc  glwCompGetIdlerFunc(GLWComponent *_this,int _id);
void          *glwCompGetIdlerCtx(GLWComponent *_this,int _id);
int            glwCompGetIdlerCount(GLWComponent *_this);
int            glwCompGetIdlerID(GLWComponent *_this,int _idx);

void           glwCompDisplay(GLWComponent *_this);
void           glwCompDisplayChildren(GLWComponent *_this);
void           glwCompValidate(GLWComponent *_this);
void           glwCompEnable(GLWComponent *_this,int _s);
void           glwCompFocus(GLWComponent *_this,int _s);
void           glwCompVisibility(GLWComponent *_this,int _s);
void           glwCompEntry(GLWComponent *_this,int _s);
int            glwCompKeyboard(GLWComponent *_this,
                               unsigned char _k,int _x,int _y);
int            glwCompSpecial(GLWComponent *_this,int _k,int _x,int _y);
int            glwCompMouse(GLWComponent *_this,int _b,int _s,int _x,int _y);
int            glwCompMotion(GLWComponent *_this,int _x,int _y);
int            glwCompPassiveMotion(GLWComponent *_this,int _x,int _y);
void           glwCompDispose(GLWComponent *_this);
void           glwCompSuperDisplay(GLWComponent *_this,
                                   const GLWCallbacks *_cb);
void           glwCompSuperDisplayChildren(GLWComponent *_this,
        const GLWCallbacks *_cb);
void           glwCompSuperValidate(GLWComponent *_this,
                                    const GLWCallbacks *_cb);
void           glwCompSuperEnable(GLWComponent *_this,const GLWCallbacks *_cb,
                                  int _s);
void           glwCompSuperFocus(GLWComponent *_this,const GLWCallbacks *_cb,
                                 int _s);
void           glwCompSuperVisibility(GLWComponent *_this,
                                      const GLWCallbacks *_cb,int _s);
void           glwCompSuperEntry(GLWComponent *_this,const GLWCallbacks *_cb,
                                 int _s);
int            glwCompSuperKeyboard(GLWComponent *_this,
                                    const GLWCallbacks *_cb,unsigned char _k,
                                    int _x,int _y);
int            glwCompSuperSpecial(GLWComponent *_this,
                                   const GLWCallbacks *_cb,
                                   int _k,int _x,int _y);
int            glwCompSuperMouse(GLWComponent *_this,const GLWCallbacks *_cb,
                                 int _b,int _s,int _x,int _y);
int            glwCompSuperMotion(GLWComponent *_this,const GLWCallbacks *_cb,
                                  int _x,int _y);
int            glwCompSuperPassiveMotion(GLWComponent *_this,
        const GLWCallbacks *_cb,
        int _x,int _y);
void           glwCompSuperDispose(GLWComponent *_this,
                                   const GLWCallbacks *_cb);



struct GLWFrame
{
    GLWComponent  super;
    CDynArray     title;
};


extern const GLWCallbacks GLW_FRAME_CALLBACKS;

GLWFrame *glwFrameAlloc(const char *_title);
int       glwFrameInit(GLWFrame *_this,const char *_title);
void      glwFrameDstr(GLWFrame *_this);
void      glwFrameFree(GLWFrame *_this);

int       glwFrameSetTitle(GLWFrame *_this,const char *_title);
const char     *glwFrameGetTitle(GLWFrame *_this);
void      glwFrameShow(GLWFrame *_this);
void      glwFrameHide(GLWFrame *_this);
void      glwFramePack(GLWFrame *_this);



struct GLWLabel
{
    GLWComponent super;
    CDynArray    label;
};


extern const GLWCallbacks GLW_LABEL_CALLBACKS;

GLWLabel *glwLabelAlloc(const char *_label);
int       glwLabelInit(GLWLabel *_this,const char *_label);
void      glwLabelDstr(GLWLabel *_this);
void      glwLabelFree(GLWLabel *_this);

const char     *glwLabelGetLabel(GLWLabel *_this);
int       glwLabelSetLabel(GLWLabel *_this,const char *_label);
int       glwLabelAddLabel(GLWLabel *_this,const char *_label);



struct GLWButton
{
    GLWComponent   super;
    CDynArray      label;
    GLWActionFunc  pressed;
    void          *pressed_ctx;
    unsigned       down:1;
    unsigned       release:1;
};


extern const GLWCallbacks GLW_BUTTON_CALLBACKS;

GLWButton     *glwButtonAlloc(const char *_label);
int            glwButtonInit(GLWButton *_this,const char *_label);
void           glwButtonDstr(GLWButton *_this);
void           glwButtonFree(GLWButton *_this);

const char          *glwButtonGetLabel(GLWButton *_this);
int            glwButtonSetLabel(GLWButton *_this,const char *_label);
int            glwButtonAddLabel(GLWButton *_this,const char *_label);
GLWActionFunc  glwButtonGetPressedFunc(GLWButton *_this);
void           glwButtonSetPressedFunc(GLWButton *_this,
                                       GLWActionFunc _func);
void          *glwButtonGetPressedCtx(GLWButton *_this);
void           glwButtonSetPressedCtx(GLWButton *_this,void *_ctx);


struct GLWTextField
{
    GLWComponent   super;
    GLWActionFunc  action;                      /*Enter pressed callback function*/
    void          *action_ctx;                     /*Extra parameter for callback*/
    GLWActionFunc  changed;                      /*Text changed callback function*/
    void          *changed_ctx;                    /*Extra parameter for callback*/
    CDynArray      text;                                           /*Current text*/
    CDynArray      seld;                         /*Currently selected text buffer*/
    int            sels;                                        /*Selection start*/
    int            sele;                                          /*Selection end*/
    int            carp;                                         /*Caret position*/
    int            mark;               /*Selection mark (for dragging selections)*/
    int            offs;                                         /*Display offset*/
    int            cols;                            /*Preferred number of columns*/
    int            echo;                                         /*Echo character*/
    int            blink_timer;                       /*Timer for blinking cursor*/
    unsigned       editable:1;
    unsigned       caret:1;
};                                       /*Draw caret?*/


extern const GLWCallbacks GLW_TEXT_FIELD_CALLBACKS;

GLWTextField  *glwTextFieldAlloc(const char *_text,int _cols);
int            glwTextFieldInit(GLWTextField *_this,
                                const char *_text,int _cols);
void           glwTextFieldDstr(GLWTextField *_this);
void           glwTextFieldFree(GLWTextField *_this);

int            glwTextFieldIsEditable(GLWTextField *_this);
void           glwTextFieldSetEditable(GLWTextField *_this,int _b);
const char          *glwTextFieldGetText(GLWTextField *_this);
int            glwTextFieldSetText(GLWTextField *_this,
                                   const char *_text);
int            glwTextFieldAddText(GLWTextField *_this,
                                   const char *_text);
const char          *glwTextFieldGetSelectedText(GLWTextField *_this);
int            glwTextFieldGetCaretPos(GLWTextField *_this);
void           glwTextFieldSetCaretPos(GLWTextField *_this,int _carp);
int            glwTextFieldGetSelectionStart(GLWTextField *_this);
void           glwTextFieldSetSelectionStart(GLWTextField *_this,
        int _sels);
int            glwTextFieldGetSelectionEnd(GLWTextField *_this);
void           glwTextFieldSetSelectionEnd(GLWTextField *_this,
        int _sele);
void           glwTextFieldSelect(GLWTextField *_this,
                                  int _sels,int _sele);
int            glwTextFieldGetCols(GLWTextField *_this);
void           glwTextFieldSetCols(GLWTextField *_this,int _cols);
int            glwTextFieldGetEchoChar(GLWTextField *_this);
void           glwTextFieldSetEchoChar(GLWTextField *_this,int _echo);
GLWActionFunc  glwTextFieldGetActionFunc(GLWTextField *_this);
void           glwTextFieldSetActionFunc(GLWTextField *_this,
        GLWActionFunc _func);
void          *glwTextFieldGetActionCtx(GLWTextField *_this);
void           glwTextFieldSetActionCtx(GLWTextField *_this,void *_ctx);
GLWActionFunc  glwTextFieldGetChangedFunc(GLWTextField *_this);
void           glwTextFieldSetChangedFunc(GLWTextField *_this,
        GLWActionFunc _func);
void          *glwTextFieldGetChangedCtx(GLWTextField *_this);
void           glwTextFieldSetChangedCtx(GLWTextField *_this,
        void *_ctx);



struct GLWCheckBoxGroup
{
    CDynArray      cbs;
    int            seld;
    GLWActionFunc  changed;
    void          *changed_ctx;
};


GLWCheckBoxGroup *glwCheckBoxGroupAlloc(void);
void              glwCheckBoxGroupInit(GLWCheckBoxGroup *_this);
void              glwCheckBoxGroupDstr(GLWCheckBoxGroup *_this);
void              glwCheckBoxGroupFree(GLWCheckBoxGroup *_this);
int               glwCheckBoxGroupGetSelectedIdx(GLWCheckBoxGroup *_this);
void              glwCheckBoxGroupSetSelectedIdx(GLWCheckBoxGroup *_this,
        int _i);
void              glwCheckBoxGroupSelectNext(GLWCheckBoxGroup *_this);
void              glwCheckBoxGroupSelectPrev(GLWCheckBoxGroup *_this);
GLWActionFunc     glwCheckBoxGroupGetChangedFunc(GLWCheckBoxGroup *_this);
void              glwCheckBoxGroupSetChangedFunc(GLWCheckBoxGroup *_this,
        GLWActionFunc _func);
void             *glwCheckBoxGroupGetChangedCtx(GLWCheckBoxGroup *_this);
void              glwCheckBoxGroupSetChangedCtx(GLWCheckBoxGroup *_this,
        void *_ctx);



struct GLWCheckBox
{
    GLWComponent      super;
    GLWActionFunc     changed;
    void             *changed_ctx;
    GLWCheckBoxGroup *group;
    CDynArray         label;
    unsigned          state:1;
    unsigned          down:1;
};


extern const GLWCallbacks GLW_CHECK_BOX_CALLBACKS;

GLWCheckBox      *glwCheckBoxAlloc(const char *_label,int _state,
                                   GLWCheckBoxGroup *_group);
int               glwCheckBoxInit(GLWCheckBox *_this,
                                  const char *_label,int _state,
                                  GLWCheckBoxGroup *_group);
void              glwCheckBoxDstr(GLWCheckBox *_this);
void              glwCheckBoxFree(GLWCheckBox *_this);

int               glwCheckBoxGetState(GLWCheckBox *_this);
void              glwCheckBoxSetState(GLWCheckBox *_this,int _state);
const char             *glwCheckBoxGetLabel(GLWCheckBox *_this);
int               glwCheckBoxSetLabel(GLWCheckBox *_this,
                                      const char *_label);
int               glwCheckBoxAddLabel(GLWCheckBox *_this,
                                      const char *_label);
GLWCheckBoxGroup *glwCheckBoxGetGroup(GLWCheckBox *_this);
int               glwCheckBoxSetGroup(GLWCheckBox *_this,
                                      GLWCheckBoxGroup *_group);
GLWActionFunc     glwCheckBoxGetChangedFunc(GLWCheckBox *_this);
void              glwCheckBoxSetChangedFunc(GLWCheckBox *_this,
        GLWActionFunc _func);
void             *glwCheckBoxGetChangedCtx(GLWCheckBox *_this);
void              glwCheckBoxSetChangedCtx(GLWCheckBox *_this,
        void *_ctx);



struct GLWSlider
{
    GLWComponent   super;
    GLWActionFunc  changed;
    void          *changed_ctx;
    std::unordered_map<int, std::string>     labels;
    int            major_ticks;
    int            major_offs;
    int            minor_ticks;
    int            minor_offs;
    int            snap;
    int            ornt;
    int            min;
    int            max;
    int            val;
    int            ext;
    int            label_hi;                      /*Values used for slider layout*/
    int            label_lo;
    int            thumb_offs;
    GLWRect        thumb_rect;
    int            track_offs;
    GLWRect        track_rect;
    GLWRect        tick_rect;
    GLWRect        label_rect;
    unsigned       center_labels:1;
};


extern const GLWCallbacks GLW_SLIDER_CALLBACKS;

GLWSlider     *glwSliderAlloc(int _min,int _max,int _val,int _ext);
void           glwSliderInit(GLWSlider *_this,int _min,int _max,
                             int _val,int _ext);
void           glwSliderDstr(GLWSlider *_this);
void           glwSliderFree(GLWSlider *_this);

int            glwSliderIsCenteringLabels(GLWSlider *_this);
void           glwSliderSetCenteringLabels(GLWSlider *_this,int _b);
int            glwSliderGetMin(GLWSlider *_this);
int            glwSliderGetMax(GLWSlider *_this);
void           glwSliderSetRange(GLWSlider *_this,int _min,int _max);
int            glwSliderGetVal(GLWSlider *_this);
int            glwSliderGetExt(GLWSlider *_this);
void           glwSliderSetVal(GLWSlider *_this,int _val,int _ext);
int            glwSliderGetMajorTickSpacing(GLWSlider *_this);
void           glwSliderSetMajorTickSpacing(GLWSlider *_this,int _s);
int            glwSliderGetMajorTickOffset(GLWSlider *_this);
void           glwSliderSetMajorTickOffset(GLWSlider *_this,int _o);
int            glwSliderGetMinorTickSpacing(GLWSlider *_this);
void           glwSliderSetMinorTickSpacing(GLWSlider *_this,int _s);
int            glwSliderGetMinorTickOffset(GLWSlider *_this);
void           glwSliderSetMinorTickOffset(GLWSlider *_this,int _o);
int            glwSliderGetSnap(GLWSlider *_this);
void           glwSliderSetSnap(GLWSlider *_this,int _s);
int            glwSliderGetOrientation(GLWSlider *_this);
void           glwSliderSetOrientation(GLWSlider *_this,int _o);
int            glwSliderAddLabel(GLWSlider *_this,int _val,
                                 const char *_label);
int            glwSliderDelLabel(GLWSlider *_this,int _val);
int            glwSliderMakeLabels(GLWSlider *_this,int _start,int _inc);
GLWActionFunc  glwSliderGetChangedFunc(GLWSlider *_this);
void           glwSliderSetChangedFunc(GLWSlider *_this,GLWActionFunc _func);
void          *glwSliderGetChangedCtx(GLWSlider *_this);
void           glwSliderSetChangedCtx(GLWSlider *_this,void *_ctx);



struct GLWTabbedPane
{
    GLWComponent super;
    CDynArray    tabs;
    CDynArray    runs;
    GLWRect      area;
    int          tplc;
    int          seld;
    int          max_tab_w;
    int          max_tab_h;
    unsigned     valid:1;
};


extern const GLWCallbacks GLW_TABBED_PANE_CALLBACKS;


GLWTabbedPane *glwTabbedPaneAlloc(void);
void           glwTabbedPaneInit(GLWTabbedPane *_this);
void           glwTabbedPaneDstr(GLWTabbedPane *_this);
void           glwTabbedPaneFree(GLWTabbedPane *_this);

int            glwTabbedPaneGetPlacement(GLWTabbedPane *_this);
void           glwTabbedPaneSetPlacement(GLWTabbedPane *_this,
        int _tplc);
int            glwTabbedPaneGetSelectedIdx(GLWTabbedPane *_this);
void           glwTabbedPaneSetSelectedIdx(GLWTabbedPane *_this,
        int _idx);
GLWComponent  *glwTabbedPaneGetSelectedComp(GLWTabbedPane *_this);
void           glwTabbedPaneSetSelectedComp(GLWTabbedPane *_this,
        GLWComponent *_comp);
int            glwTabbedPaneAdd(GLWTabbedPane *_this,
                                GLWComponent *_comp,
                                const char *_title,int _idx);
int            glwTabbedPaneDel(GLWTabbedPane *_this,
                                GLWComponent *_comp);
void           glwTabbedPaneDelAt(GLWTabbedPane *_this,int _idx);
void           glwTabbedPaneDelAll(GLWTabbedPane *_this);
int            glwTabbedPaneGetTabCount(GLWTabbedPane *_this);
int            glwTabbedPaneGetRunCount(GLWTabbedPane *_this);
GLWcolor       glwTabbedPaneGetBackColorAt(GLWTabbedPane *_this,
        int _idx);
void           glwTabbedPaneSetBackColorAt(GLWTabbedPane *_this,
        int _idx,GLWcolor _c);
GLWcolor       glwTabbedPaneGetForeColorAt(GLWTabbedPane *_this,
        int _idx);
void           glwTabbedPaneSetForeColorAt(GLWTabbedPane *_this,
        int _idx,GLWcolor _c);
const char          *glwTabbedPaneGetTitleAt(GLWTabbedPane *_this,int _idx);
int            glwTabbedPaneSetTitleAt(GLWTabbedPane *_this,int _idx,
                                       const char *_s);
int            glwTabbedPaneAddTitleAt(GLWTabbedPane *_this,int _idx,
                                       const char *_s);



void     glwColor(GLWcolor _c);
void     glwClearColor(GLWcolor _c);
GLWcolor glwColorDarken(GLWcolor _c);
GLWcolor glwColorLighten(GLWcolor _c);
GLWcolor glwColorInvert(GLWcolor _c);
GLWcolor glwColorBlend(GLWcolor _c1,GLWcolor _c2);

GLWfont glwFontGet(int _family,int _size);
int     glwFontGetHeight(GLWfont _font);
int     glwFontGetAscent(GLWfont _font);
int     glwFontGetDescent(GLWfont _font);
int     glwFontGetWidth(GLWfont _font,int _c);
int     glwFontGetStringWidth(GLWfont _font,const char *_s);
int     glwFontDrawString(GLWfont _font,const char *_s,double _x,double _y);
int     glwFontDrawChar(GLWfont _font,int _c,double _x,double _y);

void glwRectInit(GLWRect *_this,int _x,int _y,int _w,int _h);

void glwInsetsInit(GLWInsets *_this,int _t,int _b,int _l,int _r);



GLWGridBagLayout *glwGridBagLayoutAlloc(void);
void              glwGridBagLayoutInit(GLWGridBagLayout *_this);
void              glwGridBagLayoutDstr(GLWGridBagLayout *_this);
void              glwGridBagLayoutFree(GLWGridBagLayout *_this);



void glwInit(int *_argc,char **_argv);

#endif                                                                /*_glw_H*/
