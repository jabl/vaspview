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

#if !defined(_HashTable_H)
# define _HashTable_H (1)

#include <stddef.h>
#include <stdlib.h>

#define HT_LOAD_MIN    (0.0625)
#define HT_LOAD_DEF    (0.75)
#define HT_LOAD_MAX    (1024)
#define HT_SIZE_THRESH (sizeof(void *)<<2)
#define HT_CAP_DEF     (64)

typedef struct CHashGroup CHashGroup;
typedef struct CHashTable CHashTable;
typedef struct CHashIterator CHashIterator;
/*A compare function used to determine if two keys are equal
  ctx:  A context value supplied by the user
  key1: A pointer to the first key
  key2: A pointer to the second key
  Return: 0 iff the two keys are equivalent (i.e.: they are pointers to two
           equivalent strings, or are two equal integers, etc...)             */
typedef int (*HTKeyCmpFunc)(void *_ctx,const void *_key1,const void *_key2);
/*A hash function used to obtain a hash value for a key
  ctx: A context value supplied by the user
  key: A pointer to the key
  Return: An integer suitable for hashing. This integer should be based solely
           on the value of the key, and two keys which the compare function
           determines are equal should return the same hash value. Similarly,
           the hash value for a given key should not change over time as the
           state of external variables changes                                */
typedef unsigned (*HTHashFunc)(void *_ctx,const void *_key);

/*An entry in the hash table. This holds the actual data for up to
  HashTable.grp_sz key/value pairs, and links to the next group in the
  collision list                                                              */
struct CHashGroup
{
    CHashGroup    *next;                   /*The next group in the collision list*/
    unsigned       inuse;  /*Bitfield indicating which key/value pairs are filled*/
    unsigned char  data[1];
};                      /*Data for the key/value pairs*/



/*A general-purpose, open hash table. The hash table can store any fixed-size
  key or value. For variable sized keys and values, a fixed-size pointer to
  the actual key or value should be used. A function must be provided to
  compare two keys and determine if they are equal. Another must be provided
  to obtain a hash code for a given key. Hash codes are not cached in the
  table, though it would be fairly simple to extend the key value so that it
  stores its hash code with it, and define hash and compare functions to take
  advantage of this as appropriate. The general assumption is that many more
  searches will be performed than insertions or deletions, so if load, defined
  as the ratio of entries in the hash table (including collisions) to slots to
  contain them (excluding those added to resolve collisions) exceeds a certain
  threshold, the hash table capacity will be increased. The threshold can be
  adjusted by the application, using smaller values for applications with more
  frequent searches or larger values for applications with more frequent
  insertions and deletions. The load factor should generally not be less than
  0.5, though smaller values are allowed
  The hash table is not thread-safe, so multi-threaded applications will have
  to supply their own synchronization for hash tables shared among threads.
  However, there is a mod_count field which allows for the detection of
  concurrent modification, so that iteration and such can fail quickly and
  safely when concurrent modification is detected, instead of producing
  unpredictable results. Any structural change to the hash table, such as
  inserting a new key or deleting an existing key, will cause the mod_count
  variable to change. However, searching the table or replacing a value
  associated with an existing key will not cause mod_count to change
  The hash table allocates space for and makes literal copies of all key/value
  pairs. If these keys or values contain pointers to dynamically allocated
  resources, care should be taken to manage this memory appropriately. The
  resources should not be freed so long as the key or value is in the hash
  table. If a new key/value pair is inserted, and an existing key in the hash
  table compares equal to it, the old key and value are replaced. If you need
  to deallocate memory associated with the key or value beforehand, you should
  call htGet() first, and if it returns a value, free the resources used by
  that value, and then manually copy in the new value. In this case, you will
  not need to insert the new key (since an equivalent key is already in the
  hash table), and can free the resouces used by it as appropriate. If htGet()
  returns NULL, then you should call htIns() instead of htPut(), to forego
  the search for an existing key that is equivalent just performed by htGet().
  When removing key/value pairs, you should supply a non-NULL pointer for the
  'val' parameter to htDel(), which will give you access to the old key/value
  pair after they are removed from the table                                  */
struct CHashTable
{
    size_t         key_sz;                         /*The number of bytes in a key*/
    size_t         val_sz;                       /*The number of bytes in a value*/
    unsigned       grp_sz;                /*The number of entries in a hash group*/
    size_t         thresh; /*Number of entries at which the capacity is increased*/
    double         load;    /*Determines the threshold: thresh=(size_t)(load*cap)*/
    HTKeyCmpFunc   cmp;                           /*The compare function for keys*/
    HTHashFunc     hash;                             /*The hash function for keys*/
    void          *cmp_ctx;    /*The context value passed to the compare function*/
    void          *hash_ctx;      /*The context value passed to the hash function*/
    size_t         mod_count;      /*Value used to detect concurrent modification*/
    size_t         size;                           /*The number of entries in use*/
    size_t         cap;                        /*The size of the hash group table*/
    CHashGroup   **table;
};                         /*The hash group table itself*/



/*An iterator for a hash table. This will iterate through every entry in the
  hash table, using the functions provided, in no particular order. The only
  guarantee is that each entry in the table is iterated through exactly once.
  If the hash table is modified while the iteration is in progress, the
  modification will be detected, and the iterator will fail cleanly on the
  next attempt to access a value from the table, or increment the iterator.
  Like the HashTable, no synchronization is provided for multi-threaded
  applications                                                                */
struct CHashIterator
{
    CHashTable *hash_table;               /*The hash table that is being iterated*/
    CHashGroup *entry;
    CHashGroup *next;
    size_t      grp;
    unsigned    off;
    size_t      k_off;
    size_t      mod_count;
};



/*The default compare function for two keys. This simply compares the key
  values byte-by-byte
  ctx:  A pointer to a size_t, the size of the keys
  key1: A pointer to the first key
  key2: A pointer to the second key
  Return: 0 iff the two keys are equal                                        */
int htDefCmpFunc(void *_ctx,const void *_key1,const void *_key2);

/*The default hash function. This simply uses the literal value of each byte
  in the key, scrambled with integer arithmetic so as to give a distribution
  more suitable for hashing
  ctx: A pointer to a size_t, the size of the key
  key: A pointer to the key
  Return: An integer suitable for hashing                                     */
unsigned htDefHashFunc(void *_ctx,const void *_key);

/*Initializes the hash table. If the load factor is 0 or less, it will be
  inialized to a default value (HT_LOAD_DEF). If it is outside the range
  defined by HT_LOAD_MIN and HT_LOAD_MAX, it will be clamped to this range. If
  the group size is 0, it will be initialized to a default value, possibly
  based on other parameters for the hash table. If the group size exceeds the
  number bits in an integer, it is reduced to this size. If the initial
  capacity is 0, it is intialized to a default value (HT_CAP_DEF). In any
  case, the capacity will be rounded up to the next multiple of the group
  size, and it is the number of groups that is stored in the structure, not
  the actual capacity. If the compare function is NULL, it will be initialized
  to the default compare function, which simply compares the value of each
  key, byte by byte (as if by memcmp), and the compare function context will
  be set to the hashtable itself. If the hash function is NULL, it will be
  initialized to the default hash function, which computes its hash function
  based on the value of each byte of the key
  key_sz:   The number of bytes in each key. This should be a multiple of the
             alignment required for the key
  val_sz:   The number of bytes in each value. This should be a multiple of
             the alignment required for the value
  cmp:      The compare function. This is used to determine if two keys are
             equal when searching the hash table
  hash:     The hash function. This is used to map each key value into a
             location in the hash table
  cap:      The initial capacity of the hash table. This is the number of
             slots available to store key-value pairs in
  load:     The is the load factor of the hash table. When the number of
             entries in the hash table exceeds this number times the capacity
             of the hash table, the hash table is enlarged, and all the
             entries are re-hashed within it
  grp_sz:   The size of hash groups. This is the number of hash entries that
             are allocated at a time to store key/value pairs put into the
             hash table. Increasing this number reduces the frequency and
             overhead of memory allocations, but can waste space in a sparsely
             populated hash table, or one with large key-value pairs
  cmp_ctx:  The compare function context. This value is passed verbatim to the
             compare function each time it is called
  hash_ctx: The hash function context. This value is passed verbatim to the
             hash function each time it is called                             */
void  htInit(CHashTable *_this,size_t _key_sz,size_t _val_sz,
             HTKeyCmpFunc _cmp,HTHashFunc _hash,
             size_t _cap,double _load,unsigned _grp_sz,
             void *_cmp_ctx,void *_hash_ctx);

# define _HTInit(_this,_cap,_key_type,_val_type,_cmp,_hash)                   \
 (htInit((_this),sizeof(_key_type),sizeof(_val_type),                         \
         (_cmp),(_hash),(_cap),0,0,NULL,NULL))

/*Makes a shallow copy of the hash table. This means that all key-value pairs
  are copied by value. New, separate memory space is allocated for the
  internal structure of the hash table, however, so that it can be modified
  without causing any change to the original hash table.
  that: The new, uninitialized hash table to clone this one into
  Return: true iff the cloning was succesful                                  */
int   htClone(CHashTable *_this,CHashTable *_that);

/*Frees all resources currently used by this hash table                       */
void  htDstr(CHashTable *_this);


/*Determines if a key is contained in the hash table. If so, it returns a
  pointer to the key in the hash table. This pointer is suitable for short term
  access only. Subsequent calls to functions which modify the structure of the
  hash table can invalidate this pointer (see HashTable.mod_count)
  key: A pointer to the key to search for
  Return: A pointer to the key, or NULL if it was not found                   */
const void *htGetKey(const CHashTable *_this,const void *_key);

/*Gets a pointer to the value which is stored in the hash table for this key.
  If the key is not found in the hash table, returns NULL. This pointer is
  suitable for short term access only. Subsequent calls to functions which
  modify the structure of the hash table can invalidate this pointer (see
  CHashTable.mod_count)
  key: A pointer to the key to retrieve the value for
  Return: A pointer to the value stored with this key, or NULL if the key was
           not found                                                          */
void *htGet(const CHashTable *_this,const void *_key);

/*Associates the specified value with the specified key. The hash table should
  not already contain a mapping for this key. New space is allocated if
  necessary, and the hash table's mod_count is increased. This function
  returns a pointer to the location of the new value in the hash table, or
  NULL if there was not enough memory to complete the insertion. This pointer
  is suitable for short term access only. Subsequent calls to functions which
  modify the structure of the hash table can invalidate this pointer
  (see CHashTable.mod_count)
  key:  A pointer to the key to associate the value with
  val:  A pointer to the value to associate with the key
  Return: A pointer the the new location of the value in the hash table, or
           NULL, if the value could not be inserted.                          */
void *htIns(CHashTable *_this,const void *_key,const void *_val);

/*Associates the specified value with the specified key. If the hash table
  previously contained a mapping for this key, the old value is replaced, and
  a flag indicates whether or not the old values should be retrieved before
  being overwritten. If there was no mapping to begin with, new space is
  allocated for the key/value pair if neccessary, and the hash table's
  mod_count is increased. In any case, this function returns a pointer to the
  location of the new value in the hash table, or NULL if there was not enough
  memory to complete the insertion. This pointer is suitable for short term
  access only. Subsequent calls to functions which modify the structure of the
  hash table can invalidate this pointer (see CHashTable.mod_count)
  key:  A pointer to the key to associate the value with
  val:  A pointer to the value to associate with the key
  Return: A pointer the the new location of the value in the hash table, or
           NULL, if the value could not be inserted.                          */
void *htPut(CHashTable *_this,const void *_key,const void *_val);

/*Removes the mapping for this key from this hash table if present. mod_count
  is increased to reflect the change
  key: The key whose mapping is to be removed from the map
  val: A place to store the value that used to be associated with this key. If
        this is not NULL, the value will be copied into this location before
        its storage in the hash table is deallocated, and the key found in the
        hash table will also be copied into the key location
  Return: true iff the key was found and removed                              */
int   htDel(CHashTable *_this,void *_key,void *_val);

/*Removes all mappings from this hash table                                   */
void  htClear(CHashTable *_this);



/*Initializes this hash table iterator to iterate through values of the given
  hash table
  ht: The hash table to iterate through                                       */
void  hiInit(CHashIterator *_this,CHashTable *_ht);

/*Determines if there are more entries in the current hash table
  Return: Whether or not there are more entries in the hash table to iterate
           through                                                            */
int   hiHasNext(CHashIterator *_this);

/*Moves the iterator to refer to the next entry in the hash table. If the
  hash table has been structurally modified, this function will fail cleanly
  Return: false iff the hash table has been modified since this iterator was
           initialized in a way that would cause unpredictable results, or
           there are no more entries in the hash table                        */
int   hiInc(CHashIterator *_this);

/*Gets the key associated with the current value of the hash table. If the
  hash table has been structurally modified, this function will fail cleanly
  Return: A pointer to the key, or NULL if the hash table has been modified
           since this iterator was initialized in a way that would cause
           unpredictable results, or has no more entries                      */
void *hiGetKey(CHashIterator *_this);

/*Gets the value associated with the current value of the hash table. If the
  hash table has been structurally modified, this function will fail cleanly
  Return: A pointer to the value, or NULL if the hash table has been modified
           since this iterator was initialized in a way that would cause
           unpredictable results, or has no more entries                      */
void *hiGetValue(CHashIterator *_this);

/*Deletes the current item of the hash iterator. The iterator is automatically
  incremented to the next entry after the deletion. Incrementing the iterator
  again will cause an entry to be skipped
  key: A place to store the key of the deleted item. If this is non-NULL, the
        key will be copied into this location before being removed from the
        hash table
  val: A place to store the value of the deleted item. If this is non-NULL,
        the value will be copied into this location before being removed from
        the hash table
  Return: Whether or not the deletion was successful                          */
int   hiDel(CHashIterator *_this,void *_key,void *_val);
#endif                                                          /*_HashTable_H*/
