#pragma once

#include <cstdint>
#include <functional>

#include "cartridge.h"

void memory_init();

void memory_install_rw_callback(
	uint32_t start, uint32_t end, 
	std::function<void(uint32_t, uint32_t, void*)>&& read, 
	std::function<void(uint32_t, uint32_t, const void*)>&& write, 
	const char* name
);

bool memory_read(uint32_t address, uint32_t size, void* data);
bool memory_write(uint32_t address, uint32_t size, const void* data);
bool memory_do_dma(uint32_t dst, uint32_t src, uint32_t size);

bool memory_read8(uint32_t address, uint8_t&);
bool memory_read16(uint32_t address, uint16_t&);
bool memory_read32(uint32_t address, uint32_t&);

bool memory_write8(uint32_t address, uint8_t data);
bool memory_write16(uint32_t address, uint16_t data);
bool memory_write32(uint32_t address, uint32_t data);

void memory_load_rom(const char* path);
CartHeader* memory_get_rom_header();

void default_buffer_read(const void* buffer, uint32_t addr, uint32_t size, void* dst);
void default_buffer_write(void* buffer, uint32_t addr, uint32_t size, const void* src);