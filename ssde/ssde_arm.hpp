// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#pragma once
#include <cstdint>
#include <vector>


// Unlike real ARM CPU, SSDE doesn't strongly require you to have PC aligned to
// 4 (or 2) byte boundary. Instead, if PC happens to be misaligned (if intended
// by user) upon decode Error::alignment will be signaled. Instruction will
// still be decoded normally.

namespace ssde
{

class Inst_ARM
{
public:
	enum class Error : std::uint8_t
	{
		eof       = 1 << 0, // Reached end of buffer before finished decoding
		alignment = 1 << 1, // PC is misaligned
		cpu_state = 1 << 2, // Unknown CPU state
		opcode    = 1 << 3, // Badly encoded instruction
	};

	enum class CPU_state
	{
		arm    = 0x00,
		thumb  = 0x01,
	};

	enum class Exec_cond : std::uint8_t // ARM's execution condition
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


	Inst_ARM()
	{
	}

	Inst_ARM(const std::vector<std::uint8_t>& in_buffer,
	         std::size_t in_pos = 0,
	         CPU_state   in_state = CPU_state::arm) :
		pos(in_pos)
	{
		internal_decode(in_buffer, in_state);
	}

	bool has_error(Error signal) const
	{
		return (error_flags & static_cast<std::uint8_t>(signal)) ? true : false;
	}

	bool has_error() const
	{
		return error_flags != 0 ? true : false;
	}


	std::int32_t length = 0;

	// Specifies condition required to execute the instruction
	Exec_cond cond = Exec_cond::al;

	bool    is_branch = false;
	bool    has_link = false;
	// abs = pos + rel
	std::int32_t rel = 0;

	bool is_swi = false;
	std::int32_t swi_data = 0;

private:
	void internal_decode(const std::vector<std::uint8_t>&, CPU_state);
	void decode_as_arm(std::uint32_t);
	// void decode_as_thumb(uint32_t);

	std::uint32_t fetch(const std::vector<std::uint8_t>& buffer,
	                    std::size_t amount)
	{
		if ((pos + amount) < buffer.size())
		{
			std::uint32_t result = 0;

			for (size_t i = 0; i < amount; ++i)
				result |= static_cast<std::uint32_t>(buffer[pos++]) << i*8;

			return result;
		}
		else
		{
			signal_error(Error::eof);
			return 0;
		}
	}

	void signal_error(Error signal)
	{
		error_flags |= static_cast<std::uint8_t>(signal);
	}

	std::size_t  pos = 0;
	std::uint8_t error_flags = 0;
};

} // namespace ssde