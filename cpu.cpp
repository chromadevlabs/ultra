
#include "cpu.h"

#include "magic_enum.hpp"
#include "memory.h"
#include "platform.h"
#include "cpu_types.h"
#include "cartridge.h"
#include "disassembler.h"

#include <cstring>

// Implemented in parser.cpp
const char* parser_get_symbolic_gpr_name(int i);
const char* parser_get_symbolic_cop0_name(int i);

struct RSP
{
	uint8_t dmem[0xFFF]{};
	uint8_t imem[0xFFF]{};
} rsp{};

CPU cpu{};

uint64_t branch_delay_slot_address{};
uint64_t cycle_counter{};

static uint8_t rdram[MB(8)];
//static uint8_t rdram[0x03EFFFFF];
static uint8_t pif_ram[0x3F];
uint8_t cartridge_rom[MB(64)];

static MemoryMappedRegister<uint32_t> RI_MODE_REG 			= { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_CONFIG_REG 		= { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_CURRENT_LOAD_REG 	= { [](uint32_t& value, bool write){}};

static MemoryMappedRegister<uint32_t> RI_SELECT_REG 		= { 
	[](uint32_t& value, bool write)
	{
		//value = 0x10101010;
	}
};

static MemoryMappedRegister<uint32_t> RI_REFRESH_REG 		= { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_LATENCY_REG 		= { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_RERROR_REG 		= { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_WERROR_REG 		= { [](uint32_t& value, bool write){}};

static MemoryMappedRegister<uint32_t> MI_INIT_MODE_REG = {
	[](uint32_t& value, bool write)
	{
		if (write)
		{
		}
		else
		{

		}
	}
};

static MemoryMappedRegister<uint32_t> MI_VERSION_REG = {
	[](uint32_t& value, bool write)
	{
		// version register
		//value = 0x00000000;
	}
};

static MemoryMappedRegister<uint32_t> MI_INTR_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> MI_INTR_MASK_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_MEM_ADDR_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_DRAM_ADDR_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_RD_LEN_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_WR_LEN_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_STATUS_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_DMA_FULL_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};


static MemoryMappedRegister<uint32_t> SP_DMA_BUSY_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_SEMAPHORE_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_PC_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> SP_IBIST_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

// DMA Destination address
static uint32_t dma_dst_addr{};
static uint32_t dma_src_addr{};

static MemoryMappedRegister<uint32_t> PI_DRAM_ADDR_REG = {
	[](uint32_t& value, bool write)
	{
		dma_dst_addr = value;
	}
};

// DMA Source Address
static MemoryMappedRegister<uint32_t> PI_CART_ADDR_REG = {
	[](uint32_t& value, bool write)
	{
		dma_src_addr = value;
	}
};

// DMA READ LENGTH, also fires the operation
static MemoryMappedRegister<uint32_t> PI_RD_LEN_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

// DMA WRITE LENGTH, also fires the operation
static MemoryMappedRegister<uint32_t> PI_WR_LEN_REG = {
	[](uint32_t& value, bool write)
	{
		memory_enable_logging(false);
		memory_do_dma(dma_dst_addr, dma_src_addr, value);
		memory_enable_logging(true);
	}
};
static MemoryMappedRegister<uint32_t> PI_STATUS_REG = {
	[](uint32_t& value, bool write)
	{
		if (write)
		{
		}
		else
		{
			value = 0;
			set_bit(value, 0, 0);
			set_bit(value, 1, 0);
			set_bit(value, 2, 0);
		}
	}
};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM1_LAT_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM1_PWD_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM1_PGS_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM1_RLS_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM2_LAT_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM2_PWD_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM2_PGS_REG = {};
//static MemoryMappedRegister<uint32_t> PI_BSD_DOM2_RLS_REG = {};

static MemoryMappedRegister<uint32_t> VI_CONTROL_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_ORIGIN_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_WIDTH_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_INTR_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_V_CURRENT_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_BURST_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_V_SYNC_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};

static MemoryMappedRegister<uint32_t> VI_H_SYNC_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};
static MemoryMappedRegister<uint32_t> VI_LEAP_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};
static MemoryMappedRegister<uint32_t> VI_H_START_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};
static MemoryMappedRegister<uint32_t> VI_V_START_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};
static MemoryMappedRegister<uint32_t> VI_V_BURST_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};
static MemoryMappedRegister<uint32_t> VI_X_SCALE_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};
static MemoryMappedRegister<uint32_t> VI_Y_SCALE_REG = {
	[](uint32_t& value, bool write)
	{
		throw nullptr;
	}
};



template<typename T>
void bind_register(MemoryMappedRegister<T>* reg, uint32_t addr_start, uint32_t addr_end, const char* name)
{
	memory_install_rw_callback(
		addr_start, addr_end,
		std::bind(&MemoryMappedRegister<T>::read, reg, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(&MemoryMappedRegister<T>::write, reg, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		name
	);
}

void cpu_link(uint64_t address)
{
	cpu.gpr[31] = address;
}

void cpu_init()
{
	memory_init();
	disassembler_init();

	// main system ram (with expansion pack)
	memory_install_rw_callback(
		0x00000000, 0x03EFFFFF,
		[](uint32_t address, uint32_t size, void* dst)
		{
			memcpy(dst, rdram + address, size);
		},
		[](uint32_t address, uint32_t size, const void* src)
		{
			assert(address + size < sizeof(rdram));
			memcpy(rdram + address, src, size);
		},
		"RDRAM Memory"
	);

	// hack to fix the crash that happens when we try and work with the stack
	static uint8_t stack_hack[MB(1)]{};
	memory_install_rw_callback(
		0x04002000, 0x0403FFFF,
		[](uint32_t addr, uint32_t size, void* dst) { memcpy(dst, stack_hack + addr, size); },
		[](uint32_t addr, uint32_t size, const void* src) { memcpy(stack_hack + addr, src, size); },
		"RDRAM Memory Mirror???"
	);

	// RSP data memory
	memory_install_rw_callback(
		0x04000000, 0x04000FFF,
		std::bind(default_buffer_read, rsp.dmem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(default_buffer_write, rsp.dmem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		"RSP_DMEM"
	);

	// RSP code memory
	memory_install_rw_callback(
		0x04001000, 0x04001FFF,
		std::bind(default_buffer_read, rsp.imem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(default_buffer_write, rsp.imem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		"RSP_IMEM"
	);

	// N64DD address (return all 0xFF when not connected)
	memory_install_rw_callback(
		0x05000000, 0x07FFFFFF,
		[](uint32_t, uint32_t size, void* dst){ memset(dst, 0xFF, size); },
		[](uint32_t, uint32_t, const void* src){ },
		"N64DD"
	);

	// SRAM
	memory_install_rw_callback(
		0x08000000, 0x0FFFFFFF,
		[](uint32_t, uint32_t, void*){ },
		[](uint32_t, uint32_t, const void*){ },
		"Cartridge SRAM"
	);
	
	// cartridge data
	memory_install_rw_callback(
		0x10000000, 0x1FBFFFFF, 
		std::bind(default_buffer_read, cartridge_rom, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		[](uint32_t, uint32_t, const void*) { printf("Write attempt to cartridge space\n"); throw nullptr; },
		"Cartridge ROM"
	);
	
	// PIF RAM
	memory_install_rw_callback(
		0x1FC007C0, 0x1FC007FF, 
		std::bind(default_buffer_read, pif_ram, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(default_buffer_write, pif_ram, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		"PIF RAM"
	);

	// RDMEM registers (ignored)
	memory_install_rw_callback(
		0x03F00000, 0x03FFFFFF,
		[](uint32_t, uint32_t, void*){ },
		[](uint32_t, uint32_t, const void*){ },
		"RDMEM registers "
	);

	// Unused SP register space?
	memory_install_rw_callback(
		0x04040020, 0x0407FFFF,
		[](uint32_t, uint32_t, void*){ },
		[](uint32_t, uint32_t, const void*){ },
		"Unused SP registers"
	);

	/*static char debug_string_buffer[128]{};

	memory_install_rw_callback(
		0xB3FF0020, 0xB3FF0220,
		[](uint32_t, uint32_t, void*) {},
		[](uint32_t, uint32_t, const void*)
		{ 
			printf("string buffer written to!\n");
			throw nullptr;
		},
		"debug string buffer"
	);

	memory_install_rw_callback(
		0xB3FF0014, 0xB3FF0014 + 2,
		[](uint32_t, uint32_t, void*) {},
		[](uint32_t, uint32_t, const void* data)
		{
			printf("string buffer length: %d!\n", *(uint16_t*)data);
			throw nullptr;
		},
		"debug string length"
	);*/

	bind_register(&RI_MODE_REG, 		0x04700000, 0x04700003, "RI_MODE_REG");
	bind_register(&RI_CONFIG_REG, 		0x04700004, 0x04700007, "RI_CONFIG_REG");
	bind_register(&RI_CURRENT_LOAD_REG, 0x04700008, 0x0470000B, "RI_CURRENT_LOAD_REG");
	bind_register(&RI_SELECT_REG, 		0x0470000C, 0x0470000F, "RI_SELECT_REG");
	bind_register(&RI_REFRESH_REG, 		0x04700010, 0x04700013, "RI_REFRESH_REG");
	bind_register(&RI_LATENCY_REG, 		0x04700014, 0x04700017, "RI_LATENCY_REG");
	bind_register(&RI_RERROR_REG, 		0x04700018, 0x0470001B, "RI_RERROR_REG");
	bind_register(&RI_WERROR_REG, 		0x0470001C, 0x0470001F, "RI_WERROR_REG");

	bind_register(&MI_INIT_MODE_REG, 	0x04300000, 0x04300003, "MI_INIT_MODE_REG");
	bind_register(&MI_VERSION_REG, 		0x04300004, 0x04300007, "MI_VERSION_REG");
	bind_register(&MI_INTR_REG, 		0x04300008, 0x0430000B, "MI_INTR_REG");
	bind_register(&MI_INTR_MASK_REG, 	0x0430000C, 0x0430000F, "MI_INTR_MASK_REG");

	bind_register(&SP_MEM_ADDR_REG, 	0x04040000, 0x04040003, "SP_MEM_ADDR_REG");
	bind_register(&SP_DRAM_ADDR_REG, 	0x04040004, 0x04040007, "SP_DRAM_ADDR_REG");
	bind_register(&SP_RD_LEN_REG, 		0x04040008, 0x0404000B, "SP_RD_LEN_REG");
	bind_register(&SP_WR_LEN_REG, 		0x0404000C, 0x0404000F, "SP_WR_LEN_REG");
	bind_register(&SP_STATUS_REG, 		0x04040010, 0x04040013, "SP_STATUS_REG");
	bind_register(&SP_DMA_FULL_REG, 	0x04040014, 0x04040017, "SP_DMA_FULL_REG");
	bind_register(&SP_DMA_BUSY_REG, 	0x04040018, 0x0404001B, "SP_DMA_BUSY_REG");
	bind_register(&SP_SEMAPHORE_REG, 	0x0404001C, 0x0404001F, "SP_SEMAPHORE_REG");
	bind_register(&SP_PC_REG, 			0x04080000, 0x04080003, "SP_PC_REG");
	bind_register(&SP_IBIST_REG, 		0x04080004, 0x04080007, "SP_IBIST_REG");
	
	bind_register(&VI_CONTROL_REG, 		0x04400000, 0x04400000 + 4, "VI_CONTROL_REG");
	bind_register(&VI_ORIGIN_REG, 		0x04400004, 0x04400004 + 4, "VI_ORIGIN_REG");
	bind_register(&VI_WIDTH_REG, 		0x04400008, 0x04400008 + 4, "VI_WIDTH_REG");
	bind_register(&VI_INTR_REG, 		0x0440000C, 0x0440000C + 4, "VI_INTR_REG");
	bind_register(&VI_V_CURRENT_REG, 	0x04400010, 0x04400010 + 4, "VI_V_CURRENT_REG");
	bind_register(&VI_BURST_REG, 		0x04400014, 0x04400014 + 4, "VI_BURST_REG");
	bind_register(&VI_V_SYNC_REG, 		0x04400018, 0x04400018 + 4, "VI_V_SYNC_REG");
	bind_register(&VI_H_SYNC_REG, 		0x0440001C, 0x0440001C + 4, "VI_H_SYNC_REG");
	bind_register(&VI_LEAP_REG, 		0x04400020, 0x04400020 + 4, "VI_LEAP_REG");
	bind_register(&VI_H_START_REG, 		0x04400024, 0x04400024 + 4, "VI_H_START_REG");
	bind_register(&VI_V_START_REG, 		0x04400028, 0x04400028 + 4, "VI_V_START_REG");
	bind_register(&VI_V_BURST_REG, 		0x0440002C, 0x0440002C + 4, "VI_V_BURST_REG");
	bind_register(&VI_X_SCALE_REG, 		0x04400030, 0x04400030 + 4, "VI_X_SCALE_REG");
	bind_register(&VI_Y_SCALE_REG, 		0x04400034, 0x04400034 + 4, "VI_Y_SCALE_REG");
	
	bind_register(&PI_DRAM_ADDR_REG,	0x04600000, 0x04600003, "PI_DRAM_ADDR_REG");
	bind_register(&PI_CART_ADDR_REG,	0x04600004, 0x04600007, "PI_CART_ADDR_REG");
	bind_register(&PI_RD_LEN_REG,		0x04600008, 0x0460000B, "PI_RD_LEN_REG");
	bind_register(&PI_WR_LEN_REG,		0x0460000C, 0x0460000F, "PI_WR_LEN_REG");
	bind_register(&PI_STATUS_REG,		0x04600010, 0x04600013, "PI_STATUS_REG");

	// reset cpu
	cpu.pc = 0;
	cpu.hi_lo = 0;
	cpu.fcr[0] = 0;
	cpu.fcr[1] = 0;
	cpu.ll = false;

	/*******************************************************/
	// PIF emulation
	memory_enable_logging(false);

	memory_write32(0xA4001000 + 0,  0x3c0dbfc0);
	memory_write32(0xA4001000 + 4,  0x8da807fc);
	memory_write32(0xA4001000 + 8,  0x25ad07c0);
	memory_write32(0xA4001000 + 12, 0x31080080);
	memory_write32(0xA4001000 + 16, 0x5500fffc);
	memory_write32(0xA4001000 + 20, 0x3c0dbfc0);
	memory_write32(0xA4001000 + 24, 0x8da80024);
	memory_write32(0xA4001000 + 28, 0x3c0bb000);
	
	cpu.t3() = 0xFFFFFFFFA4000040;
	cpu.s4() = 0x0000000000000001;
	cpu.s6() = 0x000000000000003F;
	cpu.sp() = 0xFFFFFFFFA4001FF0;

	cpu.cop0.r[0] = 0;
	cpu.cop0.r[1] = 0x0000001F; 		// random
	cpu.cop0.r[12] = 0x70400004;		// status
	cpu.cop0.r[15] = 0x00000B00;		// PRId
	cpu.cop0.r[16] = 0x0006E463;		// Config

	memory_write32(0x04300004, 0x10101010);
	/*******************************************************/

	memory_load_rom(
		"/Users/chroma/Downloads/N64-master/HelloWorld/16BPP/HelloWorldCPU320x240/HelloWorldCPU16BPP320X240.N64",
		true
	);

	printf("Copying rom code to rdram...");
	memory_do_dma(0xA4000000, 0xB0000000, 0x1000);
	printf("OK!\n");

	//cpu.pc = memory_get_rom_header()->pc;
	cpu.pc = 0xFFFFFFFFA4000040;

	memory_enable_logging(true);
}

void cpu_get_cop0_register(int index, uint64_t& value)
{
	throw nullptr;
}

void cpu_set_cop0_register(int index, uint64_t value)
{
	switch (index)
	{
		case 6:		// wired
		case 9:		// count
		case 11:	// cause
		case 13:	// compare
			cpu.cop0.r[index] = value;
			break;

		// ignore cache stuff
		case 28:	// tag_lo
		case 29:	// tag_hi
			break;

		default:
			printf("Unsupported COP0 register write: %d\n", index);
			throw nullptr;
			break;
	}
}

bool logging_enabled{false};
static bool stepping{false};
static uint32_t breakpoint_address{0x80000000};
static uint64_t previous_gpr_state[32]{};

bool run_debugger()
{
	/*if (program_counter == breakpoint_address)
	{
		printf("breakpoint hit\n");
		stepping = true;
	}

	if (stepping)
	{
		auto get_line = []()->const char*
		{
			int ch{};
			static char line_buf[128]{};
			auto buf = line_buf;

			while ((ch = getchar()) != '\n')
			{
				*buf = ch;
				buf++;
			}

			*buf = '\0';

			return line_buf;
		};

		while (true)
		{
			printf("--> ");
			const auto* line = get_line();

			if (strcmp(line, "q") == 0)
			{
				return false;
			}
			else if (strcmp(line, "l") == 0)
			{
				logging_enabled = !logging_enabled;
				printf("logging %s\n", logging_enabled ? "enabled" : "disabled");
				continue;
			}
			else if (strcmp(line, "s") == 0)
			{
				break;
			}
			else if (strcmp(line, "c") == 0)
			{
				stepping = false;
				break;
			}
			else if (strcmp(line, "p") == 0)
			{
				for (int i = 0; i < 16; i++)
				{
					printf("\t%-2s: 0x%016llX - %-2s: 0x%016llX\n",
						parser_get_symbolic_gpr_name(i), cpu.gpr[i],
						parser_get_symbolic_gpr_name(i + 16), cpu.gpr[i + 16]
					);
				}
			}
		}
	}*/

	return false;
}

bool cpu_step()
{
	uint32_t opcode{};
	uint64_t program_counter{};

	// fill this execution with the delay slot address
	if (branch_delay_slot_address != 0)
	{
		program_counter = branch_delay_slot_address;
		branch_delay_slot_address = 0;
		cpu.pc -= 4; // predec the pc to account for the inc that will happen in function impl
	}
	else
	{
		program_counter = cpu.pc;
	}

	// reset r[0] to 0 every cycle
	// the r0 access member handles this but internal CPU functions
	// don't use the access members
	cpu.gpr[0] = 0;

	// random register
	cpu.cop0.random() = ((cpu.cop0.wired() + rand()) & 0x3F);

	// count register is incremented every other cycle
	cpu.cop0.count() += cycle_counter && (cycle_counter % 2) ? 1 : 0;

	memory_enable_logging(false);
	if (!memory_read32(program_counter, opcode))
	{
		printf("CPU: bad instruction memory read\n");
		return false;
	}
	memory_enable_logging(logging_enabled);

	const auto* op = disassembler_decode_instruction(opcode);
	ExecutionContext ctx{opcode};

	if ((program_counter & 0xFFFFFFFF) == 0x80000000)
	{
		logging_enabled = true;
		//stepping = true;
	}

	if (logging_enabled)
	{
		char parse_buffer[256]{};
		disassembler_parse_instruction(opcode, op, parse_buffer, 0);
		printf("0x%016llX: %08X: %s\n", program_counter, opcode, parse_buffer);
	}

	if (stepping)
		getchar();

	//run_debugger();

	if (op)
	{
		try {
			// catch memory exceptions
			op->func(ctx);
		} catch (const MemException& mem_ex)
		{
			printf("CPU: Memory Exception: %s\n", mem_ex.message);
			return false;
		}
	}
	else
	{
		printf("CPU: Unknown opcode: %08X\n", opcode);
		return false;
	}

	if (logging_enabled)
	{
		bool effected{};
		for (int i = 0; i < 32; i++)
		{
			if (cpu.gpr[i] != previous_gpr_state[i])
			{
				printf("\t$%s: 0x%016llX -> 0x%016llX\n", parser_get_symbolic_gpr_name(i), previous_gpr_state[i], cpu.gpr[i]);
				effected = true;
			}
		}

		if (effected)
			printf("\n");
	}

	memcpy(previous_gpr_state, cpu.gpr, 32 * 8);

	cycle_counter++;

	return true;
}
