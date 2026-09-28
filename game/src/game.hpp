#pragma once

#include <engine/game.hpp>
#include <engine/engine.hpp>
#include <stdio.h>


class Game : public IGame {
public:
    void init(IEngine* engine) override {
        printf("TestGame::init()\n");
        this->engine = engine;
    }          
    void shutdown() override {
        printf("TestGame::shutdown()\n");
    }
    void update(float dt) override;

private:
    IEngine* engine;
};
