#pragma once

#include <cstdint>

enum class InstructionType : uint8_t
{
	ADD, ADDI, ADDIU, ADDU,
	DADDI,
	AND, ANDI,
	LUI,
	MFHI, MFLO, MTHI, MTLO, 
	MOVE,
	
	NOR, OR, ORI,
	SLT, SLTI, SLTIU, SLTU,
	SUB, SUBU,
	XOR, XORI,
	SLL, SLLV, SRA, SRAV, SRL, SRLV,
	DIV, DIVU, 
	EXT,
	MULT, MULTU,
	B, BAL, BEQ, BEQZ, BEQL, BGEZ, BGEZL, BGEZAL, BGTZ, BLEZ, BLEZL, BLTZ, BLTZAL, BNEZ, BNEZL,
	BNE, BNEL,
	Break,
	J, JAL, JALR, JR, 
	MFC0, MTC0,
	CTC1,
	CACHE,
	SYSCALL,
	LWR,
	LB, LBU, LH, LHU, LW, LWU,
	SB, SH, SW,
	NOP,

	NumInstructions,
	UnknownInstruction
};
