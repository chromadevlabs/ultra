
#include "cpu.h"
#include "memory.h"

#include <cassert>

void cpu_run_tests()
{
	cpu_init();

	//memory_do_dma(0xA4000000, 0xB0000000, 0x1000);

	memory_write32(0xA4000040, 0x00000000);
	cpu_step();

	
}