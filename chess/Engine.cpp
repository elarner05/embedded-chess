#include "Engine.h"

// evaluates a position heuristically and returns a score, based on piece values and positioning
int evaluatePosition(const struct GameState &game) {
  constexpr int pawnWt = 100;
  constexpr int knightWt = 320;
  constexpr int bishopWt = 330;
  constexpr int rookWt = 500;
  constexpr int queenWt = 900;

  constexpr int pawnTable[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {5, 5, 5, 5, 5, 5, 5, 5},
    {1, 1, 2, 3, 3, 2, 1, 1},
    {0, 0, 1, 2, 2, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
};
 
  int score = 0;
  for (int y = 0;y<8;y++) {
    for (int x = 0;x<8;x++) {
      switch (game.board[y][x]) {
        case WPAWN: score += pawnWt; break;
        case BPAWN: score -= pawnWt; break;
        case WKNIGHT: score += knightWt; break;
        case BKNIGHT: score -= knightWt; break;
        case WBISHOP: score += bishopWt; break;
        case BBISHOP: score -= bishopWt; break;
        case WROOK: score += rookWt; break;
        case BROOK: score -= rookWt; break;
        case WQUEEN: score += queenWt; break;
        case BQUEEN: score -= queenWt; break;
        default: break;
      }
    
    }
  }

  return score;
}

// Finds the best move to play in the given gamestate (always plays as black)
struct Ply engineMove(const struct GameState &game) {
  struct Ply move = {{0, 0, 0}, {0, 0, 0}};
  return move;
}