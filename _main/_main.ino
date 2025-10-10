// Author: Ethan Larner. Project started: Sept 2022
#include <Elegoo_GFX.h>     // Core graphics library
#include <Elegoo_TFTLCD.h>  // Hardware-specific library
#include <TouchScreen.h>

#define LCD_CS A3  // Chip Select goes to Analog 3
#define LCD_CD A2  // Command/Data goes to Analog 2
#define LCD_WR A1  // LCD Write goes to Analog 1
#define LCD_RD A0  // LCD Read goes to Analog 0

#define SD_SCK_PIN 13

#define LCD_RESET A4  // Can alternately connect to reset pin

// Assign human-readable names to some common 16-bit color values:
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define KHAKI 0x84AD
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define DARK_BROWN 0xB44C
#define LIGHT_BROWN 0xEED6
#define DARK_OLIVE 0x6368
#define OLIVE 0xAD07
#define LIGHT_OLIVE 0xCE8D
#define BRICK_RED 0xC367
#define BEIGE_GREEN 0xAD6F

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


#define YP A3  // Must be an analog pin
#define XM A2  // Must be an analog pin
#define YM 9   // Can be a digital pin
#define XP 8   // Can be a digital pin

#define MINPRESSURE 5
#define MAXPRESSURE 1000  // possibly 100, needs tested

// Some funny magic numbers
#define TS_MINX 110
#define TS_MAXX 910

#define TS_MINY 70
#define TS_MAXY 900

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300); // Setup the TouchScreen Object
TSPoint p;

// Board buffer
#define BOARD_BUFFER 80
#define SQUARE_SIZE 30

// Chess piece IDs
#define BLANK_SPACE 0

#define WPAWN 1
#define WKNIGHT 2
#define WBISHOP 3
#define WROOK 4
#define WQUEEN 5
#define WKING 6

#define BPAWN 7
#define BKNIGHT 8
#define BBISHOP 9
#define BROOK 10
#define BQUEEN 11
#define BKING 12

struct Square {
      byte x : 3; // x coord, 0-7
      byte y : 3; // y coord, 0-7
      byte : 2; //empty as 3bit doesn't fit neatly.
};

//                          white         black
//                        left  right   left  right
bool rook_movement[4] = { false, false, false, false };

// Chess board with initial setup.   To access a square, use the notation:' board[y][x] ', where x,y are the coords of the square.  0<= x,y <=7
uint8_t board[8][8] = {
  { BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK },
  { BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN },
  { WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK }
};

// Bitboard overlay, where "1" indicates a filled in pixel, and "0" indicates a pixel left untouched. Use pgm_read_byte(&capture_pic[x][y]) to load the bool
static const bool capture_pic[SQUARE_SIZE][SQUARE_SIZE] PROGMEM = { { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
                                                                    { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
                                                                    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                                                                    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
                                                                    { 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
                                                                    { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
                                                                    { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 } };

// Blank board used in move validation
uint8_t hypothetical_board[8][8];

bool turn = 0;  // 0 = white, 1 = black         boolean may be better?
bool white_king_moved = false;
bool black_king_moved = false;

// Setup the serial connection (if applicable), the tft interface, and then finally the initial board
void setup() {
  Serial.begin(9600);

  #ifdef USE_Elegoo_SHIELD_PINOUT
    // Elegoo 2.8\" TFT Arduino Shield Pinout
  #else
    // Elegoo 2.8\" TFT Breakout Board Pinout
  #endif

  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {        // ILI9325 LCD driver
  } else if (identifier == 0x9328) { // ILI9328 LCD driver
  } else if (identifier == 0x4535) { // LGDP4535 LCD driver
  } else if (identifier == 0x7575) { // HX8347G LCD driver
  } else if (identifier == 0x9341) { // ILI9341 LCD driver
  } else if (identifier == 0x8357) { // X8357D LCD driver
  } else if (identifier == 0x0101) { 
    identifier = 0x9341;             // 0x9341 LCD driver
  } else {
    identifier = 0x9341;             // Unknown LCD driver chip
  }

  tft.begin(identifier);

  pinMode(SD_SCK_PIN, OUTPUT);

  tft.setRotation(1);
  tft.fillScreen(BLACK);

  drawBoard(board);
}

void drawPiece(uint8_t id, int x, int y) {
  if (turn == 0) {
    const int buffer = 5; // Buffer to center the piece on its square on the board
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
  } else {
    const int buffer = 24; // Buffer to center the piece on its square on the board. (I think the logic is 30-5 and it didn't look center, 24 was better centered)
    switch (id) {
      case BLANK_SPACE:
        break;
      case WPAWN:
        drawPawn2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
        break;
      case WKNIGHT:
        drawKnight2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
        break;
      case WBISHOP:
        drawBishop2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
        break;
      case WROOK:
        drawRook2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
        break;
      case WQUEEN:
        drawQueen2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
        break;
      case WKING:
        drawKing2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, WHITE, BLACK);
        break;
      case BPAWN:
        drawPawn2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
        break;
      case BKNIGHT:
        drawKnight2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
        break;
      case BBISHOP:
        drawBishop2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
        break;
      case BROOK:
        drawRook2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
        break;
      case BQUEEN:
        drawQueen2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
        break;
      case BKING:
        drawKing2(BOARD_BUFFER + (x)*SQUARE_SIZE + buffer, (y)*SQUARE_SIZE + buffer, BLACK, WHITE);
        break;
    }
  }
}

void updateBoard(uint8_t board[8][8], int8_t last_squares[4], int8_t current_squares[4]) {  
  // Turns all pieces to face correct direction
  // Only updates squares with pieces to save time
  // x, y are the coords of the square currently being updated

  int i = 1; // Incremented each loop, even = dark square, odd = light
  
  for (int y = 0; y < 8; y += 1) {
    for (int x = 0; x < 8; x += 1) {
      if ((x == last_squares[0] && y == last_squares[1]) || (x == last_squares[2] && y == last_squares[3]) || (x == current_squares[0] && y == current_squares[1]) || (x == current_squares[2] && y == current_squares[3])) {
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

void drawBoard(uint8_t board[8][8]) {
  // Similar to update board, does not consider the squares currently pressed

  int i = 1; // Incremented each loop, even = dark square, odd = light
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

int8_t current_king_location[2] = { -1, -1 };  // Stores the location of the king to add red "check" effects. LEAVE AS A SIGNED TYPE

bool selectingPiece = true; // true for the first press, false for the second press
bool promotingPiece = false;    // Promotion menu activity indicator
bool cancel_promotion = false;  // Flag to cancel the promotion menu

bool castle_alert = false;  // Flag for the "castle" move
bool passant_alert = false; // Flag for the "en passant" move

int8_t current_squares[4] = { 0, 0, 0, 0 };  // Coords for both squares for the current half-move
int8_t last_squares[4] = { -1, -1, -1, -1 }; // Coords for both squares for the last half-move

#define MILLIS_BETWEEN_PRESS 250 // The delay required between presses on the screen. Prevents double pressing the same spot accidently
unsigned long lastPress = millis();

uint16_t color; // general purpose color definition, used in the loop and other functions. Make sure it is reset before use


void loop() {
  digitalWrite(SD_SCK_PIN, HIGH);
  p = ts.getPoint(); // TSPoint instance
  digitalWrite(SD_SCK_PIN, LOW);

  // Confirm the direction of the pins
  
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  
  // Check that the pressure is enough to be considered a press
  if (validPress(p.z)) {
    lastPress = millis();

    //Serial.print(p.x);Serial.print(" ");Serial.println(p.y);
    int p_x = 330-map(p.y, TS_MINY, TS_MAXY, 0, 320);
    int p_y = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    //Serial.print(p_x);Serial.print(" ");Serial.println(p_y);
    

    // x,y refer to the coords of the pixel; sx,sy refer to the coords of the square related to the board; nx,ny refer to the promotion square if relevent
    int x, y;
    for (int sy = 0; sy < 8; sy += 1) {
      for (int sx = 0; sx < 8; sx += 1) {

        x = BOARD_BUFFER + sx * SQUARE_SIZE;
        y = sy * SQUARE_SIZE;
        if (x < p_x - 5 && y < p_y + 5 && x + SQUARE_SIZE > p_x - 5 && y + SQUARE_SIZE > p_y + 5) {
          if (selectingPiece) { //----- First press on screen

            if (!((board[sy][sx] < BPAWN && turn == 0) || (board[sy][sx] > WKING && turn == 1))) {
              break;
            }
            current_squares[0] = sx;
            current_squares[1] = sy;
            if (board[current_squares[1]][current_squares[0]] == BLANK_SPACE) { break; }

            draw_possible_moves(last_squares, current_squares, board);

            tft.fillRect(BOARD_BUFFER + sx * SQUARE_SIZE, sy * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, KHAKI);
            drawPiece(board[sy][sx], sx, sy);
            selectingPiece = false;

            break;
          } else {              //----- Second press on screen
          
            delete_possible_moves(last_squares, current_squares, board);
            current_squares[2] = sx;
            current_squares[3] = sy;

            if (validMove(current_squares[0], current_squares[1], current_squares[2], current_squares[3], last_squares, board)) {
              if ((board[current_squares[1]][current_squares[0]] == WPAWN && current_squares[3] == 0) || (board[current_squares[1]][current_squares[0]] == BPAWN && current_squares[3] == 7)) {
                if (board[current_squares[1]][current_squares[0]] == WPAWN) {
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(WQUEEN, current_squares[2], current_squares[3]);

                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(WROOK, current_squares[2], current_squares[3] + 1);

                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(WBISHOP, current_squares[2], current_squares[3] + 2);

                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(WKNIGHT, current_squares[2], current_squares[3] + 3);
                } else {
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(BQUEEN, current_squares[2], current_squares[3]);

                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(BROOK, current_squares[2], current_squares[3] - 1);

                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(BBISHOP, current_squares[2], current_squares[3] - 2);

                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
                  tft.drawRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
                  drawPiece(BKNIGHT, current_squares[2], current_squares[3] - 3);
                }


                promotingPiece = true;
                int nx = -1;
                int ny = -1;
                while (promotingPiece) {
                  digitalWrite(SD_SCK_PIN, HIGH);
                  TSPoint p = ts.getPoint();
                  digitalWrite(SD_SCK_PIN, LOW);

                  pinMode(XM, OUTPUT);
                  pinMode(YP, OUTPUT);

                  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
                    if (millis() - MILLIS_BETWEEN_PRESS > lastPress) {
                      lastPress = millis();
                      int p_x = 330-map(p.y, TS_MINY, TS_MAXY, 0, 320);
                      int p_y = map(p.x, TS_MINX, TS_MAXX, 240, 0);

                      // x,y refer to the coords of the pixel; sx,sy refer to the coords of the square related to the board 
                      for (int sy = 0; sy < 8; sy += 1) {
                        for (int sx = 0; sx < 8; sx += 1) {
                          x = BOARD_BUFFER + (sx) * SQUARE_SIZE;
                          y = sy * SQUARE_SIZE;
                          if (x < p_x - 5 && y < p_y + 5 && x + SQUARE_SIZE > p_x - 5 && y + SQUARE_SIZE > p_y + 5) {
                            nx = sx;
                            ny = sy;
                            promotingPiece = false;
                            sy = 8;  // this breaks the 'for' loop faster
                            break;
                          }
                        }
                      }
                      promotingPiece = false;
                    }
                  }
                }

                if (board[current_squares[1]][current_squares[0]] == WPAWN) {
                  if ((current_squares[2] + current_squares[3]) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3]][current_squares[2]], current_squares[2], current_squares[3]);

                  if ((current_squares[2] + current_squares[3] + 1) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3] + 1][current_squares[2]], current_squares[2], current_squares[3] + 1);

                  if ((current_squares[2] + current_squares[3] + 2) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3] + 2][current_squares[2]], current_squares[2], current_squares[3] + 2);

                  if ((current_squares[2] + current_squares[3] + 3) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3] + 3][current_squares[2]], current_squares[2], current_squares[3] + 3);
                } else {
                  if ((current_squares[2] + current_squares[3]) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3]][current_squares[2]], current_squares[2], current_squares[3]);

                  if ((current_squares[2] + current_squares[3] - 1) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3] - 1][current_squares[2]], current_squares[2], current_squares[3] - 1);

                  if ((current_squares[2] + current_squares[3] - 2) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3] - 2][current_squares[2]], current_squares[2], current_squares[3] - 2);

                  if ((current_squares[2] + current_squares[3] - 3) % 2 == 1) {
                    color = DARK_BROWN;
                  } else {
                    color = LIGHT_BROWN;
                  }
                  tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3] - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                  drawPiece(board[current_squares[3] - 3][current_squares[2]], current_squares[2], current_squares[3] - 3);
                }


                // Interpretation of the promotion
                if (nx == -1 || ny == -1 || nx != current_squares[2]) {
                  cancel_promotion = true; /* user did not press a square or pressed a square on the wrong file */

                } else if (board[current_squares[1]][current_squares[0]] == WPAWN) {
                  if (ny == 0) {
                    board[current_squares[1]][current_squares[0]] = WQUEEN;  // Directly changes the pawn to the piece ; see first line after promote menu
                  } else if (ny == current_squares[3] + 1) {
                    board[current_squares[1]][current_squares[0]] = WROOK;
                  } else if (ny == current_squares[3] + 2) {
                    board[current_squares[1]][current_squares[0]] = WBISHOP;
                  } else if (ny == current_squares[3] + 3) {
                    board[current_squares[1]][current_squares[0]] = WKNIGHT;
                  }
                } else if (board[current_squares[1]][current_squares[0]] == BPAWN) {
                  if (ny == current_squares[3]) {
                    board[current_squares[1]][current_squares[0]] = BQUEEN;  // Directly changes the pawn to the piece ; see first line after promote menu
                  } else if (ny == current_squares[3] - 1) {
                    board[current_squares[1]][current_squares[0]] = BROOK;
                  } else if (ny == current_squares[3] - 2) {
                    board[current_squares[1]][current_squares[0]] = BBISHOP;
                  } else if (ny == current_squares[3] - 3) {
                    board[current_squares[1]][current_squares[0]] = BKNIGHT;
                  }
                }
              }
              if (!cancel_promotion) {
                if (turn == 0) {
                  turn = 1;
                } else {
                  turn = 0;
                }

                // rook move check  -  hard-coded values                           left white rook                                                                                                   right white rook                                                                                                                                    black left rook                                                                                                                              black right rook
                if (board[current_squares[1]][current_squares[0]] == WROOK && current_squares[0] == 0 && current_squares[1] == 7) {
                  rook_movement[0] = true;
                } else if (board[current_squares[1]][current_squares[0]] == WROOK && current_squares[0] == 7 && current_squares[1] == 7) {
                  rook_movement[1] = true;
                } else if (board[current_squares[1]][current_squares[0]] == BROOK && current_squares[0] == 0 && current_squares[1] == 0) {
                  rook_movement[2] = true;
                } else if (board[current_squares[1]][current_squares[0]] == WROOK && current_squares[0] == 7 && current_squares[1] == 0) {
                  rook_movement[3] = true;
                }

                // king move check
                if (board[current_squares[1]][current_squares[0]] == WKING && current_squares[0] == 4 && current_squares[1] == 7) {
                  white_king_moved = true;
                } else if (board[current_squares[1]][current_squares[0]] == BKING && current_squares[0] == 4 && current_squares[1] == 0) {
                  black_king_moved = true;
                }

                board[current_squares[3]][current_squares[2]] = board[current_squares[1]][current_squares[0]];
                board[current_squares[1]][current_squares[0]] = BLANK_SPACE;



                if (passant_alert == true) {
                  passant_alert = false;
                  board[last_squares[3]][last_squares[2]] = BLANK_SPACE;
                }
                if (castle_alert) {
                  castle_alert = false;
                  if (board[current_squares[3]][current_squares[2]] == WKING) {
                    if (current_squares[0] - current_squares[2] > 0) {
                      board[7][0] = BLANK_SPACE;
                      board[7][3] = WROOK;
                    } else {
                      board[7][7] = BLANK_SPACE;
                      board[7][5] = WROOK;
                    }
                  } else if (current_squares[0] - current_squares[2] > 0) {
                    board[0][0] = BLANK_SPACE;
                    board[0][3] = BROOK;
                  } else {
                    board[0][7] = BLANK_SPACE;
                    board[0][5] = BROOK;
                  }
                }
                //drawBoard(board);

                // remove last move just moved colors
                if ((last_squares[0] + last_squares[1]) % 2 == 1) {
                  color = DARK_BROWN;
                } else {
                  color = LIGHT_BROWN;
                }
                tft.fillRect(BOARD_BUFFER + (last_squares[0]) * SQUARE_SIZE, (last_squares[1]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                drawPiece(board[last_squares[1]][last_squares[0]], last_squares[0], last_squares[1]);
                if ((last_squares[2] + last_squares[3]) % 2 == 1) {
                  color = DARK_BROWN;
                } else {
                  color = LIGHT_BROWN;
                }
                tft.fillRect(BOARD_BUFFER + (last_squares[2]) * SQUARE_SIZE, (last_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                drawPiece(board[last_squares[3]][last_squares[2]], last_squares[2], last_squares[3]);


                // change squares to just moved colors
                tft.fillRect(BOARD_BUFFER + (current_squares[0]) * SQUARE_SIZE, (current_squares[1]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, OLIVE);
                drawPiece(board[current_squares[1]][current_squares[0]], current_squares[0], current_squares[1]);
                tft.fillRect(BOARD_BUFFER + (current_squares[2]) * SQUARE_SIZE, (current_squares[3]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, LIGHT_OLIVE);
                drawPiece(board[current_squares[3]][current_squares[2]], current_squares[2], current_squares[3]);

                last_squares[0] = current_squares[0];
                last_squares[1] = current_squares[1];
                last_squares[2] = current_squares[2];
                last_squares[3] = current_squares[3];
                updateBoard(board, last_squares, current_squares);

                // Add red effect if in 'check'
                if (check_for_check(last_squares, board)) {
                  find_king(board);

                  tft.fillRect(BOARD_BUFFER + (current_king_location[0]) * SQUARE_SIZE, (current_king_location[1]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);
                  drawPiece(board[current_king_location[1]][current_king_location[0]], current_king_location[0], current_king_location[1]);
                }



              } else {
                if (current_squares[0] == last_squares[2] && current_squares[1] == last_squares[3]) {
                  color = LIGHT_OLIVE;
                } else if ((current_squares[0] + current_squares[1]) % 2 == 1) {
                  color = DARK_BROWN;
                } else {
                  color = LIGHT_BROWN;
                }
                tft.fillRect(BOARD_BUFFER + (current_squares[0]) * SQUARE_SIZE, (current_squares[1]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
                drawPiece(board[current_squares[1]][current_squares[0]], current_squares[0], current_squares[1]);
                cancel_promotion = false;
              }
            } else {
              if (check_for_check(last_squares, board) && ((board[current_squares[1]][current_squares[0]] == BKING || board[current_squares[1]][current_squares[0]] == WKING))) {
                color = RED;
              } else if (current_squares[0] == last_squares[2] && current_squares[1] == last_squares[3]) {
                color = LIGHT_OLIVE;
              } else if ((current_squares[0] + current_squares[1]) % 2 == 1) {
                color = DARK_BROWN;
              } else {
                color = LIGHT_BROWN;
              }
              tft.fillRect(BOARD_BUFFER + (current_squares[0]) * SQUARE_SIZE, (current_squares[1]) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
              drawPiece(board[current_squares[1]][current_squares[0]], current_squares[0], current_squares[1]);
            }
            selectingPiece = true;

            break;
          }
        }
      }
    }
  }
  
}

// Check that the given pressure is enough to be considered a press
bool validPress(int16_t z) {
  return (z > MINPRESSURE && z < MAXPRESSURE && millis() - MILLIS_BETWEEN_PRESS > lastPress);
}

struct Square findSquareFromScreenPos(int x, int y) {
  struct Square square;
  square.x = -1;square.y = -1;

  int x, y;
  for (int sy = 0; sy < 8; sy += 1) {
    for (int sx = 0; sx < 8; sx += 1) {

      x = BOARD_BUFFER + sx * SQUARE_SIZE;
      y = sy * SQUARE_SIZE;
      if (x < p_x - 5 && y < p_y + 5 && x + SQUARE_SIZE > p_x - 5 && y + SQUARE_SIZE > p_y + 5) {
        square.x = sx;
        square.y = sy;
        break;
      }

  return square;
}



// Big valid move function, checks all rules (castling, en passent, check, etc)
bool validMove(int x1, int y1, int x2, int y2, int8_t last_moves[4], uint8_t board[8][8]) {  //                             VALIDATE                    MOVE
                                                                                     //                                             ^ en passant ^
  // initial checks                   |--------------------------------(WHITE)---------------------------------|     |--------------------(BLACK)----------------|
  //   check if first space blank                                                                      check if own capture
  if (board[y1][x1] == BLANK_SPACE || (board[y1][x1] < BPAWN && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE) || (board[y1][x1] > WKING && board[y2][x2] > WKING)) { return false; }

  // check if move is valid normally
  uint8_t id = board[y1][x1];

  if (id == WPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE || (y1 == 6 && ((y2 + 2 == y1 && board[y1 - 1][x1] == BLANK_SPACE && board[y1 - 2][x1] == BLANK_SPACE) || y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 + 1 == y1 && board[y2][x2] > WKING)) {
    } else if (
      abs(x1 - x2) == 1 && y2 + 1 == y1 && y1 == 3 /*&& board[y2+1][x2] > WKING*/ && last_moves[1] == 1 && last_moves[2] == x2 && last_moves[3] == 3 && board[last_moves[3]][last_moves[2]] == BPAWN) {
      passant_alert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && last_moves[1] == 6 && last_moves[2] == x2 && last_moves[3] == 4 && board[last_moves[3]][last_moves[2]] == WPAWN) {
      passant_alert = true;
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
    int white_king_moved = false;
    int black_king_moved = false;

  } else if (id == WKING) {
    // The king can move a maximum of one in any direction
    if (!(abs(x1 - x2) > 1 || abs(y1 - y2) > 1)) {
    } else if ((!white_king_moved) && abs(x1 - x2) == 2 && abs(y1 - y2) == 0) {
      if ((x1 - x2 > 0 && !rook_movement[0] && board[7][3] == BLANK_SPACE && board[7][2] == BLANK_SPACE && board[7][1] == BLANK_SPACE) || (x1 - x2 < 0 && !rook_movement[1] && board[7][5] == BLANK_SPACE)) {
        castle_alert = true;
      } else {
        return false;
      }
    } else {
      return false;
    }

  } else if (id == BKING) {
    // The king can move a maximum of one in any direction
    if (!(abs(x1 - x2) > 1 || abs(y1 - y2) > 1)) {
    } else if ((!black_king_moved) && abs(x1 - x2) == 2 && abs(y1 - y2) == 0) {
      if ((x1 - x2 > 0 && !rook_movement[2] && board[0][3] == BLANK_SPACE && board[0][2] == BLANK_SPACE && board[0][1] == BLANK_SPACE) || (x1 - x2 < 0 && !rook_movement[3] && board[0][5] == BLANK_SPACE)) {
        castle_alert = true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  // check if king is capturable after move


  if (castle_alert) {
    if (check_for_check(last_moves, board)) { return false; }  // starts in check
    if (x1 - x2 > 0) {
      update_hypothetical(board);
      hypothetical_board[y2][x2 + 1] = hypothetical_board[y1][x1];
      hypothetical_board[y1][x1] = BLANK_SPACE;
      if (check_for_check(last_moves, hypothetical_board)) { return false; }  /// passes through check
    } else if (x1 - x2 < 0) {
      update_hypothetical(board);
      hypothetical_board[y2][x2 - 1] = hypothetical_board[y1][x1];
      hypothetical_board[y1][x1] = BLANK_SPACE;
      if (check_for_check(last_moves, hypothetical_board)) { return false; }
    }
  }
  update_hypothetical(board);

  //memcpy(hypothetical_board, board, 8);
  hypothetical_board[y2][x2] = hypothetical_board[y1][x1];
  hypothetical_board[y1][x1] = BLANK_SPACE;
  if (passant_alert == true) {
    hypothetical_board[last_squares[3]][last_squares[2]] = BLANK_SPACE;
  }



  int kx = 0;
  int ky = 0;

  if (hypothetical_board[y2][x2] < BPAWN) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {

        if (hypothetical_board[y][x] == WKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypothetical_board[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypothetical_board)) {
            passant_alert = false;
            castle_alert = false;
            return false;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypothetical_board[y][x] == BKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             BLACK
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypothetical_board[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypothetical_board)) {
            passant_alert = false;
            castle_alert = false;
            return false;
          }
        }
      }
    }
  }



  return true;  // return true if all checks say the move could be valid
}

// Checks if the king is in check and if so, updates the "castle_alert" flag as relevant
bool check_for_check(int8_t last_moves[4], uint8_t board[8][8]) {
  update_hypothetical(board);
  int kx = 0;
  int ky = 0;

  if (turn == 0) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {

        if (hypothetical_board[y][x] == WKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypothetical_board[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypothetical_board)) {
            castle_alert = false;
            return true;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypothetical_board[y][x] == BKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             BLACK
    }
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypothetical_board[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypothetical_board)) {
            castle_alert = false;
            return true;
          }
        }
      }
    }
  }
  return false;
}

// Checks if the move is valid, without checking if the king is in "check"
bool validMoveWithoutCheck(int x1, int y1, int x2, int y2, int8_t last_moves[4], uint8_t board[8][8]) {  //                             VALIDATE                    MOVE
                                                                                                 //                                             ^ en passant ^
  // initial checks                   |--------------------------------(WHITE)---------------------------------|     |--------------------(BLACK)----------------|
  //   check if first space blank                                                                      check if own capture
  if (board[y1][x1] == BLANK_SPACE || (board[y1][x1] < BPAWN && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE) || (board[y1][x1] > WKING && board[y2][x2] > WKING)) { return false; }

  // check if move is valid normally
  uint8_t id = board[y1][x1];

  if (id == WPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE || (y1 == 6 && ((y2 + 2 == y1 && board[y1 - 1][x1] == BLANK_SPACE && board[y1 - 2][x1] == BLANK_SPACE) || y2 + 1 == y1 && board[y1 - 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 + 1 == y1 && board[y2][x2] > WKING)) {
    } else if (
      abs(x1 - x2) == 1 && y2 + 1 == y1 && y1 == 3 /*&& board[y2+1][x2] > WKING*/ && last_moves[1] == 1 && last_moves[2] == x2 && last_moves[3] == 3 && board[last_moves[3]][last_moves[2]] == BPAWN) {
      passant_alert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && last_moves[1] == 6 && last_moves[2] == x2 && last_moves[3] == 4 && board[last_moves[3]][last_moves[2]] == WPAWN) {
      passant_alert = true;
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
void update_hypothetical(uint8_t from[8][8]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      hypothetical_board[y][x] = from[y][x];
    }
  }
}

// Function has been updated, if problems return to version 1
void find_king(uint8_t board[8][8]) {
    uint8_t king = (turn == 0) ? WKING : BKING; // Set the king's piece based on the turn
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[y][x] == king) {
                current_king_location[0] = x;
                current_king_location[1] = y;
                return; // Return early once the king is found
            }
        }
    }
}

void draw_possible_moves(int8_t last_squares[4], int8_t current_squares[4], uint8_t board[8][8]) {
  uint16_t color;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (validMove(current_squares[0], current_squares[1], x, y, last_squares, board)) {
        if (board[y][x] == BLANK_SPACE) {
          tft.fillCircle(BOARD_BUFFER + (x)*SQUARE_SIZE + 15, (y)*SQUARE_SIZE + 15, 4, DARK_OLIVE);

        } else {
          if (last_squares[2] == x && last_squares[3] == y) {
            color = LIGHT_OLIVE;
          } else if ((x + y) % 2 == 1) {
            color = DARK_BROWN;
          } else {
            color = LIGHT_BROWN;
          }
          //tft.fillRect(BOARD_BUFFER+(x)*SQUARE_SIZE, (y)*SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
          for (int j = 0; j < SQUARE_SIZE; j++) {
            for (int i = 0; i < SQUARE_SIZE; i++) {
              if (pgm_read_byte(&capture_pic[j][i]) == 1) { tft.drawPixel((BOARD_BUFFER + (x)*SQUARE_SIZE) + i, (y)*SQUARE_SIZE + j, DARK_OLIVE); }
            }
          }
          drawPiece(board[y][x], x, y);
        }
      }
    }
  }
  passant_alert = false;
  castle_alert = false;
}

void delete_possible_moves(int8_t last_squares[4], int8_t current_squares[4], uint8_t board[8][8]) {
  uint16_t color;
  
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (validMove(current_squares[0], current_squares[1], x, y, last_squares, board)) {
        if (board[y][x] == BLANK_SPACE) {
          if (last_squares[0] == x && last_squares[1] == y) {
            color = OLIVE;
          } else if ((x + y) % 2 == 1) {
            color = DARK_BROWN;
          } else {
            color = LIGHT_BROWN;
          }
          tft.fillCircle(BOARD_BUFFER + (x)*SQUARE_SIZE + 15, (y)*SQUARE_SIZE + 15, 4, color);

        } else {
          if (last_squares[2] == x && last_squares[3] == y) {
            color = LIGHT_OLIVE;
          } else if ((x + y) % 2 == 1) {
            color = DARK_BROWN;
          } else {
            color = LIGHT_BROWN;
          }
          
          for (int j = 0; j < SQUARE_SIZE; j++) {
            for (int i = 0; i < SQUARE_SIZE; i++) {
              if (pgm_read_byte(&capture_pic[j][i]) == 1) { tft.drawPixel((BOARD_BUFFER + (x)*SQUARE_SIZE) + i, (y)*SQUARE_SIZE + j, color); }
            }
          }
          drawPiece(board[y][x], x, y);
        }
      }
    }
  }
  passant_alert = false;
  castle_alert = false;
}

// Space to distinguish draw functions







void drawKnight(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) { // Enhanced using .drawFastHLine()
  tft.drawPixel(x + 7, y, color_2);
  tft.drawPixel(x + 9, y, color_2);

  tft.drawFastHLine(x + 6, y + 1, 5, color_2);
  tft.drawPixel(x + 7, y + 1, color_1);
  tft.drawPixel(x + 9, y + 1, color_1);
  

  tft.drawPixel(x + 5, y + 2, color_2);
  tft.drawFastHLine(x + 6, y + 2, 5, color_1);
  tft.drawPixel(x + 11, y + 2, color_2);

  tft.drawPixel(x + 4, y + 3, color_2);
  tft.drawFastHLine(x + 5, y + 3, 7, color_1);
  tft.drawPixel(x + 12, y + 3, color_2);

  tft.drawPixel(x + 4, y + 4, color_2);
  tft.drawFastHLine(x + 5, y + 4, 8, color_1);
  tft.drawPixel(x + 13, y + 4, color_2);

  tft.drawPixel(x + 3, y + 5, color_2);
  tft.drawFastHLine(x + 4, y + 5, 10, color_1);
  tft.drawPixel(x + 12, y + 5, color_2);
  tft.drawPixel(x + 14, y + 5, color_2);

  tft.drawPixel(x + 3, y + 6, color_2);
  tft.drawFastHLine(x + 4, y + 6, 11, color_1);
  tft.drawPixel(x + 5, y + 6, color_2);
  tft.drawPixel(x + 13, y + 6, color_2);
  tft.drawPixel(x + 15, y + 6, color_2);

  tft.drawPixel(x + 2, y + 7, color_2);
  tft.drawFastHLine(x + 3, y + 7, 12, color_1);
  tft.drawPixel(x + 13, y + 7, color_2);
  tft.drawPixel(x + 15, y + 7, color_2);

  tft.drawFastHLine(x + 1, y + 8, 2, color_2);
  tft.drawFastHLine(x + 3, y + 8, 13, color_1);
  tft.drawPixel(x + 9, y + 8, color_2);
  tft.drawPixel(x + 14, y + 8, color_2);
  tft.drawPixel(x + 16, y + 8, color_2);

  tft.drawFastHLine(x + 0, y + 9, 2, color_2);
  tft.drawFastHLine(x + 2, y + 9, 14, color_1);
  tft.drawFastHLine(x + 8, y + 9, 3, color_2);
  tft.drawPixel(x + 14, y + 9, color_2);
  tft.drawPixel(x + 16, y + 9, color_2);

  tft.drawPixel(x + 0, y + 10, color_2);
  tft.drawFastHLine(x + 1, y + 10, 5, color_1);
  tft.drawFastHLine(x + 6, y + 10, 2, color_2);
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 10, y + 10, color_2);
  tft.drawFastHLine(x + 11, y + 10, 6, color_1);
  tft.drawPixel(x + 15, y + 10, color_2);
  tft.drawPixel(x + 17, y + 10, color_2);

  tft.drawFastHLine(x + 1, y + 11, 5, color_2);
  tft.drawPixel(x + 2, y + 11, color_1);
  tft.drawPixel(x + 4, y + 11, color_1);
  //------------------------------------
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 9, y + 11, color_2);
  tft.drawFastHLine(x + 10, y + 11, 7, color_1);
  tft.drawPixel(x + 15, y + 11, color_2);
  tft.drawPixel(x + 17, y + 11, color_2);

  tft.drawFastHLine(x + 1, y + 12, 4, color_2);
  //------------------------------------
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 8, y + 12, color_2);
  tft.drawFastHLine(x + 9, y + 12, 8, color_1);
  tft.drawPixel(x + 15, y + 12, color_2);
  tft.drawPixel(x + 17, y + 12, color_2);

  tft.drawPixel(x + 7, y + 13, color_2);
  tft.drawFastHLine(x + 8, y + 13, 9, color_1);
  tft.drawPixel(x + 15, y + 13, color_2);
  tft.drawPixel(x + 17, y + 13, color_2);

  tft.drawPixel(x + 6, y + 14, color_2);
  tft.drawFastHLine(x + 7, y + 14, 10, color_1);
  tft.drawPixel(x + 17, y + 14, color_2);

  tft.drawPixel(x + 5, y + 15, color_2);
  tft.drawFastHLine(x + 6, y + 15, 11, color_1);
  tft.drawPixel(x + 18, y + 15, color_2);

  tft.drawPixel(x + 5, y + 16, color_2);
  tft.drawFastHLine(x + 6, y + 16, 11, color_1);
  tft.drawPixel(x + 18, y + 16, color_2);

  tft.drawFastHLine(x + 4, y + 17, 16, color_2);

  tft.drawPixel(x + 4, y + 18, color_2);
  tft.drawFastHLine(x + 5, y + 18, 14, color_1);
  tft.drawPixel(x + 19, y + 18, color_2);

  tft.drawFastHLine(x + 4, y + 19, 16, color_2);
}

void drawPawn(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawFastHLine(x + 8, y + 0, 4, color_2);

  tft.drawPixel(x + 7, y + 1, color_2);
  tft.drawFastHLine(x + 8, y + 1, 4, color_1);
  tft.drawPixel(x + 12, y + 1, color_2);

  tft.drawPixel(x + 6, y + 2, color_2);
  tft.drawFastHLine(x + 7, y + 2, 6, color_1);
  tft.drawPixel(x + 13, y + 2, color_2);

  tft.drawPixel(x + 6, y + 3, color_2);
  tft.drawFastHLine(x + 7, y + 3, 6, color_1);
  tft.drawPixel(x + 13, y + 3, color_2);

  tft.drawPixel(x + 6, y + 4, color_2);
  tft.drawFastHLine(x + 7, y + 4, 6, color_1);
  tft.drawPixel(x + 13, y + 4, color_2);

  tft.drawPixel(x + 6, y + 5, color_2);
  tft.drawFastHLine(x + 7, y + 5, 6, color_1);
  tft.drawPixel(x + 13, y + 5, color_2);

  tft.drawPixel(x + 5, y + 6, color_2);
  tft.drawFastHLine(x + 6, y + 6, 8, color_1);
  tft.drawPixel(x + 14, y + 6, color_2);

  tft.drawFastHLine(x + 5, y + 7, 10,  color_2);//         //

  tft.drawFastHLine(x + 6, y + 8, 2, color_2);
  tft.drawFastHLine(x + 8, y + 8, 4, color_1);
  tft.drawFastHLine(x + 12, y + 8, 2, color_2);

  tft.drawPixel(x + 7, y + 9, color_2);
  tft.drawFastHLine(x + 8, y + 9, 4, color_1);
  tft.drawPixel(x + 12, y + 9, color_2);

  tft.drawFastHLine(x + 6, y + 10, 2, color_2);
  tft.drawFastHLine(x + 8, y + 10, 4, color_1);
  tft.drawFastHLine(x + 12, y + 10, 2, color_2);

  tft.drawPixel(x + 6, y + 11, color_2);
  tft.drawFastHLine(x + 7, y + 11, 6, color_1);
  tft.drawPixel(x + 13, y + 11, color_2);

  tft.drawPixel(x + 6, y + 12, color_2);
  tft.drawFastHLine(x + 7, y + 12, 6, color_1);
  tft.drawPixel(x + 13, y + 12, color_2);

  tft.drawPixel(x + 5, y + 13, color_2);
  tft.drawFastHLine(x + 6, y + 13, 8, color_1);
  tft.drawPixel(x + 14, y + 13, color_2);

  tft.drawFastHLine(x + 4, y + 14, 2, color_2);
  tft.drawFastHLine(x + 6, y + 14, 8, color_1);
  tft.drawFastHLine(x + 14, y + 14, 2, color_2);

  tft.drawPixel(x + 4, y + 15, color_2);
  tft.drawFastHLine(x + 5, y + 15, 10, color_1);
  tft.drawPixel(x + 15, y + 15, color_2);

  tft.drawPixel(x + 3, y + 16, color_2);
  tft.drawFastHLine(x + 4, y + 16, 12, color_1);
  tft.drawPixel(x + 16, y + 16, color_2);

  tft.drawFastHLine(x + 2, y + 17, 16, color_2);

  tft.drawPixel(x + 1, y + 18, color_2);
  tft.drawFastHLine(x + 2, y + 18, 16, color_1);
  tft.drawPixel(x + 18, y + 18, color_2);

  tft.drawFastHLine(x + 1, y + 19, 18, color_2);
}

void drawQueen(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawFastHLine(x + 6, y + 0, 3, color_2);
  tft.drawFastHLine(x + 11, y + 0, 3, color_2);

  tft.drawFastHLine(x + 6, y + 1, 3, color_2);
  tft.drawPixel(x + 7, y + 1, color_1);
  tft.drawFastHLine(x + 11, y + 1, 3, color_2);
  tft.drawPixel(x + 12, y + 1, color_1);

  tft.drawFastHLine(x + 0, y + 2, 3, color_2);
  tft.drawFastHLine(x + 6, y + 2, 3, color_2);
  tft.drawFastHLine(x + 11, y + 2, 3, color_2);
  tft.drawFastHLine(x + 17, y + 2, 3, color_2);

  tft.drawFastHLine(x + 0, y + 3, 3, color_2);
  tft.drawPixel(x + 1, y + 3, color_1);
  tft.drawPixel(x + 7, y + 3, color_2);
  tft.drawPixel(x + 12, y + 3, color_2);
  tft.drawFastHLine(x + 17, y + 3, 3, color_2);
  tft.drawPixel(x + 18, y + 3, color_1);

  tft.drawFastHLine(x + 0, y + 4, 3, color_2);
  tft.drawFastHLine(x + 6, y + 4, 3, color_2);
  tft.drawPixel(x + 7, y + 4, color_1);
  tft.drawFastHLine(x + 11, y + 4, 3, color_2);
  tft.drawPixel(x + 12, y + 4, color_1);
  tft.drawFastHLine(x + 17, y + 4, 3, color_2);

  tft.drawFastHLine(x + 1, y + 5, 2, color_2);
  tft.drawFastHLine(x + 6, y + 5, 3, color_2);
  tft.drawPixel(x + 7, y + 5, color_1);
  tft.drawFastHLine(x + 11, y + 5, 3, color_2);
  tft.drawPixel(x + 12, y + 5, color_1);
  tft.drawFastHLine(x + 17, y + 5, 2, color_2);

  tft.drawPixel(x + 2, y + 6, color_2);
  tft.drawFastHLine(x + 5, y + 6, 4, color_2);
  tft.drawPixel(x + 7, y + 6, color_1);
  tft.drawFastHLine(x + 11, y + 6, 4, color_2);
  tft.drawPixel(x + 12, y + 6, color_1);
  tft.drawPixel(x + 17, y + 6, color_2);

  tft.drawFastHLine(x + 2, y + 7, 2, color_2);
  tft.drawFastHLine(x + 5, y + 7, 10, color_2);
  tft.drawFastHLine(x + 6, y + 7, 3, color_1);
  tft.drawFastHLine(x + 11, y + 7, 3, color_1);
  tft.drawFastHLine(x + 16, y + 7, 2, color_2);

  tft.drawFastHLine(x + 2, y + 8, 16, color_2);
  tft.drawPixel(x + 3, y + 8, color_1);
  tft.drawFastHLine(x + 6, y + 8, 3, color_1);
  tft.drawFastHLine(x + 11, y + 8, 3, color_1);
  tft.drawPixel(x + 16, y + 8, color_1);

  tft.drawPixel(x + 2, y + 9, color_2);
  tft.drawFastHLine(x + 3, y + 9, 14, color_1);
  tft.drawPixel(x + 17, y + 9, color_2);

  tft.drawPixel(x + 3, y + 10, color_2);
  tft.drawFastHLine(x + 4, y + 10, 12, color_1);
  tft.drawPixel(x + 16, y + 10, color_2);

  tft.drawPixel(x + 3, y + 11, color_2);
  tft.drawFastHLine(x + 4, y + 11, 12, color_1);
  tft.drawPixel(x + 16, y + 11, color_2);

  tft.drawFastHLine(x + 3, y + 12, 2, color_2);
  tft.drawFastHLine(x + 5, y + 12, 10, color_1);
  tft.drawFastHLine(x + 15, y + 12, 2, color_2);

  tft.drawFastHLine(x + 4, y + 13, 12, color_2);

  tft.drawFastHLine(x + 3, y + 14, 2, color_2);
  tft.drawFastHLine(x + 5, y + 14, 10, color_1);
  tft.drawFastHLine(x + 15, y + 14, 2, color_2);

  tft.drawPixel(x + 3, y + 15, color_2);
  tft.drawFastHLine(x + 4, y + 15, 12, color_1);
  tft.drawPixel(x + 16, y + 15, color_2);

  tft.drawFastHLine(x + 3, y + 16, 14, color_2);

  tft.drawPixel(x + 2, y + 17, color_2);
  tft.drawFastHLine(x + 3, y + 17, 14, color_1);
  tft.drawPixel(x + 17, y + 17, color_2);

  tft.drawPixel(x + 2, y + 18, color_2);
  tft.drawFastHLine(x + 3, y + 18, 14, color_1);
  tft.drawPixel(x + 17, y + 18, color_2);

  tft.drawFastHLine(x + 2, y + 19, 16, color_2);
}

void drawKing(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x + 9, y + 0, color_2);
  tft.drawPixel(x + 10, y + 0, color_2);

  tft.drawPixel(x + 8, y + 1, color_2);
  tft.drawPixel(x + 9, y + 1, color_2);
  tft.drawPixel(x + 10, y + 1, color_2);
  tft.drawPixel(x + 11, y + 1, color_2);

  tft.drawPixel(x + 9, y + 2, color_2);
  tft.drawPixel(x + 10, y + 2, color_2);

  tft.drawPixel(x + 9, y + 3, color_2);
  tft.drawPixel(x + 10, y + 3, color_2);

  tft.drawPixel(x + 3, y + 4, color_2);
  tft.drawPixel(x + 4, y + 4, color_2);
  tft.drawPixel(x + 5, y + 4, color_2);
  tft.drawPixel(x + 6, y + 4, color_2);
  tft.drawPixel(x + 8, y + 4, color_2);
  tft.drawPixel(x + 9, y + 4, color_2);
  tft.drawPixel(x + 10, y + 4, color_2);
  tft.drawPixel(x + 11, y + 4, color_2);
  tft.drawPixel(x + 13, y + 4, color_2);
  tft.drawPixel(x + 14, y + 4, color_2);
  tft.drawPixel(x + 15, y + 4, color_2);
  tft.drawPixel(x + 16, y + 4, color_2);

  tft.drawPixel(x + 1, y + 5, color_2);
  tft.drawPixel(x + 2, y + 5, color_2);
  tft.drawPixel(x + 3, y + 5, color_2);
  tft.drawPixel(x + 4, y + 5, color_1);
  tft.drawPixel(x + 5, y + 5, color_1);
  tft.drawPixel(x + 6, y + 5, color_2);
  tft.drawPixel(x + 7, y + 5, color_2);
  tft.drawPixel(x + 8, y + 5, color_2);
  tft.drawPixel(x + 9, y + 5, color_1);
  tft.drawPixel(x + 10, y + 5, color_1);
  tft.drawPixel(x + 11, y + 5, color_2);
  tft.drawPixel(x + 12, y + 5, color_2);
  tft.drawPixel(x + 13, y + 5, color_2);
  tft.drawPixel(x + 14, y + 5, color_1);
  tft.drawPixel(x + 15, y + 5, color_1);
  tft.drawPixel(x + 16, y + 5, color_2);
  tft.drawPixel(x + 17, y + 5, color_2);
  tft.drawPixel(x + 18, y + 5, color_2);

  tft.drawPixel(x + 0, y + 6, color_2);
  tft.drawPixel(x + 1, y + 6, color_2);
  tft.drawPixel(x + 2, y + 6, color_1);
  tft.drawPixel(x + 3, y + 6, color_1);
  tft.drawPixel(x + 4, y + 6, color_1);
  tft.drawPixel(x + 5, y + 6, color_1);
  tft.drawPixel(x + 6, y + 6, color_1);
  tft.drawPixel(x + 7, y + 6, color_2);
  tft.drawPixel(x + 8, y + 6, color_1);
  tft.drawPixel(x + 9, y + 6, color_1);
  tft.drawPixel(x + 10, y + 6, color_1);
  tft.drawPixel(x + 11, y + 6, color_1);
  tft.drawPixel(x + 12, y + 6, color_2);
  tft.drawPixel(x + 13, y + 6, color_1);
  tft.drawPixel(x + 14, y + 6, color_1);
  tft.drawPixel(x + 15, y + 6, color_1);
  tft.drawPixel(x + 16, y + 6, color_1);
  tft.drawPixel(x + 17, y + 6, color_1);
  tft.drawPixel(x + 18, y + 6, color_2);
  tft.drawPixel(x + 19, y + 6, color_2);

  tft.drawPixel(x + 0, y + 7, color_2);
  tft.drawPixel(x + 1, y + 7, color_1);
  tft.drawPixel(x + 2, y + 7, color_1);
  tft.drawPixel(x + 3, y + 7, color_1);
  tft.drawPixel(x + 4, y + 7, color_1);
  tft.drawPixel(x + 5, y + 7, color_1);
  tft.drawPixel(x + 6, y + 7, color_1);
  tft.drawPixel(x + 7, y + 7, color_2);
  tft.drawPixel(x + 8, y + 7, color_2);
  tft.drawPixel(x + 9, y + 7, color_1);
  tft.drawPixel(x + 10, y + 7, color_1);
  tft.drawPixel(x + 11, y + 7, color_2);
  tft.drawPixel(x + 12, y + 7, color_2);
  tft.drawPixel(x + 13, y + 7, color_1);
  tft.drawPixel(x + 14, y + 7, color_1);
  tft.drawPixel(x + 15, y + 7, color_1);
  tft.drawPixel(x + 16, y + 7, color_1);
  tft.drawPixel(x + 17, y + 7, color_1);
  tft.drawPixel(x + 18, y + 7, color_1);
  tft.drawPixel(x + 19, y + 7, color_2);

  tft.drawPixel(x + 0, y + 8, color_2);
  tft.drawPixel(x + 1, y + 8, color_1);
  tft.drawPixel(x + 2, y + 8, color_1);
  tft.drawPixel(x + 3, y + 8, color_1);
  tft.drawPixel(x + 4, y + 8, color_1);
  tft.drawPixel(x + 5, y + 8, color_1);
  tft.drawPixel(x + 6, y + 8, color_1);
  tft.drawPixel(x + 7, y + 8, color_1);
  tft.drawPixel(x + 8, y + 8, color_2);
  tft.drawPixel(x + 9, y + 8, color_2);
  tft.drawPixel(x + 10, y + 8, color_2);
  tft.drawPixel(x + 11, y + 8, color_2);
  tft.drawPixel(x + 12, y + 8, color_1);
  tft.drawPixel(x + 13, y + 8, color_1);
  tft.drawPixel(x + 14, y + 8, color_1);
  tft.drawPixel(x + 15, y + 8, color_1);
  tft.drawPixel(x + 16, y + 8, color_1);
  tft.drawPixel(x + 17, y + 8, color_1);
  tft.drawPixel(x + 18, y + 8, color_1);
  tft.drawPixel(x + 19, y + 8, color_2);

  tft.drawPixel(x + 0, y + 9, color_2);
  tft.drawPixel(x + 1, y + 9, color_1);
  tft.drawPixel(x + 2, y + 9, color_1);
  tft.drawPixel(x + 3, y + 9, color_1);
  tft.drawPixel(x + 4, y + 9, color_1);
  tft.drawPixel(x + 5, y + 9, color_1);
  tft.drawPixel(x + 6, y + 9, color_1);
  tft.drawPixel(x + 7, y + 9, color_1);
  tft.drawPixel(x + 8, y + 9, color_1);
  tft.drawPixel(x + 9, y + 9, color_2);
  tft.drawPixel(x + 10, y + 9, color_2);
  tft.drawPixel(x + 11, y + 9, color_1);
  tft.drawPixel(x + 12, y + 9, color_1);
  tft.drawPixel(x + 13, y + 9, color_1);
  tft.drawPixel(x + 14, y + 9, color_1);
  tft.drawPixel(x + 15, y + 9, color_1);
  tft.drawPixel(x + 16, y + 9, color_1);
  tft.drawPixel(x + 17, y + 9, color_1);
  tft.drawPixel(x + 18, y + 9, color_1);
  tft.drawPixel(x + 19, y + 9, color_2);

  tft.drawPixel(x + 0, y + 10, color_2);
  tft.drawPixel(x + 1, y + 10, color_2);
  tft.drawPixel(x + 2, y + 10, color_1);
  tft.drawPixel(x + 3, y + 10, color_1);
  tft.drawPixel(x + 4, y + 10, color_1);
  tft.drawPixel(x + 5, y + 10, color_1);
  tft.drawPixel(x + 6, y + 10, color_1);
  tft.drawPixel(x + 7, y + 10, color_1);
  tft.drawPixel(x + 8, y + 10, color_1);
  tft.drawPixel(x + 9, y + 10, color_2);
  tft.drawPixel(x + 10, y + 10, color_2);
  tft.drawPixel(x + 11, y + 10, color_1);
  tft.drawPixel(x + 12, y + 10, color_1);
  tft.drawPixel(x + 13, y + 10, color_1);
  tft.drawPixel(x + 14, y + 10, color_1);
  tft.drawPixel(x + 15, y + 10, color_1);
  tft.drawPixel(x + 16, y + 10, color_1);
  tft.drawPixel(x + 17, y + 10, color_1);
  tft.drawPixel(x + 18, y + 10, color_2);
  tft.drawPixel(x + 19, y + 10, color_2);

  tft.drawPixel(x + 1, y + 11, color_2);
  tft.drawPixel(x + 2, y + 11, color_1);
  tft.drawPixel(x + 3, y + 11, color_1);
  tft.drawPixel(x + 4, y + 11, color_1);
  tft.drawPixel(x + 5, y + 11, color_1);
  tft.drawPixel(x + 6, y + 11, color_1);
  tft.drawPixel(x + 7, y + 11, color_1);
  tft.drawPixel(x + 8, y + 11, color_1);
  tft.drawPixel(x + 9, y + 11, color_2);
  tft.drawPixel(x + 10, y + 11, color_2);
  tft.drawPixel(x + 11, y + 11, color_1);
  tft.drawPixel(x + 12, y + 11, color_1);
  tft.drawPixel(x + 13, y + 11, color_1);
  tft.drawPixel(x + 14, y + 11, color_1);
  tft.drawPixel(x + 15, y + 11, color_1);
  tft.drawPixel(x + 16, y + 11, color_1);
  tft.drawPixel(x + 17, y + 11, color_1);
  tft.drawPixel(x + 18, y + 11, color_2);

  tft.drawPixel(x + 1, y + 12, color_2);
  tft.drawPixel(x + 2, y + 12, color_2);
  tft.drawPixel(x + 3, y + 12, color_1);
  tft.drawPixel(x + 4, y + 12, color_1);
  tft.drawPixel(x + 5, y + 12, color_1);
  tft.drawPixel(x + 6, y + 12, color_1);
  tft.drawPixel(x + 7, y + 12, color_1);
  tft.drawPixel(x + 8, y + 12, color_1);
  tft.drawPixel(x + 9, y + 12, color_2);
  tft.drawPixel(x + 10, y + 12, color_2);
  tft.drawPixel(x + 11, y + 12, color_1);
  tft.drawPixel(x + 12, y + 12, color_1);
  tft.drawPixel(x + 13, y + 12, color_1);
  tft.drawPixel(x + 14, y + 12, color_1);
  tft.drawPixel(x + 15, y + 12, color_1);
  tft.drawPixel(x + 16, y + 12, color_1);
  tft.drawPixel(x + 17, y + 12, color_2);
  tft.drawPixel(x + 18, y + 12, color_2);

  tft.drawPixel(x + 2, y + 13, color_2);
  tft.drawPixel(x + 3, y + 13, color_2);
  tft.drawPixel(x + 4, y + 13, color_1);
  tft.drawPixel(x + 5, y + 13, color_1);
  tft.drawPixel(x + 6, y + 13, color_1);
  tft.drawPixel(x + 7, y + 13, color_1);
  tft.drawPixel(x + 8, y + 13, color_1);
  tft.drawPixel(x + 9, y + 13, color_2);
  tft.drawPixel(x + 10, y + 13, color_2);
  tft.drawPixel(x + 11, y + 13, color_1);
  tft.drawPixel(x + 12, y + 13, color_1);
  tft.drawPixel(x + 13, y + 13, color_1);
  tft.drawPixel(x + 14, y + 13, color_1);
  tft.drawPixel(x + 15, y + 13, color_1);
  tft.drawPixel(x + 16, y + 13, color_2);
  tft.drawPixel(x + 17, y + 13, color_2);

  tft.drawPixel(x + 3, y + 14, color_2);
  tft.drawPixel(x + 4, y + 14, color_2);
  tft.drawPixel(x + 5, y + 14, color_2);
  tft.drawPixel(x + 6, y + 14, color_2);
  tft.drawPixel(x + 7, y + 14, color_2);
  tft.drawPixel(x + 8, y + 14, color_2);
  tft.drawPixel(x + 9, y + 14, color_2);
  tft.drawPixel(x + 10, y + 14, color_2);
  tft.drawPixel(x + 11, y + 14, color_2);
  tft.drawPixel(x + 12, y + 14, color_2);
  tft.drawPixel(x + 13, y + 14, color_2);
  tft.drawPixel(x + 14, y + 14, color_2);
  tft.drawPixel(x + 15, y + 14, color_2);
  tft.drawPixel(x + 16, y + 14, color_2);

  tft.drawPixel(x + 2, y + 15, color_2);
  tft.drawPixel(x + 3, y + 15, color_2);
  tft.drawPixel(x + 4, y + 15, color_1);
  tft.drawPixel(x + 5, y + 15, color_1);
  tft.drawPixel(x + 6, y + 15, color_1);
  tft.drawPixel(x + 7, y + 15, color_1);
  tft.drawPixel(x + 8, y + 15, color_1);
  tft.drawPixel(x + 9, y + 15, color_1);
  tft.drawPixel(x + 10, y + 15, color_1);
  tft.drawPixel(x + 11, y + 15, color_1);
  tft.drawPixel(x + 12, y + 15, color_1);
  tft.drawPixel(x + 13, y + 15, color_1);
  tft.drawPixel(x + 14, y + 15, color_1);
  tft.drawPixel(x + 15, y + 15, color_1);
  tft.drawPixel(x + 16, y + 15, color_2);
  tft.drawPixel(x + 17, y + 15, color_2);

  tft.drawPixel(x + 2, y + 16, color_2);
  tft.drawPixel(x + 3, y + 16, color_1);
  tft.drawPixel(x + 4, y + 16, color_1);
  tft.drawPixel(x + 5, y + 16, color_2);
  tft.drawPixel(x + 6, y + 16, color_2);
  tft.drawPixel(x + 7, y + 16, color_2);
  tft.drawPixel(x + 8, y + 16, color_2);
  tft.drawPixel(x + 9, y + 16, color_2);
  tft.drawPixel(x + 10, y + 16, color_2);
  tft.drawPixel(x + 11, y + 16, color_2);
  tft.drawPixel(x + 12, y + 16, color_2);
  tft.drawPixel(x + 13, y + 16, color_2);
  tft.drawPixel(x + 14, y + 16, color_2);
  tft.drawPixel(x + 15, y + 16, color_1);
  tft.drawPixel(x + 16, y + 16, color_1);
  tft.drawPixel(x + 17, y + 16, color_2);

  tft.drawPixel(x + 2, y + 17, color_2);
  tft.drawPixel(x + 3, y + 17, color_2);
  tft.drawPixel(x + 4, y + 17, color_2);
  tft.drawPixel(x + 5, y + 17, color_1);
  tft.drawPixel(x + 6, y + 17, color_1);
  tft.drawPixel(x + 7, y + 17, color_1);
  tft.drawPixel(x + 8, y + 17, color_1);
  tft.drawPixel(x + 9, y + 17, color_1);
  tft.drawPixel(x + 10, y + 17, color_1);
  tft.drawPixel(x + 11, y + 17, color_1);
  tft.drawPixel(x + 12, y + 17, color_1);
  tft.drawPixel(x + 13, y + 17, color_1);
  tft.drawPixel(x + 14, y + 17, color_1);
  tft.drawPixel(x + 15, y + 17, color_2);
  tft.drawPixel(x + 16, y + 17, color_2);
  tft.drawPixel(x + 17, y + 17, color_2);

  tft.drawPixel(x + 2, y + 18, color_2);
  tft.drawPixel(x + 3, y + 18, color_1);
  tft.drawPixel(x + 4, y + 18, color_1);
  tft.drawPixel(x + 5, y + 18, color_1);
  tft.drawPixel(x + 6, y + 18, color_1);
  tft.drawPixel(x + 7, y + 18, color_1);
  tft.drawPixel(x + 8, y + 18, color_1);
  tft.drawPixel(x + 9, y + 18, color_1);
  tft.drawPixel(x + 10, y + 18, color_1);
  tft.drawPixel(x + 11, y + 18, color_1);
  tft.drawPixel(x + 12, y + 18, color_1);
  tft.drawPixel(x + 13, y + 18, color_1);
  tft.drawPixel(x + 14, y + 18, color_1);
  tft.drawPixel(x + 15, y + 18, color_1);
  tft.drawPixel(x + 16, y + 18, color_1);
  tft.drawPixel(x + 17, y + 18, color_2);

  tft.drawPixel(x + 2, y + 19, color_2);
  tft.drawPixel(x + 3, y + 19, color_2);
  tft.drawPixel(x + 4, y + 19, color_2);
  tft.drawPixel(x + 5, y + 19, color_2);
  tft.drawPixel(x + 6, y + 19, color_2);
  tft.drawPixel(x + 7, y + 19, color_2);
  tft.drawPixel(x + 8, y + 19, color_2);
  tft.drawPixel(x + 9, y + 19, color_2);
  tft.drawPixel(x + 10, y + 19, color_2);
  tft.drawPixel(x + 11, y + 19, color_2);
  tft.drawPixel(x + 12, y + 19, color_2);
  tft.drawPixel(x + 13, y + 19, color_2);
  tft.drawPixel(x + 14, y + 19, color_2);
  tft.drawPixel(x + 15, y + 19, color_2);
  tft.drawPixel(x + 16, y + 19, color_2);
  tft.drawPixel(x + 17, y + 19, color_2);
}

void drawRook(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x + 2, y + 0, color_2);
  tft.drawPixel(x + 3, y + 0, color_2);
  tft.drawPixel(x + 4, y + 0, color_2);
  tft.drawPixel(x + 5, y + 0, color_2);
  tft.drawPixel(x + 8, y + 0, color_2);
  tft.drawPixel(x + 9, y + 0, color_2);
  tft.drawPixel(x + 10, y + 0, color_2);
  tft.drawPixel(x + 11, y + 0, color_2);
  tft.drawPixel(x + 14, y + 0, color_2);
  tft.drawPixel(x + 15, y + 0, color_2);
  tft.drawPixel(x + 16, y + 0, color_2);
  tft.drawPixel(x + 17, y + 0, color_2);

  tft.drawPixel(x + 2, y + 1, color_2);
  tft.drawPixel(x + 3, y + 1, color_1);
  tft.drawPixel(x + 4, y + 1, color_1);
  tft.drawPixel(x + 5, y + 1, color_2);
  tft.drawPixel(x + 8, y + 1, color_2);
  tft.drawPixel(x + 9, y + 1, color_1);
  tft.drawPixel(x + 10, y + 1, color_1);
  tft.drawPixel(x + 11, y + 1, color_2);
  tft.drawPixel(x + 14, y + 1, color_2);
  tft.drawPixel(x + 15, y + 1, color_1);
  tft.drawPixel(x + 16, y + 1, color_1);
  tft.drawPixel(x + 17, y + 1, color_2);

  tft.drawPixel(x + 2, y + 2, color_2);
  tft.drawPixel(x + 3, y + 2, color_1);
  tft.drawPixel(x + 4, y + 2, color_1);
  tft.drawPixel(x + 5, y + 2, color_2);
  tft.drawPixel(x + 6, y + 2, color_2);
  tft.drawPixel(x + 7, y + 2, color_2);
  tft.drawPixel(x + 8, y + 2, color_2);
  tft.drawPixel(x + 9, y + 2, color_1);
  tft.drawPixel(x + 10, y + 2, color_1);
  tft.drawPixel(x + 11, y + 2, color_2);
  tft.drawPixel(x + 12, y + 2, color_2);
  tft.drawPixel(x + 13, y + 2, color_2);
  tft.drawPixel(x + 14, y + 2, color_2);
  tft.drawPixel(x + 15, y + 2, color_1);
  tft.drawPixel(x + 16, y + 2, color_1);
  tft.drawPixel(x + 17, y + 2, color_2);

  tft.drawPixel(x + 2, y + 3, color_2);
  tft.drawPixel(x + 3, y + 3, color_1);
  tft.drawPixel(x + 4, y + 3, color_1);
  tft.drawPixel(x + 5, y + 3, color_1);
  tft.drawPixel(x + 6, y + 3, color_1);
  tft.drawPixel(x + 7, y + 3, color_1);
  tft.drawPixel(x + 8, y + 3, color_1);
  tft.drawPixel(x + 9, y + 3, color_1);
  tft.drawPixel(x + 10, y + 3, color_1);
  tft.drawPixel(x + 11, y + 3, color_1);
  tft.drawPixel(x + 12, y + 3, color_1);
  tft.drawPixel(x + 13, y + 3, color_1);
  tft.drawPixel(x + 14, y + 3, color_1);
  tft.drawPixel(x + 15, y + 3, color_1);
  tft.drawPixel(x + 16, y + 3, color_1);
  tft.drawPixel(x + 17, y + 3, color_2);

  tft.drawPixel(x + 2, y + 4, color_2);
  tft.drawPixel(x + 3, y + 4, color_2);
  tft.drawPixel(x + 4, y + 4, color_1);
  tft.drawPixel(x + 5, y + 4, color_1);
  tft.drawPixel(x + 6, y + 4, color_1);
  tft.drawPixel(x + 7, y + 4, color_1);
  tft.drawPixel(x + 8, y + 4, color_1);
  tft.drawPixel(x + 9, y + 4, color_1);
  tft.drawPixel(x + 10, y + 4, color_1);
  tft.drawPixel(x + 11, y + 4, color_1);
  tft.drawPixel(x + 12, y + 4, color_1);
  tft.drawPixel(x + 13, y + 4, color_1);
  tft.drawPixel(x + 14, y + 4, color_1);
  tft.drawPixel(x + 15, y + 4, color_1);
  tft.drawPixel(x + 16, y + 4, color_2);
  tft.drawPixel(x + 17, y + 4, color_2);

  tft.drawPixel(x + 3, y + 5, color_2);
  tft.drawPixel(x + 4, y + 5, color_2);
  tft.drawPixel(x + 5, y + 5, color_1);
  tft.drawPixel(x + 6, y + 5, color_1);
  tft.drawPixel(x + 7, y + 5, color_1);
  tft.drawPixel(x + 8, y + 5, color_1);
  tft.drawPixel(x + 9, y + 5, color_1);
  tft.drawPixel(x + 10, y + 5, color_1);
  tft.drawPixel(x + 11, y + 5, color_1);
  tft.drawPixel(x + 12, y + 5, color_1);
  tft.drawPixel(x + 13, y + 5, color_1);
  tft.drawPixel(x + 14, y + 5, color_1);
  tft.drawPixel(x + 15, y + 5, color_2);
  tft.drawPixel(x + 16, y + 5, color_2);

  tft.drawPixel(x + 4, y + 6, color_2);
  tft.drawPixel(x + 5, y + 6, color_2);
  tft.drawPixel(x + 6, y + 6, color_2);
  tft.drawPixel(x + 7, y + 6, color_2);
  tft.drawPixel(x + 8, y + 6, color_2);
  tft.drawPixel(x + 9, y + 6, color_2);
  tft.drawPixel(x + 10, y + 6, color_2);
  tft.drawPixel(x + 11, y + 6, color_2);
  tft.drawPixel(x + 12, y + 6, color_2);
  tft.drawPixel(x + 13, y + 6, color_2);
  tft.drawPixel(x + 14, y + 6, color_2);
  tft.drawPixel(x + 15, y + 6, color_2);

  tft.drawPixel(x + 5, y + 7, color_2);
  tft.drawPixel(x + 6, y + 7, color_1);
  tft.drawPixel(x + 7, y + 7, color_1);
  tft.drawPixel(x + 8, y + 7, color_1);
  tft.drawPixel(x + 9, y + 7, color_1);
  tft.drawPixel(x + 10, y + 7, color_1);
  tft.drawPixel(x + 11, y + 7, color_1);
  tft.drawPixel(x + 12, y + 7, color_1);
  tft.drawPixel(x + 13, y + 7, color_1);
  tft.drawPixel(x + 14, y + 7, color_2);

  tft.drawPixel(x + 5, y + 8, color_2);
  tft.drawPixel(x + 6, y + 8, color_1);
  tft.drawPixel(x + 7, y + 8, color_1);
  tft.drawPixel(x + 8, y + 8, color_1);
  tft.drawPixel(x + 9, y + 8, color_1);
  tft.drawPixel(x + 10, y + 8, color_1);
  tft.drawPixel(x + 11, y + 8, color_1);
  tft.drawPixel(x + 12, y + 8, color_1);
  tft.drawPixel(x + 13, y + 8, color_1);
  tft.drawPixel(x + 14, y + 8, color_2);

  tft.drawPixel(x + 5, y + 9, color_2);
  tft.drawPixel(x + 6, y + 9, color_1);
  tft.drawPixel(x + 7, y + 9, color_1);
  tft.drawPixel(x + 8, y + 9, color_1);
  tft.drawPixel(x + 9, y + 9, color_1);
  tft.drawPixel(x + 10, y + 9, color_1);
  tft.drawPixel(x + 11, y + 9, color_1);
  tft.drawPixel(x + 12, y + 9, color_1);
  tft.drawPixel(x + 13, y + 9, color_1);
  tft.drawPixel(x + 14, y + 9, color_2);

  tft.drawPixel(x + 5, y + 10, color_2);
  tft.drawPixel(x + 6, y + 10, color_1);
  tft.drawPixel(x + 7, y + 10, color_1);
  tft.drawPixel(x + 8, y + 10, color_1);
  tft.drawPixel(x + 9, y + 10, color_1);
  tft.drawPixel(x + 10, y + 10, color_1);
  tft.drawPixel(x + 11, y + 10, color_1);
  tft.drawPixel(x + 12, y + 10, color_1);
  tft.drawPixel(x + 13, y + 10, color_1);
  tft.drawPixel(x + 14, y + 10, color_2);

  tft.drawPixel(x + 5, y + 11, color_2);
  tft.drawPixel(x + 6, y + 11, color_1);
  tft.drawPixel(x + 7, y + 11, color_1);
  tft.drawPixel(x + 8, y + 11, color_1);
  tft.drawPixel(x + 9, y + 11, color_1);
  tft.drawPixel(x + 10, y + 11, color_1);
  tft.drawPixel(x + 11, y + 11, color_1);
  tft.drawPixel(x + 12, y + 11, color_1);
  tft.drawPixel(x + 13, y + 11, color_1);
  tft.drawPixel(x + 14, y + 11, color_2);

  tft.drawPixel(x + 5, y + 12, color_2);
  tft.drawPixel(x + 6, y + 12, color_1);
  tft.drawPixel(x + 7, y + 12, color_1);
  tft.drawPixel(x + 8, y + 12, color_1);
  tft.drawPixel(x + 9, y + 12, color_1);
  tft.drawPixel(x + 10, y + 12, color_1);
  tft.drawPixel(x + 11, y + 12, color_1);
  tft.drawPixel(x + 12, y + 12, color_1);
  tft.drawPixel(x + 13, y + 12, color_1);
  tft.drawPixel(x + 14, y + 12, color_2);

  tft.drawPixel(x + 5, y + 13, color_2);
  tft.drawPixel(x + 6, y + 13, color_1);
  tft.drawPixel(x + 7, y + 13, color_1);
  tft.drawPixel(x + 8, y + 13, color_1);
  tft.drawPixel(x + 9, y + 13, color_1);
  tft.drawPixel(x + 10, y + 13, color_1);
  tft.drawPixel(x + 11, y + 13, color_1);
  tft.drawPixel(x + 12, y + 13, color_1);
  tft.drawPixel(x + 13, y + 13, color_1);
  tft.drawPixel(x + 14, y + 13, color_2);

  tft.drawPixel(x + 4, y + 14, color_2);
  tft.drawPixel(x + 5, y + 14, color_2);
  tft.drawPixel(x + 6, y + 14, color_2);
  tft.drawPixel(x + 7, y + 14, color_2);
  tft.drawPixel(x + 8, y + 14, color_2);
  tft.drawPixel(x + 9, y + 14, color_2);
  tft.drawPixel(x + 10, y + 14, color_2);
  tft.drawPixel(x + 11, y + 14, color_2);
  tft.drawPixel(x + 12, y + 14, color_2);
  tft.drawPixel(x + 13, y + 14, color_2);
  tft.drawPixel(x + 14, y + 14, color_2);
  tft.drawPixel(x + 15, y + 14, color_2);

  tft.drawPixel(x + 3, y + 15, color_2);
  tft.drawPixel(x + 4, y + 15, color_2);
  tft.drawPixel(x + 5, y + 15, color_1);
  tft.drawPixel(x + 6, y + 15, color_1);
  tft.drawPixel(x + 7, y + 15, color_1);
  tft.drawPixel(x + 8, y + 15, color_1);
  tft.drawPixel(x + 9, y + 15, color_1);
  tft.drawPixel(x + 10, y + 15, color_1);
  tft.drawPixel(x + 11, y + 15, color_1);
  tft.drawPixel(x + 12, y + 15, color_1);
  tft.drawPixel(x + 13, y + 15, color_1);
  tft.drawPixel(x + 14, y + 15, color_1);
  tft.drawPixel(x + 15, y + 15, color_2);
  tft.drawPixel(x + 16, y + 15, color_2);

  tft.drawPixel(x + 2, y + 16, color_2);
  tft.drawPixel(x + 3, y + 16, color_2);
  tft.drawPixel(x + 4, y + 16, color_1);
  tft.drawPixel(x + 5, y + 16, color_1);
  tft.drawPixel(x + 6, y + 16, color_1);
  tft.drawPixel(x + 7, y + 16, color_1);
  tft.drawPixel(x + 8, y + 16, color_1);
  tft.drawPixel(x + 9, y + 16, color_1);
  tft.drawPixel(x + 10, y + 16, color_1);
  tft.drawPixel(x + 11, y + 16, color_1);
  tft.drawPixel(x + 12, y + 16, color_1);
  tft.drawPixel(x + 13, y + 16, color_1);
  tft.drawPixel(x + 14, y + 16, color_1);
  tft.drawPixel(x + 15, y + 16, color_1);
  tft.drawPixel(x + 16, y + 16, color_2);
  tft.drawPixel(x + 17, y + 16, color_2);

  tft.drawPixel(x + 1, y + 17, color_2);
  tft.drawPixel(x + 2, y + 17, color_2);
  tft.drawPixel(x + 3, y + 17, color_2);
  tft.drawPixel(x + 4, y + 17, color_2);
  tft.drawPixel(x + 5, y + 17, color_2);
  tft.drawPixel(x + 6, y + 17, color_2);
  tft.drawPixel(x + 7, y + 17, color_2);
  tft.drawPixel(x + 8, y + 17, color_2);
  tft.drawPixel(x + 9, y + 17, color_2);
  tft.drawPixel(x + 10, y + 17, color_2);
  tft.drawPixel(x + 11, y + 17, color_2);
  tft.drawPixel(x + 12, y + 17, color_2);
  tft.drawPixel(x + 13, y + 17, color_2);
  tft.drawPixel(x + 14, y + 17, color_2);
  tft.drawPixel(x + 15, y + 17, color_2);
  tft.drawPixel(x + 16, y + 17, color_2);
  tft.drawPixel(x + 17, y + 17, color_2);
  tft.drawPixel(x + 18, y + 17, color_2);

  tft.drawPixel(x + 1, y + 18, color_2);
  tft.drawPixel(x + 2, y + 18, color_1);
  tft.drawPixel(x + 3, y + 18, color_1);
  tft.drawPixel(x + 4, y + 18, color_1);
  tft.drawPixel(x + 5, y + 18, color_1);
  tft.drawPixel(x + 6, y + 18, color_1);
  tft.drawPixel(x + 7, y + 18, color_1);
  tft.drawPixel(x + 8, y + 18, color_1);
  tft.drawPixel(x + 9, y + 18, color_1);
  tft.drawPixel(x + 10, y + 18, color_1);
  tft.drawPixel(x + 11, y + 18, color_1);
  tft.drawPixel(x + 12, y + 18, color_1);
  tft.drawPixel(x + 13, y + 18, color_1);
  tft.drawPixel(x + 14, y + 18, color_1);
  tft.drawPixel(x + 15, y + 18, color_1);
  tft.drawPixel(x + 16, y + 18, color_1);
  tft.drawPixel(x + 17, y + 18, color_1);
  tft.drawPixel(x + 18, y + 18, color_2);

  tft.drawPixel(x + 1, y + 19, color_2);
  tft.drawPixel(x + 2, y + 19, color_2);
  tft.drawPixel(x + 3, y + 19, color_2);
  tft.drawPixel(x + 4, y + 19, color_2);
  tft.drawPixel(x + 5, y + 19, color_2);
  tft.drawPixel(x + 6, y + 19, color_2);
  tft.drawPixel(x + 7, y + 19, color_2);
  tft.drawPixel(x + 8, y + 19, color_2);
  tft.drawPixel(x + 9, y + 19, color_2);
  tft.drawPixel(x + 10, y + 19, color_2);
  tft.drawPixel(x + 11, y + 19, color_2);
  tft.drawPixel(x + 12, y + 19, color_2);
  tft.drawPixel(x + 13, y + 19, color_2);
  tft.drawPixel(x + 14, y + 19, color_2);
  tft.drawPixel(x + 15, y + 19, color_2);
  tft.drawPixel(x + 16, y + 19, color_2);
  tft.drawPixel(x + 17, y + 19, color_2);
  tft.drawPixel(x + 18, y + 19, color_2);
}

void drawBishop(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x + 9, y + 0, color_2);
  tft.drawPixel(x + 10, y + 0, color_2);

  tft.drawPixel(x + 8, y + 1, color_2);
  tft.drawPixel(x + 9, y + 1, color_1);
  tft.drawPixel(x + 10, y + 1, color_1);
  tft.drawPixel(x + 11, y + 1, color_2);

  tft.drawPixel(x + 9, y + 2, color_2);
  tft.drawPixel(x + 10, y + 2, color_2);

  tft.drawPixel(x + 7, y + 3, color_2);
  tft.drawPixel(x + 8, y + 3, color_2);
  tft.drawPixel(x + 9, y + 3, color_1);
  tft.drawPixel(x + 10, y + 3, color_1);
  tft.drawPixel(x + 11, y + 3, color_2);
  tft.drawPixel(x + 12, y + 3, color_2);

  tft.drawPixel(x + 7, y + 4, color_2);
  tft.drawPixel(x + 8, y + 4, color_1);
  tft.drawPixel(x + 9, y + 4, color_1);
  tft.drawPixel(x + 10, y + 4, color_2);
  tft.drawPixel(x + 11, y + 4, color_2);
  tft.drawPixel(x + 12, y + 4, color_2);

  tft.drawPixel(x + 6, y + 5, color_2);
  tft.drawPixel(x + 7, y + 5, color_2);
  tft.drawPixel(x + 8, y + 5, color_1);
  tft.drawPixel(x + 9, y + 5, color_1);
  tft.drawPixel(x + 10, y + 5, color_2);
  tft.drawPixel(x + 11, y + 5, color_2);
  tft.drawPixel(x + 12, y + 5, color_2);
  tft.drawPixel(x + 13, y + 5, color_2);

  tft.drawPixel(x + 6, y + 6, color_2);
  tft.drawPixel(x + 7, y + 6, color_1);
  tft.drawPixel(x + 8, y + 6, color_1);
  tft.drawPixel(x + 9, y + 6, color_1);
  tft.drawPixel(x + 10, y + 6, color_2);
  tft.drawPixel(x + 11, y + 6, color_2);
  tft.drawPixel(x + 12, y + 6, color_1);
  tft.drawPixel(x + 13, y + 6, color_2);

  tft.drawPixel(x + 6, y + 7, color_2);
  tft.drawPixel(x + 7, y + 7, color_1);
  tft.drawPixel(x + 8, y + 7, color_1);
  tft.drawPixel(x + 9, y + 7, color_2);
  tft.drawPixel(x + 10, y + 7, color_2);
  tft.drawPixel(x + 11, y + 7, color_1);
  tft.drawPixel(x + 12, y + 7, color_1);
  tft.drawPixel(x + 13, y + 7, color_2);

  tft.drawPixel(x + 6, y + 8, color_2);
  tft.drawPixel(x + 7, y + 8, color_1);
  tft.drawPixel(x + 8, y + 8, color_1);
  tft.drawPixel(x + 9, y + 8, color_2);
  tft.drawPixel(x + 10, y + 8, color_2);
  tft.drawPixel(x + 11, y + 8, color_1);
  tft.drawPixel(x + 12, y + 8, color_1);
  tft.drawPixel(x + 13, y + 8, color_2);

  tft.drawPixel(x + 6, y + 9, color_2);
  tft.drawPixel(x + 7, y + 9, color_1);
  tft.drawPixel(x + 8, y + 9, color_1);
  tft.drawPixel(x + 9, y + 9, color_1);
  tft.drawPixel(x + 10, y + 9, color_1);
  tft.drawPixel(x + 11, y + 9, color_1);
  tft.drawPixel(x + 12, y + 9, color_1);
  tft.drawPixel(x + 13, y + 9, color_2);

  tft.drawPixel(x + 6, y + 10, color_2);
  tft.drawPixel(x + 7, y + 10, color_1);
  tft.drawPixel(x + 8, y + 10, color_1);
  tft.drawPixel(x + 9, y + 10, color_1);
  tft.drawPixel(x + 10, y + 10, color_1);
  tft.drawPixel(x + 11, y + 10, color_1);
  tft.drawPixel(x + 12, y + 10, color_1);
  tft.drawPixel(x + 13, y + 10, color_2);

  tft.drawPixel(x + 6, y + 11, color_2);
  tft.drawPixel(x + 7, y + 11, color_1);
  tft.drawPixel(x + 8, y + 11, color_1);
  tft.drawPixel(x + 9, y + 11, color_1);
  tft.drawPixel(x + 10, y + 11, color_1);
  tft.drawPixel(x + 11, y + 11, color_1);
  tft.drawPixel(x + 12, y + 11, color_1);
  tft.drawPixel(x + 13, y + 11, color_2);

  tft.drawPixel(x + 6, y + 12, color_2);
  tft.drawPixel(x + 7, y + 12, color_2);
  tft.drawPixel(x + 8, y + 12, color_1);
  tft.drawPixel(x + 9, y + 12, color_1);
  tft.drawPixel(x + 10, y + 12, color_1);
  tft.drawPixel(x + 11, y + 12, color_1);
  tft.drawPixel(x + 12, y + 12, color_2);
  tft.drawPixel(x + 13, y + 12, color_2);

  tft.drawPixel(x + 7, y + 13, color_2);
  tft.drawPixel(x + 8, y + 13, color_2);
  tft.drawPixel(x + 9, y + 13, color_1);
  tft.drawPixel(x + 10, y + 13, color_1);
  tft.drawPixel(x + 11, y + 13, color_2);
  tft.drawPixel(x + 12, y + 13, color_2);

  tft.drawPixel(x + 8, y + 14, color_2);
  tft.drawPixel(x + 9, y + 14, color_1);
  tft.drawPixel(x + 10, y + 14, color_1);
  tft.drawPixel(x + 11, y + 14, color_2);

  tft.drawPixel(x + 7, y + 15, color_2);
  tft.drawPixel(x + 8, y + 15, color_2);
  tft.drawPixel(x + 9, y + 15, color_1);
  tft.drawPixel(x + 10, y + 15, color_1);
  tft.drawPixel(x + 11, y + 15, color_2);
  tft.drawPixel(x + 12, y + 15, color_2);

  tft.drawPixel(x + 6, y + 16, color_2);
  tft.drawPixel(x + 7, y + 16, color_2);
  tft.drawPixel(x + 8, y + 16, color_1);
  tft.drawPixel(x + 9, y + 16, color_1);
  tft.drawPixel(x + 10, y + 16, color_1);
  tft.drawPixel(x + 11, y + 16, color_1);
  tft.drawPixel(x + 12, y + 16, color_2);
  tft.drawPixel(x + 13, y + 16, color_2);

  tft.drawPixel(x + 3, y + 17, color_2);
  tft.drawPixel(x + 4, y + 17, color_2);
  tft.drawPixel(x + 5, y + 17, color_2);
  tft.drawPixel(x + 6, y + 17, color_2);
  tft.drawPixel(x + 7, y + 17, color_2);
  tft.drawPixel(x + 8, y + 17, color_2);
  tft.drawPixel(x + 9, y + 17, color_2);
  tft.drawPixel(x + 10, y + 17, color_2);
  tft.drawPixel(x + 11, y + 17, color_2);
  tft.drawPixel(x + 12, y + 17, color_2);
  tft.drawPixel(x + 13, y + 17, color_2);
  tft.drawPixel(x + 14, y + 17, color_2);
  tft.drawPixel(x + 15, y + 17, color_2);
  tft.drawPixel(x + 16, y + 17, color_2);

  tft.drawPixel(x + 3, y + 18, color_2);
  tft.drawPixel(x + 4, y + 18, color_1);
  tft.drawPixel(x + 5, y + 18, color_1);
  tft.drawPixel(x + 6, y + 18, color_1);
  tft.drawPixel(x + 7, y + 18, color_1);
  tft.drawPixel(x + 8, y + 18, color_1);
  tft.drawPixel(x + 9, y + 18, color_1);
  tft.drawPixel(x + 10, y + 18, color_1);
  tft.drawPixel(x + 11, y + 18, color_1);
  tft.drawPixel(x + 12, y + 18, color_1);
  tft.drawPixel(x + 13, y + 18, color_1);
  tft.drawPixel(x + 14, y + 18, color_1);
  tft.drawPixel(x + 15, y + 18, color_1);
  tft.drawPixel(x + 16, y + 18, color_2);

  tft.drawPixel(x + 3, y + 19, color_2);
  tft.drawPixel(x + 4, y + 19, color_2);
  tft.drawPixel(x + 5, y + 19, color_2);
  tft.drawPixel(x + 6, y + 19, color_2);
  tft.drawPixel(x + 7, y + 19, color_2);
  tft.drawPixel(x + 8, y + 19, color_2);
  tft.drawPixel(x + 9, y + 19, color_2);
  tft.drawPixel(x + 10, y + 19, color_2);
  tft.drawPixel(x + 11, y + 19, color_2);
  tft.drawPixel(x + 12, y + 19, color_2);
  tft.drawPixel(x + 13, y + 19, color_2);
  tft.drawPixel(x + 14, y + 19, color_2);
  tft.drawPixel(x + 15, y + 19, color_2);
  tft.drawPixel(x + 16, y + 19, color_2);
}



void drawKnight2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x - 7, y, color_2);
  tft.drawPixel(x - 9, y, color_2);

  tft.drawPixel(x - 6, y - 1, color_2);
  tft.drawPixel(x - 7, y - 1, color_1);
  tft.drawPixel(x - 8, y - 1, color_2);
  tft.drawPixel(x - 9, y - 1, color_1);
  tft.drawPixel(x - 10, y - 1, color_2);

  tft.drawPixel(x - 5, y - 2, color_2);
  tft.drawPixel(x - 6, y - 2, color_1);
  tft.drawPixel(x - 7, y - 2, color_1);
  tft.drawPixel(x - 8, y - 2, color_1);
  tft.drawPixel(x - 9, y - 2, color_1);
  tft.drawPixel(x - 10, y - 2, color_1);
  tft.drawPixel(x - 11, y - 2, color_2);

  tft.drawPixel(x - 4, y - 3, color_2);
  tft.drawPixel(x - 5, y - 3, color_1);
  tft.drawPixel(x - 6, y - 3, color_1);
  tft.drawPixel(x - 7, y - 3, color_1);
  tft.drawPixel(x - 8, y - 3, color_1);
  tft.drawPixel(x - 9, y - 3, color_1);
  tft.drawPixel(x - 10, y - 3, color_1);
  tft.drawPixel(x - 11, y - 3, color_1);
  tft.drawPixel(x - 12, y - 3, color_2);

  tft.drawPixel(x - 4, y - 4, color_2);
  tft.drawPixel(x - 5, y - 4, color_1);
  tft.drawPixel(x - 6, y - 4, color_1);
  tft.drawPixel(x - 7, y - 4, color_1);
  tft.drawPixel(x - 8, y - 4, color_1);
  tft.drawPixel(x - 9, y - 4, color_1);
  tft.drawPixel(x - 10, y - 4, color_1);
  tft.drawPixel(x - 11, y - 4, color_1);
  tft.drawPixel(x - 12, y - 4, color_1);
  tft.drawPixel(x - 13, y - 4, color_2);

  tft.drawPixel(x - 3, y - 5, color_2);
  tft.drawPixel(x - 4, y - 5, color_1);
  tft.drawPixel(x - 5, y - 5, color_1);
  tft.drawPixel(x - 6, y - 5, color_1);
  tft.drawPixel(x - 7, y - 5, color_1);
  tft.drawPixel(x - 8, y - 5, color_1);
  tft.drawPixel(x - 9, y - 5, color_1);
  tft.drawPixel(x - 10, y - 5, color_1);
  tft.drawPixel(x - 11, y - 5, color_1);
  tft.drawPixel(x - 12, y - 5, color_2);
  tft.drawPixel(x - 13, y - 5, color_1);
  tft.drawPixel(x - 14, y - 5, color_2);

  tft.drawPixel(x - 3, y - 6, color_2);
  tft.drawPixel(x - 4, y - 6, color_1);
  tft.drawPixel(x - 5, y - 6, color_2);
  tft.drawPixel(x - 6, y - 6, color_1);
  tft.drawPixel(x - 7, y - 6, color_1);
  tft.drawPixel(x - 8, y - 6, color_1);
  tft.drawPixel(x - 9, y - 6, color_1);
  tft.drawPixel(x - 10, y - 6, color_1);
  tft.drawPixel(x - 11, y - 6, color_1);
  tft.drawPixel(x - 12, y - 6, color_1);
  tft.drawPixel(x - 13, y - 6, color_2);
  tft.drawPixel(x - 14, y - 6, color_1);
  tft.drawPixel(x - 15, y - 6, color_2);

  tft.drawPixel(x - 2, y - 7, color_2);
  tft.drawPixel(x - 3, y - 7, color_1);
  tft.drawPixel(x - 4, y - 7, color_1);
  tft.drawPixel(x - 5, y - 7, color_1);
  tft.drawPixel(x - 6, y - 7, color_1);
  tft.drawPixel(x - 7, y - 7, color_1);
  tft.drawPixel(x - 8, y - 7, color_1);
  tft.drawPixel(x - 9, y - 7, color_1);
  tft.drawPixel(x - 10, y - 7, color_1);
  tft.drawPixel(x - 11, y - 7, color_1);
  tft.drawPixel(x - 12, y - 7, color_1);
  tft.drawPixel(x - 13, y - 7, color_2);
  tft.drawPixel(x - 14, y - 7, color_1);
  tft.drawPixel(x - 15, y - 7, color_2);

  tft.drawPixel(x - 1, y - 8, color_2);
  tft.drawPixel(x - 2, y - 8, color_2);
  tft.drawPixel(x - 3, y - 8, color_1);
  tft.drawPixel(x - 4, y - 8, color_1);
  tft.drawPixel(x - 5, y - 8, color_1);
  tft.drawPixel(x - 6, y - 8, color_1);
  tft.drawPixel(x - 7, y - 8, color_1);
  tft.drawPixel(x - 8, y - 8, color_1);
  tft.drawPixel(x - 9, y - 8, color_2);
  tft.drawPixel(x - 10, y - 8, color_1);
  tft.drawPixel(x - 11, y - 8, color_1);
  tft.drawPixel(x - 12, y - 8, color_1);
  tft.drawPixel(x - 13, y - 8, color_1);
  tft.drawPixel(x - 14, y - 8, color_2);
  tft.drawPixel(x - 15, y - 8, color_1);
  tft.drawPixel(x - 16, y - 8, color_2);

  tft.drawPixel(x, y - 9, color_2);
  tft.drawPixel(x - 1, y - 9, color_2);
  tft.drawPixel(x - 2, y - 9, color_1);
  tft.drawPixel(x - 3, y - 9, color_1);
  tft.drawPixel(x - 4, y - 9, color_1);
  tft.drawPixel(x - 5, y - 9, color_1);
  tft.drawPixel(x - 6, y - 9, color_1);
  tft.drawPixel(x - 7, y - 9, color_1);
  tft.drawPixel(x - 8, y - 9, color_2);
  tft.drawPixel(x - 9, y - 9, color_2);
  tft.drawPixel(x - 10, y - 9, color_2);
  tft.drawPixel(x - 11, y - 9, color_1);
  tft.drawPixel(x - 12, y - 9, color_1);
  tft.drawPixel(x - 13, y - 9, color_1);
  tft.drawPixel(x - 14, y - 9, color_2);
  tft.drawPixel(x - 15, y - 9, color_1);
  tft.drawPixel(x - 16, y - 9, color_2);

  tft.drawPixel(x, y - 10, color_2);
  tft.drawPixel(x - 1, y - 10, color_1);
  tft.drawPixel(x - 2, y - 10, color_1);
  tft.drawPixel(x - 3, y - 10, color_1);
  tft.drawPixel(x - 4, y - 10, color_1);
  tft.drawPixel(x - 5, y - 10, color_1);
  tft.drawPixel(x - 6, y - 10, color_2);
  tft.drawPixel(x - 7, y - 10, color_2);
  //tft.drawPixel(x-8, y-10, color_2);
  //tft.drawPixel(x-9, y-10, color_2);
  tft.drawPixel(x - 10, y - 10, color_2);
  tft.drawPixel(x - 11, y - 10, color_1);
  tft.drawPixel(x - 12, y - 10, color_1);
  tft.drawPixel(x - 13, y - 10, color_1);
  tft.drawPixel(x - 14, y - 10, color_1);
  tft.drawPixel(x - 15, y - 10, color_2);
  tft.drawPixel(x - 16, y - 10, color_1);
  tft.drawPixel(x - 17, y - 10, color_2);

  //tft.drawPixel(x, y-11, color_2);
  tft.drawPixel(x - 1, y - 11, color_2);
  tft.drawPixel(x - 2, y - 11, color_1);
  tft.drawPixel(x - 3, y - 11, color_2);
  tft.drawPixel(x - 4, y - 11, color_1);
  tft.drawPixel(x - 5, y - 11, color_2);
  //tft.drawPixel(x-6, y-11, color_2);
  //tft.drawPixel(x-7, y-11, color_2);
  //tft.drawPixel(x-8, y-11, color_2);
  tft.drawPixel(x - 9, y - 11, color_2);
  tft.drawPixel(x - 10, y - 11, color_1);
  tft.drawPixel(x - 11, y - 11, color_1);
  tft.drawPixel(x - 12, y - 11, color_1);
  tft.drawPixel(x - 13, y - 11, color_1);
  tft.drawPixel(x - 14, y - 11, color_1);
  tft.drawPixel(x - 15, y - 11, color_2);
  tft.drawPixel(x - 16, y - 11, color_1);
  tft.drawPixel(x - 17, y - 11, color_2);

  //tft.drawPixel(x, y-12, color_2);
  tft.drawPixel(x - 1, y - 12, color_2);
  tft.drawPixel(x - 2, y - 12, color_2);
  tft.drawPixel(x - 3, y - 12, color_2);
  tft.drawPixel(x - 4, y - 12, color_2);
  //tft.drawPixel(x-5, y-12, color_2);
  //tft.drawPixel(x-6, y-12, color_2);
  //tft.drawPixel(x-7, y-12, color_2);
  tft.drawPixel(x - 8, y - 12, color_2);
  tft.drawPixel(x - 9, y - 12, color_1);
  tft.drawPixel(x - 10, y - 12, color_1);
  tft.drawPixel(x - 11, y - 12, color_1);
  tft.drawPixel(x - 12, y - 12, color_1);
  tft.drawPixel(x - 13, y - 12, color_1);
  tft.drawPixel(x - 14, y - 12, color_1);
  tft.drawPixel(x - 15, y - 12, color_2);
  tft.drawPixel(x - 16, y - 12, color_1);
  tft.drawPixel(x - 17, y - 12, color_2);

  //tft.drawPixel(x, y-13, color_2);
  //tft.drawPixel(x-1, y-13, color_2);
  //tft.drawPixel(x-2, y-13, color_2);
  //tft.drawPixel(x-3, y-13, color_2);
  //tft.drawPixel(x-4, y-13, color_2);
  //tft.drawPixel(x-5, y-13, color_2);
  //tft.drawPixel(x-6, y-13, color_2);
  tft.drawPixel(x - 7, y - 13, color_2);
  tft.drawPixel(x - 8, y - 13, color_1);
  tft.drawPixel(x - 9, y - 13, color_1);
  tft.drawPixel(x - 10, y - 13, color_1);
  tft.drawPixel(x - 11, y - 13, color_1);
  tft.drawPixel(x - 12, y - 13, color_1);
  tft.drawPixel(x - 13, y - 13, color_1);
  tft.drawPixel(x - 14, y - 13, color_1);
  tft.drawPixel(x - 15, y - 13, color_2);
  tft.drawPixel(x - 16, y - 13, color_1);
  tft.drawPixel(x - 17, y - 13, color_2);

  //tft.drawPixel(x, y-14, color_2);
  //tft.drawPixel(x-1, y-14, color_2);
  //tft.drawPixel(x-2, y-14, color_2);
  //tft.drawPixel(x-3, y-14, color_2);
  //tft.drawPixel(x-4, y-14, color_2);
  //tft.drawPixel(x-5, y-14, color_2);
  tft.drawPixel(x - 6, y - 14, color_2);
  tft.drawPixel(x - 7, y - 14, color_1);
  tft.drawPixel(x - 8, y - 14, color_1);
  tft.drawPixel(x - 9, y - 14, color_1);
  tft.drawPixel(x - 10, y - 14, color_1);
  tft.drawPixel(x - 11, y - 14, color_1);
  tft.drawPixel(x - 12, y - 14, color_1);
  tft.drawPixel(x - 13, y - 14, color_1);
  tft.drawPixel(x - 14, y - 14, color_1);
  tft.drawPixel(x - 15, y - 14, color_1);
  tft.drawPixel(x - 16, y - 14, color_1);
  tft.drawPixel(x - 17, y - 14, color_2);

  //tft.drawPixel(x, y-15, color_2);
  //tft.drawPixel(x-1, y-15, color_2);
  //tft.drawPixel(x-2, y-15, color_2);
  //tft.drawPixel(x-3, y-15, color_2);
  //tft.drawPixel(x-4, y-15, color_2);
  tft.drawPixel(x - 5, y - 15, color_2);
  tft.drawPixel(x - 6, y - 15, color_1);
  tft.drawPixel(x - 7, y - 15, color_1);
  tft.drawPixel(x - 8, y - 15, color_1);
  tft.drawPixel(x - 9, y - 15, color_1);
  tft.drawPixel(x - 10, y - 15, color_1);
  tft.drawPixel(x - 11, y - 15, color_1);
  tft.drawPixel(x - 12, y - 15, color_1);
  tft.drawPixel(x - 13, y - 15, color_1);
  tft.drawPixel(x - 14, y - 15, color_1);
  tft.drawPixel(x - 15, y - 15, color_1);
  tft.drawPixel(x - 16, y - 15, color_1);
  tft.drawPixel(x - 17, y - 15, color_1);
  tft.drawPixel(x - 18, y - 15, color_2);

  //tft.drawPixel(x, y-16, color_2);
  //tft.drawPixel(x-1, y-16, color_2);
  //tft.drawPixel(x-2, y-16, color_2);
  //tft.drawPixel(x-3, y-16, color_2);
  //tft.drawPixel(x-4, y-16, color_2);
  tft.drawPixel(x - 5, y - 16, color_2);
  tft.drawPixel(x - 6, y - 16, color_1);
  tft.drawPixel(x - 7, y - 16, color_1);
  tft.drawPixel(x - 8, y - 16, color_1);
  tft.drawPixel(x - 9, y - 16, color_1);
  tft.drawPixel(x - 10, y - 16, color_1);
  tft.drawPixel(x - 11, y - 16, color_1);
  tft.drawPixel(x - 12, y - 16, color_1);
  tft.drawPixel(x - 13, y - 16, color_1);
  tft.drawPixel(x - 14, y - 16, color_1);
  tft.drawPixel(x - 15, y - 16, color_1);
  tft.drawPixel(x - 16, y - 16, color_1);
  tft.drawPixel(x - 17, y - 16, color_1);
  tft.drawPixel(x - 18, y - 16, color_2);

  //tft.drawPixel(x, y-17, color_2);
  //tft.drawPixel(x-1, y-17, color_2);
  //tft.drawPixel(x-2, y-17, color_2);
  //tft.drawPixel(x-3, y-17, color_2);
  tft.drawPixel(x - 4, y - 17, color_2);
  tft.drawPixel(x - 5, y - 17, color_2);
  tft.drawPixel(x - 6, y - 17, color_2);
  tft.drawPixel(x - 7, y - 17, color_2);
  tft.drawPixel(x - 8, y - 17, color_2);
  tft.drawPixel(x - 9, y - 17, color_2);
  tft.drawPixel(x - 10, y - 17, color_2);
  tft.drawPixel(x - 11, y - 17, color_2);
  tft.drawPixel(x - 12, y - 17, color_2);
  tft.drawPixel(x - 13, y - 17, color_2);
  tft.drawPixel(x - 14, y - 17, color_2);
  tft.drawPixel(x - 15, y - 17, color_2);
  tft.drawPixel(x - 16, y - 17, color_2);
  tft.drawPixel(x - 17, y - 17, color_2);
  tft.drawPixel(x - 18, y - 17, color_2);
  tft.drawPixel(x - 19, y - 17, color_2);

  //tft.drawPixel(x, y-18, color_2);
  //tft.drawPixel(x-1, y-18, color_2);
  //tft.drawPixel(x-2, y-18, color_2);
  //tft.drawPixel(x-3, y-18, color_2);
  tft.drawPixel(x - 4, y - 18, color_2);
  tft.drawPixel(x - 5, y - 18, color_1);
  tft.drawPixel(x - 6, y - 18, color_1);
  tft.drawPixel(x - 7, y - 18, color_1);
  tft.drawPixel(x - 8, y - 18, color_1);
  tft.drawPixel(x - 9, y - 18, color_1);
  tft.drawPixel(x - 10, y - 18, color_1);
  tft.drawPixel(x - 11, y - 18, color_1);
  tft.drawPixel(x - 12, y - 18, color_1);
  tft.drawPixel(x - 13, y - 18, color_1);
  tft.drawPixel(x - 14, y - 18, color_1);
  tft.drawPixel(x - 15, y - 18, color_1);
  tft.drawPixel(x - 16, y - 18, color_1);
  tft.drawPixel(x - 17, y - 18, color_1);
  tft.drawPixel(x - 18, y - 18, color_1);
  tft.drawPixel(x - 19, y - 18, color_2);

  //tft.drawPixel(x, y-19, color_2);
  //tft.drawPixel(x-1, y-19, color_2);
  //tft.drawPixel(x-2, y-19, color_2);
  //tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x - 4, y - 19, color_2);
  tft.drawPixel(x - 5, y - 19, color_2);
  tft.drawPixel(x - 6, y - 19, color_2);
  tft.drawPixel(x - 7, y - 19, color_2);
  tft.drawPixel(x - 8, y - 19, color_2);
  tft.drawPixel(x - 9, y - 19, color_2);
  tft.drawPixel(x - 10, y - 19, color_2);
  tft.drawPixel(x - 11, y - 19, color_2);
  tft.drawPixel(x - 12, y - 19, color_2);
  tft.drawPixel(x - 13, y - 19, color_2);
  tft.drawPixel(x - 14, y - 19, color_2);
  tft.drawPixel(x - 15, y - 19, color_2);
  tft.drawPixel(x - 16, y - 19, color_2);
  tft.drawPixel(x - 17, y - 19, color_2);
  tft.drawPixel(x - 18, y - 19, color_2);
  tft.drawPixel(x - 19, y - 19, color_2);
}

void drawPawn2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x - 8, y - 0, color_2);
  tft.drawPixel(x - 9, y - 0, color_2);
  tft.drawPixel(x - 10, y - 0, color_2);
  tft.drawPixel(x - 11, y - 0, color_2);

  tft.drawPixel(x - 7, y - 1, color_2);
  tft.drawPixel(x - 8, y - 1, color_1);
  tft.drawPixel(x - 9, y - 1, color_1);
  tft.drawPixel(x - 10, y - 1, color_1);
  tft.drawPixel(x - 11, y - 1, color_1);
  tft.drawPixel(x - 12, y - 1, color_2);

  tft.drawPixel(x - 6, y - 2, color_2);
  tft.drawPixel(x - 7, y - 2, color_1);
  tft.drawPixel(x - 8, y - 2, color_1);
  tft.drawPixel(x - 9, y - 2, color_1);
  tft.drawPixel(x - 10, y - 2, color_1);
  tft.drawPixel(x - 11, y - 2, color_1);
  tft.drawPixel(x - 12, y - 2, color_1);
  tft.drawPixel(x - 13, y - 2, color_2);

  tft.drawPixel(x - 6, y - 3, color_2);
  tft.drawPixel(x - 7, y - 3, color_1);
  tft.drawPixel(x - 8, y - 3, color_1);
  tft.drawPixel(x - 9, y - 3, color_1);
  tft.drawPixel(x - 10, y - 3, color_1);
  tft.drawPixel(x - 11, y - 3, color_1);
  tft.drawPixel(x - 12, y - 3, color_1);
  tft.drawPixel(x - 13, y - 3, color_2);

  tft.drawPixel(x - 6, y - 4, color_2);
  tft.drawPixel(x - 7, y - 4, color_1);
  tft.drawPixel(x - 8, y - 4, color_1);
  tft.drawPixel(x - 9, y - 4, color_1);
  tft.drawPixel(x - 10, y - 4, color_1);
  tft.drawPixel(x - 11, y - 4, color_1);
  tft.drawPixel(x - 12, y - 4, color_1);
  tft.drawPixel(x - 13, y - 4, color_2);

  tft.drawPixel(x - 6, y - 5, color_2);
  tft.drawPixel(x - 7, y - 5, color_1);
  tft.drawPixel(x - 8, y - 5, color_1);
  tft.drawPixel(x - 9, y - 5, color_1);
  tft.drawPixel(x - 10, y - 5, color_1);
  tft.drawPixel(x - 11, y - 5, color_1);
  tft.drawPixel(x - 12, y - 5, color_1);
  tft.drawPixel(x - 13, y - 5, color_2);

  tft.drawPixel(x - 5, y - 6, color_2);
  tft.drawPixel(x - 6, y - 6, color_1);
  tft.drawPixel(x - 7, y - 6, color_1);
  tft.drawPixel(x - 8, y - 6, color_1);
  tft.drawPixel(x - 9, y - 6, color_1);
  tft.drawPixel(x - 10, y - 6, color_1);
  tft.drawPixel(x - 11, y - 6, color_1);
  tft.drawPixel(x - 12, y - 6, color_1);
  tft.drawPixel(x - 13, y - 6, color_1);
  tft.drawPixel(x - 14, y - 6, color_2);

  tft.drawPixel(x - 5, y - 7, color_2);
  tft.drawPixel(x - 6, y - 7, color_2);
  tft.drawPixel(x - 7, y - 7, color_2);
  tft.drawPixel(x - 8, y - 7, color_2);
  tft.drawPixel(x - 9, y - 7, color_2);
  tft.drawPixel(x - 10, y - 7, color_2);
  tft.drawPixel(x - 11, y - 7, color_2);
  tft.drawPixel(x - 12, y - 7, color_2);
  tft.drawPixel(x - 13, y - 7, color_2);
  tft.drawPixel(x - 14, y - 7, color_2);

  tft.drawPixel(x - 6, y - 8, color_2);
  tft.drawPixel(x - 7, y - 8, color_2);
  tft.drawPixel(x - 8, y - 8, color_1);
  tft.drawPixel(x - 9, y - 8, color_1);
  tft.drawPixel(x - 10, y - 8, color_1);
  tft.drawPixel(x - 11, y - 8, color_1);
  tft.drawPixel(x - 12, y - 8, color_2);
  tft.drawPixel(x - 13, y - 8, color_2);

  tft.drawPixel(x - 7, y - 9, color_2);
  tft.drawPixel(x - 8, y - 9, color_1);
  tft.drawPixel(x - 9, y - 9, color_1);
  tft.drawPixel(x - 10, y - 9, color_1);
  tft.drawPixel(x - 11, y - 9, color_1);
  tft.drawPixel(x - 12, y - 9, color_2);

  tft.drawPixel(x - 6, y - 10, color_2);
  tft.drawPixel(x - 7, y - 10, color_2);
  tft.drawPixel(x - 8, y - 10, color_1);
  tft.drawPixel(x - 9, y - 10, color_1);
  tft.drawPixel(x - 10, y - 10, color_1);
  tft.drawPixel(x - 11, y - 10, color_1);
  tft.drawPixel(x - 12, y - 10, color_2);
  tft.drawPixel(x - 13, y - 10, color_2);

  tft.drawPixel(x - 6, y - 11, color_2);
  tft.drawPixel(x - 7, y - 11, color_1);
  tft.drawPixel(x - 8, y - 11, color_1);
  tft.drawPixel(x - 9, y - 11, color_1);
  tft.drawPixel(x - 10, y - 11, color_1);
  tft.drawPixel(x - 11, y - 11, color_1);
  tft.drawPixel(x - 12, y - 11, color_1);
  tft.drawPixel(x - 13, y - 11, color_2);

  tft.drawPixel(x - 6, y - 12, color_2);
  tft.drawPixel(x - 7, y - 12, color_1);
  tft.drawPixel(x - 8, y - 12, color_1);
  tft.drawPixel(x - 9, y - 12, color_1);
  tft.drawPixel(x - 10, y - 12, color_1);
  tft.drawPixel(x - 11, y - 12, color_1);
  tft.drawPixel(x - 12, y - 12, color_1);
  tft.drawPixel(x - 13, y - 12, color_2);

  tft.drawPixel(x - 5, y - 13, color_2);
  tft.drawPixel(x - 6, y - 13, color_1);
  tft.drawPixel(x - 7, y - 13, color_1);
  tft.drawPixel(x - 8, y - 13, color_1);
  tft.drawPixel(x - 9, y - 13, color_1);
  tft.drawPixel(x - 10, y - 13, color_1);
  tft.drawPixel(x - 11, y - 13, color_1);
  tft.drawPixel(x - 12, y - 13, color_1);
  tft.drawPixel(x - 13, y - 13, color_1);
  tft.drawPixel(x - 14, y - 13, color_2);

  tft.drawPixel(x - 4, y - 14, color_2);
  tft.drawPixel(x - 5, y - 14, color_2);
  tft.drawPixel(x - 6, y - 14, color_1);
  tft.drawPixel(x - 7, y - 14, color_1);
  tft.drawPixel(x - 8, y - 14, color_1);
  tft.drawPixel(x - 9, y - 14, color_1);
  tft.drawPixel(x - 10, y - 14, color_1);
  tft.drawPixel(x - 11, y - 14, color_1);
  tft.drawPixel(x - 12, y - 14, color_1);
  tft.drawPixel(x - 13, y - 14, color_1);
  tft.drawPixel(x - 14, y - 14, color_2);
  tft.drawPixel(x - 15, y - 14, color_2);

  tft.drawPixel(x - 4, y - 15, color_2);
  tft.drawPixel(x - 5, y - 15, color_1);
  tft.drawPixel(x - 6, y - 15, color_1);
  tft.drawPixel(x - 7, y - 15, color_1);
  tft.drawPixel(x - 8, y - 15, color_1);
  tft.drawPixel(x - 9, y - 15, color_1);
  tft.drawPixel(x - 10, y - 15, color_1);
  tft.drawPixel(x - 11, y - 15, color_1);
  tft.drawPixel(x - 12, y - 15, color_1);
  tft.drawPixel(x - 13, y - 15, color_1);
  tft.drawPixel(x - 14, y - 15, color_1);
  tft.drawPixel(x - 15, y - 15, color_2);

  tft.drawPixel(x - 3, y - 16, color_2);
  tft.drawPixel(x - 4, y - 16, color_1);
  tft.drawPixel(x - 5, y - 16, color_1);
  tft.drawPixel(x - 6, y - 16, color_1);
  tft.drawPixel(x - 7, y - 16, color_1);
  tft.drawPixel(x - 8, y - 16, color_1);
  tft.drawPixel(x - 9, y - 16, color_1);
  tft.drawPixel(x - 10, y - 16, color_1);
  tft.drawPixel(x - 11, y - 16, color_1);
  tft.drawPixel(x - 12, y - 16, color_1);
  tft.drawPixel(x - 13, y - 16, color_1);
  tft.drawPixel(x - 14, y - 16, color_1);
  tft.drawPixel(x - 15, y - 16, color_1);
  tft.drawPixel(x - 16, y - 16, color_2);

  tft.drawPixel(x - 2, y - 17, color_2);
  tft.drawPixel(x - 3, y - 17, color_2);
  tft.drawPixel(x - 4, y - 17, color_2);
  tft.drawPixel(x - 5, y - 17, color_2);
  tft.drawPixel(x - 6, y - 17, color_2);
  tft.drawPixel(x - 7, y - 17, color_2);
  tft.drawPixel(x - 8, y - 17, color_2);
  tft.drawPixel(x - 9, y - 17, color_2);
  tft.drawPixel(x - 10, y - 17, color_2);
  tft.drawPixel(x - 11, y - 17, color_2);
  tft.drawPixel(x - 12, y - 17, color_2);
  tft.drawPixel(x - 13, y - 17, color_2);
  tft.drawPixel(x - 14, y - 17, color_2);
  tft.drawPixel(x - 15, y - 17, color_2);
  tft.drawPixel(x - 16, y - 17, color_2);
  tft.drawPixel(x - 17, y - 17, color_2);

  tft.drawPixel(x - 1, y - 18, color_2);
  tft.drawPixel(x - 2, y - 18, color_1);
  tft.drawPixel(x - 3, y - 18, color_1);
  tft.drawPixel(x - 4, y - 18, color_1);
  tft.drawPixel(x - 5, y - 18, color_1);
  tft.drawPixel(x - 6, y - 18, color_1);
  tft.drawPixel(x - 7, y - 18, color_1);
  tft.drawPixel(x - 8, y - 18, color_1);
  tft.drawPixel(x - 9, y - 18, color_1);
  tft.drawPixel(x - 10, y - 18, color_1);
  tft.drawPixel(x - 11, y - 18, color_1);
  tft.drawPixel(x - 12, y - 18, color_1);
  tft.drawPixel(x - 13, y - 18, color_1);
  tft.drawPixel(x - 14, y - 18, color_1);
  tft.drawPixel(x - 15, y - 18, color_1);
  tft.drawPixel(x - 16, y - 18, color_1);
  tft.drawPixel(x - 17, y - 18, color_1);
  tft.drawPixel(x - 18, y - 18, color_2);

  tft.drawPixel(x - 1, y - 19, color_2);
  tft.drawPixel(x - 2, y - 19, color_2);
  tft.drawPixel(x - 3, y - 19, color_2);
  tft.drawPixel(x - 4, y - 19, color_2);
  tft.drawPixel(x - 5, y - 19, color_2);
  tft.drawPixel(x - 6, y - 19, color_2);
  tft.drawPixel(x - 7, y - 19, color_2);
  tft.drawPixel(x - 8, y - 19, color_2);
  tft.drawPixel(x - 9, y - 19, color_2);
  tft.drawPixel(x - 10, y - 19, color_2);
  tft.drawPixel(x - 11, y - 19, color_2);
  tft.drawPixel(x - 12, y - 19, color_2);
  tft.drawPixel(x - 13, y - 19, color_2);
  tft.drawPixel(x - 14, y - 19, color_2);
  tft.drawPixel(x - 15, y - 19, color_2);
  tft.drawPixel(x - 16, y - 19, color_2);
  tft.drawPixel(x - 17, y - 19, color_2);
  tft.drawPixel(x - 18, y - 19, color_2);
}

void drawQueen2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x - 6, y - 0, color_2);
  tft.drawPixel(x - 7, y - 0, color_2);
  tft.drawPixel(x - 8, y - 0, color_2);
  tft.drawPixel(x - 11, y - 0, color_2);
  tft.drawPixel(x - 12, y - 0, color_2);
  tft.drawPixel(x - 13, y - 0, color_2);

  tft.drawPixel(x - 6, y - 1, color_2);
  tft.drawPixel(x - 7, y - 1, color_1);
  tft.drawPixel(x - 8, y - 1, color_2);
  tft.drawPixel(x - 11, y - 1, color_2);
  tft.drawPixel(x - 12, y - 1, color_1);
  tft.drawPixel(x - 13, y - 1, color_2);

  tft.drawPixel(x - 0, y - 2, color_2);
  tft.drawPixel(x - 1, y - 2, color_2);
  tft.drawPixel(x - 2, y - 2, color_2);
  tft.drawPixel(x - 6, y - 2, color_2);
  tft.drawPixel(x - 7, y - 2, color_2);
  tft.drawPixel(x - 8, y - 2, color_2);
  tft.drawPixel(x - 11, y - 2, color_2);
  tft.drawPixel(x - 12, y - 2, color_2);
  tft.drawPixel(x - 13, y - 2, color_2);
  tft.drawPixel(x - 17, y - 2, color_2);
  tft.drawPixel(x - 18, y - 2, color_2);
  tft.drawPixel(x - 19, y - 2, color_2);

  tft.drawPixel(x - 0, y - 3, color_2);
  tft.drawPixel(x - 1, y - 3, color_1);
  tft.drawPixel(x - 2, y - 3, color_2);
  tft.drawPixel(x - 7, y - 3, color_2);
  tft.drawPixel(x - 12, y - 3, color_2);
  tft.drawPixel(x - 17, y - 3, color_2);
  tft.drawPixel(x - 18, y - 3, color_1);
  tft.drawPixel(x - 19, y - 3, color_2);

  tft.drawPixel(x - 0, y - 4, color_2);
  tft.drawPixel(x - 1, y - 4, color_2);
  tft.drawPixel(x - 2, y - 4, color_2);
  tft.drawPixel(x - 6, y - 4, color_2);
  tft.drawPixel(x - 7, y - 4, color_1);
  tft.drawPixel(x - 8, y - 4, color_2);
  tft.drawPixel(x - 11, y - 4, color_2);
  tft.drawPixel(x - 12, y - 4, color_1);
  tft.drawPixel(x - 13, y - 4, color_2);
  tft.drawPixel(x - 17, y - 4, color_2);
  tft.drawPixel(x - 18, y - 4, color_2);
  tft.drawPixel(x - 19, y - 4, color_2);

  tft.drawPixel(x - 1, y - 5, color_2);
  tft.drawPixel(x - 2, y - 5, color_2);
  tft.drawPixel(x - 6, y - 5, color_2);
  tft.drawPixel(x - 7, y - 5, color_1);
  tft.drawPixel(x - 8, y - 5, color_2);
  tft.drawPixel(x - 11, y - 5, color_2);
  tft.drawPixel(x - 12, y - 5, color_1);
  tft.drawPixel(x - 13, y - 5, color_2);
  tft.drawPixel(x - 17, y - 5, color_2);
  tft.drawPixel(x - 18, y - 5, color_2);

  tft.drawPixel(x - 2, y - 6, color_2);
  tft.drawPixel(x - 5, y - 6, color_2);
  tft.drawPixel(x - 6, y - 6, color_2);
  tft.drawPixel(x - 7, y - 6, color_1);
  tft.drawPixel(x - 8, y - 6, color_2);
  tft.drawPixel(x - 11, y - 6, color_2);
  tft.drawPixel(x - 12, y - 6, color_1);
  tft.drawPixel(x - 13, y - 6, color_2);
  tft.drawPixel(x - 14, y - 6, color_2);
  tft.drawPixel(x - 17, y - 6, color_2);

  tft.drawPixel(x - 2, y - 7, color_2);
  tft.drawPixel(x - 3, y - 7, color_2);
  tft.drawPixel(x - 5, y - 7, color_2);
  tft.drawPixel(x - 6, y - 7, color_1);
  tft.drawPixel(x - 7, y - 7, color_1);
  tft.drawPixel(x - 8, y - 7, color_1);
  tft.drawPixel(x - 9, y - 7, color_2);
  tft.drawPixel(x - 10, y - 7, color_2);
  tft.drawPixel(x - 11, y - 7, color_1);
  tft.drawPixel(x - 12, y - 7, color_1);
  tft.drawPixel(x - 13, y - 7, color_1);
  tft.drawPixel(x - 14, y - 7, color_2);
  tft.drawPixel(x - 16, y - 7, color_2);
  tft.drawPixel(x - 17, y - 7, color_2);

  tft.drawPixel(x - 2, y - 8, color_2);
  tft.drawPixel(x - 3, y - 8, color_1);
  tft.drawPixel(x - 4, y - 8, color_2);
  tft.drawPixel(x - 5, y - 8, color_2);
  tft.drawPixel(x - 6, y - 8, color_1);
  tft.drawPixel(x - 7, y - 8, color_1);
  tft.drawPixel(x - 8, y - 8, color_1);
  tft.drawPixel(x - 9, y - 8, color_2);
  tft.drawPixel(x - 10, y - 8, color_2);
  tft.drawPixel(x - 11, y - 8, color_1);
  tft.drawPixel(x - 12, y - 8, color_1);
  tft.drawPixel(x - 13, y - 8, color_1);
  tft.drawPixel(x - 14, y - 8, color_2);
  tft.drawPixel(x - 15, y - 8, color_2);
  tft.drawPixel(x - 16, y - 8, color_1);
  tft.drawPixel(x - 17, y - 8, color_2);

  tft.drawPixel(x - 2, y - 9, color_2);
  tft.drawPixel(x - 3, y - 9, color_1);
  tft.drawPixel(x - 4, y - 9, color_1);
  tft.drawPixel(x - 5, y - 9, color_1);
  tft.drawPixel(x - 6, y - 9, color_1);
  tft.drawPixel(x - 7, y - 9, color_1);
  tft.drawPixel(x - 8, y - 9, color_1);
  tft.drawPixel(x - 9, y - 9, color_1);
  tft.drawPixel(x - 10, y - 9, color_1);
  tft.drawPixel(x - 11, y - 9, color_1);
  tft.drawPixel(x - 12, y - 9, color_1);
  tft.drawPixel(x - 13, y - 9, color_1);
  tft.drawPixel(x - 14, y - 9, color_1);
  tft.drawPixel(x - 15, y - 9, color_1);
  tft.drawPixel(x - 16, y - 9, color_1);
  tft.drawPixel(x - 17, y - 9, color_2);

  tft.drawPixel(x - 3, y - 10, color_2);
  tft.drawPixel(x - 4, y - 10, color_1);
  tft.drawPixel(x - 5, y - 10, color_1);
  tft.drawPixel(x - 6, y - 10, color_1);
  tft.drawPixel(x - 7, y - 10, color_1);
  tft.drawPixel(x - 8, y - 10, color_1);
  tft.drawPixel(x - 9, y - 10, color_1);
  tft.drawPixel(x - 10, y - 10, color_1);
  tft.drawPixel(x - 11, y - 10, color_1);
  tft.drawPixel(x - 12, y - 10, color_1);
  tft.drawPixel(x - 13, y - 10, color_1);
  tft.drawPixel(x - 14, y - 10, color_1);
  tft.drawPixel(x - 15, y - 10, color_1);
  tft.drawPixel(x - 16, y - 10, color_2);

  tft.drawPixel(x - 3, y - 11, color_2);
  tft.drawPixel(x - 4, y - 11, color_1);
  tft.drawPixel(x - 5, y - 11, color_1);
  tft.drawPixel(x - 6, y - 11, color_1);
  tft.drawPixel(x - 7, y - 11, color_1);
  tft.drawPixel(x - 8, y - 11, color_1);
  tft.drawPixel(x - 9, y - 11, color_1);
  tft.drawPixel(x - 10, y - 11, color_1);
  tft.drawPixel(x - 11, y - 11, color_1);
  tft.drawPixel(x - 12, y - 11, color_1);
  tft.drawPixel(x - 13, y - 11, color_1);
  tft.drawPixel(x - 14, y - 11, color_1);
  tft.drawPixel(x - 15, y - 11, color_1);
  tft.drawPixel(x - 16, y - 11, color_2);

  tft.drawPixel(x - 3, y - 12, color_2);
  tft.drawPixel(x - 4, y - 12, color_2);
  tft.drawPixel(x - 5, y - 12, color_1);
  tft.drawPixel(x - 6, y - 12, color_1);
  tft.drawPixel(x - 7, y - 12, color_1);
  tft.drawPixel(x - 8, y - 12, color_1);
  tft.drawPixel(x - 9, y - 12, color_1);
  tft.drawPixel(x - 10, y - 12, color_1);
  tft.drawPixel(x - 11, y - 12, color_1);
  tft.drawPixel(x - 12, y - 12, color_1);
  tft.drawPixel(x - 13, y - 12, color_1);
  tft.drawPixel(x - 14, y - 12, color_1);
  tft.drawPixel(x - 15, y - 12, color_2);
  tft.drawPixel(x - 16, y - 12, color_2);

  tft.drawPixel(x - 4, y - 13, color_2);
  tft.drawPixel(x - 5, y - 13, color_2);
  tft.drawPixel(x - 6, y - 13, color_2);
  tft.drawPixel(x - 7, y - 13, color_2);
  tft.drawPixel(x - 8, y - 13, color_2);
  tft.drawPixel(x - 9, y - 13, color_2);
  tft.drawPixel(x - 10, y - 13, color_2);
  tft.drawPixel(x - 11, y - 13, color_2);
  tft.drawPixel(x - 12, y - 13, color_2);
  tft.drawPixel(x - 13, y - 13, color_2);
  tft.drawPixel(x - 14, y - 13, color_2);
  tft.drawPixel(x - 15, y - 13, color_2);

  tft.drawPixel(x - 3, y - 14, color_2);
  tft.drawPixel(x - 4, y - 14, color_2);
  tft.drawPixel(x - 5, y - 14, color_1);
  tft.drawPixel(x - 6, y - 14, color_1);
  tft.drawPixel(x - 7, y - 14, color_1);
  tft.drawPixel(x - 8, y - 14, color_1);
  tft.drawPixel(x - 9, y - 14, color_1);
  tft.drawPixel(x - 10, y - 14, color_1);
  tft.drawPixel(x - 11, y - 14, color_1);
  tft.drawPixel(x - 12, y - 14, color_1);
  tft.drawPixel(x - 13, y - 14, color_1);
  tft.drawPixel(x - 14, y - 14, color_1);
  tft.drawPixel(x - 15, y - 14, color_2);
  tft.drawPixel(x - 16, y - 14, color_2);

  tft.drawPixel(x - 3, y - 15, color_2);
  tft.drawPixel(x - 4, y - 15, color_1);
  tft.drawPixel(x - 5, y - 15, color_1);
  tft.drawPixel(x - 6, y - 15, color_1);
  tft.drawPixel(x - 7, y - 15, color_1);
  tft.drawPixel(x - 8, y - 15, color_1);
  tft.drawPixel(x - 9, y - 15, color_1);
  tft.drawPixel(x - 10, y - 15, color_1);
  tft.drawPixel(x - 11, y - 15, color_1);
  tft.drawPixel(x - 12, y - 15, color_1);
  tft.drawPixel(x - 13, y - 15, color_1);
  tft.drawPixel(x - 14, y - 15, color_1);
  tft.drawPixel(x - 15, y - 15, color_1);
  tft.drawPixel(x - 16, y - 15, color_2);

  tft.drawPixel(x - 3, y - 16, color_2);
  tft.drawPixel(x - 4, y - 16, color_2);
  tft.drawPixel(x - 5, y - 16, color_2);
  tft.drawPixel(x - 6, y - 16, color_2);
  tft.drawPixel(x - 7, y - 16, color_2);
  tft.drawPixel(x - 8, y - 16, color_2);
  tft.drawPixel(x - 9, y - 16, color_2);
  tft.drawPixel(x - 10, y - 16, color_2);
  tft.drawPixel(x - 11, y - 16, color_2);
  tft.drawPixel(x - 12, y - 16, color_2);
  tft.drawPixel(x - 13, y - 16, color_2);
  tft.drawPixel(x - 14, y - 16, color_2);
  tft.drawPixel(x - 15, y - 16, color_2);
  tft.drawPixel(x - 16, y - 16, color_2);

  tft.drawPixel(x - 2, y - 17, color_2);
  tft.drawPixel(x - 3, y - 17, color_1);
  tft.drawPixel(x - 4, y - 17, color_1);
  tft.drawPixel(x - 5, y - 17, color_1);
  tft.drawPixel(x - 6, y - 17, color_1);
  tft.drawPixel(x - 7, y - 17, color_1);
  tft.drawPixel(x - 8, y - 17, color_1);
  tft.drawPixel(x - 9, y - 17, color_1);
  tft.drawPixel(x - 10, y - 17, color_1);
  tft.drawPixel(x - 11, y - 17, color_1);
  tft.drawPixel(x - 12, y - 17, color_1);
  tft.drawPixel(x - 13, y - 17, color_1);
  tft.drawPixel(x - 14, y - 17, color_1);
  tft.drawPixel(x - 15, y - 17, color_1);
  tft.drawPixel(x - 16, y - 17, color_1);
  tft.drawPixel(x - 17, y - 17, color_2);

  tft.drawPixel(x - 2, y - 18, color_2);
  tft.drawPixel(x - 3, y - 18, color_1);
  tft.drawPixel(x - 4, y - 18, color_1);
  tft.drawPixel(x - 5, y - 18, color_1);
  tft.drawPixel(x - 6, y - 18, color_1);
  tft.drawPixel(x - 7, y - 18, color_1);
  tft.drawPixel(x - 8, y - 18, color_1);
  tft.drawPixel(x - 9, y - 18, color_1);
  tft.drawPixel(x - 10, y - 18, color_1);
  tft.drawPixel(x - 11, y - 18, color_1);
  tft.drawPixel(x - 12, y - 18, color_1);
  tft.drawPixel(x - 13, y - 18, color_1);
  tft.drawPixel(x - 14, y - 18, color_1);
  tft.drawPixel(x - 15, y - 18, color_1);
  tft.drawPixel(x - 16, y - 18, color_1);
  tft.drawPixel(x - 17, y - 18, color_2);

  tft.drawPixel(x - 2, y - 19, color_2);
  tft.drawPixel(x - 3, y - 19, color_2);
  tft.drawPixel(x - 4, y - 19, color_2);
  tft.drawPixel(x - 5, y - 19, color_2);
  tft.drawPixel(x - 6, y - 19, color_2);
  tft.drawPixel(x - 7, y - 19, color_2);
  tft.drawPixel(x - 8, y - 19, color_2);
  tft.drawPixel(x - 9, y - 19, color_2);
  tft.drawPixel(x - 10, y - 19, color_2);
  tft.drawPixel(x - 11, y - 19, color_2);
  tft.drawPixel(x - 12, y - 19, color_2);
  tft.drawPixel(x - 13, y - 19, color_2);
  tft.drawPixel(x - 14, y - 19, color_2);
  tft.drawPixel(x - 15, y - 19, color_2);
  tft.drawPixel(x - 16, y - 19, color_2);
  tft.drawPixel(x - 17, y - 19, color_2);
}

void drawKing2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x - 9, y - 0, color_2);
  tft.drawPixel(x - 10, y - 0, color_2);

  tft.drawPixel(x - 8, y - 1, color_2);
  tft.drawPixel(x - 9, y - 1, color_2);
  tft.drawPixel(x - 10, y - 1, color_2);
  tft.drawPixel(x - 11, y - 1, color_2);

  tft.drawPixel(x - 9, y - 2, color_2);
  tft.drawPixel(x - 10, y - 2, color_2);

  tft.drawPixel(x - 9, y - 3, color_2);
  tft.drawPixel(x - 10, y - 3, color_2);

  tft.drawPixel(x - 3, y - 4, color_2);
  tft.drawPixel(x - 4, y - 4, color_2);
  tft.drawPixel(x - 5, y - 4, color_2);
  tft.drawPixel(x - 6, y - 4, color_2);
  tft.drawPixel(x - 8, y - 4, color_2);
  tft.drawPixel(x - 9, y - 4, color_2);
  tft.drawPixel(x - 10, y - 4, color_2);
  tft.drawPixel(x - 11, y - 4, color_2);
  tft.drawPixel(x - 13, y - 4, color_2);
  tft.drawPixel(x - 14, y - 4, color_2);
  tft.drawPixel(x - 15, y - 4, color_2);
  tft.drawPixel(x - 16, y - 4, color_2);

  tft.drawPixel(x - 1, y - 5, color_2);
  tft.drawPixel(x - 2, y - 5, color_2);
  tft.drawPixel(x - 3, y - 5, color_2);
  tft.drawPixel(x - 4, y - 5, color_1);
  tft.drawPixel(x - 5, y - 5, color_1);
  tft.drawPixel(x - 6, y - 5, color_2);
  tft.drawPixel(x - 7, y - 5, color_2);
  tft.drawPixel(x - 8, y - 5, color_2);
  tft.drawPixel(x - 9, y - 5, color_1);
  tft.drawPixel(x - 10, y - 5, color_1);
  tft.drawPixel(x - 11, y - 5, color_2);
  tft.drawPixel(x - 12, y - 5, color_2);
  tft.drawPixel(x - 13, y - 5, color_2);
  tft.drawPixel(x - 14, y - 5, color_1);
  tft.drawPixel(x - 15, y - 5, color_1);
  tft.drawPixel(x - 16, y - 5, color_2);
  tft.drawPixel(x - 17, y - 5, color_2);
  tft.drawPixel(x - 18, y - 5, color_2);

  tft.drawPixel(x - 0, y - 6, color_2);
  tft.drawPixel(x - 1, y - 6, color_2);
  tft.drawPixel(x - 2, y - 6, color_1);
  tft.drawPixel(x - 3, y - 6, color_1);
  tft.drawPixel(x - 4, y - 6, color_1);
  tft.drawPixel(x - 5, y - 6, color_1);
  tft.drawPixel(x - 6, y - 6, color_1);
  tft.drawPixel(x - 7, y - 6, color_2);
  tft.drawPixel(x - 8, y - 6, color_1);
  tft.drawPixel(x - 9, y - 6, color_1);
  tft.drawPixel(x - 10, y - 6, color_1);
  tft.drawPixel(x - 11, y - 6, color_1);
  tft.drawPixel(x - 12, y - 6, color_2);
  tft.drawPixel(x - 13, y - 6, color_1);
  tft.drawPixel(x - 14, y - 6, color_1);
  tft.drawPixel(x - 15, y - 6, color_1);
  tft.drawPixel(x - 16, y - 6, color_1);
  tft.drawPixel(x - 17, y - 6, color_1);
  tft.drawPixel(x - 18, y - 6, color_2);
  tft.drawPixel(x - 19, y - 6, color_2);

  tft.drawPixel(x - 0, y - 7, color_2);
  tft.drawPixel(x - 1, y - 7, color_1);
  tft.drawPixel(x - 2, y - 7, color_1);
  tft.drawPixel(x - 3, y - 7, color_1);
  tft.drawPixel(x - 4, y - 7, color_1);
  tft.drawPixel(x - 5, y - 7, color_1);
  tft.drawPixel(x - 6, y - 7, color_1);
  tft.drawPixel(x - 7, y - 7, color_2);
  tft.drawPixel(x - 8, y - 7, color_2);
  tft.drawPixel(x - 9, y - 7, color_1);
  tft.drawPixel(x - 10, y - 7, color_1);
  tft.drawPixel(x - 11, y - 7, color_2);
  tft.drawPixel(x - 12, y - 7, color_2);
  tft.drawPixel(x - 13, y - 7, color_1);
  tft.drawPixel(x - 14, y - 7, color_1);
  tft.drawPixel(x - 15, y - 7, color_1);
  tft.drawPixel(x - 16, y - 7, color_1);
  tft.drawPixel(x - 17, y - 7, color_1);
  tft.drawPixel(x - 18, y - 7, color_1);
  tft.drawPixel(x - 19, y - 7, color_2);

  tft.drawPixel(x - 0, y - 8, color_2);
  tft.drawPixel(x - 1, y - 8, color_1);
  tft.drawPixel(x - 2, y - 8, color_1);
  tft.drawPixel(x - 3, y - 8, color_1);
  tft.drawPixel(x - 4, y - 8, color_1);
  tft.drawPixel(x - 5, y - 8, color_1);
  tft.drawPixel(x - 6, y - 8, color_1);
  tft.drawPixel(x - 7, y - 8, color_1);
  tft.drawPixel(x - 8, y - 8, color_2);
  tft.drawPixel(x - 9, y - 8, color_2);
  tft.drawPixel(x - 10, y - 8, color_2);
  tft.drawPixel(x - 11, y - 8, color_2);
  tft.drawPixel(x - 12, y - 8, color_1);
  tft.drawPixel(x - 13, y - 8, color_1);
  tft.drawPixel(x - 14, y - 8, color_1);
  tft.drawPixel(x - 15, y - 8, color_1);
  tft.drawPixel(x - 16, y - 8, color_1);
  tft.drawPixel(x - 17, y - 8, color_1);
  tft.drawPixel(x - 18, y - 8, color_1);
  tft.drawPixel(x - 19, y - 8, color_2);

  tft.drawPixel(x - 0, y - 9, color_2);
  tft.drawPixel(x - 1, y - 9, color_1);
  tft.drawPixel(x - 2, y - 9, color_1);
  tft.drawPixel(x - 3, y - 9, color_1);
  tft.drawPixel(x - 4, y - 9, color_1);
  tft.drawPixel(x - 5, y - 9, color_1);
  tft.drawPixel(x - 6, y - 9, color_1);
  tft.drawPixel(x - 7, y - 9, color_1);
  tft.drawPixel(x - 8, y - 9, color_1);
  tft.drawPixel(x - 9, y - 9, color_2);
  tft.drawPixel(x - 10, y - 9, color_2);
  tft.drawPixel(x - 11, y - 9, color_1);
  tft.drawPixel(x - 12, y - 9, color_1);
  tft.drawPixel(x - 13, y - 9, color_1);
  tft.drawPixel(x - 14, y - 9, color_1);
  tft.drawPixel(x - 15, y - 9, color_1);
  tft.drawPixel(x - 16, y - 9, color_1);
  tft.drawPixel(x - 17, y - 9, color_1);
  tft.drawPixel(x - 18, y - 9, color_1);
  tft.drawPixel(x - 19, y - 9, color_2);

  tft.drawPixel(x - 0, y - 10, color_2);
  tft.drawPixel(x - 1, y - 10, color_2);
  tft.drawPixel(x - 2, y - 10, color_1);
  tft.drawPixel(x - 3, y - 10, color_1);
  tft.drawPixel(x - 4, y - 10, color_1);
  tft.drawPixel(x - 5, y - 10, color_1);
  tft.drawPixel(x - 6, y - 10, color_1);
  tft.drawPixel(x - 7, y - 10, color_1);
  tft.drawPixel(x - 8, y - 10, color_1);
  tft.drawPixel(x - 9, y - 10, color_2);
  tft.drawPixel(x - 10, y - 10, color_2);
  tft.drawPixel(x - 11, y - 10, color_1);
  tft.drawPixel(x - 12, y - 10, color_1);
  tft.drawPixel(x - 13, y - 10, color_1);
  tft.drawPixel(x - 14, y - 10, color_1);
  tft.drawPixel(x - 15, y - 10, color_1);
  tft.drawPixel(x - 16, y - 10, color_1);
  tft.drawPixel(x - 17, y - 10, color_1);
  tft.drawPixel(x - 18, y - 10, color_2);
  tft.drawPixel(x - 19, y - 10, color_2);

  tft.drawPixel(x - 1, y - 11, color_2);
  tft.drawPixel(x - 2, y - 11, color_1);
  tft.drawPixel(x - 3, y - 11, color_1);
  tft.drawPixel(x - 4, y - 11, color_1);
  tft.drawPixel(x - 5, y - 11, color_1);
  tft.drawPixel(x - 6, y - 11, color_1);
  tft.drawPixel(x - 7, y - 11, color_1);
  tft.drawPixel(x - 8, y - 11, color_1);
  tft.drawPixel(x - 9, y - 11, color_2);
  tft.drawPixel(x - 10, y - 11, color_2);
  tft.drawPixel(x - 11, y - 11, color_1);
  tft.drawPixel(x - 12, y - 11, color_1);
  tft.drawPixel(x - 13, y - 11, color_1);
  tft.drawPixel(x - 14, y - 11, color_1);
  tft.drawPixel(x - 15, y - 11, color_1);
  tft.drawPixel(x - 16, y - 11, color_1);
  tft.drawPixel(x - 17, y - 11, color_1);
  tft.drawPixel(x - 18, y - 11, color_2);

  tft.drawPixel(x - 1, y - 12, color_2);
  tft.drawPixel(x - 2, y - 12, color_2);
  tft.drawPixel(x - 3, y - 12, color_1);
  tft.drawPixel(x - 4, y - 12, color_1);
  tft.drawPixel(x - 5, y - 12, color_1);
  tft.drawPixel(x - 6, y - 12, color_1);
  tft.drawPixel(x - 7, y - 12, color_1);
  tft.drawPixel(x - 8, y - 12, color_1);
  tft.drawPixel(x - 9, y - 12, color_2);
  tft.drawPixel(x - 10, y - 12, color_2);
  tft.drawPixel(x - 11, y - 12, color_1);
  tft.drawPixel(x - 12, y - 12, color_1);
  tft.drawPixel(x - 13, y - 12, color_1);
  tft.drawPixel(x - 14, y - 12, color_1);
  tft.drawPixel(x - 15, y - 12, color_1);
  tft.drawPixel(x - 16, y - 12, color_1);
  tft.drawPixel(x - 17, y - 12, color_2);
  tft.drawPixel(x - 18, y - 12, color_2);

  tft.drawPixel(x - 2, y - 13, color_2);
  tft.drawPixel(x - 3, y - 13, color_2);
  tft.drawPixel(x - 4, y - 13, color_1);
  tft.drawPixel(x - 5, y - 13, color_1);
  tft.drawPixel(x - 6, y - 13, color_1);
  tft.drawPixel(x - 7, y - 13, color_1);
  tft.drawPixel(x - 8, y - 13, color_1);
  tft.drawPixel(x - 9, y - 13, color_2);
  tft.drawPixel(x - 10, y - 13, color_2);
  tft.drawPixel(x - 11, y - 13, color_1);
  tft.drawPixel(x - 12, y - 13, color_1);
  tft.drawPixel(x - 13, y - 13, color_1);
  tft.drawPixel(x - 14, y - 13, color_1);
  tft.drawPixel(x - 15, y - 13, color_1);
  tft.drawPixel(x - 16, y - 13, color_2);
  tft.drawPixel(x - 17, y - 13, color_2);

  tft.drawPixel(x - 3, y - 14, color_2);
  tft.drawPixel(x - 4, y - 14, color_2);
  tft.drawPixel(x - 5, y - 14, color_2);
  tft.drawPixel(x - 6, y - 14, color_2);
  tft.drawPixel(x - 7, y - 14, color_2);
  tft.drawPixel(x - 8, y - 14, color_2);
  tft.drawPixel(x - 9, y - 14, color_2);
  tft.drawPixel(x - 10, y - 14, color_2);
  tft.drawPixel(x - 11, y - 14, color_2);
  tft.drawPixel(x - 12, y - 14, color_2);
  tft.drawPixel(x - 13, y - 14, color_2);
  tft.drawPixel(x - 14, y - 14, color_2);
  tft.drawPixel(x - 15, y - 14, color_2);
  tft.drawPixel(x - 16, y - 14, color_2);

  tft.drawPixel(x - 2, y - 15, color_2);
  tft.drawPixel(x - 3, y - 15, color_2);
  tft.drawPixel(x - 4, y - 15, color_1);
  tft.drawPixel(x - 5, y - 15, color_1);
  tft.drawPixel(x - 6, y - 15, color_1);
  tft.drawPixel(x - 7, y - 15, color_1);
  tft.drawPixel(x - 8, y - 15, color_1);
  tft.drawPixel(x - 9, y - 15, color_1);
  tft.drawPixel(x - 10, y - 15, color_1);
  tft.drawPixel(x - 11, y - 15, color_1);
  tft.drawPixel(x - 12, y - 15, color_1);
  tft.drawPixel(x - 13, y - 15, color_1);
  tft.drawPixel(x - 14, y - 15, color_1);
  tft.drawPixel(x - 15, y - 15, color_1);
  tft.drawPixel(x - 16, y - 15, color_2);
  tft.drawPixel(x - 17, y - 15, color_2);

  tft.drawPixel(x - 2, y - 16, color_2);
  tft.drawPixel(x - 3, y - 16, color_1);
  tft.drawPixel(x - 4, y - 16, color_1);
  tft.drawPixel(x - 5, y - 16, color_2);
  tft.drawPixel(x - 6, y - 16, color_2);
  tft.drawPixel(x - 7, y - 16, color_2);
  tft.drawPixel(x - 8, y - 16, color_2);
  tft.drawPixel(x - 9, y - 16, color_2);
  tft.drawPixel(x - 10, y - 16, color_2);
  tft.drawPixel(x - 11, y - 16, color_2);
  tft.drawPixel(x - 12, y - 16, color_2);
  tft.drawPixel(x - 13, y - 16, color_2);
  tft.drawPixel(x - 14, y - 16, color_2);
  tft.drawPixel(x - 15, y - 16, color_1);
  tft.drawPixel(x - 16, y - 16, color_1);
  tft.drawPixel(x - 17, y - 16, color_2);

  tft.drawPixel(x - 2, y - 17, color_2);
  tft.drawPixel(x - 3, y - 17, color_2);
  tft.drawPixel(x - 4, y - 17, color_2);
  tft.drawPixel(x - 5, y - 17, color_1);
  tft.drawPixel(x - 6, y - 17, color_1);
  tft.drawPixel(x - 7, y - 17, color_1);
  tft.drawPixel(x - 8, y - 17, color_1);
  tft.drawPixel(x - 9, y - 17, color_1);
  tft.drawPixel(x - 10, y - 17, color_1);
  tft.drawPixel(x - 11, y - 17, color_1);
  tft.drawPixel(x - 12, y - 17, color_1);
  tft.drawPixel(x - 13, y - 17, color_1);
  tft.drawPixel(x - 14, y - 17, color_1);
  tft.drawPixel(x - 15, y - 17, color_2);
  tft.drawPixel(x - 16, y - 17, color_2);
  tft.drawPixel(x - 17, y - 17, color_2);

  tft.drawPixel(x - 2, y - 18, color_2);
  tft.drawPixel(x - 3, y - 18, color_1);
  tft.drawPixel(x - 4, y - 18, color_1);
  tft.drawPixel(x - 5, y - 18, color_1);
  tft.drawPixel(x - 6, y - 18, color_1);
  tft.drawPixel(x - 7, y - 18, color_1);
  tft.drawPixel(x - 8, y - 18, color_1);
  tft.drawPixel(x - 9, y - 18, color_1);
  tft.drawPixel(x - 10, y - 18, color_1);
  tft.drawPixel(x - 11, y - 18, color_1);
  tft.drawPixel(x - 12, y - 18, color_1);
  tft.drawPixel(x - 13, y - 18, color_1);
  tft.drawPixel(x - 14, y - 18, color_1);
  tft.drawPixel(x - 15, y - 18, color_1);
  tft.drawPixel(x - 16, y - 18, color_1);
  tft.drawPixel(x - 17, y - 18, color_2);

  tft.drawPixel(x - 2, y - 19, color_2);
  tft.drawPixel(x - 3, y - 19, color_2);
  tft.drawPixel(x - 4, y - 19, color_2);
  tft.drawPixel(x - 5, y - 19, color_2);
  tft.drawPixel(x - 6, y - 19, color_2);
  tft.drawPixel(x - 7, y - 19, color_2);
  tft.drawPixel(x - 8, y - 19, color_2);
  tft.drawPixel(x - 9, y - 19, color_2);
  tft.drawPixel(x - 10, y - 19, color_2);
  tft.drawPixel(x - 11, y - 19, color_2);
  tft.drawPixel(x - 12, y - 19, color_2);
  tft.drawPixel(x - 13, y - 19, color_2);
  tft.drawPixel(x - 14, y - 19, color_2);
  tft.drawPixel(x - 15, y - 19, color_2);
  tft.drawPixel(x - 16, y - 19, color_2);
  tft.drawPixel(x - 17, y - 19, color_2);
}

void drawRook2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x - 2, y - 0, color_2);
  tft.drawPixel(x - 3, y - 0, color_2);
  tft.drawPixel(x - 4, y - 0, color_2);
  tft.drawPixel(x - 5, y - 0, color_2);
  tft.drawPixel(x - 8, y - 0, color_2);
  tft.drawPixel(x - 9, y - 0, color_2);
  tft.drawPixel(x - 10, y - 0, color_2);
  tft.drawPixel(x - 11, y - 0, color_2);
  tft.drawPixel(x - 14, y - 0, color_2);
  tft.drawPixel(x - 15, y - 0, color_2);
  tft.drawPixel(x - 16, y - 0, color_2);
  tft.drawPixel(x - 17, y - 0, color_2);

  tft.drawPixel(x - 2, y - 1, color_2);
  tft.drawPixel(x - 3, y - 1, color_1);
  tft.drawPixel(x - 4, y - 1, color_1);
  tft.drawPixel(x - 5, y - 1, color_2);
  tft.drawPixel(x - 8, y - 1, color_2);
  tft.drawPixel(x - 9, y - 1, color_1);
  tft.drawPixel(x - 10, y - 1, color_1);
  tft.drawPixel(x - 11, y - 1, color_2);
  tft.drawPixel(x - 14, y - 1, color_2);
  tft.drawPixel(x - 15, y - 1, color_1);
  tft.drawPixel(x - 16, y - 1, color_1);
  tft.drawPixel(x - 17, y - 1, color_2);

  tft.drawPixel(x - 2, y - 2, color_2);
  tft.drawPixel(x - 3, y - 2, color_1);
  tft.drawPixel(x - 4, y - 2, color_1);
  tft.drawPixel(x - 5, y - 2, color_2);
  tft.drawPixel(x - 6, y - 2, color_2);
  tft.drawPixel(x - 7, y - 2, color_2);
  tft.drawPixel(x - 8, y - 2, color_2);
  tft.drawPixel(x - 9, y - 2, color_1);
  tft.drawPixel(x - 10, y - 2, color_1);
  tft.drawPixel(x - 11, y - 2, color_2);
  tft.drawPixel(x - 12, y - 2, color_2);
  tft.drawPixel(x - 13, y - 2, color_2);
  tft.drawPixel(x - 14, y - 2, color_2);
  tft.drawPixel(x - 15, y - 2, color_1);
  tft.drawPixel(x - 16, y - 2, color_1);
  tft.drawPixel(x - 17, y - 2, color_2);

  tft.drawPixel(x - 2, y - 3, color_2);
  tft.drawPixel(x - 3, y - 3, color_1);
  tft.drawPixel(x - 4, y - 3, color_1);
  tft.drawPixel(x - 5, y - 3, color_1);
  tft.drawPixel(x - 6, y - 3, color_1);
  tft.drawPixel(x - 7, y - 3, color_1);
  tft.drawPixel(x - 8, y - 3, color_1);
  tft.drawPixel(x - 9, y - 3, color_1);
  tft.drawPixel(x - 10, y - 3, color_1);
  tft.drawPixel(x - 11, y - 3, color_1);
  tft.drawPixel(x - 12, y - 3, color_1);
  tft.drawPixel(x - 13, y - 3, color_1);
  tft.drawPixel(x - 14, y - 3, color_1);
  tft.drawPixel(x - 15, y - 3, color_1);
  tft.drawPixel(x - 16, y - 3, color_1);
  tft.drawPixel(x - 17, y - 3, color_2);

  tft.drawPixel(x - 2, y - 4, color_2);
  tft.drawPixel(x - 3, y - 4, color_2);
  tft.drawPixel(x - 4, y - 4, color_1);
  tft.drawPixel(x - 5, y - 4, color_1);
  tft.drawPixel(x - 6, y - 4, color_1);
  tft.drawPixel(x - 7, y - 4, color_1);
  tft.drawPixel(x - 8, y - 4, color_1);
  tft.drawPixel(x - 9, y - 4, color_1);
  tft.drawPixel(x - 10, y - 4, color_1);
  tft.drawPixel(x - 11, y - 4, color_1);
  tft.drawPixel(x - 12, y - 4, color_1);
  tft.drawPixel(x - 13, y - 4, color_1);
  tft.drawPixel(x - 14, y - 4, color_1);
  tft.drawPixel(x - 15, y - 4, color_1);
  tft.drawPixel(x - 16, y - 4, color_2);
  tft.drawPixel(x - 17, y - 4, color_2);

  tft.drawPixel(x - 3, y - 5, color_2);
  tft.drawPixel(x - 4, y - 5, color_2);
  tft.drawPixel(x - 5, y - 5, color_1);
  tft.drawPixel(x - 6, y - 5, color_1);
  tft.drawPixel(x - 7, y - 5, color_1);
  tft.drawPixel(x - 8, y - 5, color_1);
  tft.drawPixel(x - 9, y - 5, color_1);
  tft.drawPixel(x - 10, y - 5, color_1);
  tft.drawPixel(x - 11, y - 5, color_1);
  tft.drawPixel(x - 12, y - 5, color_1);
  tft.drawPixel(x - 13, y - 5, color_1);
  tft.drawPixel(x - 14, y - 5, color_1);
  tft.drawPixel(x - 15, y - 5, color_2);
  tft.drawPixel(x - 16, y - 5, color_2);

  tft.drawPixel(x - 4, y - 6, color_2);
  tft.drawPixel(x - 5, y - 6, color_2);
  tft.drawPixel(x - 6, y - 6, color_2);
  tft.drawPixel(x - 7, y - 6, color_2);
  tft.drawPixel(x - 8, y - 6, color_2);
  tft.drawPixel(x - 9, y - 6, color_2);
  tft.drawPixel(x - 10, y - 6, color_2);
  tft.drawPixel(x - 11, y - 6, color_2);
  tft.drawPixel(x - 12, y - 6, color_2);
  tft.drawPixel(x - 13, y - 6, color_2);
  tft.drawPixel(x - 14, y - 6, color_2);
  tft.drawPixel(x - 15, y - 6, color_2);

  tft.drawPixel(x - 5, y - 7, color_2);
  tft.drawPixel(x - 6, y - 7, color_1);
  tft.drawPixel(x - 7, y - 7, color_1);
  tft.drawPixel(x - 8, y - 7, color_1);
  tft.drawPixel(x - 9, y - 7, color_1);
  tft.drawPixel(x - 10, y - 7, color_1);
  tft.drawPixel(x - 11, y - 7, color_1);
  tft.drawPixel(x - 12, y - 7, color_1);
  tft.drawPixel(x - 13, y - 7, color_1);
  tft.drawPixel(x - 14, y - 7, color_2);

  tft.drawPixel(x - 5, y - 8, color_2);
  tft.drawPixel(x - 6, y - 8, color_1);
  tft.drawPixel(x - 7, y - 8, color_1);
  tft.drawPixel(x - 8, y - 8, color_1);
  tft.drawPixel(x - 9, y - 8, color_1);
  tft.drawPixel(x - 10, y - 8, color_1);
  tft.drawPixel(x - 11, y - 8, color_1);
  tft.drawPixel(x - 12, y - 8, color_1);
  tft.drawPixel(x - 13, y - 8, color_1);
  tft.drawPixel(x - 14, y - 8, color_2);

  tft.drawPixel(x - 5, y - 9, color_2);
  tft.drawPixel(x - 6, y - 9, color_1);
  tft.drawPixel(x - 7, y - 9, color_1);
  tft.drawPixel(x - 8, y - 9, color_1);
  tft.drawPixel(x - 9, y - 9, color_1);
  tft.drawPixel(x - 10, y - 9, color_1);
  tft.drawPixel(x - 11, y - 9, color_1);
  tft.drawPixel(x - 12, y - 9, color_1);
  tft.drawPixel(x - 13, y - 9, color_1);
  tft.drawPixel(x - 14, y - 9, color_2);

  tft.drawPixel(x - 5, y - 10, color_2);
  tft.drawPixel(x - 6, y - 10, color_1);
  tft.drawPixel(x - 7, y - 10, color_1);
  tft.drawPixel(x - 8, y - 10, color_1);
  tft.drawPixel(x - 9, y - 10, color_1);
  tft.drawPixel(x - 10, y - 10, color_1);
  tft.drawPixel(x - 11, y - 10, color_1);
  tft.drawPixel(x - 12, y - 10, color_1);
  tft.drawPixel(x - 13, y - 10, color_1);
  tft.drawPixel(x - 14, y - 10, color_2);

  tft.drawPixel(x - 5, y - 11, color_2);
  tft.drawPixel(x - 6, y - 11, color_1);
  tft.drawPixel(x - 7, y - 11, color_1);
  tft.drawPixel(x - 8, y - 11, color_1);
  tft.drawPixel(x - 9, y - 11, color_1);
  tft.drawPixel(x - 10, y - 11, color_1);
  tft.drawPixel(x - 11, y - 11, color_1);
  tft.drawPixel(x - 12, y - 11, color_1);
  tft.drawPixel(x - 13, y - 11, color_1);
  tft.drawPixel(x - 14, y - 11, color_2);

  tft.drawPixel(x - 5, y - 12, color_2);
  tft.drawPixel(x - 6, y - 12, color_1);
  tft.drawPixel(x - 7, y - 12, color_1);
  tft.drawPixel(x - 8, y - 12, color_1);
  tft.drawPixel(x - 9, y - 12, color_1);
  tft.drawPixel(x - 10, y - 12, color_1);
  tft.drawPixel(x - 11, y - 12, color_1);
  tft.drawPixel(x - 12, y - 12, color_1);
  tft.drawPixel(x - 13, y - 12, color_1);
  tft.drawPixel(x - 14, y - 12, color_2);

  tft.drawPixel(x - 5, y - 13, color_2);
  tft.drawPixel(x - 6, y - 13, color_1);
  tft.drawPixel(x - 7, y - 13, color_1);
  tft.drawPixel(x - 8, y - 13, color_1);
  tft.drawPixel(x - 9, y - 13, color_1);
  tft.drawPixel(x - 10, y - 13, color_1);
  tft.drawPixel(x - 11, y - 13, color_1);
  tft.drawPixel(x - 12, y - 13, color_1);
  tft.drawPixel(x - 13, y - 13, color_1);
  tft.drawPixel(x - 14, y - 13, color_2);

  tft.drawPixel(x - 4, y - 14, color_2);
  tft.drawPixel(x - 5, y - 14, color_2);
  tft.drawPixel(x - 6, y - 14, color_2);
  tft.drawPixel(x - 7, y - 14, color_2);
  tft.drawPixel(x - 8, y - 14, color_2);
  tft.drawPixel(x - 9, y - 14, color_2);
  tft.drawPixel(x - 10, y - 14, color_2);
  tft.drawPixel(x - 11, y - 14, color_2);
  tft.drawPixel(x - 12, y - 14, color_2);
  tft.drawPixel(x - 13, y - 14, color_2);
  tft.drawPixel(x - 14, y - 14, color_2);
  tft.drawPixel(x - 15, y - 14, color_2);

  tft.drawPixel(x - 3, y - 15, color_2);
  tft.drawPixel(x - 4, y - 15, color_2);
  tft.drawPixel(x - 5, y - 15, color_1);
  tft.drawPixel(x - 6, y - 15, color_1);
  tft.drawPixel(x - 7, y - 15, color_1);
  tft.drawPixel(x - 8, y - 15, color_1);
  tft.drawPixel(x - 9, y - 15, color_1);
  tft.drawPixel(x - 10, y - 15, color_1);
  tft.drawPixel(x - 11, y - 15, color_1);
  tft.drawPixel(x - 12, y - 15, color_1);
  tft.drawPixel(x - 13, y - 15, color_1);
  tft.drawPixel(x - 14, y - 15, color_1);
  tft.drawPixel(x - 15, y - 15, color_2);
  tft.drawPixel(x - 16, y - 15, color_2);

  tft.drawPixel(x - 2, y - 16, color_2);
  tft.drawPixel(x - 3, y - 16, color_2);
  tft.drawPixel(x - 4, y - 16, color_1);
  tft.drawPixel(x - 5, y - 16, color_1);
  tft.drawPixel(x - 6, y - 16, color_1);
  tft.drawPixel(x - 7, y - 16, color_1);
  tft.drawPixel(x - 8, y - 16, color_1);
  tft.drawPixel(x - 9, y - 16, color_1);
  tft.drawPixel(x - 10, y - 16, color_1);
  tft.drawPixel(x - 11, y - 16, color_1);
  tft.drawPixel(x - 12, y - 16, color_1);
  tft.drawPixel(x - 13, y - 16, color_1);
  tft.drawPixel(x - 14, y - 16, color_1);
  tft.drawPixel(x - 15, y - 16, color_1);
  tft.drawPixel(x - 16, y - 16, color_2);
  tft.drawPixel(x - 17, y - 16, color_2);

  tft.drawPixel(x - 1, y - 17, color_2);
  tft.drawPixel(x - 2, y - 17, color_2);
  tft.drawPixel(x - 3, y - 17, color_2);
  tft.drawPixel(x - 4, y - 17, color_2);
  tft.drawPixel(x - 5, y - 17, color_2);
  tft.drawPixel(x - 6, y - 17, color_2);
  tft.drawPixel(x - 7, y - 17, color_2);
  tft.drawPixel(x - 8, y - 17, color_2);
  tft.drawPixel(x - 9, y - 17, color_2);
  tft.drawPixel(x - 10, y - 17, color_2);
  tft.drawPixel(x - 11, y - 17, color_2);
  tft.drawPixel(x - 12, y - 17, color_2);
  tft.drawPixel(x - 13, y - 17, color_2);
  tft.drawPixel(x - 14, y - 17, color_2);
  tft.drawPixel(x - 15, y - 17, color_2);
  tft.drawPixel(x - 16, y - 17, color_2);
  tft.drawPixel(x - 17, y - 17, color_2);
  tft.drawPixel(x - 18, y - 17, color_2);

  tft.drawPixel(x - 1, y - 18, color_2);
  tft.drawPixel(x - 2, y - 18, color_1);
  tft.drawPixel(x - 3, y - 18, color_1);
  tft.drawPixel(x - 4, y - 18, color_1);
  tft.drawPixel(x - 5, y - 18, color_1);
  tft.drawPixel(x - 6, y - 18, color_1);
  tft.drawPixel(x - 7, y - 18, color_1);
  tft.drawPixel(x - 8, y - 18, color_1);
  tft.drawPixel(x - 9, y - 18, color_1);
  tft.drawPixel(x - 10, y - 18, color_1);
  tft.drawPixel(x - 11, y - 18, color_1);
  tft.drawPixel(x - 12, y - 18, color_1);
  tft.drawPixel(x - 13, y - 18, color_1);
  tft.drawPixel(x - 14, y - 18, color_1);
  tft.drawPixel(x - 15, y - 18, color_1);
  tft.drawPixel(x - 16, y - 18, color_1);
  tft.drawPixel(x - 17, y - 18, color_1);
  tft.drawPixel(x - 18, y - 18, color_2);

  tft.drawPixel(x - 1, y - 19, color_2);
  tft.drawPixel(x - 2, y - 19, color_2);
  tft.drawPixel(x - 3, y - 19, color_2);
  tft.drawPixel(x - 4, y - 19, color_2);
  tft.drawPixel(x - 5, y - 19, color_2);
  tft.drawPixel(x - 6, y - 19, color_2);
  tft.drawPixel(x - 7, y - 19, color_2);
  tft.drawPixel(x - 8, y - 19, color_2);
  tft.drawPixel(x - 9, y - 19, color_2);
  tft.drawPixel(x - 10, y - 19, color_2);
  tft.drawPixel(x - 11, y - 19, color_2);
  tft.drawPixel(x - 12, y - 19, color_2);
  tft.drawPixel(x - 13, y - 19, color_2);
  tft.drawPixel(x - 14, y - 19, color_2);
  tft.drawPixel(x - 15, y - 19, color_2);
  tft.drawPixel(x - 16, y - 19, color_2);
  tft.drawPixel(x - 17, y - 19, color_2);
  tft.drawPixel(x - 18, y - 19, color_2);
}

void drawBishop2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x - 9, y - 0, color_2);
  tft.drawPixel(x - 10, y - 0, color_2);

  tft.drawPixel(x - 8, y - 1, color_2);
  tft.drawPixel(x - 9, y - 1, color_1);
  tft.drawPixel(x - 10, y - 1, color_1);
  tft.drawPixel(x - 11, y - 1, color_2);

  tft.drawPixel(x - 9, y - 2, color_2);
  tft.drawPixel(x - 10, y - 2, color_2);

  tft.drawPixel(x - 7, y - 3, color_2);
  tft.drawPixel(x - 8, y - 3, color_2);
  tft.drawPixel(x - 9, y - 3, color_1);
  tft.drawPixel(x - 10, y - 3, color_1);
  tft.drawPixel(x - 11, y - 3, color_2);
  tft.drawPixel(x - 12, y - 3, color_2);

  tft.drawPixel(x - 7, y - 4, color_2);
  tft.drawPixel(x - 8, y - 4, color_1);
  tft.drawPixel(x - 9, y - 4, color_1);
  tft.drawPixel(x - 10, y - 4, color_2);
  tft.drawPixel(x - 11, y - 4, color_2);
  tft.drawPixel(x - 12, y - 4, color_2);

  tft.drawPixel(x - 6, y - 5, color_2);
  tft.drawPixel(x - 7, y - 5, color_2);
  tft.drawPixel(x - 8, y - 5, color_1);
  tft.drawPixel(x - 9, y - 5, color_1);
  tft.drawPixel(x - 10, y - 5, color_2);
  tft.drawPixel(x - 11, y - 5, color_2);
  tft.drawPixel(x - 12, y - 5, color_2);
  tft.drawPixel(x - 13, y - 5, color_2);

  tft.drawPixel(x - 6, y - 6, color_2);
  tft.drawPixel(x - 7, y - 6, color_1);
  tft.drawPixel(x - 8, y - 6, color_1);
  tft.drawPixel(x - 9, y - 6, color_1);
  tft.drawPixel(x - 10, y - 6, color_2);
  tft.drawPixel(x - 11, y - 6, color_2);
  tft.drawPixel(x - 12, y - 6, color_1);
  tft.drawPixel(x - 13, y - 6, color_2);

  tft.drawPixel(x - 6, y - 7, color_2);
  tft.drawPixel(x - 7, y - 7, color_1);
  tft.drawPixel(x - 8, y - 7, color_1);
  tft.drawPixel(x - 9, y - 7, color_2);
  tft.drawPixel(x - 10, y - 7, color_2);
  tft.drawPixel(x - 11, y - 7, color_1);
  tft.drawPixel(x - 12, y - 7, color_1);
  tft.drawPixel(x - 13, y - 7, color_2);

  tft.drawPixel(x - 6, y - 8, color_2);
  tft.drawPixel(x - 7, y - 8, color_1);
  tft.drawPixel(x - 8, y - 8, color_1);
  tft.drawPixel(x - 9, y - 8, color_2);
  tft.drawPixel(x - 10, y - 8, color_2);
  tft.drawPixel(x - 11, y - 8, color_1);
  tft.drawPixel(x - 12, y - 8, color_1);
  tft.drawPixel(x - 13, y - 8, color_2);

  tft.drawPixel(x - 6, y - 9, color_2);
  tft.drawPixel(x - 7, y - 9, color_1);
  tft.drawPixel(x - 8, y - 9, color_1);
  tft.drawPixel(x - 9, y - 9, color_1);
  tft.drawPixel(x - 10, y - 9, color_1);
  tft.drawPixel(x - 11, y - 9, color_1);
  tft.drawPixel(x - 12, y - 9, color_1);
  tft.drawPixel(x - 13, y - 9, color_2);

  tft.drawPixel(x - 6, y - 10, color_2);
  tft.drawPixel(x - 7, y - 10, color_1);
  tft.drawPixel(x - 8, y - 10, color_1);
  tft.drawPixel(x - 9, y - 10, color_1);
  tft.drawPixel(x - 10, y - 10, color_1);
  tft.drawPixel(x - 11, y - 10, color_1);
  tft.drawPixel(x - 12, y - 10, color_1);
  tft.drawPixel(x - 13, y - 10, color_2);

  tft.drawPixel(x - 6, y - 11, color_2);
  tft.drawPixel(x - 7, y - 11, color_1);
  tft.drawPixel(x - 8, y - 11, color_1);
  tft.drawPixel(x - 9, y - 11, color_1);
  tft.drawPixel(x - 10, y - 11, color_1);
  tft.drawPixel(x - 11, y - 11, color_1);
  tft.drawPixel(x - 12, y - 11, color_1);
  tft.drawPixel(x - 13, y - 11, color_2);

  tft.drawPixel(x - 6, y - 12, color_2);
  tft.drawPixel(x - 7, y - 12, color_2);
  tft.drawPixel(x - 8, y - 12, color_1);
  tft.drawPixel(x - 9, y - 12, color_1);
  tft.drawPixel(x - 10, y - 12, color_1);
  tft.drawPixel(x - 11, y - 12, color_1);
  tft.drawPixel(x - 12, y - 12, color_2);
  tft.drawPixel(x - 13, y - 12, color_2);

  tft.drawPixel(x - 7, y - 13, color_2);
  tft.drawPixel(x - 8, y - 13, color_2);
  tft.drawPixel(x - 9, y - 13, color_1);
  tft.drawPixel(x - 10, y - 13, color_1);
  tft.drawPixel(x - 11, y - 13, color_2);
  tft.drawPixel(x - 12, y - 13, color_2);

  tft.drawPixel(x - 8, y - 14, color_2);
  tft.drawPixel(x - 9, y - 14, color_1);
  tft.drawPixel(x - 10, y - 14, color_1);
  tft.drawPixel(x - 11, y - 14, color_2);

  tft.drawPixel(x - 7, y - 15, color_2);
  tft.drawPixel(x - 8, y - 15, color_2);
  tft.drawPixel(x - 9, y - 15, color_1);
  tft.drawPixel(x - 10, y - 15, color_1);
  tft.drawPixel(x - 11, y - 15, color_2);
  tft.drawPixel(x - 12, y - 15, color_2);

  tft.drawPixel(x - 6, y - 16, color_2);
  tft.drawPixel(x - 7, y - 16, color_2);
  tft.drawPixel(x - 8, y - 16, color_1);
  tft.drawPixel(x - 9, y - 16, color_1);
  tft.drawPixel(x - 10, y - 16, color_1);
  tft.drawPixel(x - 11, y - 16, color_1);
  tft.drawPixel(x - 12, y - 16, color_2);
  tft.drawPixel(x - 13, y - 16, color_2);

  tft.drawPixel(x - 3, y - 17, color_2);
  tft.drawPixel(x - 4, y - 17, color_2);
  tft.drawPixel(x - 5, y - 17, color_2);
  tft.drawPixel(x - 6, y - 17, color_2);
  tft.drawPixel(x - 7, y - 17, color_2);
  tft.drawPixel(x - 8, y - 17, color_2);
  tft.drawPixel(x - 9, y - 17, color_2);
  tft.drawPixel(x - 10, y - 17, color_2);
  tft.drawPixel(x - 11, y - 17, color_2);
  tft.drawPixel(x - 12, y - 17, color_2);
  tft.drawPixel(x - 13, y - 17, color_2);
  tft.drawPixel(x - 14, y - 17, color_2);
  tft.drawPixel(x - 15, y - 17, color_2);
  tft.drawPixel(x - 16, y - 17, color_2);

  tft.drawPixel(x - 3, y - 18, color_2);
  tft.drawPixel(x - 4, y - 18, color_1);
  tft.drawPixel(x - 5, y - 18, color_1);
  tft.drawPixel(x - 6, y - 18, color_1);
  tft.drawPixel(x - 7, y - 18, color_1);
  tft.drawPixel(x - 8, y - 18, color_1);
  tft.drawPixel(x - 9, y - 18, color_1);
  tft.drawPixel(x - 10, y - 18, color_1);
  tft.drawPixel(x - 11, y - 18, color_1);
  tft.drawPixel(x - 12, y - 18, color_1);
  tft.drawPixel(x - 13, y - 18, color_1);
  tft.drawPixel(x - 14, y - 18, color_1);
  tft.drawPixel(x - 15, y - 18, color_1);
  tft.drawPixel(x - 16, y - 18, color_2);

  tft.drawPixel(x - 3, y - 19, color_2);
  tft.drawPixel(x - 4, y - 19, color_2);
  tft.drawPixel(x - 5, y - 19, color_2);
  tft.drawPixel(x - 6, y - 19, color_2);
  tft.drawPixel(x - 7, y - 19, color_2);
  tft.drawPixel(x - 8, y - 19, color_2);
  tft.drawPixel(x - 9, y - 19, color_2);
  tft.drawPixel(x - 10, y - 19, color_2);
  tft.drawPixel(x - 11, y - 19, color_2);
  tft.drawPixel(x - 12, y - 19, color_2);
  tft.drawPixel(x - 13, y - 19, color_2);
  tft.drawPixel(x - 14, y - 19, color_2);
  tft.drawPixel(x - 15, y - 19, color_2);
  tft.drawPixel(x - 16, y - 19, color_2);
}
