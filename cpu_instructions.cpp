
#include "cpu_types.h"
#include "memory.h"
#include "platform.h"
#include "disassembler.h"

#include <limits>
#include <vector>

#define INST(value) 				{ SET_INST_BITS(value), 	INST_ENCODING_BITMASK }
#define RS(value) 					{ SET_RS_BITS(value), 		RS_ENCODING_BITMASK }
#define RT(value) 					{ SET_RT_BITS(value), 		RT_ENCODING_BITMASK }
#define RD(value) 					{ SET_RD_BITS(value), 		RD_ENCODING_BITMASK }
#define SHIFT(value) 				{ SET_SHIFT_BITS(value),	SHIFT_ENCODING_BITMASK }
#define FUNC(value) 				{ SET_FUNC_BITS(value), 	FUNC_ENCODING_BITMASK }

#define ENCODING(...) 				{ __VA_ARGS__ }
#define FORMAT(...) 				__VA_ARGS__

#define R4300_IMPL(type, name, encoding, format_string, ...) \
static void cpu_##name(ExecutionContext& ctx); \
static const EncodingDescriptorRegistrar CONCAT(_, __COUNTER__){EncodingDescriptor(type, encoding, format_string, cpu_##name)};

// This should be called before main and will populate our encodings buffer
std::vector<EncodingDescriptor> encodings;
struct EncodingDescriptorRegistrar
{
	constexpr EncodingDescriptorRegistrar(EncodingDescriptor&& desc)
	{
		encodings.push_back(std::move(desc));
	}
};

// taken from project64
static const uint32_t SWL_MASK[4] = { 0x00000000, 0xFF000000, 0xFFFF0000, 0xFFFFFF00 };
static const uint32_t SWR_MASK[4] = { 0x00FFFFFF, 0x0000FFFF, 0x000000FF, 0x00000000 };
static const uint32_t LWL_MASK[4] = { 0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF };
static const uint32_t LWR_MASK[4] = { 0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0x0000000 };

static const int32_t  SWL_SHIFT[4] = { 0, 8, 16, 24 };
static const int32_t  SWR_SHIFT[4] = { 24, 16, 8, 0 };
static const int32_t  LWL_SHIFT[4] = { 0, 8, 16, 24 };
static const int32_t  LWR_SHIFT[4] = { 24, 16, 8, 0 };

R4300_IMPL(InstructionType::NOP, nop, 
	ENCODING(INST(0b000000), RS(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b000000)),
	"");
	void cpu_nop(ExecutionContext& ctx)	
	{
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADD, add,
	ENCODING(INST(0b000000), SHIFT(0b00000), FUNC(0b100000)),
	FORMAT("RD, RS, RT"));
	void cpu_add(ExecutionContext& ctx)
	{
		auto c 
				= scast<int32_t>(ctx.rs())
				+ scast<int32_t>(ctx.rt());

		ctx.rd() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDU, addu,
	ENCODING(INST(0b000000), SHIFT(0b00000), FUNC(0b100001)),
	FORMAT("RD, RS, RT"));
	void cpu_addu(ExecutionContext& ctx)
	{
		auto c 
				= scast<uint32_t>(ctx.rs())
				+ scast<uint32_t>(ctx.rt());

		ctx.rd() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDI, addi,
	ENCODING(INST(0b001000)),
	FORMAT("RT, RS, IMM"));
	void cpu_addi(ExecutionContext& ctx)
	{
		auto c
			= static_cast<int32_t>(ctx.rs()) 
			+ static_cast<int16_t>(ctx.imm());

		ctx.rt() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDIU, addiu,
	ENCODING(INST(0b001001)),
	FORMAT("RT, RS, IMM"));
	void cpu_addiu(ExecutionContext& ctx)
	{
		auto c
			= static_cast<uint32_t>(ctx.rs()) 
			+ static_cast<int16_t>(ctx.imm());

		ctx.rt() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SUBU, subu,
	ENCODING(INST(0b000000), SHIFT(0b00000), FUNC(0b100011)),
	FORMAT("RD, RS, RT"));
	void cpu_subu(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() - ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MULT, mult,
	ENCODING(INST(0b000000), RD(0b00000), SHIFT(0b00000), FUNC(0b011000)),
	FORMAT("RS, RT"));
	void cpu_mult(ExecutionContext& ctx)
	{
		cpu.hi_lo = (int64_t)ctx.rt() * (int64_t)ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MULTU, multu,
	ENCODING(INST(0b000000), RD(0b00000), SHIFT(0b00000), FUNC(0b011001)),
	FORMAT("RS, RT"));
	void cpu_multu(ExecutionContext& ctx)
	{
		cpu.hi_lo = ctx.rt() * ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::AND, and,
	ENCODING(INST(0b000000), SHIFT(0b00000), FUNC(0b100100)),
	FORMAT("RD, RS, RT"));
	void cpu_and(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() & ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ANDI, andi,
	ENCODING(INST(0b001100)),
	FORMAT("RT, RS, IMM"));
	void cpu_andi(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() & ctx.imm();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LUI, lui,
	ENCODING(INST(0b001111)),
	FORMAT("RT, IMM"));
	void cpu_lui(ExecutionContext& ctx)
	{
		int32_t imm = ctx.imm() << 16;
		ctx.rt() = imm;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFLO, mflo,
	ENCODING(INST(0b000000), RS(0b00000), RT(0b00000), SHIFT(0b00000), FUNC(0b010010)),
	FORMAT("RD"));
	void cpu_mflo(ExecutionContext& ctx)
	{
		ctx.rd() = cpu.lo;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFHI, mfhi,
	ENCODING(INST(0b000000), RS(0b00000), RT(0b00000), SHIFT(0b00000), FUNC(0b010000)),
	FORMAT("RD"));
	void cpu_mfhi(ExecutionContext& ctx)
	{
		ctx.rd() = cpu.hi;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::OR, or,
	ENCODING(INST(0b000000), SHIFT(0b00000), FUNC(0b100101)),
	FORMAT("RD, RS, RT"));
	void cpu_or(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() | ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ORI, ori,
	ENCODING(INST(0b001101)),
	FORMAT("RT, RS, IMM"));
	void cpu_ori(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() | scast<uint16_t>(ctx.imm());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::XOR, xor,
	ENCODING(INST(0b000000), SHIFT(0b000000), FUNC(0b100110)),
	FORMAT("RD, RS, RT"));
	void cpu_xor(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() ^ ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::XORI, xori,
	ENCODING(INST(0b001110)),
	FORMAT("RT, RS, IMM"));
	void cpu_xori(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() ^ scast<uint16_t>(ctx.imm());
		cpu.pc += 4;
	}

/**************************************************************************/
// Branch
/**************************************************************************/
R4300_IMPL(InstructionType::JR, jr,
	ENCODING(INST(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b001001)),
	FORMAT("RS"));

R4300_IMPL(InstructionType::JR, jr,
	ENCODING(INST(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b001000)),
	FORMAT("RS"));
	void cpu_jr(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		cpu.pc = ctx.rs();
	}
	
R4300_IMPL(InstructionType::J, j,
	ENCODING(INST(0b000010)),
	FORMAT("TARGET"));
	void cpu_j(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		cpu.pc = (cpu.pc & 0xF0000000) + (ctx.jmp() << 2);
	}

R4300_IMPL(InstructionType::JALR, jalr,
	ENCODING(INST(0b000000), RT(0b00000), SHIFT(0b00000), FUNC(0b001001)),
	FORMAT("RD, RS"));
	void cpu_jalr(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		ctx.rd() = cpu.pc + 4;
		cpu.pc = ctx.rs();
	}


R4300_IMPL(InstructionType::JAL, jal,
	ENCODING(INST(0b000011)),
	FORMAT("TARGET"));
	void cpu_jal(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;

		cpu_link(cpu.pc + 8);
		cpu.pc = (cpu.pc & 0xF0000000) + (ctx.jmp() << 2);
	}

R4300_IMPL(InstructionType::BEQ, beq,
	ENCODING(INST(0b000100)),
	FORMAT("RS, RT, OFFSET"));
	void cpu_beq(ExecutionContext& ctx)
	{
		if (ctx.rs() == ctx.rt())
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() << 2;
			cpu.pc += off + 4;
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BEQL, beql,
	ENCODING(INST(0b010100)),
	FORMAT("RS, RT, OFFSET"));
	void cpu_beql(ExecutionContext& ctx)
	{
		if (ctx.rs() == ctx.rt())
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() << 2;
			cpu.pc += off + 4;
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BNE, bne,
	ENCODING(INST(0b000101)),
	FORMAT("RS, RT, OFFSET"));
	void cpu_bne(ExecutionContext& ctx)
	{
		if (ctx.rs() != ctx.rt())
		{
			branch_delay_slot_address = cpu.pc + 4;
			
			int32_t off = ctx.offset() << 2;
			cpu.pc += off + 4;
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BNEL, bne,
	ENCODING(INST(0b010101)),
	FORMAT("RS, RT, OFFSET"));
	// same as above

R4300_IMPL(InstructionType::BLEZL, blezl,
	ENCODING(INST(0b010110), RT(0b000000)),
	FORMAT("RS, OFFSET"));
	void cpu_blezl(ExecutionContext& ctx)
	{
		if (int32_t(ctx.rs()) <= 0)
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() << 2;
			cpu.pc += (off & 0x3FFFF) + 4;
		}
		else
		{
			cpu.pc += 8;
		}
	}	

R4300_IMPL(InstructionType::BLTZ, bltz,
	ENCODING(INST(0b000001), RT(0b000000)),
	FORMAT("RS, OFFSET"));
	void cpu_bltz(ExecutionContext& ctx)
	{
		if (int32_t(ctx.rs()) < 0)
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() << 2;
			cpu.pc += (off & 0x3FFFF) + 4;
		}
		else
		{
			cpu.pc += 8;
		}
	}
/**************************************************************************/

/**************************************************************************/
// Load/Store
/**************************************************************************/

R4300_IMPL(InstructionType::LW, lw,
	ENCODING(INST(0b100011)),
	FORMAT("RT, OFFSET(RS)"));
	void cpu_lw(ExecutionContext& ctx)
	{
		uint32_t address = ctx.rs() + (int16_t)ctx.offset();

		if ((address & 3) != 0)
			throw MemException{"lw poop", address, 4 };

		uint32_t v{};
		if (!memory_read32(address, v))
			throw MemException{"bad mem read", address, 4 };

		ctx.rt() = v;

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LWU, lwu,
	ENCODING(INST(0b100111)),
	FORMAT("RT, OFFSET(RS)"));
	void cpu_lwu(ExecutionContext& ctx)
	{
		uint32_t v{};

		auto address = ctx.rs() + (int16_t)ctx.offset();
		if (!memory_read32(address, v))
			throw MemException{"bad mem read", uint32_t(address), 4 };
		
		ctx.rt() = v;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LWR, lwr,
	ENCODING(INST(0b100110)),
	FORMAT("RT, OFFSET(RS)"));
	void cpu_lwr(ExecutionContext& ctx)
	{
		auto addr = ctx.base() + int16_t(ctx.offset());

		// top 2 bits are shift mode
		auto off = addr & 3;

		// clear top 3 bits and get value in mem
		addr &= ~3;
		uint32_t v{};
		
		if (!memory_read32(addr, v))
			throw MemException{"bad mem read", uint32_t(addr), 4 };
	
		auto rt = (int32_t)ctx.rt() & LWR_MASK[off];
		ctx.rt() = (int32_t)(rt | (v >> LWR_SHIFT[off]));

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SB, sb,
	ENCODING(INST(0b101000)),
	FORMAT("RT, OFFSET(RS)"));
	void cpu_sb(ExecutionContext& ctx)
	{
		auto addr = ctx.rs() + ctx.offset();
		if (!memory_write8(addr, ctx.rt()))
			throw MemException{"bad mem write", uint32_t(addr), 1 };

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SW, sw,
	ENCODING(INST(0b101011)),
	FORMAT("RT, OFFSET(RS)"));
	void cpu_sw(ExecutionContext& ctx)
	{
		auto addr = ctx.rs() + ctx.offset();
		if (!memory_write32(addr, ctx.rt()))
			throw MemException{"bad mem write", uint32_t(addr), 4};

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFC0, mfc0,
	ENCODING(INST(0b010000), RS(0b00000), SHIFT(0b00000), FUNC(0b000000)),
	FORMAT("RT, COP_RD"));
	void cpu_mfc0(ExecutionContext& ctx)
	{
		ctx.rt() = cpu.cop0[ctx.rd_bits()];
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MTC0, mtc0,
	ENCODING(INST(0b010000), RS(0b00100), SHIFT(0b00000), FUNC(0b000000)),
	FORMAT("RT, COP_RD"));
	void cpu_mtc0(ExecutionContext& ctx)
	{
		switch (ctx.rd_bits())
		{
			case 6:		cpu.cop0[ctx.rd_bits()] = ctx.rt();	break; // wired
			case 9:		cpu.cop0[ctx.rd_bits()] = ctx.rt();	break; // count
			case 13: 	cpu.cop0[ctx.rd_bits()] = ctx.rt(); break; // cause 
			case 11: 	cpu.cop0[ctx.rd_bits()] = ctx.rt(); break; // compare 		
			
			default:
				printf("MTC0: poop\n");
				throw nullptr;
				break;
		}

		cpu.pc += 4;
	}
/**************************************************************************/

R4300_IMPL(InstructionType::SLL, sll,
	ENCODING(INST(0b000000), FUNC(0b000000)),
	FORMAT("RD, RT, SA"));
	void cpu_sll(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() << ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLLV, sllv,
	ENCODING(INST(0b000000), SHIFT(0b000000), FUNC(0b000100)),
	FORMAT("RD, RT, RS"));
	void cpu_sllv(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() << ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRA, sra,
	ENCODING(INST(0b000000), RS(0b000000), FUNC(0b000011)),
	FORMAT("RD, RT, SA"));
	void cpu_sra(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRAV, srav,
	ENCODING(INST(0b000000), SHIFT(0b000000), FUNC(0b000111)),
	FORMAT("RD, RT, RS"));
	void cpu_srav(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRL, srl,
	ENCODING(INST(0b000000), FUNC(0b000010)),
	FORMAT("RD, RT, SA"));
	void cpu_srl(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> (ctx.shift_bits() & 0x1F);
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRLV, srlv,
	ENCODING(INST(0b000000), SHIFT(0b000000), FUNC(0b000110)),
	FORMAT("RD, RT, RS"));
	void cpu_srlv(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> (ctx.rs() & 0x1F);
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLTI, slti,
	ENCODING(INST(0b001010)),
	FORMAT("RT, RS, IMM"));
	void cpu_slti(ExecutionContext& ctx)
	{
		ctx.rt() = int64_t(ctx.rs()) < int16_t(ctx.imm()) ? 1 : 0;
		cpu.pc += 4;
	}