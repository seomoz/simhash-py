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

Usage
=====
`simhash` differs from most hashes in that its goal is to have two similar documents
produce similar hashes, where most hashes have the goal of producing very different
hashes even in the face of small changes to the input.

The input to `simhash` is a list of hashes representative of a document. The output is an
unsigned 64-bit integer. The input list of hashes can be produced in several ways, but
one common mechanism is to:

1. tokenize the document
1. consider overlapping shingles of these tokens (`simhash.shingle`)
1. `hash` these overlapping shingles
1. input these hashes into `simhash.compute`

This has the effect of considering phrases in a document, rather than just a bag of the
words in it.

Once we've produced a `simhash`, we would like to compare it to other documents. For two
documents to be considered near-duplicates, they must have few bits that differ. We can
compare two documents:

```python
import simhash

a = simhash.compute(...)
b = simhash.compute(...)
simhash.num_differing_bits(a, b)
```

One of the key advantages of `simhash` is that it does not require `O(n^2)` time to find
all near-duplicate pairs from a set of hashes. Given a whole set of `simhashes`, we can
find all pairs efficiently:

```python
import simhash

# The `simhash`-es from our documents
hashes = []

# Number of blocks to use (more in the next section)
blocks = 4
# Number of bits that may differ in matching pairs
distance = 3
matches = simhash.find_all(hashes, blocks, distance)
```

All the matches returned are guaranteed to be _all_ pairs where the hashes differ by
`distance` bits or fewer. The `blocks` parameter is less intuitive, but is best described
in [this article](https://moz.com/devblog/near-duplicate-detection/) or in
[the paper](http://www2007.cpsc.ucalgary.ca/papers/paper215.pdf). The best parameter to
choose depends on the distribution of the input simhashes, but it must always be at least
one greater than the provided `distance`.

Internally, `find_all` takes `blocks C distance` passes to complete. The idea is that as
that value increases (for instance by increasing `blocks`), each pass completes faster.
In terms of memory, `find_all` takes `O(hashes + matches)` memory.

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
