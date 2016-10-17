#! /usr/bin/env python

from .simhash import unsigned_hash, num_differing_bits, compute, find_all
import os
import sys
from six.moves import range as six_range

if sys.version_info >= (3,2) and \
        ('PYTHONHASHSEED' not in os.environ or \
         int(os.environ['PYTHONHASHSEED']) != 0):
    # overwrite the unsigned_hash function
    def unsigned_hash(obj):
        '''The built-in hash suitable for use as a hash_t.'''
        raise ValueError('Starting from version 3.2, Python enables hash randomization by default:'
                         'Please set the environement variable PYTHONHASHSEED=0 to use unsigned_hash function'
                         'see https://docs.python.org/3/using/cmdline.html#envvar-PYTHONHASHSEED')



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
