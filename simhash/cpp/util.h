#ifndef SIMHASH_UTIL_H
#define SIMHASH_UTIL_H

#include <Judy.h>

namespace Simhash {
    // The type used for our hashes. This comes from our use of Judy
    typedef Word_t hash_t;
    
    /**
     * Compute a similarity hash for the provided string using a rolling hash
     * function and a provided hash window
     *
     * @param s      - a string to hash
     * @param length - length of the provided string
     * @param window - number of characters considered as a token
     *
     * @return hash representative of the content of the text */
    hash_t simhash(char* s, size_t length, size_t window=4);
    
    /**
     * Compute the number of bits that are flipped between two numbers
     *
     * @param a - reference number
     * @param b - number to compare
     *
     * @return number of bits that differ between a and b */
    size_t num_differing_bits(hash_t a, hash_t b);
}

#endif
