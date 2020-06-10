#include <thread>
#include "CustomAPI.h"


using RunFileInternal_t = int(*)(uint64_t, const char*, std::function<int(char*)>);
using LoadSystemFileInternal_t = int(*)(uint64_t, const char*);
char LSFIShell[] = { 0x55, 0x56, 0x57, 0x53, 0x48, 0x83, 0xEC, 0x38, 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69 };

uint64_t csLuaBase;
uint64_t grabbedInstance;
uint64_t allocLSFI;


int _fastcall LoadSystemFileInternal(uint64_t luaRuntime, const char* scriptFile) {
    if (!allocLSFI) {
        allocLSFI = reinterpret_cast<uint64_t>(VirtualAlloc(NULL, sizeof(LSFIShell), MEM_COMMIT, 0x40));
        memcpy((void*)allocLSFI, (void*)LSFIShell, sizeof(LSFIShell));
        *(uint64_t*)(allocLSFI + 14) = csLuaBase + 0x27998;
    }

    return ((LoadSystemFileInternal_t)(allocLSFI))(luaRuntime, scriptFile);
}

int LoadSystemFile(uint64_t luaRuntime, const char* scriptFile) {
    *(BYTE*)(CustomAPI::GetModuleA("adhesive") + 0x471448) = 1;
    auto result = ((RunFileInternal_t)(csLuaBase + 0x27A80))(luaRuntime, scriptFile, std::bind(&LoadSystemFileInternal, luaRuntime, std::placeholders::_1));
    return result;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  callReason, LPVOID lpReserved ){
    if (callReason == DLL_PROCESS_ATTACH) {
        std::thread([&] {
            while (!csLuaBase)
                csLuaBase = CustomAPI::GetModuleA("citizen-scripting-lua");
            
            for (;;) {
                uint64_t* c1 = (uint64_t*)(csLuaBase + 0x60CE70);
                if (*c1 != 0)
                    grabbedInstance = *c1;

                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        }).detach();

        std::thread([&] {

            for (;;) {
                if (!grabbedInstance)
                    std::this_thread::sleep_for(std::chrono::seconds(5));

                if (GetAsyncKeyState(VK_F5)) {
                    LoadSystemFile(grabbedInstance, "C:/memes/test.lua");
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

        }).detach();
    }

    return TRUE;
}

