Simhash Near-Duplicate Detection
================================
This library enables the identification of near-duplicate documents. In this 
context, a document is simply a bytestring -- be it the content of a webpage
or an essay or a text file.

It contains a C++-level extension designed to speed up queries, as well as 
facilities to distribute the lookup tables. This implementation follows that
described in the Google paper on the subject of near-duplicate detection with 
simhash.

Building
========
This library links against [`libJudy`](http://judy.sourceforge.net/), which must
be installed before building this. It also depends on Cython. With those pieces 
in place, it's business as usual

	python setup.py install

Usage
=====
A `Corpus` is a collection of all the tables necessary to perform the query 
efficiently. There are two parameters, `num_blocks` and `diff_bits` which 
describe the number of blocks into which the 64-bit hashes should be divided
(see more about this below) and the number of bits by which two hashes may 
differ before being considered near-duplicates. The number of tables needed is
a function of these two parameters.

	import simhash
	
	# 6 blocks, 3 bits may differ
	corpus = simhash.Corpus(6, 3)

With a corpus, you can then insert, remove and query the data structure. You may
be interested in just _any_ near-duplicate fingerprint in which case you can use
`find_first` or `find_first_bulk`. If you're interested in finding _all_ matches
then you should use `find_all` or `find_all_bulk`:

	# Generate 1M random hashes and random queries
	import random
	hashes  = [random.randint(0, 1 << 64) for i in range(1000000)]
	queries = [random.randint(0, 1 << 64) for i in range(1000000)]
	
	# Insert the hashes
	corpus.insert_bulk(hashes)
	
	# Find matches; returns a list of results, each element contains the match
	# for the corresponding element in the query
	matches = corpus.find_first_bulk(queries)
	
	# Find more matches; returns a list of lists, each of which corresponds to 
	# the query of the same index
	matches = corpus.find_all_bulk(queries)

Benchmark
=========
This is a rough benchmark, but should help to give you an idea of the order of 
magnitude for the performance available. Running on a single core on a 2011-ish
MacBook Pro:

	# ./bench.py --random 1000000 --blocks 5 --bits 3
	Generating 1000000 hashes
	Generating 1000000 queries
	Starting Bulk Insertion
		 Ran Bulk Insertion in 2.534197s
	Starting Bulk Find First
		 Ran Bulk Find First in 4.795310s
	Starting Bulk Find All
		 Ran Bulk Find All in 7.415205s
	Starting Bulk Removal
		 Ran Bulk Removal in 3.346022s

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