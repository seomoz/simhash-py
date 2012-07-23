#ifndef SIMHASH_HASHES_FNV_H
#define SIMHASH_HASHES_FNV_H

namespace Simhash {
	struct fnv {
		uint64_t operator()(const char* data, size_t len, uint64_t seed) {
			return 0;
		}
	};
}

#endif
