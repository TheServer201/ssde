/*
* The SSDE implementation for X86 instruction set.
* Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md
*/
#include "ssde_x86.hpp"

#include <string>

#include <stdint.h>


/*
* Major amounts of information this code was based on ware taken from
* the "Intel(R) 64 and IA-32 Architectures Software Developer's Manual".
* If You are unfamiliar with the X86 architecture, it's recommended that
* you read the manual first. The manuals can be obtained @
*   http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html
*
* Basic architecture @
*   http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-1-manual.pdf
* Instruction set reference @
*   http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383.pdf
*/

enum : uint16_t
{
	none = 0,

	rm  = 1 << 0, // expect Mod byte
	rel = 1 << 1, // instruction's imm is a relative address
	i8  = 1 << 2, // has  8 bit imm
	i16 = 1 << 3, // has 16 bit imm
	i32 = 1 << 4, // has 32 bit imm, which can be turned to 16 with 66 prefix
	am  = 1 << 5, // instruction uses address mode, imm is a memory address
	vx  = 1 << 6, // instruction requires a VEX prefix
	mp  = 1 << 7, // instruction has a mandatory 66 prefix

	r8  = i8  | rel,
	r32 = i32 | rel,

	error = (uint16_t)-1
};

/* 1st opcode flag table */
static const uint16_t op_table[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7*/
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none , /* 00x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , error, /* 01x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none , /* 02x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , none , none , /* 03x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 04x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 05x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 06x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, none , /* 07x */
	 none , none , none , none , none , none , none , none , /* 10x */
	 none , none , none , none , none , none , none , none , /* 11x */
	 none , none , none , none , none , none , none , none , /* 12x */
	 none , none , none , none , none , none , none , none , /* 13x */
	 none , none ,  rm  ,  rm  , error, error, error, error, /* 14x */
	  i32 ,rm|i32,  i8  , rm|i8, none , none , none , none , /* 15x */
	  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  , /* 16x */
	  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  , /* 17x */
	 rm|i8,rm|i32, rm|i8, rm|i8,  rm  ,  rm  ,  rm  ,  rm  , /* 20x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 21x */
	 none , none , none , none , none , none , none , none , /* 22x */
	 none , none,i32|i16, error, none , none , none , none , /* 23x */
	  am  ,  am  ,  am  ,  am  , none , none , none , none , /* 24x */
	  i8  ,  i32 , none , none , none , none , none , none , /* 25x */
	  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  , /* 26x */
	  i32 ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 ,  i32 , /* 27x */
	 rm|i8, rm|i8,  i16 , none ,  rm  ,  rm  , rm|i8,rm|i32, /* 30x */
	i16|i8, none ,  i16 , none , none ,  i8  , none , none , /* 31x */
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i8  , none , none , /* 32x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 33x */
	  r8  ,  r8  ,  r8  ,  r8  ,  i8  ,  i8  ,  i8  ,  i8  , /* 34x */
	  r32 ,  r32,i32|i16,  r8  , none , none , none , none , /* 35x */
	 none , none , error, error, none , none , error, error, /* 36x */
	 none , none , none , none , none , none ,  rm  ,  rm  , /* 37x */
};

/*
* 2nd opcode flag table
* 0F xx
*/
static const uint16_t op_table_0f[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7*/
	  rm  ,  rm  ,  rm  ,  rm  , error, error, none , error, /* 00x */
	 none , none , error, none , error,  rm  , none , error, /* 01x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 02x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 03x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error,  rm  , error, /* 04x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 05x */
	 none , none , none , none , none , none , error, none , /* 06x */
	 error, error, error, error, error, error, error, error, /* 07x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 10x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 11x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 12x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 13x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 14x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 15x */
	 rm|i8, rm|i8, rm|i8, rm|i8,  rm  ,  rm  ,  rm  , none , /* 16x */
	  rm  ,  rm  , error, error,  rm  ,  rm  ,  rm  ,  rm  , /* 17x */
	  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 , /* 20x */
	  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 , /* 21x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 22x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 23x */
	 none , none , none ,  rm  , rm|i8,  rm  , error, error, /* 24x */
	 none , none , none ,  rm  , rm|i8,  rm  ,  rm  ,  rm  , /* 25x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 26x */
	  rm  , none , rm|i8,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 27x */
	  rm  ,  rm  , rm|i8,  rm  , rm|i8, rm|i8, rm|i8,  rm  , /* 30x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 31x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 32x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 33x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 34x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 35x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 36x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 37x */
};

/*
* 3rd opcode flag table
* 0F 38 xx
*/
static const uint16_t op_table_38[256] =
{
	/*x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7*/
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 00x */
	  rm  ,  rm  ,  rm  ,  rm  , vx|rm, vx|rm, error, error, /* 01x */
	 mp|rm, error, error, error, mp|rm, mp|rm, error, mp|rm, /* 02x */
	 vx|rm, error, vx|rm, error,  rm  ,  rm  ,  rm  , error, /* 03x */
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, error, error, /* 04x */
	 mp|rm, mp|rm, mp|rm, mp|rm, vx|rm, vx|rm, error, error, /* 05x */
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, error, mp|rm, /* 06x */
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, /* 07x */
	 mp|rm, mp|rm, error, error, error, error, error, error, /* 10x */
	 error, error, error, error, error, error, error, error, /* 11x */
	 error, error, error, error, error, error, error, error, /* 12x */
	 vx|rm, vx|rm, error, error, error, error, error, error, /* 13x */
	 error, error, error, error, error, error, error, error, /* 14x */
	 error, error, error, error, error, error, error, error, /* 15x */
	 error, error, error, error, error, error, error, error, /* 16x */
	 vx|rm, vx|rm, error, error, error, error, error, error, /* 17x */
	 mp|rm, mp|rm, error, error, error, error, error, error, /* 20x */
	 error, error, error, error, error, error, error, error, /* 21x */
	 error, error, error, error, error, error, vx|rm, vx|rm, /* 22x */
	 vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, /* 23x */
	 error, error, error, error, error, error, vx|rm, vx|rm, /* 24x */
	 vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, /* 25x */
	 error, error, error, error, error, error, vx|rm, vx|rm, /* 26x */
	 vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, /* 27x */
	 error, error, error, error, error, error, error, error, /* 30x */
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error, error, /* 31x */
	 error, error, error, error, error, error, error, error, /* 32x */
	 error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , /* 33x */
	 error, error, error, error, error, error, error, error, /* 34x */
	 error, error, error, error, error, error, error, error, /* 35x */
	  rm  ,  rm  , error, error, error, error,  rm  , error, /* 36x */
	 error, error, error, error, error, error, error, error, /* 37x */
};

/*
* 3rd opcode flag table
* 0F 3A xx
*/
static const uint16_t op_table_3a[256] =
{
	/* x0   |   x1   |   x2   |   x3   |   x4   |   x5   |   x6   |   x7 */
	  error ,  error ,  error ,  error ,  error ,  error ,vx|rm|i8,  error , /* 00x */
	mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,   rm   , /* 01x */
	  error ,  error ,  error ,  error ,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8, /* 02x */
	vx|rm|i8,vx|rm|i8,  error ,  error ,  error ,  error ,  error ,  error , /* 03x */
	mp|rm|i8,mp|rm|i8,mp|rm|i8,  error ,  error ,  error ,  error ,  error , /* 04x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 05x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 06x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 07x */
	  mp|rm ,  mp|rm ,mp|rm|i8,  error ,  error ,  error ,  error ,  error , /* 10x */
	  error ,  error ,vx|rm|i8,vx|rm|i8,vx|rm|i8,  error ,  error ,  error , /* 11x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 12x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 13x */
	mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,  error ,  error ,  error ,  error , /* 14x */
	vx|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 15x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 16x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 17x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 20x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 21x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 22x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 23x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 24x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 25x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 26x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 27x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 30x */
	  error ,  error ,  error ,  error ,mp|rm|i8,  error ,  error ,  error , /* 31x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 32x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 33x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 34x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 35x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 36x */
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , /* 37x */
};


bool ssde_x86::dec()
{
	if (ip >= buffer.length())
		return false;

	reset_fields();

	decode_prefixes();
	decode_opcode();

	if (flags != ::error)
		/* it's not a bullshit instruction */
	{
		if (flags & ::mp && group3 != pref::p66)
			/* this instruction lacks mandatory 66 prefix */
		{
			error = true;
			error_opcode = true;
		}

		if (flags & ::rm)
			/* this instruction has a Mod byte, decode it */
		{
			decode_modrm();

			if (has_sib)
				/* instruction has a SIB byte, decode it, too */
			{
				decode_sib();
			}

			if (has_disp)
				/* instruction has displacement value, read it */
			{
				read_disp();
			}
		}
		else if (group1 == pref::lock)
			/* LOCK prefix only makes sense for Mod M */
		{
			error = true;
			error_lock = true;
		}

		/* read moffs, imm or rel */
		read_imm();


		if (length > 15)
			/* CPU can't handle instructions longer than 15 bytes */
		{
			length = 15;

			error = true;
			error_length = true;
		}
	}
	else
	{
		error = true;
		error_opcode = true;

		length = 1;
	}

	return true;
}

/* resets fields before new iteration of the instruction decoder */
void ssde_x86::reset_fields()
{
	length = 0;

	error         = false;
	error_opcode  = false;
	error_operand = false;
	error_length  = false;
	error_lock    = false;
	error_novex   = false;

	has_modrm = false;
	has_sib   = false;
	has_imm   = false;
	has_imm2  = false;
	has_disp  = false;
	has_rel   = false;
	has_vex   = false;

	group1 = pref::none;
	group2 = pref::none;
	group3 = pref::none;
	group4 = pref::none;

	opcode1 = 0;
	opcode2 = 0;
	opcode3 = 0;

	has_vex    = false;
	vex_zero   = false;
	vex_reg    = 0;
	vex_opmask = 0;
	vex_l      = 0;
	vex_round  = vex_rm::off;
	vex_sae    = false;

	flags = ::error;
}

/* decode legacy prefixes the same way CPU does */
void ssde_x86::decode_prefixes()
{
	for (int x = 0; x < 14; ++x, ++length)
		/*
		* This is prefix analyzer. It behaves exactly the
		* same way real CPUs analyze instructions for
		* prefixes. Normally, each instruction is allowed
		* to have up to 4 prefixes from each group. Though,
		* in cases when instruction has more prefixes, it
		* will ignore any prefix it meets if there was a
		* prefix from the same group before it. Instruction
		* decoders can only handle words up to 15 bytes long,
		* if the word is longer than that, decoder will fail.
		*/
	{
		pref prefix = (pref)buffer[ip + length];

		/* 1st group */
		if (prefix == pref::lock  ||
		    prefix == pref::repnz ||
		    prefix == pref::repz)
		{
			if (group1 == pref::none)
				group1 = prefix;

			continue;
		}

		/* 2nd group */
		if (prefix == pref::seg_cs || prefix == pref::seg_ss ||
		    prefix == pref::seg_ds || prefix == pref::seg_es ||
		    prefix == pref::seg_fs || prefix == pref::seg_gs
			/* pref::branch_not_taken, pref::branch_taken, */)
		{
			if (group2 == pref::none)
				group2 = prefix;

			continue;
		}

		/* 3rd group */
		if (prefix == pref::p66)
		{
			if (group3 == pref::none)
				group3 = prefix;

			continue;
		}

		/* 4th group */
		if (prefix == pref::p67)
		{
			if (group4 == pref::none)
				group4 = prefix;

			continue;
		}

		break;
	}
}

/* read opcode bytes or decode them from VEX */
void ssde_x86::decode_opcode()
{
	if (((uint8_t)buffer[ip + length] == 0xc4 ||
	     (uint8_t)buffer[ip + length] == 0xc5 ||
	     (uint8_t)buffer[ip + length] == 0x62) &&
	    (buffer[ip + length+1] & 0xc0) == 0xc0)
		/* looks like we've found a VEX prefix */
	{
		has_vex = true;

		if (group1 != pref::none ||
		    group2 != pref::none ||
		    group3 != pref::none ||
		    group4 != pref::none)
			/* VEX-encoded instructions are not allowed to be preceeded by legacy prefixes */
		{
			error = true;
			error_opcode = true;
		}


		uint8_t prefix = buffer[ip + length++];

		if (prefix == 0x62)
			/* this is a 4 byte VEX */
		{
			vex_size = 4;


			uint8_t vex_1 = buffer[ip + length++];
			uint8_t vex_2 = buffer[ip + length++];
			uint8_t vex_3 = buffer[ip + length++];

			vex_decode_mm(vex_1 & 0x03);


			/* determine destination register from vvvv */
			vex_reg = ((~vex_2 >> 3) & 0x0f) | (vex_3 & 0x80 ? 0x10 : 0);

			vex_decode_pp(vex_2 & 0x03);

			vex_zero = vex_3 & 0x80 ? true : false;
			vex_l    = (vex_3 >> 5) & 0x03;

			vex_sae  = vex_3 & 0x10 ? true : false;

			vex_opmask = vex_3 & 0x07;

			if (vex_rc)
				/* rounding control, implies vector is 512 bits wide */
			{
				vex_round = (vex_rm)vex_l;
				vex_l     = 0x02;
			}
			else if (vex_l == 0x03)
				/* destination vector can't be wider than 512 bits */
			{
				error = true;
				error_operand = true;
			}
		}
		else
			/* 2 or 3 byte VEX */
		{
			if (prefix == 0xc4)
				/* this is a 3 byte VEX */
			{
				vex_size = 3;


				uint8_t vex_1 = buffer[ip + length++];
				vex_decode_mm(vex_1 & 0x1f);
			}
			else
				/* this is a 2 byte VEX */
			{
				vex_size = 2;
				opcode1  = 0x0f;
			}


			uint8_t vex_2 = buffer[ip + length++];

			vex_l = vex_2 & 0x04 ? 1 : 0;

			/* determine destination register from vvvv */
			vex_reg = (~vex_2 >> 3) & 0x0f;

			vex_decode_pp(vex_2 & 0x03);
		}
	}
	else
		/* instruction operands are written normal way */
	{
		opcode1 = buffer[ip + length++];

		if (opcode1 == 0x0f)
		{
			opcode2 = buffer[ip + length++];
		}
		else
			/* this is a regular single opcode instruction */
		{
			flags = op_table[opcode1];
		}
	}

	if (opcode1 == 0x0f)
		/* decode 2nd or 3rd opcode byte */
	{
		switch (opcode2)
		{
		case 0x38:
			opcode3 = buffer[ip + length++];
			flags   = op_table_38[opcode3];
			break;

		case 0x3a:
			opcode3 = buffer[ip + length++];
			flags   = op_table_3a[opcode3];
			break;

		default:
			opcode2 = buffer[ip + length++];
			flags   = opcode2;
			break;
		}
	}

	if (flags & ::vx && !has_vex)
		/* this instruction can only be VEX-encoded */
	{
		error = true;
		error_novex = true;
	}

	if (opcode1 == 0xf6 || opcode1 == 0xf7)
		/*
		* These are two exceptional opcodes that extend
		* using 3 bits of Mod R/M byte and they lack
		* consistent flags. Instead of creating a new
		* flags table for each extended opcode, I decided
		* to put this little bit of code that is dedicated
		* to these two exceptional opcodes.
		*/
	{
		switch (buffer[ip + length] >> 3 & 0x07)
		{
		case 0x00:
		case 0x01:
			{
				if (opcode1 == 0xf6)
					flags = rm | i8;

				if (opcode1 == 0xf7)
					flags = rm | i32;
			}
			break;

		default:
			flags = rm;
			break;
		}
	}
}

/* decodes a Mod R/M byte */
void ssde_x86::decode_modrm()
{
	uint8_t modrm_byte = buffer[ip + length++];

	has_modrm = true;
	modrm_mod = modrm_byte >> 6 & 0x03;
	modrm_reg = modrm_byte >> 3 & 0x07;
	modrm_rm  = modrm_byte      & 0x07;

	switch (modrm_mod)
	{
	case 0x00:
		if (group4 == pref::p67)
		{
			if (modrm_rm == 0x06)
			{
				has_disp  = true;
				disp_size = 2;
			}
		}
		else
		{
			if (modrm_rm == 0x04)
				has_sib = true;

			if (modrm_rm == 0x05)
			{
				has_disp  = true;
				disp_size = 4;
			}
		}
		break;

	case 0x01:
		{
			if (group4 != pref::p67 && modrm_rm == 0x04)
				has_sib = true;

			has_disp  = true;
			disp_size = 1;
		}
		break;

	case 0x02:
		{
			if (group4 != pref::p67 && modrm_rm == 0x04)
				has_sib = true;

			has_disp  = true;
			disp_size = group4 != pref::p67 ? 4 : 2;
		}
		break;

	case 0x03:
		if (group1 == pref::lock)
			/* LOCK prefix is not allowed to be used with Mod R */
		{
			error = true;
			error_lock = true;
		}
		break;

	default:
		break;
	}
}

/* decodes SIB byte */
void ssde_x86::decode_sib()
{
	uint8_t sib_byte = buffer[ip + length++];

	sib_scale = 1 << (sib_byte >> 6 & 0x03);
	sib_index = sib_byte >> 3 & 0x07;
	sib_base  = sib_byte      & 0x07;
}

/* read displacement */
void ssde_x86::read_disp()
{
	disp = 0;

	for (int i = 0; i < disp_size; i++)
		disp |= buffer[ip + length++] << i*8;


	if (disp & (1 << (disp_size*8 - 1)))
		/* disp is signed, extend the sign if needed */
	{
		switch (disp_size)
		{
		case 1:
			disp |= 0xffffff00;
			break;

		case 2:
			disp |= 0xffff0000;
			break;

		default:
			break;
		}
	}
}

/* decodes a moffs, imm or rel operand */
void ssde_x86::read_imm()
{
	if (flags & ::am)
		/* address mode instructions behave a little differently */
	{
		has_imm  = true;
		imm_size = group4 != pref::p67 ? 4 : 2;
	}
	else
	{
		if (flags & ::i32)
		{
			has_imm  = true;
			imm_size = group3 != pref::p66 ? 4 : 2;
		}

		if (flags & ::i16)
		{
			if (has_imm)
			{
				has_imm2  = true;
				imm2_size = 2;
			}
			else
			{
				has_imm  = true;
				imm_size = 2;
			}
		}

		if (flags & ::i8)
		{
			if (has_imm)
			{
				has_imm2  = true;
				imm2_size = 1;
			}
			else
			{
				has_imm  = true;
				imm_size = 1;
			}
		}
	}

	if (has_imm)
	{
		imm = 0;

		for (int i = 0; i < imm_size; ++i)
			imm |= buffer[ip + length++] << i*8;


		if (has_imm2)
		{
			imm2 = 0;

			for (int i = 0; i < imm2_size; ++i)
				imm2 |= buffer[ip + length++] << i*8;
		}
	}

	if (flags & ::rel)
		/* this instruction has relative address, move imm to rel */
	{
		has_imm = false;

		rel_size = imm_size;
		rel = imm;

		if (rel & (1 << (rel_size*8 - 1)))
			/* rel is negative, extend the sign */
		{
			switch (rel_size)
			{
			case 1:
				rel |= 0xffffff00;
				break;

			case 2:
				rel |= 0xffff0000;
				break;

			default:
				break;
			}
		}

		abs = (uint32_t)ip + length + rel;

		has_rel = true;
	}
}

/* decode SIMD prefix from pp field of VEX */
void ssde_x86::vex_decode_pp(uint8_t pp)
{
	switch (pp)
	{
	case 0x01:
		group3 = pref::p66;
		break;

	case 0x02:
		group1 = pref::repz;
		break;

	case 0x03:
		group1 = pref::repnz;
		break;

	default:
		break;
	}
}

/* determine opcode bytes from mm field of VEX */
void ssde_x86::vex_decode_mm(uint8_t mm)
{
	switch (mm)
	{
	case 0x01:
		opcode1 = 0x0f;
		break;

	case 0x02:
		opcode1 = 0x0f;
		opcode2 = 0x38;
		break;

	case 0x03:
		opcode1 = 0x0f;
		opcode2 = 0x3a;
		break;

	default:
		error = true;
		error_opcode = true;
		break;
	}
}