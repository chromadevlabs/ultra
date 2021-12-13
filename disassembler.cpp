
#include "disassembler.h"
#include "cpu_types.h"
#include "magic_enum.hpp"

#include <vector>
#include <cstring>

const char* parser_get_symbolic_cop0_name(int i);
const char* parser_get_symbolic_gpr_name(int i);

// implemented in cpu_instructions.cpp
extern std::vector<EncodingDescriptor> encodings;

// string compare that is limited to the length of the second string
constexpr bool str_find(const char* s1, const char* s2)
{
	while (*s2)
	{
		if (!s1)
			return false;

		if (*s1 != *s2)
			return false;

		s1++;
		s2++;
	}

	return true;
};

void disassembler_init()
{
	/*auto err = cs_open(CS_ARCH_MIPS, CS_MODE_MIPS32, &cs);

	if (err != CS_ERR_OK)
	{
		printf("%s\n", cs_strerror(err));
	}*/
}

/*const EncodingDescriptor* disassembler_decode_instruction(uint32_t opcode)
{
	cs_insn* decoded{};

	auto addr = cpu.pc;
	if (auto count = cs_disasm(cs, (const uint8_t*)&opcode, sizeof(uint32_t), addr, 1, &decoded))
	{
		auto d = decoded[0];
		char m[32]{};
		strcpy(m, d.mnemonic);

		std::transform(m + 0, m + strlen(m), m, toupper);

		for (const auto& desc : encodings)
		{
			if (strcmp(m, magic_enum::enum_name(desc.type).data()) == 0)
			{
				return &desc;
			}
		}

		printf("Unknown opcode '%s'\n", m);
	}

	return nullptr;
}*/

const EncodingDescriptor* disassembler_find_descriptor(InstructionType type)
{
	for (const auto& encoding : encodings)
	{
		if (encoding.type == type)
			return &encoding;
	}

	return nullptr;
}

const EncodingDescriptor* disassembler_decode_instruction(uint32_t opcode)
{
	for (const auto& encoding : encodings)
	{
		if (encoding.match(opcode))
			return &encoding;
	}

	return nullptr;
}

bool disassembler_parse_instruction(uint32_t opcode, const EncodingDescriptor* desc, char* dst_buf, int)
{
	if (desc)
	{
		dst_buf += sprintf(dst_buf, "%-8s", magic_enum::enum_name(desc->type).data());

		auto* str = desc->debug_format;

		while (*str)
		{
			if (str_find(str, "RTI"))
			{
				dst_buf += sprintf(dst_buf, "%d", (uint8_t)GET_RT_BITS(opcode));
				str += 3;
			}
			else if (str_find(str, "COP_RD"))
			{
				dst_buf += sprintf(dst_buf, "%s", parser_get_symbolic_cop0_name(GET_RD_BITS(opcode)));		
				str += 6;
			}
			else if (str_find(str, "RS"))
			{
				dst_buf += sprintf(dst_buf, "$%s", parser_get_symbolic_gpr_name(GET_RS_BITS(opcode)));
				str += 2;
			}
			else if (str_find(str, "RT"))
			{
				dst_buf += sprintf(dst_buf, "$%s", parser_get_symbolic_gpr_name(GET_RT_BITS(opcode)));
				str += 2;
			}
			else if (str_find(str, "RD"))
			{
				dst_buf += sprintf(dst_buf, "$%s", parser_get_symbolic_gpr_name(GET_RD_BITS(opcode)));
				str += 2;
			}
			else if (str_find(str, "IMM"))
			{
				dst_buf += sprintf(dst_buf, "0x%04X", (uint16_t)GET_IMM_BITS(opcode));
				str += 3;
			}
			else if (str_find(str, "OFFSET"))
			{
				dst_buf += sprintf(dst_buf, "%d", (int16_t)GET_IMM_BITS(opcode));
				str += 6;
			}
			else if (str_find(str, "SA"))
			{
				dst_buf += sprintf(dst_buf, "%d", GET_SHIFT_BITS(opcode));
				str += 2;
			}
			else if (str_find(str, "TARGET"))
			{
				auto address = (cpu.pc & 0xF0000000) + (GET_JMP_BITS(opcode) << 2);
				dst_buf += sprintf(dst_buf, "0x%08X", (uint32_t)address);
				str += 6;
			}
			else
			{
				dst_buf += sprintf(dst_buf, "%c", *str);
				str++;
			}
		}

		*dst_buf = '\0';
		return true;
	}

	return false;
}

const char* parser_get_symbolic_cop0_name(int i)
{
	switch (i)
	{
		case 0: return "index";
		case 1: return "random";
		case 2: return "entry_lo0";
		case 3: return "entry_hi0";
		case 4: return "context";
		case 5: return "pagemask";
		case 6: return "wired";
		case 8: return "bad_vaddr";
		case 9: return "count";
		case 10: return "entry_hi";
		case 11: return "compare";
		case 12: return "status";
		case 13: return "cause";
		case 14: return "epc"; 		
		case 15: return "prid"; 	
		case 16: return "config"; 		
		case 17: return "ll_addr"; 	
		case 18: return "watch_lo"; 	
		case 19: return "watch_hi"; 	
		case 20: return "xcontext"; 	
		case 26: return "parity_error";
		case 27: return "cache_error";
		case 28: return "tag_lo";
		case 29: return "tag_hi";
		case 30: return "error_epc";
	}

	return nullptr;
}

const char* parser_get_symbolic_gpr_name(int i)
{
	switch (i)
	{
		case 0: return "0";
		case 1: return "at";
		
		case 2: return "v0";
		case 3: return "v1";
		
		case 4: return "a0";
		case 5: return "a1";
		case 6: return "a2";
		case 7: return "a3";

		case 8: return "t0";
		case 9: return "t1";
		case 10: return "t2";
		case 11: return "t3";
		case 12: return "t4";
		case 13: return "t5";
		case 14: return "t6";
		case 15: return "t7";
		case 24: return "t8";
		case 25: return "t9";

		case 16: return "s0";
		case 17: return "s1";
		case 18: return "s2";
		case 19: return "s3";
		case 20: return "s4";
		case 21: return "s5";
		case 22: return "s6";
		case 23: return "s7";

		case 26: return "k0";
		case 27: return "k1";

		case 28: return "gp";
		case 29: return "sp";
		case 30: return "fp";
		case 31: return "ra";
	}

	return nullptr;
}
