#ifndef SIMHASH_HASH_H
#define SIMHASH_HASH_H

/* Simply holds references to all of our hash functions */
#include "hashes/jenkins.h"
#include "hashes/murmur.h"
#include "hashes/fnv.h"

/* Include a reference to the tokenizer to use */
#include "tokenizers/strspn.h"

/* For hash_t */
#include "common.h"
#include "cyclic.hpp"

namespace Simhash {
    /**
     * Compute a similarity hash for the provided string using a rolling hash
     * function and a provided hash window
     *
     * @param s      - a string to hash
     * @param length - length of the provided string
     *
     * @return hash representative of the content of the text */
    template <typename Hash=jenkins, typename Tokenizer=Strspn>
    class Simhash {
    public:
        typedef Hash      hash_type;
        typedef Tokenizer tokenizer_type;

        hash_t operator()(const char* s, size_t length) {
            /* Simhash works by calculating the hashes of overlaping windows 
             * of the input, and then for each bit of that hash, increments a 
             * corresponding count. At the end, each of the counts is 
             * transformed back into a bit by whether or not the count is 
             * positive or negative */
        
            // Counts
            int64_t v[64] = {
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0
            };
        
            hash_t hash(0);             // The hash we're trying to produce
            size_t   j(0);              // Counter
            size_t   window(4);         // How many tokens in rolling hash?
            const char*    next(NULL);  // Pointer to the char /after/ token
            const char*    current(s);  // String pointers
            
            /* Create a tokenizer, hash function, and cyclic */
            Hash hasher;
            Tokenizer tokenizer;
            Cyclic<hash_t> cyclic(window);

            next = tokenizer(current);
            while (next != NULL) {
                if (next != current) {
                    hash_t r =
                        cyclic.push(hasher(current, next - current, 0));
                    /* Update the hash array. For each of the bits in the 
                     * output hash (whose bits are the union of a and b), 
                     * increment or decrement the corresponding count */
                    for (j = 63; j > 0; --j) {
                        v[j] += (r & 1) ? 1 : -1;
                        r = r >> 1;
                    }
                    v[j] += (r & 1) ? 1 : -1;
                }
                current = next + 1;
                next = tokenizer(current);
            }
            
            /* With counts appropriately tallied, create a 1 bit for each of  
             * the counts that's positive. That result is the hash. */
            for (j = 0; j < 64; ++j) {
                if (v[j] > 0) {
                    hash = hash | (static_cast<hash_t>(1) << j);
                }
            }
            return hash;
        }
    private:
        /* Internal stuffs */
        hash_type      hasher;
        tokenizer_type tokenizer;
    };
}

#endif
