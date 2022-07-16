/*
 * FragmentEntry.h
 *
 *  Created on: Jan 5, 2022
 * @author Andrey Belomutskiy, (c) 2012-2022
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

struct FragmentEntry {
	template <typename TData>
	FragmentEntry(const TData* data)
		: data(reinterpret_cast<const uint8_t*>(data))
		, size(sizeof(TData))
	{
	}
	FragmentEntry(const uint8_t* data, size_t size)
		: data(reinterpret_cast<const uint8_t*>(data))
		, size(size)
	{
	}

	const uint8_t* const data;
	const size_t size;
};

struct FragmentList {
	const FragmentEntry* fragments;
	const size_t count;
};

// copy `size` of fragmented outputs in to destination, skipping the first `skip` bytes
void copyRange(uint8_t* destination, FragmentList src, size_t skip, size_t size);
FragmentList getFragments();
