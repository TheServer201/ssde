// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#pragma once
#include "ssde.hpp"
#include <string>
#include <stdint.h>


// Unlike real ARM CPU, SSDE doesn't strongly require you to have PC aligned to
// 4 (or 2) byte boundary. Instead, if PC happens to be misaligned (if intended
// by user) upon ::decode call "error_alignment" field will be set. Instruction
// still will be decoded normally.

namespace ssde
{

struct Inst_arm : public Inst
{
public:
	enum class CPU_state
	{
		arm    = 0x00,
		thumb  = 0x01,
		thumb2 = 0x02,
	};

	// ARM's instruction execution conditions
	enum class Exec_cond : uint8_t
	{
		eq = 0x0,
		ne = 0x1,
		hs = 0x2,
		lo = 0x3,
		mi = 0x4,
		pl = 0x5,
		vs = 0x6,
		vc = 0x7,
		hi = 0x8,
		ls = 0x9,
		ge = 0xa,
		lt = 0xb,
		gt = 0xc,
		le = 0xd,
		al = 0xe,
		nv = 0xf,
	};

public:

public:
	// Specifies which state the CPU is in and changes decoder's behaviour
	CPU_state state = CPU_state::arm;

	bool error_cpu_state; // Unknown CPU state
	bool error_alignment; // PC is not aligned to 4 (or 2) byte boundary

	union
	{
		Exec_cond cond = Exec_cond::al;

		struct
		{
			uint8_t n : 1; // 'negative'
			uint8_t z : 1; // 'zero'
			uint8_t c : 1; // 'carry'
			uint8_t v : 1; // 'overflow'
		}
		cond_bits;
	};

	bool is_branch;
	bool is_branch_link;
	int32_t  rel;
	uint32_t abs;

	bool     is_swi;
	uint32_t swi_data;

private:
	virtual void internal_decode(const std::string&) override;

	uint32_t fetch(const std::string& buffer);
	void decode_arm(const std::string& buffer);

private:
	//uint32_t bc;
};

} // namespace ssde