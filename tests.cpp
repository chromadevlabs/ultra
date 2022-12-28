
#include "cpu.h"
#include "cpu_types.h"
#include "disassembler.h"

#include <cstring>
#include <functional>

// Mac OS X / Darwin features
#include <libkern/OSByteOrder.h>
#define bswap_16(x)             OSSwapInt16(x)
#define bswap_32(x)             OSSwapInt32(x)
#define bswap_64(x)             OSSwapInt64(x)

extern uint8_t rdram[MB(8)];

void cpu_test_reset()
{
    memset(&cpu, 0x00, sizeof(CPU));
}

bool cpu_execute_single(InstructionType type, ExecutionContext context, std::function<bool()>&& pred)
{
    if (auto* desc = disassembler_find_descriptor(type))
    {
        desc->func(context);
        return pred();
    }

    return false;
}