#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include "../ssde/ssde_x86.h"
#include "../ssde/ssde_x64.h"
#include "../ssde/ssde_arm.h"


int main(int argc, const char* argv[])
{
	using namespace std;
	using namespace ssde;

	ios::sync_with_stdio(false);


	const vector<uint8_t> bc =
	{
		0x55,                   // push  ebp
		0x31, 0xd2,             // xor   edx, edx
		0x89, 0xe5,             // mov   ebp, esp
		0x8b, 0x45, 0x08,       // mov   eax, [ebp+0x8]
		0x56,                   // push  esi
		0x8b, 0x75, 0x0c,       // mov   esi, [ebp+0xc]
		0x53,                   // push  ebx
		0x8d, 0x58, 0xff,       // lea   ebx, [eax-0x1]
		0x0f, 0xb6, 0x0c, 0x16, // movzx ecx, byte ptr [esi+edx] <- loop
		0x88, 0x4c, 0x13, 0x01, // mov   [ebx+edx+0x1], cl             ^
		0x83, 0xc2, 0x01,       // add   edx, 0x1                      |
		0x84, 0xc9,             // test  cl, cl                        |
		0x75, 0xf1,             // jne   loop --------------------------
		0x5b,                   // pop   ebx
		0x5e,                   // pop   esi
		0x5d,                   // pop   ebp
		0xc3,                   // ret
	};

	for (size_t i = 0; i < bc.size(); 0)
	{
		ssde::Inst_x86 inst{bc, i};

		cout << setfill('0') << setw(8) << hex << i << ": ";

		for (int32_t j = 0; j < inst.length; ++j)
			cout << setfill('0') << setw(2) << hex << (static_cast<int32_t>(bc.at(i+j)) & 0xff);

		if (inst.has_rel)
			cout << " # -> " << setfill('0') << setw(8) << hex << (static_cast<int32_t>(i) + inst.rel);

		cout << "\n";

		i += inst.length;
	}

	return 0;
}
