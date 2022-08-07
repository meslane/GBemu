#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdint>

uint8_t getBit(uint64_t n, uint8_t i);

uint64_t setBit(uint64_t n, uint8_t i, uint8_t state);

#endif