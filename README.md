Simhash Near-Duplicate Detection
================================
This library enables the identification of near-duplicate documents. In this 
context, a document is simply a bytestring -- be it the content of a webpage
or an essay or a text file.

It contains a C++-level extension designed to speed up queries, as well as 
facilities to distribute the lookup tables. This implementation follows that
described in the Google paper on the subject of near-duplicate detection with 
simhash.

Architecture
============
Each document gets associated with a 64-bit hash calculated using a rolling 
hash function and simhash. This hash can be thought of as a fingerprint for
the content. Two documents are considered near-duplicates if their hashes differ
by at most _k_ bits, a parameter chosen by the user.

In this context, there is a large corpus of known fingerprints, and we would 
like to determine all the fingerprints that differ by our query byt _k_ or fewer
bits. To accomplish this, we divide up the 64 bits into at _m_ blocks, where 
_m_ is greater than _k_. If hashes A and B differ by at most _k_ bits, then at
least _m - k_ groups are the same.

Choosing all the unique combinations of _m - k_ blocks, we perform a permutation
on each of the hashes for the documents so that those blocks are first in the 
hash. Perhaps a picture would illustrate it better:

	64------54|53------43|42-----33|32------22|21------11|10------0|
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





