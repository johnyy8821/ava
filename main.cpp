#include <windows.h>
#include <thread>
#include <chrono>
#include <cstdint>

// Определение структур для координат
struct Vector3D {
    float x, y, z;
};

// Прототипы функций игры (GTA SA)
typedef void(__cdecl* tRemoveLessUsedNodes)(int, int); // 0x53C500
typedef void(__cdecl* tRemoveAllUnusedModels)();      // 0x53C810
typedef void(__cdecl* tFunction40CF80)();             // 0x40CF80
typedef void(__cdecl* tFunction4090A0)();             // 0x4090A0
typedef void(__cdecl* tFunction5A18B0)();             // 0x5A18B0
typedef void(__cdecl* tFunction707770)();             // 0x707770
typedef void(__cdecl* tRequestCollision)(float, float); // 0x40AF40
typedef void(__cdecl* tLoadScene)(float, float, float); // 0x40EB70

// Адреса SAMP (для чата)
uintptr_t dwSAMPAddr = 0;

void AddChatMessage(const char* text, uint32_t color) {
    if (dwSAMPAddr == 0) return;
    
    // Смещение для SAMP 0.3.7-R1 (самая популярная версия для модов)
    // Если у вас R3 или R4, смещения будут другими.
    uintptr_t pChat = *(uintptr_t*)(dwSAMPAddr + 0x21A0E4); 
    if (pChat) {
        typedef void(__thiscall* tAddChat)(void*, int, const char*, const char*, uint32_t, uint32_t);
        tAddChat AddChat = (tAddChat)(dwSAMPAddr + 0x64010);
        AddChat((void*)pChat, 8, text, nullptr, color, 0);
    }
}

void CleanStreamMemoryBuffer() {
    ((tRemoveLessUsedNodes)0x53C500)(2, 2);
    ((tRemoveAllUnusedModels)0x53C810)();
    ((tFunction40CF80)0x40CF80)();
    ((tFunction4090A0)0x4090A0)();
    ((tFunction5A18B0)0x5A18B0)();
    ((tFunction707770)0x707770)();

    // Получаем координаты игрока (Ped pointer -> Matrix -> Pos)
    uintptr_t pPlayer = *(uintptr_t*)0xB7CD98;
    if (pPlayer) {
        Vector3D* pos = (Vector3D*)(pPlayer + 0x30); // 0x30 - смещение позиции в CPed
        if (pos) {
            ((tRequestCollision)0x40AF40)(pos->x, pos->y);
            ((tLoadScene)0x40EB70)(pos->x, pos->y, pos->z);
        }
    }
}

void ScriptThread() {
    // Ожидание загрузки SAMP
    while (dwSAMPAddr == 0) {
        dwSAMPAddr = (uintptr_t)GetModuleHandleA("samp.dll");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Проверка готовности SAMP (структура инициализирована)
    while (*(uintptr_t*)(dwSAMPAddr + 0x21A0E4) == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    AddChatMessage("[Cleaner] {d5dedd}- скрипт был успешно загружен. Автор: {01A0E9}Azller Lollison.", 0xFF01A0E9);
    AddChatMessage("[Cleaner] {d5dedd}Отдельное спасибо {01A0E9}DarkP1xel`у.", 0xFF01A0E9);

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        // Чтение текущего использования памяти стриминга
        uint32_t currentMem = *(uint32_t*)0x8E4CB4;
        if (currentMem > 419430400) { // 400 MB
            CleanStreamMemoryBuffer();
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        std::thread(ScriptThread).detach();
    }
    return TRUE;
}
