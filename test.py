#! /usr/bin/env python

import unittest
import simhash

class TestSimhash(unittest.TestCase):
    def setUp(self):
        self.corpus = simhash.Corpus(6, 3)
    
    def tearDown(self):
        pass
    
    def test_insert(self):
        # We should be able to at least insert into the damned thing, and then
        # retrieve them. We should try various blocks as the differing bits. In
        # fact, we should try with every valid combination of differeint bit 
        # blocks.
        offsets = (5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60)
        masks   = [1 << i for i in offsets]
        inputs  = [1 << i for i in range(64) if i not in offsets]
        # Insert a variety of inputs, and check that we can find them afterwards
        
        self.corpus.insert_bulk(inputs)
        
        # Now let's try a bunch of combinarions where we twiddle few bits, and 
        # many.
        import itertools
        for count in range(8):
            for combo in itertools.combinations(masks, count):
                # Let's test it against each of these inputs
                for i in inputs:
                    for mask in combo:
                        i = i | mask
                    if count <= 3:
                        self.assertNotEqual(self.corpus.find_first(i), 0)
                    else:
                        self.assertEqual(self.corpus.find_first(i), 0)
    
    def test_find_all(self):
        # We should be able to find /all/ the fingerprints that we expect. This
        # also tests that results given back are equal to the original 
        # fingerprints and not the permuted one.
        inputs = [1 << i for i in range(63)]
        self.corpus.insert_bulk(inputs)
        self.assertEqual(set(self.corpus.find_all((1 << 63))), set(inputs))
    
    def test_remove(self):
        # Although I don't imagine there will be much call for this, we should 
        # be able to remove fingerprints from the corpus as well as insert them.
        inputs = [1 << i for i in range(30    )]
        remove = [1 << i for i in range(30, 63)]
        self.corpus.insert_bulk(inputs)
        self.corpus.insert_bulk(remove)
        self.corpus.remove_bulk(remove)
        self.assertEqual(set(self.corpus.find_all((1 << 63))), set(inputs))
    
    def test_permute(self):
        # We should be able to permute and unpermute and always get the original
        # number back. Unfortunately, it's kind of difficult to ensure that the
        # permutation is correct, but we'll have a limited number of examples.
        import random
        queries = [random.randint(0, 1 << 63) for i in range(100)]
        
        # Let's test with a few different combinations of differing bits and 
        # numbers of blocks
        configs = [(6, 3), (6, 2), (8, 6), (20, 16)]
        for config in configs:
            corpus = simhash.Corpus(*config)
            for t in corpus.tables:
                for q in queries:
                    self.assertEqual(t.unpermute(t.permute(q)), q)
    
    def test_get_all(self):
        # We should also be able to get a list of all the hashes that are in the
        # corpus
        inputs = range(1000)
        self.corpus.insert_bulk(inputs)
        self.assertEqual(self.corpus.hashes(), inputs)

if __name__ == '__main__':
    unittest.main()
