/*
* The SSDE header file for ssde_arm.cpp
* Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#pragma once

#include "ssde.hpp"


/*
* A note on ssde_arm:
* Unlike real ARM CPU, SSDE doesn't strongly require you
* to have PC aligned to 4 (or 2) byte boundary. Instead,
* if PC goes misaligned (with user's intention), upon
* ::dec() "error_misalign" will be set, but instruction
* still will be decoded from the address without realigning
* the PC just fine
*/

/*
* SSDE disassembler for ARM architecture
*/
class ssde_arm : public ssde
{
public:
	/*
	* ARM CPU states
	*/
	enum class cpu_state
	{
		arm    = 0x00,                      // Normal ARM mode
		thumb  = 0x01,                      // Thumb (16 bit) mode
		thumb2 = 0x02,                      // Thumb 2 (32 bit) mode
	};

	/*
	* ARM execution conditions
	*/
	enum class cc : uint8_t
	{
		eq,
		ne,
		hs,
		lo,
		mi,
		pl,
		vs,
		vc,
		hi,
		ls,
		ge,
		lt,
		gt,
		le,
		al,
		nv,
	};

	using ssde::ssde;

	bool dec() override final;

private:
	void reset_fields();

	uint32_t fetch();

public:
	size_t &pc = ip;                        // ARM's naming of IP
	cpu_state state = cpu_state::arm;       // Specifies which state the CPU is in and changes decoder's behaviour

	bool error_cpu_state = false;           // Unknown CPU state
	bool error_misalign  = false;           // PC is not aligned to 4 (or 2) byte boundary

	union
	{
		cc condition = (cc)0;               // Instruction's condition required for execution

		struct
		{
			uint8_t n : 1;                  // Flag 'negative'
			uint8_t z : 1;                  // Flag 'zero'
			uint8_t c : 1;                  // Flag 'carry'
			uint8_t v : 1;                  // Flag 'overflow'
		} condition_bits;
	};

	bool     is_branch = false;             // Is this a branch instruction?
	bool     has_link  = false;             // Does this branch instruction have a link? (for return)
	int32_t  rel       = 0;                 // Relative (to PC) address value
	uint32_t abs       = 0;                 // Absolute address value

	bool     is_swi   = false;              // Is this a call to SWI handler?
	uint32_t swi_data = 0;                  // SWI data, its meaning is up to the OS software is built for

private:
};