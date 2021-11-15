#pragma once

#include "platform.h"
#include "instruction_types.h"
#include "cpu_types.h"

// TODO: use constexpr variadic templates to build the encoding mask and bit fields
struct EncodingDescriptor
{
	InstructionType type{};
	const char* debug_format{};
	cpu_func_t func{};
	uint32_t mask{};
	uint32_t bits{};

	constexpr EncodingDescriptor() = default;
	constexpr EncodingDescriptor(InstructionType inst, std::initializer_list<std::pair<uint32_t, uint32_t>>&& descs, const char* debug_format_string, cpu_func_t _func) :
		type(inst),
		debug_format(debug_format_string),
		func(_func),
		mask(build_mask(descs)),
		bits(build_bits(descs))
	{
	}

	constexpr bool match(uint32_t op) const 
	{
		auto masked = op & mask;
		return masked == bits;
	}

	static constexpr uint32_t build_bits(const std::initializer_list<std::pair<uint32_t, uint32_t>>& descs)
	{
		uint32_t bits{};

		for (auto desc : descs)
			bits |= desc.first;

		return bits;
	}

	static constexpr uint32_t build_mask(const std::initializer_list<std::pair<uint32_t, uint32_t>>& descs)
	{
		uint32_t mask{};

		for (auto desc : descs)
			mask |= desc.second;

		return mask;
	}
};

const EncodingDescriptor* disassembler_decode_instruction(uint32_t opcode);

// dissasemble a single instruction into text form using symbolic register names
bool disassembler_parse_instruction(uint32_t opcode, const EncodingDescriptor* desc, char* dst_buf, int);