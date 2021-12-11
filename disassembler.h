#pragma once

#include "platform.h"
#include "instruction_types.h"
#include "cpu_types.h"

struct EncodingDescriptor
{
	InstructionType type{};
	const char* debug_format{};
	cpu_func_t func;

	uint32_t mask;
	uint32_t value;

	constexpr EncodingDescriptor() = default;
	constexpr EncodingDescriptor(InstructionType inst, const char* descriptor, const char* debug_format_string, cpu_func_t _func) :
		type(inst),
		debug_format(debug_format_string),
		func(_func),
		mask(gen_mask(descriptor)),
		value(gen_value(descriptor))
	{
		assert(strlen(descriptor) == 32);
	}

	constexpr bool match(uint32_t op) const 
	{
		return (op & mask) == value;
	}

	constexpr uint32_t gen_mask(const char* str)
	{
		uint32_t mask{};
		uint32_t bit_index{31};

		while (*str)
		{
			switch (*str)
			{
				case '0':
				case '1':
					mask |= 1 << bit_index; 
					break;
				
				case 's':
				case 't':
				case 'd':
				case 'j':
				case 'i':
				case 'a':
					break;

				default: 
					printf("unknown opcode bit type '%c'\n", *str);
					assert(false);
					break;
			}

			str++;
			bit_index--;
		}

		return mask;
	}

	constexpr uint32_t gen_value(const char* str)
	{
		uint32_t value{};
		uint32_t bit_index{31};

		while (*str)
		{
			switch (*str)
			{
				case '0':
					break;
				
				case '1': 
					value |= 1 << bit_index;
					break;
					
				default: 
					break;
			}

			str++;
			bit_index--;
		}

		return value;
	}
};

void disassembler_init();

const EncodingDescriptor* disassembler_decode_instruction(uint32_t opcode);

// dissasemble a single instruction into text form using symbolic register names
bool disassembler_parse_instruction(uint32_t opcode, const EncodingDescriptor* desc, char* dst_buf, int);