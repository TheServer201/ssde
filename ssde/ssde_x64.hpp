// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#pragma once
#include "ssde.hpp"
#include <string>
#include <stdint.h>


namespace ssde
{

struct Inst_x64 : public Inst
{
public:
	enum class Prefix : uint8_t // X86 prefixes
	{
		none = 0,

		seg_cs = 0x2e,
		seg_ss = 0x36,
		seg_ds = 0x3e,
		seg_es = 0x26,
		seg_fs = 0x64,
		seg_gs = 0x65,
		lock   = 0xf0,
		repnz  = 0xf2,
		repz   = 0xf3,
		p66    = 0x66,
		p67    = 0x67,

		branch_taken     = 0x3e,
		branch_not_taken = 0x2e,

		fpu_double = 0xf2,
		fpu_single = 0xf3,
	};

	enum class VEX_rm : uint8_t // EVEX rounding modes
	{
		near  = 0x00,
		floor = 0x01,
		ceil  = 0x02,
		trunc = 0x03,
		none  = (uint8_t)-1
	};

	// Mod R/M addressing mode
	enum class RM_mode : uint8_t
	{
		mem        = 0x00, // [r]
		mem_disp8  = 0x01, // [r]+disp8
		mem_disp32 = 0x02, // [r]+disp32
		reg        = 0x03, // r
	};


	bool has_prefix(Prefix pref) const
	{
		return (prefixes[0] == pref ||
				prefixes[1] == pref ||
				prefixes[2] == pref ||
				prefixes[3] == pref);
	}


	bool error_lock;  // LOCK prefix is not allowed
	bool error_novex; // Instruction is only allowed to be VEX encoded

	size_t& ip = pc;

	// Instruction's prefixes (grouped)
	// To check if instruction has prefix, use Inst_x64::has_prefix
	// 0: LOCK, REPNZ and REPZ prefixes and/or FPU op. size modifiers
	// 1: Segment (seg_*) prefixes and/or branch hints
	// 2: Operand size override prefix (p66)
	// 3: Address size override prefix (p67)
	Prefix prefixes[4];

	bool    has_rex;
	uint8_t rex_w;
	uint8_t rex_r;
	uint8_t rex_x;
	uint8_t rex_b;

	bool    has_vex;
	uint8_t vex_l;
	uint8_t vex_rr;
	bool    vex_zero; // Should zero or merge?; z field
	uint8_t vex_size;
	uint8_t vex_reg;
	uint8_t vex_opmask;
	VEX_rm  vex_round_to; // EVEX: Rounding mode
	bool    vex_sae; // EVEX: suppress all exceptions
	bool&   vex_rc = vex_sae; // EVEX: rounding control, MXCSR override, implies SAE
	bool&   vex_broadcast = vex_sae; // EVEX: broadcast element across register, for load instructions only

	int     opcode_length;
	uint8_t opcode[3];

	bool    has_modrm;
	RM_mode modrm_mod; // Mod R/M address mode
	uint8_t modrm_reg; // Register number or opcode information
	uint8_t modrm_rm; // Operand register

	bool    has_sib;
	uint8_t sib_scale;
	uint8_t sib_index;
	uint8_t sib_base;

	bool     has_disp;
	int      disp_size;
	uint32_t disp;

	bool     has_imm;
	bool     has_imm2;
	int      imm_size;
	int      imm2_size;
	uint64_t imm;
	uint64_t imm2;

	bool     has_rel;
	int      rel_size;
	int32_t  rel;
	uint64_t rel_abs; // Absolute address for destination


private:
	virtual void internal_decode(const std::string&) override;

	void decode_prefixes(const std::string&);
	void decode_opcode(const std::string&);
	void decode_modrm(const std::string&);
	void decode_sib(const std::string&);
	void rex_extend_modrm();
	void read_disp(const std::string&);
	void read_imm(const std::string&);
	void vex_decode_pp(uint8_t pp);
	void vex_decode_mm(uint8_t mm);

	uint16_t flags;
};

} // namespace ssde