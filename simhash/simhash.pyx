from simhash cimport compute as c_compute
from simhash cimport find_all as c_find_all

def unsigned_hash(obj):
    '''The built-in hash suitable for use as a hash_t.'''
    return hash(obj) & 0xFFFFFFFFFFFFFFFF

def compute(hashes):
    '''Compute the simhash of a vector of hashes.'''
    return c_compute(hashes)

def find_all(hashes, number_of_blocks, different_bits):
    '''
    Find the set of all matches within the provided vector of hashes.
    
    The provided hashes are manipulated in place, but upon completion are
    restored to their original state.
    '''
    cdef matches_t results_set = c_find_all(hashes, number_of_blocks, different_bits)
    cdef vector[match_t] results_vector
    results_vector.assign(results_set.begin(), results_set.end())
    return results_vector
