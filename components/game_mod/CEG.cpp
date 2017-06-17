#include "stdafx.h"

#ifdef CEG_DEBUG
#define CEG_DEBUG_PRINT Log::Write("Region: 0x%X | Redirected: 0x%X | Return: 0x%X", address, dwNew, _ReturnAddress());
#else
#define CEG_DEBUG_PRINT
#endif

typedef void(__thiscall * sub_5CBF00_t)(void *thisptr, DWORD address, size_t scanSize);
sub_5CBF00_t sub_5CBF00_o;

typedef void(__thiscall * sub_661450_t)(void *thisptr, DWORD address, size_t scanSize);
sub_661450_t sub_661450_o;

typedef void(__thiscall * sub_4E9880_t)(void *thisptr, DWORD address, size_t scanSize);
sub_4E9880_t sub_4E9880_o;

typedef void(__thiscall * sub_640020_t)(void *thisptr, DWORD address, size_t scanSize);
sub_640020_t sub_640020_o;

typedef void(__thiscall * sub_51A140_t)(void *thisptr, DWORD address, size_t scanSize);
sub_51A140_t sub_51A140_o;

typedef void(__thiscall * sub_54A430_t)(void *thisptr, DWORD address, size_t scanSize);
sub_51A140_t sub_54A430_o;

LPVOID g_MemoryBuffer;
ULONG_PTR g_ImageBase;
ULONG_PTR g_ImageEnd;
ULONG_PTR g_ImageCodeSize;

void Patch_CEG()
{
	g_ImageBase = (ULONG_PTR)GetModuleHandle(nullptr);
	g_ImageCodeSize = 0x5A1C00;
	g_ImageEnd = g_ImageBase + g_ImageCodeSize;

	g_MemoryBuffer = VirtualAlloc(nullptr, g_ImageCodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memcpy(g_MemoryBuffer, (LPVOID)g_ImageBase, g_ImageCodeSize);

	sub_5CBF00_o = (sub_5CBF00_t)Detours::X86::DetourFunction((PBYTE)0x5CBF00, (PBYTE)&sub_5CBF00);
	sub_661450_o = (sub_661450_t)Detours::X86::DetourFunction((PBYTE)0x661450, (PBYTE)&sub_661450);
	sub_4E9880_o = (sub_4E9880_t)Detours::X86::DetourFunction((PBYTE)0x4E9880, (PBYTE)&sub_4E9880);
	sub_640020_o = (sub_640020_t)Detours::X86::DetourFunction((PBYTE)0x640020, (PBYTE)&sub_640020);
	sub_51A140_o = (sub_51A140_t)Detours::X86::DetourFunction((PBYTE)0x51A140, (PBYTE)&sub_51A140);
	sub_54A430_o = (sub_51A140_t)Detours::X86::DetourFunction((PBYTE)0x54A430, (PBYTE)&sub_54A430);

	Detours::X86::DetourFunction((PBYTE)0x967760, (PBYTE)&hk_memcpy);
	Detours::X86::DetourFunction((PBYTE)0x8EF04F, (PBYTE)&hk_inline_memcpy);
	Detours::X86::DetourFunction((PBYTE)0x8EF168, (PBYTE)&hk_inline_memcpy2);

	Detours::X86::DetourFunction((PBYTE)0x8EE640, (PBYTE)&sub_8EE640);	// Patch intentional nullptr
	Detours::X86::DetourFunction((PBYTE)0x9A2860, (PBYTE)&sub_9A2860);	// Patch HWBP check
	Detours::X86::DetourFunction((PBYTE)0x9A2800, (PBYTE)&sub_9A2800);	// Patch HWBP check
	Detours::X86::DetourFunction((PBYTE)0x9A2900, (PBYTE)&sub_9A2900);	// Patch HWBP check

	PatchMemory(0x004682B0, (PBYTE)"\xC3", 1);	// Buggy CEG MD5 checks
	FixupFunction(0x0060CC10, 0x004F20F0);		// CEGObfuscate<LiveStats_Init> => LiveStats_Init
	FixupFunction(0x00580460, 0x0079E6D0);		// CEGObfuscate<Con_Restricted_SetLists> => Con_Restricted_SetLists
}

DWORD __declspec(noinline) GetNewAddress(DWORD dwOld)
{
	if (dwOld > g_ImageBase && dwOld < g_ImageEnd)
		return ((DWORD)g_MemoryBuffer + (dwOld - g_ImageBase));

	return dwOld;
}

void *hk_memcpy(void *dest, const void *src, size_t size)
{
	PVOID dwNew = (PVOID)GetNewAddress((DWORD)src);

	return memcpy(dest, dwNew, size);
}

void __declspec(naked) hk_inline_memcpy()
{
	static DWORD dwCall = 0x8EE910;
	static DWORD dwJmp = 0x8EF056;

	__asm
	{
		push eax
		push ecx
		push edx

		push esi
		call GetNewAddress
		add esp, 4
		mov esi, eax

		pop edx
		pop ecx
		pop eax

		rep movs dword ptr es : [edi], dword ptr ds : [esi]
		call DWORD PTR ds : [dwCall]
		jmp DWORD PTR ds : [dwJmp]
	}
}

void __declspec(naked) hk_inline_memcpy2()
{
	static DWORD dwJmp = 0x8EF16E;

	__asm
	{
		push eax
		push ecx
		push edx

		push esi
		call GetNewAddress
		add esp, 4
		mov esi, eax

		pop edx
		pop ecx
		pop eax

		rep movs dword ptr es : [edi], dword ptr ds : [esi]
		mov ecx, dword ptr ss : [esp + 0x24]
		jmp DWORD PTR ds : [dwJmp]
	}
}

void *sub_8EE640(void *Nullptr1, void *Nullptr2)
{
	if (Nullptr1 != nullptr || Nullptr2 != nullptr)
		__debugbreak();

	*(void **)0xBA1C24 = Nullptr2;
	return (void *)0xBA1C24;
}

DWORD sub_9A2860(PEXCEPTION_RECORD a1, int a2, PCONTEXT a3)
{
	if ((a3->ContextFlags & 0x10010) == 0x10010)
	{
		if (a1->ExceptionCode != 0xE06D7363)
		{
			a1->ExceptionFlags = 0;
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}
	else
	{
		a3->ContextFlags |= 0x10010u;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

DWORD sub_9A2800(PEXCEPTION_RECORD a1, int a2, PCONTEXT a3)
{
	if (a1->ExceptionCode == 0xE06D7363 && a1->NumberParameters == 3)
		*(DWORD *)a1->ExceptionInformation[2] = a1->ExceptionInformation[1];

	a1->ExceptionFlags = 0;

	if ((a3->ContextFlags & 0x10010) != 0x10010)
		a3->ContextFlags |= 0x10010u;

	return EXCEPTION_CONTINUE_SEARCH;
}

DWORD sub_9A2900(PEXCEPTION_RECORD a1, int a2, PCONTEXT a3)
{
	DWORD tempDR7 = a3->Dr7;
	a3->Dr7 = 0;

	if ((a3->ContextFlags & 0x10010) == 0x10010 && LOBYTE(a3->Dr7))
	{
		DWORD v6 = *(DWORD *)a2;
		*(DWORD *)(v6 + 4) = 0x9A2228;
		*(DWORD *)(v6 + 8) = 0x9A28C0;

		a3->ContextFlags |= 0x10010u;
		a3->Dr7 = tempDR7;
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else if (a1->ExceptionCode == 0xC0000005)
	{
		// See: sub_8EE640
		a3->ContextFlags |= 0x10010u;
		a3->Dr7 = tempDR7;
		a3->Eax = 0xBA1C24;
		return EXCEPTION_CONTINUE_SEARCH;
	}

	a3->ContextFlags |= 0x10010u;
	a3->Dr7 = tempDR7;

	if (a1->ExceptionCode == 0xE06D7363)
	{
		a1->ExceptionFlags = 0;
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}

void __fastcall sub_5CBF00(void *thisptr, PVOID _EDX, DWORD address, size_t scanSize)
{
	DWORD dwNew = GetNewAddress(address);

	CEG_DEBUG_PRINT;

	sub_5CBF00_o(thisptr, dwNew, scanSize);
}

void __fastcall sub_661450(void *thisptr, PVOID _EDX, DWORD address, size_t scanSize)
{
	DWORD dwNew = GetNewAddress(address);

	CEG_DEBUG_PRINT;

	sub_661450_o(thisptr, dwNew, scanSize);
}

void __fastcall sub_4E9880(void *thisptr, PVOID _EDX, DWORD address, size_t scanSize)
{
	DWORD dwNew = GetNewAddress(address);

	CEG_DEBUG_PRINT;

	sub_4E9880_o(thisptr, dwNew, scanSize);
}

void __fastcall sub_640020(void *thisptr, PVOID _EDX, DWORD address, size_t scanSize)
{
	DWORD dwNew = GetNewAddress(address);

	CEG_DEBUG_PRINT;

	sub_640020_o(thisptr, dwNew, scanSize);
}

void __fastcall sub_51A140(void *thisptr, PVOID _EDX, DWORD address, size_t scanSize)
{
	DWORD dwNew = GetNewAddress(address);

	CEG_DEBUG_PRINT;

	sub_51A140_o(thisptr, dwNew, scanSize);
}

void __fastcall sub_54A430(void *thisptr, PVOID _EDX, DWORD address, size_t scanSize)
{
	DWORD dwNew = GetNewAddress(address);

	CEG_DEBUG_PRINT;

	sub_54A430_o(thisptr, dwNew, scanSize);
}