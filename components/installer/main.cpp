// installer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "installer.h"
#include <iostream>
 void debug_main();

 extern std::string g_data ;

extern "C" {
	// module exports file: $(ProjectDir)\exports.def
	int32_t __stdcall TestFunc(char* p)
	{
		g_data = "DEFAULT";
		debug_main();

		char buf[256];
		sprintf_s(buf, "0x%X\n", p);
		MessageBoxA(0, buf, 0, 0);

		if (!p )
			return g_data.length() + 1;

		strcpy(p, g_data.c_str());
		// debug_main();
		return g_data.length() + 1;
	}
}