#include <stdio.h>
#include <engine/game.hpp>
#include <engine/engine.hpp>
#include "net/net.hpp"
#include "net/networkmanager.h"
#include "assetmanager.hpp"


#include "engine.hpp"
#include <dlfcn.h>

int main(){
    using game_entry_t = IGame* (*)();

    void* dllib = dlopen("lib/libgame.so", RTLD_LAZY);

    if (!dllib) {
    printf("Failed to load game library: %s\n", dlerror());
    return 1;
    }

    auto game_entry =
    reinterpret_cast<game_entry_t>(dlsym(dllib, "game_entry"));

    if (!game_entry) {
        printf("Failed to find game_entry: %s\n", dlerror());
        return 1;
    }

    Engine* engine = new Engine();
    g_NetworkManager = new CNetworkManager;
    g_NetworkManager->Init();
    if (g_NetworkManager->StartServer()){
        printf("Server\n");
    }
    if (g_NetworkManager->StartClient()){
        printf("Client\n");
    }

    g_NetworkManager->ConnectClient(
           "10.32.82.230",
           NET_SERVER_DEFAULT_PORT
            );

    const char* modelpath = "resource/models\0";
    
    CAssetManager* asmgr = new CAssetManager();
    asmgr->Init("resource/models", "resource/sounds", nullptr);
    IGame* game = game_entry();

    // Init Engine
    engine->LoadGame(game);
    // Init Game
    game->init(engine);

    engine->Run();

    return 0;
}
