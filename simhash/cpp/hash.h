/* This code was taken from http://www.burtleburtle.net/bob/c/lookup3.c, and
 * under a public domain licence on May 25, 2012, reproduced below:
 * -----------------------------------------------------------------------------
 * lookup3.c, by Bob Jenkins, May 2006, Public Domain.
 * 
 * These are functions for producing 32-bit hashes for hash table lookup.
 * hashword(), hashlittle(), hashlittle2(), hashbig(), mix(), and final() 
 * are externally useful functions.  Routines to test the hash are included 
 * if SELF_TEST is defined.  You can use this free for any purpose.  It's in
 * the public domain.  It has no warranty.
 * 
 * You probably want to use hashlittle().  hashlittle() and hashbig()
 * hash byte arrays.  hashlittle() is is faster than hashbig() on
 * little-endian machines.  Intel and AMD are little-endian machines.
 * On second thought, you probably want hashlittle2(), which is identical to
 * hashlittle() except it returns two 32-bit hashes for the price of one.  
 * You could implement hashbig2() if you wanted but I haven't bothered here.
 * 
 * If you want to find a hash of, say, exactly 7 integers, do
 * a = i1;  b = i2;  c = i3;
 * mix(a,b,c);
 * a += i4; b += i5; c += i6;
 * mix(a,b,c);
 * a += i7;
 * final(a,b,c);
 * then use c as the hash value.  If you have a variable length array of
 * 4-byte integers to hash, use hashword().  If you have a byte array (like
 * a character string), use hashlittle().  If you have several byte arrays, or
 * a mix of things, see the comments above hashlittle().  
 * 
 * Why is this so big?  I read 12 bytes at a time into 3 4-byte integers, 
 * then mix those integers.  This is fast (you can do a lot more thorough
 * mixing with 12*3 instructions on 3 integers than you can with 3 instructions
 * on 1 byte), but shoehorning those bytes into integers efficiently is messy.
 * -----------------------------------------------------------------------------
 */

#ifndef SIMHASH_HASH_H
#define SIMHASH_HASH_H

#include <cstring>      /* defined size_t */
#include <stdint.h>     /* defines uint32_t etc */

void hashlittle2( 
    const void *key,       /* the key to hash */
    size_t      length,    /* length of the key */
    uint32_t   *pc,        /* IN: primary initval, OUT: primary hash */
    uint32_t   *pb);       /* IN: secondary initval, OUT: secondary hash */

#endif
