#pragma once

#include <cstdint>

void cpu_init();
void cpu_set_pc(uint64_t pc);
bool cpu_step();

void cpu_run_tests();