#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct CartHeader
{
    // fuck knows what these do
    uint8_t PI_BSB_DOM1_LAT_REG;
    uint8_t PI_BSB_DOM1_PGS_REG;
    uint8_t PI_BSB_DOM1_PWD_REG;
    uint8_t PI_BSB_DOM1_PGS_REG2;

    // nope
    uint32_t clock;

    // program counter
    uint32_t pc;

    // nope
    uint32_t release;

    // checksum
    uint32_t crc1;
    uint32_t crc2;

    uint64_t unknown;

    // duh
    char name[20];
    uint32_t manufacturerID;
    uint16_t cartID;
    uint16_t countryID;
};
#pragma pack(pop)