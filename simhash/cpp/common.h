#ifndef SIMHASH_COMMON_H
#define SIMHASH_COMMON_H

/* For Word_t */
#include <Judy.h>

namespace Simhash {
    // The type used for our hashes. This comes from our use of Judy
    typedef Word_t hash_t;
}

#endif
