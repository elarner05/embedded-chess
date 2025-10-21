
// File created: Sept 2025

#pragma once
#include <Arduino.h>
#include "Types.h"
#include "Draw.h"
#include "Game.h"

// Notation constants and variables
constexpr uint16_t MAX_PLY = 512;
constexpr uint16_t MAX_LINES_PER_PANE = 25;



// Singleton, used to track notation buttons, 1 byte
struct ButtonState {
  byte up : 1;
  byte down : 1;
  byte left : 1;
  byte right : 1;
  byte : 4;
};

struct NotationState {
  struct Ply moves[MAX_PLY]; // Allocates memory for storing moves for notation

  uint16_t currentPlyNumber;
  uint16_t lastDrawnPlyNumber;

  uint8_t notationPaneNo;

  struct ButtonState states;

};

extern struct NotationState notation;



void initNotationState();
void similarPieces(struct GameState &game, Square targetSquare, Piece targetType, Square dst[9]);
void updateNotation();