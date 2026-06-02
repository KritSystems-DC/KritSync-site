#include "FasolaFlintScaler/PluginModule.h"

#include <windows.h>

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
    }

    return TRUE;
}

extern "C" __declspec(dllexport) void Plugin_Init() {
    FasolaFlintScaler::GetPluginModule().Load();
}

extern "C" __declspec(dllexport) void Plugin_Unload() {
    FasolaFlintScaler::GetPluginModule().Unload();
}
