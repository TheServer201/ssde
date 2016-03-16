// SSDE implementation for ARM arch
// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#include "ssde_arm.hpp"
#include <cstdint>
#include <vector>


using ssde::Inst_ARM;
using std::vector;
using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;


void Inst_ARM::internal_decode(const vector<uint8_t>& buffer,
                               CPU_state state)
{
	switch (state)
	{
	case CPU_state::arm:
		decode_as_arm(fetch(buffer, 4));
		break;

	case CPU_state::thumb:
		break;

	default:
		signal_error(Error::cpu_state);
		break;
	}
}

void Inst_ARM::decode_as_arm(uint32_t bc)
{
	if ((bc & 0x0c000000) == 0x00000000)
	{
		// Data processing / PSR Transfer
	}
	else if ((bc & 0x0fc000f0) == 0x00000090)
	{
		// Multiply
	}
	else if ((bc & 0x0f8000f0) == 0x00800090)
	{
		// Long Multiply (v3M / v4 only)
	}
	else if ((bc & 0x0fb00ff0) == 0x01000090)
	{
		// Swap
	}
	else if ((bc & 0x0c000000) == 0x04000000)
	{
		// Load/Store Byte/Word
	}
	else if ((bc & 0x0e000000) == 0x08000000)
	{
		// Load/Store Multiple
	}
	else if ((bc & 0x0e000090) == 0x00000090)
	{
		// Halfword transfer: Immediate offset (v4 only)
	}
	else if ((bc & 0x0e000f90) == 0x00000090)
	{
		// Halfword transfer: Immediate offset (v4 only)
	}
	else if ((bc & 0x0e000000) == 0x0a000000)
	{
		// Branch
	}
	else if ((bc & 0x0ffffff0) == 0x012fff10)
	{
		// Branch Exchange
	}
	else if ((bc & 0x0e000000) == 0x0c000000)
	{
		// Coprocessor data transfer
	}
	else if ((bc & 0x0f000010) == 0x0e000000)
	{
		// Coprocessor data operation
	}
	else if ((bc & 0x0f000010) == 0x0e000010)
	{
		// Coprocessor data register transfer
	}
	else if ((bc & 0x0f000000) == 0x0f000000)
	{
		// Software interrupt
	}
	else
	{
		signal_error(Error::opcode);
	}
}

// void Inst_ARM::decode_as_thumb(const std::vector<uint8_t>& buffer)
// {
// }