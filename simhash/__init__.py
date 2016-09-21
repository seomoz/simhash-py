#! /usr/bin/env python

from .simhash import unsigned_hash, num_differing_bits, compute, find_all

def shingle(tokens, window=4):
    '''A generator for a moving window of the provided tokens.'''
    if window <= 0:
        raise ValueError('Window size must be positive')
    its = []
    for number in range(window):
        it = iter(tokens)
        its.append(it)
        for _ in range(number):
            next(it)
    while True:
        yield list(map(next, its))
