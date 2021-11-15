
#include "disassembler.h"
#include "magic_enum.hpp"

#include <cassert>
#include <capstone/capstone.h>

static csh cs{};

bool init_capstone()
{
	auto err{cs_open(CS_ARCH_MIPS, CS_MODE_MIPS32, &cs)};

	if (err != CS_ERR_OK)
	{
		printf("%s\n", cs_strerror(err));
		return false;
	}

	return true;
}

void shutdown_capstone()
{
	cs_close(&cs);
}

const char* cs_decode(uint32_t opcode)
{
	cs_insn* decoded_inst{};
	static char buf[32]{};

	if (auto count = cs_disasm(cs, (const uint8_t*)&opcode, sizeof(uint32_t), 0x0000, 1, &decoded_inst))
	{
		strcpy(buf, decoded_inst[0].mnemonic);

		for (auto& ch : buf)
		{
			if (!ch)
				break;

			ch = std::toupper(ch);
		}
		
		cs_free(decoded_inst, count);
		return buf;
	}

	return nullptr;	
}


void cpu_run_tests()
{
	disassembler_init();
	init_capstone();

	for (uint64_t i = 0; i < 0xFFFFFFFF; i++)
	{
		auto opcode = (uint32_t)i;
		auto* inst = disassembler_decode_instruction(opcode);
		const char* cs_inst = cs_decode(opcode);

		if (inst && cs_inst)
		{
			auto n1 = magic_enum::enum_name(inst->type).data();
			auto n2 = cs_inst;

			if (strcmp(n1, n2) != 0)
			{
				printf("'%08X' %s -> %s FAIL!\n", opcode, n1, n2);
				return;
			}
			else
			{
				printf("'%08X' %s -> %s OK!\n", opcode, n1, n2);
			}
		}
	}
}