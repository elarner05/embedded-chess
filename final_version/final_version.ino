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

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);


#define YP A3  // Must be an analog pin
#define XM A2  // Must be an analog pin
#define YM 9   // Can be a digital pin
#define XP 8   // Can be a digital pin

constexpr int MINPRESSURE = 5;
constexpr int MAXPRESSURE = 1000;  // possibly 100, needs tested

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

// Board buffer
constexpr int BOARD_BUFFER = 80;
constexpr int SQUARE_SIZE = 30;



// Chess piece IDs
enum Piece : uint8_t {
  BLANK_SPACE = 0,  // Empty square
  WPAWN,
  WKNIGHT,
  WBISHOP,
  WROOK,
  WQUEEN,
  WKING,
  BPAWN,
  BKNIGHT,
  BBISHOP,
  BROOK,
  BQUEEN,
  BKING
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
int8_t x_rot = 1;
int8_t y_rot = 1;
int8_t lineWidthMulitipiler = 0;
int8_t lineHeightMulitipiler = 0;

// Chess board with initial setup.   To access a square, use the notation:' board[y][x] ', where x,y are the coords of the square.  0<= x,y <=7
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

// Bitboard overlay, where "1" indicates a filled in pixel, and "0" indicates a pixel left untouched. Use pgm_read_byte(&capture_pic[x][y]) to load the bool
static const bool capture_pic[SQUARE_SIZE][SQUARE_SIZE] PROGMEM = {
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
Piece hypothetical_board[8][8];

//                           white          black
// Rook movement flags     left  right   left  right
bool rook_movement[4] = { false, false, false, false };

bool white_king_moved = false;
bool black_king_moved = false;

bool turn = 0;  // 0 = white, 1 = black

struct Square current_king_location;  // Stores the square of the king to add red "check" effects.

bool selectingPiece = true;     // true for the first press, false for the second press
bool promotingPiece = false;    // Promotion menu activity indicator
bool cancel_promotion = false;  // Flag to cancel the promotion menu

bool castle_alert = false;   // Flag for the "castle" move
bool passant_alert = false;  // Flag for the "en passant" move

struct Ply selectedPly = { { 0, 0 }, { 0, 0 } };  // Coords for both squares for the current half-move
struct Ply previousPly = { { 0, 0 }, { 0, 0 } };  // Coords for both squares for the last half-move


// Setup the serial connection (if applicable), the tft interface, and then finally displays the initial chess board
void setup() {
  //Serial.begin(9600);

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
    lastPress = millis();

    //Serial.print(p.x);Serial.print(" ");Serial.println(p.y);
    int p_x = 330 - map(p.y, TS_MINY, TS_MAXY, 0, 320);
    int p_y = map(p.x, TS_MINX, TS_MAXX, 240, 0);
    //Serial.print(p_x);Serial.print(" ");Serial.println(p_y);

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
          cancel_promotion = true; /* user did not press a square or pressed a square on the wrong column or wrong row */

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

        if (((board[sy][sx] < BPAWN && turn == 0) || (board[sy][sx] > WKING && turn == 1))) {
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
          if (check_for_check(previousPly, board) && ((board[selectedPly.from.y][selectedPly.from.x] == BKING || board[selectedPly.from.y][selectedPly.from.x] == WKING))) {
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
    }
  }
}

// Check that the given pressure is enough to be considered a press
bool validPress(int16_t z) {
  return (z > MINPRESSURE && z < MAXPRESSURE && millis() - MILLIS_BETWEEN_PRESS > lastPress);
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



// The handle move logic, which runs after the second square is pressed
void handleMove() {
  uint16_t color;

  if (!cancel_promotion) {

    turn = (turn == 0) ? 1 : 0;  // flip the turn value
    flipRotation();

    // rook move check  -  hard-coded values                           left white rook                                                                                                   right white rook                                                                                                                                    black left rook                                                                                                                              black right rook
    if (board[selectedPly.from.y][selectedPly.from.x] == WROOK && selectedPly.from.x == 0 && selectedPly.from.y == 7) {
      rook_movement[0] = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == WROOK && selectedPly.from.x == 7 && selectedPly.from.y == 7) {
      rook_movement[1] = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == BROOK && selectedPly.from.x == 0 && selectedPly.from.y == 0) {
      rook_movement[2] = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == WROOK && selectedPly.from.x == 7 && selectedPly.from.y == 0) {
      rook_movement[3] = true;
    }

    // king move check
    if (board[selectedPly.from.y][selectedPly.from.x] == WKING && selectedPly.from.x == 4 && selectedPly.from.y == 7) {
      white_king_moved = true;
    } else if (board[selectedPly.from.y][selectedPly.from.x] == BKING && selectedPly.from.x == 4 && selectedPly.from.y == 0) {
      black_king_moved = true;
    }

    board[selectedPly.to.y][selectedPly.to.x] = board[selectedPly.from.y][selectedPly.from.x];
    board[selectedPly.from.y][selectedPly.from.x] = BLANK_SPACE;



    if (passant_alert == true) {
      passant_alert = false;
      board[previousPly.to.y][previousPly.to.x] = BLANK_SPACE;
    }
    if (castle_alert) {
      castle_alert = false;
      if (board[selectedPly.to.y][selectedPly.to.x] == WKING) {
        if (selectedPly.from.x - selectedPly.to.x > 0) {
          board[7][0] = BLANK_SPACE;
          board[7][3] = WROOK;
        } else {
          board[7][7] = BLANK_SPACE;
          board[7][5] = WROOK;
        }
      } else if (selectedPly.from.x - selectedPly.to.x > 0) {
        board[0][0] = BLANK_SPACE;
        board[0][3] = BROOK;
      } else {
        board[0][7] = BLANK_SPACE;
        board[0][5] = BROOK;
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

    previousPly = selectedPly;
    updateBoard(previousPly, selectedPly, board);

    // Add red effect if in 'check'
    if (check_for_check(previousPly, board)) {
      find_king(board);

      tft.fillRect(BOARD_BUFFER + (current_king_location.x) * SQUARE_SIZE, (current_king_location.y) * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, RED);
      drawPiece(board[current_king_location.y][current_king_location.x], current_king_location.x, current_king_location.y);
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
    cancel_promotion = false;
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
      passant_alert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && previousPly.from.y == 6 && previousPly.to.x == x2 && previousPly.to.y == 4 && board[previousPly.to.y][previousPly.to.x] == WPAWN) {
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
    if (check_for_check(previousPly, board)) { return false; }  // starts in check
    if (x1 - x2 > 0) {
      update_hypothetical(board);
      hypothetical_board[y2][x2 + 1] = hypothetical_board[y1][x1];
      hypothetical_board[y1][x1] = BLANK_SPACE;
      if (check_for_check(previousPly, hypothetical_board)) { return false; }  /// passes through check
    } else if (x1 - x2 < 0) {
      update_hypothetical(board);
      hypothetical_board[y2][x2 - 1] = hypothetical_board[y1][x1];
      hypothetical_board[y1][x1] = BLANK_SPACE;
      if (check_for_check(previousPly, hypothetical_board)) { return false; }
    }
  }
  update_hypothetical(board);

  //memcpy(hypothetical_board, board, 8);
  hypothetical_board[y2][x2] = hypothetical_board[y1][x1];
  hypothetical_board[y1][x1] = BLANK_SPACE;
  if (passant_alert == true) {
    hypothetical_board[previousPly.to.y][previousPly.to.x] = BLANK_SPACE;
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
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypothetical_board)) {
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
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypothetical_board)) {
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
bool check_for_check(Ply previousPly, Piece board[8][8]) {
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
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypothetical_board)) {
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
          if (validMoveWithoutCheck(x, y, kx, ky, previousPly, hypothetical_board)) {
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
      passant_alert = true;
    } else {
      return false;
    }


  } else if (id == BPAWN) {
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) && y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE || (y1 == 1 && ((y2 - 2 == y1 && board[y1 + 1][x1] == BLANK_SPACE && board[y1 + 2][x1] == BLANK_SPACE) || y2 - 1 == y1 && board[y1 + 1][x1] == BLANK_SPACE)) && (x1 == x2)) || (abs(x1 - x2) == 1 && y2 - 1 == y1 && board[y2][x2] < BPAWN && board[y2][x2] > BLANK_SPACE)) {
    } else if (
      abs(x1 - x2) == 1 && y2 - 1 == y1 && y1 == 4 && previousPly.from.y == 6 && previousPly.to.x == x2 && previousPly.to.y == 4 && board[previousPly.to.y][previousPly.to.x] == WPAWN) {
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
void update_hypothetical(Piece from[8][8]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      hypothetical_board[y][x] = from[y][x];
    }
  }
}

// Finds the king on the given board and updates the global king position
void find_king(Piece board[8][8]) {
  uint8_t king = (turn == 0) ? WKING : BKING;  // Set the king's piece based on the turn
  for (uint8_t y = 0; y < 8; y++) {
    for (uint8_t x = 0; x < 8; x++) {
      if (board[y][x] == king) {
        current_king_location.x = x;
        current_king_location.y = y;
        return;  // Return early once the king is found
      }
    }
  }
}

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
              if (pgm_read_byte(&capture_pic[j][i]) == 1) {
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
  passant_alert = false;
  castle_alert = false;
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
              if (pgm_read_byte(&capture_pic[j][i]) == 1) {
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
  passant_alert = false;
  castle_alert = false;
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

// Space to distinguish draw functions





void drawPiece(Piece id, int x, int y) {
  const int buffer = (turn == 0) ? 5 : 24;  // Buffer to center the piece on its square on the board
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

void updateBoard(Ply previousPly, Ply selectedPly, Piece board[8][8]) {
  // Turns all pieces to face correct direction
  // Only updates squares with pieces to save time
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

void drawBoard(Piece board[8][8]) {
  // Similar to update board, does not consider the squares currently pressed

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



void flipRotation() {
  x_rot *= -1;
  y_rot *= -1;
  lineWidthMulitipiler = (x_rot == 1) ? 0 : 1;
  lineHeightMulitipiler = (y_rot == 1) ? 0 : 1;
}

void drawFastHLineWithRotation(uint16_t x, uint16_t changeX, uint16_t y, uint16_t changeY, uint16_t w, uint16_t color) {
  const int buffer = (x_rot == 1) ? 0 : 1; // i hate this but its the only easy way to make this work. Apparently when the board is rotated the lines get shifted one pixel or smth
  tft.drawFastHLine(x + changeX * x_rot - w * lineWidthMulitipiler + buffer, y + changeY * y_rot, w, color);
}

void drawFastVLineWithRotation(uint16_t x, uint16_t changeX, uint16_t y, uint16_t changeY, uint16_t h, uint16_t color) {
  tft.drawFastHLine(x + changeX * x_rot, y + changeY * y_rot - h * lineHeightMulitipiler, h, color);
}


void drawKnight(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {  // Enhanced using .drawFastHLine()
  tft.drawPixel(x + 7 * x_rot, y, color_2);
  tft.drawPixel(x + 9 * x_rot, y, color_2);

  drawFastHLineWithRotation(x, 6, y, 1, 5, color_2);
  tft.drawPixel(x + 7 * x_rot, y + 1 * y_rot, color_1);
  tft.drawPixel(x + 9 * x_rot, y + 1 * y_rot, color_1);


  tft.drawPixel(x + 5 * x_rot, y + 2 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 2, 5, color_1);
  tft.drawPixel(x + 11 * x_rot, y + 2 * y_rot, color_2);

  tft.drawPixel(x + 4 * x_rot, y + 3 * y_rot, color_2);
  drawFastHLineWithRotation(x, 5, y, 3, 7, color_1);
  tft.drawPixel(x + 12 * x_rot, y + 3 * y_rot, color_2);

  tft.drawPixel(x + 4 * x_rot, y + 4 * y_rot, color_2);
  drawFastHLineWithRotation(x, 5, y, 4, 8, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 4 * y_rot, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 5 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 5, 10, color_1);
  tft.drawPixel(x + 12 * x_rot, y + 5 * y_rot, color_2);
  tft.drawPixel(x + 14 * x_rot, y + 5 * y_rot, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 6 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 6, 11, color_1);
  tft.drawPixel(x + 5 * x_rot, y + 6 * y_rot, color_2);
  tft.drawPixel(x + 13 * x_rot, y + 6 * y_rot, color_2);
  tft.drawPixel(x + 15 * x_rot, y + 6 * y_rot, color_2);

  tft.drawPixel(x + 2 * x_rot, y + 7 * y_rot, color_2);
  drawFastHLineWithRotation(x, 3, y, 7, 12, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 7 * y_rot, color_2);
  tft.drawPixel(x + 15 * x_rot, y + 7 * y_rot, color_2);

  drawFastHLineWithRotation(x, 1, y, 8, 2, color_2);
  drawFastHLineWithRotation(x, 3, y, 8, 13, color_1);
  tft.drawPixel(x + 9 * x_rot, y + 8 * y_rot, color_2);
  tft.drawPixel(x + 14 * x_rot, y + 8 * y_rot, color_2);
  tft.drawPixel(x + 16 * x_rot, y + 8 * y_rot, color_2);

  drawFastHLineWithRotation(x, 0, y, 9, 2, color_2);
  drawFastHLineWithRotation(x, 2, y, 9, 14, color_1);
  drawFastHLineWithRotation(x, 8, y, 9, 3, color_2);
  tft.drawPixel(x + 14 * x_rot, y + 9 * y_rot, color_2);
  tft.drawPixel(x + 16 * x_rot, y + 9 * y_rot, color_2);

  tft.drawPixel(x + 0 * x_rot, y + 10 * y_rot, color_2);
  drawFastHLineWithRotation(x, 1, y, 10, 5, color_1);
  drawFastHLineWithRotation(x, 6, y, 10, 2, color_2);
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 10 * x_rot, y + 10 * y_rot, color_2);
  drawFastHLineWithRotation(x, 11, y, 10, 6, color_1);
  tft.drawPixel(x + 15 * x_rot, y + 10 * y_rot, color_2);
  tft.drawPixel(x + 17 * x_rot, y + 10 * y_rot, color_2);

  drawFastHLineWithRotation(x, 1, y, 11, 5, color_2);
  tft.drawPixel(x + 2 * x_rot, y + 11 * y_rot, color_1);
  tft.drawPixel(x + 4 * x_rot, y + 11 * y_rot, color_1);
  //------------------------------------
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 9 * x_rot, y + 11 * y_rot, color_2);
  drawFastHLineWithRotation(x, 10, y, 11, 7, color_1);
  tft.drawPixel(x + 15 * x_rot, y + 11 * y_rot, color_2);
  tft.drawPixel(x + 17 * x_rot, y + 11 * y_rot, color_2);

  drawFastHLineWithRotation(x, 1, y, 12, 4, color_2);
  //------------------------------------
  //------------------------------------
  //------------------------------------
  tft.drawPixel(x + 8 * x_rot, y + 12 * y_rot, color_2);
  drawFastHLineWithRotation(x, 9, y, 12, 8, color_1);
  tft.drawPixel(x + 15 * x_rot, y + 12 * y_rot, color_2);
  tft.drawPixel(x + 17 * x_rot, y + 12 * y_rot, color_2);

  tft.drawPixel(x + 7 * x_rot, y + 13 * y_rot, color_2);
  drawFastHLineWithRotation(x, 8, y, 13, 9, color_1);
  tft.drawPixel(x + 15 * x_rot, y + 13 * y_rot, color_2);
  tft.drawPixel(x + 17 * x_rot, y + 13 * y_rot, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 14 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 14, 10, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 14 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 15 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 15, 11, color_1);
  tft.drawPixel(x + 18 * x_rot, y + 15 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 16 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 16, 11, color_1);
  tft.drawPixel(x + 18 * x_rot, y + 16 * y_rot, color_2);

  drawFastHLineWithRotation(x, 4, y, 17, 16, color_2);

  tft.drawPixel(x + 4 * x_rot, y + 18 * y_rot, color_2);
  drawFastHLineWithRotation(x, 5, y, 18, 14, color_1);
  tft.drawPixel(x + 19 * x_rot, y + 18 * y_rot, color_2);

  drawFastHLineWithRotation(x, 4, y, 19, 16, color_2);
}

void drawPawn(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 8, y, 0, 4, color_2);

  tft.drawPixel(x + 7 * x_rot, y + 1 * y_rot, color_2);
  drawFastHLineWithRotation(x, 8, y, 1, 4, color_1);
  tft.drawPixel(x + 12 * x_rot, y + 1 * y_rot, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 2 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 2, 6, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 2 * y_rot, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 3 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 3, 6, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 3 * y_rot, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 4 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 4, 6, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 4 * y_rot, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 5 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 5, 6, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 5 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 6 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 6, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 6 * y_rot, color_2);

  drawFastHLineWithRotation(x, 5, y, 7, 10, color_2);  //         //

  drawFastHLineWithRotation(x, 6, y, 8, 2, color_2);
  drawFastHLineWithRotation(x, 8, y, 8, 4, color_1);
  drawFastHLineWithRotation(x, 12, y, 8, 2, color_2);

  tft.drawPixel(x + 7 * x_rot, y + 9 * y_rot, color_2);
  drawFastHLineWithRotation(x, 8, y, 9, 4, color_1);
  tft.drawPixel(x + 12 * x_rot, y + 9 * y_rot, color_2);

  drawFastHLineWithRotation(x, 6, y, 10, 2, color_2);
  drawFastHLineWithRotation(x, 8, y, 10, 4, color_1);
  drawFastHLineWithRotation(x, 12, y, 10, 2, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 11 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 11, 6, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 11 * y_rot, color_2);

  tft.drawPixel(x + 6 * x_rot, y + 12 * y_rot, color_2);
  drawFastHLineWithRotation(x, 7, y, 12, 6, color_1);
  tft.drawPixel(x + 13 * x_rot, y + 12 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 13 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 13, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 13 * y_rot, color_2);

  drawFastHLineWithRotation(x, 4, y, 14, 2, color_2);
  drawFastHLineWithRotation(x, 6, y, 14, 8, color_1);
  drawFastHLineWithRotation(x, 14, y, 14, 2, color_2);

  tft.drawPixel(x + 4 * x_rot, y + 15 * y_rot, color_2);
  drawFastHLineWithRotation(x, 5, y, 15, 10, color_1);
  tft.drawPixel(x + 15 * x_rot, y + 15 * y_rot, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 16 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 16, 12, color_1);
  tft.drawPixel(x + 16 * x_rot, y + 16 * y_rot, color_2);

  drawFastHLineWithRotation(x, 2, y, 17, 16, color_2);

  tft.drawPixel(x + 1 * x_rot, y + 18 * y_rot, color_2);
  drawFastHLineWithRotation(x, 2, y, 18, 16, color_1);
  tft.drawPixel(x + 18 * x_rot, y + 18 * y_rot, color_2);

  drawFastHLineWithRotation(x, 1, y, 19, 18, color_2);
}

void drawQueen(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 6, y, 0, 3, color_2);
  drawFastHLineWithRotation(x, 11, y, 0, 3, color_2);

  drawFastHLineWithRotation(x, 6, y, 1, 3, color_2);
  tft.drawPixel(x + 7 * x_rot, y + 1 * y_rot, color_1);
  drawFastHLineWithRotation(x, 11, y, 1, 3, color_2);
  tft.drawPixel(x + 12 * x_rot, y + 1 * y_rot, color_1);

  drawFastHLineWithRotation(x, 0, y, 2, 3, color_2);
  drawFastHLineWithRotation(x, 6, y, 2, 3, color_2);
  drawFastHLineWithRotation(x, 11, y, 2, 3, color_2);
  drawFastHLineWithRotation(x, 17, y, 2, 3, color_2);

  drawFastHLineWithRotation(x, 0, y, 3, 3, color_2);
  tft.drawPixel(x + 1 * x_rot, y + 3 * y_rot, color_1);
  tft.drawPixel(x + 7 * x_rot, y + 3 * y_rot, color_2);
  tft.drawPixel(x + 12 * x_rot, y + 3 * y_rot, color_2);
  drawFastHLineWithRotation(x, 17, y, 3, 3, color_2);
  tft.drawPixel(x + 18 * x_rot, y + 3 * y_rot, color_1);

  drawFastHLineWithRotation(x, 0, y, 4, 3, color_2);
  drawFastHLineWithRotation(x, 6, y, 4, 3, color_2);
  tft.drawPixel(x + 7 * x_rot, y + 4 * y_rot, color_1);
  drawFastHLineWithRotation(x, 11, y, 4, 3, color_2);
  tft.drawPixel(x + 12 * x_rot, y + 4 * y_rot, color_1);
  drawFastHLineWithRotation(x, 17, y, 4, 3, color_2);

  drawFastHLineWithRotation(x, 1, y, 5, 2, color_2);
  drawFastHLineWithRotation(x, 6, y, 5, 3, color_2);
  tft.drawPixel(x + 7 * x_rot, y + 5 * y_rot, color_1);
  drawFastHLineWithRotation(x, 11, y, 5, 3, color_2);
  tft.drawPixel(x + 12 * x_rot, y + 5 * y_rot, color_1);
  drawFastHLineWithRotation(x, 17, y, 5, 2, color_2);

  tft.drawPixel(x + 2 * x_rot, y + 6 * y_rot, color_2);
  drawFastHLineWithRotation(x, 5, y, 6, 4, color_2);
  tft.drawPixel(x + 7 * x_rot, y + 6 * y_rot, color_1);
  drawFastHLineWithRotation(x, 11, y, 6, 4, color_2);
  tft.drawPixel(x + 12 * x_rot, y + 6 * y_rot, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 6 * y_rot, color_2);

  drawFastHLineWithRotation(x, 2, y, 7, 2, color_2);
  drawFastHLineWithRotation(x, 5, y, 7, 10, color_2);
  drawFastHLineWithRotation(x, 6, y, 7, 3, color_1);
  drawFastHLineWithRotation(x, 11, y, 7, 3, color_1);
  drawFastHLineWithRotation(x, 16, y, 7, 2, color_2);

  drawFastHLineWithRotation(x, 2, y, 8, 16, color_2);
  tft.drawPixel(x + 3 * x_rot, y + 8 * y_rot, color_1);
  drawFastHLineWithRotation(x, 6, y, 8, 3, color_1);
  drawFastHLineWithRotation(x, 11, y, 8, 3, color_1);
  tft.drawPixel(x + 16 * x_rot, y + 8 * y_rot, color_1);

  tft.drawPixel(x + 2 * x_rot, y + 9 * y_rot, color_2);
  drawFastHLineWithRotation(x, 3, y, 9, 14, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 9 * y_rot, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 10 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 10, 12, color_1);
  tft.drawPixel(x + 16 * x_rot, y + 10 * y_rot, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 11 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 11, 12, color_1);
  tft.drawPixel(x + 16 * x_rot, y + 11 * y_rot, color_2);

  drawFastHLineWithRotation(x, 3, y, 12, 2, color_2);
  drawFastHLineWithRotation(x, 5, y, 12, 10, color_1);
  drawFastHLineWithRotation(x, 15, y, 12, 2, color_2);

  drawFastHLineWithRotation(x, 4, y, 13, 12, color_2);

  drawFastHLineWithRotation(x, 3, y, 14, 2, color_2);
  drawFastHLineWithRotation(x, 5, y, 14, 10, color_1);
  drawFastHLineWithRotation(x, 15, y, 14, 2, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 15 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 15, 12, color_1);
  tft.drawPixel(x + 16 * x_rot, y + 15 * y_rot, color_2);

  drawFastHLineWithRotation(x, 3, y, 16, 14, color_2);

  tft.drawPixel(x + 2 * x_rot, y + 17 * y_rot, color_2);
  drawFastHLineWithRotation(x, 3, y, 17, 14, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 17 * y_rot, color_2);

  tft.drawPixel(x + 2 * x_rot, y + 18 * y_rot, color_2);
  drawFastHLineWithRotation(x, 3, y, 18, 14, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 18 * y_rot, color_2);

  drawFastHLineWithRotation(x, 2, y, 19, 16, color_2);
}

void drawKing(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 9, y, 0, 2, color_2);

  drawFastHLineWithRotation(x, 8, y, 1, 4, color_2);

  drawFastHLineWithRotation(x, 9, y, 2, 2, color_2);

  drawFastHLineWithRotation(x, 9, y, 3, 2, color_2);

  drawFastHLineWithRotation(x, 3, y, 4, 4, color_2);
  drawFastHLineWithRotation(x, 8, y, 4, 4, color_2);
  drawFastHLineWithRotation(x, 13, y, 4, 4, color_2);

  drawFastHLineWithRotation(x, 1, y, 5, 18, color_2);
  drawFastHLineWithRotation(x, 4, y, 5, 2, color_1);
  drawFastHLineWithRotation(x, 9, y, 5, 2, color_1);
  drawFastHLineWithRotation(x, 14, y, 5, 2, color_1);

  drawFastHLineWithRotation(x, 0, y, 6, 2, color_2);
  drawFastHLineWithRotation(x, 2, y, 6, 16, color_1);
  tft.drawPixel(x + 7 * x_rot, y + 6 * y_rot, color_2);
  tft.drawPixel(x + 12 * x_rot, y + 6 * y_rot, color_2);
  drawFastHLineWithRotation(x, 18, y, 6, 2, color_2);

  tft.drawPixel(x + 0 * x_rot, y + 7 * y_rot, color_2);
  drawFastHLineWithRotation(x, 1, y, 7, 18, color_1);
  drawFastHLineWithRotation(x, 7, y, 7, 2, color_2);
  drawFastHLineWithRotation(x, 11, y, 7, 2, color_2);
  tft.drawPixel(x + 19 * x_rot, y + 7 * y_rot, color_2);

  tft.drawPixel(x + 0 * x_rot, y + 8 * y_rot, color_2);
  drawFastHLineWithRotation(x, 1, y, 8, 18, color_1);
  drawFastHLineWithRotation(x, 8, y, 8, 4, color_2);
  tft.drawPixel(x + 19 * x_rot, y + 8 * y_rot, color_2);

  tft.drawPixel(x + 0 * x_rot, y + 9 * y_rot, color_2);
  drawFastHLineWithRotation(x, 1, y, 9, 18, color_1);
  drawFastHLineWithRotation(x, 9, y, 9, 2, color_2);
  tft.drawPixel(x + 19 * x_rot, y + 9 * y_rot, color_2);

  drawFastHLineWithRotation(x, 0, y, 10, 2, color_2);
  drawFastHLineWithRotation(x, 2, y, 10, 16, color_1);
  drawFastHLineWithRotation(x, 9, y, 10, 2, color_2);
  drawFastHLineWithRotation(x, 18, y, 10, 2, color_2);

  tft.drawPixel(x + 1 * x_rot, y + 11 * y_rot, color_2);
  drawFastHLineWithRotation(x, 2, y, 11, 16, color_1);
  drawFastHLineWithRotation(x, 9, y, 11, 2, color_2);
  tft.drawPixel(x + 18 * x_rot, y + 11 * y_rot, color_2);

  drawFastHLineWithRotation(x, 1, y, 12, 2, color_2);
  drawFastHLineWithRotation(x, 3, y, 12, 14, color_1);
  drawFastHLineWithRotation(x, 9, y, 12, 2, color_2);
  drawFastHLineWithRotation(x, 17, y, 12, 2, color_2);

  drawFastHLineWithRotation(x, 2, y, 13, 2, color_2);
  drawFastHLineWithRotation(x, 4, y, 13, 12, color_1);
  drawFastHLineWithRotation(x, 9, y, 13, 2, color_2);
  drawFastHLineWithRotation(x, 16, y, 13, 2, color_2);

  drawFastHLineWithRotation(x, 3, y, 14, 14, color_2);

  drawFastHLineWithRotation(x, 2, y, 15, 2, color_2);
  drawFastHLineWithRotation(x, 4, y, 15, 12, color_1);
  drawFastHLineWithRotation(x, 16, y, 15, 2, color_2);

  drawFastHLineWithRotation(x, 2, y, 16, 16, color_2);
  drawFastHLineWithRotation(x, 3, y, 16, 2, color_1);
  drawFastHLineWithRotation(x, 15, y, 16, 2, color_1);

  drawFastHLineWithRotation(x, 2, y, 17, 3, color_2);
  drawFastHLineWithRotation(x, 5, y, 17, 10, color_1);
  drawFastHLineWithRotation(x, 15, y, 17, 3, color_2);

  tft.drawPixel(x + 2 * x_rot, y + 18 * y_rot, color_2);
  drawFastHLineWithRotation(x, 3, y, 18, 14, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 18 * y_rot, color_2);

  drawFastHLineWithRotation(x, 2, y, 19, 16, color_2);
}

void drawRook(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {  // Enhanced using .drawFastHLine()
  drawFastHLineWithRotation(x, 2, y, 0, 4, color_2);
  drawFastHLineWithRotation(x, 8, y, 0, 4, color_2);
  drawFastHLineWithRotation(x, 14, y, 0, 4, color_2);

  drawFastHLineWithRotation(x, 2, y, 1, 4, color_2);
  drawFastHLineWithRotation(x, 3, y, 1, 2, color_1);
  drawFastHLineWithRotation(x, 8, y, 1, 4, color_2);
  drawFastHLineWithRotation(x, 9, y, 1, 2, color_1);
  drawFastHLineWithRotation(x, 14, y, 1, 4, color_2);
  drawFastHLineWithRotation(x, 15, y, 1, 2, color_1);

  drawFastHLineWithRotation(x, 2, y, 2, 16, color_2);
  drawFastHLineWithRotation(x, 3, y, 2, 2, color_1);
  drawFastHLineWithRotation(x, 9, y, 2, 2, color_1);
  drawFastHLineWithRotation(x, 15, y, 2, 2, color_1);

  tft.drawPixel(x + 2 * x_rot, y + 3 * y_rot, color_2);
  drawFastHLineWithRotation(x, 3, y, 3, 14, color_1);
  tft.drawPixel(x + 17 * x_rot, y + 3 * y_rot, color_2);

  drawFastHLineWithRotation(x, 2, y, 4, 2, color_2);
  drawFastHLineWithRotation(x, 4, y, 4, 12, color_1);
  drawFastHLineWithRotation(x, 16, y, 4, 2, color_2);

  drawFastHLineWithRotation(x, 3, y, 5, 2, color_2);
  drawFastHLineWithRotation(x, 5, y, 5, 10, color_1);
  drawFastHLineWithRotation(x, 15, y, 5, 2, color_2);

  drawFastHLineWithRotation(x, 4, y, 6, 12, color_2);

  //tft.drawRect(x + 5*x_rot, y + 7*y_rot, 10, 7, color_2);
  //tft.fillRect(x + 6*x_rot, y + 7*y_rot, 8, 7, color_1);
  tft.drawPixel(x + 5 * x_rot, y + 7 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 7, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 7 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 8 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 8, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 8 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 9 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 9, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 9 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 10 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 10, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 10 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 11 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 11, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 11 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 12 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 12, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 12 * y_rot, color_2);

  tft.drawPixel(x + 5 * x_rot, y + 13 * y_rot, color_2);
  drawFastHLineWithRotation(x, 6, y, 13, 8, color_1);
  tft.drawPixel(x + 14 * x_rot, y + 13 * y_rot, color_2);

  drawFastHLineWithRotation(x, 4, y, 14, 12, color_2);

  drawFastHLineWithRotation(x, 3, y, 15, 2, color_2);
  drawFastHLineWithRotation(x, 5, y, 15, 10, color_1);
  drawFastHLineWithRotation(x, 15, y, 15, 2, color_2);

  drawFastHLineWithRotation(x, 2, y, 16, 2, color_2);
  drawFastHLineWithRotation(x, 4, y, 16, 12, color_1);
  drawFastHLineWithRotation(x, 16, y, 16, 2, color_2);

  drawFastHLineWithRotation(x, 1, y, 17, 18, color_2);

  tft.drawPixel(x + 1 * x_rot, y + 18 * y_rot, color_2);
  drawFastHLineWithRotation(x, 2, y, 18, 16, color_1);
  tft.drawPixel(x + 18 * x_rot, y + 18 * y_rot, color_2);

  drawFastHLineWithRotation(x, 1, y, 19, 18, color_2);
}

void drawBishop(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  drawFastHLineWithRotation(x, 9, y, 0, 2, color_2);

  tft.drawPixel(x + 8 * x_rot, y + 1 * y_rot, color_2);
  drawFastHLineWithRotation(x, 9, y, 1, 2, color_1);
  tft.drawPixel(x + 11 * x_rot, y + 1 * y_rot, color_2);

  drawFastHLineWithRotation(x, 9, y, 2, 2, color_2);

  drawFastHLineWithRotation(x, 7, y, 3, 6, color_2);
  drawFastHLineWithRotation(x, 9, y, 3, 2, color_1);

  drawFastHLineWithRotation(x, 7, y, 4, 6, color_2);
  drawFastHLineWithRotation(x, 8, y, 4, 2, color_1);

  drawFastHLineWithRotation(x, 6, y, 5, 8, color_2);
  drawFastHLineWithRotation(x, 8, y, 5, 2, color_1);

  drawFastHLineWithRotation(x, 6, y, 6, 8, color_2);
  drawFastHLineWithRotation(x, 7, y, 6, 3, color_1);
  tft.drawPixel(x + 12 * x_rot, y + 6 * y_rot, color_1);

  drawFastHLineWithRotation(x, 6, y, 7, 8, color_2);
  drawFastHLineWithRotation(x, 7, y, 7, 2, color_1);
  drawFastHLineWithRotation(x, 11, y, 7, 2, color_1);

  drawFastHLineWithRotation(x, 6, y, 8, 8, color_2);
  drawFastHLineWithRotation(x, 7, y, 8, 2, color_1);
  drawFastHLineWithRotation(x, 11, y, 8, 2, color_1);

  drawFastHLineWithRotation(x, 6, y, 9, 8, color_2);
  drawFastHLineWithRotation(x, 7, y, 9, 6, color_1);

  drawFastHLineWithRotation(x, 6, y, 10, 8, color_2);
  drawFastHLineWithRotation(x, 7, y, 10, 6, color_1);

  drawFastHLineWithRotation(x, 6, y, 11, 8, color_2);
  drawFastHLineWithRotation(x, 7, y, 11, 6, color_1);

  drawFastHLineWithRotation(x, 6, y, 12, 8, color_2);
  drawFastHLineWithRotation(x, 8, y, 12, 4, color_1);

  drawFastHLineWithRotation(x, 7, y, 13, 6, color_2);
  drawFastHLineWithRotation(x, 9, y, 13, 2, color_1);

  drawFastHLineWithRotation(x, 8, y, 14, 4, color_2);
  drawFastHLineWithRotation(x, 9, y, 14, 2, color_1);

  drawFastHLineWithRotation(x, 7, y, 15, 6, color_2);
  drawFastHLineWithRotation(x, 9, y, 15, 2, color_1);

  drawFastHLineWithRotation(x, 6, y, 16, 8, color_2);
  drawFastHLineWithRotation(x, 8, y, 16, 4, color_1);

  drawFastHLineWithRotation(x, 3, y, 17, 14, color_2);

  tft.drawPixel(x + 3 * x_rot, y + 18 * y_rot, color_2);
  drawFastHLineWithRotation(x, 4, y, 18, 12, color_1);
  tft.drawPixel(x + 16 * x_rot, y + 18 * y_rot, color_2);

  drawFastHLineWithRotation(x, 3, y, 19, 14, color_2);
}
