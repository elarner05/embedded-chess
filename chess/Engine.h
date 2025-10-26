#pragma once
#include <Arduino.h>

#include "Types.h"
#include "Game.h"


int evaluatePosition(const struct GameState &game);
struct Ply engineMove(const struct GameState &game);
