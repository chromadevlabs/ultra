#pragma once

#include <cstdint>
#include <functional>

void set_log_callback(std::function<void(const char*)>&&);

void cpu_init();
void cpu_set_pc(uint64_t pc);
bool cpu_step();

uint64_t& cpu_get_gp_register(int index);