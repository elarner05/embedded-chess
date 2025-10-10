
// File created: Sept 2025
// Contains chess game related code

#include <Arduino.h>
#include "Game.h"

struct GameState game = { };

// Fills a board array with pieces from the initialBoard, stored in PROGMEM
void fillBoard(Piece from[8][8]) {
  for (int y = 0;y<8;y++) {
    for (int x = 0;x<8;x++) {
      from[y][x] = pgm_read_byte(&initialBoard[y][x]);
    }
  }
}

void initGameState() {
  game.turn = 0;

  fillBoard(game.board);
  fillBoard(game.hypotheticalBoard);

  struct Ply selectedPly = { { 0, 0, 0 }, { 0, 0, 0 } };  // Coords for both squares for the current half-move
  struct Ply previousPly = { { 0, 0, 0 }, { 0, 0, 0 } };  // Coords for both squares for the last half-move
  game.selectedPly = selectedPly;
  game.previousPly = previousPly;

  //                                  white          black
  //   Rook movement flags         left  right   left  right
  // bool rookMovementFlags[4] = { false, false, false, false };
  for (int i=0; i<4; i++) {
    game.rookMovementFlags[i] = false;
  }

  // King-movement flags, used to check castling validity
  game.whiteKingHasMoved = false;
  game.blackKingHasMoved = false;

  game.castleAlert = false;   // Flag for the "castle" move
  game.passantAlert = false;  // Flag for the "en passant" move

  game.plyClock = 0;

  game.selectingPiece = true;     // true for the first press, false for the second press
  game.promotingPiece = false;    // Promotion menu activity indicator
  game.cancelPromotion = false;  // Flag to cancel the promotion menu

}

// Check if the move just played was a promotion
bool checkAttemptedPromotion() {
  return (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WPAWN && game.selectedPly.to.y == 0) || (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BPAWN && game.selectedPly.to.y == 7);
}



// The handle move logic, which runs after the second square is pressed
void handleMove() {
  Color color;

  if (!game.cancelPromotion) {
    //Check if previously was in check and save location if so
    struct Square prevKingLocation = {0,0,3};
    if (checkForCheck(game.previousPly, game.board)) {
      prevKingLocation = findKing(game.board);
    }

    game.turn = (game.turn == 0) ? 1 : 0;  // flip the turn value
    if (rotationOn) {
      flipRotation();
    }
    
    // rook move check  -  hard-coded values                           left white rook                                                                                                   right white rook                                                                                                                                    black left rook                                                                                                                              black right rook
    if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WROOK && game.selectedPly.from.x == 0 && game.selectedPly.from.y == 7) {
      game.rookMovementFlags[0] = true;
    } else if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WROOK && game.selectedPly.from.x == 7 && game.selectedPly.from.y == 7) {
      game.rookMovementFlags[1] = true;
    } else if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BROOK && game.selectedPly.from.x == 0 && game.selectedPly.from.y == 0) {
      game.rookMovementFlags[2] = true;
    } else if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WROOK && game.selectedPly.from.x == 7 && game.selectedPly.from.y == 0) {
      game.rookMovementFlags[3] = true;
    }

    // king move check
    if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WKING && game.selectedPly.from.x == 4 && game.selectedPly.from.y == 7) {
      game.whiteKingHasMoved = true;
    } else if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BKING && game.selectedPly.from.x == 4 && game.selectedPly.from.y == 0) {
      game.blackKingHasMoved = true;
    }

    // Update plyClock
    if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WPAWN || game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BPAWN || !(game.board[game.selectedPly.to.y][game.selectedPly.to.x] == BLANK_SPACE)) {
      game.plyClock = 0;
    } else {
      game.plyClock++;
    }

    game.board[game.selectedPly.to.y][game.selectedPly.to.x] = game.board[game.selectedPly.from.y][game.selectedPly.from.x];
    game.board[game.selectedPly.from.y][game.selectedPly.from.x] = BLANK_SPACE;



    if (game.passantAlert == true) {
      game.passantAlert = false;
      game.board[game.previousPly.to.y][game.previousPly.to.x] = BLANK_SPACE;

      // remove the en passanted piece in case the update board isn't called
      updateSquare(game.previousPly.to, game.board);
    }
    if (game.castleAlert) {
      game.castleAlert = false;
      if (game.board[game.selectedPly.to.y][game.selectedPly.to.x] == WKING) {
        if (game.selectedPly.from.x - game.selectedPly.to.x > 0) {
          game.board[7][0] = BLANK_SPACE;
          game.board[7][3] = WROOK;
          struct Square s1 = {0, 7};
          struct Square s2 = {3, 7};
          updateSquare(s1, game.board);
          updateSquare(s2, game.board);
        } else {
          game.board[7][7] = BLANK_SPACE;
          game.board[7][5] = WROOK;
          struct Square s1 = {7, 7};
          struct Square s2 = {5, 7};
          updateSquare(s1, game.board);
          updateSquare(s2, game.board);
        }
      } else if (game.selectedPly.from.x - game.selectedPly.to.x > 0) {
        game.board[0][0] = BLANK_SPACE;
        game.board[0][3] = BROOK;
        struct Square s1 = {0, 0};
        struct Square s2 = {3, 0};
        updateSquare(s1, game.board);
        updateSquare(s2, game.board);
      } else {
        game.board[0][7] = BLANK_SPACE;
        game.board[0][5] = BROOK;
        struct Square s1 = {7, 0};
        struct Square s2 = {5, 0};
        updateSquare(s1, game.board);
        updateSquare(s2, game.board);
      }
    }
    //drawBoard(board);

    // remove last move just moved colors
    if ((game.previousPly.from.x + game.previousPly.from.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.previousPly.from.x) * SQUARE_SIZE, (game.previousPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.previousPly.from.y][game.previousPly.from.x], game.previousPly.from.x, game.previousPly.from.y);
    if ((game.previousPly.to.x + game.previousPly.to.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.previousPly.to.x) * SQUARE_SIZE, (game.previousPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.previousPly.to.y][game.previousPly.to.x], game.previousPly.to.x, game.previousPly.to.y);


    // change squares to just moved colors
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.from.x) * SQUARE_SIZE, (game.selectedPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, OLIVE);
    drawPiece(game.board[game.selectedPly.from.y][game.selectedPly.from.x], game.selectedPly.from.x, game.selectedPly.from.y);
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, LIGHT_OLIVE);
    drawPiece(game.board[game.selectedPly.to.y][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y);
    
    // Remove any 'check' color from before
    if (prevKingLocation.promo!=3 && !((prevKingLocation.x == game.selectedPly.from.x && prevKingLocation.y == game.selectedPly.from.y) || (prevKingLocation.x == game.selectedPly.to.x && prevKingLocation.y == game.selectedPly.to.y))) {// If it was check and we haven't already changed the square
      updateSquare(prevKingLocation, game.board);
    }
    if (MAX_PLY > notation.currentPlyNumber) {
      notation.moves[notation.currentPlyNumber] = game.selectedPly;notation.currentPlyNumber++;
    }
    game.previousPly = game.selectedPly;
    if (rotationOn) {
      updateBoard(game.previousPly, game.selectedPly, game.board);
    }
    

    // Add red effect if in 'check'
    if (checkForCheck(game.previousPly, game.board)) {
      struct Square kingLocation = findKing(game.board);

      tft.fillRect(BOARD_BUFFER + (kingLocation.x) * SQUARE_SIZE, (kingLocation.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);
      drawPiece(game.board[kingLocation.y][kingLocation.x], kingLocation.x, kingLocation.y);
    }



  } else {
    if (game.selectedPly.from.x == game.previousPly.to.x && game.selectedPly.from.y == game.previousPly.to.y) {
      color = LIGHT_OLIVE;
    } else if ((game.selectedPly.from.x + game.selectedPly.from.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.from.x) * SQUARE_SIZE, (game.selectedPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.from.y][game.selectedPly.from.x], game.selectedPly.from.x, game.selectedPly.from.y);
    game.cancelPromotion = false;
  }

  if ((notation.notationPaneNo+1)*MAX_LINES_PER_PANE*2<notation.currentPlyNumber) {
    notation.notationPaneNo+=1;
    drawNotation();
  } else {
    updateNotation();
  }

  
}

// Big valid move function, checks all rules (castling, en passent, check, etc)
bool validMove(int x1, int y1, int x2, int y2, Ply previousPly, Piece board[8][8]) {  //                             VALIDATE                    MOVE
  //                                            ^ en passant ^
  // initial checks                   |--------------------------------(WHITE)-------------------------------------|      |---------------------------(BLACK)------------------------------|
  // | check if first space blank |   |->                                                                   <-(check if own capture)->                                                   <-|
  if (board[y1][x1] == BLANK_SPACE || (board[y1][x1] < BPAWN && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE) || (board[y1][x1] > WKING && board[y2][x2] > WKING)) { return false; }

  // check if move is valid normally
  Piece id = board[y1][x1];

  if (id == WPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE || (y1 == 6 && ((y2 + 2 == y1 && board[y1 - 1][x1] == BLANK_SPACE && board[y1 - 2][x1] == BLANK_SPACE) || y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 + 1 == y1 && board[y2][x2] > WKING)) {
    } else if (
      abs(x1 - x2) == 1 && y2 + 1 == y1 && y1 == 3 /*&& board[y2+1][x2] > WKING*/ && previousPly.from.y == 1 && previousPly.to.x == x2 && previousPly.to.y == 3 && board[previousPly.to.y][previousPly.to.x] == BPAWN) {
      game.passantAlert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && previousPly.from.y == 6 && previousPly.to.x == x2 && previousPly.to.y == 4 && board[previousPly.to.y][previousPly.to.x] == WPAWN) {
      game.passantAlert = true;
    } else {
      return false;
    }

  } else if (id == WKNIGHT || id == BKNIGHT) {

    // Knights move by changing x or y by positive or negative 1 and 2
    // 8 possibilitites: (x+1, y+2)  (x+2, y+1)  (x-1, y-2)  (x-2, y-1)  (x+1, y-2)  (x-2, y+1)  (x-1, y+2)  (x+2, y-1)
    //               1                              2                              3                              4                              5                              6                              7                              8
    if ((x1 + 1 == x2 && y1 + 2 == y2) || (x1 + 2 == x2 && y1 + 1 == y2) || (x1 - 1 == x2 && y1 - 2 == y2) || (x1 - 2 == x2 && y1 - 1 == y2) || (x1 + 1 == x2 && y1 - 2 == y2) || (x1 - 2 == x2 && y1 + 1 == y2) || (x1 - 1 == x2 && y1 + 2 == y2) || (x1 + 2 == x2 && y1 - 1 == y2)) {
    } else {
      return false;
    }

  } else if (id == WBISHOP || id == BBISHOP) {

    // Bishops move with equal changes in x and y but not passing over any other pieces
    if (abs(x1 - x2) != abs(y1 - y2)) { return false; }
    if (x1 > x2 && y1 > y2) {
      int s;
      int t;
      for (s = x1 - 1, t = y1 - 1; s > x2; s--, t--) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    } else if (x1 < x2 && y1 > y2) {
      int s;
      int t;
      for (s = x1 + 1, t = y1 - 1; s < x2; s++, t--) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    } else if (x1 < x2 && y1 < y2) {
      int s;
      int t;
      for (s = x1 + 1, t = y1 + 1; s < x2; s++, t++) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    } else if (x1 > x2 && y1 < y2) {
      int s;
      int t;
      for (s = x1 - 1, t = y1 + 1; s > x2; s--, t++) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    }

  } else if (id == WROOK || id == BROOK) {

    // Rooks move by changing x or y in any direction but not passing over any other pieces
    if (x2 != x1 && y2 == y1) {  // row
      if (x2 < x1) {
        for (int i = x1 - 1; i > x2; i--) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = x1 + 1; i < x2; i++) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      };
    } else if (x2 == x1 && y2 != y1) {  // column
      if (y2 < y1) {
        for (int i = y1 - 1; i > y2; i--) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = y1 + 1; i < y2; i++) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      };
    } else {
      return false; /* not moved in only x OR y */
    }

  } else if (id == WQUEEN || id == BQUEEN) {

    // Queens move in a combination of the rook && bishop's moves
    if (x2 != x1 && y2 == y1) {  // row
      if (x2 < x1) {
        for (int i = x1 - 1; i > x2; i--) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = x1 + 1; i < x2; i++) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      };
    } else if (x2 == x1 && y2 != y1) {  // column //                       ROOK  MOVE
      if (y2 < y1) {
        for (int i = y1 - 1; i > y2; i--) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = y1 + 1; i < y2; i++) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      };

    } else if (abs(x1 - x2) == abs(y1 - y2)) {
      if (x1 > x2 && y1 > y2) {
        int s;
        int t;
        for (s = x1 - 1, t = y1 - 1; s > x2; s--, t--) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }
      } else if (x1 < x2 && y1 > y2) {
        int s;
        int t;
        for (s = x1 + 1, t = y1 - 1; s < x2; s++, t--) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }  // BISHOP  MOVE
      } else if (x1 < x2 && y1 < y2) {
        int s;
        int t;
        for (s = x1 + 1, t = y1 + 1; s < x2; s++, t++) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }
      } else if (x1 > x2 && y1 < y2) {
        int s;
        int t;
        for (s = x1 - 1, t = y1 + 1; s > x2; s--, t++) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }
      }

    } else {
      return false; /* not moved in only x OR y */
    }
    //int game.whiteKingHasMoved = false;
    //int game.blackKingHasMoved = false;

  } else if (id == WKING) {
    // The king can move a maximum of one in any direction
    if (!(abs(x1 - x2) > 1 || abs(y1 - y2) > 1)) {
    } else if ((!game.whiteKingHasMoved) && abs(x1 - x2) == 2 && abs(y1 - y2) == 0) {
      if ((x1 - x2 > 0 && !game.rookMovementFlags[0] && board[7][3] == BLANK_SPACE && board[7][2] == BLANK_SPACE && board[7][1] == BLANK_SPACE) || (x1 - x2 < 0 && !game.rookMovementFlags[1] && board[7][5] == BLANK_SPACE)) {
        game.castleAlert = true;
      } else {
        return false;
      }
    } else {
      return false;
    }

  } else if (id == BKING) {
    // The king can move a maximum of one in any direction
    if (!(abs(x1 - x2) > 1 || abs(y1 - y2) > 1)) {
    } else if ((!game.blackKingHasMoved) && abs(x1 - x2) == 2 && abs(y1 - y2) == 0) {
      if ((x1 - x2 > 0 && !game.rookMovementFlags[2] && board[0][3] == BLANK_SPACE && board[0][2] == BLANK_SPACE && board[0][1] == BLANK_SPACE) || (x1 - x2 < 0 && !game.rookMovementFlags[3] && board[0][5] == BLANK_SPACE)) {
        game.castleAlert = true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  // check if king is capturable after move


  if (game.castleAlert) {
    if (checkForCheck(previousPly, board)) { return false; }  // starts in check
    if (x1 - x2 > 0) {
      updateHypothetical(board);
      game.hypotheticalBoard[y2][x2 + 1] = game.hypotheticalBoard[y1][x1];
      game.hypotheticalBoard[y1][x1] = BLANK_SPACE;
      if (checkForCheck(previousPly, game.hypotheticalBoard)) { return false; }  /// passes through check
    } else if (x1 - x2 < 0) {
      updateHypothetical(board);
      game.hypotheticalBoard[y2][x2 - 1] = game.hypotheticalBoard[y1][x1];
      game.hypotheticalBoard[y1][x1] = BLANK_SPACE;
      if (checkForCheck(previousPly, game.hypotheticalBoard)) { return false; }
    }
  }
  updateHypothetical(board);

  //memcpy(game.hypotheticalBoard, board, 8);
  game.hypotheticalBoard[y2][x2] = game.hypotheticalBoard[y1][x1];
  game.hypotheticalBoard[y1][x1] = BLANK_SPACE;
  if (game.passantAlert == true) {
    game.hypotheticalBoard[previousPly.to.y][previousPly.to.x] = BLANK_SPACE;
  }



  int kx = 0;
  int ky = 0;

  if (game.hypotheticalBoard[y2][x2] < BPAWN) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {

        if (game.hypotheticalBoard[y][x] == WKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (game.hypotheticalBoard[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, game.hypotheticalBoard)) {
            game.passantAlert = false;
            game.castleAlert = false;
            return false;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (game.hypotheticalBoard[y][x] == BKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             BLACK
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (game.hypotheticalBoard[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, game.hypotheticalBoard)) {
            game.passantAlert = false;
            game.castleAlert = false;
            return false;
          }
        }
      }
    }
  }



  return true;  // return true if all checks say the move could be valid
}

// Checks if the king is in check and if so, updates the "game.castleAlert" flag as relevant
bool checkForCheck(Ply previousPly, Piece board[8][8]) {
  updateHypothetical(board);
  int kx = 0;
  int ky = 0;

  if (game.turn == 0) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {

        if (game.hypotheticalBoard[y][x] == WKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (game.hypotheticalBoard[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, game.hypotheticalBoard)) {
            game.castleAlert = false;
            return true;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (game.hypotheticalBoard[y][x] == BKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             BLACK
    }

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (game.hypotheticalBoard[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, game.hypotheticalBoard)) {
            game.castleAlert = false;
            return true;
          }
        }
      }
    }
  }
  return false;
}

// Checks if the move is valid, without checking if the king is in "check"
bool validMoveWithoutCheck(int x1, int y1, int x2, int y2, Ply previousPly, Piece board[8][8]) {  //                             VALIDATE                    MOVE
  //                                            ^ en passant ^
  // Initial checks                   |--------------------------------(WHITE)-------------------------------------|      |---------------------------(BLACK)------------------------------|
  // |-check if first space blank-|   |->                                                                   <-(check if own capture)->                                                   <-|
  //   check if first space blank                                                                      check if own capture
  if (board[y1][x1] == BLANK_SPACE || (board[y1][x1] < BPAWN && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE) || (board[y1][x1] > WKING && board[y2][x2] > WKING)) { return false; }

  // check if move is valid normally
  Piece id = board[y1][x1];

  if (id == WPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE || (y1 == 6 && ((y2 + 2 == y1 && board[y1 - 1][x1] == BLANK_SPACE && board[y1 - 2][x1] == BLANK_SPACE) || y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 + 1 == y1 && board[y2][x2] > WKING)) {
    } else if (
      abs(x1 - x2) == 1 && y2 + 1 == y1 && y1 == 3 /*&& board[y2+1][x2] > WKING*/ && previousPly.from.y == 1 && previousPly.to.x == x2 && previousPly.to.y == 3 && board[previousPly.to.y][previousPly.to.x] == BPAWN) {
      game.passantAlert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && previousPly.from.y == 6 && previousPly.to.x == x2 && previousPly.to.y == 4 && board[previousPly.to.y][previousPly.to.x] == WPAWN) {
      game.passantAlert = true;
    } else {
      return false;
    }

  } else if (id == WKNIGHT || id == BKNIGHT) {

    // Knights move by changing x or y by positive or negative 1 and 2
    // 8 possibilitites: (x+1, y+2)  (x+2, y+1)  (x-1, y-2)  (x-2, y-1)  (x+1, y-2)  (x-2, y+1)  (x-1, y+2)  (x+2, y-1)
    //               1                              2                              3                              4                              5                              6                              7                              8
    if ((x1 + 1 == x2 && y1 + 2 == y2) || (x1 + 2 == x2 && y1 + 1 == y2) || (x1 - 1 == x2 && y1 - 2 == y2) || (x1 - 2 == x2 && y1 - 1 == y2) || (x1 + 1 == x2 && y1 - 2 == y2) || (x1 - 2 == x2 && y1 + 1 == y2) || (x1 - 1 == x2 && y1 + 2 == y2) || (x1 + 2 == x2 && y1 - 1 == y2)) {
    } else {
      return false;
    }

  } else if (id == WBISHOP || id == BBISHOP) {

    // Bishops move with equal changes in x and y but not passing over any other pieces
    if (abs(x1 - x2) != abs(y1 - y2)) { return false; }
    if (x1 > x2 && y1 > y2) {
      int s;
      int t;
      for (s = x1 - 1, t = y1 - 1; s > x2; s--, t--) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    } else if (x1 < x2 && y1 > y2) {
      int s;
      int t;
      for (s = x1 + 1, t = y1 - 1; s < x2; s++, t--) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    } else if (x1 < x2 && y1 < y2) {
      int s;
      int t;
      for (s = x1 + 1, t = y1 + 1; s < x2; s++, t++) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    } else if (x1 > x2 && y1 < y2) {
      int s;
      int t;
      for (s = x1 - 1, t = y1 + 1; s > x2; s--, t++) {
        if (board[t][s] != BLANK_SPACE) { return false; }
      }
    }

  } else if (id == WROOK || id == BROOK) {

    // Rooks move by changing x or y in any direction but not passing over any other pieces
    if (x2 != x1 && y2 == y1) {  // row
      if (x2 < x1) {
        for (int i = x1 - 1; i > x2; i--) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = x1 + 1; i < x2; i++) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      };
    } else if (x2 == x1 && y2 != y1) {  // column
      if (y2 < y1) {
        for (int i = y1 - 1; i > y2; i--) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = y1 + 1; i < y2; i++) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      };
    } else {
      return false; /* not moved in only x OR y */
    }

  } else if (id == WQUEEN || id == BQUEEN) {

    // Queens move in a combination of the rook and bishop's moves
    if (x2 != x1 && y2 == y1) {  // row
      if (x2 < x1) {
        for (int i = x1 - 1; i > x2; i--) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = x1 + 1; i < x2; i++) {
          if (board[y1][i] != BLANK_SPACE) { return false; }
        }
      };
    } else if (x2 == x1 && y2 != y1) {  // column //                       ROOK  MOVE
      if (y2 < y1) {
        for (int i = y1 - 1; i > y2; i--) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      } else {
        for (int i = y1 + 1; i < y2; i++) {
          if (board[i][x1] != BLANK_SPACE) { return false; }
        }
      };

    } else if (abs(x1 - x2) == abs(y1 - y2)) {
      if (x1 > x2 && y1 > y2) {
        int s;
        int t;
        for (s = x1 - 1, t = y1 - 1; s > x2; s--, t--) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }
      } else if (x1 < x2 && y1 > y2) {
        int s;
        int t;
        for (s = x1 + 1, t = y1 - 1; s < x2; s++, t--) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }  // BISHOP  MOVE
      } else if (x1 < x2 && y1 < y2) {
        int s;
        int t;
        for (s = x1 + 1, t = y1 + 1; s < x2; s++, t++) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }
      } else if (x1 > x2 && y1 < y2) {
        int s;
        int t;
        for (s = x1 - 1, t = y1 + 1; s > x2; s--, t++) {
          if (board[t][s] != BLANK_SPACE) { return false; }
        }
      }

    } else {
      return false; /* not moved in only x OR y */
    }


  } else if (id == WKING || id == BKING) {
    // The king can move a maximum of one in any direction
    if (abs(x1 - x2) > 1 || abs(y1 - y2) > 1) { return false; }
  }

  // check if king is capturable after move

  return true;  // return true if all checks say the move could be valid
}

// Updates a copy of the chess board
void updateHypothetical(Piece from[8][8]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      game.hypotheticalBoard[y][x] = from[y][x];
    }
  }
}

// Finds the king on the given board
struct Square findKing(Piece board[8][8]) {
  uint8_t king = (game.turn == 0) ? WKING : BKING;  // Set the king's piece based on the turn
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      if (board[y][x] == king) {
        struct Square kingLocation = {x, y, 0};
        return kingLocation;  // Return early once the king is found
      }
    }
  }
  struct Square exceptionCase = {0, 0, 3};
  return exceptionCase; // returns a default square if no king is found; IS NEVER USED
}



// Counts all the possible moves the selected piece can move
uint8_t countPossibleMoves(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  uint8_t numberOfMoves = 0;
  bool passantBefore = game.passantAlert;
  bool castleBefore = game.castleAlert;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (validMove(selectedPly.from.x, selectedPly.from.y, x, y, previousPly, board)) {
        numberOfMoves++;
        game.passantAlert = passantBefore;
        game.castleAlert = castleBefore;   // reset flags for every move, preventing untrackable errors
      }
    }
  }
  game.passantAlert = passantBefore; 
  game.castleAlert = castleBefore; // reset flags to their original state
  return numberOfMoves;
}

// Counts all the possible moves all pieces can play
uint8_t countAllPossibleMoves(Ply previousPly, bool turn, Piece board[8][8]) {
  uint8_t numberOfMoves = 0;
  bool passantBefore = game.passantAlert;
  bool castleBefore = game.castleAlert;

  for (int y1 = 0;y1<8;y1++) {
    for (int x1 = 0;x1<8;x1++) {
      if (( turn == 0 && (board[y1][x1] > WKING || board[y1][x1] == BLANK_SPACE) ) || ( turn == 1 && board[y1][x1] < BPAWN)) {
        continue;
      }

      for (int y2 = 0; y2 < 8; y2++) {
        for (int x2 = 0; x2 < 8; x2++) {
          if (validMove(x1, y1, x2, y2, previousPly, board)) {
            numberOfMoves++;
            game.passantAlert = passantBefore;
            game.castleAlert = castleBefore;   // reset flags for every move, preventing untrackable errors
          }
        }
      }
    }
  }
  game.passantAlert = passantBefore;
  game.castleAlert = castleBefore;
  return numberOfMoves;
}


// Test the board for checkmate, draw, and both
bool checkForCheckmate(Ply lastPly, Piece board[8][8]) {

  if (countAllPossibleMoves(lastPly, game.turn, board) == 0 && checkForCheck(lastPly, board)) {
    return true;
  }
  return false;
}
// Check for insufficient material by FIDE regulation
bool checkForInsufficientMaterial(Piece board[8][8]) {
  bool whiteKnight = false;
  bool blackKnight = false;
  bool whiteBishop = false;
  bool blackBishop = false;
  bool whiteSquareWhiteBishop = false;
  bool whiteSquareBlackBishop = false;

  for (int r = 0; r < 8; r++) {
    for (int c =  0; c < 8; c++) {
      Piece p = board[r][c];

      switch (p) {
        case WPAWN:
        case WROOK:
        case WQUEEN:
        case BPAWN:
        case BROOK:
        case BQUEEN:
          return false;
        case WKNIGHT:
          if (whiteKnight) {
            return false;
          }
          whiteKnight = true;
          break;
        case BKNIGHT:
          if (blackKnight) {
            return false;
          }
          blackKnight = true;
          break;
        case WBISHOP:
          if(whiteBishop) {
            return false;
          }
          whiteBishop = true;
          whiteSquareWhiteBishop = static_cast<bool>((r + c) % 2);
          break;
        case BBISHOP:
          if (blackBishop) {
            return false;
          }
          blackBishop = true;
          whiteSquareBlackBishop = static_cast<bool>((r + c) % 2);
          break;
        default: break;
      }
    }
  }

  // Case 1: Only kings
  // None of the flags are set to true

  // Case 2: King + minor piece vs King
  // Only one of the four flags are true

  // -> count the number of flags set, count=0 is case 1, count=1 is case 2
  uint8_t count = (whiteKnight ? 1 : 0) + (blackKnight ? 1 : 0) + (whiteBishop ? 1 : 0) + (blackBishop ? 1 : 0);
  if (count <= 1) {
    return true;
  }

  // Case 3: King + Bishop vs King + Bishop (same color bishops)
  if (whiteBishop && blackBishop && !whiteKnight && !blackKnight && (whiteSquareWhiteBishop == whiteSquareBlackBishop)) {
    return true;
  }
  
  return false;
}
bool check50MoveRule(GameState gameState) {
  if (gameState.plyClock >= 100) {
    return true;
  }
  return false;
}
bool checkForDraw(Ply lastPly, Piece board[8][8]) {
  if (countAllPossibleMoves(lastPly, game.turn, board) == 0) {
    return true;
  } 

  if (check50MoveRule(game)) {
    return true;
  }
  
  // three-fold repetition should be added
  return false;
}
bool checkForGameOver(Ply lastPly, Piece board[8][8]) {
  if (checkForDraw(lastPly, board) || checkForCheckmate(lastPly, board)) {
    return true;
  }
  return false;
}


// Plays a ply on the given board, updating flags but not updating the screen
void playMove(Ply ply, Piece from[8][8]) {
  if (from[ply.from.y][ply.from.x] == WROOK && ply.from.x == 0 && ply.from.y == 7) {
    game.rookMovementFlags[0] = true;
  } else if (from[ply.from.y][ply.from.x] == WROOK && ply.from.x == 7 && ply.from.y == 7) {
    game.rookMovementFlags[1] = true;
  } else if (from[ply.from.y][ply.from.x] == BROOK && ply.from.x == 0 && ply.from.y == 0) {
    game.rookMovementFlags[2] = true;
  } else if (from[ply.from.y][ply.from.x] == WROOK && ply.from.x == 7 && ply.from.y == 0) {
    game.rookMovementFlags[3] = true;
  }

  // king move check
  if (from[ply.from.y][ply.from.x] == WKING && ply.from.x == 4 && ply.from.y == 7) {
    game.whiteKingHasMoved = true;
  } else if (from[ply.from.y][ply.from.x] == BKING && ply.from.x == 4 && ply.from.y == 0) {
    game.blackKingHasMoved = true;
  }

  if (from[ply.from.y][ply.from.x] == WPAWN && ply.to.y == 0) {
    if (ply.to.promo==3) {
      from[ply.from.y][ply.from.x] = WQUEEN;  // Directly changes the pawn to the piece

    } else if (ply.to.promo==2) {
      from[ply.from.y][ply.from.x] = WROOK;

    } else if (ply.to.promo==1) {
      from[ply.from.y][ply.from.x] = WBISHOP;

    } else if (ply.to.promo==0) {
      from[ply.from.y][ply.from.x] = WKNIGHT;

    }
  } else if (from[ply.from.y][ply.from.x] == BPAWN && ply.to.y == 7) {
    if (ply.to.promo==3) {
      from[ply.from.y][ply.from.x] = BQUEEN;  // Directly changes the pawn to the piece

    } else if (ply.to.promo==2) {
      from[ply.from.y][ply.from.x] = BROOK;

    } else if (ply.to.promo==1) {
      from[ply.from.y][ply.from.x] = BBISHOP;

    } else if (ply.to.promo==0) {
      from[ply.from.y][ply.from.x] = BKNIGHT;
    }
  }

  from[ply.to.y][ply.to.x] = from[ply.from.y][ply.from.x];
  from[ply.from.y][ply.from.x] = BLANK_SPACE;



  if (game.passantAlert == true) {
    game.passantAlert = false;
    from[game.previousPly.to.y][game.previousPly.to.x] = BLANK_SPACE;
  }
  if (game.castleAlert) {
    game.castleAlert = false;
    if (from[ply.to.y][ply.to.x] == WKING) {
      if (ply.from.x - ply.to.x > 0) {
        from[7][0] = BLANK_SPACE;
        from[7][3] = WROOK;
      } else {
        from[7][7] = BLANK_SPACE;
        from[7][5] = WROOK;
      }
    } else if (ply.from.x - ply.to.x > 0) {
      from[0][0] = BLANK_SPACE;
      from[0][3] = BROOK;
    } else {
      from[0][7] = BLANK_SPACE;
      from[0][5] = BROOK;
    }
  }
}

// Displays the board on the serial monitor for easy debugging
void printBoard(Piece board[8][8]) {
  for (int y=0;y<8;y++) {
    for (int x=0;x<8;x++) {
      Serial.print(board[y][x]);
      Serial.print(" ");
    }
    Serial.println();
  }
}
