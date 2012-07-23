#ifndef SIMHASH_CYCLIC_H
#define SIMHASH_CYCLIC_H

#include <vector>

namespace Simhash {
    /* Keep a cyclic hash */
    template<typename Int>
    class Cyclic {
    public:
        /* An easy typedef */
        typedef Int hash_type;

        /* Useful elsewhere */
        static const size_t bits = sizeof(hash_type) * 8;

        Cyclic(size_t l=4): length(l), current(0), counter(0), tokens() {
            /* Initialize all of these to 0 */
            tokens.resize(length);
            for (size_t i = 0; i < l; ++i) {
                tokens[i] = 0;
            }
        }

        Cyclic(const Cyclic<hash_type>& o):
            length(o.length), current(o.current), counter(o.counter), 
            tokens(o.tokens) {}

        const Cyclic<hash_type>& operator=(const Cyclic<hash_type>& o) {
            length  = o.length;
            current = o.current;
            counter = o.counter;
            tokens  = o.tokens;
            return *this;
        }

        /* Push a new hash onto the stack and get the return value */
        hash_type push(hash_type val) {
            /* Increment the counter. That's the index of the value we have to 
             * pop off, and the index that we'll replace */
            current = rotate(current) ^ rotate(tokens[counter], length) ^ val;
            tokens[counter] = val;
            counter = (counter + 1) % length;
            return current;
        }

        /* Rotation shift by 1 */
        static inline hash_type rotate(hash_type v) {
            return (v << 1) | (v >> (bits - 1));
        }

        static inline hash_type rotate(hash_type v, size_t count) {
            count = count % bits;
            return (v << count) | (v >> (bits - count));
        }
    private:
        size_t    length;  // How many pieces of data to store
        hash_type current; // Our current value
        size_t    counter; // Where we are in our tokens array

        std::vector<hash_type> tokens;  // Array holding our tokens
    };
}

#endif
