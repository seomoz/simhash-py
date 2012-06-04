#include "simhash.h"

namespace Simhash {
    Simhash::Table::Table(size_t d, std::vector<hash_t>& p):
        widths(),
        offsets(),
        masks(p),
        differing_bits(d),
        search_mask(0),
        judy(NULL)
    {
    	std::vector<hash_t>::iterator mask_it( masks.begin());
    	size_t j(0), i(0), width(0); // A couple of counters
    	/* To more easily and reasonably-efficiently calculate the permutations 
    	 * of each of the hashes we insert, and since each block is just 
    	 * contiguous  set bits, we will calculate the widths of each of these 
    	 * blocks, and the  offset of their rightmost bit. */
    	for(; mask_it != masks.end(); ++mask_it) {
    		hash_t mask = *mask_it;
    		/* Find where the 1's start, and where they end. Yes. It's ugly, but 
    		 * it works. */
    		for (i = 0;          !((1UL << i) & mask); ++i) {}
    		for (j = i; j < 64 && ((1UL << j) & mask); ++j) {}
    		offsets.push_back(i    );
    		 widths.push_back(j - i);
    	}
    	
    	/* Alright, we have to determine the low and high masks for this 
    	 * paritcular table. If we are searching for up to /d/ differing bits, 
    	 * then we should  include all but the last /d/ blocks in our mask.
    	 *
    	 * After this, width should hold the number of bits that are in all but 
    	 * the last d blocks */
    	std::vector<size_t>::iterator width_it(widths.begin());
    	for (; d < widths.size(); ++d, ++width_it) {
    		width += *width_it;
    	}
    	
    	/* Set the first /width/ bits in the low mask to 1, and then shift it up
    	 * until it's a full 64-bit number. */
    	for(i = 0    ; i < width; ++i) { search_mask = (search_mask << 1) | 1; }
    	for(i = width; i < 64   ; ++i) { search_mask =  search_mask << 1;      }
    };
    
    /* Hash permutation */
    hash_t Simhash::Table::permute(hash_t hash) {
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
    
    /* Insertion */
    template <class InputIterator>
    void Simhash::Table::insert(InputIterator first, InputIterator last) {
    	hash_t result(0);
    	for(; first != last; ++first) {
    		J1S(result, judy, permute(*first));
    	}
    }
    
    void Simhash::Table::insert(hash_t hash) {
    	hash_t result(0);
    	J1S(result, judy, permute(hash));
    }
    
    /* Removal */
    template <class InputIterator>
    void Simhash::Table::remove(InputIterator first, InputIterator last) {
    	hash_t result(0);
    	for(; first != last; ++first) {
    		J1U(result, judy, permute(*first));
    	}
    }
    
    void Simhash::Table::remove(hash_t hash) {
    	hash_t result(0);
    	J1U(result, judy, permute(hash));
    }
    
    /* Find duplicates */
    hash_t Simhash::Table::find_first(hash_t query) {
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
}
