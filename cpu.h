#pragma once

#include <cstdint>

void cpu_init();
void cpu_set_pc(uint64_t pc);
bool cpu_step();

uint64_t& cpu_get_gp_register(int index);
uint64_t& cpu_get_cp0_register(int index);

uint64_t& cpu_get_pc_register();
uint32_t& cpu_get_lo_register();
uint32_t& cpu_get_hi_register();

uint64_t cpu_get_next_instruction_address();
