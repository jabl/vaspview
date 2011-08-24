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
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#if !defined(_DynArray_H)
# define _DynArray_H (1)

typedef struct CDynArray CDynArray;



/*A dynamic array of fixed-size elements of arbitrary type. The array
  expands as necessary to add elements, using a fixed capacity increment
  provided or guessing at a good increment to prevent frequent allocations.
  The array will never reduce its size unless daTrimToSize() is called.
  Elements may be inserted or deleted at any position in the array, shifting
  the elements trailing behind them, or simply added to the end               */
/*A note about pointers: the two functions that provide individual element
  access, daGetAt() and daSetAt(), return pointers to the element just set
  or retrieved. These pointers are only valid so long as the array is not
  repositioned due to a daIns*(), a daEnsureCapacity() or a daTrimToSize()
  call (and, of course, daDstr()). If one of these calls is made, all pointers
  into the array must be re-retrieved to ensure it points to a valid portion
  of the address space. Note that the daDel*() functions may also reposition
  other elements in the array, so that any pointer to an element after the one
  that was deleted must also be repositioned. In this case, its index in the
  array will also have changed. If you must keep a large number of pointers
  to array elements, even when the array may be repositioned, consider
  dynamically allocating each element, and storing only a pointer to it in the
  array*/
/*This structure provides no synchronization mechanism. Multiple threads
  sharing a single instance of this structure must provide their own mutual
  exclusion to ensure no two threads modify the contents of the structure at
  the same time. Note that extra care must be taken with pointers to elements
  in the array when other threads may be modfiying, and possibly repositioning
  it*/
struct CDynArray
{
    char   *data;
    size_t  elm_sz;
    size_t  size;
    size_t  cap;
    size_t  inc;
};


# define /*void*/ _DAInit(/*CDynArray **/_this,/*size_t*/_cap,_type)          \
 (daInit((_this),sizeof(_type),(_cap),0))

# define /*type **/_DAGetAt(/*CDynArray **/_this,/*size_t*/ _i,_type)         \
 ((_i)+(_type *)daGetAt((_this),0))

# define /*type **/_DASetAt(/*CDynArray **/_this,/*size_t*/ _i,               \
                            /*type **/_elm,_type)                             \
 ((_type *)daSetAt((_this),(_i),(_type *)(_elm)))

void    daInit(CDynArray *_this,size_t _elm_sz,
               size_t _cap,size_t _inc);
void    daDstr(CDynArray *_this);

void    daTrimToSize(CDynArray *_this);
int     daEnsureCapacity(CDynArray *_this,size_t _cap);

int     daSetSize(CDynArray *_this,size_t _sz);

# define /*int*/   daInsHead(/*CDynArray **/_this,/*void */_elm)              \
 (daInsBefore(_this,0,_elm))
# define /*int*/   daInsArrayHead(/*CDynArray **/_this,                       \
                                  /*void **/_elms,/*size_t*/ _n)              \
 (daInsArrayBefore(_this,0,_elms,_n))
# define /*int*/   daInsTail(/*CDynArray **/_this,/*void **/_elm)             \
 (daInsBefore(_this,(_this)->size,_elm))
# define /*int*/   daInsArrayTail(/*CDynArray **/_this,                       \
                                  /*void **/_elms,/*size_t*/ _n)              \
 (daInsArrayBefore(_this,(_this)->size,_elms,_n))
# define /*int*/   daInsBefore(/*CDynArray **/_this,/*size_t*/ _i,            \
                               /*void **/_elm)                                \
 (daInsArrayBefore((_this),(_i),(_elm),1))
int     daInsArrayBefore(CDynArray *_this,size_t _i,
                         const void *_elms,size_t _n);
# define /*int*/   daInsAfter(/*CDynArray **/_this,/*size_t*/ _i,             \
                              /*void **/_elm)                                 \
 (daInsBefore(_this,(_i)+1,_elm))
# define /*int*/   daInsArrayAfter(/*CDynArray **/_this,/*size_t*/ _i,        \
                                   /*void **/_elms,/*size_t*/ _n)             \
 (daInsArrayBefore(_this,(_i)+1,_elms,_n))
# define /*int*/   daInsRange(/*CDynArray **/_this,/*size_t*/ _i,             \
                              /*size_t*/ _n)                                  \
 (daInsArrayBefore(_this,_i,NULL,_n))
# define /*void **/daGetAt(/*CDynArray **/_this,/*size_t*/ _i)                \
 (&((_this)->data[(_i)*(_this)->elm_sz]))
# define /*void **/daSetAt(/*CDynArray **/_this,/*size_t*/ _i,/*void **/_elm) \
 (memcpy(daGetAt(_this,_i),_elm,(_this)->elm_sz))
# define /*void*/  daDelHead(/*CDynArray **/_this)                            \
 (daDelAt(_this,0))
# define /*void*/  daDelTail(/*CDynArray **/_this)                            \
 (daDelAt(_this,(_this)->size-1))
# define /*void*/  daDelAt(/*CDynArray **/_this,/*size_t*/ _i)                \
 (daDelRange(_this,_i,1))
void daDelRange(CDynArray *_this,size_t _i,size_t _n);
# define /*void*/  daDelAll(/*CDynArray **/_this)                             \
 ((_this)->size=0)

int  daFGetS(CDynArray *_line,FILE *_in);
void daTrimWS(CDynArray *_line);

#endif                                                           /*_DynArray_C*/