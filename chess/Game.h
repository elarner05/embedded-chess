#include <stdint.h>

// File created: Sept 2025

#pragma once
#include <Arduino.h>
#include "Types.h"
#include "Draw.h"
#include "Zobrist.h"


 
// Chess board with initial setup.   To access a square, use the notation:' board[y][x] ', where x,y are the coords of the square.  0<= x,y <=7
constexpr Piece initialBoard[8][8] PROGMEM = {
  {       BROOK,     BKNIGHT,     BBISHOP,      BQUEEN,       BKING,     BBISHOP,     BKNIGHT,       BROOK },
  {       BPAWN,       BPAWN,       BPAWN,       BPAWN,       BPAWN,       BPAWN,       BPAWN,       BPAWN },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  {       WPAWN,       WPAWN,       WPAWN,       WPAWN,       WPAWN,       WPAWN,       WPAWN,       WPAWN },
  {       WROOK,     WKNIGHT,     WBISHOP,      WQUEEN,       WKING,     WBISHOP,     WKNIGHT,       WROOK }
};

constexpr Piece testBoard[8][8] PROGMEM = {
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, WKING, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { WKING, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BBISHOP, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, WBISHOP },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BPAWN, BLANK_SPACE },
  { WKING, BLANK_SPACE, BKING, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, WKING, BLANK_SPACE }
};







extern struct GameState game; // Global game state



void initGameState();

bool checkAttemptedPromotion(struct GameState &game);
void handleMove(struct GameState &game);
bool validMove(int x1, int y1, int x2, int y2, Ply previousPly, Piece board[8][8]);
bool inBounds(int8_t x, int8_t y);
bool isEnemy(Piece p, bool turn);
bool checkForCheck(struct GameState &game);
bool checkForCheckAfterPly(const struct Ply ply, struct GameState &game);
bool validMoveWithoutCheck(int x1, int y1, int x2, int y2, Ply previousPly, Piece board[8][8]);
// void updateHypothetical(Piece from[8][8]);
void fillBoard(Piece from[8][8]);
struct Square findKing(Piece board[8][8]);
uint8_t countPossibleMoves(Ply previousPly, Ply selectedPly, Piece board[8][8]);
uint8_t countAllPossibleMoves(struct GameState &game);

void resetHashStorage(struct GameState &game);
bool checkForThreeFoldRepetition(const struct GameState &game);
bool checkForCheckmate(struct GameState &game);
bool checkForInsufficientMaterial(const struct GameState &game);
bool check50MoveRule(const struct GameState &game);                  
bool checkForDraw(struct GameState &game);
bool checkForGameOver(struct GameState &game);
void playMove(Ply ply, Piece from[8][8]);
void printBoard(Piece board[8][8]);
