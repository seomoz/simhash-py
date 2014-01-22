#! /usr/bin/env python

import unittest
import simhash


class TestHash(unittest.TestCase):
    jabberwocky = '''Twas brillig, and the slithy toves
      Did gyre and gimble in the wabe:
    All mimsy were the borogoves,
      And the mome raths outgrabe.
    "Beware the Jabberwock, my son!
      The jaws that bite, the claws that catch!
    Beware the Jubjub bird, and shun
      The frumious Bandersnatch!"
    He took his vorpal sword in hand:
      Long time the manxome foe he sought --
    So rested he by the Tumtum tree,
      And stood awhile in thought.
    And, as in uffish thought he stood,
      The Jabberwock, with eyes of flame,
    Came whiffling through the tulgey wood,
      And burbled as it came!
    One, two! One, two! And through and through
      The vorpal blade went snicker-snack!
    He left it dead, and with its head
      He went galumphing back.
    "And, has thou slain the Jabberwock?
      Come to my arms, my beamish boy!
    O frabjous day! Callooh! Callay!'
      He chortled in his joy.
    `Twas brillig, and the slithy toves
      Did gyre and gimble in the wabe;
    All mimsy were the borogoves,
      And the mome raths outgrabe.'''

    pope = '''There once was a man named 'Pope'
    Who loved an oscilloscope
      And the cyclical trace
      Of their carnal embrace
    Had a damned-near infinite slope!'''

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def diff_bits(self, a, b):
        a = a ^ b
        count = 0
        while a:
            count += 1
            a = a & (a - 1)
        return count

    def assertMatch(self, a, b, threshold=3):
        a_h = simhash.hash(a)
        b_h = simhash.hash(b)
        diff = self.diff_bits(a_h, b_h)
        self.assertLessEqual(diff, threshold,
            'Expected (%i) "%s" to match (%i) "%s" (%i)' % (
                a_h, a[0:50], b_h, b[0:50], diff))

    def assertNotMatch(self, a, b, threshold=3):
        a_h = simhash.hash(a)
        b_h = simhash.hash(b)
        diff = self.diff_bits(a_h, b_h)
        self.assertGreater(diff, threshold,
            'Expected (%i) "%s" to NOT match (%i) "%s" (%i)' % (
                a_h, a[0:50], b_h, b[0:50], diff))

    def test_exact(self):
        # Two strings that are exactly the same must match exactly.
        self.assertMatch(TestHash.jabberwocky, TestHash.jabberwocky)

    def test_similarity(self):
        # Two string that are very similar should appear similar
        self.assertMatch(TestHash.jabberwocky, TestHash.jabberwocky +
            'by Lewis Carroll', 4)

    def test_dissimilar(self):
        # Two strings that are very different should not appear similar
        self.assertNotMatch(TestHash.jabberwocky, TestHash.pope)

    def test_whitespace(self):
        # The removal of _excess_ should not affect the results
        import re
        self.assertMatch(re.sub('\s+', ' ', TestHash.jabberwocky),
            TestHash.jabberwocky)

    def test_transposition(self):
        # We should be able to transpose a small number of words and still
        # score well
        import re
        a = re.split('\s+', TestHash.jabberwocky)
        for i in range(5, 100, 20):
            a[i:i + 2] = a[i + 1:i - 1:-1]

        self.assertNotEqual(a, re.split('\s+', TestHash.jabberwocky))
        a = ' '.join(a)
        # Good match...
        self.assertMatch(a, TestHash.jabberwocky)
        # But not perfect match
        self.assertNotMatch(a, TestHash.jabberwocky, 0)

    def test_shuffled(self):
        # Ideally, our simhashing should depend on the order of words,
        # and not just their existence.
        #
        # This test actually originally reversed the order of the words, but
        # since we use a moving window, the reversed order simhash is by
        # definition identical to the forward order
        import re
        import random
        shuffled = re.split('\s+', TestHash.jabberwocky)
        random.shuffle(shuffled)
        self.assertNotMatch(TestHash.jabberwocky, ' '.join(shuffled))

    def test_negative_numbers(self):
        # We should be able to hash on negative numbers. In not throwing an
        # exception, this test passes
        simhash.hash_fp(range(-10, 10))


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
        masks = [1 << i for i in offsets]
        inputs = [1 << i for i in range(64) if i not in offsets]
        # Insert a variety of inputs, and check that we can find them
        # afterwards

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
        # be able to remove fingerprints from the corpus as well as insert them
        inputs = [1 << i for i in range(30)]
        remove = [1 << i for i in range(30, 63)]
        self.corpus.insert_bulk(inputs)
        self.corpus.insert_bulk(remove)
        self.corpus.remove_bulk(remove)
        self.assertEqual(set(self.corpus.find_all((1 << 63))), set(inputs))

    def test_permute(self):
        # We should be able to permute and unpermute and always get the
        # original number back. Unfortunately, it's kind of difficult to
        # ensure that the permutation is correct, but we'll have a limited
        # number of examples.
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
        # We should also be able to get a list of all the hashes that are in
        # the corpus
        inputs = range(1000)
        self.corpus.insert_bulk(inputs)
        self.assertEqual(self.corpus.hashes(), inputs)

if __name__ == '__main__':
    unittest.main()
