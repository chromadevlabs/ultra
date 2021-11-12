

/*#define loop(var, length) for (int var = 0; var < length; var++)

void print_bits32(uint32_t bits)
{
	for (int i = 0; i < 32; i++)
	{
		auto b = bits >> (31 - i) & 1;
		printf("%c", b ? '1' : '0');
	}
}

void test_reg_op(InstructionType type, uint8_t opcode, int shift, int func)
{
	loop(rs, 32)
	{
		loop(rt, 32)
		{
			loop(rd, 32)
			{
				const uint32_t op = 
					SET_INST_BITS(opcode) 
					| SET_RD_BITS(rd) 
					| SET_RS_BITS(rs) 
					| SET_RT_BITS(rt)
					| SET_SHIFT_BITS(shift)
					| SET_FUNC_BITS(func);

				printf("%s: rd%d rs%d rt%d (%08X)\n", InstructionString[(int)type], rd, rs, rt, op);

				printf("\toooooossssstttttdddddaaaaaffffff\n");
				printf("\t");
				print_bits32(op);

				auto parsed = parseOpcode(op);
				if (parsed == type)
				{
					printf(" - OK!\n");
				}
				else
				{
					printf(" - FAIL! Got %s\n", InstructionString[(int)parsed]);
					throw nullptr;
				}
			}
		}
	}
}

void test_imm_op(InstructionType type, uint8_t opcode, uint16_t imm)
{
	loop(rs, 32)
	{
		loop(rt, 32)
		{
			const uint32_t op = 
				SET_INST_BITS(opcode) 
				| SET_RS_BITS(rs) 
				| SET_RT_BITS(rt)
				| SET_IMM_BITS(imm);

			printf("%s: rd%d rs%d imm%d (%08X)\n", InstructionString[(int)type], rs, rt, imm, op);

			printf("\toooooossssstttttiiiiiiiiiiiiiiii\n");
			printf("\t");
			print_bits32(op);

			auto parsed = parseOpcode(op);
			if (parsed == type)
			{
				printf(" - OK!\n");
			}
			else
			{
				printf(" - FAIL! Got %s\n", InstructionString[(int)parsed]);
				throw nullptr;
			}
		}
	}	
}

void test_load_store_op(InstructionType type, uint8_t opcode, uint16_t offset)
{
	loop(rs, 32)
	{
		loop(rt, 32)
		{
			const uint32_t op = 
				SET_INST_BITS(opcode) 
				| SET_RS_BITS(rs) 
				| SET_RT_BITS(rt)
				| SET_OFFSET_BITS(offset);

			printf("%s: rd%d rs%d imm%d (%08X)\n", InstructionString[(int)type], rs, rt, offset, op);

			printf("\toooooossssstttttaaaaaaaaaaaaaaaa\n");
			printf("\t");
			print_bits32(op);

			auto parsed = parseOpcode(op);
			if (parsed == type)
			{
				printf(" - OK!\n");
			}
			else
			{
				printf(" - FAIL! Got %s\n", InstructionString[(int)parsed]);
				throw nullptr;
			}
		}
	}	
}*/

void test_cpu_opcode_parser()
{
	/*
	test_reg_op(InstructionType::ADD, 	0b000000, 0b00000, 0b100000);
	test_reg_op(InstructionType::AND, 	0b000000, 0b00000, 0b100100);
	test_reg_op(InstructionType::NOR, 	0b000000, 0b00000, 0b100111);
	test_reg_op(InstructionType::OR, 	0b000000, 0b00000, 0b100101);
	test_reg_op(InstructionType::SLT,	0b000000, 0b00000, 0b101010);
	test_reg_op(InstructionType::SLTU,	0b000000, 0b00000, 0b101011);
	test_reg_op(InstructionType::SUB, 	0b000000, 0b00000, 0b100010);
	test_reg_op(InstructionType::SUBU, 	0b000000, 0b00000, 0b100011);

	test_reg_op(InstructionType::SLLV, 	0b000000, 0b00000, 0b000100);
	test_reg_op(InstructionType::SRAV, 	0b000000, 0b00000, 0b000111);
	test_reg_op(InstructionType::SRLV, 	0b000000, 0b00000, 0b000110);

	test_imm_op(InstructionType::ADDI, 0b001000, 0xf0f0);
	test_imm_op(InstructionType::ADDIU, 0b001001, 0xf0f0);
	test_imm_op(InstructionType::ANDI, 0b001100, 0xf0f0);
	test_imm_op(InstructionType::LUI, 0b001111, 0xf0f0);
	test_imm_op(InstructionType::ORI, 0b001101, 0xf0f0);
	test_imm_op(InstructionType::SLTI, 0b001010, 0xf0f0);
	test_imm_op(InstructionType::XORI, 0b001110, 0xf0f0);

	test_load_store_op(InstructionType::LB, 0b100000, 0x123456);
	test_load_store_op(InstructionType::LBU, 0b100100, 0x123456);
	test_load_store_op(InstructionType::LH, 0b100001, 0x123456);
	test_load_store_op(InstructionType::LHU, 0b100101, 0x123456);
	test_load_store_op(InstructionType::LW, 0b100011, 0x123456);
	test_load_store_op(InstructionType::SB, 0b101000, 0x123456);
	test_load_store_op(InstructionType::SH, 0b101001, 0x123456);
	test_load_store_op(InstructionType::SW, 0b101011, 0x123456);
	*/

}