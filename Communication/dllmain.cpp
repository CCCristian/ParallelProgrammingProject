#include <winsock2.h>
#include "Error.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	(hModule);
	(ul_reason_for_call);
	(lpReserved);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		WSADATA wsadata;
		if (int error = WSAStartup(MAKEWORD(2, 2), &wsadata); error)
			return FALSE;
		if (!(LOBYTE(wsadata.wVersion) == 2 && HIBYTE(wsadata.wVersion) == 2))
			return FALSE;
			break;
    case DLL_PROCESS_DETACH:
		if (int error = WSACleanup(); error)
			return FALSE;
        break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	default: break;
    }

    return TRUE;
}
