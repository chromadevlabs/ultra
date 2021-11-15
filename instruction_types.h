#pragma once

#include <cstdint>

enum class InstructionType : uint8_t
{
	ADD, ADDI, ADDIU, ADDU,
	DADDI,
	AND, ANDI,
	LUI,
	NOR, OR, ORI,
	SLT, SLTI, SLTIU, SLTU,
	SUB, SUBU,
	XOR, XORI,
	SLL, SLLV, SRA, SRAV, SRL, SRLV,
	DIV, DIVU, 
	EXT,
	MFHI, MFLO, MTHI, MTLO, 
	MULT, MULTU,
	B, BEQ, BEQL, BGEZ, BGEZAL, BGTZ, BLEZ, BLEZL, BLTZ, BLTZAL,
	BNE, BNEL,
	Break,
	J, JAL, JALR, JR, 
	MFC0, MTC0,
	SYSCALL,
	LWR,
	LB, LBU, LH, LHU, LW, LWU,
	SB, SH, SW,
	NOP,

	NumInstructions,
	UnknownInstruction
};
