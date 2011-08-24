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
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "HashTbl.h"
#pragma hdrstop
# if !defined(_util_HashTable_C)
# define _util_HashTable_C (1)

# define HT_GRP_MAX (sizeof(int)*CHAR_BIT)

/*Convenience macros for accessing a hash group*/
# define hgIsInUse(_entry,_e_bit)                                             \
 (((_entry)->inuse&(_e_bit))!=0)
# define hgGetKey(_entry,_k_off)                                              \
 ((_entry)->data+(_k_off))
# define hgGetValue(_entry,_v_off)                                            \
 ((_entry)->data+(_v_off))

/*The default compare function for two keys*/
int htDefCmpFunc(void *_ctx,const void *_key1,const void *_key2){
 return memcmp(_key1,_key2,*((size_t *)_ctx));}

/*The default hash function*/
unsigned htDefHashFunc(void *_ctx,const void *_key){
 size_t   sz=*((size_t *)_ctx);
 unsigned ret=0;
 size_t   i;
 /*This function is more suitable for little-endian data than big-endian*/
 for(i=0;i<sz;i++)ret=ret*31+((unsigned char *)_key)[i];
 return ret;}

/*Frees all the entries in a hash table
  table: The table to clear
  cap:   The number of groups in the table                                    */
static void clearTable(CHashGroup **_table,size_t _cap){
 size_t i;
 for(i=0;i<_cap;i++){
  CHashGroup *entry;
  CHashGroup *next;
  for(entry=_table[i];entry!=NULL;entry=next){
   next=entry->next;
   free(entry);} } }

/*Frees a hash entry table, and all the entries in it
  table: The table to free
  cap:   The number of groups in the table                                    */
static void freeTable(CHashGroup **_table,size_t _cap){
 clearTable(_table,_cap);
 free(_table);}

/*Initializes the hash table*/
void htInit(CHashTable *_this,size_t _key_sz,size_t _val_sz,
            HTKeyCmpFunc _cmp,HTHashFunc _hash,
            size_t _cap,double _load,unsigned _grp_sz,
            void *_cmp_ctx,void *_hash_ctx){
 if(_this!=NULL){
  _this->key_sz=_key_sz;
  _this->val_sz=_val_sz;
  if(_load<=0)_load=HT_LOAD_DEF;
  else if(_load<HT_LOAD_MIN)_load=HT_LOAD_MIN;
  else if(_load>HT_LOAD_MAX)_load=HT_LOAD_MAX;
  _this->load=_load;
  if(_grp_sz<=0){
   if(_key_sz+_val_sz<=HT_SIZE_THRESH){
    _grp_sz=(unsigned)(HT_GRP_MAX*_load);}
   if(_grp_sz<=0)_grp_sz=1;}
  if(_grp_sz>HT_GRP_MAX)_grp_sz=HT_GRP_MAX;
  _this->grp_sz=_grp_sz;
  if(_cap<=0)_cap=HT_CAP_DEF;
  _this->cap=(_cap+_grp_sz-1)/_grp_sz;
  if(_cmp==NULL){
   _this->cmp=htDefCmpFunc;
   _this->cmp_ctx=&_this->key_sz;}
  else{
   _this->cmp=_cmp;
   _this->cmp_ctx=_cmp_ctx;}
  if(_hash==NULL){
   _this->hash=htDefHashFunc;
   _this->hash_ctx=&_this->key_sz;}
  else{
   _this->hash=_hash;
   _this->hash_ctx=_hash_ctx;}
  _this->thresh=(size_t)(_this->cap*_grp_sz*_load);
  _this->size=0;
  _this->mod_count=0;
  _this->table=(CHashGroup **)calloc(_this->cap,sizeof(CHashGroup *));
  if(_this->table==NULL)_this->cap=_this->thresh=0;} }
 
/*Makes a shallow copy of the hash table*/
int htClone(CHashTable *_this,CHashTable *_that){
 if(_this!=NULL&&_that!=NULL){
  CHashGroup **new_table;
  *_that=*_this;
  new_table=(CHashGroup **)malloc(_this->cap*sizeof(CHashGroup *));
  if(new_table!=NULL){
   size_t i;
   size_t grp_bytes=sizeof(CHashGroup)-1+
                    (_this->key_sz+_this->val_sz)*_this->grp_sz;
   for(i=0;i<_this->cap;i++){
    CHashGroup *entry;
    entry=_this->table[i];
    if(entry==NULL)_that->table[i]=NULL;
    else{
     CHashGroup *copy=(CHashGroup *)malloc(grp_bytes);
     _that->table[i]=copy;
     for(;;copy=copy->next){
      if(copy==NULL)goto copy_fail;
      memcpy(copy,entry,grp_bytes);
      entry=entry->next;
      if(entry==NULL)break;
      copy->next=(CHashGroup *)malloc(grp_bytes);} } }
   return 1;
copy_fail:
   freeTable(_that->table,i);
   _that->table=NULL;}
  _that->size=_that->cap=0;}
 return 0;}
 
/*Frees all resources currently used by this hash table*/
void htDstr(CHashTable *_this){
 if(_this!=NULL&&_this->table!=NULL){
  _this->mod_count++;
  freeTable(_this->table,_this->cap);
  _this->table=NULL;
  _this->size=_this->cap=0;} }


/*Installs a new key/value pair in the hash table at the specified position
  key:   A pointer to the key to install
  val:   A pointer to the value to install
  prev:  A pointer to the hash group before the one to install it at, or NULL
          if that hash group is the first in the collision list
  entry: A pointer to the hash group to install it at, or NULL to allocate a
          new collision list entry
  grp:   The location of the hash group in the table
  e_bit: The "in use" bit for the entry
  k_off: The byte offset of the key in the group's data
  Return: A pointer to the newly inserted value, or NULL if there was not
           enough memory to complete the insertion                            */
static void *htPutImpl(CHashTable *_this,const void *_key,const void *_val,
                       CHashGroup *_prev,CHashGroup *_entry,
                       size_t _grp,unsigned _e_bit,size_t _k_off){
 void *val;
 if(_entry==NULL){
  _entry=(CHashGroup *)malloc(sizeof(CHashGroup)-1+
                              (_this->key_sz+_this->val_sz)*_this->grp_sz);
  if(_entry==NULL)return NULL;
  _entry->inuse=0;
  _entry->next=NULL;
  if(_prev!=NULL)_prev->next=_entry;
  else _this->table[_grp]=_entry;}
 _this->mod_count++;
 _this->size++;
 _entry->inuse|=_e_bit;
 memcpy(hgGetKey(_entry,_k_off),_key,_this->key_sz);
 val=hgGetValue(_entry,_k_off+_this->key_sz);
 memcpy(val,_val,_this->val_sz);
 return val;}

/*Installs a new key/value pair in the hash table without first checking to
  ensure that it does not already exist. If you do not make this check somehow
  yourself before calling this function, you may end up with multiple values
  installed for the same key, and which one is returned on a given query is
  undefined (and can change with the hash table's contents)
  key:  A pointer to the key to install
  val:  A pointer to the value to install
  hash: The hash value for the given key, which must not be negative
  Return: A pointer to the newly inserted value, or NULL if there was not
           enough memory to complete the insertion                            */
static void *htPutBlind(CHashTable *_this,const void *_key,
                        const void *_val,unsigned _hash){
 CHashGroup *entry;
 CHashGroup *prev;
 size_t      idx;
 size_t      grp;
 size_t      k_off;
 unsigned    e_bit;
 idx=_hash%(_this->cap*_this->grp_sz);
 grp=idx/_this->grp_sz;
 k_off=idx-grp*_this->grp_sz;
 e_bit=1<<k_off;
 k_off*=(_this->key_sz+_this->val_sz);
 entry=_this->table[grp];
 prev=NULL;
 for(;entry!=NULL&&hgIsInUse(entry,e_bit);entry=entry->next)prev=entry;
 return htPutImpl(_this,_key,_val,prev,entry,grp,e_bit,k_off);}

/*Rehashes the contents of this hash table into a new table with a larger
  capacity. This method is called automatically when the number of keys in
  this hash table exceeds its capacity and load factor. If there is not enough
  memory to increase the size of the hash table, it is restored to its
  previous state, although the threshold for reallocation is left at its
  increased value. Otherwise, its mod_count is increased                      */
static void htRehash(CHashTable *_this){
 size_t       old_cap=_this->cap;
 size_t       new_cap=old_cap<<1|1;
 CHashGroup **new_table=(CHashGroup **)calloc(new_cap,sizeof(CHashGroup *));
 if(new_table!=NULL){
  CHashGroup **old_table=_this->table;
  size_t       old_mod_count=_this->mod_count;
  size_t       old_size=_this->size;
  size_t       i;
  _this->mod_count++;
  _this->cap=new_cap;
  _this->table=new_table;
  _this->thresh=(size_t)(new_cap*_this->grp_sz*_this->load);
  _this->size=0;
  for(i=0;i<old_cap;i++){
   CHashGroup *entry;
   for(entry=old_table[i];entry!=NULL;entry=entry->next){
    unsigned j;
    unsigned e_bit=1;
    size_t   k_off=0;
    for(j=0;j<_this->grp_sz;j++){
     if(hgIsInUse(entry,e_bit)){
      if(htIns(_this,hgGetKey(entry,k_off),
               hgGetValue(entry,k_off+_this->key_sz))==NULL){
       freeTable(new_table,new_cap);
       _this->cap=old_cap;
       _this->table=old_table;
       _this->mod_count=old_mod_count;
       _this->size=old_size;
       if(old_table==NULL)_this->thresh=0;
       return;} }
     e_bit<<=1;
     k_off+=_this->key_sz+_this->val_sz;} } }
  freeTable(old_table,old_cap);} }

/*Determines if a key is contained in the hash table*/
const void *htGetKey(const CHashTable *_this,const void *_key){
 void *ret=htGet(_this,_key);
 if(ret!=NULL)ret=((char *)ret)-_this->key_sz;
 return ret;}
 
/*Gets a pointer to the value which is stored in the hash table for this key*/
void *htGet(const CHashTable *_this,const void *_key){
 if(_this!=NULL&&_key!=NULL&&_this->table!=NULL){
  CHashGroup *entry;
  unsigned    hash;
  size_t      idx;
  size_t      grp;
  size_t      k_off;
  unsigned    e_bit;
  hash=_this->hash(_this->hash_ctx,_key);
  idx=hash%(_this->cap*_this->grp_sz);
  grp=idx/_this->grp_sz;
  k_off=idx-grp*_this->grp_sz;
  e_bit=1<<k_off;
  k_off*=(_this->key_sz+_this->val_sz);
  entry=_this->table[grp];
  for(;entry!=NULL&&hgIsInUse(entry,e_bit);entry=entry->next){
   if(!_this->cmp(_this->cmp_ctx,_key,hgGetKey(entry,k_off))){
    return hgGetValue(entry,k_off+_this->key_sz);} } }
 return NULL;}

/*Associates the specified value with the specified key*/
void *htIns(CHashTable *_this,const void *_key,const void *_val){
 if(_this!=NULL){
  return htPutBlind(_this,_key,_val,_this->hash(_this->hash_ctx,_key));}
 return NULL;}

/*Associates the specified value with the specified key*/
void *htPut(CHashTable *_this,const void *_key,const void *_val){
 if(_this!=NULL&&_key!=NULL&&_val!=NULL){
  CHashGroup *entry;
  CHashGroup *prev;
  unsigned    hash;
  size_t      idx;
  size_t      grp;
  size_t      k_off;
  unsigned    e_bit;
  hash=_this->hash(_this->hash_ctx,_key);
  if(_this->table!=NULL){
   idx=hash%(_this->cap*_this->grp_sz);
   grp=idx/_this->grp_sz;
   k_off=idx-grp*_this->grp_sz;
   e_bit=1<<k_off;
   k_off*=(_this->key_sz+_this->val_sz);
   prev=NULL;
   entry=_this->table[grp];
   for(;entry!=NULL&&hgIsInUse(entry,e_bit);entry=entry->next){
    void *key=hgGetKey(entry,k_off);
    if(!_this->cmp(_this->cmp_ctx,_key,key)){
     void *val=hgGetValue(entry,k_off+_this->key_sz);
     memcpy(key,_key,_this->key_sz);
     memcpy(val,_val,_this->val_sz);
     return val;}
    prev=entry;} }
  if(_this->size>=_this->thresh){
   htRehash(_this);
   if(_this->table==NULL)return NULL;
   return htPutBlind(_this,_key,_val,hash);}
  else return htPutImpl(_this,_key,_val,prev,entry,grp,e_bit,k_off);}
 return NULL;}

/*Removes the mapping for this key from this hash table if present*/
int htDel(CHashTable *_this,void *_key,void *_val){
 if(_this!=NULL&&_key!=NULL&&_this->table!=NULL){
  CHashGroup *entry;
  CHashGroup *prev;
  unsigned    hash;
  size_t      idx;
  size_t      grp;
  size_t      k_off;
  unsigned    e_bit;
  /*Search for the key*/
  hash=_this->hash(_this->hash_ctx,_key);
  idx=hash%(_this->cap*_this->grp_sz);
  grp=idx/_this->grp_sz;
  k_off=idx-grp*_this->grp_sz;
  e_bit=1<<k_off;
  k_off*=(_this->key_sz+_this->val_sz);
  entry=_this->table[grp];
  prev=NULL;
  for(;entry!=NULL&&hgIsInUse(entry,e_bit);entry=entry->next){
   void *key=hgGetKey(entry,k_off);
   if(!_this->cmp(_this->cmp_ctx,_key,key)){
    /*Found the key, now copy it back for the user if necessary*/
    void *val=hgGetValue(entry,k_off+_this->key_sz);
    if(_val!=NULL){
     memcpy(_key,key,_this->key_sz);
     memcpy(_val,val,_this->val_sz);}
    _this->mod_count++;
    /*If we're not the last entry in the list, move whatever is the last entry
      into our position, to keep the entries in use contiguous. This saves
      time on searches and reduces wasted space*/
    if(entry->next!=NULL&&hgIsInUse(entry->next,e_bit)){
     do{
      prev=entry;
      entry=entry->next;}while(entry->next!=NULL&&
                               hgIsInUse(entry->next,e_bit));
     memcpy(key,hgGetKey(entry,k_off),_this->key_sz);
     memcpy(val,hgGetValue(entry,k_off+_this->key_sz),_this->val_sz);}
    /*Mark this entry as unused, and free the group if everything is unused*/
    entry->inuse^=e_bit;
    if(!entry->inuse){
     free(entry);
     if(prev!=NULL)prev->next=NULL;
     else _this->table[grp]=NULL;}
    return 1;}
   prev=entry;} }
 return 0;}

/*Removes all mappings from this hash table*/
void htClear(CHashTable *_this){
 if(_this!=NULL&&_this->table!=NULL){
  _this->mod_count++;
  clearTable(_this->table,_this->cap);
  memset(_this->table,0,_this->cap*sizeof(CHashGroup *));
  _this->size=0;} }



/*Initializes this hash table iterator*/
void hiInit(CHashIterator *_this,CHashTable *_ht){
 if(_this!=NULL){
  _this->hash_table=_ht;
  _this->entry=_this->next=NULL;
  if(_ht!=NULL){
   _this->grp=_ht->cap;
   _this->off=0;
   _this->mod_count=_ht->mod_count;}
  else _this->grp=_this->off=_this->mod_count=0;} }

/*Determines if there are more entries in the current hash table*/
int hiHasNext(CHashIterator *_this){
 if(_this!=NULL&&_this->hash_table!=NULL){
  if(_this->entry==NULL||!hgIsInUse(_this->entry,(1<<_this->off)-1)){
   while(_this->next==NULL){
    if(_this->grp<=0)return 0;
    _this->next=_this->hash_table->table[--(_this->grp)];} }
  return 1;}
 return 0;}

/*Moves the iterator to refer to the next entry in the hash table*/
int hiInc(CHashIterator *_this){
 if(_this!=NULL&&_this->hash_table!=NULL&&
    _this->mod_count==_this->hash_table->mod_count){
  /*First check to see if there are more entries in the current hash group*/
  if(_this->entry!=NULL){
   unsigned e_bit=1<<_this->off;
   if(hgIsInUse(_this->entry,e_bit-1))do{
    _this->off--;
    _this->k_off-=_this->hash_table->key_sz+_this->hash_table->val_sz;
    e_bit>>=1;}
   while(!hgIsInUse(_this->entry,e_bit));
   else _this->entry=NULL;}
  /*If not, find a new hash group with entries*/
  if(_this->entry==NULL){
   unsigned e_bit;
   while(_this->next==NULL){
    if(_this->grp<=0)return 0;
    _this->next=_this->hash_table->table[--(_this->grp)];}
   _this->entry=_this->next;
   _this->next=_this->next->next;
   _this->off=_this->hash_table->grp_sz-1;
   _this->k_off=(_this->hash_table->key_sz+_this->hash_table->val_sz)*
                _this->off;
   e_bit=1<<_this->off;
   while(!hgIsInUse(_this->entry,e_bit)){          /*Find the first used entry*/
    _this->off--;
    _this->k_off-=_this->hash_table->key_sz+_this->hash_table->val_sz;
    e_bit>>=1;} }
  return 1;}
 return 0;}

/*Gets the key associated with the current value of the hash table*/
void *hiGetKey(CHashIterator *_this){
 if(_this!=NULL&&_this->entry!=NULL&&_this->hash_table!=NULL&&
    _this->mod_count==_this->hash_table->mod_count){
  return hgGetKey(_this->entry,_this->k_off);}
 return NULL;}

/*Gets the value associated with the current value of the hash table*/
void *hiGetValue(CHashIterator *_this){
 if(_this!=NULL&&_this->entry!=NULL&&_this->hash_table!=NULL&&
    _this->mod_count==_this->hash_table->mod_count){
  return hgGetValue(_this->entry,_this->k_off+_this->hash_table->key_sz);}
 return NULL;}

/*Deletes the current item of the hash iterator*/
int hiDel(CHashIterator *_this,void *_key,void *_val){
 if(_this!=NULL){
  void *key=hiGetKey(_this);
  if(key!=NULL){
   unsigned e_bit=1<<_this->off;
   if(!hgIsInUse(_this->entry,~e_bit)&&_this->entry->next==NULL){
    _this->entry=NULL;}
   if(_key!=NULL){
    memcpy(_key,key,_this->hash_table->key_sz);
    if(_val!=NULL)key=_key;}
   htDel(_this->hash_table,key,_val);                  /*Should always succeed*/
   _this->mod_count=_this->hash_table->mod_count;
   if(_this->entry==NULL||!hgIsInUse(_this->entry,e_bit))hiInc(_this);
   return 1;} }
 return 0;}

#endif                                                     /*_util_HashTable_C*/