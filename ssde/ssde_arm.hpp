// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#pragma once
#include <vector>
#include <stdexcept>
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


	void decode(const std::vector<uint8_t>& buffer,
	            size_t    s_pc = 0,
	            CPU_state state = CPU_state::arm)
	{
		pc = s_pc;

		try
		{
			internal_decode(buffer, state);
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


	size_t  pc = 0;
	int32_t length = 0;

	// Specifies processor state instruction was/is/will be encoded for
	CPU_state state = CPU_state::arm;
	// Specifies condition required to execute instruction
	Exec_cond cond = Exec_cond::al;

	bool    is_branch = false;
	bool    has_link = false;
	int32_t rel = 0;
	size_t  rel_abs = 0;

	bool    is_swi = false;
	int32_t swi_data = 0;

private:
	void internal_decode(const std::vector<uint8_t>&, CPU_state);
	uint32_t fetch(const std::vector<uint8_t>& buffer, int32_t);
	void decode_as_arm(uint32_t bc);
	void decode_as_thumb(uint32_t bc);

	void signal_error(Error signal)
	{
		error_flags |= static_cast<uint8_t>(signal);
	}

	uint8_t  error_flags = 0;
};

} // namespace ssde