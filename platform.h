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

#define CONCAT_IMPL(x, y) 		x##y
#define CONCAT(x, y) 			CONCAT_IMPL(x, y)

template<typename T>
constexpr bool get_bit(T value, int index)
{
	const auto size = (sizeof(T) * 8) - 1;
	return (value >> (size - index)) & 1;
}

template<typename T>
constexpr void set_bit(T& value, int index, bool bit)
{
	const auto size = (sizeof(T) * 8) - 1;
	if (bit)
	{
		value |= 1 << (size - index);
	}
	else
	{
		value &= ~(1 << (size - index));
	}
}

template<typename T>
constexpr T extract_bits(T value, int index, int num_bits)
{
	T out{};

	int dst_index{};
	for (int i = index; i < index + num_bits; i++)
	{
		auto b = get_bit(value, i);
		set_bit(out, num_bits - dst_index, b);
		
		dst_index++;
	}

	return out;
}

template<typename T, typename TT>
constexpr bool any(const T& t, TT tt)
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