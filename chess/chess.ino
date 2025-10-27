
// Author: Ethan Larner. Project started: Sept 2022

/*

    TODO:
  Testing and optimizations
  Split the main functions into sub-divided sections (remove that hellish looking nested mess)

  Version 3
  Chess AI
  
    IDEAS:
  Go back moves and then jump to where it is being played
  Play from here button

*/

// TFT screen used in project   : https://www.amazon.co.uk/Elegoo-EL-SM-004-Inches-Technical-Arduino/dp/B01EUVJYME
// Libraries are obtainable from: https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-2-8-inch-touch-screen-for-raspberry-pi-manual?srsltid=AfmBOooPWvJi5iR3kjFEz30FoBpyVXjYLvpluqeob0fl5NJ7rkZVzApn


#include "Setup.h"
#include "Types.h"
#include "Draw.h"
#include "Game.h"
#include "Notation.h"


TSPoint p;

// Setup of the main data singletons and the touchscreen
void setup() {

  // Set up the global game struct
  initGameState();
  initNotationState();
  
  Serial.begin(9600);
  

  tft.reset();

  constexpr uint16_t identifier = 0x9341;  // 0x9341 LCD driver
  tft.begin(identifier);

  pinMode(SD_SCK_PIN, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  tft.setRotation(1);
  tft.fillScreen(BLACK);

  tft.fillRect(0, 0, 80, 240, NOTATION_BACK);
  tft.drawRect(0, 0, 80, 240, NOTATION_FRONT);
  drawNotation();
  drawButtons();
  drawBoard(game.board);
}

// Main loop function
void loop() {

  digitalWrite(SD_SCK_PIN, HIGH);
  p = ts.getPoint();             // Poll the latest TSPoint instance
  digitalWrite(SD_SCK_PIN, LOW);

  pinMode(XM, OUTPUT); // ensure the pin directions are correct after polling
  pinMode(YP, OUTPUT);
  int p_x = 330 - map(p.y, TS_MINY, TS_MAXY, 0, 320);
  int p_y = map(p.x, TS_MINX, TS_MAXX, 240, 0);      // map the coords to better values

  if (pressedButton(p_x, p_y) > 0) {
    runButtons(p_x, p_y, notation);
  }  

  if (!validDelay()) { // if it has been long enough to consider it a press
    return;
  }

  updateNotationButtons();
  
  if (!validPress(p.z)) { // Check that the pressure is enough to be considered a press
    return;
  }
  

  lastPress = millis(); // (the screen has been pressed, update this to prevent accidental multiple presses)


  if (!pressedBoard(p_x, p_y)) { // if the board has not been pressed, leave
    return;
  }

  struct Square square = findSquareFromScreenPos(p_x, p_y);// get the square for the last press

  // Serial.println("X: " + static_cast<String>(square.x) + " | Y: " + static_cast<String>(square.y));

  if (game.promotingPiece) { // if the promotion menu is being displayed
    game.promotingPiece = false;
    runPromotingPiece(square, game);
    

  } else if (game.selectingPiece) {  // first press for the current ply
    
    runFirstPress(square, game);

  } else {  // second press for the current ply
    runSecondPress(square, game);
    game.selectingPiece = true;
  }
}


// ====== Encapsulation functions ======

// runs when the screen has been pressed if and the promotion menu is displaying 
void runPromotingPiece(struct Square &square, struct GameState &game) {
  // nx, ny are the promotion square coords being looked at promotion 
  uint8_t nx = square.x;
  uint8_t ny = square.y;
  Color color;

  if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WPAWN) { // if its white's turn
  // individually update and every square in the promotion menu return to the proper piece
    if ((game.selectedPly.to.x + game.selectedPly.to.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y);

    if ((game.selectedPly.to.x + game.selectedPly.to.y + 1) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y + 1][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y + 1);

    if ((game.selectedPly.to.x + game.selectedPly.to.y + 2) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y + 2][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y + 2);

    if ((game.selectedPly.to.x + game.selectedPly.to.y + 3) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y + 3][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y + 3);

  } else { // if it's blacks turn
    if ((game.selectedPly.to.x + game.selectedPly.to.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y);

    if ((game.selectedPly.to.x + game.selectedPly.to.y - 1) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y - 1][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y - 1);

    if ((game.selectedPly.to.x + game.selectedPly.to.y - 2) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y - 2][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y - 2);

    if ((game.selectedPly.to.x + game.selectedPly.to.y - 3) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.to.x) * SQUARE_SIZE, (game.selectedPly.to.y - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.to.y - 3][game.selectedPly.to.x], game.selectedPly.to.x, game.selectedPly.to.y - 3);
  }


  // Interpretation of the promotion
  if (nx != game.selectedPly.to.x || (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WPAWN && ny >= 4) || (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BPAWN && ny <= 3)) {
    game.cancelPromotion = true; /* user did not press a square or pressed a square on the wrong column or wrong row */

  } else if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WPAWN) {
    if (ny == 0) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = WQUEEN;  // Directly changes the pawn to the piece ; see first line after promote menu
      game.selectedPly.to.promo = 3;
    } else if (ny == game.selectedPly.to.y + 1) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = WROOK;
      game.selectedPly.to.promo = 2;
    } else if (ny == game.selectedPly.to.y + 2) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = WBISHOP;
      game.selectedPly.to.promo = 1;
    } else if (ny == game.selectedPly.to.y + 3) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = WKNIGHT;
      game.selectedPly.to.promo = 0;
    }
  } else if (game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BPAWN) {
    if (ny == game.selectedPly.to.y) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = BQUEEN;  // Directly changes the pawn to the piece ; see first line after promote menu
      game.selectedPly.to.promo = 3;
    } else if (ny == game.selectedPly.to.y - 1) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = BROOK;
      game.selectedPly.to.promo = 2;
    } else if (ny == game.selectedPly.to.y - 2) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = BBISHOP;
      game.selectedPly.to.promo = 1;
    } else if (ny == game.selectedPly.to.y - 3) {
      game.board[game.selectedPly.from.y][game.selectedPly.from.x] = BKNIGHT;
      game.selectedPly.to.promo = 0;
    }
  }

  handleMove(game);
}

// run first press on the screen
void runFirstPress(struct Square &square, struct GameState &game) {
  Serial.println();
  printBoard(game.board);
  Serial.println(static_cast<String>(game.kingSquares[0].x) + " " + static_cast<String>(game.kingSquares[0].y) + " " + static_cast<String>(game.kingSquares[1].x) + " " + static_cast<String>(game.kingSquares[1].y));
  Serial.println();
  if (checkForGameOver(game)) {
    // Game over?
  } else if (((game.board[square.y][square.x] < BPAWN && game.turn == 0) || (game.board[square.y][square.x] > WKING && game.turn == 1))) {
    game.selectedPly.from.x = square.x;
    game.selectedPly.from.y = square.y;
    if (!(game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BLANK_SPACE)) {

      draw_possible_moves(game.previousPly, game.selectedPly, game.board);

      tft.fillRect(BOARD_BUFFER + square.x * SQUARE_SIZE, square.y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, KHAKI);
      drawPiece(game.board[square.y][square.x], square.x, square.y);
      game.selectingPiece = false;
    }
  }
}

void runSecondPress(struct Square &square, struct GameState &game) {
  delete_possible_moves(game.previousPly, game.selectedPly, game.board);
  game.selectedPly.to.x = square.x;
  game.selectedPly.to.y = square.y;

  if (validMove(game.selectedPly.from.x, game.selectedPly.from.y, game.selectedPly.to.x, game.selectedPly.to.y, game.previousPly, game.board)) {
    if (checkAttemptedPromotion(game)) {
      displayPromotionMenu();
      game.promotingPiece = true;

    } else {
      handleMove(game);
    }

  } else {
    Color color;
    if (checkForCheck(game) && ((game.board[game.selectedPly.from.y][game.selectedPly.from.x] == BKING || game.board[game.selectedPly.from.y][game.selectedPly.from.x] == WKING))) {
      color = RED;
    } else if (game.selectedPly.from.x == game.previousPly.to.x && game.selectedPly.from.y == game.previousPly.to.y) {
      color = LIGHT_OLIVE;
    } else if ((game.selectedPly.from.x + game.selectedPly.from.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (game.selectedPly.from.x) * SQUARE_SIZE, (game.selectedPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(game.board[game.selectedPly.from.y][game.selectedPly.from.x], game.selectedPly.from.x, game.selectedPly.from.y);
  }
}

// runs the relevent operation if one of the other buttons are pressed
void runButtons(int p_x, int p_y, struct NotationState &notation) {
  uint8_t button = pressedButton(p_x, p_y);

  if (button == 1) {
    notation.states.up = true;
    drawButtons();
    if (notation.notationPaneNo > 0) {
      notation.notationPaneNo -= 1;
      drawNotation();
    }
  } else if (button == 2) {
    notation.states.down = true;
    drawButtons();
    if (notation.notationPaneNo < ((notation.currentPlyNumber > 0 ? (notation.currentPlyNumber - 1) / 2 : 0) )/MAX_LINES_PER_PANE) {
      notation.notationPaneNo += 1;
      drawNotation();
    }
  }
}

// redraws the buttons as unpressed after the delay has passed
void updateNotationButtons() {
  if (notation.states.up && validDelay()) {
      notation.states.up = false;
      drawUpButton(NOTATION_BACK, NOTATION_FRONT);
    }
    if (notation.states.down && validDelay()) {
      notation.states.down = false;
      drawDownButton(NOTATION_BACK, NOTATION_FRONT);
    }
}


// ====== Input helper functions ======

// Check if the delay has been long enough since the last press to consider another press
bool validDelay() {
  return (millis() - MILLIS_BETWEEN_PRESS > lastPress);
}

// Check that the given pressure is enough to be considered a press
bool validPress(const int16_t z) {
  return (z > MINPRESSURE && z < MAXPRESSURE);
}

// Poll on-screen buttons for presses; NO OVERLAPPING BUTTONS
uint8_t pressedButton(const uint16_t p_x, const uint16_t p_y) {
  if(p_x>=22 && p_x<=57 && p_y>=208 && p_y<=221) {
    return 1; // Up button pressed
  }
  if (p_x>=22 && p_x<=57 && p_y>=223 && p_y<=236) {
    return 2; // Down button pressed
  }

  return 0; // Return 0 if no button pressed
}

// Check if the chess board has been pressed given the input coordinates
bool pressedBoard(const int p_x, const int p_y) {
  return p_x > BOARD_BUFFER;
}
// Finds the square on the chess board.    DOES NOT CHECK IF THE CHESS BOARD IS PRESSED -> will return 0,0 if invalid
struct Square findSquareFromScreenPos(const int p_x, const int p_y) {
  struct Square square = { 0, 0, 3 };
  int x;
  int y;

  // sx,sy refer to the coords of the square related to the board
  for (int sy = 0; sy < 8; sy += 1) {
    for (int sx = 0; sx < 8; sx += 1) {

      x = BOARD_BUFFER + sx * SQUARE_SIZE;
      y = sy * SQUARE_SIZE;
      if (x < p_x - 5 && y < p_y + 5 && x + SQUARE_SIZE > p_x - 5 && y + SQUARE_SIZE > p_y + 5) {
        square.x = sx;
        square.y = sy;
        square.promo = 0;
        break;
      }
    }
  }
  // Serial.print(square.x);Serial.print(" ");Serial.println(square.y);
  return square;
}

