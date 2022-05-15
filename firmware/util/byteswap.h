#pragma once

// http://en.wikipedia.org/wiki/Endianness

static inline uint16_t SWAP_UINT16(uint16_t x)
{
	return ((x << 8) | (x >> 8));
}

static inline uint32_t SWAP_UINT32(uint32_t x)
{
	return (((x >> 24) & 0x000000ff) | ((x <<  8) & 0x00ff0000) |
			((x >>  8) & 0x0000ff00) | ((x << 24) & 0xff000000));
}