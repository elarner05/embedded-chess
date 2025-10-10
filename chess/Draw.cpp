
// File created: Sept 2025

#include "Draw.h"

// board rotation variables
int8_t xRot = 1;
int8_t yRot = 1;
int8_t lineWidthMulitipiler = 0;
int8_t lineHeightMulitipiler = 0;

// Flips the rotation variables to rotate the pieces on the board
void flipRotation() {
  xRot *= -1;
  yRot *= -1;
  lineWidthMulitipiler = (xRot == 1) ? 0 : 1;
  lineHeightMulitipiler = (yRot == 1) ? 0 : 1;
}

// Updates a square, without adding special colors
void updateSquare(Square square, Piece board[8][8]) {
  Color color = ((square.x + square.y) % 2 == 1) ? DARK_BROWN : LIGHT_BROWN;
  tft.fillRect(BOARD_BUFFER + (square.x) * SQUARE_SIZE, (square.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);

  if (board[square.y][square.x] != BLANK_SPACE) {
    drawPiece(board[square.y][square.x], square.x, square.y);
  }
} 

// draw / delete the indicators on the squares the selected piece can move to
void draw_possible_moves(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  Color color;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (validMove(selectedPly.from.x, selectedPly.from.y, x, y, previousPly, board)) {
        if (board[y][x] == BLANK_SPACE) {
          tft.fillCircle(BOARD_BUFFER + (x)*SQUARE_SIZE + 15, (y)*SQUARE_SIZE + 15, 4, DARK_OLIVE);

        } else {
          if (previousPly.to.x == x && previousPly.to.y == y) {
            color = LIGHT_OLIVE;
          } else if ((x + y) % 2 == 1) {
            color = DARK_BROWN;
          } else {
            color = LIGHT_BROWN;
          }
          //tft.fillRect(BOARD_BUFFER+(x)*SQUARE_SIZE, (y)*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
          int acc = 0; // Optimisation to use .drawFastHLine(); if it is only one pixel, it uses .drawPixel()
          int i;
          for (int j = 0; j < SQUARE_SIZE; j++) {
            for (i = 0; i < SQUARE_SIZE; i++) {
              if (pgm_read_byte(&captureOverlay[j][i]) == 1) {
                acc++;
              } else if (acc == 1) {
                tft.drawPixel((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, DARK_OLIVE);
                acc = 0;
              } else if (acc > 0) {
                tft.drawFastHLine((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, acc, DARK_OLIVE);
                acc = 0;
              }
            }
            if (acc == 1) {
              tft.drawPixel((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, DARK_OLIVE);
              acc = 0;
            } else if (acc > 0) {
              tft.drawFastHLine((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, acc, DARK_OLIVE);
              acc = 0;
            }
          }
          drawPiece(board[y][x], x, y);
        }
      }
    }
  }
  game.passantAlert = false;
  game.castleAlert = false;
}
void delete_possible_moves(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  Color color;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (validMove(selectedPly.from.x, selectedPly.from.y, x, y, previousPly, board)) {
        if (board[y][x] == BLANK_SPACE) {
          if (previousPly.from.x == x && previousPly.from.y == y) {
            color = OLIVE;
          } else if ((x + y) % 2 == 1) {
            color = DARK_BROWN;
          } else {
            color = LIGHT_BROWN;
          }
          tft.fillCircle(BOARD_BUFFER + (x)*SQUARE_SIZE + 15, (y)*SQUARE_SIZE + 15, 4, color);

        } else {
          if (previousPly.to.x == x && previousPly.to.y == y) {
            color = LIGHT_OLIVE;
          } else if ((x + y) % 2 == 1) {
            color = DARK_BROWN;
          } else {
            color = LIGHT_BROWN;
          }

          int acc = 0;
          int i;
          for (int j = 0; j < SQUARE_SIZE; j++) {
            for (i = 0; i < SQUARE_SIZE; i++) {
              if (pgm_read_byte(&captureOverlay[j][i]) == 1) {
                acc++;
              } else if (acc == 1) {
                tft.drawPixel((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, color);
                acc = 0;
              } else if (acc > 0) {
                tft.drawFastHLine((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, acc, color);
                acc = 0;
              }
            }
            if (acc == 1) {
              tft.drawPixel((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, color);
              acc = 0;
            } else if (acc > 0) {
              tft.drawFastHLine((BOARD_BUFFER + (x)*SQUARE_SIZE) + i - acc, (y)*SQUARE_SIZE + j, acc, color);
              acc = 0;
            }
          }
          drawPiece(board[y][x], x, y);
        }
      }
    }
  }
  game.passantAlert = false;
  game.castleAlert = false;
}


// Displays a promotion menu on screen. DOES NOT CHECK IF PROMOTION IS VALID
void displayPromotionMenu() {
  if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WPAWN) {
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WQUEEN, game.selectedPly.to.x, game.selectedPly.to.y);

    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WROOK, game.selectedPly.to.x, game.selectedPly.to.y + 1);

    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WBISHOP, game.selectedPly.to.x, game.selectedPly.to.y + 2);

    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WKNIGHT, game.selectedPly.to.x, game.selectedPly.to.y + 3);
  } else {
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BQUEEN, game.selectedPly.to.x, game.selectedPly.to.y);

    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BROOK, game.selectedPly.to.x, game.selectedPly.to.y - 1);

    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BBISHOP, game.selectedPly.to.x, game.selectedPly.to.y - 2);

    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BKNIGHT, game.selectedPly.to.x, game.selectedPly.to.y - 3);
  }
}

// Function to display all the characters on the screen, currently supported by the drawCharacter() function, for easy testing
void _testCharacters() {
  uint16_t lineCount = 0;
  uint16_t colCount = 0;
  for (int i = 0;i<200;i++) {
    colCount += drawCharacter(2+colCount, 5 + 6*lineCount, NOTATION_FRONT, static_cast<ASCII>(35+i));
    colCount++;
    if (colCount+5>75) {
      colCount = 0;
      lineCount++;
    }
  }
}


// Takes in the (x,y) location of the upper left corner of the character, the color and the ASCII code of the character to draw on the screen
// returns the width of the character in pixels
int8_t drawCharacter(uint16_t x, uint16_t y, Color color, ASCII code) {
  switch (code) {
    case ASCII::CHAR_HASH:  // # (in ASCII)
      tft.drawFastVLine(x+1, y, 5, color);
      tft.drawFastVLine(x+3, y, 5, color);
      tft.drawFastHLine(x, y+1, 5, color);
      tft.drawFastHLine(x, y+3, 5, color);
      return 5;
    case ASCII::CHAR_PLUS:  // +
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastVLine(x+1, y+1, 3, color);
      return 3;
    case ASCII::CHAR_MINUS:  // -
      tft.drawFastHLine(x, y+2, 2, color);
      return 2;
    case ASCII::CHAR_DOT:  // .
      tft.drawPixel(x, y+4, color);
      return 2;
    case ASCII::CHAR_FORWARD_SLASH:  // /
      tft.drawPixel(x+2, y, color);
      tft.drawFastVLine(x+1, y+1, 3, color);
      tft.drawPixel(x, y+4, color);
      return 3;
    case ASCII::CHAR_0:  // 0
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x + 2, y, 5, color);
      tft.drawPixel(x + 1, y,color);
      tft.drawPixel(x + 1, y+4, color);
      return 3;
    case ASCII::CHAR_1:  // 1
      tft.drawFastVLine(x + 1, y, 4, color);
      tft.drawPixel(x, y + 1, color);
      tft.drawFastHLine(x, y+4, 3, color);
      return 3;
    case ASCII::CHAR_2:  // 2
      tft.drawFastHLine(x, y, 3, color);
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastHLine(x, y+4, 3, color);
      tft.drawPixel(x+2, y+1, color);
      tft.drawPixel(x, y+3, color);
      return 3;
    case ASCII::CHAR_3:  // 3
      tft.drawFastHLine(x, y, 2, color);
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawFastHLine(x, y+4, 2, color);
      tft.drawPixel(x+1,y+2, color);
      return 3;
    case ASCII::CHAR_4:  // 4
      tft.drawFastVLine(x, y, 3, color);
      tft.drawFastVLine(x + 2, y, 5, color);
      tft.drawPixel(x + 1, y + 2, color);
      return 3;
    case ASCII::CHAR_5:  // 5
      tft.drawFastHLine(x, y, 3, color);
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastHLine(x, y+4, 3, color);
      tft.drawPixel(x, y+1, color);
      tft.drawPixel(x+2, y+3, color);
      return 3;
    case ASCII::CHAR_6:  // 6
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastHLine(x + 1, y, 2, color);
      tft.drawFastVLine(x + 2, y + 2, 3, color);
      tft.drawPixel(x + 1, y + 2, color);
      tft.drawPixel(x + 1, y + 4, color);
      return 3;
    case ASCII::CHAR_7:  // 7
      tft.drawFastHLine(x, y, 3, color);
      tft.drawPixel(x + 2, y + 1, color);
      tft.drawFastVLine(x + 1, y + 2, 3, color);
      return 3;
    case ASCII::CHAR_8:  // 8
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawPixel(x+1, y, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case ASCII::CHAR_9:  // 9
      tft.drawFastVLine(x, y, 3, color);
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawPixel(x+1, y, color);
      tft.drawPixel(x+1, y+2, color);
      return 3;
    case ASCII::CHAR_LESS_THAN:  // <
      tft.drawFastHLine(x+1, y+1, 2, color);
      tft.drawFastHLine(x, y+2, 2, color);
      tft.drawFastHLine(x+1, y+3, 2, color);
      return 3;
    case ASCII::CHAR_EQUALS:  // =
      tft.drawFastHLine(x, y+1, 3, color);
      tft.drawFastHLine(x, y+3, 3, color);
      return 3;
    case ASCII::CHAR_GREATER_THAN:  // >
      tft.drawFastHLine(x, y+1, 2, color);
      tft.drawFastHLine(x+1, y+2, 2, color);
      tft.drawFastHLine(x, y+3, 2, color);
      return 3;
    case ASCII::CHAR_B:  // B
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastHLine(x+1, y, 2, color);
      tft.drawFastHLine(x+1, y+2, 2, color);
      tft.drawFastHLine(x+1, y+4, 2, color);
      tft.drawPixel(x+3, y+1, color);
      tft.drawPixel(x+3, y+3, color);
      return 4;
    case ASCII::CHAR_K:  // K 
      tft.drawFastVLine(x, y, 5, color);
      tft.drawPixel(x+3, y, color);
      tft.drawPixel(x+2, y+1, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+2, y+3, color);
      tft.drawPixel(x+3, y+4, color);
      return 4;
    case ASCII::CHAR_N:  // N
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+1, y+1, 2, color);
      tft.drawFastVLine(x+2, y+2, 2, color);
      tft.drawFastVLine(x+3, y, 5, color);
      return 4;
    case ASCII::CHAR_O:  // O
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+3, y, 5, color);
      tft.drawFastHLine(x+1, y, 2, color);
      tft.drawFastHLine(x+1, y+4, 2, color);
      return 4;
    case ASCII::CHAR_Q:  // Q
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+3, y, 5, color);
      tft.drawFastHLine(x+1, y, 2, color);
      tft.drawPixel(x+1, y+4, color);
      tft.drawFastVLine(x+2, y+3, 3, color);
      return 4;
    case ASCII::CHAR_R:  // R
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastHLine(x+1, y, 3, color);
      tft.drawFastHLine(x+1, y+2, 3, color);
      tft.drawPixel(x+3, y+1, color);
      tft.drawPixel(x+2, y+3, color);
      tft.drawPixel(x+3, y+4, color);
      return 4;
    case ASCII::CHAR_CIRCUMFLEX:  // ^
      tft.drawFastVLine(x, y+2, 2, color);
      tft.drawFastVLine(x+1, y+1, 2, color);
      tft.drawFastVLine(x+2, y+2, 2, color);
      return 3;
    case ASCII::CHAR_a:  // a
      tft.drawFastVLine(x, y+2, 3, color);
      tft.drawFastVLine(x+2, y+1, 4, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case ASCII::CHAR_b:  // b
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+2, y+2, 3, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case ASCII::CHAR_c:  // c
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastHLine(x, y+4, 3, color);
      tft.drawPixel(x, y+3, color);
      return 3;
    case ASCII::CHAR_d: // d
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawFastVLine(x, y+2, 3, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case ASCII::CHAR_e: // e
      tft.drawFastVLine(x, y+1, 3, color);
      tft.drawFastHLine(x+1, y+4, 2, color);
      tft.drawFastHLine(x+1, y+2, 2, color);
      tft.drawPixel(x+1, y, color);
      tft.drawPixel(x+2, y+1, color);
      return 3;
    case ASCII::CHAR_f: // f
      tft.drawFastVLine(x+1, y, 5, color);
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawPixel(x+2, y, color);
      return 3;
    case ASCII::CHAR_g: // g
      tft.drawFastVLine(x, y+1, 3, color);
      tft.drawFastVLine(x+2, y+2, 4, color);
      tft.drawFastHLine(x, y+5, 2, color);
      tft.drawPixel(x+1, y+1, color);
      tft.drawPixel(x+1, y+3, color);
      return 3;
    case ASCII::CHAR_h: // h
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+2, y+2, 3, color);
      tft.drawPixel(x+1, y+2, color);
      return 3;
    case ASCII::CHAR_v: // v
      tft.drawFastVLine(x, y+1, 2, color);
      tft.drawFastVLine(x+1, y+2, 2, color);
      tft.drawFastVLine(x+2, y+1, 2, color);
      return 3;
    case ASCII::CHAR_x: // x
      tft.drawPixel(x, y+2, color);
      tft.drawPixel(x, y+4, color);
      tft.drawPixel(x+2, y+2, color);
      tft.drawPixel(x+1, y+3, color);
      tft.drawPixel(x+2, y+4, color);
      return 3;
    default:
      return -1;
  }
}


// Draws a chess piece at the (x,y) location
void drawPiece(Piece id, int x, int y) {
  
  const int buffer = (game.turn == 0 || !rotationOn) ? 5 : 24;  // Buffer to center the piece on its square on the board
  
  switch (id) {
    case BLANK_SPACE:
      break;
    case WPAWN:
      drawPawn(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
      break;
    case WKNIGHT:
      drawKnight(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
      break;
    case WBISHOP:
      drawBishop(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
      break;
    case WROOK:
      drawRook(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
      break;
    case WQUEEN:
      drawQueen(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
      break;
    case WKING:
      drawKing(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
      break;
    case BPAWN:
      drawPawn(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
      break;
    case BKNIGHT:
      drawKnight(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
      break;
    case BBISHOP:
      drawBishop(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
      break;
    case BROOK:
      drawRook(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
      break;
    case BQUEEN:
      drawQueen(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
      break;
    case BKING:
      drawKing(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
      break;
  }
}


// Redisplays board; only updates squares with pieces to save time
void updateBoard(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  // x, y are the coords of the square currently being updated

  int i = 1;  // Incremented each loop, even = dark square, odd = light

  for (int y = 0; y < 8; y += 1) {
    for (int x = 0; x < 8; x += 1) {
      if ((x == previousPly.from.x && y == previousPly.from.y) || (x == previousPly.to.x && y == previousPly.to.y) || (x == selectedPly.from.x && y == selectedPly.from.y) || (x == selectedPly.to.x && y == selectedPly.to.y)) {
        // The squares that are currently pressed do not need updated, update script for them is in main loop
        i += 1;
        continue;
      }

      // Draw the tile color
      if (i % 2 == 0) {
        tft.fillRect(BOARD_BUFFER + (x)*SQUARE_SIZE, (y)*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, DARK_BROWN);
      } else if (i % 2 == 1) {
        tft.fillRect(BOARD_BUFFER + (x)*SQUARE_SIZE, (y)*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, LIGHT_BROWN);
      }

      // Draw the piece on the board
      drawPiece(board[y][x], x, y);

      i += 1;
    }
    i += 1;
  }
}


// Similar to update board, but draws all the squares
void drawBoard(Piece board[8][8]) {

  int i = 1;  // Incremented each loop, even = dark square, odd = light
  for (int y = 0; y < 8; y += 1) {
    for (int x = 0; x < 8; x += 1) {

      // Draw the tile color
      if (i % 2 == 0) {
        tft.fillRect(BOARD_BUFFER + (x)*SQUARE_SIZE, (y)*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, DARK_BROWN);
      } else if (i % 2 == 1) {
        tft.fillRect(BOARD_BUFFER + (x)*SQUARE_SIZE, (y)*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, LIGHT_BROWN);
      }

      // Draw the piece on the board
      drawPiece(board[y][x], x, y);

      i += 1;
    }
    i += 1;
  }
}


// Implements the tft.drawFastHLine function, but modifies the x location based on the width and current rotation
void drawFastHLineWithRotation(uint16_t x, uint16_t changeX, uint16_t y, uint16_t changeY, uint16_t w, Color color) {
  const int buffer = (xRot == 1) ? 0 : 1; // i hate this but its the only easy way to make this work. Apparently when the board is rotated the lines get shifted one pixel or smth
  tft.drawFastHLine(x + changeX * xRot - w * lineWidthMulitipiler + buffer, y + changeY * yRot, w, color);
}

// Similar to tft.drawFastHLineWithRotation; is not currently used
void drawFastVLineWithRotation(uint16_t x, uint16_t changeX, uint16_t y, uint16_t changeY, uint16_t h, Color color) {
  tft.drawFastHLine(x + changeX * xRot, y + changeY * yRot - h * lineHeightMulitipiler, h, color);
}



void drawButtons() {
  if (notation.states.up) {
    drawUpButton(NOTATION_FRONT, NOTATION_BACK);
  } else {
    drawUpButton(NOTATION_BACK, NOTATION_FRONT);
  }
  if (notation.states.down) {
    drawDownButton(NOTATION_FRONT, NOTATION_BACK);
  } else {
    drawDownButton(NOTATION_BACK, NOTATION_FRONT);
  }
}

void drawUpButton(Color color1, Color color2) {
  uint16_t x = 22;
  uint16_t y = 208;

  tft.fillRect(x, y, 35, 14, color1);

  tft.drawFastHLine(x, y, 35, color2);
  tft.drawFastVLine(x, y+1, 12, color2);
  tft.drawFastHLine(x, y+13, 35, color2);
  tft.drawFastVLine(x+34, y+1, 12, color2);

  tft.drawFastHLine(x+13, y+8, 2, color2);
  tft.drawFastHLine(x+14, y+7, 2, color2);
  tft.drawFastHLine(x+15, y+6, 4, color2);
  tft.drawFastHLine(x+16, y+5, 2, color2);
  tft.drawFastHLine(x+18, y+7, 2, color2);
  tft.drawFastHLine(x+19, y+8, 2, color2);

}
void drawDownButton(Color color1, Color color2) {
  uint16_t x = 22;
  uint16_t y = 223;
  
  tft.fillRect(x, y, 35, 14, color1);

  tft.drawFastHLine(x, y, 35, color2);
  tft.drawFastVLine(x, y+1, 12, color2);
  tft.drawFastHLine(x, y+13, 35, color2);
  tft.drawFastVLine(x+34, y+1, 12, color2);

  tft.drawFastHLine(x+13, y+5, 2, color2);
  tft.drawFastHLine(x+14, y+6, 2, color2);
  tft.drawFastHLine(x+15, y+7, 4, color2);
  tft.drawFastHLine(x+16, y+8, 2, color2);
  tft.drawFastHLine(x+18, y+6, 2, color2);
  tft.drawFastHLine(x+19, y+5, 2, color2);

}


// Draws the clears and updates the notation
void drawNotation() {
  tft.fillRect(0, 0, 80, 206, NOTATION_BACK);
  tft.drawRect(0, 0, 80, 206, NOTATION_FRONT);
  //tft.drawFastHLine(0, 205, 80, NOTATION_FRONT);
  notation.lastDrawnPlyNumber = 0;
  updateNotation();
}


// Individual draw function for each of the pieces, partially generated through a python script but mostly edited by hand
// Accelerated using the .drawFastHLine() and .drawFastVLine(), methods, which can be up to 2 orders of magnitude faster

void drawKnight(uint16_t x, uint16_t y, Color color1, Color color2) {  // Enhanced using .drawFastHLine()
  tft.drawPixel(x + 7 * xRot, y, color2);
  tft.drawPixel(x + 9 * xRot, y, color2);

  drawFastHLineWithRotation(x, 6, y, 1, 5, color2);
  tft.drawPixel(x + 7 * xRot, y + 1 * yRot, color1);
  tft.drawPixel(x + 9 * xRot, y + 1 * yRot, color1);


  tft.drawPixel(x + 5 * xRot, y + 2 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 2, 5, color1);
  tft.drawPixel(x + 11 * xRot, y + 2 * yRot, color2);

  tft.drawPixel(x + 4 * xRot, y + 3 * yRot, color2);
  drawFastHLineWithRotation(x, 5, y, 3, 7, color1);
  tft.drawPixel(x + 12 * xRot, y + 3 * yRot, color2);

  tft.drawPixel(x + 4 * xRot, y + 4 * yRot, color2);
  drawFastHLineWithRotation(x, 5, y, 4, 8, color1);
  tft.drawPixel(x + 13 * xRot, y + 4 * yRot, color2);

  tft.drawPixel(x + 3 * xRot, y + 5 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 5, 10, color1);
  tft.drawPixel(x + 12 * xRot, y + 5 * yRot, color2);
  tft.drawPixel(x + 14 * xRot, y + 5 * yRot, color2);

  tft.drawPixel(x + 3 * xRot, y + 6 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 6, 11, color1);
  tft.drawPixel(x + 5 * xRot, y + 6 * yRot, color2);
  tft.drawPixel(x + 13 * xRot, y + 6 * yRot, color2);
  tft.drawPixel(x + 15 * xRot, y + 6 * yRot, color2);

  tft.drawPixel(x + 2 * xRot, y + 7 * yRot, color2);
  drawFastHLineWithRotation(x, 3, y, 7, 12, color1);
  tft.drawPixel(x + 13 * xRot, y + 7 * yRot, color2);
  tft.drawPixel(x + 15 * xRot, y + 7 * yRot, color2);

  drawFastHLineWithRotation(x, 1, y, 8, 2, color2);
  drawFastHLineWithRotation(x, 3, y, 8, 13, color1);
  tft.drawPixel(x + 9 * xRot, y + 8 * yRot, color2);
  tft.drawPixel(x + 14 * xRot, y + 8 * yRot, color2);
  tft.drawPixel(x + 16 * xRot, y + 8 * yRot, color2);

  drawFastHLineWithRotation(x, 0, y, 9, 2, color2);
  drawFastHLineWithRotation(x, 2, y, 9, 14, color1);
  drawFastHLineWithRotation(x, 8, y, 9, 3, color2);
  tft.drawPixel(x + 14 * xRot, y + 9 * yRot, color2);
  tft.drawPixel(x + 16 * xRot, y + 9 * yRot, color2);

  tft.drawPixel(x + 0 * xRot, y + 10 * yRot, color2);
  drawFastHLineWithRotation(x, 1, y, 10, 5, color1);
  drawFastHLineWithRotation(x, 6, y, 10, 2, color2);
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 10 * xRot, y + 10 * yRot, color2);
  drawFastHLineWithRotation(x, 11, y, 10, 6, color1);
  tft.drawPixel(x + 15 * xRot, y + 10 * yRot, color2);
  tft.drawPixel(x + 17 * xRot, y + 10 * yRot, color2);

  drawFastHLineWithRotation(x, 1, y, 11, 5, color2);
  tft.drawPixel(x + 2 * xRot, y + 11 * yRot, color1);
  tft.drawPixel(x + 4 * xRot, y + 11 * yRot, color1);
  //------------------------------------
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 9 * xRot, y + 11 * yRot, color2);
  drawFastHLineWithRotation(x, 10, y, 11, 7, color1);
  tft.drawPixel(x + 15 * xRot, y + 11 * yRot, color2);
  tft.drawPixel(x + 17 * xRot, y + 11 * yRot, color2);

  drawFastHLineWithRotation(x, 1, y, 12, 4, color2);
  //------------------------------------
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 8 * xRot, y + 12 * yRot, color2);
  drawFastHLineWithRotation(x, 9, y, 12, 8, color1);
  tft.drawPixel(x + 15 * xRot, y + 12 * yRot, color2);
  tft.drawPixel(x + 17 * xRot, y + 12 * yRot, color2);

  tft.drawPixel(x + 7 * xRot, y + 13 * yRot, color2);
  drawFastHLineWithRotation(x, 8, y, 13, 9, color1);
  tft.drawPixel(x + 15 * xRot, y + 13 * yRot, color2);
  tft.drawPixel(x + 17 * xRot, y + 13 * yRot, color2);

  tft.drawPixel(x + 6 * xRot, y + 14 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 14, 10, color1);
  tft.drawPixel(x + 17 * xRot, y + 14 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 15 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 15, 11, color1);
  tft.drawPixel(x + 18 * xRot, y + 15 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 16 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 16, 11, color1);
  tft.drawPixel(x + 18 * xRot, y + 16 * yRot, color2);

  drawFastHLineWithRotation(x, 4, y, 17, 16, color2);

  tft.drawPixel(x + 4 * xRot, y + 18 * yRot, color2);
  drawFastHLineWithRotation(x, 5, y, 18, 14, color1);
  tft.drawPixel(x + 19 * xRot, y + 18 * yRot, color2);

  drawFastHLineWithRotation(x, 4, y, 19, 16, color2);
}

void drawPawn(uint16_t x, uint16_t y, Color color1, Color color2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 8, y, 0, 4, color2);

  tft.drawPixel(x + 7 * xRot, y + 1 * yRot, color2);
  drawFastHLineWithRotation(x, 8, y, 1, 4, color1);
  tft.drawPixel(x + 12 * xRot, y + 1 * yRot, color2);

  tft.drawPixel(x + 6 * xRot, y + 2 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 2, 6, color1);
  tft.drawPixel(x + 13 * xRot, y + 2 * yRot, color2);

  tft.drawPixel(x + 6 * xRot, y + 3 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 3, 6, color1);
  tft.drawPixel(x + 13 * xRot, y + 3 * yRot, color2);

  tft.drawPixel(x + 6 * xRot, y + 4 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 4, 6, color1);
  tft.drawPixel(x + 13 * xRot, y + 4 * yRot, color2);

  tft.drawPixel(x + 6 * xRot, y + 5 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 5, 6, color1);
  tft.drawPixel(x + 13 * xRot, y + 5 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 6 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 6, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 6 * yRot, color2);

  drawFastHLineWithRotation(x, 5, y, 7, 10, color2);  //         //

  drawFastHLineWithRotation(x, 6, y, 8, 2, color2);
  drawFastHLineWithRotation(x, 8, y, 8, 4, color1);
  drawFastHLineWithRotation(x, 12, y, 8, 2, color2);

  tft.drawPixel(x + 7 * xRot, y + 9 * yRot, color2);
  drawFastHLineWithRotation(x, 8, y, 9, 4, color1);
  tft.drawPixel(x + 12 * xRot, y + 9 * yRot, color2);

  drawFastHLineWithRotation(x, 6, y, 10, 2, color2);
  drawFastHLineWithRotation(x, 8, y, 10, 4, color1);
  drawFastHLineWithRotation(x, 12, y, 10, 2, color2);

  tft.drawPixel(x + 6 * xRot, y + 11 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 11, 6, color1);
  tft.drawPixel(x + 13 * xRot, y + 11 * yRot, color2);

  tft.drawPixel(x + 6 * xRot, y + 12 * yRot, color2);
  drawFastHLineWithRotation(x, 7, y, 12, 6, color1);
  tft.drawPixel(x + 13 * xRot, y + 12 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 13 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 13, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 13 * yRot, color2);

  drawFastHLineWithRotation(x, 4, y, 14, 2, color2);
  drawFastHLineWithRotation(x, 6, y, 14, 8, color1);
  drawFastHLineWithRotation(x, 14, y, 14, 2, color2);

  tft.drawPixel(x + 4 * xRot, y + 15 * yRot, color2);
  drawFastHLineWithRotation(x, 5, y, 15, 10, color1);
  tft.drawPixel(x + 15 * xRot, y + 15 * yRot, color2);

  tft.drawPixel(x + 3 * xRot, y + 16 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 16, 12, color1);
  tft.drawPixel(x + 16 * xRot, y + 16 * yRot, color2);

  drawFastHLineWithRotation(x, 2, y, 17, 16, color2);

  tft.drawPixel(x + 1 * xRot, y + 18 * yRot, color2);
  drawFastHLineWithRotation(x, 2, y, 18, 16, color1);
  tft.drawPixel(x + 18 * xRot, y + 18 * yRot, color2);

  drawFastHLineWithRotation(x, 1, y, 19, 18, color2);
}

void drawQueen(uint16_t x, uint16_t y, Color color1, Color color2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 6, y, 0, 3, color2);
  drawFastHLineWithRotation(x, 11, y, 0, 3, color2);

  drawFastHLineWithRotation(x, 6, y, 1, 3, color2);
  tft.drawPixel(x + 7 * xRot, y + 1 * yRot, color1);
  drawFastHLineWithRotation(x, 11, y, 1, 3, color2);
  tft.drawPixel(x + 12 * xRot, y + 1 * yRot, color1);

  drawFastHLineWithRotation(x, 0, y, 2, 3, color2);
  drawFastHLineWithRotation(x, 6, y, 2, 3, color2);
  drawFastHLineWithRotation(x, 11, y, 2, 3, color2);
  drawFastHLineWithRotation(x, 17, y, 2, 3, color2);

  drawFastHLineWithRotation(x, 0, y, 3, 3, color2);
  tft.drawPixel(x + 1 * xRot, y + 3 * yRot, color1);
  tft.drawPixel(x + 7 * xRot, y + 3 * yRot, color2);
  tft.drawPixel(x + 12 * xRot, y + 3 * yRot, color2);
  drawFastHLineWithRotation(x, 17, y, 3, 3, color2);
  tft.drawPixel(x + 18 * xRot, y + 3 * yRot, color1);

  drawFastHLineWithRotation(x, 0, y, 4, 3, color2);
  drawFastHLineWithRotation(x, 6, y, 4, 3, color2);
  tft.drawPixel(x + 7 * xRot, y + 4 * yRot, color1);
  drawFastHLineWithRotation(x, 11, y, 4, 3, color2);
  tft.drawPixel(x + 12 * xRot, y + 4 * yRot, color1);
  drawFastHLineWithRotation(x, 17, y, 4, 3, color2);

  drawFastHLineWithRotation(x, 1, y, 5, 2, color2);
  drawFastHLineWithRotation(x, 6, y, 5, 3, color2);
  tft.drawPixel(x + 7 * xRot, y + 5 * yRot, color1);
  drawFastHLineWithRotation(x, 11, y, 5, 3, color2);
  tft.drawPixel(x + 12 * xRot, y + 5 * yRot, color1);
  drawFastHLineWithRotation(x, 17, y, 5, 2, color2);

  tft.drawPixel(x + 2 * xRot, y + 6 * yRot, color2);
  drawFastHLineWithRotation(x, 5, y, 6, 4, color2);
  tft.drawPixel(x + 7 * xRot, y + 6 * yRot, color1);
  drawFastHLineWithRotation(x, 11, y, 6, 4, color2);
  tft.drawPixel(x + 12 * xRot, y + 6 * yRot, color1);
  tft.drawPixel(x + 17 * xRot, y + 6 * yRot, color2);

  drawFastHLineWithRotation(x, 2, y, 7, 2, color2);
  drawFastHLineWithRotation(x, 5, y, 7, 10, color2);
  drawFastHLineWithRotation(x, 6, y, 7, 3, color1);
  drawFastHLineWithRotation(x, 11, y, 7, 3, color1);
  drawFastHLineWithRotation(x, 16, y, 7, 2, color2);

  drawFastHLineWithRotation(x, 2, y, 8, 16, color2);
  tft.drawPixel(x + 3 * xRot, y + 8 * yRot, color1);
  drawFastHLineWithRotation(x, 6, y, 8, 3, color1);
  drawFastHLineWithRotation(x, 11, y, 8, 3, color1);
  tft.drawPixel(x + 16 * xRot, y + 8 * yRot, color1);

  tft.drawPixel(x + 2 * xRot, y + 9 * yRot, color2);
  drawFastHLineWithRotation(x, 3, y, 9, 14, color1);
  tft.drawPixel(x + 17 * xRot, y + 9 * yRot, color2);

  tft.drawPixel(x + 3 * xRot, y + 10 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 10, 12, color1);
  tft.drawPixel(x + 16 * xRot, y + 10 * yRot, color2);

  tft.drawPixel(x + 3 * xRot, y + 11 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 11, 12, color1);
  tft.drawPixel(x + 16 * xRot, y + 11 * yRot, color2);

  drawFastHLineWithRotation(x, 3, y, 12, 2, color2);
  drawFastHLineWithRotation(x, 5, y, 12, 10, color1);
  drawFastHLineWithRotation(x, 15, y, 12, 2, color2);

  drawFastHLineWithRotation(x, 4, y, 13, 12, color2);

  drawFastHLineWithRotation(x, 3, y, 14, 2, color2);
  drawFastHLineWithRotation(x, 5, y, 14, 10, color1);
  drawFastHLineWithRotation(x, 15, y, 14, 2, color2);

  tft.drawPixel(x + 3 * xRot, y + 15 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 15, 12, color1);
  tft.drawPixel(x + 16 * xRot, y + 15 * yRot, color2);

  drawFastHLineWithRotation(x, 3, y, 16, 14, color2);

  tft.drawPixel(x + 2 * xRot, y + 17 * yRot, color2);
  drawFastHLineWithRotation(x, 3, y, 17, 14, color1);
  tft.drawPixel(x + 17 * xRot, y + 17 * yRot, color2);

  tft.drawPixel(x + 2 * xRot, y + 18 * yRot, color2);
  drawFastHLineWithRotation(x, 3, y, 18, 14, color1);
  tft.drawPixel(x + 17 * xRot, y + 18 * yRot, color2);

  drawFastHLineWithRotation(x, 2, y, 19, 16, color2);
}

void drawKing(uint16_t x, uint16_t y, Color color1, Color color2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 9, y, 0, 2, color2);

  drawFastHLineWithRotation(x, 8, y, 1, 4, color2);

  drawFastHLineWithRotation(x, 9, y, 2, 2, color2);

  drawFastHLineWithRotation(x, 9, y, 3, 2, color2);

  drawFastHLineWithRotation(x, 3, y, 4, 4, color2);
  drawFastHLineWithRotation(x, 8, y, 4, 4, color2);
  drawFastHLineWithRotation(x, 13, y, 4, 4, color2);

  drawFastHLineWithRotation(x, 1, y, 5, 18, color2);
  drawFastHLineWithRotation(x, 4, y, 5, 2, color1);
  drawFastHLineWithRotation(x, 9, y, 5, 2, color1);
  drawFastHLineWithRotation(x, 14, y, 5, 2, color1);

  drawFastHLineWithRotation(x, 0, y, 6, 2, color2);
  drawFastHLineWithRotation(x, 2, y, 6, 16, color1);
  tft.drawPixel(x + 7 * xRot, y + 6 * yRot, color2);
  tft.drawPixel(x + 12 * xRot, y + 6 * yRot, color2);
  drawFastHLineWithRotation(x, 18, y, 6, 2, color2);

  tft.drawPixel(x + 0 * xRot, y + 7 * yRot, color2);
  drawFastHLineWithRotation(x, 1, y, 7, 18, color1);
  drawFastHLineWithRotation(x, 7, y, 7, 2, color2);
  drawFastHLineWithRotation(x, 11, y, 7, 2, color2);
  tft.drawPixel(x + 19 * xRot, y + 7 * yRot, color2);

  tft.drawPixel(x + 0 * xRot, y + 8 * yRot, color2);
  drawFastHLineWithRotation(x, 1, y, 8, 18, color1);
  drawFastHLineWithRotation(x, 8, y, 8, 4, color2);
  tft.drawPixel(x + 19 * xRot, y + 8 * yRot, color2);

  tft.drawPixel(x + 0 * xRot, y + 9 * yRot, color2);
  drawFastHLineWithRotation(x, 1, y, 9, 18, color1);
  drawFastHLineWithRotation(x, 9, y, 9, 2, color2);
  tft.drawPixel(x + 19 * xRot, y + 9 * yRot, color2);

  drawFastHLineWithRotation(x, 0, y, 10, 2, color2);
  drawFastHLineWithRotation(x, 2, y, 10, 16, color1);
  drawFastHLineWithRotation(x, 9, y, 10, 2, color2);
  drawFastHLineWithRotation(x, 18, y, 10, 2, color2);

  tft.drawPixel(x + 1 * xRot, y + 11 * yRot, color2);
  drawFastHLineWithRotation(x, 2, y, 11, 16, color1);
  drawFastHLineWithRotation(x, 9, y, 11, 2, color2);
  tft.drawPixel(x + 18 * xRot, y + 11 * yRot, color2);

  drawFastHLineWithRotation(x, 1, y, 12, 2, color2);
  drawFastHLineWithRotation(x, 3, y, 12, 14, color1);
  drawFastHLineWithRotation(x, 9, y, 12, 2, color2);
  drawFastHLineWithRotation(x, 17, y, 12, 2, color2);

  drawFastHLineWithRotation(x, 2, y, 13, 2, color2);
  drawFastHLineWithRotation(x, 4, y, 13, 12, color1);
  drawFastHLineWithRotation(x, 9, y, 13, 2, color2);
  drawFastHLineWithRotation(x, 16, y, 13, 2, color2);

  drawFastHLineWithRotation(x, 3, y, 14, 14, color2);

  drawFastHLineWithRotation(x, 2, y, 15, 2, color2);
  drawFastHLineWithRotation(x, 4, y, 15, 12, color1);
  drawFastHLineWithRotation(x, 16, y, 15, 2, color2);

  drawFastHLineWithRotation(x, 2, y, 16, 16, color2);
  drawFastHLineWithRotation(x, 3, y, 16, 2, color1);
  drawFastHLineWithRotation(x, 15, y, 16, 2, color1);

  drawFastHLineWithRotation(x, 2, y, 17, 3, color2);
  drawFastHLineWithRotation(x, 5, y, 17, 10, color1);
  drawFastHLineWithRotation(x, 15, y, 17, 3, color2);

  tft.drawPixel(x + 2 * xRot, y + 18 * yRot, color2);
  drawFastHLineWithRotation(x, 3, y, 18, 14, color1);
  tft.drawPixel(x + 17 * xRot, y + 18 * yRot, color2);

  drawFastHLineWithRotation(x, 2, y, 19, 16, color2);
}

void drawRook(uint16_t x, uint16_t y, Color color1, Color color2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 2, y, 0, 4, color2);
  drawFastHLineWithRotation(x, 8, y, 0, 4, color2);
  drawFastHLineWithRotation(x, 14, y, 0, 4, color2);

  drawFastHLineWithRotation(x, 2, y, 1, 4, color2);
  drawFastHLineWithRotation(x, 3, y, 1, 2, color1);
  drawFastHLineWithRotation(x, 8, y, 1, 4, color2);
  drawFastHLineWithRotation(x, 9, y, 1, 2, color1);
  drawFastHLineWithRotation(x, 14, y, 1, 4, color2);
  drawFastHLineWithRotation(x, 15, y, 1, 2, color1);

  drawFastHLineWithRotation(x, 2, y, 2, 16, color2);
  drawFastHLineWithRotation(x, 3, y, 2, 2, color1);
  drawFastHLineWithRotation(x, 9, y, 2, 2, color1);
  drawFastHLineWithRotation(x, 15, y, 2, 2, color1);

  tft.drawPixel(x + 2 * xRot, y + 3 * yRot, color2);
  drawFastHLineWithRotation(x, 3, y, 3, 14, color1);
  tft.drawPixel(x + 17 * xRot, y + 3 * yRot, color2);

  drawFastHLineWithRotation(x, 2, y, 4, 2, color2);
  drawFastHLineWithRotation(x, 4, y, 4, 12, color1);
  drawFastHLineWithRotation(x, 16, y, 4, 2, color2);

  drawFastHLineWithRotation(x, 3, y, 5, 2, color2);
  drawFastHLineWithRotation(x, 5, y, 5, 10, color1);
  drawFastHLineWithRotation(x, 15, y, 5, 2, color2);

  drawFastHLineWithRotation(x, 4, y, 6, 12, color2);

  //tft.drawRect(x + 5*xRot, y + 7*yRot, 10, 7, color2);
  //tft.fillRect(x + 6*xRot, y + 7*yRot, 8, 7, color1);
  tft.drawPixel(x + 5 * xRot, y + 7 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 7, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 7 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 8 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 8, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 8 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 9 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 9, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 9 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 10 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 10, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 10 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 11 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 11, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 11 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 12 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 12, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 12 * yRot, color2);

  tft.drawPixel(x + 5 * xRot, y + 13 * yRot, color2);
  drawFastHLineWithRotation(x, 6, y, 13, 8, color1);
  tft.drawPixel(x + 14 * xRot, y + 13 * yRot, color2);

  drawFastHLineWithRotation(x, 4, y, 14, 12, color2);

  drawFastHLineWithRotation(x, 3, y, 15, 2, color2);
  drawFastHLineWithRotation(x, 5, y, 15, 10, color1);
  drawFastHLineWithRotation(x, 15, y, 15, 2, color2);

  drawFastHLineWithRotation(x, 2, y, 16, 2, color2);
  drawFastHLineWithRotation(x, 4, y, 16, 12, color1);
  drawFastHLineWithRotation(x, 16, y, 16, 2, color2);

  drawFastHLineWithRotation(x, 1, y, 17, 18, color2);

  tft.drawPixel(x + 1 * xRot, y + 18 * yRot, color2);
  drawFastHLineWithRotation(x, 2, y, 18, 16, color1);
  tft.drawPixel(x + 18 * xRot, y + 18 * yRot, color2);

  drawFastHLineWithRotation(x, 1, y, 19, 18, color2);
}

void drawBishop(uint16_t x, uint16_t y, Color color1, Color color2) {// Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 9, y, 0, 2, color2);

  tft.drawPixel(x + 8 * xRot, y + 1 * yRot, color2);
  drawFastHLineWithRotation(x, 9, y, 1, 2, color1);
  tft.drawPixel(x + 11 * xRot, y + 1 * yRot, color2);

  drawFastHLineWithRotation(x, 9, y, 2, 2, color2);

  drawFastHLineWithRotation(x, 7, y, 3, 6, color2);
  drawFastHLineWithRotation(x, 9, y, 3, 2, color1);

  drawFastHLineWithRotation(x, 7, y, 4, 6, color2);
  drawFastHLineWithRotation(x, 8, y, 4, 2, color1);

  drawFastHLineWithRotation(x, 6, y, 5, 8, color2);
  drawFastHLineWithRotation(x, 8, y, 5, 2, color1);

  drawFastHLineWithRotation(x, 6, y, 6, 8, color2);
  drawFastHLineWithRotation(x, 7, y, 6, 3, color1);
  tft.drawPixel(x + 12 * xRot, y + 6 * yRot, color1);

  drawFastHLineWithRotation(x, 6, y, 7, 8, color2);
  drawFastHLineWithRotation(x, 7, y, 7, 2, color1);
  drawFastHLineWithRotation(x, 11, y, 7, 2, color1);

  drawFastHLineWithRotation(x, 6, y, 8, 8, color2);
  drawFastHLineWithRotation(x, 7, y, 8, 2, color1);
  drawFastHLineWithRotation(x, 11, y, 8, 2, color1);

  drawFastHLineWithRotation(x, 6, y, 9, 8, color2);
  drawFastHLineWithRotation(x, 7, y, 9, 6, color1);

  drawFastHLineWithRotation(x, 6, y, 10, 8, color2);
  drawFastHLineWithRotation(x, 7, y, 10, 6, color1);

  drawFastHLineWithRotation(x, 6, y, 11, 8, color2);
  drawFastHLineWithRotation(x, 7, y, 11, 6, color1);

  drawFastHLineWithRotation(x, 6, y, 12, 8, color2);
  drawFastHLineWithRotation(x, 8, y, 12, 4, color1);

  drawFastHLineWithRotation(x, 7, y, 13, 6, color2);
  drawFastHLineWithRotation(x, 9, y, 13, 2, color1);

  drawFastHLineWithRotation(x, 8, y, 14, 4, color2);
  drawFastHLineWithRotation(x, 9, y, 14, 2, color1);

  drawFastHLineWithRotation(x, 7, y, 15, 6, color2);
  drawFastHLineWithRotation(x, 9, y, 15, 2, color1);

  drawFastHLineWithRotation(x, 6, y, 16, 8, color2);
  drawFastHLineWithRotation(x, 8, y, 16, 4, color1);

  drawFastHLineWithRotation(x, 3, y, 17, 14, color2);

  tft.drawPixel(x + 3 * xRot, y + 18 * yRot, color2);
  drawFastHLineWithRotation(x, 4, y, 18, 12, color1);
  tft.drawPixel(x + 16 * xRot, y + 18 * yRot, color2);

  drawFastHLineWithRotation(x, 3, y, 19, 14, color2);
}