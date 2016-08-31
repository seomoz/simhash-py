################################################################################
# Cython declarations
################################################################################

from libcpp.vector cimport vector
from libcpp.utility cimport pair
from libcpp.unordered_set cimport unordered_set

cdef extern from "stdint.h":
    ctypedef unsigned long long uint64_t
    ctypedef          long long  int64_t
    ctypedef unsigned int       size_t

cdef extern from "simhash-cpp/include/simhash.h" namespace "Simhash":
    ctypedef uint64_t hash_t
    ctypedef pair[hash_t, hash_t] match_t

    cppclass match_t_hash:
        size_t operator()(const match_t& v) const

    ctypedef unordered_set[match_t, match_t_hash] matches_t

    cpdef size_t num_differing_bits(hash_t a, hash_t b)
    hash_t compute(const vector[hash_t]& hashes)
    matches_t find_all(unordered_set[hash_t] hashes,
                       size_t number_of_blocks,
                       size_t different_bits)
