// SSDE implementation for ARM arch
// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#include "ssde.hpp"
#include "ssde_arm.hpp"

#include <string>
#include <stdint.h>


using ssde::Inst_arm;

void Inst_arm::internal_decode(const std::string& buffer)
{
	// bc = fetch(buffer);

	switch (state)
	{
	case CPU_state::arm:
		//decode_arm(buffer);
		break;

	default:
		break;
	}
}

#if 0
uint32_t Inst_arm::fetch(const std::string& buffer)
{
	uint32_t bc = 0;

	switch (state)
	{
	case CPU_state::arm:
	case CPU_state::thumb2:
		{
			for (int i = 0; i < 4; ++i)
				bc |= (uint8_t)buffer.at(pc + i) << i*8;

			length = 4;

			if (pc & 3)
			{
				error = true;
				error_alignment = true;
			}
		}
		break;

	case CPU_state::thumb:
		{
			for (int i = 0; i < 2; ++i)
				bc |= (uint8_t)buffer.at(pc + i) << i*8;

			length = 2;

			if (pc & 1)
			{
				error = true;
				error_alignment = true;
			}
		}
		break;

	default:
		length = state == CPU_state::thumb ? 2 : 4;
		error = true;
		error_cpu_state = true;
		break;
	}

	return bc;
}

void Inst_arm::decode_arm(const std::string& buffer)
{
	cond = (Exec_cond)((bc & 0xf0000000) >> 28);

	if ((bc & 0x0c000000) == 0x00000000)
		// data processing / psr transfer
	{

	}
	else if ((bc & 0x0fc000f0) == 0x00000090)
		// multiply
	{

	}
	else if ((bc & 0x0f8000f0) == 0x00800090)
		// long multiply (v3M/v4 only)
	{

	}
	else if ((bc & 0x0fb00ff0) == 0x01000090)
		// swap
	{

	}
	else if ((bc & 0x0c000000) == 0x04000000)
		// load/store byte/word
	{

	}
	else if ((bc & 0x0e000000) == 0x08000000)
		// load/store multiple
	{

	}
	else if ((bc & 0x0e400090) == 0x00400090)
		// halfword transfer: immediate offset (v4 only)
	{

	}
	else if ((bc & 0x0e400f90) == 0x00000090)
		// halfword transfer: register offset (v4 only)
	{

	}
	else if ((bc & 0x0e000000) == 0x0a000000)
		// branch
	{
		is_branch = true;

		if (bc & 0x01000000)
			is_branch_link = true;

		// extract & unpack offset
		rel = (bc & 0x00ffffff) << 2;

		if (rel & 0x02000000)
			// sign extend rel if needed
		{
			rel |= 0xfc000000;
		}

		abs = (uint32_t)pc + 4 + rel;
	}
	else if ((bc & 0x0ffffff0) == 0x012fff10)
		// branch exchange (v4T only)
	{

	}
	else if ((bc & 0xc000000) == 0xc000000)
		// coprocessor data transfer
	{

	}
	else if ((bc & 0xf000010) == 0xe000010)
		// coprocessor register transfer
	{

	}
	else if ((bc & 0xf000000) == 0xf000000)
		// software interrupt
	{
		is_swi = true;
		swi_data = bc & 0x00ffffff;
	}
	else
	{
		error = true;
		error_opcode = true;
	}
}
#endif