
// File created: Sept 2025
// Contains shared enums

#pragma once
#include <Arduino.h>


// Assign human-readable names to some common 16-bit color values:
enum Color : uint16_t {
  BLACK = 0x0000,
  DARK_GRAY = 0x2924,
  DARK_EMERALD = 0x6c6e,
  LIGHT_EMERALD = 0xadf1,
  DARK_MARINE = 0x6b9a,
  LIGHT_MARINE = 0x749f,//0x9d5f
  BLUE = 0x001f,
  RED = 0xf800,
  GREEN = 0x07e0,
  KHAKI = 0x84ad,
  CYAN = 0x07ff,
  MAGENTA = 0xf81f,
  YELLOW = 0xffe0,
  WHITE = 0xffff,
  DARK_BROWN = 0xb44c,
  LIGHT_BROWN = 0xeed6,
  DARK_OLIVE = 0x6368,
  OLIVE = 0xad07,
  LIGHT_OLIVE = 0xce8d,
  BRICK_RED = 0xc367,
  BEIGE_GREEN = 0xad6f,

  NOTATION_BACK = DARK_EMERALD,
  NOTATION_FRONT = LIGHT_EMERALD
};

// Chess piece IDs
enum Piece : uint8_t {
  BLANK_SPACE = 0,  // Empty square
  WPAWN, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
  BPAWN, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING
};

// ASCII codes used in the chess notation system
enum class ASCII : uint8_t {
  CHAR_NULL = 0,
  CHAR_HASH = 35,
  CHAR_PLUS = 43,
  CHAR_MINUS = 45,
  CHAR_DOT = 46,
  CHAR_FORWARD_SLASH = 47,
  CHAR_0 = 48,
  CHAR_1 = 49,
  CHAR_2 = 50,
  CHAR_3 = 51,
  CHAR_4 = 52,
  CHAR_5 = 53,
  CHAR_6 = 54,
  CHAR_7 = 55,
  CHAR_8 = 56,
  CHAR_9 = 57,
  CHAR_LESS_THAN = 60,
  CHAR_EQUALS = 61,
  CHAR_GREATER_THAN = 62,
  CHAR_B = 66,
  CHAR_K = 75,
  CHAR_N = 78,
  CHAR_O = 79,
  CHAR_Q = 81,
  CHAR_R = 82,
  CHAR_CIRCUMFLEX = 94,
  CHAR_a = 97,
  CHAR_b = 98,
  CHAR_c = 99,
  CHAR_d = 100,
  CHAR_e = 101,
  CHAR_f = 102,
  CHAR_g = 103,
  CHAR_h = 104,
  CHAR_v = 118,
  CHAR_x = 120
};


// Coords of a square on a chess board, 1 byte
struct Square {
  byte x : 3;      // 3-bit X coordinate (0-7)
  byte y : 3;      // 3-bit Y coordinate (0-7)
  byte promo : 2;  // 2-bit promotion type (only relevant if y == 0 or 7) ::: 00 = Knight, 01 = Bishop, 10 = Rook, 11 = Queen
};

// Ply - A half move, stores two squares; 2 byte
struct Ply {
  struct Square from;
  struct Square to;  // 'to.promo' only matters if 'to.y' is 0 or 7 and the piece is a pawn
};

// Two ply = one move; 4 bytes
struct Move {
  struct Ply white;
  struct Ply black;
};

