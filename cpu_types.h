#pragma once

#include <cstdint>
#include <initializer_list>
#include <utility>
#include <functional>

#include "instruction_types.h"

#define INST_ENCODING_BITMASK		0xFC000000
#define RS_ENCODING_BITMASK			0x3E00000
#define RT_ENCODING_BITMASK			0x1F0000
#define RD_ENCODING_BITMASK			0xF800
#define SHIFT_ENCODING_BITMASK		0x7C0
#define FUNC_ENCODING_BITMASK		0x3F

#define GET_INST_BITS(value)		((value >> 26) & 0x3F)
#define GET_RS_BITS(value)			((value >> 21) & 0x1F)
#define GET_RT_BITS(value)			((value >> 16) & 0x1F)
#define GET_RD_BITS(value)			((value >> 11) & 0x1F)
#define GET_SHIFT_BITS(value)		((value >> 6) & 0x1F)
#define GET_FUNC_BITS(value)		((value >> 0) & 0x3F)

#define GET_IMM_BITS(value)			(value & 0xFFFF)
#define GET_JMP_BITS(value)			(value & 0x3FFFFFF)

#define SET_INST_BITS(value) 		((value & 0x3F) << 26)
#define SET_RS_BITS(value) 			((value & 0x1F) << 21)
#define SET_RT_BITS(value) 			((value & 0x1F) << 16)
#define SET_RD_BITS(value) 			((value & 0x1F) << 11)
#define SET_SHIFT_BITS(value) 		((value & 0x1F) << 6)
#define SET_FUNC_BITS(value) 		((value & 0x3F) << 0)
#define SET_IMM_BITS(value)			(value & 0xFFFF)
#define SET_OFFSET_BITS(value)		SET_IMM_BITS(value)

using cpu_func_t = void(*)(struct ExecutionContext&);

struct MemException
{
	const char* message;
	uint32_t address;
	uint32_t size;
};

template<typename T>
struct MemoryMappedRegister
{
	T value{};
	std::function<void(T&, bool)> rw_callback;

	MemoryMappedRegister(decltype(rw_callback)&& cb) :
		rw_callback(cb)
	{
	}

	void read(uint32_t addr, uint32_t size, void* dst)
	{
		rw_callback(value, false);
		memcpy(dst, &value, size);
	}

	void write(uint32_t addr, uint32_t size, const void* src)
	{
		memcpy(&value, src, size);
		rw_callback(value, true);
	}
};


//https://mathcs.holycross.edu/~csci226/MIPS/summaryHO.pdf
struct CPU
{
	uint64_t gpr[32]{};
	uint64_t pc{};

	union
	{
		struct {
			uint32_t lo;
			uint32_t hi;
		};

		uint64_t hi_lo;
	};

	uint32_t fcr[2]{};
	bool ll{};

	uint64_t cop0[32]{};
	float cop1[32]{};

	uint64_t r0() { return 0; }
	auto& at() { return gpr[1]; }
	
	auto& v0() { return gpr[2]; }
	auto& v1() { return gpr[3]; }
	
	auto& a0() { return gpr[4]; }
	auto& a1() { return gpr[5]; }
	auto& a2() { return gpr[6]; }
	auto& a3() { return gpr[7]; }
	
	auto& t0() { return gpr[8]; }
	auto& t1() { return gpr[9]; }
	auto& t2() { return gpr[10]; }
	auto& t3() { return gpr[11]; }
	auto& t4() { return gpr[12]; }
	auto& t5() { return gpr[13]; }
	auto& t6() { return gpr[14]; }
	auto& t7() { return gpr[15]; }
	auto& t8() { return gpr[24]; }
	auto& t9() { return gpr[25]; }

	auto& s0() { return gpr[16]; }
	auto& s1() { return gpr[17]; }
	auto& s2() { return gpr[18]; }
	auto& s3() { return gpr[19]; }
	auto& s4() { return gpr[20]; }
	auto& s5() { return gpr[21]; }
	auto& s6() { return gpr[22]; }
	auto& s7() { return gpr[23]; }

	auto& k0() { return gpr[26]; }
	auto& k1() { return gpr[27]; }

	auto& gp() { return gpr[28]; }
	auto& sp() { return gpr[29]; }
	auto& fp() { return gpr[30]; }
	auto& ra() { return gpr[31]; }

	auto& index() 		{ return cop0[0]; }
	auto& random() 		{ return cop0[1]; }
	auto& entry_lo0() 	{ return cop0[2]; }
	auto& entry_hi0()	{ return cop0[3]; }
	auto& context() 	{ return cop0[4]; }
	auto& pagemask() 	{ return cop0[5]; }
	auto& wired() 		{ return cop0[6]; }
	auto& bad_vaddr() 	{ return cop0[8]; }
	auto& count() 		{ return cop0[9]; }
	auto& entry_hi() 	{ return cop0[10]; }
	auto& compare() 	{ return cop0[11]; }
	auto& status() 		{ return cop0[12]; }
	auto& cause() 		{ return cop0[13]; }
	auto& epc() 		{ return cop0[14]; }
	auto& prid() 		{ return cop0[15]; }
	auto& config() 		{ return cop0[16]; }
	auto& ll_addr() 	{ return cop0[17]; }
	auto& watch_lo() 	{ return cop0[18]; }
	auto& watch_hi() 	{ return cop0[19]; }
	auto& xcontext() 	{ return cop0[20]; }
	auto& parity_error(){ return cop0[26]; }
	auto& cache_error() { return cop0[27]; }
	auto& tag_lo() 		{ return cop0[28]; }
	auto& tag_hi() 		{ return cop0[29]; }
	auto& error_epc() 	{ return cop0[30]; }	
};

extern CPU cpu;
extern uint64_t branch_delay_slot_address;
extern uint64_t cycle_counter;

void cpu_link(uint64_t);
void cpu_trap(const char*);

struct ExecutionContext
{
	const uint32_t opbits;

	constexpr uint8_t rd_bits() const { return GET_RD_BITS(opbits); }
	constexpr uint8_t rs_bits() const { return GET_RS_BITS(opbits); }
	constexpr uint8_t rt_bits() const { return GET_RT_BITS(opbits); }
	constexpr uint8_t shift_bits() const { return GET_SHIFT_BITS(opbits); }
	
	constexpr int16_t imm_bits() const { return GET_IMM_BITS(opbits); }
	constexpr int32_t jmp_bits() const { return GET_JMP_BITS(opbits); }

	auto& rd() { return cpu.gpr[rd_bits()]; }
	auto& rs() { return cpu.gpr[rs_bits()]; }
	auto& base() { return rd(); }
	auto& rt() { return cpu.gpr[rt_bits()]; }

	constexpr auto imm() const { return imm_bits(); }

	constexpr int32_t jmp() const { return jmp_bits(); }
	constexpr int16_t offset() const { return imm_bits(); }
};