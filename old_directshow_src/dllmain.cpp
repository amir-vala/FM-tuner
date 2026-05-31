#include <windows.h>
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
HINSTANCE g_hInst = nullptr;

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_hInst = hInst;
        DisableThreadLibraryCalls(hInst);
    }
    return DllEntryPoint(hInst, reason, nullptr);
}
