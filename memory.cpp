
#include "memory.h"

#include "platform.h"

#include <functional>
#include <cstring>
#include <vector>

struct Range
{
	uint32_t begin;
	uint32_t end;

	constexpr Range(uint32_t _begin, uint32_t _end) :
		begin(_begin), end(_end)
	{
	}

	constexpr uint32_t size() const { return end - begin; }

	constexpr bool contains(uint32_t value) const
	{
		return value >= begin && value <= end;
	}

	constexpr uint32_t map(uint32_t value) const
	{
		return value - begin;
	}
};

struct MemoryMapping
{
	Range range;
	const char* name;
	std::function<void(uint32_t, uint32_t, void*)> read;
	std::function<void(uint32_t, uint32_t, const void*)> write;
};

static std::vector<MemoryMapping> mmu_map;

void default_buffer_read(const void* buffer, uint32_t addr, uint32_t size, void* dst)
{
	memcpy((uint8_t*)dst, (const uint8_t*)buffer + addr, size);
}

void default_buffer_write(void* buffer, uint32_t addr, uint32_t size, const void* src)
{
	memcpy((uint8_t*)buffer + addr, (const uint8_t*)src, size);
}

/*

0x00000000	0x003FFFFF	RDRAM	RDRAM - built in
0x00400000	0x007FFFFF	RDRAM	RDRAM - expansion pak (available if inserted)
0x00800000	0x03EFFFFF	Unused	Unused
0x03F00000	0x03FFFFFF	RDRAM Registers	RDRAM MMIO, configures timings, etc. Irrelevant for emulation
0x04000000	0x04000FFF	SP DMEM	RSP Data Memory
0x04001000	0x04001FFF	SP IMEM	RSP Instruction Memory
0x04002000	0x0403FFFF	Unused	Unused
0x04040000	0x040FFFFF	SP Registers	Control RSP DMA engine, status, program counter
0x04100000	0x041FFFFF	DP Command Registers	Send commands to the RDP
0x04200000	0x042FFFFF	DP Span Registers	Unknown
0x04300000	0x043FFFFF	MIPS Interface (MI)	System information, interrupts.
0x04400000	0x044FFFFF	Video Interface (VI)	Screen resolution, framebuffer settings
0x04500000	0x045FFFFF	Audio Interface (AI)	Control the audio subsystem
0x04600000	0x046FFFFF	Peripheral Interface (PI)	Control the cartridge interface. Set up DMAs cart <==> RDRAM
0x04700000	0x047FFFFF	RDRAM Interface (RI)	Control RDRAM settings (timings?) Irrelevant for emulation.
0x04800000	0x048FFFFF	Serial Interface (SI)	Control PIF RAM <==> RDRAM DMA engine
0x04900000	0x04FFFFFF	Unused	Unused
0x05000000	0x05FFFFFF	Cartridge Domain 2 Address 1	N64DD control registers - returns open bus (or all 0xFF) when not present
0x06000000	0x07FFFFFF	Cartridge Domain 1 Address 1	N64DD IPL ROM - returns open bus (or all 0xFF) when not present
0x08000000	0x0FFFFFFF	Cartridge Domain 2 Address 2	SRAM is mapped here
0x10000000	0x1FBFFFFF	Cartridge Domain 1 Address 2	ROM is mapped here
0x1FC00000	0x1FC007BF	PIF Boot Rom	First code run on boot. Baked into hardware.
0x1FC007C0	0x1FC007FF	PIF RAM	Used to communicate with PIF chip (controllers, memory cards)
0x1FC00800	0x1FCFFFFF	Reserved	 
0x1FD00000	0x7FFFFFFF	Cartridge Domain 1 Address 3
0x80000000	0xFFFFFFFF	Unknown	Unknown

*/

void memory_init()
{
	mmu_map.clear();
}

extern uint8_t cartridge_rom[MB(64)];

void memory_load_rom(const char* path)
{
	if (auto* file = fopen(path, "rb"))
	{
		fread(cartridge_rom, 1, sizeof(cartridge_rom), file);
		fclose(file);
	}
}

CartHeader* memory_get_rom_header()
{
	return (CartHeader*)cartridge_rom;
}

void memory_install_rw_callback(
	uint32_t start, uint32_t end, 
	std::function<void(uint32_t, uint32_t, void*)>&& read, 
	std::function<void(uint32_t, uint32_t, const void*)>&& write, 
	const char* name)
{
	MemoryMapping mr{
		{ start, end },
		name,
		std::move(read), std::move(write)
	};

	mmu_map.push_back(std::move(mr));
}

enum class ReadWrite { Read, Write };
bool memory_map(ReadWrite rw, uint32_t address, uint32_t size, void* data)
{
	switch (address)
	{
		// KUSEG  TLB mapping
		case 0x00000000 ... 0x7FFFFFFF:
			// user mode segment, all good!
			// TODO: confirm we're in user mode
			//printf("Unsupported TLB access: KUSEG (0x%08X)\n", address);
			//throw nullptr;
			break;

		// KSEG0  Direct map, cache
		case 0x80000000 ... 0x9FFFFFFF: 
			address -= 0x80000000;
			break;
		
		// KSEG1  Direct map, non-cache
		case 0xA0000000 ... 0xBFFFFFFF: 
			address -= 0xA0000000;
			break;
		
		// KSSEG  TLB mapping
		case 0xC0000000 ... 0xDFFFFFFF: 
			printf("\nUnsupported TLB access: KSSEG (0x%08X)\n", address);
			return false;
			break;
		
		// KSEG3  TLB mapping
		case 0xE0000000 ... 0xFFFFFFFF: 
			address -= 0xE0000000;
			//printf("\nUnsupported TLB access: KSEG3 (0x%08X)\n", address);
			//throw nullptr;
			break;
	}

	for (const auto& entry : mmu_map)
	{
		if (entry.range.contains(address))
		{
			/*if (!entry.range.contains(address + size))
			{
				printf("Memory access across bounds: 0x%08X::0x%08X (%s)\n", address, size, entry.name);
				
				throw nullptr;
			}*/

			switch (rw)
			{
			case ReadWrite::Read:
				entry.read(entry.range.map(address), size, data);
				return true;
				break;
			
			case ReadWrite::Write:
				entry.write(entry.range.map(address), size, data);
				return true;
			}
		}
	}

	printf("\nUnmapped memory access: 0x%08X::0x%08X\n", address, size);
	return false;
}

bool memory_read(uint32_t address, uint32_t size, void* data)
{
	return memory_map(ReadWrite::Read, address, size, data);
}

bool memory_write(uint32_t address, uint32_t size, const void* data)
{
	return memory_map(ReadWrite::Write, address, size, (void*)data);
}

bool memory_do_dma(uint32_t dst, uint32_t src, uint32_t size)
{
	printf("\nmemory_do_dma 0x%08X -> 0x%08X::0x%X\n", src, dst, size);

	for (uint32_t i = 0; i < size; i++)
	{
		uint8_t b{};

		if (!memory_read8(src, b))
			return false;
			
		if (!memory_write8(dst, b))
			return false;

		dst++;
		src++;
	}

	return true;
}

bool memory_read8(uint32_t address, uint8_t& value)
{
	return memory_read(address, 1, &value);
}

bool memory_read16(uint32_t address, uint16_t& value)
{
	if (memory_read(address, 2, &value))
	{
		value = bswap_16(value);
		return true;
	}

	return false;
}

bool memory_read32(uint32_t address, uint32_t& value)
{
	if (memory_read(address, 4, &value))
	{
		value = bswap_32(value);
		return true;
	}

	return false;
}

bool memory_write8(uint32_t address, uint8_t data)
{
	return memory_write(address, 1, &data);
}

bool memory_write16(uint32_t address, uint16_t data)
{
	data = bswap_16(data);
	return memory_write(address, 2, &data);
}

bool memory_write32(uint32_t address, uint32_t data)
{
	data = bswap_32(data);
	return memory_write(address, 4, &data);
}