#include <dshow.h>
#include "VirtualRadioSource.h"

// Minimal AMOVIESETUP structures (no pins defined here; dynamic)
AMOVIESETUP_MEDIATYPE sudPinTypes = {
    &MEDIATYPE_Audio, &MEDIASUBTYPE_NULL
};

AMOVIESETUP_PIN sudPins[] = {
    { L"Out", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 1, &sudPinTypes }
};

AMOVIESETUP_FILTER sudFilter = {
    &CLSID_VirtualRadioSource,
    L"Virtual Radio Source",
    MERIT_DO_NOT_USE+1,
    1, sudPins
};

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

STDAPI DllRegisterServer() {
    HRESULT hr = AMovieDllRegisterServer2(TRUE);
    return hr;
}

STDAPI DllUnregisterServer() {
    return AMovieDllRegisterServer2(FALSE);
}
