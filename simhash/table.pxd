################################################################################
# Cython declarations
################################################################################

from libcpp.vector cimport vector

cdef extern from "stdint.h":
    ctypedef unsigned long long uint64_t
    ctypedef unsigned int       size_t

cdef extern from *:
    ctypedef char* const_char_ptr "const char*"

cdef extern from "simhash-cpp/src/hash.hpp" namespace "Simhash":
    ctypedef uint64_t hash_t
    cdef cppclass Strspn:
        const_char_ptr operator()(const_char_ptr lst)

    cdef cppclass jenkins:
        uint64_t operator()(const_char_ptr data, size_t len, uint64_t s)

    cdef cppclass Simhash[Hash, Tokenizer]:
        hash_t hash(char* s, size_t length)

cdef extern from "simhash-cpp/src/simhash.h" namespace "Simhash":
    size_t num_differing_bits(hash_t a, hash_t b)

    cppclass const_iterator_t:
        const_iterator_t()
        hash_t operator*()
        const_iterator_t& operator++()
        # const_iterator_t  operator++(int)
        # const_iterator_t& operator--()
        # const_iterator_t  operator--(int)
        bint operator==(const_iterator_t&)
        bint operator!=(const_iterator_t&)
    
    cdef cppclass Table:
        Table(size_t d, vector[hash_t]& p)
        
        #void insert[InputIterator, InputIterator](first, last)
        void insert(hash_t h)
        
        #void remove[InputIterator, InputIterator](first, last)
        void remove(hash_t h)
        hash_t find(hash_t h)
        void   find(hash_t h, vector[hash_t]& results)
        
        hash_t   permute(hash_t h)
        hash_t unpermute(hash_t h)
        
        const_iterator_t begin()
        const_iterator_t end()
