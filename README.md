Simhash Near-Duplicate Detection
================================
[![Build Status](https://travis-ci.org/seomoz/simhash-py.svg?branch=master)](https://travis-ci.org/seomoz/simhash-py)

![Status: Production](https://img.shields.io/badge/status-production-green.svg?style=flat)
![Team: Big Data](https://img.shields.io/badge/team-big_data-green.svg?style=flat)
![Scope: External](https://img.shields.io/badge/scope-external-green.svg?style=flat)
![Open Source: MIT](https://img.shields.io/badge/open_source-MIT-green.svg?style=flat)
![Critical: Yes](https://img.shields.io/badge/critical-yes-red.svg?style=flat)

This library enables the efficient identification of near-duplicate documents using
`simhash` using a C++ extension.

Overview
========
`simhash` is a bit of an overloaded word. It is often used interchangeably for:
1) a function to generate a simhash from input, and 2) method used for identifying
near-duplicates from a set of simhashes. This document will try to preserve that
distinction.

The `simhash` hashing function accepts a list of input hashes and produces a
single hash. It doesn't matter how long the input list is - `simhash` will always
give a hash of the same size, 64 bits. The details of this transformation aren't
particularly interesting, but the input to this function is very important. **The
way the input list of hashes is computed has a huge impact on the quality of the
matches found.**

If there is one thing that we should make very clear, it's that while this library
provides tooling to help with 1) performing the simhash hashing function on an
input list of hashes and 2) performing the duplication detection on the resulting
hashes, **users of the library must be able to convert each of their documents
to a representative list of hashes in order to get good results.** How that
"representative" list is created is highly application-specific: the techniques
that could be used on a photograph are very different from those that could be
used on a webpage.

Practical Considerations
========================
There is a [writeup](https://moz.com/devblog/near-duplicate-detection/) that goes
into more detail, but we will try to summarize some important points here.

Suppose we have a bunch of text documents we want to compare. Before identifying
duplicates, we much first generate the simhashes for each. To do so, we must
turn each document into a list of hashes. Let's start by just taking all the
words of the document in order, and put it through the `unsigned_hash` function:

```python
from simhash import unsigned_hash, compute

document = 'some really long block of text...'
words = document.split(' ')

# We'll see in a minute that this is a BAD technique. Do not do this.
hashes = map(unsigned_hash, words)
simhash = compute(hashes)
```

The problem with this is that a very different document could have the exact same
hash:

```python
# These documents would have the exact same simhashes if we
a = 'one two three four five ...'
b = 'two four three five one ...'
```

To improve this situation, a common technique is to use "shingling." Just like
shingles on a roof overlap, we will consider overlapping ranges of words. For
example, for the text `one two three four five ...`, we could get shingles of
size three: `one two three`, `two three four`, `three four five`, `four five ...`.
Using this technique ensures that the order of words is important, not just which
words are used.

We provide a `shingle` function to help with this:

```python
from simhash import shingle

words = ...
# Use four words per shingle
shingles = shingle(words, window=4)
# Use the shingles when computing the hashes, instead of words
hashes = map(unsigned_hash, shingles)
simhash = compute(hashes)
```

Two simhashes are considered near-duplicates if their simhashes differ by a few
bits. Exactly how many "a few" means is highly application-specific (as much so
as the method for computing the input hashes to the simhash function).

Example Code
============
If you were looking for a shortcut for getting good near-duplicates, this is
the closest thing to it. **However, read and understand this document or risk
sub-par duplicate-detection.** And with that, let's dive into an example using
tokenized text documents:

```python
from simhash import shingle, unsigned_hash, compute, find_all, num_differing_bits
from collections import defaultdict
import itertools

def simhashDocument(doc):
    '''Do rudimentary tokenization and produce a simhash.'''
    shingles = shingle(doc.split(' '), window=4)
    return compute(map(unsigned_hash, shingles))

# We need to keep a mapping of simhashes to the original document. This is a
# dict to lists, because theoretically documents could collide.
simhashMap = defaultdict(list)

# The paths to each of the document
paths = [...]

# Compute all the simhashes
for path in paths:
    with open(path) as fin:
        doc = fin.read()
        simhashMap[simhashDocument(doc)].append(path)

# Find all the matching pairs
#
# The different_bits parameter is application-specific, and we'll talk about how
# to pick a good value in the next section.
#
# The number_of_blocks parameter affects performance. It must be in the range
# [different_bits + 1, 64]. Try starting with different_bits + 2 and tweak from
# there for the best performance.
pairs = find_all(simhashMap.keys(), number_of_blocks=6, different_bits=3)

# For each pair in the matches, all associated documents are near-duplicates
for a, b in pairs:
    distance = num_differing_bits(a, b)
    aPaths = simhashMap[a]
    bPaths = simhashMap[b]
    for aPath, bPath in itertools.product(aPaths, bPaths):
        print '%s is a near-duplicate of %s (distance = %s)' % (aPath, bPath, distance)

# Technically, all the documents with the same simhash are near-duplicates as
# well, but that's left as an exercise for the reader.
```

Choosing Parameters
===================

The `number_of_blocks` parameter is not particularly intuitive. They are described
in more detail in this article](https://moz.com/devblog/near-duplicate-detection/) or in
[the paper](http://www2007.cpsc.ucalgary.ca/papers/paper215.pdf). Internally,
`find_all` takes `number_of_blocks C different_bits` passes to complete. With
more blocks, the number of passes required increases combinatorially, but each
pass becomes faster. It is important to find the correct balance for performance.

The pairs returned by `find_all` are guaranteed to be _all_ the pairs where the
simhashes differ by `different_bits` or fewer. This may find all the documents
you are attempting to match, but that gets back to the two main factors that
determine the quality of matches: 1) the way the representative document hashes
are computed, and 2) the `different_bits` parameter.

Choosing the best `different_bits` parameter is difficult. It usually involves
taking an example set of documents and a gold standard of all the near-duplicate
document pairs, and then evaluating the
[precision and recall](https://en.wikipedia.org/wiki/Precision_and_recall) for
different choices of parameters. While perfect results are unlikely, it is
certainly possible to get both precision and recall to be very high. The big
upside to the `simhash` approach is that it can be easily run on datasets that
would otherwise be prohibitively large.

Building
========
This is installable via `pip`:

```bash
pip install git+https://github.com/seomoz/simhash-py.git
```

It can also be built from `git`:

```bash
git submodule update --init --recursive
python setup.py install
```

or 
```bash
pip install simhash-py
```
under osx, you should 
```bash
export MACOSX_DEPLOYMENT_TARGET = 10.x (10.9,10.10...)
```
first

Benchmark
=========
This is a rough benchmark, but should help to give you an idea of the order of
magnitude for the performance available. Running on a single core on a `vagrant` instance
on a 2015 MacBook Pro:

```bash
$ ./bench.py --random 1000000 --blocks 5 --bits 3
Generating 1000000 hashes
Starting Find all
     Ran Find all in 1.595416s
```

Architecture
============
Each document gets associated with a 64-bit hash calculated using a rolling
hash function and simhash. This hash can be thought of as a fingerprint for
the content. Two documents are considered near-duplicates if their hashes differ
by at most _k_ bits, a parameter chosen by the user.

In this context, there is a large corpus of known fingerprints, and we would
like to determine all the fingerprints that differ by our query by _k_ or fewer
bits. To accomplish this, we divide up the 64 bits into at _m_ blocks, where
_m_ is greater than _k_. If hashes A and B differ by at most _k_ bits, then at
least _m - k_ groups are the same.

Choosing all the unique combinations of _m - k_ blocks, we perform a permutation
on each of the hashes for the documents so that those blocks are first in the
hash. Perhaps a picture would illustrate it better:

    63------53|52------42|41-----32|31------21|20------10|09------0|
    |    A    |     B    |    C    |     D    |     E    |    F    |

    If m = 6, k = 3, we'll choose permutations:
    - A B C D E F
    - A B D C E F
    - A B E C D F
    ...
    - C D F A B E
    - C E F A B D
    - D E F A B C

This generates a number of tables that can be put into sorted order, and then a
small range of candidates can be found in each of those tables for a query, and
then each candidate in that range can be compared to our query.

The corpus is represented by the union of these tables, could conceivably be
hosted on a separate machine. And each of these tables is also amenable to
sharding, where each shard would comprise a contiguous range of numbers. For
example, you might divide a table into 256 shards, where each shard is
associated with each of the possible first bytes.

The best partitioning remains to be seen, likely from experimentation, but the
basis of this is the `table`. The `table` tracks hashes inserted into it subject
to a permutation associated with the table. This permutation is described as a
vector of bitmasks of contiguous bit ranges, whose populations sum to 64.

Example
=======

Let's suppose that our corpus has a fingerprint:

    0100101110111011001000101111101110111100001010011101100110110101

and we have a query:

    0100101110111011011000101111101110011100001010011100100110110101

and they differ by only three bits which happen to fall in blocks B, D and E:

    63------53|52------42|41-----32|31------21|20------10|09------0|
    |    A    |     B    |    C    |     D    |     E    |    F    |
    |         |          |         |          |          |         |
    0000000000000000010000000000000000100000000000000001000000000000

Since any fingerprint matching the query differs by at most 3 bits, at most 3
blocks can differ, and at least 3 must match. Whatever table has the 3 blocks
that do not differ as the leading blocks will match the query when doing a scan.
In this case, the table that's permuted `A C F B D E` will match. It's important
to note that it's possible for a query to match from more than one table. For
example, if two of the non-matching bits are in the same block, or the query
differs by fewer than 3 bits.

32-Bit Systems
==============
The only requirement of `simhash-py` is that it has `uint64_t`.
