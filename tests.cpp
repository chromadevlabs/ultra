
#include "cpu_types.h"
#include "disassembler.h"

#include <cstring>
#include <functional>

void cpu_test_reset()
{
	memset(&cpu, 0x00, sizeof(CPU));
}

bool cpu_execute_single(InstructionType type, ExecutionContext context, std::function<bool()>&& pred)
{
	if (auto* desc = disassembler_find_descriptor(type))
	{
		desc->func(context);
		return pred();
	}

	return false;
}

#define TEST(Type, op_bits, target, result) { \
if (cpu_execute_single(InstructionType::Type, {uint32_t(op_bits)}, [=]{ return target == result; })) { \
printf("TEST %s: %s == %d OK!!\n", #Type, #target, (int)result); } else { \
printf("TEST %s: %s == %d (expected %d) Failed!!\n", #Type, #target, (int)target, (int)result); throw; }}

void cpu_run_tests()
{
	{
		cpu_test_reset();
		TEST(ADDI, 	SET_RT_BITS(1) | SET_RS_BITS(1) | SET_IMM_BITS(-10),	cpu.gpr[1], -10);
		TEST(ADD, 	SET_RD_BITS(1) | SET_RS_BITS(1) | SET_RT_BITS(1), 		cpu.gpr[1], -20);
	}

	{
		cpu_test_reset();
		cpu.gpr[1] = 0xDEAD;
		cpu.gpr[2] = 0xAD;
		TEST(AND, 	SET_RD_BITS(1) | SET_RS_BITS(1) | SET_RT_BITS(2),		cpu.gpr[1], 0xAD)
		TEST(ANDI,	SET_RT_BITS(1) | SET_RS_BITS(1) | SET_IMM_BITS(0xAD), 	cpu.gpr[1], 0xAD);	
	}

	{
		cpu_test_reset();
		TEST(LUI,	SET_RT_BITS(1) | SET_IMM_BITS(0xFF), 					cpu.gpr[1], uint32_t(0xFF) << 16);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 1;
		cpu.gpr[3] = 1;
		uint64_t result = ~(1 | 1);
		TEST(NOR,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], result);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 5;
		cpu.gpr[3] = 5;
		TEST(OR,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], 5);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 5;
		TEST(ORI,	SET_RT_BITS(1) | SET_RS_BITS(2) |  SET_IMM_BITS(5),		cpu.gpr[1], 5);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 0;
		cpu.gpr[3] = 1;
		TEST(SLT,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], 1);
		TEST(SLT,	SET_RD_BITS(1) | SET_RS_BITS(3) | SET_RT_BITS(2),		cpu.gpr[1], 0);

		TEST(SLTI,	SET_RT_BITS(1) | SET_RS_BITS(2) |  SET_IMM_BITS(1),		cpu.gpr[1], 1);
		TEST(SLTI,	SET_RT_BITS(1) | SET_RS_BITS(2) |  SET_IMM_BITS(-1),	cpu.gpr[1], 0);

		cpu.gpr[2] = 5;
		TEST(SLTIU,	SET_RT_BITS(1) | SET_RS_BITS(2) |  SET_IMM_BITS(6),		cpu.gpr[1], 1);
		TEST(SLTIU,	SET_RT_BITS(1) | SET_RS_BITS(2) |  SET_IMM_BITS(4),		cpu.gpr[1], 0);

		cpu.gpr[2] = 2;
		cpu.gpr[3] = 5;
		TEST(SLTU,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], 1);
		TEST(SLTU,	SET_RD_BITS(1) | SET_RS_BITS(3) | SET_RT_BITS(2),		cpu.gpr[1], 0);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = -10;
		cpu.gpr[3] = 10;
		TEST(SUB,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], -20);
		
		cpu.gpr[2] = 10;
		TEST(SUBU,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], 0);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 0xedad;
		cpu.gpr[3] = 12345;
		uint64_t result = 0xedad ^ 12345;
		TEST(XOR,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], result);
		TEST(XORI,	SET_RT_BITS(1) | SET_RS_BITS(2) | SET_IMM_BITS(12345),	cpu.gpr[1], result);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 0xedad;
		cpu.gpr[3] = 12345;
		uint64_t result = 0xedad ^ 12345;
		TEST(XOR,	SET_RD_BITS(1) | SET_RS_BITS(2) | SET_RT_BITS(3),		cpu.gpr[1], result);
		TEST(XORI,	SET_RT_BITS(1) | SET_RS_BITS(2) | SET_IMM_BITS(12345),	cpu.gpr[1], result);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = 1;
		cpu.gpr[3] = 1;
		
		TEST(SLL,	SET_RD_BITS(1) | SET_RT_BITS(2) | SET_SHIFT_BITS(1),	cpu.gpr[1], 2);
		TEST(SLLV,	SET_RD_BITS(1) | SET_RT_BITS(2) | SET_RS_BITS(3),		cpu.gpr[1], 2);

		cpu.gpr[2] = 4;
		cpu.gpr[3] = 1;
		TEST(SRA,	SET_RD_BITS(1) | SET_RT_BITS(2) | SET_SHIFT_BITS(1),	cpu.gpr[1], 2);
		TEST(SRAV,	SET_RD_BITS(1) | SET_RT_BITS(2) | SET_RS_BITS(3),		cpu.gpr[1], 2);
		TEST(SRL,	SET_RD_BITS(1) | SET_RT_BITS(2) | SET_SHIFT_BITS(1),	cpu.gpr[1], 2);
		TEST(SRLV,	SET_RD_BITS(1) | SET_RT_BITS(2) | SET_RS_BITS(3),		cpu.gpr[1], 2);
	}

	{
		cpu_test_reset();
		cpu.gpr[2] = -1000;
		cpu.gpr[3] = 666;

		auto hi = -1000 % 666;
		auto lo = -1000 / 666;

		TEST(DIV,	SET_RS_BITS(2) | SET_RT_BITS(3),						cpu.hi, (int32_t)hi);
		TEST(DIV,	SET_RS_BITS(2) | SET_RT_BITS(3),						cpu.lo, (int32_t)lo);

		cpu.gpr[2] = 1000;
		cpu.gpr[3] = 666;

		hi = cpu.gpr[2] % cpu.gpr[3];
		lo = cpu.gpr[2] / cpu.gpr[3];

		TEST(DIVU,	SET_RS_BITS(2) | SET_RT_BITS(3),						cpu.hi, hi);
		TEST(DIVU,	SET_RS_BITS(2) | SET_RT_BITS(3),						cpu.lo, lo);
	}

	{
		cpu_test_reset();

		cpu.hi = 0xdeadbeef;
		cpu.lo = 0xdeadbeef;
		TEST(MFHI,	SET_RD_BITS(1),											cpu.gpr[1], 0xdeadbeef);
		TEST(MFLO,	SET_RD_BITS(2),											cpu.gpr[2], 0xdeadbeef);

		cpu.gpr[1] = 0xcafebabe;
		TEST(MTHI,	SET_RS_BITS(1), 										cpu.hi, 	0xcafebabe);
		TEST(MTLO,	SET_RS_BITS(1), 										cpu.lo, 	0xcafebabe);
	}

	{
		cpu_test_reset();

		cpu.gpr[1] = -1000;
		cpu.gpr[2] = 1000;

		TEST(MULT,	SET_RS_BITS(1) | SET_RT_BITS(2),						cpu.hi_lo,  (-1000 * 1000));

		cpu.gpr[1] = 1000;
		cpu.gpr[2] = 1000;
		TEST(MULTU,	SET_RS_BITS(1) | SET_RT_BITS(2),						cpu.hi_lo, 	(1000 * 1000));
	}

	{
		cpu_test_reset();

		cpu.pc = 0;
		cpu.gpr[1] = 1;
		cpu.gpr[2] = 1;
		TEST(BEQ,	SET_RS_BITS(1) | SET_RT_BITS(2) | SET_IMM_BITS(2), 		cpu.pc,		 (2 * 4) + 4);
		
		cpu.pc = 0;
		cpu.gpr[2] = 0;
		TEST(BEQ,	SET_RS_BITS(1) | SET_RT_BITS(2) | SET_IMM_BITS(2), 		cpu.pc,		(1 * 4) + 4);

		cpu.pc = 0;
		cpu.gpr[1] = 0;
		TEST(BEQZ,	SET_RS_BITS(1) | SET_IMM_BITS(2), 						cpu.pc,		(2 * 4) + 4);
		
		cpu.pc = 0;
		cpu.gpr[1] = 1;
		TEST(BEQZ,	SET_RS_BITS(1) | SET_IMM_BITS(2), 						cpu.pc,		(1 * 4) + 4);

		cpu.pc = 0;
		cpu.gpr[1] = 10;
		TEST(BGTZ,	SET_RS_BITS(1) | SET_IMM_BITS(2), 						cpu.pc,		(2 * 4) + 4);

		cpu.pc = 0;
		cpu.gpr[1] = -10;
		TEST(BLEZ,	SET_RS_BITS(1) | SET_IMM_BITS(2), 						cpu.pc,		(2 * 4) + 4);

		cpu.pc = 0;
		cpu.gpr[1] = -10;
		TEST(BLTZ,	SET_RS_BITS(1) | SET_IMM_BITS(2), 						cpu.pc,		(2 * 4) + 4);

		cpu.pc = 0;
		cpu.gpr[1] = -10;
		TEST(BNE,	SET_RS_BITS(1) | SET_RT_BITS(2) | SET_IMM_BITS(2), 		cpu.pc,		(2 * 4) + 4);

		cpu.pc = 0;
		TEST(J,		SET_JMP_BITS(1), 										cpu.pc,		0x8);
	}

	{
		
	}
}