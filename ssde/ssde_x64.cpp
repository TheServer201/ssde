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
//
// SSDE implementation for X64 arch
#include "ssde_x64.h"
#include <cstdint>
#include <cstddef>
#include <vector>


// Major amounts of information this code was based on were taken from the
// "Intel(R) 64 and IA-32 Architectures Software Developer's Manual". If You
// are unfamiliar with the X86 architecture, it's recommended that you read
// the manuals first. The manuals can be obtained at
//   http://www.intel.com/content/www/us/en/processors/architectures-software-developer-manuals.html

using ssde::Inst_x64;
using std::vector;
using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;


namespace opcodes
{

enum : uint16_t
{
	none = 0,

	rm  = 1 << 0, // expect Mod byte
	ox  = 1 << 1, // expect Mod opcode extension + change behaviour of REX.B
	rel = 1 << 2, // instruction's imm is a relative address
	i8  = 1 << 3, // has  8 bit imm
	i16 = 1 << 4, // has 16 bit imm
	i32 = 1 << 5, // has 32 bit imm, which can be turned to 16 with 66 prefix
	rw  = 1 << 6, // supports REX.W
	am  = 1 << 7, // instruction uses address mode, imm is a memory address
	vx  = 1 << 8, // instruction requires a VEX prefix
	mp  = 1 << 9, // instruction has a mandatory 66 prefix

	ex  = rm  | ox,
	r8  = i8  | rel,
	r32 = i32 | rel,

	error = 0xffff
};

// 1st opcode flag table
static const uint16_t table[256] =
{
	//x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error, // 0x
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error, // 1x
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error, // 2x
	  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error,  rm  ,  rm  ,  rm  ,  rm  ,  i8  ,  i32 , error, error, // 3x
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, // 4x
	 none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , none , // 5x
	 error, error, error,  rm  , error, error, error, error,  i32 ,rm|i32,  i8  , rm|i8, none , none , none , none , // 6x
	  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  ,  r8  , // 7x
	 ex|i8,ex|i32, error, ex|i8,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  ex  , // 8x
	 none , none , none , none , none , none , none , none , none , none , error, error, none , none , none , none , // 9x
	  am  ,  am  ,  am  ,  am  , none , none , none , none ,  i8  ,  i32 , none , none , none , none , none , none , // Ax
	  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,  i8  ,rw|i32,rw|i32,rw|i32,rw|i32,rw|i32,rw|i32,rw|i32,rw|i32, // Bx
	 ex|i8, ex|i8,  i16 , none , error, error, ex|i8,ex|i32,i16|i8, none ,  i16 , none , none ,  i8  , none , none , // Cx
	  ex  ,  ex  ,  ex  ,  ex  , error, error, error, none ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  , // Dx
	  r8  ,  r8  ,  r8  ,  r8  ,  i8  ,  i8  ,  i8  ,  i8  ,  r32 ,  r32 , error,  r8  , none , none , none , none , // Ex
	 none , none , error, error, none , none , error, error, none , none , none , none , none , none ,  rm  ,  ex  , // Fx
};

// 2nd opcode flag table
// 0F xx
static const uint16_t table_0f[256] =
{
	//x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF
	  ex  ,  ex  ,  rm  ,  rm  , error, error, none , error, none , none , error, none , error,  rm  , none , error, // 0x
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  ex  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  ex  , // 1x
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error,  rm  , error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // 2x
	 none , none , none , none , none , none , error, none , error, error, error, error, error, error, error, error, // 3x
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // 4x
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // 5x
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // 6x
	 rm|i8, ex|i8, ex|i8, ex|i8,  rm  ,  rm  ,  rm  , none ,  rm  ,  rm  , error, error,  rm  ,  rm  ,  rm  ,  rm  , // 7x
	  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 ,  r32 , // 8x
	  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  ,  ex  , // 9x
	 none , none , none ,  rm  , rm|i8,  rm  , error, error, none , none , none ,  rm  , rm|i8,  rm  ,  ex  ,  rm  , // Ax
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , none , ex|i8,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // Bx
	  rm  ,  rm  , rm|i8,  rm  , rm|i8, rm|i8, rm|i8,  ex  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // Cx
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // Dx
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // Ex
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // Fx
};

// 3rd opcode flag table
// 0F 38 xx
static const uint16_t table_38[256] =
{
	//x0  |  x1  |  x2  |  x3  |  x4  |  x5  |  x6  |  x7  |  x8  |  x9  |  xA  |  xB  |  xC  |  xD  |  xE  |  xF
	  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , vx|rm, vx|rm, error, error, // 0x
	 mp|rm, error, error, error, mp|rm, mp|rm, error, mp|rm, vx|rm, error, vx|rm, error,  rm  ,  rm  ,  rm  , error, // 1x
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, error, error, mp|rm, mp|rm, mp|rm, mp|rm, vx|rm, vx|rm, error, error, // 2x
	 mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, error, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, mp|rm, // 3x
	 mp|rm, mp|rm, error, error, error, error, error, error, error, error, error, error, error, error, error, error, // 4x
	 error, error, error, error, error, error, error, error, vx|rm, vx|rm, error, error, error, error, error, error, // 5x
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, // 6x
	 error, error, error, error, error, error, error, error, vx|rm, vx|rm, error, error, error, error, error, error, // 7x
	 mp|rm, mp|rm, error, error, error, error, error, error, error, error, error, error, error, error, error, error, // 8x
	 error, error, error, error, error, error, vx|rm, vx|rm, vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, // 9x
	 error, error, error, error, error, error, vx|rm, vx|rm, vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, // Ax
	 error, error, error, error, error, error, vx|rm, vx|rm, vx|rm, error, vx|rm, error, vx|rm, error, vx|rm, error, // Bx
	 error, error, error, error, error, error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , error, error, // Cx
	 error, error, error, error, error, error, error, error, error, error, error,  rm  ,  rm  ,  rm  ,  rm  ,  rm  , // Dx
	 error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, error, // Ex
	  rm  ,  rm  , error, error, error, error,  rm  , error, error, error, error, error, error, error, error, error, // Fx
};

// 3rd opcode flag table
// 0F 3A xx
static const uint16_t table_3a[256] =
{
	// x0   |   x1   |   x2   |   x3   |   x4   |   x5   |   x6   |   x7   |   x8   |   x9   |   xA   |   xB   |   xC   |   xD   |   xE   |   xF
	  error ,  error ,  error ,  error ,  error ,  error ,vx|rm|i8,  error ,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,   rm   , // 0x
	  error ,  error ,  error ,  error ,mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,vx|rm|i8,vx|rm|i8,  error ,  error ,  error ,  error ,  error ,  error , // 1x
	mp|rm|i8,mp|rm|i8,mp|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 2x
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 3x
	  mp|rm ,  mp|rm ,mp|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error ,vx|rm|i8,vx|rm|i8,vx|rm|i8,  error ,  error ,  error , // 4x
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 5x
	mp|rm|i8,mp|rm|i8,mp|rm|i8,mp|rm|i8,  error ,  error ,  error ,  error ,vx|rm|i8,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 6x
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 7x
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 8x
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // 9x
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // Ax
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // Bx
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,mp|rm|i8,  error ,  error ,  error , // Cx
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // Dx
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // Ex
	  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error ,  error , // Fx
};

} // namespace opcodes

void Inst_x64::internal_decode(const vector<uint8_t>& buffer)
{
	decode_prefixes(buffer);
	decode_opcode(buffer);

	if (flags != opcodes::error)
	{
		if ((flags & opcodes::mp) && prefixes[2] != Prefix::p66)
		{
			// this instruction lacks mandatory 66 prefix

			signal_error(Error::opcode);
		}

		if (flags & opcodes::rm)
		{
			decode_modrm(buffer);

			if (has_sib)
				decode_sib(buffer);

			rex_extend_modrm();

			if (has_disp)
				read_disp(buffer);
		}
		else if (prefixes[0] == Prefix::lock)
		{
			// LOCK prefix only makes sense for Mod M

			signal_error(Error::lock);
		}

		// read moffs, imm or rel
		read_imm(buffer);

		if (length > 15)
		{
			length = 15;
			signal_error(Error::length);
		}
	}
	else
	{
		length = 1;
		signal_error(Error::opcode);
	}
}

void Inst_x64::decode_prefixes(const vector<uint8_t>& buffer)
{
	// This is prefix analyzer. It behaves exactly the same way real CPUs
	// analyze instructions for prefixes. Normally, each instruction is
	// allowed to have up to 4 prefixes from each group. Though, in cases when
	// instruction has more prefixes, it will ignore any prefix it meets if
	// there was a prefix from the same group before it. Instruction decoders
	// can only handle words up to 15 bytes long, if the word is longer than
	// that, decoder will fail.

	for (int32_t i = 0; i < 15; ++i, ++length)
	{
		Prefix pref = static_cast<Prefix>(peek_byte(buffer));

		if (pref == Prefix::lock ||
		    pref == Prefix::repnz ||
		    pref == Prefix::repz)
		{
			if (prefixes[0] == Prefix::none)
				prefixes[0] = pref;

			has_rex = false;
		}
		else if (pref == Prefix::seg_cs || pref == Prefix::seg_ss ||
		         pref == Prefix::seg_ds || pref == Prefix::seg_es ||
		         pref == Prefix::seg_fs || pref == Prefix::seg_gs)
		{
			if (prefixes[1] == Prefix::none)
				prefixes[1] = pref;

			has_rex = false;
		}
		else if (pref == Prefix::p66)
		{
			if (prefixes[2] == Prefix::none)
				prefixes[2] = pref;

			has_rex = false;
		}
		else if (pref == Prefix::p67)
		{
			if (prefixes[3] == Prefix::none)
				prefixes[3] = pref;

			has_rex = false;
		}
		else if ((static_cast<uint8_t>(pref) & 0xf0) == 0x40)
		{
			// Unlike all legacy prefixes, if CPU meets multiple of REX
			// prefixes, it will only take the last one into account.
			// REX prefixes before legacy ones are silently ignored.

			has_rex = true;

			uint8_t rex_byte = static_cast<uint8_t>(pref);

			rex_W = (rex_byte & 0x08) ? true : false;
			rex_R = (rex_byte & 0x04) ? true : false;
			rex_X = (rex_byte & 0x02) ? true : false;
			rex_B = (rex_byte & 0x01) ? true : false;
		}
		else
		{
			break;
		}
	}
}

void Inst_x64::decode_opcode(const vector<uint8_t>& buffer)
{
	uint8_t byte_0 = peek_byte(buffer);

	if (byte_0 == 0xc4 ||
	    byte_0 == 0xc5 ||
	    byte_0 == 0x62)
	{
		// looks like we've found a VEX prefix

		decode_vex(buffer);
	}
	else
	{
		opcode[0] = get_byte(buffer);
		opcode[1] = opcode[0] == 0x0f ? get_byte(buffer) : 0;
		opcode[2] = (opcode[1] == 0x38 || opcode[1] == 0x3a) ?
		            get_byte(buffer) : 0;
	}

	if (opcode[0] != 0x0f)
	{
		opcode_length = 1;
		flags = opcodes::table[opcode[0]];
	}
	else
	{
		switch (opcode[1])
		{
		default:
			opcode_length = 2;
			flags = opcodes::table_0f[opcode[1]];
			break;

		case 0x38:
			opcode_length = 3;
			flags = opcodes::table_38[opcode[2]];
			break;

		case 0x3a:
			opcode_length = 3;
			flags = opcodes::table_3a[opcode[2]];
			break;
		}
	}

	if (!has_vex && (flags & opcodes::vx))
	{
		// this instruction can only be VEX-encoded

		signal_error(Error::no_vex);
	}


	// These are two exceptional opcodes that extend using 3 bits of Mod R/M
	// byte and they lack consistent flags. Instead of creating a new flags
	// table for each extended opcode, I decided to put this little bit of code
	// that is dedicated to these two exceptional opcodes.

	if (opcode[0] == 0xf6)
	{
		uint8_t op_ex = (peek_byte(buffer) >> 3) & 0x07;

		if (op_ex == 0x00 || op_ex == 0x01)
			flags = opcodes::rm | opcodes::i8;
		else
			flags = opcodes::rm;
	}
	else if (opcode[0] == 0xf7)
	{
		uint8_t op_ex = (peek_byte(buffer) >> 3) & 0x07;

		if (op_ex == 0x00 || op_ex == 0x01)
			flags = opcodes::rm | opcodes::i32;
		else
			flags = opcodes::rm;
	}
}

void Inst_x64::decode_vex(const vector<uint8_t>& buffer)
{
	has_vex = true;

	if (has_prefix())
		signal_error(Error::opcode);

	if (has_rex)
		signal_error(Error::rex);

	uint8_t byte_0 = get_byte(buffer);

	if (byte_0 == 0xc5)
	{
		vex_size = 2;

		uint8_t byte_1 = get_byte(buffer);

		rex_R = (byte_1 & 0x80) ? false : true;
		vex_L = (byte_1 & 0x04) ? true : false;
		// determine destination register from vvvv
		vex_reg = (~byte_1 >> 3) & 0x0f;

		opcode[0] = 0x0f;
		opcode[1] = get_byte(buffer);

		// read prefix bytes from pp field
		vex_decode_pp(byte_1 & 0x03);

		vex_vec_bits = 128;
	}
	else if (byte_0 == 0xc4)
	{
		vex_size = 3;

		uint8_t byte_1 = get_byte(buffer);
		uint8_t byte_2 = get_byte(buffer);

		// 3 byte VEX stores REX bits inverted in 2nd byte
		rex_R = (byte_1 & 0x80) ? false : true;
		rex_X = (byte_1 & 0x40) ? false : true;
		rex_B = (byte_1 & 0x20) ? false : true;
		rex_W = (byte_2 & 0x80) ? true : false;
		vex_L = (byte_2 & 0x04) ? true : false;
		// determine destination register from vvvv
		vex_reg = (~byte_2 >> 3) & 0x0f;

		// read opcode bytes from mm field
		vex_decode_mm(byte_1 & 0x1f);
		opcode[opcode[1] != 0 ? 2 : 1] = get_byte(buffer);

		// read prefix bytes from pp field
		vex_decode_pp(byte_2 & 0x03);

		vex_vec_bits = 128;
	}
	else if (byte_0 == 0x62)
	{
		vex_size = 4;

		uint8_t byte_1 = get_byte(buffer);
		uint8_t byte_2 = get_byte(buffer);
		uint8_t byte_3 = get_byte(buffer);

		rex_R  = (byte_1 & 0x80) ? true : false;
		rex_X  = (byte_1 & 0x40) ? true : false;
		rex_B  = (byte_1 & 0x20) ? true : false;
		vex_RR = (byte_1 & 0x10) ? true : false;

		vex_decode_mm(byte_1 & 0x03);
		opcode[opcode[1] != 0 ? 2 : 1] = get_byte(buffer);

		rex_W = (byte_2 & 0x80) ? true : false;

		// determine destination register from vvvv
		vex_reg = ((~byte_2 >> 3) & 0x0f) | ((byte_3 & 0x80) ? 0x10 : 0);

		vex_decode_pp(byte_2 & 0x03);

		vex_sae  = (byte_3 & 0x10) ? true : false;
		vex_L    = (byte_3 & 0x20) ? true : false;
		vex_LL   = (byte_3 & 0x40) ? true : false;
		vex_zero = (byte_3 & 0x80) ? true : false;

		vex_opmask = byte_3 & 0x07;

		if (vex_rc) // aka vex_sae
		{
			// rounding control, implies vector is 512 bits wide

			vex_round_to = static_cast<VEX_rm>((vex_L ? 0x1 : 0) |
			                                   (vex_LL ? 0x2 : 0));
			vex_L  = false;
			vex_LL = true;
		}
		else if (vex_L && vex_LL)
		{
			// TODO: Remove this block if AVX-1024 ever comes out
			// destination vector can't be wider than 512 bits

			signal_error(Error::operand);
		}
		// all cases were checked

		vex_vec_bits = 128 << (vex_L ? 0x1 : 0) | (vex_LL ? 0x2 : 0);
	}
	// byte_0 is guaranteed to be one of values in if cascade
}

void Inst_x64::vex_decode_pp(uint8_t pp)
{
	switch (pp)
	{
	case 0x01:
		prefixes[2] = Prefix::p66;
		break;

	case 0x02:
		prefixes[0] = Prefix::repz;
		break;

	case 0x03:
		prefixes[0] = Prefix::repnz;
		break;

	default:
		break;
	}
}

void Inst_x64::vex_decode_mm(uint8_t mm)
{
	switch (mm)
	{
	case 0x01:
		opcode[0] = 0x0f;
		break;

	case 0x02:
		opcode[0] = 0x0f;
		opcode[1] = 0x38;
		break;

	case 0x03:
		opcode[0] = 0x0f;
		opcode[1] = 0x3a;
		break;

	default:
		signal_error(Error::opcode);
		break;
	}
}

void Inst_x64::decode_modrm(const vector<uint8_t>& buffer)
{
	uint8_t modrm_byte = get_byte(buffer);

	has_modrm = true;
	modrm_mod = static_cast<RM_mode>((modrm_byte >> 6) & 0x03);
	modrm_reg = (modrm_byte >> 3) & 0x07;
	modrm_rm  = modrm_byte & 0x07;

	switch (modrm_mod)
	{
	case RM_mode::mem:
		if (prefixes[3] == Prefix::p67)
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

	case RM_mode::mem_disp8:
		{
			if (prefixes[3] != Prefix::p67 && modrm_rm == 0x04)
				has_sib = true;

			has_disp  = true;
			disp_size = 1;
		}
		break;

	case RM_mode::mem_disp32:
		{
			if (prefixes[3] != Prefix::p67 && modrm_rm == 0x04)
				has_sib = true;

			has_disp  = true;
			disp_size = prefixes[3] != Prefix::p67 ? 4 : 2;
		}
		break;

	case RM_mode::reg:
		if (prefixes[0] == Prefix::lock)
		{
			// LOCK prefix is not allowed to be used with Mod R

			signal_error(Error::lock);
		}
		break;
	}
}

void Inst_x64::decode_sib(const vector<uint8_t>& buffer)
{
	uint8_t sib_byte = get_byte(buffer);

	sib_scale = 1U << ((sib_byte >> 6) & 0x03);
	sib_index = (sib_byte >> 3) & 0x07;
	sib_base  = sib_byte & 0x07;
}

void Inst_x64::rex_extend_modrm()
{
	if (has_sib)
	{
		modrm_reg |= rex_R ? 0x08 : 0;

		sib_index |= rex_X ? 0x08 : 0;
		sib_base  |= rex_B ? 0x08 : 0;
	}
	else
	{
		if (flags & opcodes::ox)
		{
			// Mod extended opcodes are extended differently

			modrm_reg |= rex_B ? 0x08 : 0;
		}
		else
		{
			modrm_reg |= rex_R ? 0x08 : 0;
			modrm_rm  |= rex_B ? 0x08 : 0;
		}
	}
}

void Inst_x64::read_disp(const vector<uint8_t>& buffer)
{
	disp = 0;

	for (int32_t i = 0; i < disp_size; ++i)
		disp |= static_cast<int32_t>(get_byte(buffer)) << i*8;

	if (disp & (1U << (disp_size*8 - 1)))
	{
		switch (disp_size)
		{
		default:
			break;

		case 1:
			disp |= 0xffffff00;
			break;

		case 2:
			disp |= 0xffff0000;
			break;
		}
	}
}

void Inst_x64::read_imm(const vector<uint8_t>& buffer)
{
	if (flags & opcodes::am)
	{
		// address mode instructions use a different prefix

		has_imm  = true;
		imm_size = prefixes[3] != Prefix::p67 ? 8 : 4;
	}
	else
	{
		if (flags & opcodes::i32)
		{
			has_imm  = true;
			imm_size = (rex_W && (flags & opcodes::rw)) ? 8 :
			           prefixes[2] != Prefix::p66 ? 4 : 2;
		}

		if (flags & opcodes::i16)
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

		if (flags & opcodes::i8)
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

		for (int32_t i = 0; i < imm_size; ++i)
			imm |= static_cast<uint64_t>(get_byte(buffer)) << i*8;

		if (has_imm2)
		{
			imm2 = 0;

			for (int32_t i = 0; i < imm2_size; ++i)
				imm2 |= static_cast<uint64_t>(get_byte(buffer)) << i*8;
		}
	}

	if (flags & opcodes::rel)
	{
		has_imm = false;

		rel_size = imm_size;
		rel = static_cast<int32_t>(imm) + static_cast<int32_t>(length);

		if (rel & (1U << (rel_size*8 - 1)))
		{
			switch (rel_size)
			{
			default:
				break;

			case 1:
				rel |= 0xffffff00;
				break;

			case 2:
				rel |= 0xffff0000;
				break;
			}
		}

		has_rel = true;
	}
}