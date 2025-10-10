// Author: Ethan Larner. Project started: Sept 2022

/*

    TODO:

  Button navigation
  Adding a move that symbolises a draw "state"
  Display wins and draws in the notation
  50-move rule
  Testing and optimizations, particularly around the newly implemented updateSquare() function
  Three-fold repition rule. (Represent the board state as a 33-byte array and compare against the previous 50 board states: use implementation of 50-move rule)

  Version 3
  Chess AI
  
    IDEAS:
  Go back moves and then jump to where it is being played
  Play from here button

*/

// TFT screen used in project   : https://www.amazon.co.uk/Elegoo-EL-SM-004-Inches-Technical-Arduino/dp/B01EUVJYME
// Libraries are obtainable from: https://www.elegoo.com/en-gb/blogs/arduino-projects/elegoo-2-8-inch-touch-screen-for-raspberry-pi-manual?srsltid=AfmBOooPWvJi5iR3kjFEz30FoBpyVXjYLvpluqeob0fl5NJ7rkZVzApn
#include <Elegoo_GFX.h>     // Core graphics library
#include <Elegoo_TFTLCD.h>  // Hardware-specific library
#include <TouchScreen.h>    // Touch-screen library

#define LCD_CS A3  // Chip Select goes to Analog 3
#define LCD_CD A2  // Command/Data goes to Analog 2
#define LCD_WR A1  // LCD Write goes to Analog 1
#define LCD_RD A0  // LCD Read goes to Analog 0

#define SD_SCK_PIN 13

#define LCD_RESET A4  // Can alternately connect to reset pin

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define YP A3  // Must be an analog pin
#define XM A2  // Must be an analog pin
#define YM 9   // Can be a digital pin
#define XP 8   // Can be a digital pin

#define MINPRESSURE 5
constexpr int MAXPRESSURE = 1000;

// Some calibration numbers for the touchscreen screen, found through testing
constexpr int TS_MINX = 110;
constexpr int TS_MAXX = 910;

constexpr int TS_MINY = 70;
constexpr int TS_MAXY = 900;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);  // Setup the TouchScreen Object
TSPoint p;

constexpr int MILLIS_BETWEEN_PRESS = 250;  // The delay required between presses on the screen. Prevents double pressing the same spot accidently
unsigned long lastPress = millis();


// Assign human-readable names to some common 16-bit color values:
constexpr uint16_t BLACK = 0x0000;
constexpr uint16_t DARK_GRAY = 0x2924;
constexpr uint16_t DARK_EMERALD = 0x6c6e;
constexpr uint16_t LIGHT_EMERALD = 0xadf1;
constexpr uint16_t DARK_MARINE = 0x6b9a;
constexpr uint16_t LIGHT_MARINE = 0x749f;//0x9d5f;
constexpr uint16_t BLUE = 0x001F;
constexpr uint16_t RED = 0xF800;
constexpr uint16_t GREEN = 0x07E0;
constexpr uint16_t KHAKI = 0x84AD;
constexpr uint16_t CYAN = 0x07FF;
constexpr uint16_t MAGENTA = 0xF81F;
constexpr uint16_t YELLOW = 0xFFE0;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t DARK_BROWN = 0xB44C;
constexpr uint16_t LIGHT_BROWN = 0xEED6;
constexpr uint16_t DARK_OLIVE = 0x6368;
constexpr uint16_t OLIVE = 0xAD07;
constexpr uint16_t LIGHT_OLIVE = 0xCE8D;
constexpr uint16_t BRICK_RED = 0xC367;
constexpr uint16_t BEIGE_GREEN = 0xAD6F;

constexpr uint16_t NOTATION_BACK = DARK_EMERALD;
constexpr uint16_t NOTATION_FRONT = LIGHT_EMERALD;

// Board buffer
constexpr int BOARD_BUFFER = 80;
constexpr int SQUARE_SIZE = 30;



// Chess piece IDs
enum Piece : uint8_t {
  BLANK_SPACE = 0,  // Empty square
  WPAWN, WKNIGHT, WBISHOP, WROOK, WQUEEN, WKING,
  BPAWN, BKNIGHT, BBISHOP, BROOK, BQUEEN, BKING
};

struct ButtonState {
  byte up : 1;
  byte down : 1;
  byte left : 1;
  byte right : 1;
  byte : 4;
};

struct ScreenLocation {
  uint16_t x;
  uint16_t y;
};

struct Square {
  byte x : 3;      // 3-bit X coordinate (0-7)
  byte y : 3;      // 3-bit Y coordinate (0-7)
  byte promo : 2;  // 2-bit promotion type (only relevant if y == 0 or 7) ::: 00 = Knight, 01 = Bishop, 10 = Rook, 11 = Queen
};

struct Ply {
  struct Square from;
  struct Square to;  // 'to.promo' only matters if 'to.y' is 0 or 7 and the piece is a pawn
};

struct Move {
  struct Ply white;
  struct Ply black;
};

// board rotation variables
int8_t xRot = 1;
int8_t yRot = 1;
int8_t lineWidthMulitipiler = 0;
int8_t lineHeightMulitipiler = 0;

constexpr bool rotationOn = false;

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

Piece board[8][8] = {
  { BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK },
  { BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE },
  { WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN },
  { WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK }
};

// Bitboard overlay, where "1" indicates a filled in pixel, and "0" indicates a pixel left untouched. Use pgm_read_byte(&captureOverlay[x][y]) to load the bool
static const bool captureOverlay[SQUARE_SIZE][SQUARE_SIZE] PROGMEM = {
  { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
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
  { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 }
};

// Blank board used in move validation
Piece hypotheticalBoard[8][8];

constexpr uint16_t MAX_PLY = 512;
constexpr uint16_t MAX_LINES_PER_PANE = 25;
struct Ply moves[MAX_PLY];
uint16_t currentPlyNumber = 0;
uint16_t lastDrawnPlyNumber = 0;

uint8_t notationPaneNo = 0;

//                           white          black
// Rook movement flags     left  right   left  right
bool rookMovementFlags[4] = { false, false, false, false };
struct ButtonState states = { 0, 0, 0, 0 };

bool whiteKingHasMoved = false;
bool blackKingHasMoved = false;

bool turn = 0;  // 0 = white, 1 = black

bool selectingPiece = true;     // true for the first press, false for the second press
bool promotingPiece = false;    // Promotion menu activity indicator
bool cancelPromotion = false;  // Flag to cancel the promotion menu

bool castleAlert = false;   // Flag for the "castle" move
bool passantAlert = false;  // Flag for the "en passant" move

struct Ply selectedPly = { { 0, 0, 0 }, { 0, 0, 0 } };  // Coords for both squares for the current half-move
struct Ply previousPly = { { 0, 0, 0 }, { 0, 0, 0 } };  // Coords for both squares for the last half-move

// Setup the serial connection (if applicable), the tft interface, and then finally displays the initial chess board
void setup() {
  /*
  Serial.begin(9600);
  */

  #ifdef USE_Elegoo_SHIELD_PINOUT
    // Elegoo 2.8\" TFT Arduino Shield Pinout
  #else
    // Elegoo 2.8\" TFT Breakout Board Pinout
  #endif

  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {         // ILI9325 LCD driver
  } else if (identifier == 0x9328) {  // ILI9328 LCD driver
  } else if (identifier == 0x4535) {  // LGDP4535 LCD driver
  } else if (identifier == 0x7575) {  // HX8347G LCD driver
  } else if (identifier == 0x9341) {  // ILI9341 LCD driver
  } else if (identifier == 0x8357) {  // X8357D LCD driver
  } else if (identifier == 0x0101) {
    identifier = 0x9341;  // 0x9341 LCD driver
  } else {
    identifier = 0x9341;  // Unknown LCD driver chip
  }

  tft.begin(identifier);

  pinMode(SD_SCK_PIN, OUTPUT);

  tft.setRotation(1);
  tft.fillScreen(BLACK);

  tft.fillRect(0, 0, 80, 240, NOTATION_BACK);
  tft.drawRect(0, 0, 80, 240, NOTATION_FRONT);
  drawNotation();
  drawButtons();
  drawBoard(board);
}

// Main loop function
void loop() {
  digitalWrite(SD_SCK_PIN, HIGH);
  p = ts.getPoint();  // Poll the latest TSPoint instance
  digitalWrite(SD_SCK_PIN, LOW);

  // Confirm the direction of the pins

  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  
  // Check that the pressure is enough to be considered a press
  if (validPress(p.z)) {
    //Serial.print(p.x);Serial.print(" ");Serial.println(p.y);
    int p_x = 330 - map(p.y, TS_MINY, TS_MAXY, 0, 320);
    int p_y = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    //Serial.print(p_x);Serial.print(" ");Serial.println(p_y);
    if (validDelay()) {
      lastPress = millis();
      // sx,sy refer to the coords of the square related to the board; nx,ny refer to the promotion square if relevent
      if (pressedBoard(p_x, p_y)) {
        struct Square square = findSquareFromScreenPos(p_x, p_y);

        uint8_t sx = square.x;
        uint8_t sy = square.y;

        if (promotingPiece) {
          promotingPiece = false;
          uint8_t nx = sx;
          uint8_t ny = sy;
          uint16_t color;

          if (board[selectedPly.from.y][selectedPly.from.x] == WPAWN) {
            if ((selectedPly.to.x + selectedPly.to.y) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y);

            if ((selectedPly.to.x + selectedPly.to.y + 1) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y + 1][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y + 1);

            if ((selectedPly.to.x + selectedPly.to.y + 2) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y + 2][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y + 2);

            if ((selectedPly.to.x + selectedPly.to.y + 3) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y + 3][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y + 3);
          } else {
            if ((selectedPly.to.x + selectedPly.to.y) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y);

            if ((selectedPly.to.x + selectedPly.to.y - 1) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y - 1][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y - 1);

            if ((selectedPly.to.x + selectedPly.to.y - 2) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y - 2][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y - 2);

            if ((selectedPly.to.x + selectedPly.to.y - 3) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.to.y - 3][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y - 3);
          }


          // Interpretation of the promotion
          if (nx != selectedPly.to.x || (board[selectedPly.from.y][selectedPly.from.x] == WPAWN && ny >= 4) || (board[selectedPly.from.y][selectedPly.from.x] == BPAWN && ny <= 3)) {
            cancelPromotion = true; /* user did not press a square or pressed a square on the wrong column or wrong row */

          } else if (board[selectedPly.from.y][selectedPly.from.x] == WPAWN) {
            if (ny == 0) {
              board[selectedPly.from.y][selectedPly.from.x] = WQUEEN;  // Directly changes the pawn to the piece ; see first line after promote menu
              selectedPly.to.promo = 3;
            } else if (ny == selectedPly.to.y + 1) {
              board[selectedPly.from.y][selectedPly.from.x] = WROOK;
              selectedPly.to.promo = 2;
            } else if (ny == selectedPly.to.y + 2) {
              board[selectedPly.from.y][selectedPly.from.x] = WBISHOP;
              selectedPly.to.promo = 1;
            } else if (ny == selectedPly.to.y + 3) {
              board[selectedPly.from.y][selectedPly.from.x] = WKNIGHT;
              selectedPly.to.promo = 0;
            }
          } else if (board[selectedPly.from.y][selectedPly.from.x] == BPAWN) {
            if (ny == selectedPly.to.y) {
              board[selectedPly.from.y][selectedPly.from.x] = BQUEEN;  // Directly changes the pawn to the piece ; see first line after promote menu
              selectedPly.to.promo = 3;
            } else if (ny == selectedPly.to.y - 1) {
              board[selectedPly.from.y][selectedPly.from.x] = BROOK;
              selectedPly.to.promo = 2;
            } else if (ny == selectedPly.to.y - 2) {
              board[selectedPly.from.y][selectedPly.from.x] = BBISHOP;
              selectedPly.to.promo = 1;
            } else if (ny == selectedPly.to.y - 3) {
              board[selectedPly.from.y][selectedPly.from.x] = BKNIGHT;
              selectedPly.to.promo = 0;
            }
          }

          handleMove();

        } else if (selectingPiece) {  //----- First press on screen
          if (checkForGameOver(previousPly, board)) {
            // Game over?
          } else if (((board[sy][sx] < BPAWN && turn == 0) || (board[sy][sx] > WKING && turn == 1))) {
            selectedPly.from.x = sx;
            selectedPly.from.y = sy;
            if (!(board[selectedPly.from.y][selectedPly.from.x] == BLANK_SPACE)) {

              draw_possible_moves(previousPly, selectedPly, board);

              tft.fillRect(BOARD_BUFFER + sx * SQUARE_SIZE, sy * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, KHAKI);
              drawPiece(board[sy][sx], sx, sy);
              selectingPiece = false;
            }
          }

        } else {  //----- Second press on screen

          delete_possible_moves(previousPly, selectedPly, board);
          selectedPly.to.x = sx;
          selectedPly.to.y = sy;

          if (validMove(selectedPly.from.x, selectedPly.from.y, selectedPly.to.x, selectedPly.to.y, previousPly, board)) {
            if (checkAttemptedPromotion()) {
              displayPromotionMenu();
              promotingPiece = true;

            } else {
              handleMove();
            }

          } else {
            uint16_t color;
            if (checkForCheck(previousPly, board) && ((board[selectedPly.from.y][selectedPly.from.x] == BKING || board[selectedPly.from.y][selectedPly.from.x] == WKING))) {
              color = RED;
            } else if (selectedPly.from.x == previousPly.to.x && selectedPly.from.y == previousPly.to.y) {
              color = LIGHT_OLIVE;
            } else if ((selectedPly.from.x + selectedPly.from.y) % 2 == 1) {
              color = DARK_BROWN;
            } else {
              color = LIGHT_BROWN;
            }
            tft.fillRect(BOARD_BUFFER + (selectedPly.from.x) * SQUARE_SIZE, (selectedPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
            drawPiece(board[selectedPly.from.y][selectedPly.from.x], selectedPly.from.x, selectedPly.from.y);
          }
          selectingPiece = true;
        }
      } else if (pressedButton(p_x, p_y) > 0) {
        uint8_t button = pressedButton(p_x, p_y);

        if (button == 1) {
          states.up = true;
          drawButtons();
          if (notationPaneNo > 0) {
            notationPaneNo -= 1;
            drawNotation();
          }
        } else if (button == 2) {
          states.down = true;
          drawButtons();
          if (notationPaneNo < ((currentPlyNumber > 0 ? (currentPlyNumber - 1) / 2 : 0) )/MAX_LINES_PER_PANE) {
            notationPaneNo += 1;
            drawNotation();
          }
        }
      }
    } else if (pressedButton(p_x, p_y) > 0) {
    
      uint8_t button = pressedButton(p_x, p_y);

      if (button == 1) {
        states.up = true;
        
      } else if (button == 2) {
        states.down = true;
        
      }
    }

  } else {
    if (states.up && validDelay()) {
      states.up = false;
      drawUpButton(NOTATION_BACK, NOTATION_FRONT);
    }
    if (states.down && validDelay()) {
      states.down = false;
      drawDownButton(NOTATION_BACK, NOTATION_FRONT);
    }
  }
}

// Check that the given pressure is enough to be considered a press
bool validDelay() {
  return (millis() - MILLIS_BETWEEN_PRESS > lastPress);
}

bool validPress(int16_t z) {
  return (z > MINPRESSURE && z < MAXPRESSURE);
}

uint8_t pressedButton(uint16_t p_x, uint16_t p_y) {
  if(p_x>=22 && p_x<=57 && p_y>=208 && p_y<=221) {
    return 1; // Up button pressed
  }
  if (p_x>=22 && p_x<=57 && p_y>=223 && p_y<=236) {
    return 2; // Down button pressed
  }

  return 0; // Return 0 if no button pressed
}

// Check if the chess board has been pressed given the input coordinates
bool pressedBoard(int p_x, int p_y) {
  return p_x > BOARD_BUFFER;
}

// Finds the square on the chess board.    DOES NOT CHECK IF THE CHESS BOARD IS PRESSED -> will return 0,0 if invalid
struct Square findSquareFromScreenPos(int p_x, int p_y) {
  struct Square square = { 0, 0 };
  int x;
  int y;
  for (int sy = 0; sy < 8; sy += 1) {
    for (int sx = 0; sx < 8; sx += 1) {

      x = BOARD_BUFFER + sx * SQUARE_SIZE;
      y = sy * SQUARE_SIZE;
      if (x < p_x - 5 && y < p_y + 5 && x + SQUARE_SIZE > p_x - 5 && y + SQUARE_SIZE > p_y + 5) {
        square.x = sx;
        square.y = sy;
        break;
      }
    }
  }
  return square;
}

// Check if the move just played was a promotion
bool checkAttemptedPromotion() {
  return (board[selectedPly.from.y][selectedPly.from.x] == WPAWN && selectedPly.to.y == 0) || (board[selectedPly.from.y][selectedPly.from.x] == BPAWN && selectedPly.to.y == 7);
}

// Updates a square, without adding special colors
void updateSquare(Square square, Piece board[8][8]) {
  uint16_t color = ((square.x + square.y) % 2 == 1) ? DARK_BROWN : LIGHT_BROWN;
  tft.fillRect(BOARD_BUFFER + (square.x) * SQUARE_SIZE, (square.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);

  if (board[square.y][square.x] != BLANK_SPACE) {
    drawPiece(board[square.y][square.x], square.x, square.y);
  }
} 

// The handle move logic, which runs after the second square is pressed
void handleMove() {
  uint16_t color;

  if (!cancelPromotion) {
    //Check if previously was in check and save location if so
    struct Square prevKingLocation = {0,0,3};
    if (checkForCheck(previousPly, board)) {
      prevKingLocation = findKing(board);
    }

    turn = (turn == 0) ? 1 : 0;  // flip the turn value
    if (rotationOn) {
      flipRotation();
    }
    
    // rook move check  -  hard-coded values                           left white rook                                                                                                   right white rook                                                                                                                                    black left rook                                                                                                                              black right rook
    if (board[selectedPly.from.y][selectedPly.from.x] == WROOK && selectedPly.from.x == 0 && selectedPly.from.y == 7) {
      rookMovementFlags[0] = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == WROOK && selectedPly.from.x == 7 && selectedPly.from.y == 7) {
      rookMovementFlags[1] = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == BROOK && selectedPly.from.x == 0 && selectedPly.from.y == 0) {
      rookMovementFlags[2] = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == WROOK && selectedPly.from.x == 7 && selectedPly.from.y == 0) {
      rookMovementFlags[3] = true;
    }

    // king move check
    if (board[selectedPly.from.y][selectedPly.from.x] == WKING && selectedPly.from.x == 4 && selectedPly.from.y == 7) {
      whiteKingHasMoved = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == BKING && selectedPly.from.x == 4 && selectedPly.from.y == 0) {
      blackKingHasMoved = true;
    }

    board[selectedPly.to.y][selectedPly.to.x] = board[selectedPly.from.y][selectedPly.from.x];
    board[selectedPly.from.y][selectedPly.from.x] = BLANK_SPACE;



    if (passantAlert == true) {
      passantAlert = false;
      board[previousPly.to.y][previousPly.to.x] = BLANK_SPACE;

      // remove the en passanted piece in case the update board isn't called
      updateSquare(previousPly.to, board);
    }
    if (castleAlert) {
      castleAlert = false;
      if (board[selectedPly.to.y][selectedPly.to.x] == WKING) {
        if (selectedPly.from.x - selectedPly.to.x > 0) {
          board[7][0] = BLANK_SPACE;
          board[7][3] = WROOK;
          struct Square s1 = {0, 7};
          struct Square s2 = {3, 7};
          updateSquare(s1, board);
          updateSquare(s2, board);
        } else {
          board[7][7] = BLANK_SPACE;
          board[7][5] = WROOK;
          struct Square s1 = {7, 7};
          struct Square s2 = {5, 7};
          updateSquare(s1, board);
          updateSquare(s2, board);
        }
      } else if (selectedPly.from.x - selectedPly.to.x > 0) {
        board[0][0] = BLANK_SPACE;
        board[0][3] = BROOK;
        struct Square s1 = {0, 0};
        struct Square s2 = {3, 0};
        updateSquare(s1, board);
        updateSquare(s2, board);
      } else {
        board[0][7] = BLANK_SPACE;
        board[0][5] = BROOK;
        struct Square s1 = {7, 0};
        struct Square s2 = {5, 0};
        updateSquare(s1, board);
        updateSquare(s2, board);
      }
    }
    //drawBoard(board);

    // remove last move just moved colors
    if ((previousPly.from.x + previousPly.from.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (previousPly.from.x) * SQUARE_SIZE, (previousPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(board[previousPly.from.y][previousPly.from.x], previousPly.from.x, previousPly.from.y);
    if ((previousPly.to.x + previousPly.to.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (previousPly.to.x) * SQUARE_SIZE, (previousPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(board[previousPly.to.y][previousPly.to.x], previousPly.to.x, previousPly.to.y);


    // change squares to just moved colors
    tft.fillRect(BOARD_BUFFER + (selectedPly.from.x) * SQUARE_SIZE, (selectedPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, OLIVE);
    drawPiece(board[selectedPly.from.y][selectedPly.from.x], selectedPly.from.x, selectedPly.from.y);
    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, LIGHT_OLIVE);
    drawPiece(board[selectedPly.to.y][selectedPly.to.x], selectedPly.to.x, selectedPly.to.y);
    
    // Remove any 'check' color from before
    if (prevKingLocation.promo!=3 && !((prevKingLocation.x == selectedPly.from.x && prevKingLocation.y == selectedPly.from.y) || (prevKingLocation.x == selectedPly.to.x && prevKingLocation.y == selectedPly.to.y))) {// If it was check and we haven't already changed the square
      updateSquare(prevKingLocation, board);
    }
    if (MAX_PLY > currentPlyNumber) {
      moves[currentPlyNumber] = selectedPly;currentPlyNumber++;
    }
    previousPly = selectedPly;
    if (rotationOn) {
      updateBoard(previousPly, selectedPly, board);
    }
    

    // Add red effect if in 'check'
    if (checkForCheck(previousPly, board)) {
      struct Square kingLocation = findKing(board);

      tft.fillRect(BOARD_BUFFER + (kingLocation.x) * SQUARE_SIZE, (kingLocation.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);
      drawPiece(board[kingLocation.y][kingLocation.x], kingLocation.x, kingLocation.y);
    }



  } else {
    if (selectedPly.from.x == previousPly.to.x && selectedPly.from.y == previousPly.to.y) {
      color = LIGHT_OLIVE;
    } else if ((selectedPly.from.x + selectedPly.from.y) % 2 == 1) {
      color = DARK_BROWN;
    } else {
      color = LIGHT_BROWN;
    }
    tft.fillRect(BOARD_BUFFER + (selectedPly.from.x) * SQUARE_SIZE, (selectedPly.from.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, color);
    drawPiece(board[selectedPly.from.y][selectedPly.from.x], selectedPly.from.x, selectedPly.from.y);
    cancelPromotion = false;
  }

  if ((notationPaneNo+1)*MAX_LINES_PER_PANE*2<currentPlyNumber) {
    notationPaneNo+=1;
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
      passantAlert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && previousPly.from.y == 6 && previousPly.to.x == x2 && previousPly.to.y == 4 && board[previousPly.to.y][previousPly.to.x] == WPAWN) {
      passantAlert = true;
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
    //int whiteKingHasMoved = false;
    //int blackKingHasMoved = false;

  } else if (id == WKING) {
    // The king can move a maximum of one in any direction
    if (!(abs(x1 - x2) > 1 || abs(y1 - y2) > 1)) {
    } else if ((!whiteKingHasMoved) && abs(x1 - x2) == 2 && abs(y1 - y2) == 0) {
      if ((x1 - x2 > 0 && !rookMovementFlags[0] && board[7][3] == BLANK_SPACE && board[7][2] == BLANK_SPACE && board[7][1] == BLANK_SPACE) || (x1 - x2 < 0 && !rookMovementFlags[1] && board[7][5] == BLANK_SPACE)) {
        castleAlert = true;
      } else {
        return false;
      }
    } else {
      return false;
    }

  } else if (id == BKING) {
    // The king can move a maximum of one in any direction
    if (!(abs(x1 - x2) > 1 || abs(y1 - y2) > 1)) {
    } else if ((!blackKingHasMoved) && abs(x1 - x2) == 2 && abs(y1 - y2) == 0) {
      if ((x1 - x2 > 0 && !rookMovementFlags[2] && board[0][3] == BLANK_SPACE && board[0][2] == BLANK_SPACE && board[0][1] == BLANK_SPACE) || (x1 - x2 < 0 && !rookMovementFlags[3] && board[0][5] == BLANK_SPACE)) {
        castleAlert = true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  // check if king is capturable after move


  if (castleAlert) {
    if (checkForCheck(previousPly, board)) { return false; }  // starts in check
    if (x1 - x2 > 0) {
      updateHypothetical(board);
      hypotheticalBoard[y2][x2 + 1] = hypotheticalBoard[y1][x1];
      hypotheticalBoard[y1][x1] = BLANK_SPACE;
      if (checkForCheck(previousPly, hypotheticalBoard)) { return false; }  /// passes through check
    } else if (x1 - x2 < 0) {
      updateHypothetical(board);
      hypotheticalBoard[y2][x2 - 1] = hypotheticalBoard[y1][x1];
      hypotheticalBoard[y1][x1] = BLANK_SPACE;
      if (checkForCheck(previousPly, hypotheticalBoard)) { return false; }
    }
  }
  updateHypothetical(board);

  //memcpy(hypotheticalBoard, board, 8);
  hypotheticalBoard[y2][x2] = hypotheticalBoard[y1][x1];
  hypotheticalBoard[y1][x1] = BLANK_SPACE;
  if (passantAlert == true) {
    hypotheticalBoard[previousPly.to.y][previousPly.to.x] = BLANK_SPACE;
  }



  int kx = 0;
  int ky = 0;

  if (hypotheticalBoard[y2][x2] < BPAWN) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {

        if (hypotheticalBoard[y][x] == WKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypotheticalBoard[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypotheticalBoard)) {
            passantAlert = false;
            castleAlert = false;
            return false;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypotheticalBoard[y][x] == BKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             BLACK
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypotheticalBoard[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypotheticalBoard)) {
            passantAlert = false;
            castleAlert = false;
            return false;
          }
        }
      }
    }
  }



  return true;  // return true if all checks say the move could be valid
}

// Checks if the king is in check and if so, updates the "castleAlert" flag as relevant
bool checkForCheck(Ply previousPly, Piece board[8][8]) {
  updateHypothetical(board);
  int kx = 0;
  int ky = 0;

  if (turn == 0) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {

        if (hypotheticalBoard[y][x] == WKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypotheticalBoard[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypotheticalBoard)) {
            castleAlert = false;
            return true;
          }
        }
      }
    }
  } else {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypotheticalBoard[y][x] == BKING) {  // locate king
          kx = x;
          ky = y;
        }
      }  //                                                             BLACK
    }

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypotheticalBoard[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypotheticalBoard)) {
            castleAlert = false;
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
      passantAlert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && previousPly.from.y == 6 && previousPly.to.x == x2 && previousPly.to.y == 4 && board[previousPly.to.y][previousPly.to.x] == WPAWN) {
      passantAlert = true;
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
      hypotheticalBoard[y][x] = from[y][x];
    }
  }
}

// Finds the king on the given board and updates the global king position
struct Square findKing(Piece board[8][8]) {
  uint8_t king = (turn == 0) ? WKING : BKING;  // Set the king's piece based on the turn
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      if (board[y][x] == king) {
        struct Square kingLocation = {x, y, 0};
        return kingLocation;  // Return early once the king is found
      }
    }
  }
}

uint16_t countPossibleMoves(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  uint16_t numberOfMoves = 0;
  bool passantBefore = passantAlert;
  bool castleBefore = castleAlert;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (validMove(selectedPly.from.x, selectedPly.from.y, x, y, previousPly, board)) {
        numberOfMoves++;
      }
    }
  }
  passantAlert = passantBefore;
  castleAlert = castleBefore;
  return numberOfMoves;
}

uint16_t countAllPossibleMoves(Ply previousPly, bool turn, Piece board[8][8]) {
  uint16_t numberOfMoves = 0;
  bool passantBefore = passantAlert;
  bool castleBefore = castleAlert;

  for (int y1 = 0;y1<8;y1++) {
    for (int x1 = 0;x1<8;x1++) {
      if (( turn == 0 && (board[y1][x1] > WKING || board[y1][x1] == BLANK_SPACE) ) || ( turn == 1 && board[y1][x1] < BPAWN)) {
        continue;
      }

      for (int y2 = 0; y2 < 8; y2++) {
        for (int x2 = 0; x2 < 8; x2++) {
          if (validMove(x1, y1, x2, y2, previousPly, board)) {
            numberOfMoves++;
          }
        }
      }
    }
  }
  passantAlert = passantBefore;
  castleAlert = castleBefore;
  return numberOfMoves;
}

// Creates an array of squares which contains a piece of the target type that can legally move to the target square. End square has "3" in the promo position instead of "0"
void similarPieces(Ply previousPly, Square targetSquare, Piece targetType, Piece board[8][8], Square squares[9]) {
  uint8_t nextSquare = 0;


  bool passantBefore = passantAlert;
  bool castleBefore = castleAlert;

  for (uint8_t y1 = 0;y1<8;y1++) {
    for (uint8_t x1 = 0;x1<8;x1++) {
      if (board[y1][x1] == targetType) {
        if (validMove(x1, y1, targetSquare.x, targetSquare.y, previousPly, board)) {
          struct Square square = {x1, y1, 0};
          squares[nextSquare] = square;
          nextSquare++;
        }
      }
      
    }
  }

  struct Square end = {0, 0, 3};
  squares[nextSquare] = end;

  passantAlert = passantBefore;
  castleAlert = castleBefore;
}


bool checkForCheckmate(Ply lastPly, Piece board[8][8]) {
  //findKing(board);
  if (countAllPossibleMoves(lastPly, turn, board) == 0 && checkForCheck(lastPly, board)) {
    return true;
  }
  return false;
}
bool checkForDraw(Ply lastPly, Piece board[8][8]) {
  if (countAllPossibleMoves(lastPly, turn, board) == 0) {
    return true;
  } // 50-move rule, three-fold repetition should be added
  return false;
}
bool checkForGameOver(Ply lastPly, Piece board[8][8]) {
  if (checkForDraw(lastPly, board) || checkForCheckmate(lastPly, board)) {
    return true;
  }
  return false;
}

// draw / delete the indicators on the squares the selected piece can move to
void draw_possible_moves(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  uint16_t color;

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
          int acc = 0;
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
  passantAlert = false;
  castleAlert = false;
}

void delete_possible_moves(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  uint16_t color;

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
  passantAlert = false;
  castleAlert = false;
}

// Displays a promotion menu on screen. DOES NOT CHECK IF PROMOTION IS VALID
void displayPromotionMenu() {
  if (board[selectedPly.from.y][selectedPly.from.x] == WPAWN) {
    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WQUEEN, selectedPly.to.x, selectedPly.to.y);

    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WROOK, selectedPly.to.x, selectedPly.to.y + 1);

    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WBISHOP, selectedPly.to.x, selectedPly.to.y + 2);

    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y + 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(WKNIGHT, selectedPly.to.x, selectedPly.to.y + 3);
  } else {
    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BQUEEN, selectedPly.to.x, selectedPly.to.y);

    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 1) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BROOK, selectedPly.to.x, selectedPly.to.y - 1);

    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 2) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BBISHOP, selectedPly.to.x, selectedPly.to.y - 2);

    tft.fillRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BRICK_RED);
    tft.drawRect(BOARD_BUFFER + (selectedPly.to.x) * SQUARE_SIZE, (selectedPly.to.y - 3) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
    drawPiece(BKNIGHT, selectedPly.to.x, selectedPly.to.y - 3);
  }
}

// Function to display all the characters on the screen, currently supported by the drawCharacter() function, for easy testing
void _testNotation() {
  uint16_t lineCount = 0;
  uint16_t colCount = 0;
  for (int i = 0;i<200;i++) {
    colCount += drawCharacter(2+colCount, 5 + 6*lineCount, NOTATION_FRONT, 35+i);
    colCount++;
    if (colCount+5>75) {
      colCount = 0;
      lineCount++;
    }
  }
}

// Plays a ply on the given board, updating flags but not updating the screen
void playMove(Ply ply, Piece from[8][8]) {
  if (from[ply.from.y][ply.from.x] == WROOK && ply.from.x == 0 && ply.from.y == 7) {
    rookMovementFlags[0] = true;
  } else if (from[ply.from.y][ply.from.x] == WROOK && ply.from.x == 7 && ply.from.y == 7) {
    rookMovementFlags[1] = true;
  } else if (from[ply.from.y][ply.from.x] == BROOK && ply.from.x == 0 && ply.from.y == 0) {
    rookMovementFlags[2] = true;
  } else if (from[ply.from.y][ply.from.x] == WROOK && ply.from.x == 7 && ply.from.y == 0) {
    rookMovementFlags[3] = true;
  }

  // king move check
  if (from[ply.from.y][ply.from.x] == WKING && ply.from.x == 4 && ply.from.y == 7) {
    whiteKingHasMoved = true;
  } else if (from[ply.from.y][ply.from.x] == BKING && ply.from.x == 4 && ply.from.y == 0) {
    blackKingHasMoved = true;
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



  if (passantAlert == true) {
    passantAlert = false;
    from[previousPly.to.y][previousPly.to.x] = BLANK_SPACE;
  }
  if (castleAlert) {
    castleAlert = false;
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

// Big notation function, implementing all the complex rules of chess notation (castling, en passant, minimal move disambiguation)
void updateNotation() {
  

  if (currentPlyNumber==0) {
    return;
  }

  // Save the state of any flags, so that they can be used by this function
  bool rookFlags[4];rookFlags[0] = rookMovementFlags[0];rookFlags[1] = rookMovementFlags[1];rookFlags[2] = rookMovementFlags[2];rookFlags[3] = rookMovementFlags[3];
  bool passantFlag = passantAlert;
  bool castleFlag = castleAlert;
  bool whiteFlag = whiteKingHasMoved;
  bool blackFlag = blackKingHasMoved;
  bool turnFlag = turn;

  // default all flags
  rookMovementFlags[0] = false;rookMovementFlags[1] = false;rookMovementFlags[2] = false;rookMovementFlags[3] = false;
  whiteKingHasMoved = false;blackKingHasMoved = false;turn = 0;castleAlert = false;passantAlert = false;  

  Piece notationBoard[8][8];
  fillBoard(notationBoard);

  uint16_t currentRow = 5;
  uint16_t currentCol = 2;

  if (notationPaneNo > 0) {
    for (uint16_t plyIndex = 0; plyIndex<MAX_LINES_PER_PANE * 2 * (notationPaneNo); plyIndex++) {
      struct Ply lastPly;
      if (plyIndex > 0) {lastPly=moves[plyIndex-1];}
      validMove(moves[plyIndex].from.x, moves[plyIndex].from.y, moves[plyIndex].to.x, moves[plyIndex].to.y, lastPly, notationBoard);
      playMove(moves[plyIndex], notationBoard);
      turn = (turn == 0) ? 1 : 0;
    }
  }

  for (uint16_t plyIndex = MAX_LINES_PER_PANE * 2 * notationPaneNo; plyIndex<MAX_LINES_PER_PANE * 2 * (notationPaneNo+1); plyIndex+=2) {
    uint16_t moveNumber = plyIndex/2 + 1;
    const uint16_t moveIndex = plyIndex/2;
    struct Ply lastPly;if (plyIndex > 0) {lastPly=moves[plyIndex-1];}
    
    // Skip drawing if already drawn
    if (lastDrawnPlyNumber>=plyIndex+1) {
      struct Ply currentPly = moves[plyIndex];
      validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
      playMove(currentPly, notationBoard);
      turn = (turn == 0) ? 1 : 0;

      lastPly = moves[plyIndex];
      currentPly = moves[plyIndex+1];

      validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
      playMove(currentPly, notationBoard);
      turn = (turn == 0) ? 1 : 0;

      currentRow+=8;
      currentCol =2;

      continue;
    }

    if (plyIndex > currentPlyNumber-1 && !checkForDraw(lastPly, notationBoard)) { // Break if reached last move and game has not ended
      break;
    }

    //hundreds
    if (moveNumber>=900) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 57) + 1;
      moveNumber-=900;
    } else if (moveNumber>=800) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 56) + 1;
      moveNumber-=800;
    } else if (moveNumber>=700) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 55) + 1;
      moveNumber-=700;
    } else if (moveNumber>=600) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 54) + 1;
      moveNumber-=600;
    } else if (moveNumber>=500) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 53) + 1;
      moveNumber-=500;
    } else if (moveNumber>=400) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 52) + 1;
      moveNumber-=400;
    } else if (moveNumber>=300) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 51) + 1;
      moveNumber-=300;
    } else if (moveNumber>=200) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 50) + 1;
      moveNumber-=200;
    } else if (moveNumber>=100) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 49) + 1;
      moveNumber-=100;
    } else if (moveIndex+1>999) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 48) + 1;
    } else {currentCol+=4;}

    //tens
    if (moveNumber>=90) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 57) + 1;
      moveNumber-=90;
    } else if (moveNumber>=80) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 56) + 1;
      moveNumber-=80;
    } else if (moveNumber>=70) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 55) + 1;
      moveNumber-=70;
    } else if (moveNumber>=60) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 54) + 1;
      moveNumber-=60;
    } else if (moveNumber>=50) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 53) + 1;
      moveNumber-=50;
    } else if (moveNumber>=40) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 52) + 1;
      moveNumber-=40;
    } else if (moveNumber>=30) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 51) + 1;
      moveNumber-=30;
    } else if (moveNumber>=20) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 50) + 1;
      moveNumber-=20;
    } else if (moveNumber>=10) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 49) + 1;
      moveNumber-=10;
    } else if (moveIndex+1>99) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 48) + 1;
    } else {currentCol+=4;}

    //ones
    if (moveNumber>=9) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 57) + 1;
      moveNumber-=9;
    } else if (moveNumber>=8) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 56) + 1;
      moveNumber-=8;
    } else if (moveNumber>=7) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 55) + 1;
      moveNumber-=7;
    } else if (moveNumber>=6) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 54) + 1;
      moveNumber-=6;
    } else if (moveNumber>=5) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 53) + 1;
      moveNumber-=5;
    } else if (moveNumber>=4) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 52) + 1;
      moveNumber-=4;
    } else if (moveNumber>=3) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 51) + 1;
      moveNumber-=3;
    } else if (moveNumber>=2) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 50) + 1;
      moveNumber-=2;
    } else if (moveNumber>=1) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 49) + 1;
      moveNumber-=1;
    } else if (moveIndex+1>9) {
      currentCol+= drawCharacter(currentCol, currentRow, NOTATION_FRONT, 48) + 1;
    } else {currentCol+=4;}

    currentCol=14;
    currentCol+=drawCharacter(currentCol, currentRow, NOTATION_FRONT, 46) + 1;

    if (plyIndex > currentPlyNumber-1) {// Add draw symbol if state is draw
      if (checkForCheckmate(lastPly, notationBoard)) {
        uint8_t plyChars[9] = {48,45,49,0,0,0,0,0,0};
        for (uint8_t i = 0;i<3;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
      } else if (checkForDraw(lastPly, notationBoard)) {
        uint8_t plyChars[9] = {49,47,50,45,49,47,50,0,0};
        for (uint8_t i = 0;i<7;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
        //lastDrawnPlyNumber = plyIndex;
      }
      break;
    }

    struct Ply currentPly = moves[plyIndex];
    uint8_t plyCharsIndex = 0;
    uint8_t plyChars[9] = {0,0,0,0,0,0,0,0,0};


    if (!(notationBoard[currentPly.from.y][currentPly.from.x] == WKING && abs(currentPly.from.x-currentPly.to.x) == 2)) { // If not castling
      //Piece symbol
      if (notationBoard[currentPly.from.y][currentPly.from.x] == WKING) {
        plyChars[plyCharsIndex] = 75;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WQUEEN) {
        plyChars[plyCharsIndex] = 81;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WBISHOP) {
        plyChars[plyCharsIndex] = 66;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WKNIGHT) {
        plyChars[plyCharsIndex] = 78;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WROOK) {
        plyChars[plyCharsIndex] = 82;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == WPAWN ) {
        if (notationBoard[currentPly.to.y][currentPly.to.x] > WKING || currentPly.from.x!=currentPly.to.x) {// only add file if it is a capture for pawns (include en passant)
          plyChars[plyCharsIndex] = 97+currentPly.from.x;plyCharsIndex++;
        }
      }

      // Move disambiguation (Add the starting rank, file, or both for disambiguation)
      if (notationBoard[currentPly.from.y][currentPly.from.x] > WPAWN && notationBoard[currentPly.from.y][currentPly.from.x] < WKING) {
        struct Square similarSquares[9];
        similarPieces(lastPly, currentPly.to, notationBoard[currentPly.from.y][currentPly.from.x], notationBoard, similarSquares); // fills the 'similarSquares' with the squares of pieces that can move to the target
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
          plyChars[plyCharsIndex] = 97+currentPly.from.x;plyCharsIndex++;
        }
        if (diffRank) {
        plyChars[plyCharsIndex] = 56-currentPly.from.y;plyCharsIndex++;
        }
      }

      // Add the 'x' for captures
      if (notationBoard[currentPly.to.y][currentPly.to.x] > WKING || (notationBoard[currentPly.from.y][currentPly.from.x] == WPAWN && currentPly.from.x!=currentPly.to.x)) {
          plyChars[plyCharsIndex] = 120;plyCharsIndex++;
      }

      // Add destination square
      plyChars[plyCharsIndex] = 97+currentPly.to.x;plyCharsIndex++;
      plyChars[plyCharsIndex] = 56-currentPly.to.y;plyCharsIndex++;

      // Promotion
      if (notationBoard[currentPly.from.y][currentPly.from.x] == WPAWN && currentPly.to.y == 0) {
        plyChars[plyCharsIndex] = 61;plyCharsIndex++; // =
        if (currentPly.to.promo == 0) {
          plyChars[plyCharsIndex] = 78;plyCharsIndex++; // Knight
        } else if (currentPly.to.promo == 1) {
          plyChars[plyCharsIndex] = 66;plyCharsIndex++; // Bishop
        } else if (currentPly.to.promo == 2) {
          plyChars[plyCharsIndex] = 82;plyCharsIndex++; // Rook
        } else if (currentPly.to.promo == 3) {
          plyChars[plyCharsIndex] = 81;plyCharsIndex++; // Queen
        } 
      }
    } else { // castling
      plyChars[plyCharsIndex] = 79;plyCharsIndex++;
      plyChars[plyCharsIndex] = 45;plyCharsIndex++;
      plyChars[plyCharsIndex] = 79;plyCharsIndex++;
      if (currentPly.to.x == 2) {
        plyChars[plyCharsIndex] = 45;plyCharsIndex++;
        plyChars[plyCharsIndex] = 79;plyCharsIndex++;
      }
    }
    validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
    playMove(currentPly, notationBoard);
    turn = (turn == 0) ? 1 : 0;

    // Add check as required
    if (checkForCheckmate(lastPly, notationBoard)) {
      plyChars[plyCharsIndex] = 35;plyCharsIndex++; // #
    } else if (checkForCheck(lastPly, notationBoard)) {
      plyChars[plyCharsIndex] = 43;plyCharsIndex++; // +
    }

    

    if (lastDrawnPlyNumber == 0 || lastDrawnPlyNumber<plyIndex) {
      for (uint8_t i = 0;i<plyCharsIndex;i++) {
        currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
      }
      lastDrawnPlyNumber = plyIndex;
    }

    currentCol = 48;

    if (plyIndex+1 > currentPlyNumber-1) {// Add draw symbol if state is draw
      if (checkForCheckmate(lastPly, notationBoard)) {
        uint8_t plyChars[9] = {49,45,48,0,0,0,0,0,0};
        for (uint8_t i = 0;i<3;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
      } else if (checkForDraw(lastPly, notationBoard)) {
        uint8_t plyChars[9] = {49,47,50,45,49,47,50,0,0};
        for (uint8_t i = 0;i<7;i++) {
          currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
        }
        //lastDrawnPlyNumber = plyIndex;
      }
      break;
    }

    
    plyCharsIndex = 0;
    plyChars[0]=0;plyChars[1]=0;plyChars[2]=0;plyChars[3]=0;plyChars[4]=0;plyChars[5]=0;plyChars[6]=0;plyChars[7]=0;plyChars[8]=0;
    lastPly = moves[plyIndex];
    currentPly = moves[plyIndex+1];
    if (!(notationBoard[currentPly.from.y][currentPly.from.x] == BKING && abs(currentPly.from.x-currentPly.to.x) == 2)) {
      //Piece symbol
      if (notationBoard[currentPly.from.y][currentPly.from.x] == BKING) {
        plyChars[plyCharsIndex] = 75;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BQUEEN) {
        plyChars[plyCharsIndex] = 81;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BBISHOP) {
        plyChars[plyCharsIndex] = 66;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BKNIGHT) {
        plyChars[plyCharsIndex] = 78;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BROOK) {
        plyChars[plyCharsIndex] = 82;plyCharsIndex++;
      } else if (notationBoard[currentPly.from.y][currentPly.from.x] == BPAWN) {
        if ((notationBoard[currentPly.to.y][currentPly.to.x] < BPAWN && notationBoard[currentPly.to.y][currentPly.to.x] != BLANK_SPACE) || (currentPly.from.x!=currentPly.to.x)) {// only add file if it is a capture for pawns
          plyChars[plyCharsIndex] = 97+currentPly.from.x;plyCharsIndex++;
        }
      }

      // Move disambiguation (Add the starting rank, file, or both for disambiguation)
      if (notationBoard[currentPly.from.y][currentPly.from.x] > BPAWN && notationBoard[currentPly.from.y][currentPly.from.x] < BKING) {
        struct Square similarSquares[9];
        similarPieces(lastPly, currentPly.to, notationBoard[currentPly.from.y][currentPly.from.x], notationBoard, similarSquares); // fills the 'similarSquares' with the squares of pieces that can move to the target
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
          plyChars[plyCharsIndex] = 97+currentPly.from.x;plyCharsIndex++;
        }
        if (diffRank) {
        plyChars[plyCharsIndex] = 56-currentPly.from.y;plyCharsIndex++;
        }
      }

      // Add the 'x' for captures
      if ((notationBoard[currentPly.to.y][currentPly.to.x] < BPAWN && notationBoard[currentPly.to.y][currentPly.to.x] != BLANK_SPACE) || (notationBoard[currentPly.from.y][currentPly.from.x] == BPAWN && currentPly.from.x!=currentPly.to.x)) {
          plyChars[plyCharsIndex] = 120;plyCharsIndex++;
      }

      // Add destination square
      plyChars[plyCharsIndex] = 97+currentPly.to.x;plyCharsIndex++;
      plyChars[plyCharsIndex] = 56-currentPly.to.y;plyCharsIndex++;

      // Promotion
      if (notationBoard[currentPly.from.y][currentPly.from.x] == BPAWN && currentPly.to.y == 7) {
        plyChars[plyCharsIndex] = 61;plyCharsIndex++; // =
        if (currentPly.to.promo == 0) {
          plyChars[plyCharsIndex] = 78;plyCharsIndex++; // Knight
        } else if (currentPly.to.promo == 1) {
          plyChars[plyCharsIndex] = 66;plyCharsIndex++; // Bishop
        } else if (currentPly.to.promo == 2) {
          plyChars[plyCharsIndex] = 82;plyCharsIndex++; // Rook
        } else if (currentPly.to.promo == 3) {
          plyChars[plyCharsIndex] = 81;plyCharsIndex++; // Queen
        } 
      }
    } else { // castling
      plyChars[plyCharsIndex] = 79;plyCharsIndex++;
      plyChars[plyCharsIndex] = 45;plyCharsIndex++;
      plyChars[plyCharsIndex] = 79;plyCharsIndex++;
      if (currentPly.to.x == 2) {
        plyChars[plyCharsIndex] = 45;plyCharsIndex++;
        plyChars[plyCharsIndex] = 79;plyCharsIndex++;
      }
    }
    validMove(currentPly.from.x, currentPly.from.y, currentPly.to.x, currentPly.to.y, lastPly, notationBoard);
    playMove(currentPly, notationBoard);
    turn = (turn == 0) ? 1 : 0;

    // Add check as required
    if (checkForCheckmate(lastPly, notationBoard)) {
      plyChars[plyCharsIndex] = 35;plyCharsIndex++; // #
    } else if (checkForCheck(lastPly, notationBoard)) {
      plyChars[plyCharsIndex] = 43;plyCharsIndex++; // +
    }

    if (lastDrawnPlyNumber<plyIndex+1) {
      for (uint8_t i = 0;i<plyCharsIndex;i++) {
        currentCol += drawCharacter(currentCol, currentRow, NOTATION_FRONT, plyChars[i]) + 1;
      }
      lastDrawnPlyNumber = plyIndex;
    }

    currentRow+=8;
    currentCol =2;
  }

  // reset flags
  rookMovementFlags[0] = rookFlags[0]; rookMovementFlags[1] = rookFlags[1];rookMovementFlags[2] = rookFlags[2];rookMovementFlags[3] = rookFlags[3];
  passantAlert = passantFlag;
  castleAlert = castleFlag;
  whiteKingHasMoved = whiteFlag;
  blackKingHasMoved = blackFlag;
  turn = turnFlag;
}

// Draws the clears and updates the notation
void drawNotation() {
  tft.fillRect(0, 0, 80, 206, NOTATION_BACK);
  tft.drawRect(0, 0, 80, 206, NOTATION_FRONT);
  //tft.drawFastHLine(0, 205, 80, NOTATION_FRONT);
  lastDrawnPlyNumber = 0;
  updateNotation();
}
// Fills a board array with pieces from the initialBoard, stored in PROGMEM
void fillBoard(Piece from[8][8]) {
  for (int y = 0;y<8;y++) {
    for (int x = 0;x<8;x++) {
      from[y][x] = pgm_read_byte(&initialBoard[y][x]);
    }
  }
}

// Takes in the (x,y) location of the upper left corner of the character, the color and the ASCII code of the character to draw on the screen
int16_t drawCharacter(uint16_t x, uint16_t y, uint16_t color, uint8_t code) {
  switch (code) {
    case 35:  // # (in ASCII)
      tft.drawFastVLine(x+1, y, 5, color);
      tft.drawFastVLine(x+3, y, 5, color);
      tft.drawFastHLine(x, y+1, 5, color);
      tft.drawFastHLine(x, y+3, 5, color);
      return 5;
    case 43:  // +
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastVLine(x+1, y+1, 3, color);
      return 3;
    case 45:  // -
      tft.drawFastHLine(x, y+2, 2, color);
      return 2;
    case 46:  // .
      tft.drawPixel(x, y+4, color);
      return 2;
    case 47:  // /
      tft.drawPixel(x+2, y, color);
      tft.drawFastVLine(x+1, y+1, 3, color);
      tft.drawPixel(x, y+4, color);
      return 3;
    case 48:  // 0
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x + 2, y, 5, color);
      tft.drawPixel(x + 1, y,color);
      tft.drawPixel(x + 1, y+4, color);
      return 3;
    case 49:  // 1
      tft.drawFastVLine(x + 1, y, 4, color);
      tft.drawPixel(x, y + 1, color);
      tft.drawFastHLine(x, y+4, 3, color);
      return 3;
    case 50:  // 2
      tft.drawFastHLine(x, y, 3, color);
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastHLine(x, y+4, 3, color);
      tft.drawPixel(x+2, y+1, color);
      tft.drawPixel(x, y+3, color);
      return 3;
    case 51:  // 3
      tft.drawFastHLine(x, y, 2, color);
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawFastHLine(x, y+4, 2, color);
      tft.drawPixel(x+1,y+2, color);
      return 3;
    case 52:  // 4
      tft.drawFastVLine(x, y, 3, color);
      tft.drawFastVLine(x + 2, y, 5, color);
      tft.drawPixel(x + 1, y + 2, color);
      return 3;
    case 53:  // 5
      tft.drawFastHLine(x, y, 3, color);
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastHLine(x, y+4, 3, color);
      tft.drawPixel(x, y+1, color);
      tft.drawPixel(x+2, y+3, color);
      return 3;
    case 54:  // 6
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastHLine(x + 1, y, 2, color);
      tft.drawFastVLine(x + 2, y + 2, 3, color);
      tft.drawPixel(x + 1, y + 2, color);
      tft.drawPixel(x + 1, y + 4, color);
      return 3;
    case 55:  // 7
      tft.drawFastHLine(x, y, 3, color);
      tft.drawPixel(x + 2, y + 1, color);
      tft.drawFastVLine(x + 1, y + 2, 3, color);
      return 3;
    case 56:  // 8
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawPixel(x+1, y, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case 57:  // 9
      tft.drawFastVLine(x, y, 3, color);
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawPixel(x+1, y, color);
      tft.drawPixel(x+1, y+2, color);
      return 3;
    case 60:  // <
      tft.drawFastHLine(x+1, y+1, 2, color);
      tft.drawFastHLine(x, y+2, 2, color);
      tft.drawFastHLine(x+1, y+3, 2, color);
      return 3;
    case 61:  // =
      tft.drawFastHLine(x, y+1, 3, color);
      tft.drawFastHLine(x, y+3, 3, color);
      return 3;
    case 62:  // >
      tft.drawFastHLine(x, y+1, 2, color);
      tft.drawFastHLine(x+1, y+2, 2, color);
      tft.drawFastHLine(x, y+3, 2, color);
      return 3;
    case 66:  // B
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastHLine(x+1, y, 2, color);
      tft.drawFastHLine(x+1, y+2, 2, color);
      tft.drawFastHLine(x+1, y+4, 2, color);
      tft.drawPixel(x+3, y+1, color);
      tft.drawPixel(x+3, y+3, color);
      return 4;
    case 75:  // K 
      tft.drawFastVLine(x, y, 5, color);
      tft.drawPixel(x+3, y, color);
      tft.drawPixel(x+2, y+1, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+2, y+3, color);
      tft.drawPixel(x+3, y+4, color);
      return 4;
    case 78:  // N
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+1, y+1, 2, color);
      tft.drawFastVLine(x+2, y+2, 2, color);
      tft.drawFastVLine(x+3, y, 5, color);
      return 4;
    case 79:  // O
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+3, y, 5, color);
      tft.drawFastHLine(x+1, y, 2, color);
      tft.drawFastHLine(x+1, y+4, 2, color);
      return 4;
    case 81:  // Q
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+3, y, 5, color);
      tft.drawFastHLine(x+1, y, 2, color);
      tft.drawPixel(x+1, y+4, color);
      tft.drawFastVLine(x+2, y+3, 3, color);
      return 4;
    case 82:  // R
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastHLine(x+1, y, 3, color);
      tft.drawFastHLine(x+1, y+2, 3, color);
      tft.drawPixel(x+3, y+1, color);
      tft.drawPixel(x+2, y+3, color);
      tft.drawPixel(x+3, y+4, color);
      return 4;
    case 94:  // ^
      tft.drawFastVLine(x, y+2, 2, color);
      tft.drawFastVLine(x+1, y+1, 2, color);
      tft.drawFastVLine(x+2, y+2, 2, color);
      return 3;
    case 97:  // a
      tft.drawFastVLine(x, y+2, 3, color);
      tft.drawFastVLine(x+2, y+1, 4, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case 98:  // b
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+2, y+2, 3, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case 99:  // c
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawFastHLine(x, y+4, 3, color);
      tft.drawPixel(x, y+3, color);
      return 3;
    case 100: // d
      tft.drawFastVLine(x+2, y, 5, color);
      tft.drawFastVLine(x, y+2, 3, color);
      tft.drawPixel(x+1, y+2, color);
      tft.drawPixel(x+1, y+4, color);
      return 3;
    case 101: // e
      tft.drawFastVLine(x, y+1, 3, color);
      tft.drawFastHLine(x+1, y+4, 2, color);
      tft.drawFastHLine(x+1, y+2, 2, color);
      tft.drawPixel(x+1, y, color);
      tft.drawPixel(x+2, y+1, color);
      return 3;
    case 102: // f
      tft.drawFastVLine(x+1, y, 5, color);
      tft.drawFastHLine(x, y+2, 3, color);
      tft.drawPixel(x+2, y, color);
      return 3;
    case 103: // g
      tft.drawFastVLine(x, y+1, 3, color);
      tft.drawFastVLine(x+2, y+2, 4, color);
      tft.drawFastHLine(x, y+5, 2, color);
      tft.drawPixel(x+1, y+1, color);
      tft.drawPixel(x+1, y+3, color);
      return 3;
    case 104: // h
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+2, y+2, 3, color);
      tft.drawPixel(x+1, y+2, color);
      return 3;
    case 118: // v
      tft.drawFastVLine(x, y+1, 2, color);
      tft.drawFastVLine(x+1, y+2, 2, color);
      tft.drawFastVLine(x+2, y+1, 2, color);
      return 3;
    case 120: // x
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

// Draws a chess piece at the (x,y) location
void drawPiece(Piece id, int x, int y) {
  
  const int buffer = (turn == 0 || !rotationOn) ? 5 : 24;  // Buffer to center the piece on its square on the board
  
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

// Flips the rotation variables to rotate the pieces on the board
void flipRotation() {
  xRot *= -1;
  yRot *= -1;
  lineWidthMulitipiler = (xRot == 1) ? 0 : 1;
  lineHeightMulitipiler = (yRot == 1) ? 0 : 1;
}

// Implements the tft.drawFastHLine function, but modifies the x location based on the width and current rotation
void drawFastHLineWithRotation(uint16_t x, uint16_t changeX, uint16_t y, uint16_t changeY, uint16_t w, uint16_t color) {
  const int buffer = (xRot == 1) ? 0 : 1; // i hate this but its the only easy way to make this work. Apparently when the board is rotated the lines get shifted one pixel or smth
  tft.drawFastHLine(x + changeX * xRot - w * lineWidthMulitipiler + buffer, y + changeY * yRot, w, color);
}

// Similar to tft.drawFastHLine; is not currently used
void drawFastVLineWithRotation(uint16_t x, uint16_t changeX, uint16_t y, uint16_t changeY, uint16_t h, uint16_t color) {
  tft.drawFastHLine(x + changeX * xRot, y + changeY * yRot - h * lineHeightMulitipiler, h, color);
}

void drawButtons() {
  if (states.up) {
    drawUpButton(NOTATION_FRONT, NOTATION_BACK);
  } else {
    drawUpButton(NOTATION_BACK, NOTATION_FRONT);
  }
  if (states.down) {
    drawDownButton(NOTATION_FRONT, NOTATION_BACK);
  } else {
    drawDownButton(NOTATION_BACK, NOTATION_FRONT);
  }
}

void drawUpButton(uint16_t color1, uint16_t color2) {
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
void drawDownButton(uint16_t color1, uint16_t color2) {
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

// Individual draw function for each of the pieces, partially generated through a python script but mostly edited by hand

void drawKnight(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2) {  // Enhanced using .drawFastHLine()
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

void drawPawn(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2) {  // Enhanced using .drawFastHLine()
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

void drawQueen(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2) {  // Enhanced using .drawFastHLine()
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

void drawKing(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2) {  // Enhanced using .drawFastHLine()
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

void drawRook(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2) {  // Enhanced using .drawFastHLine()
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

void drawBishop(uint16_t x, uint16_t y, uint16_t color1, uint16_t color2) {// Enhanced using .drawFastHLine()
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
