
// File created: Sept 2025

#pragma once
#include <Arduino.h>
#include "Types.h"
#include "Draw.h"
#include "Game.h"

// Notation constants and variables
constexpr uint16_t MAX_PLY = 512;
constexpr uint16_t MAX_LINES_PER_PANE = 25;

struct _PackedRow {
  byte a : 4;
  byte b : 4;
  byte c : 4;
  byte d : 4;
  byte e : 4;
  byte f : 4;
  byte g : 4;
  byte h : 4;
};
// Stores a snapshot of the chess board, used for checking three-fold repetition
// Only 100 previous positions need stored, due to the fifty-move rule
// Will use ~3.4kB, a large memory spend but within budget
struct Position {
  _PackedRow rows[8];
  byte availableMoves : 8; // Instead of storing castling / enpassant rights, we store the number of available moves, as this will change depending on those rights
  byte turn : 1;
  byte : 7;
};

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

void similarPieces(Ply previousPly, Square targetSquare, Piece targetType, Piece board[8][8], Square squares[9]);
struct Position generatePosition(Ply previousPly, Piece board[8][8], bool turn);

void updateNotation();