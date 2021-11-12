#pragma once

#include <cstdint>
#include <cstdio>
#include <cassert>

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define bswap_16(x) 			OSSwapInt16(x)
#define bswap_32(x) 			OSSwapInt32(x)
#define bswap_64(x) 			OSSwapInt64(x)

#define KB(value)				(value * 1024)
#define MB(value)				(KB(value) * 1024)

#define scast					static_cast
#define rcast					reinterpret_cast
#define dcast					dynamic_cast

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

template<typename T>
constexpr bool get_bit_left_aligned(T value, int index)
{
	const auto size = (sizeof(T) * 8) - 1;
	return (value >> (size - index)) & 1;
}

template<typename T>
constexpr bool get_bit_right_aligned(T value, int index)
{
	return (value << index) & 1;
}

template<typename T>
constexpr void set_bit_left_aligned(T& value, int index, bool state)
{
	const auto size = (sizeof(T) * 8) - 1;
	if (state)
	{
		value |= 1 << (size - index);
	}
	else
	{
		value &= ~(1 << (size - index));
	}
}

template<typename T>
constexpr void set_bit_right_aligned(T& value, int index, bool state)
{
	if (state)
	{
		value |= 1 << (index);
	}
	else
	{
		value &= ~(1 << (index));
	}
}

template<typename T>
constexpr T extract_bits_left_aligned(T value, int begin, int end)
{
    T mask = (1 << (end - begin)) - 1;
    return (value >> begin) & mask;
}

template<typename T, typename TT>
constexpr bool any(T t, TT tt)
{
	for (const auto& _tt : tt)
		if (_tt == t)
			return true;

	return false;
}

constexpr size_t string_hash(const char* string)
{
	size_t hash{};

	while (string && *string)
		hash = hash * 101 ^ *string++;

	return hash;
}