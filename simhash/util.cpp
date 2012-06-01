#include "util.h"
#include "hash.h"

/* Params:
 *      s - a string to hash
 * length - length of said string
 * window - number of characters included in the moving window of the 
 *           rolling hash function
 *
 * Returns: a hash representative of the content */
hash_t simhash(char* s, size_t length, size_t window=4) {
	/* Simhash works by calculating the hashes of overlaping windows of the
	 * input, and then for each bit of that hash, increments a corresponding 
	 * count. At the end, each of the counts is transformed back into a bit by
	 * whether or not the count is positive or negative */
	
	// Counts
	int64_t v[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
	};
	
	hash_t hash(0);      // The hash we're trying to produce
	uint32_t a(0), b(0); // Intermediates we use with the hashing function
	uint32_t i(0), j(0); // A couple of counters
	
	/* We're going to be doing a rolling hash function here. We need to generate
	 * the first few characters'-worth before we get going, and then we should
	 * be able to incrementally update the hashes that are produced as we stream
	 * in the rest */
	if (length < (window + 1)) {
		return 0;
	} else {
		hashlittle2(static_cast<void*>(s), window, &a, &b);
		s += window;
		for (i = window; i < length; ++i, ++s) {
			hashlittle2(static_cast<void*>(s), 1, &a, &b);
			/* Update the hash array. For each of the bits in the output hash
			 * (whose bits are the union of a and b), increment or decrement the
			 * corresponding count */
			for (j = 0; j < 32; ++j) {
				v[j   ] += ((a & (1 << j)) ? 1 : -1); // Lower bits
				v[j+32] += ((b & (1 << j)) ? 1 : -1); // Upper bits
			}
		}
	}
	
	/* With counts appropriately tallied, create a 1 bit for each of the counts
	 * that's positive. That result is the hash. */
	for (j = 0; j < 64; ++j) {
		if (v[j] > 0) {
			hash = hash | (1 << j);
		}
	}
	return hash;
}

/* Params:
 *      a - reference number
 *      b - query number
 *
 * Returns: the number of bits that differ between a and b */
size_t num_differing_bits(hash_t a, hash_t b) {
	size_t count(0);
	hash_t n = a ^ b;
	while ((n = (n & (n - 1)))) {
		++count;
	}
	return count + 1;
}
