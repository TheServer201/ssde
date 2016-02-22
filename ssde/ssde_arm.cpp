// SSDE implementation for ARM arch
// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#include "ssde_arm.hpp"
#include <vector>
#include <stdint.h>


using ssde::Inst_ARM;

void Inst_ARM::internal_decode(const std::vector<uint8_t>& buffer, CPU_state state)
{
	switch (state)
	{
	case CPU_state::arm:
		decode_as_arm(fetch(buffer, 4));
		break;

	case CPU_state::thumb:
		decode_as_thumb(fetch(buffer, 2));
		break;

	case CPU_state::thumb2:
		break;

	default:
		signal_error(Error::cpu_state);
		break;
	}
}

uint32_t Inst_ARM::fetch(const std::vector<uint8_t>& buffer, int32_t amount)
{
	uint32_t result = 0;

	for (int32_t i = 0; i < amount; ++i)
		result |= static_cast<uint32_t>(buffer.at(pc + i)) << i*8;

	length += amount;

	return result;
}

void Inst_ARM::decode_as_arm(uint32_t bc)
{

}

void Inst_ARM::decode_as_thumb(uint32_t bc)
{

}

// void Inst_ARM::decode_as_thumb2(const std::vector<uint8_t>& buffer)
// {
// 	uint32_t bc = fetch(buffer, 2);
//
// 	if ((bc & 0xe000) == 0xe000)
// 	{
// 		uint8_t op1 = (bc & 0x1800) >> 11;
// 		uint8_t op2 = (bc & 0x07f0) >> 4;
//
// 		if (op1 == 0x00)
// 		{
//
// 		}
// 		else
// 		{
// 			bc |= fetch(buffer, 2) << 16;
// 		}
// 	}
// 	else
// 	{
// 		signal_error();
// 	}
// }