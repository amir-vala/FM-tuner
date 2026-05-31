#include <dshow.h>
#include "VirtualRadioSource.h"

// g_Templates و sudFilter در VirtualRadioSource.cpp تعریف شده‌اند.
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

STDAPI DllRegisterServer()
{
    // BaseClasses helper will read g_Templates and write registry entries
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}
