/*
* This file is usage demo for SSDE (http://github.com/notnanocat/ssde)
*/
#include "../ssde/ssde.hpp"
#include "../ssde/ssde_x86.hpp"
#include "../ssde/ssde_x64.hpp"
#include "../ssde/ssde_arm.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>


int main(int argc, const char *argv[])
{
	using namespace std;

	ios::sync_with_stdio(false);


	const string bc("\x01\x20\x40\xe2"
	                "\x02\x20\x61\xe0"
	                "\x01\x30\xd1\xe4"
	                "\x00\x00\x53\xe3"
	                "\x02\x30\xc1\xe7"
	                "\xfb\xff\xff\x1a"
	                "\x1e\xff\x2f\xe1", 4*7);

	for (ssde_arm dis(bc); dis.dec(); dis.next())
		/*
		* call ::next() to iterate and ::dec() to
		* decode and decide whether we have reached
		* the end of the buffer or not
		*/
	{
		/* output address of the instruction */
		cout << setfill('0') << setw(8) << hex << dis.pc << ": ";

		for (int i = 0; i < dis.length; ++i)
			/* output instruction's bytes */
		{
			cout << setfill('0') << setw(2) << hex << ((int)bc[dis.pc + i] & 0xff);
		}

		if (dis.is_branch)
			/* if this is a branch instruction, print where it points to */
		{
			cout << " # -> " << setfill('0') << setw(8) << hex << dis.abs;
		}

		cout << '\n';
	}

	return 0;

#if 0
	const string bc =
		"\x55"
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
		"\xc3";

	for (ssde_x86 dis(bc); dis.dec(); dis.next())
		/*
		* call ::next() to iterate and ::dec() to
		* decode and decide whether we have reached
		* the end of the buffer or not
		*/
	{
		/* output address of the instruction */
		cout << setfill('0') << setw(8) << hex << dis.ip << ": ";

		for (int i = 0; i < dis.length; ++i)
			/* output instruction's bytes */
		{
			cout << setfill('0') << setw(2) << hex << ((int)bc[dis.ip + i] & 0xff);
		}

		if (dis.has_rel)
			/* if this instruction has relative address, print where it points to */
		{
			cout << " ; -> " << setfill('0') << setw(8) << hex << dis.abs;
		}

		cout << '\n';
	}

	return 0;
#endif
}
