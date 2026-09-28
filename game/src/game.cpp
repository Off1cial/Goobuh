#include <engine/game.hpp>
#include "game.hpp"
#include <stdio.h>


void Game::update(float dt) {
    //printf("Frametime = %0.2f\n", dt);
}

GAME_ENTRY {
    printf("Hello from game_entry!\n");
    Game* game = new Game();

    return game;
}


