**The Small Scalable Disassembler Engine**

Copyright (C) 2015, Constantine Shablya. See Copyright Notice in LICENSE.md

SSDE is a small, scalable disassembly stream engine, purposed to analyze
machine code and retrieve information on instructions (their length, opcode,
correctness, etc).

Check _doc/manual_en.txt_ or _doc/manual_ru.txt_ for information about SSDE
and documentation in the language you speak.

Check _example/_ to see how SSDE can be used.

         Supported architectures and extensions
	 ______________________________________________
	|     |                                        |
	| x86 | VMX, AES, SHA, MMX                     |
	|  /  | SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2 |
	| x64 | AVX, AVX2, AVX512, FMA3                |
	|_____|________________________________________|
	|     |*WIP*                                   |
	| ARM | Thumb, Thumb-2                         |
	|     |                                        |
	|_____|________________________________________|

           List of machines SSDE was tested on
	 ______________________________________________
	|                   |                  |       |
	| Core i3 2350m     | Windows 8.1 x64  | 64 EL |
	| ARMv6 processor   | Raspbian 7       | 32 EL |
	|___________________|__________________|_______|
