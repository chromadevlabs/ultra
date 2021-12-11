
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

static bool logging_enabled{};
static std::vector<MemoryMapping> mmu_map;

void default_buffer_read(const void* buffer, uint32_t addr, uint32_t size, void* dst)
{
	memcpy((uint8_t*)dst, (const uint8_t*)buffer + addr, size);
}

void default_buffer_write(void* buffer, uint32_t addr, uint32_t size, const void* src)
{
	memcpy((uint8_t*)buffer + addr, (const uint8_t*)src, size);
}

void memory_init()
{
	mmu_map.clear();
}

std::vector<uint64_t> breakpoints;

void memory_add_breakpoint(uint64_t address)
{
	breakpoints.push_back(address);
}

void memory_remove_breakpoint(uint64_t address)
{
	breakpoints.erase(
		std::remove(breakpoints.begin(), breakpoints.end(), address),
		breakpoints.end()
	);
}

void memory_enable_logging(bool enabled)
{
	logging_enabled = enabled;
}

extern uint8_t cartridge_rom[MB(64)];

CartHeader* memory_get_rom_header()
{
	return (CartHeader*)cartridge_rom;
}

void memory_load_rom(const char* path, bool swap)
{
	if (auto* file = fopen(path, "rb"))
	{
		fread(cartridge_rom, 1, sizeof(cartridge_rom), file);

		if (swap)
		{
			for (int i = 0; i < sizeof(cartridge_rom); i += 4)
			{
				auto& u32 = *(uint32_t*)&cartridge_rom[i];
				u32 = bswap_32(u32);
			}
		}

		fclose(file);
	}
}

void memory_install_rw_callback(
	uint32_t start, uint32_t end, 
	std::function<void(uint32_t, uint32_t, void*)>&& read, 
	std::function<void(uint32_t, uint32_t, const void*)>&& write, 
	const char* name)
{
	MemoryMapping mr
	{
		{ start, end },
		name,
		std::move(read), std::move(write)
	};

	mmu_map.push_back(std::move(mr));
}

struct TLBEntry
{
	uint32_t entry_lo0;
	uint32_t entry_lo1;
	uint32_t page_mask;
	uint32_t entry_hi0;
};

static TLBEntry tlb_entries[64]{};

static bool memory_do_tlb_translation(uint32_t& virtual_address)
{
	//https://gist.github.com/parasyte/6547020
	for (const auto& entry : tlb_entries)
	{
		const auto mask = (entry.page_mask >> 1) & 0x0FFF;
		const auto page_size = mask + 1;
		const auto vpn = entry.entry_hi0 & ~(entry.page_mask & 0x1FFF);

		if ((virtual_address & vpn) == vpn)
		{
			uint32_t phys_frame_number{};

			const auto odd = virtual_address & page_size;
			const auto even = !odd;

			if (even)
			{
				if (!(entry.entry_lo0 & 0x02)) 
					continue;
				
				phys_frame_number = (entry.entry_lo0 >> 6) & 0x00FFFFFF;
			}
			else
			{
				if (!(entry.entry_lo1 & 0x02))
					continue;

				phys_frame_number = (entry.entry_lo1 >> 6) & 0x00FFFFFF;
			}

			auto phys_address = 
				(0x80000000 | (phys_frame_number * page_size) | (virtual_address & mask));

			virtual_address = phys_address;
			return true;
		}
	}

	return false;
}

enum class ReadWrite { Read, Write };
bool memory_map(ReadWrite rw, uint32_t address, uint32_t size, void* data)
{
	uint32_t virtual_address{address};
	bool valid{};

	switch (address)
	{
		// USEG  TLB mapped
		case 0x00000000 ... 0x7FFFFFFF:
		{
			//if (!memory_do_tlb_translation(address))
			{
			//	printf("\nUnsupported TLB access: USEG (0x%08X)\n", address);
			//	return false;
			}
		} break;

		// KSSEG  TLB mapped
		case 0xC0000000 ... 0xDFFFFFFF:
		{
			if (!memory_do_tlb_translation(address))
			{
				printf("\nUnsupported TLB access: KSSEG (0x%08X)\n", address);
				return false;
			}
		} break;
		
		// KSEG3  TLB mapped
		case 0xE0000000 ... 0xFFFFFFFF: 
		{
			if (!memory_do_tlb_translation(address))
			{
				printf("\nUnsupported TLB access: KSEG3 (0x%08X)\n", address);
				return false;
			}
		} break;

		// KSEG0  Direct map, cache
		case 0x80000000 ... 0x9FFFFFFF: 
			address -= 0x80000000;
			break;
		
		// KSEG1  Direct map, non-cache
		case 0xA0000000 ... 0xBFFFFFFF: 
			address -= 0xA0000000;
			break;
	}

	auto match = std::find_if(mmu_map.begin(), mmu_map.end(), [address](const auto& e)
	{
		return e.range.contains(address);
	});

	for (auto bp : breakpoints)
	{
		if (bp == address)
		{
			printf("hit breakpoint 0x%08X\n", address);
			getchar();
		}
	}

	if (match != mmu_map.end())
	{
		switch (rw)
		{
		case ReadWrite::Read:
			match->read(match->range.map(address), size, data);
			
			if (logging_enabled)
				printf("\t(%s) Read 0x%08X[0x%08X]\n\n", match->name, address, *(uint32_t*)data);

			valid = true;
			break;
		
		case ReadWrite::Write:
			match->write(match->range.map(address), size, data);
			
			if (logging_enabled)
				printf("\t(%s) Write 0x%08X[0x%08X]\n\n", match->name, address, *(uint32_t*)data);

			valid = true;
			break;
		}
	}

	if (valid)
	{
		return true;
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
		//value = bswap_16(value);
		return true;
	}

	return false;
}

bool memory_read32(uint32_t address, uint32_t& value)
{
	if (memory_read(address, 4, &value))
	{
		//value = bswap_32(value);
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
	//data = bswap_16(data);
	return memory_write(address, 2, &data);
}

bool memory_write32(uint32_t address, uint32_t data)
{
	//data = bswap_32(data);
	return memory_write(address, 4, &data);
}