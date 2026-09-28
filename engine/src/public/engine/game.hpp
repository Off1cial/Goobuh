// src/public/game.h

#pragma once
#include "engine/engine.hpp"

class IGame {
public:
  virtual ~IGame() = default;

  virtual void init(IEngine* engine) = 0;
  virtual void shutdown() = 0;
  virtual void update(float dt) = 0;
};

#define GAME_ENTRY extern "C" IGame *game_entry()
#define GAME_UPDATE extern "C" void game_update(float dt)
extern "C" IGame *create_game();
extern "C" void update_game(float dt);