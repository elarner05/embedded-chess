
// File created: Oct 2025

#include "Zobrist.h"

// reads the required bitstring from the table
uint64_t readZobristFromProgmem(const uint64_t *ptr) {
  uint64_t value = 0;

  for (int i = 0; i < 8; i++) {
    value |= ((uint64_t)pgm_read_byte_near((const uint8_t*)ptr + i)) << (8 * i);
  }

  return value;
}

// Generates a Zobrist Hash solely for use in three-fold repetition; does not require TT support
uint64_t generateZobristHash(const GameState &gameState, uint64_t availableMoves) {
  uint64_t hash = 0;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      Piece p = game.board[y][x];
      if (p > BLANK_SPACE) {
        const uint64_t *ptr = &zobristTable[p][y][x];
        hash ^= readZobristFromProgmem(ptr);
      }
    }
  }
  if (game.turn) {
    hash ^= readZobristFromProgmem(&zobristTurn);
  }


  hash ^= ((uint64_t)availableMoves * 0x9E3779B97F4A7C15ULL);

  return hash;
}