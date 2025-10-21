
// File created: Sept 2025
// Contains all the functionality relating to the chess notation system
// The updateNotation() function is quite large and could do with better documentation; some parts of the algorithm are difficult to understand or even conceptulize

#include <Arduino.h>
#include "Notation.h"


struct NotationState notation = { };

void initNotationState() {
  memset(notation.moves, 0, sizeof(notation.moves));
  notation.currentPlyNumber = 0;
  notation.lastDrawnPlyNumber = 0;

  notation.notationPaneNo = 0;

  struct ButtonState newButtons = { 0, 0, 0, 0 };
  notation.states = newButtons;
}


// Creates an array of squares which contains a piece of the target type that can legally move to the target square. End square has "3" in the promo position instead of "0"
void similarPieces(struct GameState &game, Square targetSquare, Piece targetType, Square dst[9]) {
  uint8_t nextSquare = 0;


  bool passantBefore = game.passantAlert;
  bool castleBefore = game.castleAlert;

  for (uint8_t y1 = 0;y1<8;y1++) {
    for (uint8_t x1 = 0;x1<8;x1++) {
      if (game.board[y1][x1] == targetType) {
        if (validMove(x1, y1, targetSquare.x, targetSquare.y, game.previousPly, game.board)) {
          struct Square square = {x1, y1, 0};
          dst[nextSquare] = square;
          nextSquare++;
        }
      }
      
    }
  }

  struct Square end = {0, 0, 3};
  dst[nextSquare] = end;

  game.passantAlert = passantBefore;
  game.castleAlert = castleBefore;
}



// // Big notation function, implementing all the complex rules of chess notation (castling, en passant, minimal move disambiguation)
// void updateNotation() {
  

//   if (notation.currentPlyNumber==0) {
//     return;
//   }

//   // Save the state of any flags, so that they can be used by this function
//   bool rookFlags[4];rookFlags[0] = game.rookMovementFlags[0];rookFlags[1] = game.rookMovementFlags[1];rookFlags[2] = game.rookMovementFlags[2];rookFlags[3] = game.rookMovementFlags[3];
//   bool passantFlag = game.passantAlert;
//   bool castleFlag = game.castleAlert;
//   bool whiteFlag = game.whiteKingHasMoved;
//   bool blackFlag = game.blackKingHasMoved;
//   bool turnFlag = game.turn;
  

//   // default all flags
//   game.rookMovementFlags[0] = false;game.rookMovementFlags[1] = false;game.rookMovementFlags[2] = false;game.rookMovementFlags[3] = false;
//   game.whiteKingHasMoved = false;game.blackKingHasMoved = false;game.turn = 0;game.castleAlert = false;game.passantAlert = false;

//   Piece notationBoard[8][8];
//   fillBoard(notationBoard);

//   uint16_t currentRow = 5;
//   uint16_t currentCol = 2;

//   if (notation.notationPaneNo > 0) {
//     for (uint16_t plyIndex = 0; plyIndex<MAX_LINES_PER_PANE * 2 * (notation.notationPaneNo); plyIndex++) {
//       struct Ply lastPly;
//       if (plyIndex > 0) {lastPly=notation.moves[plyIndex-1];}
//       validMove(notation.moves[plyIndex].from.x, notation.moves[plyIndex].from.y, notation.moves[plyIndex].to.x, notation.moves[plyIndex].to.y, lastPly, notationBoard);
//       playMove(notation.moves[plyIndex], notationBoard);
//       game.turn = (game.turn == 0) ? 1 : 0;
//     }
//   }

//   for (uint16_t plyIndex = MAX_LINES_PER_PANE * 2 * notation.notationPaneNo; plyIndex<MAX_LINES_PER_PANE * 2 * (notation.notationPaneNo+1); plyIndex+=2) {
//     uint16_t moveNumber = plyIndex/2 + 1;
//     const uint16_t moveIndex = plyIndex/2;
//     struct Ply lastPly;if (plyIndex > 0) {lastPly=notation.moves[plyIndex-1];}
    
//     // Skip drawing if already drawn
//     if (notation.lastDrawnPlyNumber>=plyIndex+1) {
//       struct Ply currentPly = notation.moves[plyIndex];
//       validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
//       playMove(currentPly, notationBoard);
//       game.turn = (game.turn == 0) ? 1 : 0;

//       lastPly = notation.moves[plyIndex];
//       currentPly = notation.moves[plyIndex+1];

//       validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
//       playMove(currentPly, notationBoard);
//       game.turn = (game.turn == 0) ? 1 : 0;

//       currentRow+=8;
//       currentCol =2;

//       continue;
//     }

//     if (plyIndex > notation.currentPlyNumber-1 && !checkForDraw(lastPly, game.turn, notationBoard)) { // Break if reached last move and game has not ended
//       break;
//     }

//     //hundreds
//     if (moveNumber>=900) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_9) + 1;
//       moveNumber-=900;
//     } else if (moveNumber>=800) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_8) + 1;
//       moveNumber-=800;
//     } else if (moveNumber>=700) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_7) + 1;
//       moveNumber-=700;
//     } else if (moveNumber>=600) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_6) + 1;
//       moveNumber-=600;
//     } else if (moveNumber>=500) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_5) + 1;
//       moveNumber-=500;
//     } else if (moveNumber>=400) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_4) + 1;
//       moveNumber-=400;
//     } else if (moveNumber>=300) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_3) + 1;
//       moveNumber-=300;
//     } else if (moveNumber>=200) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_2) + 1;
//       moveNumber-=200;
//     } else if (moveNumber>=100) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_1) + 1;
//       moveNumber-=100;
//     } else if (moveIndex+1>999) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_0) + 1;
//     } else {currentCol+=4;}

//     //tens
//     if (moveNumber>=90) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_9) + 1;
//       moveNumber-=90;
//     } else if (moveNumber>=80) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_8) + 1;
//       moveNumber-=80;
//     } else if (moveNumber>=70) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_7) + 1;
//       moveNumber-=70;
//     } else if (moveNumber>=60) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_6) + 1;
//       moveNumber-=60;
//     } else if (moveNumber>=50) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_5) + 1;
//       moveNumber-=50;
//     } else if (moveNumber>=40) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_4) + 1;
//       moveNumber-=40;
//     } else if (moveNumber>=30) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_3) + 1;
//       moveNumber-=30;
//     } else if (moveNumber>=20) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_2) + 1;
//       moveNumber-=20;
//     } else if (moveNumber>=10) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_1) + 1;
//       moveNumber-=10;
//     } else if (moveIndex+1>99) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_0) + 1;
//     } else {currentCol+=4;}

//     //ones
//     if (moveNumber>=9) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_9) + 1;
//       moveNumber-=9;
//     } else if (moveNumber>=8) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_8) + 1;
//       moveNumber-=8;
//     } else if (moveNumber>=7) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_7) + 1;
//       moveNumber-=7;
//     } else if (moveNumber>=6) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_6) + 1;
//       moveNumber-=6;
//     } else if (moveNumber>=5) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_5) + 1;
//       moveNumber-=5;
//     } else if (moveNumber>=4) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_4) + 1;
//       moveNumber-=4;
//     } else if (moveNumber>=3) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_3) + 1;
//       moveNumber-=3;
//     } else if (moveNumber>=2) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_2) + 1;
//       moveNumber-=2;
//     } else if (moveNumber>=1) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_1) + 1;
//       moveNumber-=1;
//     } else if (moveIndex+1>9) {
//       currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_0) + 1;
//     } else {currentCol+=4;}

//     currentCol=14;
//     currentCol+=drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_DOT) + 1;

//     if (plyIndex > notation.currentPlyNumber-1) {// Add draw symbol if state is draw
//       if (checkForCheckmate(lastPly, game.turn, notationBoard)) {
//         ASCII plyChars[9] = {ASCII::CHAR_0, ASCII::CHAR_MINUS, ASCII::CHAR_1, ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL}; // 0-1
//         for (uint8_t i = 0;i<3;i++) {
//           currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
//         }
//       } else if (checkForDraw(lastPly, game.turn, notationBoard)) {
//         ASCII plyChars[9] = {ASCII::CHAR_1, ASCII::CHAR_FORWARD_SLASH, ASCII::CHAR_2, ASCII::CHAR_MINUS, ASCII::CHAR_1, ASCII::CHAR_FORWARD_SLASH, ASCII::CHAR_2, ASCII::CHAR_NULL,ASCII::CHAR_NULL};
//         for (uint8_t i = 0;i<7;i++) {
//           currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
//         }
//         //notation.lastDrawnPlyNumber = plyIndex;
//       }
//       break;
//     }

//     struct Ply currentPly = notation.moves[plyIndex];
//     uint8_t plyCharsIndex = 0;
//     ASCII plyChars[9] = { ASCII::CHAR_NULL };


//     if (!(notationBoard[currentPly.from.y][currentPly.from.x] == WKING && abs(currentPly.from.x-currentPly.to.x) == 2)) { // If not castling
//       //Piece symbol
//       if (notationBoard[currentPly.from.y][currentPly.from.x] == WKING) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_K;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WQUEEN) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WBISHOP) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WKNIGHT) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WROOK) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WPAWN ) {
//         if (notationBoard[currentPly.to.y][currentPly.to.x] > WKING || currentPly.from.x!=currentPly.to.x) {// only add file if it is a capture for pawns (include en passant)
//           plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++; // 97 is ASCII::CHAR_a, then offset to get the correct file, as the currentPly.from.x for the a file = 0
//         }
//       }

//       // Move disambiguation (Add the starting rank, file, or both for disambiguation)
//       if (notationBoard[currentPly.from.y][currentPly.from.x] > WPAWN && notationBoard[currentPly.from.y][currentPly.from.x] < WKING) {
//         struct Square similarSquares[9];
//         similarPieces(lastPly, currentPly.to, notationBoard[currentPly.from.y][currentPly.from.x], notationBoard, similarSquares); // fills the 'similarSquares' with the squares of pieces that can move to the target
//         bool diffFile = false;
//         bool diffRank = false;
//         bool either = false;
//         for (uint8_t squareIndex=0;squareIndex<9;squareIndex++) {
//           if (similarSquares[squareIndex].promo == 3) {
//             break;
//           }
//           if (similarSquares[squareIndex].x == currentPly.from.x && similarSquares[squareIndex].y == currentPly.from.y) {
//             continue;
//           }
//           if (similarSquares[squareIndex].x != currentPly.from.x && similarSquares[squareIndex].y != currentPly.from.y) {
//             if (!diffRank && !diffFile) {
//               either = true;
//             }
//             continue;
//           } else if (similarSquares[squareIndex].x != currentPly.from.x) {
//             diffFile = true;
//             either = false;
//           } else if (similarSquares[squareIndex].y != currentPly.from.y) {
//             diffRank = true;
//             either = false;
//           }
//         }
//         if (diffFile || either) {
//           plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++;
//         }
//         if (diffRank) {
//         plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.from.y);plyCharsIndex++;
//         }
//       }

//       // Add the 'x' for captures
//       if (notationBoard[currentPly.to.y][currentPly.to.x] > WKING || (notationBoard[currentPly.from.y][currentPly.from.x] == WPAWN && currentPly.from.x!=currentPly.to.x)) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_x;plyCharsIndex++;
//       }

//       // Add destination square
//       plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.to.x);plyCharsIndex++;
//       plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.to.y);plyCharsIndex++;

//       // Promotion
//       if (notationBoard[currentPly.from.y][currentPly.from.x] == WPAWN && currentPly.to.y == 0) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_EQUALS;plyCharsIndex++; // =
//         if (currentPly.to.promo == 0) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++; // Knight
//         } else if (currentPly.to.promo == 1) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++; // Bishop
//         } else if (currentPly.to.promo == 2) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++; // Rook
//         } else if (currentPly.to.promo == 3) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++; // Queen
//         } 
//       }
//     } else { // castling
//       plyChars[plyCharsIndex] =  ASCII::CHAR_O;plyCharsIndex++;
//       plyChars[plyCharsIndex] =  ASCII::CHAR_MINUS;plyCharsIndex++;
//       plyChars[plyCharsIndex] =  ASCII::CHAR_O;plyCharsIndex++;
//       if (currentPly.to.x == 2) { // Queen-side castling
//         plyChars[plyCharsIndex] =  ASCII::CHAR_MINUS;plyCharsIndex++;
//         plyChars[plyCharsIndex] =  ASCII::CHAR_O;plyCharsIndex++;
//       }
//     }
//     validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
//     playMove(currentPly, notationBoard);
//     game.turn = (game.turn == 0) ? 1 : 0;

//     // Add check as required
//     if (checkForCheckmate(lastPly, game.turn, notationBoard)) {
//       plyChars[plyCharsIndex] =  ASCII::CHAR_HASH;plyCharsIndex++; // #
//     } else if (checkForCheck(lastPly, notationBoard)) {
//       plyChars[plyCharsIndex] =  ASCII::CHAR_PLUS;plyCharsIndex++; // +
//     }

    

//     if (notation.lastDrawnPlyNumber == 0 || notation.lastDrawnPlyNumber<plyIndex) {
//       for (uint8_t i = 0;i<plyCharsIndex;i++) {
//         currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
//       }
//       notation.lastDrawnPlyNumber = plyIndex;
//     }

//     currentCol = 48;

//     if (plyIndex+1 > notation.currentPlyNumber-1) {// Add draw symbol if state is draw
//       if (checkForCheckmate(lastPly, game.turn, notationBoard)) {
//         ASCII plyChars[9] = {ASCII::CHAR_1,ASCII::CHAR_MINUS,ASCII::CHAR_0,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL};
//         for (uint8_t i = 0;i<3;i++) {
//           currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
//         }
//       } else if (checkForDraw(lastPly, game.turn, notationBoard)) {
//         ASCII plyChars[9] = {ASCII::CHAR_1,ASCII::CHAR_FORWARD_SLASH,ASCII::CHAR_2,ASCII::CHAR_MINUS,ASCII::CHAR_1,ASCII::CHAR_FORWARD_SLASH,ASCII::CHAR_2,ASCII::CHAR_NULL,ASCII::CHAR_NULL};
//         for (uint8_t i = 0;i<7;i++) {
//           currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
//         }
//         //notation.lastDrawnPlyNumber = plyIndex;
//       }
//       break;
//     }

    
//     plyCharsIndex = 0;
//     plyChars[0]=ASCII::CHAR_NULL;plyChars[1]=ASCII::CHAR_NULL;plyChars[2]=ASCII::CHAR_NULL;plyChars[3]=ASCII::CHAR_NULL;plyChars[4]=ASCII::CHAR_NULL;plyChars[5]=ASCII::CHAR_NULL;plyChars[6]=ASCII::CHAR_NULL;plyChars[7]=ASCII::CHAR_NULL;plyChars[8]=ASCII::CHAR_NULL;
//     lastPly = notation.moves[plyIndex];
//     currentPly = notation.moves[plyIndex+1];
//     if (!(notationBoard[currentPly.from.y][currentPly.from.x] == BKING && abs(currentPly.from.x-currentPly.to.x) == 2)) {
//       //Piece symbol
//       if (notationBoard[currentPly.from.y][currentPly.from.x] == BKING) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_K;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BQUEEN) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BBISHOP) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BKNIGHT) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BROOK) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++;
//       } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BPAWN) {
//         if ((notationBoard[currentPly.to.y][currentPly.to.x] < BPAWN && notationBoard[currentPly.to.y][currentPly.to.x] != BLANK_SPACE) || (currentPly.from.x!=currentPly.to.x)) {// only add file if it is a capture for pawns
//           plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++;
//         }
//       }

//       // Move disambiguation (Add the starting rank, file, or both for disambiguation)
//       if (notationBoard[currentPly.from.y][currentPly.from.x] > BPAWN && notationBoard[currentPly.from.y][currentPly.from.x] < BKING) {
//         struct Square similarSquares[9];
//         similarPieces(lastPly, currentPly.to, notationBoard[currentPly.from.y][currentPly.from.x], notationBoard, similarSquares); // fills the 'similarSquares' with the squares of pieces that can move to the target
//         bool diffFile = false;
//         bool diffRank = false;
//         bool either = false;
//         for (uint8_t squareIndex=0;squareIndex<9;squareIndex++) {
//           if (similarSquares[squareIndex].promo == 3) {
//             break;
//           }
//           if (similarSquares[squareIndex].x == currentPly.from.x && similarSquares[squareIndex].y == currentPly.from.y) {
//             continue;
//           }
//           if (similarSquares[squareIndex].x != currentPly.from.x && similarSquares[squareIndex].y != currentPly.from.y) {
//             if (!diffRank && !diffFile) {
//               either = true;
//             }
//             continue;
//           } else if (similarSquares[squareIndex].x != currentPly.from.x) {
//             diffFile = true;
//             either = false;
//           } else if (similarSquares[squareIndex].y != currentPly.from.y) {
//             diffRank = true;
//             either = false;
//           }
//         }
//         if (diffFile || either) {
//           plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++;
//         }
//         if (diffRank) {
//         plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.from.y);plyCharsIndex++;
//         }
//       }

//       // Add the 'x' for captures
//       if ((notationBoard[currentPly.to.y][currentPly.to.x] < BPAWN && notationBoard[currentPly.to.y][currentPly.to.x] != BLANK_SPACE) || (notationBoard[currentPly.from.y][currentPly.from.x] == BPAWN && currentPly.from.x!=currentPly.to.x)) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_x;plyCharsIndex++;
//       }

//       // Add destination square
//       plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.to.x);plyCharsIndex++;
//       plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.to.y);plyCharsIndex++;

//       // Promotion
//       if (notationBoard[currentPly.from.y][currentPly.from.x] == BPAWN && currentPly.to.y == 7) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_EQUALS;plyCharsIndex++; // =
//         if (currentPly.to.promo == 0) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++; // Knight
//         } else if (currentPly.to.promo == 1) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++; // Bishop
//         } else if (currentPly.to.promo == 2) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++; // Rook
//         } else if (currentPly.to.promo == 3) {
//           plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++; // Queen
//         } 
//       }
//     } else { // castling
//       plyChars[plyCharsIndex] = ASCII::CHAR_O;plyCharsIndex++;
//       plyChars[plyCharsIndex] = ASCII::CHAR_MINUS;plyCharsIndex++;
//       plyChars[plyCharsIndex] = ASCII::CHAR_O;plyCharsIndex++;
//       if (currentPly.to.x == 2) {
//         plyChars[plyCharsIndex] = ASCII::CHAR_MINUS;plyCharsIndex++;
//         plyChars[plyCharsIndex] = ASCII::CHAR_O;plyCharsIndex++;
//       }
//     }
//     validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
//     playMove(currentPly, notationBoard);
//     game.turn = (game.turn == 0) ? 1 : 0;

//     // Add check as required
//     if (checkForCheckmate(lastPly, game.turn, notationBoard)) {
//       plyChars[plyCharsIndex] = ASCII::CHAR_HASH;plyCharsIndex++; // #
//     } else if (checkForCheck(lastPly, notationBoard)) {
//       plyChars[plyCharsIndex] = ASCII::CHAR_PLUS;plyCharsIndex++; // +
//     }

//     if (notation.lastDrawnPlyNumber<plyIndex+1) {
//       for (uint8_t i = 0;i<plyCharsIndex;i++) {
//         currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, static_cast<ASCII>(plyChars[i])) + 1;
//       }
//       notation.lastDrawnPlyNumber = plyIndex;
//     }

//     currentRow+=8;
//     currentCol =2;
//   }

//   // reset flags
//   game.rookMovementFlags[0] = rookFlags[0]; game.rookMovementFlags[1] = rookFlags[1];game.rookMovementFlags[2] = rookFlags[2];game.rookMovementFlags[3] = rookFlags[3];
//   game.passantAlert = passantFlag;
//   game.castleAlert = castleFlag;
//   game.whiteKingHasMoved = whiteFlag;
//   game.blackKingHasMoved = blackFlag;
//   game.turn = turnFlag;
// }


// Big notation function, implementing all the complex rules of chess notation (castling, en passant, minimal move disambiguation)
void updateNotation() {
  

  if (notation.currentPlyNumber==0) {
    return;
  }
  GameState save = game;
  initGameState();

  uint16_t currentRow = 5;
  uint16_t currentCol = 2;

  if (notation.notationPaneNo > 0) {
    for (uint16_t plyIndex = 0; plyIndex<MAX_LINES_PER_PANE * 2 * (notation.notationPaneNo); plyIndex++) {
      struct Ply lastPly;
      if (plyIndex > 0) {lastPly=notation.moves[plyIndex-1];}
      validMove(notation.moves[plyIndex].from.x, notation.moves[plyIndex].from.y, notation.moves[plyIndex].to.x, notation.moves[plyIndex].to.y, lastPly, game.board);
      playMove(notation.moves[plyIndex], game.board);
      game.turn = (game.turn == 0) ? 1 : 0;
    }
  }

  for (uint16_t plyIndex = MAX_LINES_PER_PANE * 2 * notation.notationPaneNo; plyIndex<MAX_LINES_PER_PANE * 2 * (notation.notationPaneNo+1); plyIndex+=2) {
    uint16_t moveNumber = plyIndex/2 + 1;
    const uint16_t moveIndex = plyIndex/2;
    struct Ply lastPly;if (plyIndex > 0) {lastPly=notation.moves[plyIndex-1];}
    
    // Skip drawing if already drawn
    if (notation.lastDrawnPlyNumber>=plyIndex+1) {
      struct Ply currentPly = notation.moves[plyIndex];
      validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, game.board);
      playMove(currentPly, game.board);
      game.turn = (game.turn == 0) ? 1 : 0;

      lastPly = notation.moves[plyIndex];
      currentPly = notation.moves[plyIndex+1];

      validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, game.board);
      playMove(currentPly, game.board);
      game.turn = (game.turn == 0) ? 1 : 0;

      currentRow+=8;
      currentCol =2;

      continue;
    }

    if (plyIndex > notation.currentPlyNumber-1 && !checkForDraw(game)) { // Break if reached last move and game has not ended
      break;
    }

    //hundreds
    if (moveNumber>=900) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_9) + 1;
      moveNumber-=900;
    } else if (moveNumber>=800) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_8) + 1;
      moveNumber-=800;
    } else if (moveNumber>=700) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_7) + 1;
      moveNumber-=700;
    } else if (moveNumber>=600) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_6) + 1;
      moveNumber-=600;
    } else if (moveNumber>=500) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_5) + 1;
      moveNumber-=500;
    } else if (moveNumber>=400) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_4) + 1;
      moveNumber-=400;
    } else if (moveNumber>=300) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_3) + 1;
      moveNumber-=300;
    } else if (moveNumber>=200) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_2) + 1;
      moveNumber-=200;
    } else if (moveNumber>=100) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_1) + 1;
      moveNumber-=100;
    } else if (moveIndex+1>999) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_0) + 1;
    } else {currentCol+=4;}

    //tens
    if (moveNumber>=90) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_9) + 1;
      moveNumber-=90;
    } else if (moveNumber>=80) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_8) + 1;
      moveNumber-=80;
    } else if (moveNumber>=70) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_7) + 1;
      moveNumber-=70;
    } else if (moveNumber>=60) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_6) + 1;
      moveNumber-=60;
    } else if (moveNumber>=50) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_5) + 1;
      moveNumber-=50;
    } else if (moveNumber>=40) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_4) + 1;
      moveNumber-=40;
    } else if (moveNumber>=30) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_3) + 1;
      moveNumber-=30;
    } else if (moveNumber>=20) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_2) + 1;
      moveNumber-=20;
    } else if (moveNumber>=10) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_1) + 1;
      moveNumber-=10;
    } else if (moveIndex+1>99) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_0) + 1;
    } else {currentCol+=4;}

    //ones
    if (moveNumber>=9) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_9) + 1;
      moveNumber-=9;
    } else if (moveNumber>=8) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_8) + 1;
      moveNumber-=8;
    } else if (moveNumber>=7) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_7) + 1;
      moveNumber-=7;
    } else if (moveNumber>=6) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_6) + 1;
      moveNumber-=6;
    } else if (moveNumber>=5) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_5) + 1;
      moveNumber-=5;
    } else if (moveNumber>=4) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_4) + 1;
      moveNumber-=4;
    } else if (moveNumber>=3) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_3) + 1;
      moveNumber-=3;
    } else if (moveNumber>=2) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_2) + 1;
      moveNumber-=2;
    } else if (moveNumber>=1) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_1) + 1;
      moveNumber-=1;
    } else if (moveIndex+1>9) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_0) + 1;
    } else {currentCol+=4;}

    currentCol=14;
    currentCol+=drawCharacter(currentCol, currentRow, NOTATION_FRONT, ASCII::CHAR_DOT) + 1;

    if (plyIndex > notation.currentPlyNumber-1) {// Add draw symbol if state is draw
      if (checkForCheckmate(game)) {
        ASCII plyChars[9] = {ASCII::CHAR_0, ASCII::CHAR_MINUS, ASCII::CHAR_1, ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL}; // 0-1
        for (uint8_t i = 0;i<3;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
      } else if (checkForDraw(game)) {
        ASCII plyChars[9] = {ASCII::CHAR_1, ASCII::CHAR_FORWARD_SLASH, ASCII::CHAR_2, ASCII::CHAR_MINUS, ASCII::CHAR_1, ASCII::CHAR_FORWARD_SLASH, ASCII::CHAR_2, ASCII::CHAR_NULL,ASCII::CHAR_NULL};
        for (uint8_t i = 0;i<7;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
        //notation.lastDrawnPlyNumber = plyIndex;
      }
      break;
    }

    struct Ply currentPly = notation.moves[plyIndex];
    uint8_t plyCharsIndex = 0;
    ASCII plyChars[9] = { ASCII::CHAR_NULL };


    if (!(game.board[currentPly.from.y][currentPly.from.x] == WKING && abs(currentPly.from.x-currentPly.to.x) == 2)) { // If not castling
      //Piece symbol
      if (game.board[currentPly.from.y][currentPly.from.x] == WKING) {
        plyChars[plyCharsIndex] = ASCII::CHAR_K;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == WQUEEN) {
        plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == WBISHOP) {
        plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == WKNIGHT) {
        plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == WROOK) {
        plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == WPAWN ) {
        if (game.board[currentPly.to.y][currentPly.to.x] > WKING || currentPly.from.x!=currentPly.to.x) {// only add file if it is a capture for pawns (include en passant)
          plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++; // 97 is ASCII::CHAR_a, then offset to get the correct file, as the currentPly.from.x for the a file = 0
        }
      }

      // Move disambiguation (Add the starting rank, file, or both for disambiguation)
      if (game.board[currentPly.from.y][currentPly.from.x] > WPAWN && game.board[currentPly.from.y][currentPly.from.x] < WKING) {
        struct Square similarSquares[9];
        similarPieces(game, currentPly.to, game.board[currentPly.from.y][currentPly.from.x], similarSquares); // fills the 'similarSquares' with the squares of pieces that can move to the target
        bool diffFile = false;
        bool diffRank = false;
        bool either = false;
        for (uint8_t squareIndex=0;squareIndex<9;squareIndex++) {
          if (similarSquares[squareIndex].promo == 3) {
            break;
          }
          if (similarSquares[squareIndex].x == currentPly.from.x && similarSquares[squareIndex].y == currentPly.from.y) {
            continue;
          }
          if (similarSquares[squareIndex].x != currentPly.from.x && similarSquares[squareIndex].y != currentPly.from.y) {
            if (!diffRank && !diffFile) {
              either = true;
            }
            continue;
          } else if (similarSquares[squareIndex].x != currentPly.from.x) {
            diffFile = true;
            either = false;
          } else if (similarSquares[squareIndex].y != currentPly.from.y) {
            diffRank = true;
            either = false;
          }
        }
        if (diffFile || either) {
          plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++;
        }
        if (diffRank) {
        plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.from.y);plyCharsIndex++;
        }
      }

      // Add the 'x' for captures
      if (game.board[currentPly.to.y][currentPly.to.x] > WKING || (game.board[currentPly.from.y][currentPly.from.x] == WPAWN && currentPly.from.x!=currentPly.to.x)) {
          plyChars[plyCharsIndex] = ASCII::CHAR_x;plyCharsIndex++;
      }

      // Add destination square
      plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.to.x);plyCharsIndex++;
      plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.to.y);plyCharsIndex++;

      // Promotion
      if (game.board[currentPly.from.y][currentPly.from.x] == WPAWN && currentPly.to.y == 0) {
        plyChars[plyCharsIndex] = ASCII::CHAR_EQUALS;plyCharsIndex++; // =
        if (currentPly.to.promo == 0) {
          plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++; // Knight
        } else if (currentPly.to.promo == 1) {
          plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++; // Bishop
        } else if (currentPly.to.promo == 2) {
          plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++; // Rook
        } else if (currentPly.to.promo == 3) {
          plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++; // Queen
        } 
      }
    } else { // castling
      plyChars[plyCharsIndex] =  ASCII::CHAR_O;plyCharsIndex++;
      plyChars[plyCharsIndex] =  ASCII::CHAR_MINUS;plyCharsIndex++;
      plyChars[plyCharsIndex] =  ASCII::CHAR_O;plyCharsIndex++;
      if (currentPly.to.x == 2) { // Queen-side castling
        plyChars[plyCharsIndex] =  ASCII::CHAR_MINUS;plyCharsIndex++;
        plyChars[plyCharsIndex] =  ASCII::CHAR_O;plyCharsIndex++;
      }
    }
    validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, game.board);
    playMove(currentPly, game.board);
    game.turn = (game.turn == 0) ? 1 : 0;

    // Add check as required
    if (checkForCheckmate(game)) {
      plyChars[plyCharsIndex] =  ASCII::CHAR_HASH;plyCharsIndex++; // #
    } else if (checkForCheck(game)) {
      plyChars[plyCharsIndex] =  ASCII::CHAR_PLUS;plyCharsIndex++; // +
    }

    

    if (notation.lastDrawnPlyNumber == 0 || notation.lastDrawnPlyNumber<plyIndex) {
      for (uint8_t i = 0;i<plyCharsIndex;i++) {
        currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
      }
      notation.lastDrawnPlyNumber = plyIndex;
    }

    currentCol = 48;

    if (plyIndex+1 > notation.currentPlyNumber-1) {// Add draw symbol if state is draw
      if (checkForCheckmate(game)) {
        ASCII plyChars[9] = {ASCII::CHAR_1,ASCII::CHAR_MINUS,ASCII::CHAR_0,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL,ASCII::CHAR_NULL};
        for (uint8_t i = 0;i<3;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
      } else if (checkForDraw(game)) {
        ASCII plyChars[9] = {ASCII::CHAR_1,ASCII::CHAR_FORWARD_SLASH,ASCII::CHAR_2,ASCII::CHAR_MINUS,ASCII::CHAR_1,ASCII::CHAR_FORWARD_SLASH,ASCII::CHAR_2,ASCII::CHAR_NULL,ASCII::CHAR_NULL};
        for (uint8_t i = 0;i<7;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
        //notation.lastDrawnPlyNumber = plyIndex;
      }
      break;
    }

    
    plyCharsIndex = 0;
    plyChars[0]=ASCII::CHAR_NULL;plyChars[1]=ASCII::CHAR_NULL;plyChars[2]=ASCII::CHAR_NULL;plyChars[3]=ASCII::CHAR_NULL;plyChars[4]=ASCII::CHAR_NULL;plyChars[5]=ASCII::CHAR_NULL;plyChars[6]=ASCII::CHAR_NULL;plyChars[7]=ASCII::CHAR_NULL;plyChars[8]=ASCII::CHAR_NULL;
    lastPly = notation.moves[plyIndex];
    currentPly = notation.moves[plyIndex+1];
    if (!(game.board[currentPly.from.y][currentPly.from.x] == BKING && abs(currentPly.from.x-currentPly.to.x) == 2)) {
      //Piece symbol
      if (game.board[currentPly.from.y][currentPly.from.x] == BKING) {
        plyChars[plyCharsIndex] = ASCII::CHAR_K;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == BQUEEN) {
        plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == BBISHOP) {
        plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == BKNIGHT) {
        plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == BROOK) {
        plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++;
      } else if (game.board[currentPly.from.y][currentPly.from.x] == BPAWN) {
        if ((game.board[currentPly.to.y][currentPly.to.x] < BPAWN && game.board[currentPly.to.y][currentPly.to.x] != BLANK_SPACE) || (currentPly.from.x!=currentPly.to.x)) {// only add file if it is a capture for pawns
          plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++;
        }
      }

      // Move disambiguation (Add the starting rank, file, or both for disambiguation)
      if (game.board[currentPly.from.y][currentPly.from.x] > BPAWN && game.board[currentPly.from.y][currentPly.from.x] < BKING) {
        struct Square similarSquares[9];
        similarPieces(game, currentPly.to, game.board[currentPly.from.y][currentPly.from.x], similarSquares); // fills the 'similarSquares' with the squares of pieces that can move to the target
        bool diffFile = false;
        bool diffRank = false;
        bool either = false;
        for (uint8_t squareIndex=0;squareIndex<9;squareIndex++) {
          if (similarSquares[squareIndex].promo == 3) {
            break;
          }
          if (similarSquares[squareIndex].x == currentPly.from.x && similarSquares[squareIndex].y == currentPly.from.y) {
            continue;
          }
          if (similarSquares[squareIndex].x != currentPly.from.x && similarSquares[squareIndex].y != currentPly.from.y) {
            if (!diffRank && !diffFile) {
              either = true;
            }
            continue;
          } else if (similarSquares[squareIndex].x != currentPly.from.x) {
            diffFile = true;
            either = false;
          } else if (similarSquares[squareIndex].y != currentPly.from.y) {
            diffRank = true;
            either = false;
          }
        }
        if (diffFile || either) {
          plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.from.x);plyCharsIndex++;
        }
        if (diffRank) {
        plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.from.y);plyCharsIndex++;
        }
      }

      // Add the 'x' for captures
      if ((game.board[currentPly.to.y][currentPly.to.x] < BPAWN && game.board[currentPly.to.y][currentPly.to.x] != BLANK_SPACE) || (game.board[currentPly.from.y][currentPly.from.x] == BPAWN && currentPly.from.x!=currentPly.to.x)) {
          plyChars[plyCharsIndex] = ASCII::CHAR_x;plyCharsIndex++;
      }

      // Add destination square
      plyChars[plyCharsIndex] = static_cast<ASCII>(97+currentPly.to.x);plyCharsIndex++;
      plyChars[plyCharsIndex] = static_cast<ASCII>(56-currentPly.to.y);plyCharsIndex++;

      // Promotion
      if (game.board[currentPly.from.y][currentPly.from.x] == BPAWN && currentPly.to.y == 7) {
        plyChars[plyCharsIndex] = ASCII::CHAR_EQUALS;plyCharsIndex++; // =
        if (currentPly.to.promo == 0) {
          plyChars[plyCharsIndex] = ASCII::CHAR_N;plyCharsIndex++; // Knight
        } else if (currentPly.to.promo == 1) {
          plyChars[plyCharsIndex] = ASCII::CHAR_B;plyCharsIndex++; // Bishop
        } else if (currentPly.to.promo == 2) {
          plyChars[plyCharsIndex] = ASCII::CHAR_R;plyCharsIndex++; // Rook
        } else if (currentPly.to.promo == 3) {
          plyChars[plyCharsIndex] = ASCII::CHAR_Q;plyCharsIndex++; // Queen
        } 
      }
    } else { // castling
      plyChars[plyCharsIndex] = ASCII::CHAR_O;plyCharsIndex++;
      plyChars[plyCharsIndex] = ASCII::CHAR_MINUS;plyCharsIndex++;
      plyChars[plyCharsIndex] = ASCII::CHAR_O;plyCharsIndex++;
      if (currentPly.to.x == 2) {
        plyChars[plyCharsIndex] = ASCII::CHAR_MINUS;plyCharsIndex++;
        plyChars[plyCharsIndex] = ASCII::CHAR_O;plyCharsIndex++;
      }
    }
    validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, game.board);
    playMove(currentPly, game.board);
    game.turn = (game.turn == 0) ? 1 : 0;

    // Add check as required
    if (checkForCheckmate(game)) {
      plyChars[plyCharsIndex] = ASCII::CHAR_HASH;plyCharsIndex++; // #
    } else if (checkForCheck(game)) {
      plyChars[plyCharsIndex] = ASCII::CHAR_PLUS;plyCharsIndex++; // +
    }

    if (notation.lastDrawnPlyNumber<plyIndex+1) {
      for (uint8_t i = 0;i<plyCharsIndex;i++) {
        currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, static_cast<ASCII>(plyChars[i])) + 1;
      }
      notation.lastDrawnPlyNumber = plyIndex;
    }

    currentRow+=8;
    currentCol =2;
  }

  // reset gamestate
  game = save;
  printBoard(game.board);
}


