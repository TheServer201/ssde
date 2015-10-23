/*
* The SSDE implementation for ARM instruction set.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#include "ssde_arm.hpp"

#include <string>

#include <stdint.h>


bool ssde_arm::dec()
{
	if (pc >= buffer.length())
		return false;

	reset_fields();


	uint32_t bc = fetch();

	switch (state)
	{
	case cpu_state::arm:
		{
			condition = (bc & 0xf0000000) >> 28;

			if ((bc & 0x0c000000) == 0x00000000)
				/* data processing / psr transfer */
			{

			}
			else if ((bc & 0x0fc000f0) == 0x00000090)
				/* multiply */
			{

			}
			else if ((bc & 0x0f8000f0) == 0x00800090)
				/* long multiply (v3M/v4 only) */
			{

			}
			else if ((bc & 0x0fb00ff0) == 0x01000090)
				/* swap */
			{

			}
			else if ((bc & 0x0c000000) == 0x04000000)
				/* load/store byte/word */
			{

			}
			else if ((bc & 0x0e000000) == 0x08000000)
				/* load/store multiple */
			{

			}
			else if ((bc & 0x0e400090) == 0x00400090)
				/* halfword transfer: immediate offset (v4 only) */
			{

			}
			else if ((bc & 0x0e400f90) == 0x00000090)
				/* halfword transfer: register offset (v4 only) */
			{

			}
			else if ((bc & 0x0e000000) == 0x0a000000)
				/* branch */
			{
				is_branch = true;

				if (bc & 0x01000000)
					has_link = true;

				/* extract & unpack offset */
				rel = (bc & 0x00ffffff) << 2;

				if (rel & 0x02000000)
					/* sign extend rel if needed */
				{
					rel |= 0xfc000000;
				}

				abs = (uint32_t)pc + length + rel;
			}
			else if ((bc & 0x0ffffff0) == 0x012fff10)
				/* branch exchange (v4T only) */
			{

			}
			else if ((bc & 0xc000000) == 0xc000000)
				/* coprocessor data transfer */
			{

			}
			else if ((bc & 0xf000010) == 0xe000010)
				/* coprocessor register transfer */
			{

			}
			else if ((bc & 0xf000000) == 0xf000000)
				/* software interrupt */
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

	default:
		{
			error = true;
			error_cpu_state = true;

			length = 0;
		}
	}

	return true;
}

/* resets fields before new iteration of the instruction decoder */
void ssde_arm::reset_fields()
{
	length = 0;

	error          = false;
	error_opcode   = false;
	error_operand  = false;
	error_length   = false;
	error_misalign = false;

	condition = 0;

	is_branch = false;
	has_link  = false;
	rel       = 0;
	abs       = 0;
}

/* performs a fetch (4 or 2 bytes) for instriction decoder */
uint32_t ssde_arm::fetch()
{
	uint32_t bc = 0;

	switch (state)
	{
	case cpu_state::arm:
	case cpu_state::thumb2:
		{
			for (int i = 0; i < 4; ++i)
				bc |= (uint8_t)buffer[pc + length + i] << i*8;

			if (pc & 3)
				error_misalign = true;

			length = 4;

			break;
		}

	case cpu_state::thumb:
		{
			for (int i = 0; i < 2; ++i)
				bc |= (uint8_t)buffer[pc + length + i] << i*8;

			if (pc & 1)
				error_misalign = true;

			length = 2;

			break;
		}

	default:
		break;
	}

	return bc;
}