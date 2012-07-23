#include "simhash.h"

#include <iostream>

namespace Simhash {
    size_t num_differing_bits(Simhash::hash_t a, Simhash::hash_t b) {
        size_t count(0);
        Simhash::hash_t n = a ^ b;
        while ((n = (n & (n - 1)))) {
            ++count;
        }
        return count + 1;
    }

    Table::Table(size_t d, std::vector<hash_t>& p):
        offsets(),
        forward_masks(p),
        reverse_masks(),
        differing_bits(d),
        search_mask(0),
        judy(NULL)
    {
        std::vector<size_t> widths;
    	std::vector<hash_t>::iterator mask_it(forward_masks.begin());
    	int j(0), i(0), width(0); // counters
    	
    	/* To more easily and reasonably-efficiently calculate the permutations 
    	 * of each of the hashes we insert, and since each block is just 
    	 * contiguous set bits, we will calculate the widths of each of these 
    	 * blocks, and the offset of their rightmost bit. With this, we'll 
    	 * generate net offsets between their positions in the unpermuted and 
    	 * permuted forms, and simultaneously generate reverse masks */
    	for(; mask_it != forward_masks.end(); ++mask_it) {
    		hash_t mask = *mask_it;
    		/* Find where the 1's start, and where they end. After this, `i` is
    		 * the position to the right of the rightmost set bit. `j` is the 
    		 * position of the leftmost set bit. In `width`, we keep a running
    		 * tab of the widths of the bitmasks so far. */
    		for (i = 0;          !((1UL << i) & mask); ++i) {}
    		for (j = i; j < 64 && ((1UL << j) & mask); ++j) {}
    		
    		/* Just to prove that I'm sane, and in case that I'm ever running
    		 * circles around this logic in the future, consider:
    		 *
    		 *     63---53|52---42|41---32|31---21|20---10|09---00|
    		 *     |  A   |   B   |   C   |   D   |   E   |   F   |
    		 *
    		 *                       permuted to
             *     63---53|52---42|41---32|31---21|20---10|09---00|
    		 *     |  C   |   D   |   E   |   A   |   B   |   F   |
    		 *
    		 * The first loop, we'll have width = 0, and examining the mask for
    		 * C, we'll find that i = 31 and j = 41, so we find that its width 
    		 * is 10. In the end, the bit in position 32 needs to move to be in
    		 * position 53. Width serves as an accumulator of the widths of the
    		 * blocks inserted into the permuted value, so we increment it by 
    		 * the width of C (j-i) = 10. Now the end offset is 63 - width = 53,
    		 * and the original offset for that bit was (i+1) = 32, and we find
    		 * that we need an offset of 63 - width - i - 1 = 62 - width - i:
    		 * 
    		 *    C: i = 31 | j = 41 | width = 0  | end = 53
    		 *       width += (j-i)          => 10
    		 *       offset = 62 - width - i => 21
    		 *
    		 *    D: i = 20 | j = 31 | width = 10 | end = 42
    		 *       width += (j-i)          => 21
    		 *       offset = 62 - width - i => 21 */
    	    width += (j - i);
    	    widths.push_back(j - i);
    	    
    	    int offset = 64 - width - i;
    		offsets.push_back(offset);
    		
    		/* It's a trivial transformation, but we'll pre-compute our reverse
    		 * masks so that we don't have to compute for after the every time
    		 * we unpermute a number */
            if (offset > 0) {
                reverse_masks.push_back(mask <<  offset);
            } else {
                reverse_masks.push_back(mask >> -offset);
            }
    	}
            	
    	/* Alright, we have to determine the low and high masks for this 
    	 * paritcular table. If we are searching for up to /d/ differing bits, 
    	 * then we should  include all but the last /d/ blocks in our mask.
    	 *
    	 * After this, width should hold the number of bits that are in all but 
    	 * the last d blocks */
    	std::vector<size_t>::iterator width_it(widths.begin());
    	for (width = 0; d < widths.size(); ++d, ++width_it) {
    		width += *width_it;
    	}
    	
    	/* Set the first /width/ bits in the low mask to 1, and then shift it up
    	 * until it's a full 64-bit number. */
    	for(i = 0    ; i < width; ++i) { search_mask = (search_mask << 1) | 1; }
    	for(i = width; i < 64   ; ++i) { search_mask =  search_mask << 1;      }
    };
    
    /* Hash permutation */
    hash_t Table::permute(hash_t hash) {
    	std::vector<hash_t>::iterator masks_it(forward_masks.begin());
    	std::vector<int   >::iterator offset_it(     offsets.begin());
    	
    	hash_t result(0);
    	for (; masks_it != forward_masks.end(); ++masks_it, ++offset_it) {
        	if (*offset_it > 0) {
            	result = result | ((hash & *masks_it) <<   *offset_it );
        	} else {
            	result = result | ((hash & *masks_it) >> -(*offset_it));
        	}
    	}
    	return result;
    }
    
    /* Hash unpermutation */
    hash_t Table::unpermute(hash_t hash) {
        std::vector<hash_t>::iterator masks_it(reverse_masks.begin());
    	std::vector<int   >::iterator offset_it(     offsets.begin());
    	    	
    	hash_t result(0);
    	for (; masks_it != reverse_masks.end(); ++masks_it, ++offset_it) {
            if (*offset_it > 0) {
        	    result = result | ((hash & *masks_it) >>   *offset_it );
        	} else {
        	    result = result | ((hash & *masks_it) << -(*offset_it));
        	}
    	}
    	return result;
    }
    
    /* Insertion */
    template <class InputIterator>
    void Table::insert(InputIterator first, InputIterator last) {
    	hash_t result(0);
    	for(; first != last; ++first) {
    		J1S(result, judy, permute(*first));
    	}
    }
    
    void Table::insert(hash_t hash) {
    	hash_t result(0);
    	J1S(result, judy, permute(hash));
    }
    
    /* Removal */
    template <class InputIterator>
    void Table::remove(InputIterator first, InputIterator last) {
    	hash_t result(0);
    	for(; first != last; ++first) {
    		J1U(result, judy, permute(*first));
    	}
    }
    
    void Table::remove(hash_t hash) {
    	hash_t result(0);
    	J1U(result, judy, permute(hash));
    }
    
    /* Find duplicates */
    hash_t Table::find(hash_t query) {
    	// We have to perform our permutation on the 
    	query = permute(query);
    	
    	hash_t low( query &  search_mask);
    	hash_t high(query | ~search_mask);
    	hash_t result(0);
    	J1F(result, judy, low);
    	
    	for(; result && (low <= high); ) {
    		if (num_differing_bits(low, query) <= differing_bits) {
    			return unpermute(low);
    		}
    		J1N(result, judy, low);
    	}
    	return 0;
    }
    
    void Table::find(hash_t query, std::vector<hash_t>& results) {
        query = permute(query);
        
        hash_t low( query &  search_mask);
        hash_t high(query | ~search_mask);
        hash_t result(0);
        J1F(result, judy, low);
        
        for(; result && (low <= high);) {
            if (num_differing_bits(low, query) <= differing_bits) {
                results.push_back(unpermute(low));
            }
            J1N(result, judy, low);
        }
    }
}
