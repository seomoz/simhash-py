#! /usr/bin/env python

import re
import unittest

import simhash


class TestNumDifferingBits(unittest.TestCase):
    '''Tests about num_differing_bits'''

    def test_basic(self):
        a = 0xDEADBEEF
        b = 0xDEADBEAD
        self.assertEqual(2, simhash.num_differing_bits(a, b))


class TestCompute(unittest.TestCase):
    '''Tests about computing a simhash.'''

    def test_empty(self):
        self.assertEqual(0, simhash.compute([]))

    def test_repeat(self):
        number = 0xDEADBEEF
        self.assertEqual(number, simhash.compute([number] * 100))

    def test_inverse(self):
        hashes = [0xDEADBEEFDEADBEEF, 0x2152411021524110]
        self.assertEqual(64, simhash.num_differing_bits(*hashes))
        self.assertEqual(0, simhash.compute(hashes))

    def test_basic(self):
        hashes = [0xABCD, 0xBCDE, 0xCDEF]
        self.assertEqual(0xADCF, simhash.compute(hashes))


class TestFindAll(unittest.TestCase):
    '''Tests about find_all.'''

    def test_basic(self):
        hashes = [
            0x000000FF, 0x000000EF, 0x000000EE, 0x000000CE, 0x00000033,
            0x0000FF00, 0x0000EF00, 0x0000EE00, 0x0000CE00, 0x00003300,
            0x00FF0000, 0x00EF0000, 0x00EE0000, 0x00CE0000, 0x00330000,
            0xFF000000, 0xEF000000, 0xEE000000, 0xCE000000, 0x33000000
        ]
        expected = [
            (0x000000EF, 0x000000FF),
            (0x000000EE, 0x000000EF),
            (0x000000EE, 0x000000FF),
            (0x000000CE, 0x000000EE),
            (0x000000CE, 0x000000EF),
            (0x000000CE, 0x000000FF),
            (0x0000EF00, 0x0000FF00),
            (0x0000EE00, 0x0000EF00),
            (0x0000EE00, 0x0000FF00),
            (0x0000CE00, 0x0000EE00),
            (0x0000CE00, 0x0000EF00),
            (0x0000CE00, 0x0000FF00),
            (0x00EF0000, 0x00FF0000),
            (0x00EE0000, 0x00EF0000),
            (0x00EE0000, 0x00FF0000),
            (0x00CE0000, 0x00EE0000),
            (0x00CE0000, 0x00EF0000),
            (0x00CE0000, 0x00FF0000),
            (0xEF000000, 0xFF000000),
            (0xEE000000, 0xEF000000),
            (0xEE000000, 0xFF000000),
            (0xCE000000, 0xEE000000),
            (0xCE000000, 0xEF000000),
            (0xCE000000, 0xFF000000)
        ]
        for blocks in range(4, 10):
            self.assertEqual(
                sorted(expected), sorted(simhash.find_all(hashes, blocks, 3)))

    def test_diverse(self):
        hashes = [
            0x00000000, 0x10101000, 0x10100010, 0x10001010, 0x00101010,
                        0x01010100, 0x01010001, 0x01000101, 0x00010101
        ]
        expected = [
            (0x00000000, 0x10101000),
            (0x00000000, 0x10100010),
            (0x00000000, 0x10001010),
            (0x00000000, 0x00101010),
            (0x00000000, 0x01010100),
            (0x00000000, 0x01010001),
            (0x00000000, 0x01000101),
            (0x00000000, 0x00010101),
            (0x00101010, 0x10001010),
            (0x00101010, 0x10100010),
            (0x00101010, 0x10101000),
            (0x10001010, 0x10100010),
            (0x10001010, 0x10101000),
            (0x10100010, 0x10101000),
            (0x00010101, 0x01000101),
            (0x00010101, 0x01010001),
            (0x00010101, 0x01010100),
            (0x01000101, 0x01010001),
            (0x01000101, 0x01010100),
            (0x01010001, 0x01010100)
        ]
        for blocks in range(4, 10):
            self.assertEqual(
                sorted(expected), sorted(simhash.find_all(hashes, blocks, 3)))


class TestShingle(unittest.TestCase):
    '''Tests about computing shingles of tokens.'''

    def test_fewer_than_window(self):
        tokens = list(range(3))
        self.assertEqual([], list(simhash.shingle(tokens, 4)))

    def test_zero_window_size(self):
        tokens = list(range(10))
        with self.assertRaises(ValueError):
            list(simhash.shingle(tokens, 0))

    def test_negative_window_size(self):
        tokens = list(range(10))
        with self.assertRaises(ValueError):
            list(simhash.shingle(tokens, -1))

    def test_basic(self):
        tokens = list(range(10))
        expected = [
            [0, 1, 2, 3],
            [1, 2, 3, 4],
            [2, 3, 4, 5],
            [3, 4, 5, 6],
            [4, 5, 6, 7],
            [5, 6, 7, 8],
            [6, 7, 8, 9]
        ]
        self.assertEqual(expected, list(simhash.shingle(tokens, 4)))


class TestFunctional(unittest.TestCase):
    '''Can the tool be used functionally.'''

    MATCH_THRESHOLD = 3

    jabberwocky = '''
        Twas brillig, and the slithy toves
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

    def compute(self, text):
        tokens = re.split(r'\W+', text.lower(), flags=re.UNICODE)
        shingles = [''.join(shingle) for shingle in
                    simhash.shingle(''.join(tokens), 4)]
        hashes = [simhash.unsigned_hash(s.encode('utf8')) for s in shingles]
        return simhash.compute(hashes)

    def test_added_text(self):
        a = self.compute(self.jabberwocky)
        b = self.compute(
            self.jabberwocky + ' - Lewis Carroll (Alice in Wonderland)')

        self.assertLessEqual(
            simhash.num_differing_bits(a, b),
            self.MATCH_THRESHOLD)

    def test_identical_text(self):
        a = self.compute(self.jabberwocky)
        b = self.compute(self.jabberwocky)
        self.assertEqual(0, simhash.num_differing_bits(a, b))

    def test_different(self):
        a = self.compute(self.jabberwocky)
        b = self.compute(self.pope)
        self.assertGreater(
            simhash.num_differing_bits(a, b),
            self.MATCH_THRESHOLD)
