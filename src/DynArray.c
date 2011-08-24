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
#include <ctype.h>
#include <string.h>
#include "DynArray.h"
#pragma hdrstop
#if !defined(_util_DynArray_C)
# define _util_DynArray_C (1)

# if !defined(_Min)
#  define _Min(_a,_b) ((_a)<=(_b)?(_a):(_b))
# endif

# if !defined(_Max)
#  define _Max(_a,_b) ((_a)>=(_b)?(_a):(_b))
# endif

/*Initializes the dynamic array
  elm_sz: The size (in bytes) of each array element. Array elements other
           than the first will not be aligned on word or any other boundaries:
           you must ensure that this number is an appropriate multiple of any
           alignment desired
  cap:    The initial capacity, in elements. This parameter is only a
           suggestion. The array may not be able to allocate this much space.
           Use daEnsureCapacity() to ensure the array is as large as you need
           it to be
  inc:    The capacity of the array is incremented by this number every time
           it's current capacity is exceeded. If this number is 0, a heuristic
           is used to guess at a good capacity increment*/
void daInit(CDynArray *_this,size_t _elm_sz,size_t _cap,size_t _inc){
 if(_elm_sz<=0)_elm_sz=sizeof(int);
 _this->elm_sz=_elm_sz;
 _this->size=0;
 _this->cap=_cap;
 _this->inc=_inc;
 if(_cap){
  _this->data=malloc(_elm_sz*_cap*sizeof(char));
  if(_this->data==NULL)_this->cap=0;}
 else _this->data=NULL;}

/*Destroys the array, ensuring that all allocated memory is freed*/
void daDstr(CDynArray *_this){
 daDelAll(_this);
 daTrimToSize(_this);}

/*Reduces the memory allocated by the array to exactly the size required by
  the current size of the array. This is useful if you are sure the array will
  not grow beyond its current size*/
void daTrimToSize(CDynArray *_this){
 if(_this->cap>_this->size){
  char *data=realloc(_this->data,_this->elm_sz*_this->size*sizeof(char));
  if(data!=NULL||_this->size==0){
   _this->data=data;
   _this->cap=_this->size;} } }

/*Expands the capacity of the array to the indicated value if it is not
  already that large.
  cap: The capacity to ensure the array has
  Return: true iff the array is large enough to hold at least cap elements.
   If memory sufficient to store these elements cannot be allocated, the old
   array with its original capacity is left untouched*/
int daEnsureCapacity(CDynArray *_this,size_t _cap){
 if(_this->cap<_cap){
  size_t cap=_Max(_cap,_this->inc>0?_this->cap+_this->inc:
                       _Max(4,_Min(_this->cap<<1,_this->cap+1024)));
  char *data=realloc(_this->data,_this->elm_sz*cap*sizeof(char));
  if(data==NULL)return 0;
  _this->data=data;
  _this->cap=cap;}
 return 1;}

/*Sets the size of the array to the given value, also ensuring that the
  capacity of the array is large enough
  sz: The new size of the array
  Return: true iff the array is large enough to hold at least sz elements. If
   memory sufficient to store these elements cannot be allocated, the old
   array with its original size and capacity is left untouched*/
int daSetSize(CDynArray *_this,size_t _sz){
 int ret;
 if((ret=daEnsureCapacity(_this,_sz))!=0)_this->size=_sz;
 return ret;}

/*Inserts an array of elements at the given position in the array. The
  element currently at that position, and all those after, are shifted over
  to make room.
  i:    The position to insert the elements at. If this is the current size of
         the array, the elements are added to the end, and no elements are
         shifted
  elms: An array of elements to insert. If this is NULL, existing elements are
         moved over, but no new elements are stored in their place
  n:    The number of elements to insert. The array size is grown by this
         number, expanding its capacity if necessary
  Return: true iff sufficient memory could be allocated for the new elements.
   Otherwise, none of the elements are inserted and the original array is left
   untouched*/
int daInsArrayBefore(CDynArray *_this,size_t _i,const void *_elms,size_t _n){
 if(daEnsureCapacity(_this,_this->size+_n)){
  void *elm=daGetAt(_this,_i);
  size_t sz=_n*_this->elm_sz;
  memmove((char *)elm+sz,elm,(_this->size-_i)*_this->elm_sz);
  if(_elms!=NULL)memcpy(elm,_elms,sz);
  _this->size+=_n;
  return 1;}
 return 0;}

/*Deletes n elements from the array starting at position i. Any elements after
  the deleted elements are shifted over to fill up the hole
  i: The position of the first element to delete
  n: The number of elements to delete*/
void daDelRange(CDynArray *_this,size_t _i,size_t _n){
 memmove(daGetAt(_this,_i),daGetAt(_this,_i+_n),
         (_this->size-_i-_n)*_this->elm_sz);
 _this->size-=_n;}

/*Reads a single line from a text file, and stores it in line. The CDynArray
  should be initialized to store characters. The line will be null-terminated,
  and will not contain the EOL sequence.
  line: The CDynArray to store the line in
  in:   The FILE to read from
  Return: true iff there was enough memory, and the file was read successfully*/
int daFGetS(CDynArray *_line,FILE *_in){
 int r;
 daSetSize(_line,0);
 for(r=0;;){
  int  c=fgetc(_in);
  /*Recognizes any of three EOL sequences: "\r\n", "\r" or "\n"*/
  switch(c){
   case EOF :{
    if(feof(_in)){
     char d;
     d='\0';
     return daInsTail(_line,&d);}
    return 0;}
   case '\n':{
    char d;
    d='\0';
    return daInsTail(_line,&d);}
   case '\r':r=1;break;
   default  :{
    char d;
    if(r){
     if(ungetc(c,_in)==EOF)return 0;
     d='\0';
     return daInsTail(_line,&d);}
    d=(char)c;
    if(!daInsTail(_line,&d))return 0;} } } }

/*Trims leading and trailing whitespace (as returned by isspace()) from a
  null-terminated string in line. The size of line must be the length of the
  string, including the null-terminator.
  line: A CDynArray containing the string to trim*/
void daTrimWS(CDynArray *_line){
 unsigned char *line;
 int            i;
 line=_line->data;
 if(_line->size>1){
  for(i=(int)_line->size-2;i>=0&&isspace(line[i]);i--);
  daDelRange(_line,i+1,_line->size-2-i);
  line=_line->data;
  for(i=0;(size_t)i+1<_line->size&&isspace(line[i]);i++);
  daDelRange(_line,0,i);} }





#endif                                                      /*_util_DynArray_C*/