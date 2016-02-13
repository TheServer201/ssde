// SSDE base instruction and disassembler
// Copyright (C) 2015-2016, Constantine Shablya. See Copyright Notice in LICENSE.md
#pragma once
#include <string>
#include <type_traits>
#include <stdint.h>


namespace ssde
{

struct Inst;

template<class T_Inst> class Disasm
{
	static_assert(std::is_base_of<Inst, T_Inst>::value, "ssde::Disasm<T_Inst> only works with structs inherited from Inst");

public:
	Disasm(const std::string& raw_data, size_t start_pc = 0) :
		buffer(raw_data),
		pc(start_pc)
	{
	}

	T_Inst decode()
	{
		T_Inst inst { };
		inst.decode(buffer, pc);
		length = inst.length;

		return inst;
	}

	void next()
	{
		pc += length;
	}

	bool has_next() const
	{
		return get_next_pc() < buffer.length();
	}

	void set_pc(size_t new_pc)
	{
		pc     = new_pc;
		length = 0;
	}

	size_t get_pc() const
	{
		return pc;
	}

	size_t get_next_pc() const
	{
		return pc + length;
	}
	
public:
	size_t pc;
	int    length = 0;

	const std::string& buffer;
};


struct Inst
{
public:
	enum class Errors
	{
		opcode  = 1 << 0,
		operand = 1 << 1,
		length  = 1 << 2,
	};


	virtual ~Inst() = default;


	void decode(const std::string& buffer)
	{
		try
		{
			internal_decode(buffer);
		}
		catch (const std::out_of_range&)
		{
			error = true;
			error_length = true;
		}
	}

	void decode(const std::string& buffer, size_t start_pc)
	{
		pc = start_pc;
		decode(buffer);
	}


	/* replace errors with enum */

	bool error;
	bool error_opcode;  // Opcode is invalid
	bool error_operand; // Operand/s are invalid
	bool error_length;  // Instruction length is either too long or too short

	size_t pc;
	int    length;

protected:
	virtual void internal_decode(const std::string&) = 0;
};

} // namespace ssde