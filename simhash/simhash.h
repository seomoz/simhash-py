#ifndef FRESH_SIMHASH
#define FRESH_SIMHASH

// Judy trees are the basis of lookup in this
#include <Judy.h>

#include "util.h"

#include <stdint.h>
#include <vector>

class table {
	public:
		table(size_t d, std::vector<hash_t>& p);
		~table() {
			hash_t result(0);
			J1FA(result, judy);
		};
		
		/* Insertion
		 *
		 * Insertion comes in two flavors -- one form that accepts a start and
		 * end iterator for convenient batch insertions, and a one-off insertion
		 * that accepts a single hash_t. */
		template <class InputIterator>
		void insert(InputIterator first, InputIterator last);
		hash_t insert(hash_t hash);
		
		/* Removal
		 *
		 * Like insertion, there are two forms of removal -- one that accepts a
		 * start and end iterator, and another that accepts a single hash. */
		template <class InputIterator>
		void remove(InputIterator first, InputIterator last);
		hash_t remove(hash_t hash);
		
		/* Find duplicates
		 *
		 * In some cases we're interested in merely finding if /any/ duplicates
		 * exist, in which case you may use find_first. In cases where you need
		 * a complete list of all near duplicates, find_all returns an iterator
		 * that can be used to retrieve all of the near-duplicates. */
		hash_t find_first(hash_t hash);
		
		/* Hash permutation
		 *
		 * In order to correctly insert hashes into the table, they must be 
		 * permuted subject to the permutation pattern for this table. NOTE:
		 * users do not need to call this method. It's merely made publish to
		 * ease testing and for those curious about its workings. */
		hash_t permute(hash_t hash);
	private:
		// Private and undefined to prevent their use
		table();
		table(const table& other);
		
		/* Permutation internals
		 *
		 * Permuting hashes is actually kind of a pain. A relatively efficient
		 * and understandable way to do this is to keep track of the widhts of
		 * each of the blocks of bits, and then the offsets of their lsb. The
		 * blocks themselves are stored as bitmasks. */
		std::vector<size_t> widths;
		std::vector<size_t> offsets;
		std::vector<hash_t> masks;
		
		/* Differing bits
		 *
		 * The maximum number of bits by which two hashes may differ while still
		 * considered near-duplicates */
		size_t differing_bits;
		 
		/* Search mask
		 *
		 * When searching for a potential match, a match may be less than the 
		 * query, so it's insufficient to simply search for the query. This mask
		 * sets all the bits in the last _differing_bits_ blocks to 0, which is
		 * smaller than is necessary, but the easiest correct number to compute.
		 * The highest number which must be potentially searched is the query
		 * with all the bits in the last _differing_bits_ blocks set to 1. */
		hash_t search_mask;
		
		/* Judy array
		 *
		 * A judy array is the underlying data structure that keeps these keys
		 * in sorted order. It was chosen per Brandon's suggestion, after 
		 * finding that std::set was far too slow and consumed too much memory,
		 * and while considering a std::map<hash_t, std::vector<hash_t>>. The
		 * judy array has thus far proven both space and memory-efficient. */
		Pvoid_t judy;
};

#endif
