import hashlib
import struct

from simhash cimport compute as c_compute
from simhash cimport find_all as c_find_all


def unsigned_hash(bytes obj):
    '''Returns a hash suitable for use as a hash_t.'''
    # Takes first 8 bytes of MD5 digest
    digest = hashlib.md5(obj).digest()[0:8]
    # Unpacks the binary bytes in digest into a Python integer
    return struct.unpack('>Q', digest)[0] & 0xFFFFFFFFFFFFFFFF

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
