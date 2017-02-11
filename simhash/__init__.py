#! /usr/bin/env python

from .simhash import unsigned_hash, num_differing_bits, compute, find_all
from six.moves import range as six_range


def shingle(tokens, window=4):
    '''A generator for a moving window of the provided tokens.'''
    if window <= 0:
        raise ValueError('Window size must be positive')
    its = []
    for number in six_range(window):
        it = iter(tokens)
        its.append(it)
        for _ in six_range(number):
            next(it)
    while True:
        yield [next(it) for it in its]
