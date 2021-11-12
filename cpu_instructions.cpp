
#include "cpu_types.h"
#include "memory.h"
#include "platform.h"

#include <limits>

#define INST(value) 				{ SET_INST_BITS(value), 	INST_ENCODING_BITMASK }
#define RS(value) 					{ SET_RS_BITS(value), 		RS_ENCODING_BITMASK }
#define RT(value) 					{ SET_RT_BITS(value), 		RT_ENCODING_BITMASK }
#define RD(value) 					{ SET_RD_BITS(value), 		RD_ENCODING_BITMASK }
#define SHIFT(value) 				{ SET_SHIFT_BITS(value),	SHIFT_ENCODING_BITMASK }
#define FUNC(value) 				{ SET_FUNC_BITS(value), 	FUNC_ENCODING_BITMASK }

#define OPCODE(...) { __VA_ARGS__ }
#define FORMAT(...) __VA_ARGS__

#define R4300_IMPL(type, name, encoding, format_string, ...) \
static void cpu_##name(ExecutionContext& ctx); \
static const EncodingDescriptorRegistrar CONCAT(_, __COUNTER__){EncodingDescriptor(type, encoding, format_string, cpu_##name)};

void cpu_add_encoding(EncodingDescriptor&&);
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
	OPCODE(INST(0b000000), RS(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b000000)),
	"");
	void cpu_nop(ExecutionContext& ctx)	
	{
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADD, add,
	OPCODE(INST(0b000000), SHIFT(0b00000), FUNC(0b100000)),
	"RT, RS, IMM");
	void cpu_add(ExecutionContext& ctx)
	{
		auto c 
				= scast<int32_t>(ctx.rs())
				+ scast<int32_t>(ctx.rt());

		ctx.rd() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDU, addu,
	OPCODE(INST(0b000000), SHIFT(0b00000), FUNC(0b100001)),
	"RD, RS, RT");
	void cpu_addu(ExecutionContext& ctx)
	{
		auto c 
				= scast<uint32_t>(ctx.rs())
				+ scast<uint32_t>(ctx.rt());

		ctx.rd() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDI, addi,
	OPCODE(INST(0b001000)),
	"RT, RS, IMM");
	void cpu_addi(ExecutionContext& ctx)
	{
		auto c
			= static_cast<int32_t>(ctx.rs()) 
			+ static_cast<int16_t>(ctx.imm());

		ctx.rt() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDIU, addiu,
	OPCODE(INST(0b001001)),
	"RT, RS, IMM");
	void cpu_addiu(ExecutionContext& ctx)
	{
		auto c
			= static_cast<int32_t>(ctx.rs()) 
			+ static_cast<int16_t>(ctx.imm());

		ctx.rt() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::AND, and,
	OPCODE(INST(0b000000), SHIFT(0b00000), FUNC(0b100100)),
	"RD, RS, RT");
	void cpu_and(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() & ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ANDI, andi,
	OPCODE(INST(0b001100)),
	"RT, RS, IMM");
	void cpu_andi(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() & ctx.imm();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LUI, lui,
	OPCODE(INST(0b001111)),
	"RT, IMM");
	void cpu_lui(ExecutionContext& ctx)
	{
		int32_t imm = ctx.imm() << 16;
		ctx.rt() = imm;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::OR, or,
	OPCODE(INST(0b000000), SHIFT(0b00000), FUNC(0b100101)),
	"RD, RS, RT");
	void cpu_or(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() | ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ORI, ori,
	OPCODE(INST(0b001101)),
	"RT, RS, IMM");
	void cpu_ori(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() | scast<uint16_t>(ctx.imm());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::XOR, xor,
	OPCODE(INST(0b000000), SHIFT(0b000000), FUNC(0b100110)),
	"RD, RS, RT");
	void cpu_xor(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() ^ ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::XORI, xori,
	OPCODE(INST(0b001110)),
	"RT, RS, IMM");
	void cpu_xori(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() ^ scast<uint16_t>(ctx.imm());
		cpu.pc += 4;
	}

/**************************************************************************/
// Branch
/**************************************************************************/
R4300_IMPL(InstructionType::J, j,
	OPCODE(INST(0b000010)),
	"TARGET");
	void cpu_j(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		cpu.pc = (cpu.pc & 0xF0000000) + (ctx.jmp() << 2);
	}

R4300_IMPL(InstructionType::JAL, jal,
	OPCODE(INST(0b000011)),
	"TARGET");
	void cpu_jal(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;

		cpu_link(cpu.pc + 8);
		cpu.pc = (cpu.pc & 0xF0000000) + (ctx.jmp() << 2);
	}

R4300_IMPL(InstructionType::JALR, jalr,
	OPCODE(INST(0b000000), RT(0b00000), SHIFT(0b00000), FUNC(0b001001)),
	"RD, RS");
	void cpu_jalr(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		ctx.rd() = cpu.pc + 4;
		cpu.pc = ctx.rs();
	}

R4300_IMPL(InstructionType::JR, jr,
	OPCODE(INST(0b000000), RT(0b000000), RD(0b000000), SHIFT(0b000000), FUNC(0b001000)),
	"RS");
	void cpu_jr(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		cpu.pc = ctx.rs();
	}

R4300_IMPL(InstructionType::BEQ, beq,
	OPCODE(INST(0b000100)),
	"RS, RT, OFFSET");
	void cpu_beq(ExecutionContext& ctx)
	{
		if (ctx.rs() == ctx.rt())
		{
			debug_log("[taken]");
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
	OPCODE(INST(0b010100)),
	"RS, RT, OFFSET");
	void cpu_beql(ExecutionContext& ctx)
	{
		if (ctx.rs() == ctx.rt())
		{
			debug_log("[taken]");
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
	OPCODE(INST(0b000101)),
	"RS, RT, OFFSET");
	void cpu_bne(ExecutionContext& ctx)
	{
		if (ctx.rs() != ctx.rt())
		{
			debug_log("[taken]");
			branch_delay_slot_address = cpu.pc + 4;
			
			int32_t off = ctx.offset() << 2;
			cpu.pc += off + 4;
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BLTZ, bltz,
	OPCODE(INST(0b000001), RT(0b000000)),
	"RS, RT, OFFSET");
	void cpu_bltz(ExecutionContext& ctx)
	{
		if (int32_t(ctx.rs()) < 0)
		{
			debug_log("[taken]");
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
	OPCODE(INST(0b100011)),
	"RT, OFFSET(RS)");
	void cpu_lw(ExecutionContext& ctx)
	{
		uint32_t address = ctx.rs() + (int16_t)ctx.offset();

		if ((address & 3) != 0)
		{
			printf("LW: address error\n");
			throw nullptr;
		}

		ctx.rt() = memory_read32(address);

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LWU, lwu,
	OPCODE(INST(0b100111)),
	"RT, OFFSET(RS)");
	void cpu_lwu(ExecutionContext& ctx)
	{
		ctx.rt() = (uint32_t)memory_read32(ctx.rs() + (int16_t)ctx.offset());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LWR, lwr,
	OPCODE(INST(0b100110)),
	"RT, OFFSET(RS)");
	void cpu_lwr(ExecutionContext& ctx)
	{
		auto addr = ctx.base() + int16_t(ctx.offset());

		// top 2 bits are shift mode
		auto off = addr & 3;

		// clear top 3 bits and get value in mem
		addr &= ~3;
		auto v = memory_read32(addr);
	
		auto rt = (int32_t)ctx.rt() & LWR_MASK[off];
		ctx.rt() = (int32_t)(rt | (v >> LWR_SHIFT[off]));

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SB, sb,
	OPCODE(INST(0b101000)),
	"RT, OFFSET(RS)");
	void cpu_sb(ExecutionContext& ctx)
	{
		memory_write8(ctx.rs() + ctx.offset(), ctx.rt());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SW, sw,
	OPCODE(INST(0b101011)),
	"RT, OFFSET(RS)");
	void cpu_sw(ExecutionContext& ctx)
	{
		memory_write32(ctx.rs() + ctx.offset(), ctx.rt());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFC0, mfc0,
	OPCODE(INST(0b010000), RS(0b00000), SHIFT(0b00000), FUNC(0b000000)),
	"RT, COP_RD");
	void cpu_mfc0(ExecutionContext& ctx)
	{
		ctx.rt() = cpu.cop0[ctx.rd_bits()];
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MTC0, mtc0,
	OPCODE(INST(0b010000), RS(0b00100), SHIFT(0b00000), FUNC(0b000000)),
	"RT, COP_RD");
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
	OPCODE(INST(0b000000), FUNC(0b000000)),
	"RD, RT, SA");
	void cpu_sll(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() << ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLLV, sllv,
	OPCODE(INST(0b000000), SHIFT(0b000000), FUNC(0b000100)),
	"RD, RT, RS");
	void cpu_sllv(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() << ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRA, sra,
	OPCODE(INST(0b000000), RS(0b000000), FUNC(0b000011)),
	"RD, RT, SA");
	void cpu_sra(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRAV, srav,
	OPCODE(INST(0b000000), SHIFT(0b000000), FUNC(0b000111)),
	"RD, RT, RS");
	void cpu_srav(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRL, srl,
	OPCODE(INST(0b000000), FUNC(0b000010)),
	"RD, RT, SA");
	void cpu_srl(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> (ctx.shift_bits() & 0x1F);
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRLV, srlv,
	OPCODE(INST(0b000000), SHIFT(0b000000), FUNC(0b000110)),
	"RD, RT, RS");
	void cpu_srlv(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> (ctx.rs() & 0x1F);
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLTI, slti,
	OPCODE(INST(0b001010)),
	"RT, RS, IMM");
	void cpu_slti(ExecutionContext& ctx)
	{
		ctx.rt() = int64_t(ctx.rs()) < int16_t(ctx.imm()) ? 1 : 0;
		cpu.pc += 4;
	}