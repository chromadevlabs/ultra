
#include "cpu.h"
#include "memory.h"
#include "magic_enum.hpp"
#include "disassembler.h"

#include <thread>

// Implemented in parser.cpp
const char* parser_get_symbolic_gpr_name(int i);
const char* parser_get_symbolic_cop0_name(int i);

int main(int, const char**)
{
	printf("Ultra alpha v0.1\n");

	cpu_run_tests();

	//freopen("../output.txt", "w", stdout);

	//cpu_init();

	//while (cpu_step())
	{
		//std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}