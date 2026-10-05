#include <stdio.h>

#include <engine/game.hpp>
#include <engine/engine.hpp>

#include "net/net.hpp"
#include "net/networkmanager.h"
#include "assetmanager.hpp"

#include "engine.hpp"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif


int main()
{
    using game_entry_t = IGame* (*)();

#ifdef _WIN32

    HMODULE dllib = LoadLibraryA(LIB_DIR "/libgame.dll");

    if (!dllib) {
        printf("Failed to load game library: %lu\n", GetLastError());
        return 1;
    }

    auto game_entry =
        reinterpret_cast<game_entry_t>(
            GetProcAddress(dllib, "game_entry")
        );

    if (!game_entry) {
        printf("Failed to find game_entry: %lu\n", GetLastError());
        FreeLibrary(dllib);
        return 1;
    }

#else

    void* dllib = dlopen(LIB_DIR "/libgame.so", RTLD_LAZY);

    if (!dllib) {
        printf("Failed to load game library: %s\n", dlerror());
        return 1;
    }

    auto game_entry =
        reinterpret_cast<game_entry_t>(dlsym(dllib, "game_entry"));

    if (!game_entry) {
        printf("Failed to find game_entry: %s\n", dlerror());
        dlclose(dllib);
        return 1;
    }

#endif

    Engine* engine = new Engine();

    g_NetworkManager = new CNetworkManager;
    g_NetworkManager->Init();

    if (g_NetworkManager->StartServer()) {
        printf("Server\n");
    }

    if (g_NetworkManager->StartClient()) {
        printf("Client\n");
    }

    g_NetworkManager->ConnectClient(
        "10.32.82.230",
        NET_SERVER_DEFAULT_PORT
    );



    IGame* game = game_entry();

    // Init Engine
    engine->LoadGame(game);
    
    // Init Game
    game->init(engine);

    engine->Run();

#ifdef _WIN32
    FreeLibrary(dllib);
#else
    dlclose(dllib);
#endif

    return 0;
}
