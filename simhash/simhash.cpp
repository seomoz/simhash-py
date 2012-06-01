#include "simhash.h"

table::table(size_t d, std::vector<hash_t>& p): widths(), offsets(), masks(p), differing_bits(d), search_mask(0), judy(NULL) {
	std::vector<hash_t>::iterator mask_it( masks.begin());
	size_t j(0), i(0), width(0); // A couple of counters
	/* To more easily and reasonably-efficiently calculate the permutations of
	 * each of the hashes we insert, and since each block is just contiguous 
	 * set bits, we will calculate the widths of each of these blocks, and the 
	 * offset of their rightmost bit. */
	for(; mask_it != masks.end(); ++mask_it) {
		hash_t mask = *mask_it;
		// Find where the 1's start, and where they end. Yes. It's ugly, but it
		// works.
		for (     ;          !((1UL << i) & mask); ++i) {}
		for (j = i; j < 64 && ((1UL << j) & mask); ++j) {}
		offsets.push_back(i    );
		 widths.push_back(j - i);
	}
	
	/* Alright, we have to determine the low and high masks for this paritcular
	 * table. If we are searching for up to /d/ differing bits, then we should 
	 * include all but the last /d/ blocks in our mask.
	 *
	 * After this, width should hold the number of bits that are in all but the
	 * last d blocks */
	std::vector<size_t>::iterator width_it(widths.begin());
	for (; d < widths.size(); ++d, ++width_it) {
		width += *width_it;
	}
	
	/* Set the first /width/ bits in the low mask to 1, and then shift it up
	 * until it's a full 64-bit number. */
	for(i = 0    ; i < width; ++i) { search_mask = (search_mask << 1) | 1; }
	for(i = width; i < 64   ; ++i) { search_mask =  search_mask << 1;      }
};

/* Hash permutation
 *
 * In order to correctly insert hashes into the table, they must be 
 * permuted subject to the permutation pattern for this table. NOTE:
 * users do not need to call this method. It's merely made publish to
 * ease testing and for those curious about its workings. */
hash_t table::permute(hash_t hash) {
	std::vector<hash_t>::iterator masks_it(   masks.begin());
	std::vector<size_t>::iterator offset_it(offsets.begin());
	std::vector<size_t>::iterator width_it(  widths.begin());
	
	hash_t result(0);
	for (; masks_it != masks.end(); ++masks_it, ++offset_it, ++width_it) {
		result = result << *width_it;
		result = result |  ((hash & *masks_it) >> *offset_it);
	}
	return result;
}

/* Insertion
 *
 * Insertion comes in two flavors -- one form that accepts a start and
 * end iterator for convenient batch insertions, and a one-off insertion
 * that accepts a single hash_t. */
template <class InputIterator>
void table::insert(InputIterator first, InputIterator last) {
	hash_t result(0);
	for(; first != last; ++first) {
		J1S(result, judy, permute(*first));
	}
}

hash_t table::insert(hash_t hash) {
	hash_t result(0);
	J1S(result, judy, permute(hash));
	return 0;
}

/* Removal
 *
 * Like insertion, there are two forms of removal -- one that accepts a
 * start and end iterator, and another that accepts a single hash. */
template <class InputIterator>
void table::remove(InputIterator first, InputIterator last) {
	hash_t result(0);
	for(; first != last; ++first) {
		J1U(result, judy, permute(*first));
	}
}

hash_t table::remove(hash_t hash) {
	hash_t result(0);
	J1U(result, judy, permute(hash));
	return 0;
}

/* Find duplicates
 *
 * In some cases we're interested in merely finding if /any/ duplicates
 * exist, in which case you may use find_first. In cases where you need
 * a complete list of all near duplicates, find_all returns an iterator
 * that can be used to retrieve all of the near-duplicates. */		
hash_t table::find_first(hash_t query) {
	// We have to perform our permutation on the 
	query = permute(query);
	
	hash_t low( query &  search_mask);
	hash_t high(query | ~search_mask);
	hash_t result(0);
	J1F(result, judy, low);
	
	for(; result && (low <= high); ) {
		if (num_differing_bits(low, query) <= differing_bits) {
			return low;
		}
		J1N(result, judy, low);
	}
	return 0;
}
