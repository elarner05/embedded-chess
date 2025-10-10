
// File created: Sept 2025

#pragma once
#include <Arduino.h>
#include "Types.h"
#include "Draw.h"

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



// Stores all variables relating to the current game state 
struct GameState {
  bool turn; // 0 = white, 1 = black

  // 2D array that stored the postions of pieces
  Piece board[8][8];

  // Copy of board used in move validation
  Piece hypotheticalBoard[8][8];

  struct Ply selectedPly; // Coords for both squares for the current half-move
  struct Ply previousPly; // Coords for both squares for the last half-move

  //                               white          black
  // Rook movement flags         left  right   left  right
  //                               0     1       2     3
  bool rookMovementFlags[4];

  // King-movement flags, used to check castling validity
  bool whiteKingHasMoved;
  bool blackKingHasMoved;

  bool castleAlert;  // Flag for the "castle" move
  bool passantAlert; // Flag for the "en passant" move

  // Clock for use in 50 move rule; 50 moves reached when clock reaches 100
  uint8_t plyClock;

  bool selectingPiece;   // true for the first press, false for the second press
  bool promotingPiece;   // Promotion menu activity indicator
  bool cancelPromotion;  // Flag to cancel the promotion menu
};



extern struct GameState game; // Global game state



void initGameState();

bool checkAttemptedPromotion();
void handleMove();
bool validMove(int x1, int y1, int x2, int y2, Ply previousPly, Piece board[8][8]);
bool checkForCheck(Ply previousPly, Piece board[8][8]);
bool validMoveWithoutCheck(int x1, int y1, int x2, int y2, Ply previousPly, Piece board[8][8]);
void updateHypothetical(Piece from[8][8]);
void fillBoard(Piece from[8][8]);
struct Square findKing(Piece board[8][8]);
uint8_t countPossibleMoves(Ply previousPly, Ply selectedPly, Piece board[8][8]);
uint8_t countAllPossibleMoves(Ply previousPly, bool turn, Piece board[8][8]);

bool checkForCheckmate(Ply lastPly, Piece board[8][8]);
bool checkForInsufficientMaterial(Piece board[8][8]);
bool check50MoveRule(GameState gameState);                  
bool checkForDraw(Ply lastPly, Piece board[8][8]);
bool checkForGameOver(Ply lastPly, Piece board[8][8]);
void playMove(Ply ply, Piece from[8][8]);
void printBoard(Piece board[8][8]);
