// This file is usage demo for SSDE (http://github.com/notnanocat/ssde)
#include "../ssde/ssde_x86.hpp"
#include "../ssde/ssde_x64.hpp"
#include "../ssde/ssde_arm.hpp"
#include <iostream>
#include <iomanip>
#include <string>


int main(int argc, const char* argv[])
{
	using namespace std;
	using namespace ssde;

	ios::sync_with_stdio(false);


	const string bc_x86 {"\x55"
	                     "\x31\xd2"
	                     "\x89\xe5"
	                     "\x8b\x45\x08"
	                     "\x56"
	                     "\x8b\x75\x0c"
	                     "\x53"
	                     "\x8d\x58\xff"
	                     "\x0f\xb6\x0c\x16"
	                     "\x88\x4c\x13\x01"
	                     "\x83\xc2\x01"
	                     "\x84\xc9"
	                     "\x75\xf1"
	                     "\x5b"
	                     "\x5e"
	                     "\x5d"
	                     "\xc3"};

	for (size_t i = 0; i < bc_x86.length(); 0)
	{
		ssde::Inst_x86 inst {bc_x86, i};

		cout << setfill('0') << setw(8) << hex << inst.ip << ": ";

		for (int32_t j = 0; j < inst.length; ++j)
			cout << setfill('0') << setw(2) << hex << (static_cast<int32_t>(bc_x86[i+j]) & 0xff);

		if (inst.has_rel)
			cout << " # -> " << setfill('0') << setw(8) << hex << inst.rel_abs;

		cout << "\n";

		i += inst.length;
	}

	return 0;
}
