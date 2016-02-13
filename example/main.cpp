// This file is usage demo for SSDE (http://github.com/notnanocat/ssde)
#include "../ssde/ssde.hpp"
#include "../ssde/inst_x86.hpp"
#include "../ssde/inst_x64.hpp"
#include "../ssde/inst_arm.hpp"
#include <iostream>
#include <iomanip>
#include <string>


int main(int argc, const char* argv[])
{
	using namespace std;

	ios::sync_with_stdio(false);


	/*const string bc_arm {"\x01\x20\x40\xe2"
	                     "\x02\x20\x61\xe0"
	                     "\x01\x30\xd1\xe4"
	                     "\x00\x00\x53\xe3"
	                     "\x02\x30\xc1\xe7"
	                     "\xfb\xff\xff\x1a"
	                     "\x1e\xff\x2f\xe1", 4*7};*/

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

	ssde::Disasm<ssde::Inst_x86> dis {bc_x86};

	while (dis.has_next())
	{
		auto inst = dis.decode();

		cout << setfill('0') << setw(8) << hex << inst.pc << ": ";

		for (int i = 0; i < inst.length; ++i)
			cout << setfill('0') << setw(2) << hex << (static_cast<int>(bc_x86[inst.pc+i]) & 0xff);
		
		if (inst.has_rel)
			cout << " # -> " << setfill('0') << setw(8) << hex << inst.rel_abs;

		cout << "\n";

		dis.next();
	}

	return 0;
}
