
#include "cpu_types.h"
#include "memory.h"
#include "platform.h"
#include "disassembler.h"

#include <limits>
#include <vector>

#define RS 			"sssss"
#define OP			RS
#define RT 			"ttttt"
#define BASE		RT
#define RD 			"ddddd"
#define SHIFT 		"aaaaa"
#define IMM16		"iiiiiiiiiiiiiiii"
#define TARGET		"jjjjjjjjjjjjjjjjjjjjjjjjjj"

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
static const uint32_t LWR_MASK[4] = { 0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0x00000000 };

static const int32_t  SWL_SHIFT[4] = { 0, 8, 16, 24 };
static const int32_t  SWR_SHIFT[4] = { 24, 16, 8, 0 };
static const int32_t  LWL_SHIFT[4] = { 0, 8, 16, 24 };
static const int32_t  LWR_SHIFT[4] = { 24, 16, 8, 0 };

void cpu_get_cop0_register(int index, uint64_t& value);
void cpu_set_cop0_register(int index, uint64_t value);

extern bool logging_enabled;

R4300_IMPL(InstructionType::NOP, nop,
	"00000000000000000000000000000000",
	"");
	void cpu_nop(ExecutionContext& ctx)	
	{
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MOVE, add,
	"000000" RS "00000" RD "00000" "100000",
	"RD, RS");
R4300_IMPL(InstructionType::ADD, add,
	"000000" RS RT RD "00000" "100000",
	"RD, RS, RT");
	void cpu_add(ExecutionContext& ctx)
	{
		auto c 
				= scast<int32_t>(ctx.rs())
				+ scast<int32_t>(ctx.rt());

		ctx.rd() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ADDU, addu,
	"000000" RS RT RD "00000" "100001",
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
	"001000" RS RT IMM16,
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
	"001001" RS RT IMM16,
	"RT, RS, IMM");
	void cpu_addiu(ExecutionContext& ctx)
	{
		auto c
			= static_cast<uint32_t>(ctx.rs()) 
			+ static_cast<int16_t>(ctx.imm());

		ctx.rt() = c;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SUB, sub,
	"000000" RS RT RD "00000" "100010",
	"RD, RS, RT");
	void cpu_sub(ExecutionContext& ctx)
	{
		ctx.rd() = (int32_t)ctx.rs() - (int32_t)ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SUBU, subu,
	"000000" RS RT RD "00000" "100011",
	"RD, RS, RT");
	void cpu_subu(ExecutionContext& ctx)
	{
		ctx.rd() = (uint32_t)ctx.rs() - (uint32_t)ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MULT, mult,
	"000000" RS RT "0000000000" "011000",
	"RS, RT");
	void cpu_mult(ExecutionContext& ctx)
	{
		cpu.hi_lo = (int64_t)ctx.rt() * (int64_t)ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MULTU, multu,
	"000000" RS RT "0000000000" "011001",
	"RS, RT");
	void cpu_multu(ExecutionContext& ctx)
	{
		cpu.hi_lo = ctx.rt() * ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::DIV, div,
	"000000" RS RT "0000000000" "011010",
	"RS, RT");
	void cpu_div(ExecutionContext& ctx)
	{
		cpu.hi = (int32_t)ctx.rs() % (int32_t)ctx.rt();
		cpu.lo = (int32_t)ctx.rs() / (int32_t)ctx.rt();

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::DIVU, divu,
	"000000" RS RT "0000000000" "011011",
	"RS, RT");
	void cpu_divu(ExecutionContext& ctx)
	{
		cpu.hi = (uint32_t)ctx.rs() % (uint32_t)ctx.rt();
		cpu.lo = (uint32_t)ctx.rs() / (uint32_t)ctx.rt();

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::AND, and,
	"000000" RS RT RD "00000" "100100",
	"RD, RS, RT");
	void cpu_and(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() & ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ANDI, andi,
	"001100" RS RT IMM16,
	"RT, RS, IMM");
	void cpu_andi(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() & ctx.imm();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LUI, lui,
	"001111" "00000" RT IMM16,
	"RT, IMM");
	void cpu_lui(ExecutionContext& ctx)
	{
		uint32_t imm = ctx.imm() << 16;
		ctx.rt() = imm;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::NOR, nor,
	"000000" RS RT RD "00000" "100111",
	"RD, RS, RT");
	void cpu_nor(ExecutionContext& ctx)
	{
		ctx.rd() = ~(ctx.rs() | ctx.rt());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFLO, mflo,
	"000000" "0000000000" RD "00000" "010010",
	"RD");
	void cpu_mflo(ExecutionContext& ctx)
	{
		ctx.rd() = cpu.lo;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFHI, mfhi,
	"000000" "0000000000" RD "00000" "010000",
	"RD");
	void cpu_mfhi(ExecutionContext& ctx)
	{
		ctx.rd() = cpu.hi;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MTHI, mthi,
	"000000" RS "000000000000000" "010001",
	"RS");
	void cpu_mthi(ExecutionContext& ctx)
	{
		cpu.hi = ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MTLO, mtlo,
	"000000" RS "000000000000000" "010011",
	"RS");
	void cpu_mtlo(ExecutionContext& ctx)
	{
		cpu.lo = ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::OR, or,
	"000000" RS RT RD "00000" "100101",
	"RD, RS, RT");
	void cpu_or(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() | ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::ORI, ori,
	"001101" RS RT IMM16,
	"RT, RS, IMM");
	void cpu_ori(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() | scast<uint16_t>(ctx.imm());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::XOR, xor,
	"000000" RS RT RD "00000" "100110",
	"RD, RS, RT");
	void cpu_xor(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() ^ ctx.rt();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::XORI, xori,
	"001110" RS RT IMM16,
	"RT, RS, IMM");
	void cpu_xori(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() ^ scast<uint16_t>(ctx.imm());
		cpu.pc += 4;
	}

/**************************************************************************/
// Branch
/**************************************************************************/
R4300_IMPL(InstructionType::JR, jr,
	"000000" RS "0000000000" "00000" "001000" ,
	"RS");
	void cpu_jr(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		cpu.pc = ctx.rs();

		if (logging_enabled)
			printf("\tBranch taken: 0x%016llX\n", cpu.pc);
	}

R4300_IMPL(InstructionType::J, j,
	"000010" TARGET,
	"TARGET");
	void cpu_j(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;

		cpu.pc = (cpu.pc & 0xF0000000) + (ctx.jmp() * 4) + 4;
		
		if (logging_enabled)
			printf("\tBranch taken: 0x%016llX\n", cpu.pc);
	}

R4300_IMPL(InstructionType::JALR, jalr,
	"000000" RS "00000" RD "00000" "001001",
	"RD, RS");
	void cpu_jalr(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;
		ctx.rd() = cpu.pc + 4;
		cpu.pc = ctx.rs();

		if (logging_enabled)
			printf("\tBranch taken: 0x%016llX\n", cpu.pc);
	}


R4300_IMPL(InstructionType::JAL, jal,
	"000011" TARGET,
	"TARGET");
	void cpu_jal(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;

		cpu_link(cpu.pc + 8);
		cpu.pc = (cpu.pc & 0xF0000000) + (ctx.jmp() << 2);

		if (logging_enabled)
			printf("\tBranch taken: 0x%016llX\n", cpu.pc);
	}

R4300_IMPL(InstructionType::BAL, bal,
	"000001" "00000" "10001" IMM16,
	"OFFSET");
	void cpu_bal(ExecutionContext& ctx)
	{
		branch_delay_slot_address = cpu.pc + 4;

		cpu_link(cpu.pc + 8);
		int32_t off = ctx.offset() * 4;
		cpu.pc += off + 4;

		if (logging_enabled)
			printf("\tBranch taken: 0x%016llX\n", cpu.pc);
	}

R4300_IMPL(InstructionType::B, beq,
	"000100" "00000" "00000" IMM16,
	"OFFSET");
R4300_IMPL(InstructionType::BEQZ, beq,
	"010100" RS "00000" IMM16,
	"RS, OFFSET");
R4300_IMPL(InstructionType::BEQL, beq,
	"010100" RS RT IMM16,
	"RS, RT, OFFSET");
R4300_IMPL(InstructionType::BEQ, beq,
	"000100" RS RT IMM16,
	"RS, RT, OFFSET");
	void cpu_beq(ExecutionContext& ctx)
	{
		if (ctx.rs() == ctx.rt())
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() * 4;
			cpu.pc += off + 4;

			if (logging_enabled)
				printf("\tBranch taken: 0x%016llX\n", cpu.pc);
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BGTZ, bgtz,
	"000111" RS "00000" IMM16,
	"RS, OFFSET");
	void cpu_bgtz(ExecutionContext& ctx)
	{
		if (ctx.rs() > 0)
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() * 4;
			cpu.pc += off + 4;

			if (logging_enabled)
				printf("\tBranch taken: 0x%016llX\n", cpu.pc);
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BNEZ, bne,
	"000101" RS "00000" IMM16,
	"RS, OFFSET");
R4300_IMPL(InstructionType::BNE, bne,
	"000101" RS RT IMM16,
	"RS, RT, OFFSET");
R4300_IMPL(InstructionType::BNEL, bne,
	"010101" RS RT IMM16,
	"RS, RT, OFFSET");
	void cpu_bne(ExecutionContext& ctx)
	{
		if (ctx.rs() != ctx.rt())
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() * 4;
			cpu.pc += off + 4;

			if (logging_enabled)
				printf("\tBranch taken: 0x%016llX\n", cpu.pc);
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BGEZ, bgez,
	"000001" RS "00001" IMM16,
	"RS, OFFSET");
R4300_IMPL(InstructionType::BGEZL, bgez,
	"000001" RS "00011" IMM16,
	"RS, OFFSET");
	void cpu_bgez(ExecutionContext& ctx)
	{
		if (int32_t(ctx.rs()) >= 0)
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() * 4;
			cpu.pc += (off & 0x3FFFF) + 4;

			if (logging_enabled)
				printf("\tBranch taken: 0x%016llX\n", cpu.pc);
		}
		else
		{
			cpu.pc += 8;
		}
	}

R4300_IMPL(InstructionType::BLEZ, blezl,
	"000110" RS "00000" IMM16,
	"RS, OFFSET");
R4300_IMPL(InstructionType::BLEZL, blezl,
	"010110" RS "00000" IMM16,
	"RS, OFFSET");
	void cpu_blezl(ExecutionContext& ctx)
	{
		if (int32_t(ctx.rs()) <= 0)
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() * 4;
			cpu.pc += (off & 0x3FFFF) + 4;

			if (logging_enabled)
				printf("\tBranch taken: 0x%016llX\n", cpu.pc);
		}
		else
		{
			cpu.pc += 8;
		}
	}	

R4300_IMPL(InstructionType::BLTZ, bltz,
	"000001" RS "00000" IMM16,
	"RS, OFFSET");
	void cpu_bltz(ExecutionContext& ctx)
	{
		if (int32_t(ctx.rs()) < 0)
		{
			branch_delay_slot_address = cpu.pc + 4;
			int32_t off = ctx.offset() * 4;
			cpu.pc += (off & 0x3FFFF) + 4;

			if (logging_enabled)
				printf("\tBranch taken: 0x%016llX\n", cpu.pc);
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

R4300_IMPL(InstructionType::LBU, lbu,
	"100100" BASE RT IMM16,
	"RT, OFFSET(RS)");
	void cpu_lbu(ExecutionContext& ctx)
	{
		uint8_t v{};
		uint32_t addr = ctx.rs() + ctx.imm();

		if (!memory_read8(addr, v))
			throw MemException{"bad mem read", addr, 1 };

		ctx.rt() = v;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LW, lw,
	"100011" RS RT IMM16,
	"RT, OFFSET(RS)");
	void cpu_lw(ExecutionContext& ctx)
	{
		uint32_t v{};
		uint32_t address = ctx.rs() + ctx.offset();

		if ((address & 3) != 0)
			throw MemException{"lw poop", address, 4 };
		
		if (!memory_read32(address, v))
			throw MemException{"bad mem read", address, 4 };

		ctx.rt() = (int32_t)v;

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::LWR, lwr,
	"100110" RS RT IMM16,
	"RT, OFFSET(RS)");
	void cpu_lwr(ExecutionContext& ctx)
	{
		auto addr = ctx.base() + ctx.offset();

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
	"101000" RS RT IMM16,
	"RT, OFFSET(RS)");
	void cpu_sb(ExecutionContext& ctx)
	{
		auto addr = ctx.rs() + ctx.offset();
		if (!memory_write8(addr, ctx.rt()))
			throw MemException{"bad mem write", uint32_t(addr), 1 };

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SW, sw,
	"101011" RS RT IMM16,
	"RT, OFFSET(RS)");
	void cpu_sw(ExecutionContext& ctx)
	{
		auto addr = ctx.rs() + ctx.offset();

		if (!memory_write32(addr, ctx.rt() & 0xFFFFFFFF))
			throw MemException{"bad mem write", uint32_t(addr), 4};

		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::CACHE, cache,
	"101111" BASE OP IMM16,
	"")
	void cpu_cache(ExecutionContext& ctx)
	{
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MFC0, mfc0,
	"010000" "00000" RT RD "00000000" "000",
	"RT, COP_RD");
	void cpu_mfc0(ExecutionContext& ctx)
	{
		cpu_get_cop0_register(ctx.rd_bits(), ctx.rt());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::MTC0, mtc0,
	"010000" "00100" RT RD "00000000" "000",
	"RT, COP_RD");
	void cpu_mtc0(ExecutionContext& ctx)
	{
		cpu_set_cop0_register(ctx.rd_bits(), ctx.rt());
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::CTC1, ctc1,
	"010001" "00110" RT RD "000" "00000" "000",
	"RT, RD");
	void cpu_ctc1(ExecutionContext& ctx)
	{
		ctx.fs() = float(ctx.rt() & 0xFFFFFFFF);
		cpu.pc += 4;
	}
/**************************************************************************/

R4300_IMPL(InstructionType::SLL, sll,
	"000000" RS RT RD SHIFT "000000",
	"RD, RT, SA");
	void cpu_sll(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() << ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLLV, sllv,
	"000000" RS RT RD "00000" "000100",
	"RD, RT, RS");
	void cpu_sllv(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() << ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRA, sra,
	"000000" "00000" RT RD SHIFT "000011",
	"RD, RT, SA");
	void cpu_sra(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRAV, srav,
	"000000" RS RT RD "00000" "000111",
	"RD, RT, RS");
	void cpu_srav(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rt() >> ctx.rs();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRL, srl,
	"000000" "00000" RT RD SHIFT "000010",
	"RD, RT, SA");
	void cpu_srl(ExecutionContext& ctx)
	{
		uint32_t rt = ctx.rt();
		ctx.rd() = rt >> ctx.shift_bits();
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SRLV, srlv,
	"000000" RS RT RD "00000" "000110",
	"RD, RT, RS");
	void cpu_srlv(ExecutionContext& ctx)
	{
		uint32_t rt = ctx.rt();
		ctx.rd() = rt >> (ctx.rs() & 0x1F);
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLT, slt,
	"000000" RS RT RD "00000" "101010",
	"RD, RS, RT");
	void cpu_slt(ExecutionContext& ctx)
	{
		ctx.rd() = int64_t(ctx.rs()) < int64_t(ctx.rt()) ? 1 : 0;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLTU, sltu,
	"000000" RS RT RD "00000" "101011",
	"RD, RS, RT")
	void cpu_sltu(ExecutionContext& ctx)
	{
		ctx.rd() = ctx.rs() < ctx.rt() ? 1 : 0;
		cpu.pc += 4;
	}


R4300_IMPL(InstructionType::SLTI, slti,
	"001010" RS RT IMM16,
	"RT, RS, IMM");
	void cpu_slti(ExecutionContext& ctx)
	{
		ctx.rt() = int64_t(ctx.rs()) < ctx.imm() ? 1 : 0;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::SLTIU, sltiu,
	"001011" RS RT IMM16,
	"RS, RT, IMM");
	void cpu_sltiu(ExecutionContext& ctx)
	{
		ctx.rt() = ctx.rs() < uint16_t(ctx.imm()) ? 1 : 0;
		cpu.pc += 4;
	}

R4300_IMPL(InstructionType::EXT, ext,
	"011111" RS RT RD SHIFT "000000",
	"RT, RS, SHIFT, RD");
	void cpu_ext(ExecutionContext& ctx)
	{
		ctx.rt() = extract_bits<uint32_t>(ctx.rs(), ctx.shift_bits(), ctx.rd_bits());
		cpu.pc +=4;
	}