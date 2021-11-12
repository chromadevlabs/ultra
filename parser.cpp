
#include "instruction_types.h"

#include "cpu_types.h"
#include "magic_enum.hpp"

#include <initializer_list>
#include <utility>
#include <cstdio>
#include <vector>


/*static const std::pair<InstructionType, std::vector<const char*>> print_encodings[] =
{
	{ InstructionType::ADD, 	{ "RD", "RS", "RT" } },
	{ InstructionType::DADDI,	{ "RS", "RT", "IMM" } },
	{ InstructionType::NOR, 	{ "RD", "RS", "RT" } },
	{ InstructionType::SLT, 	{ "RD", "RS", "RT" } },
	{ InstructionType::SLTI, 	{ "RT", "RS", "IMM" } },
	{ InstructionType::SLTIU, 	{ "RT", "RS", "IMM" } },
	{ InstructionType::SLTU, 	{ "RD", "RS", "RT" } },
	{ InstructionType::SUB, 	{ "RD", "RS", "RT" } },
	{ InstructionType::SUBU, 	{ "RD", "RS", "RT" } },
	{ InstructionType::XORI, 	{ "RT", "RS", "IMM" } },


	{ InstructionType::BEQL, 	{ "RS",	"RT", "OFFSET" } },
	
	{ InstructionType::BGEZ, 	{ "RS",	"OFFSET" } },
	{ InstructionType::BGEZAL, 	{ "RS",	"OFFSET" } },
	{ InstructionType::BGTZ, 	{ "RS",	"OFFSET" } },
	{ InstructionType::BLEZ, 	{ "RS",	"OFFSET" } },
	{ InstructionType::BLTZAL, 	{ "RS",	"OFFSET" } },
	{ InstructionType::BNEL,	{ "RS", "RT", "OFFSET" } },

	{ InstructionType::LB, 		{ "RT", "OFFSET", "RS" } },
	{ InstructionType::LBU, 	{ "RT", "OFFSET", "RS" } },
	{ InstructionType::LH, 		{ "RT", "OFFSET", "RS" } },
	{ InstructionType::LHU, 	{ "RT", "OFFSET", "RS" } },
	{ InstructionType::SH, 		{ "RT", "OFFSET", "RS" } },
	{ InstructionType::LB, 		{ "RT", "OFFSET", "RS" } },
};*/

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
		case 0: return "r0";
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

/*InstructionType parser_parse_opcode(uint32_t opcode)
{
	

	static const EncodingDescriptor descs[] = 
	{
		

		{ InstructionType::DIV, 	{ INST(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b011010) } },
		{ InstructionType::DIVU, 	{ INST(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b011011) } },

		{ InstructionType::MFHI, 	{ INST(0b000000), RS(0b000000), RT(0b000000), SHIFT(0b000000), FUNC(0b010000) } },
		{ InstructionType::MFLO, 	{ INST(0b000000), RS(0b000000), RT(0b000000), SHIFT(0b000000), FUNC(0b010010) } },

		{ InstructionType::MTHI, 	{ INST(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b010001) } },
		{ InstructionType::MTLO, 	{ INST(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b010011) } },

		{ InstructionType::MULT, 	{ INST(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b011000) } },
		{ InstructionType::MULTU, 	{ INST(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b011001) } },

		{ InstructionType::MFC0, 	{ INST(0b010000), RS(0b000000), SHIFT(0b000000), FUNC(0b000000) } },
		{ InstructionType::MTC0, 	{ INST(0b010000), RS(0b00100), SHIFT(0b000000), FUNC(0b000000) } },

		{ InstructionType::SYSCALL, { INST(0b000000), RS(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b001100) } },

		{ InstructionType::ADD,		{ INST(0b000000), SHIFT(0b00000), FUNC(0b100000) } },
		{ InstructionType::NOR, 	{ INST(0b000000), SHIFT(0b00000), FUNC(0b100111) } },
		{ InstructionType::SLT,		{ INST(0b000000), SHIFT(0b000000), FUNC(0b101010) } },
		{ InstructionType::SLTU,	{ INST(0b000000), SHIFT(0b000000), FUNC(0b101011) } },
		{ InstructionType::SUB, 	{ INST(0b000000), SHIFT(0b000000), FUNC(0b100010) }},
		{ InstructionType::SUBU, 	{ INST(0b000000), SHIFT(0b000000), FUNC(0b100011) }},


		{ InstructionType::Break, 	{ INST(0b000000), FUNC(0b001101) } },
		{ InstructionType::BGEZ, 	{ INST(0b000001), RT(0b00001) } },
		{ InstructionType::BGEZAL, 	{ INST(0b000001), RT(0b10001) } },
		{ InstructionType::BGTZ, 	{ INST(0b000111), RT(0b000000) } },
		{ InstructionType::BLEZ, 	{ INST(0b000110), RT(0b000000) } },
		{ InstructionType::BLTZ, 	 } },
		{ InstructionType::BLTZAL, 	{ INST(0b000001), RT(0b1000) } },
		
		{ InstructionType::DADDI, 	{ INST(0b011000) } },
		{ InstructionType::SLTI,	{ INST(0b001010) } },
		{ InstructionType::SLTIU,	{ INST(0b001011) } },

		{ InstructionType::BEQL, 	{ INST(0b010100) } },
		{ InstructionType::BNEL, 	{ INST(0b010101) } },

		{ InstructionType::LB, 		{ INST(0b100000) } },
		{ InstructionType::LBU, 	{ INST(0b100100) } },
		{ InstructionType::LH, 		{ INST(0b100001) } },
		{ InstructionType::LHU, 	{ INST(0b100101) } },
		
		{ InstructionType::SH, 		{ INST(0b101001) } },
		{ InstructionType::XORI, 	{ INST(0b001110) } }
	};

	for (const auto& desc : descs)
	{
		if (desc.match(opcode))
			return desc.type;
	}
	
	return InstructionType::UnknownInstruction;
}*/