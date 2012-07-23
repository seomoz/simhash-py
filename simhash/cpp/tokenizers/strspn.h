#ifndef SIMHASH_TOKENIZERS__UTIL_H
#define SIMHASH_TOKENIZERS__UTIL_H

namespace Simhash {
    struct Strspn {
        /* Return the length of the token starting at last */
        const char* operator()(const char* last) {
            size_t s = strspn(last,
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
            return (*last != '\0') ? last + s : NULL;
        }
    };
}

#endif
