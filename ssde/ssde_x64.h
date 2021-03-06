// Copyright (C) 2016, Constantine Shablya.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// * The above copyright notice and this permission notice shall be
//   included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#ifndef SSDE_X64_H
#define SSDE_X64_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>


namespace ssde
{

class Inst_x64
{
public:
	enum class Error : std::uint8_t
	{
		eof     = 1 << 0, // Reached end of buffer before finished decoding
		length  = 1 << 1, // Instruction is too long
		opcode  = 1 << 2, // Instruction doesn't exist
		operand = 1 << 3, // Operands don't match instruction's requirements
		no_vex  = 1 << 4, // Instruction should've been VEX encoded
		lock    = 1 << 5, // LOCK prefix is not allowed
		rex     = 1 << 6, // REX prefix is not allowed
	};

	enum class Prefix : std::uint8_t // X86 instruction prefix
	{
		none             = 0x00,
		seg_cs           = 0x2e,
		seg_ss           = 0x36,
		seg_ds           = 0x3e,
		seg_es           = 0x26,
		seg_fs           = 0x64,
		seg_gs           = 0x65,
		lock             = 0xf0,
		repnz            = 0xf2,
		repz             = 0xf3,
		p66              = 0x66,
		p67              = 0x67,
		branch_taken     = 0x3e,
		branch_not_taken = 0x2e,
		fpu_double       = 0xf2,
		fpu_single       = 0xf3,
	};

	enum class VEX_rm : std::uint8_t // EVEX rounding mode
	{
		near  = 0x00,
		floor = 0x01,
		ceil  = 0x02,
		trunc = 0x03,
		none  = 0xff,
		mxcsr = 0xff,
	};

	enum class RM_mode : std::uint8_t // Mod R/M addressing mode
	{
		mem        = 0x00, // [r]
		mem_disp8  = 0x01, // [r]+disp8
		mem_disp32 = 0x02, // [r]+disp32
		reg        = 0x03, // r
	};


	Inst_x64()
	{
	}

	Inst_x64(const std::vector<std::uint8_t>& buffer, std::size_t in_pos = 0) :
		pos(in_pos)
	{
		internal_decode(buffer);
	}

	bool has_prefix(Prefix pref) const
	{
		return (prefixes[0] == pref || prefixes[1] == pref ||
		        prefixes[2] == pref || prefixes[3] == pref);
	}

	bool has_prefix() const
	{
		return (prefixes[0] != Prefix::none || prefixes[1] != Prefix::none ||
		        prefixes[2] != Prefix::none || prefixes[3] != Prefix::none);
	}

	bool has_error(Error signal) const
	{
		return (error_flags & static_cast<std::uint8_t>(signal)) ? true : false;
	}

	bool has_error() const
	{
		return error_flags != 0;
	}


	std::int32_t length = 0;

	// Instruction's prefixes (grouped)
	// To check if instruction has prefix, use Inst_x64::has_prefix

	// 0: LOCK, REPNZ and REPZ prefixes and/or FPU op. size modifiers
	// 1: Segment (seg_*) prefixes and/or branch hints
	// 2: Operand size override prefix (p66)
	// 3: Address size override prefix (p67)
	std::array<Prefix, 4> prefixes{ };

	bool has_rex = false;
	bool rex_W   = false;
	bool rex_R   = false;
	bool rex_X   = false;
	bool rex_B   = false;

	bool has_vex  = false;
	bool vex_LL   = false;
	bool vex_L    = false;
	bool vex_RR   = false;
	bool vex_zero = false; // Should zero or merge?; z field
	std::int32_t vex_vec_bits = 0;
	std::int32_t vex_size     = 0;
	std::uint8_t vex_reg      = 0;
	std::uint8_t vex_opmask   = 0;

	VEX_rm vex_round_to = VEX_rm::mxcsr; // EVEX: Rounding mode
	bool  vex_sae = false; // EVEX: suppress all exceptions
	bool& vex_rc  = vex_sae; // EVEX: rounding, MXCSR override, implies SAE
	bool& vex_broadcast = vex_sae; // EVEX: broadcast element across register

	std::int32_t opcode_length = 0;
	std::array<std::uint8_t, 3> opcode{ };

	bool    has_modrm = false;
	RM_mode modrm_mod = RM_mode::mem; // Mod R/M address mode
	std::uint8_t modrm_reg = 0; // Register number or opcode information
	std::uint8_t modrm_rm  = 0; // Operand register

	bool has_sib = false;
	std::uint8_t sib_scale = 0;
	std::uint8_t sib_index = 0;
	std::uint8_t sib_base  = 0;

	bool has_disp = false;
	std::int32_t disp_size = 0;
	std::int32_t disp = 0;

	bool has_imm  = false;
	bool has_imm2 = false;
	std::int32_t  imm_size  = 0;
	std::int32_t  imm2_size = 0;
	std::uint64_t imm  = 0;
	std::uint64_t imm2 = 0;

	bool has_rel = false;
	std::int32_t rel_size = 0;
	// abs = ip + rel
	std::int32_t rel = 0;

private:
	void internal_decode(const std::vector<std::uint8_t>&);
	void decode_prefixes(const std::vector<std::uint8_t>&);
	void decode_opcode(const std::vector<std::uint8_t>&);
	void decode_vex(const std::vector<std::uint8_t>&);
	void vex_decode_pp(std::uint8_t pp);
	void vex_decode_mm(std::uint8_t mm);
	void decode_modrm(const std::vector<std::uint8_t>&);
	void decode_sib(const std::vector<std::uint8_t>&);
	void rex_extend_modrm();
	void read_imm(const std::vector<std::uint8_t>&);
	void read_disp(const std::vector<std::uint8_t>&);

	std::uint8_t get_byte(const std::vector<std::uint8_t>& buffer)
	{
		if (pos < buffer.size())
		{
			length++;
			return buffer[pos++];
		}
		else
		{
			signal_error(Error::eof);
			return 0;
		}
	}

	std::uint8_t peek_byte(const std::vector<std::uint8_t>& buffer,
	                       std::size_t offset = 0) const
	{
		return (pos + offset) < buffer.size() ? buffer[pos + offset] : 0;
	}

	void signal_error(Error signal)
	{
		error_flags |= static_cast<std::uint8_t>(signal);
	}

	std::size_t pos = 0;
	std::uint16_t flags = 0;
	std::uint8_t error_flags = 0;
};

} // namespace ssde

#endif // SSDE_X64_H