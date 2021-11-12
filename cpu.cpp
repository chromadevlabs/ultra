
#include "cpu.h"

#include "magic_enum.hpp"
#include "memory.h"
#include "platform.h"
#include "cpu_types.h"
#include "cartridge.h"

#include <vector>
#include <cstring>

// Implemented in parser.cpp
const char* parser_get_symbolic_gpr_name(int i);
const char* parser_get_symbolic_cop0_name(int i);

char log_buffer[512]{};
static std::function<void(const char*)> log_callback;

void set_log_callback(std::function<void(const char*)>&& callback)
{
	log_callback = std::move(callback);
}

void print_log(const char* message)
{
	log_callback(message);
}

struct RSP
{
	uint8_t dmem[0xFFF]{};
	uint8_t imem[0xFFF]{};
} rsp{};

CPU cpu{};

std::vector<EncodingDescriptor> encodings;
uint64_t branch_delay_slot_address{};
uint64_t cycle_counter{};

static uint8_t rdram[MB(8)];
static uint8_t pif_ram[0x3F];
uint8_t cartridge_rom[MB(64)];

static MemoryMappedRegister<uint32_t> RI_MODE_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_CONFIG_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_CURRENT_LOAD_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_SELECT_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_REFRESH_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_LATENCY_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_RERROR_REG = { [](uint32_t& value, bool write){}};
static MemoryMappedRegister<uint32_t> RI_WERROR_REG = { [](uint32_t& value, bool write){}};

static MemoryMappedRegister<uint32_t> MI_INIT_MODE_REG = {
	[](uint32_t& value, bool write)
	{
		printf("TODO: MI_INIT_MODE_REG");
	}
};

static MemoryMappedRegister<uint32_t> MI_VERSION_REG = {
	[](uint32_t& value, bool write)
	{
		// version register
		value = 0x00000000;
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

void cpu_init()
{
	memory_init();

	// main system ram (with expansion pack)
	memory_install_rw_callback(
		0x00000000, 0x007FFFFF,
		std::bind(default_buffer_read, rdram, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
		std::bind(default_buffer_write, rdram, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
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
		std::bind(default_buffer_write, cartridge_rom, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
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

	// reset cpu
	cpu.pc = 0;
	cpu.hi = 0;
	cpu.lo = 0;
	cpu.fcr[0] = 0;
	cpu.fcr[1] = 0;
	cpu.ll = false;

	/*******************************************************/
	// PIF emulation
	memory_write32(0xA4001000 + 0, 0x3c0dbfc0);
	memory_write32(0xA4001000 + 4, 0x8da807fc);
	memory_write32(0xA4001000 + 8, 0x25ad07c0);
	memory_write32(0xA4001000 + 12, 0x31080080);
	memory_write32(0xA4001000 + 16, 0x5500fffc);
	memory_write32(0xA4001000 + 20, 0x3c0dbfc0);
	memory_write32(0xA4001000 + 24, 0x8da80024);
	memory_write32(0xA4001000 + 28, 0x3c0bb000);
	
	cpu.t3() = 0xFFFFFFFFA4000040;
	cpu.s4() = 0x0000000000000001;
	cpu.s6() = 0x000000000000003F;
	cpu.sp() = 0xFFFFFFFFA4001FF0;

	cpu.cop0[0] = 0;
	cpu.cop0[1] = 0x0000001F; 		// random
	cpu.cop0[12] = 0x70400004;		// status
	cpu.cop0[15] = 0x00000B00;		// PRId
	cpu.cop0[16] = 0x0006E463;		// Config

	memory_write32(0x04300004, 0x01010101);
	/*******************************************************/

	printf("\n");

	memory_load_rom("/Users/chroma/Desktop/ultra/sm64.z64");

	printf("Copying rom code to rdram...");
	memory_do_dma(0xA4000000, 0xB0000000, 0x1000);
	printf("OK!\n");

	cpu.pc = 0xA4000040;

	//freopen("output.txt", "wa", stdout);
}

bool logging_enabled{true};

void cpu_link(uint64_t address)
{
	cpu.gpr[31] = address;
}

void cpu_trap(const char*)
{
	debug_log("!!!!!!!!TRAP!!!!!!!!!!\n");
	throw nullptr;
}

void print_debugger_instruction(uint32_t opcode, const EncodingDescriptor* ed, ExecutionContext& ctx)
{
	if (ed)
	{
		char state_buffer[512]{};
		char* sbuf = state_buffer;

		debug_log("0x%016llX: %08X:\t%-6s ", cpu.pc, opcode, magic_enum::enum_name(ed->type).data());

		auto* str = ed->debug_format;

		auto find = [](const char* s1, const char* s2)
		{
			while (*s2)
			{
				if (*s1 != *s2)
					return false;

				s1++;
				s2++;
			}

			return true;
		};

		while (*str)
		{
			if (find(str, "RTI"))
			{
				debug_log("%d", (uint8_t)GET_RT_BITS(opcode));
				str += 3;
			}
			else if (find(str, "COP_RD"))
			{
				debug_log("%s", parser_get_symbolic_cop0_name(GET_RD_BITS(opcode)));		
				str += 6;
			}
			else if (find(str, "RS"))
			{
				auto index = GET_RS_BITS(opcode);
				debug_log("%s", parser_get_symbolic_gpr_name(index));
				sbuf += sprintf(sbuf, "0x%08X", (uint32_t)cpu.gpr[index]);
				str += 2;
			}
			else if (find(str, "RT"))
			{
				auto index = GET_RT_BITS(opcode);
				debug_log("%s", parser_get_symbolic_gpr_name(index));
				sbuf += sprintf(sbuf, "0x%08X", (uint32_t)cpu.gpr[index]);
				str += 2;
			}
			else if (find(str, "RD"))
			{
				auto index = GET_RD_BITS(opcode);
				debug_log("%s", parser_get_symbolic_gpr_name(index));
				sbuf += sprintf(sbuf, "0x%08X", (uint32_t)cpu.gpr[index]);
				str += 2;
			}
			else if (find(str, "IMM"))
			{
				auto index = GET_IMM_BITS(opcode);
				debug_log("0x%04X", (uint16_t)index);
				sbuf += sprintf(sbuf, "0x%08X", (uint32_t)index);
				str += 3;
			}
			else if (find(str, "SA"))
			{
				auto index = GET_IMM_BITS(opcode);
				debug_log("%d", GET_SHIFT_BITS(opcode));
				sbuf += sprintf(sbuf, "%d", GET_SHIFT_BITS(opcode));
				str += 2;
			}
			else if (find(str, "OFFSET"))
			{
				debug_log("%d", (int16_t)GET_IMM_BITS(opcode));
				sbuf += sprintf(sbuf, "%d", (int16_t)GET_IMM_BITS(opcode));
				str += 6;
			}
			else if (find(str, "TARGET"))
			{
				auto address = (cpu.pc & 0xF0000000) + (GET_JMP_BITS(opcode) << 2);
				debug_log("0x%08X", address);
				sbuf += sprintf(sbuf, "0x%016llX", address);
				str += 6;
			}
			else
			{
				debug_log("%c", *str);
				sbuf += sprintf(sbuf, "%c", *str);
				str++;
			}
		}

		debug_log("\t%-35s", state_buffer);
	}
	else
	{
		debug_log("0x%016llX: %08X: \n", cpu.pc, opcode);
	}
}

uint64_t& cpu_get_gp_register(int index)
{
	return cpu.gpr[index];
}

bool cpu_step()
{
	uint32_t opcode{};
	uint64_t program_counter{};

	auto parse_opcode = [](uint32_t opcode)->const EncodingDescriptor*
	{
		for (const auto& desc : encodings)
		{
			if (desc.match(opcode))
				return &desc;
		}

		return nullptr;
	};

	// execute the delay branch slot if filled
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

	// shit loop detection to stop log spam
	static const auto address_history_size{8};
	static uint64_t addresses[address_history_size]{};
	static int loop_counter{};
	static uint64_t pre_loop_gpr_state[32]{};
	bool loop_detected{false};
	for (int i = 0; i < address_history_size-1; i++)
		addresses[i] = addresses[i + 1];

	for (int i = 0; i < address_history_size-1; i++)
	{
		if (addresses[i] == program_counter)
		{
			loop_detected = true;
			break;
		}
	}

	addresses[address_history_size - 1] = program_counter;

	if (logging_enabled && loop_detected)
	{
		// loop enter, store cpu state to compare later
		debug_log("Loop enter @ 0x%016llX\n", program_counter);
		memcpy(pre_loop_gpr_state, cpu.gpr, 32 * sizeof(uint64_t));
		logging_enabled = false;
	}
	else if (logging_enabled == false && loop_detected == false)
	{
		// loop exit
		logging_enabled = true;
		debug_log("Loop ran %d times\n", loop_counter);

		debug_log("Effected registers\n");
		for (int i = 0; i < 32; i++)
		{
			if (pre_loop_gpr_state[i] != cpu.gpr[i])
				debug_log("\t%s [0x%016llX] -> [0x%016llX]\n", parser_get_symbolic_gpr_name(i), pre_loop_gpr_state[i], cpu.gpr[i]);
		}
	}

	// increment or clear the loop_counter
	loop_counter = loop_detected ? loop_counter + 1 : 0;

	if (loop_counter > 1 * 1000 * 1000)
	{
		logging_enabled = true;
		printf("loop exceeded 1 million cycles\n");
		
		debug_log("Effected registers\n");
		for (int i = 0; i < 32; i++)
		{
			if (pre_loop_gpr_state[i] != cpu.gpr[i])
				debug_log("\t%s [0x%016llX] -> [0x%016llX]\n", parser_get_symbolic_gpr_name(i), pre_loop_gpr_state[i], cpu.gpr[i]);
		}
		
		printf("\n");
		fflush(stdout);
		throw nullptr;
	}
	
	// zero register should always be zero
	cpu.v0() = 0;

	// random register
	cpu.random() = ((cpu.wired() + rand()) & 0x3F);

	// count register is incremented every other cycle
	cpu.count() = cycle_counter % 2 ? cpu.count() + 1 : 0;

	try {
		opcode = bswap_32(memory_read32(program_counter));
	} catch (void*)
	{
		debug_log("\n");
		fflush(stdout);
		return false;
	}

	const auto* op = parse_opcode(opcode);
	ExecutionContext ctx{opcode};

	print_debugger_instruction(opcode, op, ctx);

	if (op)
	{
		try {
			op->func(ctx);
		} catch (const void*)
		{
			printf("\n");
			fflush(stdout);
			return false;
		}
	}
	else
	{
		printf("\n");
		printf("\n!!!!!!!!!!!!!!!!unimplemented opcode!!!!!!!!!!!!!!!!\n");
		fflush(stdout);
		return false;
	}

	debug_log("\n");

	cycle_counter++;

	return true;
}
