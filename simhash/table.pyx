from libcpp.vector cimport vector

cdef extern from "stdint.h":
    ctypedef unsigned long long uint64_t
    ctypedef unsigned int       size_t

cdef extern from "cpp/simhash.h" namespace "Simhash":
    ctypedef uint64_t hash_t
    hash_t simhash(char *s, size_t length, size_t window=4)
    size_t num_differing_bits(hash_t a, hash_t b)
    
    cdef cppclass Table:
        Table(size_t d, vector[hash_t]& p)

        #void insert[InputIterator, InputIterator](first, last)
        hash_t insert(hash_t hash)

        #void remove[InputIterator, InputIterator](first, last)
        hash_t remove(hash_t hash)
        hash_t find_first(hash_t hash)
        hash_t permute(hash_t hash)

cpdef PyHash(s):
    return simhash(s, len(s))

cdef class PyTable:
    cdef Table* tbl
    
    def __cinit__(self, d, permutors):
        cdef vector[hash_t] perms
        for p in permutors:
            perms.push_back(p)
        tbl = new Table(d, perms)
    
    def __dealloc__(self):
        del self.tbl
    
    # Accepts a string as a parameter and inserts it into the table
    cpdef insert(self, s):
        pass
