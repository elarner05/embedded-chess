
// File created: Sept 2025
// Contains shared enums

#pragma once
#include <Arduino.h>


constexpr uint8_t THREE_FOLD_REPETITION_MAX_PLY = 100;

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

// struct _PackedRow {
//   byte a : 4;
//   byte b : 4;
//   byte c : 4;
//   byte d : 4;
//   byte e : 4;
//   byte f : 4;
//   byte g : 4;
//   byte h : 4;
// };
// // Stores a snapshot of the chess board, used for checking three-fold repetition
// // Only 100 previous positions need stored, due to the fifty-move rule
// // Will use ~3.4kB, a large memory spend but within budget
// struct Position {
//   _PackedRow rows[8];
//   byte availableMoves : 8; // Instead of storing castling / enpassant rights, we store the number of available moves, as this will change depending on those rights
//   byte turn : 1;
//   byte : 7;
// };

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

  // Storage for last 100 position states, to check three-fold repetition
  uint64_t lastHashs[THREE_FOLD_REPETITION_MAX_PLY];
  uint8_t currentPosition;
};
