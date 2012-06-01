#ifndef FRESH_UTIL
#define FRESH_UTIL

#include <Judy.h>

// The type used for our hashes. This comes from our use of Judy
typedef Word_t hash_t;

/* Params:
 *      s - a string to hash
 * length - length of said string
 * window - number of characters included in the moving window of the 
 *           rolling hash function
 *
 * Returns: a hash representative of the content */
hash_t simhash(const char* s, size_t length, size_t window);

/* Params:
 *      a - reference number
 *      b - query number
 *
 * Returns: the number of bits that differ between a and b */
size_t num_differing_bits(hash_t a, hash_t b);

#endif