from cython.operator cimport dereference  as deref
from cython.operator cimport preincrement as preinc

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

################################################################################
# Core. If you're looking for insight into the library, look here
################################################################################
cpdef PyHash(s):
    '''
    Utility function to return the simhash of a python string
    '''
    cdef Simhash[jenkins, Strspn] hasher
    return hasher.hash(s, len(s))

cdef class PyTable:
    '''
    A wrapper around the C++ implementation of a data structure that keeps all
    of a collection of fingerprints sorted subject to a bit block permutation. 
    It allows for fast queries to determine the all (or if any) known
    fingerprints differ by the query by at most _k_ bytes.
    '''

    cdef          Table* tbl
    cdef readonly hash_t search_mask

    def __cinit__(self, d, permutors):
        cdef vector[hash_t] perms
        for p in permutors:
            perms.push_back(p)
        self.tbl = new Table(d, perms)
        self.search_mask = self.tbl.get_search_mask()

    def __dealloc__(self):
        del self.tbl
    
    cpdef hashes(self):
        cdef object results = list()
        cdef const_iterator_t it = self.tbl.begin()
        while it != self.tbl.end():
            results.append(deref(it))
            preinc(it)
        return results
    
    cpdef insert_bulk(self, hashes):
        '''Accept a list of simhashes and insert them into the table'''
        for h in hashes:
            self.tbl.insert(h)
    
    cpdef insert(self, hash_t h):
        '''Insert a single simhash into the table'''
        self.tbl.insert(h)
    
    cpdef remove_bulk(self, hashes):
        '''Remove all the hashes in the provided iterable'''
        for h in hashes:
            self.tbl.remove(h)
    
    cpdef remove(self, hash_t h):
        '''Remove the provided hash from the table'''
        self.tbl.remove(h)
    
    cpdef hash_t find_first(self, hash_t query):
        '''
        Find the first example of a fingerprint that's considered a near-
        duplicate, if any. If none is found, 0 is returned. Otherwise, the 
        near-duplicate fingerprint.
        '''
        return self.tbl.find(query)
    
    cpdef find_first_bulk(self, queries):
        '''For each of the queries, find the first matching doc'''
        return [self.tbl.find(query) for query in queries]
    
    cpdef find_all(self, hash_t query):
        '''
        Find all fingerprints in the table that are considered near-duplicates
        of the query hash.
        '''
        cdef vector[hash_t] results
        self.tbl.find(query, results)
        return [results[i] for i in range(results.size())]
    
    cpdef find_all_bulk(self, queries):
        '''Find all the results for all the queres'''
        cdef vector[hash_t] tmp
        results = []
        for query in queries:
            self.tbl.find(query, tmp)
            results.append([tmp[i] for i in range(tmp.size())])
            tmp.clear()
        return results
    
    cpdef permute(self, hash_t query):
        '''
        Perform the table's permutation on the provided hash. This is not meant
        to be called regularly from external code. This is merely a utility
        function exposed to curious users.
        '''
        return self.tbl.permute(query)
    
    cpdef unpermute(self, hash_t query):
        '''Inverse of permute(query).'''
        return self.tbl.unpermute(query)

cdef class PyCorpus:
    '''
    A collection of simhash tables that, together, represent the entire corpus
    and the data structures needed to find all matches. Consult the accompanying
    README for a more detailed description of the algorithm.
    '''
    cdef readonly object tables
    cdef readonly size_t differing_bits
    
    def __init__(self, num_blocks, diff_bits):
        '''
        Given the number of blocks we want to use, make a collection of tables 
        with a roughly even number of bits in each of the blocks. This also
        calculates the permutation masks for you.
        '''
        
        import itertools
        # Permutation masks for the various blocks that we'll use
        perms = []
        for i in range(num_blocks):
            # Figure out how many (and which) bits will go into this block.
            start = ( i    * 64) / num_blocks
            end   = ((i+1) * 64) / num_blocks
            num = 0
            for j in range(start, end):
                num = num | (1 << j)
            perms.append(num)
        
        # Now we need to permute our permuators based on diffbits, and create a 
        # PyTable for each of these. We want to take each unique combination of
        # (num_blocks - diff_bits) blocks, and then concatenate the remaining
        # blocks. This ordered list is the permutation for that particular table
        self.tables = []
        for combo in itertools.combinations(perms, num_blocks - diff_bits):
            permutors = list(combo) + [p for p in perms if p not in combo]
            self.tables.append(PyTable(diff_bits, permutors))
    
    cpdef hashes(self):
        '''Return all the keys in here'''
        return self.tables[0].hashes()
    
    cpdef insert_bulk(self, hashes):
        '''Insert the provided hashes into each of the tables'''
        for table in self.tables:
            table.insert_bulk(hashes)
    
    cpdef insert(self, h):
        '''Insert a single hash into all the tables'''
        for table in self.tables:
            table.insert(h)
    
    cpdef remove_bulk(self, hashes):
        '''Remove all the provided hashes from all of our tables'''
        for table in self.tables:
            table.remove_bulk(hashes)
    
    cpdef remove(self, h):
        '''Remove a single hash from all the tables'''
        for table in self.tables:
            table.remove(h)
    
    cpdef find_first_bulk(self, queries):
        '''Sequentially find each of the queries'''
        # Off hand, this looks like a potentially slow implementation. If it 
        # turns out to be a problem, I think a little verbosity and hard work 
        # can make it much faster.
        return [self.find_first(query) for query in queries]
    
    cpdef find_first(self, hash_t query):
        '''Sequentially search each of the tables for this hash'''
        cdef hash_t result
        for table in self.tables:
            result = table.find_first(query)
            if result:
                return result
        return 0
    
    cpdef find_all_bulk(self, queries):
        '''Sequentially find all matching fingerprints for each query'''
        # Off hand, this looks like a potentially slow implementation
        return [self.find_all(query) for query in queries]
    
    cpdef find_all(self, hash_t query):
        '''Find all the matching fingerprints for this query'''
        results = []
        for table in self.tables:
            results.extend(table.find_all(query))
        return results

    cpdef distance(self, hash_t a, hash_t b):
        return num_differing_bits(a, b)
