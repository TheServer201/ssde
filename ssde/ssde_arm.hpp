// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#pragma once
#include <string>
#include <stdint.h>


// Unlike real ARM CPU, SSDE doesn't strongly require you to have PC aligned to
// 4 (or 2) byte boundary. Instead, if PC happens to be misaligned (if intended
// by user) upon decode Error::alignment will be signaled. Instruction will
// still be decoded normally.

namespace ssde
{

struct Inst_ARM
{
public:
	enum class Error : uint8_t
	{
		eof       = 1 << 0, // Reached end of buffer before finished decoding
		alignment = 1 << 1, // PC is misaligned
		cpu_state = 1 << 2, // Unknown CPU state
	};

	enum class CPU_state
	{
		arm    = 0x00,
		thumb  = 0x01,
		thumb2 = 0x02,
	};

	enum class Exec_cond : uint8_t // ARM's execution condition
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


	Inst_ARM() = default;

	Inst_ARM(const std::string& buffer, size_t start_pc = 0) :
		pc(start_pc)
	{
		try
		{
			internal_decode(buffer);
		}
		catch (const std::out_of_range&)
		{
			signal_error(Error::eof);
		}
	}

	bool has_error(Error signal) const
	{
		return (error_flags & static_cast<uint8_t>(signal)) ? true : false;
	}

	bool has_error() const
	{
		return error_flags != 0 ? true : false;
	}


	size_t pc = 0;
	int    length = 0;

	// Specifies which state the CPU is in and changes decoder's behaviour
	CPU_state state = CPU_state::arm;

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

	bool     is_branch = false;
	bool     has_link = false;
	int32_t  rel = 0;
	uint32_t rel_abs = 0;

	bool     is_swi = false;
	uint32_t swi_data = 0;

private:
	void internal_decode(const std::string&);
	uint32_t fetch(const std::string& buffer);
	void decode_arm(const std::string& buffer);

	void signal_error(Error signal)
	{
		error_flags |= static_cast<uint8_t>(signal);
	}

	//uint32_t bc = 0;
	uint8_t error_flags = 0;
};

} // namespace ssde