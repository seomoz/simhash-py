#! /usr/bin/env python
from __future__ import print_function

import time
import random
import simhash
import argparse

# Generate some random hashes with known
parser = argparse.ArgumentParser(description='Run a quick bench')
parser.add_argument('--random', dest='random', type=int, default=None,
    help='Generate N random hashes for querying')
parser.add_argument('--blocks', dest='blocks', type=int, default=6,
    help='Number of blocks to divide 64-bit hashes into')
parser.add_argument('--bits', dest='bits', type=int, default=3,
    help='How many bits may differ')
parser.add_argument('--hashes', dest='hashes', type=str, default=None,
    help='Path to file with hashes to insert')

args = parser.parse_args()

hashes  = []

if args.hashes:
    with open(args.hashes) as f:
        hashes = [int(l) for l in f.split('\n')]

if args.random:
    if args.hashes:
        print('Random supplied with --hashes')
        exit(1)

    if not hashes:
        print 'Generating %i hashes' % args.random
        hashes = [random.randint(0, 1 << 64) for i in range(args.random)]
elif not args.hashes:
    print('No hashes or queries supplied')
    exit(2)

class Timer(object):
    def __init__(self, name):
        self.name = name

    def __enter__(self):
        self.start = -time.time()
        print('Starting %s' % self.name)
        return self

    def __exit__(self, t, v, tb):
        self.start += time.time()
        if t:
            print('  Failed %s in %fs' % (self.name, self.start))
        else:
            print('     Ran %s in %fs' % (self.name, self.start))

with Timer('Find all'):
    len(simhash.find_all(hashes, args.blocks, args.bits))
