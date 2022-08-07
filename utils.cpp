#include "utils.h"

uint8_t getBit(uint64_t n, uint8_t i) {
	return static_cast<uint8_t>((n >> i) & 0x1);
}

uint64_t setBit(uint64_t n, uint8_t i, uint8_t state) {
	if (state == 0) {
		n &= ~(1ULL << i);
	}
	else if (state == 1) {
		n |= (1ULL << i);
	}

	return n;
}