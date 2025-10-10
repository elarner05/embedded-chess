// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Elegoo_TFTLCD.h FOR SETUP.
//Technical support:goodtft@163.com

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7
// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).

// Assign human-readable names to some common 16-bit color values:
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define KHAKI   0x84AD
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define DARK_BROWN 0xB44C
#define LIGHT_BROWN 0xEED6
#define DARK_OLIVE 0x6368
#define OLIVE 0xAD07
#define LIGHT_OLIVE 0xCE8D
#define BRICK_RED 0xC367
#define BEIGE_GREEN 0xAD6F

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
// If using the shield, all control and data lines are fixed, and
// a simpler declaration can optionally be used:
// Elegoo_TFTLCD tft;

// ids
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
//                          white         black
//                        left  right   left  right
bool rook_movement[4] = {false, false, false, false};
int eightToOne[] = {8, 7, 6, 5, 4, 3, 2, 1};
int board[8][8] = {
    {BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK},
    {BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN},
    {WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK}
  };
int hypthetical_board[8][8];


uint8_t moves[512][2];
int move_no = 0;
int promotion_memory [16];
uint16_t promotion_no = 0;

int turn = 1;// 1 = white, 2 = black
int white_king_moved = false;
int black_king_moved = false;
void setup(void) {
  //Serial.begin(9600);
  //Serial.println(F("TFT LCD test"));

#ifdef USE_Elegoo_SHIELD_PINOUT
  //Serial.println(F("Using Elegoo 2.8\" TFT Arduino Shield Pinout"));
#else
  //Serial.println(F("Using Elegoo 2.8\" TFT Breakout Board Pinout"));
#endif

  //Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();
   if(identifier == 0x9325) {
    //Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    //Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    //Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    //Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    //Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    //Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       //Serial.println(F("Found 0x9341 LCD driver"));
  }else {
    //Serial.print(F("Unknown LCD driver chip: "));
    //Serial.println(identifier, HEX);
    //Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    //Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    //Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    //Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    //Serial.println(F("Also if using the breakout, double-check that all wiring"));
    //Serial.println(F("matches the tutorial."));
    identifier=0x9341;
  
  }

  tft.begin(identifier);
  
  //Serial.println(F("Benchmark                Time (microseconds)"));

  pinMode(13, OUTPUT);
  
  
  /*int board[8][8] = {
    {BROOK, BKNIGHT, BBISHOP, BQUEEN, BKING, BBISHOP, BKNIGHT, BROOK},
    {BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN, BPAWN},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE, BLANK_SPACE},
    {WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN, WPAWN},
    {WROOK, WKNIGHT, WBISHOP, WQUEEN, WKING, WBISHOP, WKNIGHT, WROOK}
  };*/

  //Serial.println(board[1][1]);
  
  
  tft.setRotation(1);
  tft.fillScreen(BLACK);

  drawBoard(board);
  
}

uint8_t coord_to_uint8_t(int arr[]) {
  uint8_t num = arr[1]*8+arr[0];
  return num;
}

int uint8_t_to_coord(uint8_t num, int arr[]) {
  arr[1] = num/8; // remember both are integers; no floating result
  arr[0] = num % 8;
}

void drawPiece(int id, int x, int y) {
  if (turn == 1) {
    int extra = 0;
    switch (id) {
      case BLANK_SPACE:
        break;
      case WPAWN:
        drawPawn(85+(x)*30, 5+(y)*30, WHITE, BLACK);
        break;
      case WKNIGHT:
        drawKnight(85+(x)*30, 5+(y)*30, WHITE, BLACK);
        break;
      case WBISHOP:
        drawBishop(85+(x)*30, 5+(y)*30, WHITE, BLACK);
        break;
      case WROOK:
        drawRook(85+(x)*30, 5+(y)*30, WHITE, BLACK);
        break;
      case WQUEEN:
        drawQueen(85+(x)*30, 5+(y)*30, WHITE, BLACK);
        break;
      case WKING:
        drawKing(85+(x)*30, 5+(y)*30, WHITE, BLACK);
        break;
      case BPAWN:
        drawPawn(85+(x)*30, 5+(y)*30, BLACK, WHITE);
        break;
      case BKNIGHT:
        drawKnight(85+(x)*30, 5+(y)*30, BLACK, WHITE);
        break;
      case BBISHOP:
        drawBishop(85+(x)*30, 5+(y)*30, BLACK, WHITE);
        break;
      case BROOK:
        drawRook(85+(x)*30, 5+(y)*30, BLACK, WHITE);
        break;
      case BQUEEN:
        drawQueen(85+(x)*30, 5+(y)*30, BLACK, WHITE);
        break;
      case BKING:
        drawKing(85+(x)*30, 5+(y)*30, BLACK, WHITE);
        break;
  }
  } else {
    int extra = 19;
    switch (id) {
      case BLANK_SPACE:
        break;
      case WPAWN:
        drawPawn2(85+(x)*30+extra, 5+(y)*30+extra, WHITE, BLACK);
        break;
      case WKNIGHT:
        drawKnight2(85+(x)*30+extra, 5+(y)*30+extra, WHITE, BLACK);
        break;
      case WBISHOP:
        drawBishop2(85+(x)*30+extra, 5+(y)*30+extra, WHITE, BLACK);
        break;
      case WROOK:
        drawRook2(85+(x)*30+extra, 5+(y)*30+extra, WHITE, BLACK);
        break;
      case WQUEEN:
        drawQueen2(85+(x)*30+extra, 5+(y)*30+extra, WHITE, BLACK);
        break;
      case WKING:
        drawKing2(85+(x)*30+extra, 5+(y)*30+extra, WHITE, BLACK);
        break;
      case BPAWN:
        drawPawn2(85+(x)*30+extra, 5+(y)*30+extra, BLACK, WHITE);
        break;
      case BKNIGHT:
        drawKnight2(85+(x)*30+extra, 5+(y)*30+extra, BLACK, WHITE);
        break;
      case BBISHOP:
        drawBishop2(85+(x)*30+extra, 5+(y)*30+extra, BLACK, WHITE);
        break;
      case BROOK:
        drawRook2(85+(x)*30+extra, 5+(y)*30+extra, BLACK, WHITE);
        break;
      case BQUEEN:
        drawQueen2(85+(x)*30+extra, 5+(y)*30+extra, BLACK, WHITE);
        break;
      case BKING:
        drawKing2(85+(x)*30+extra, 5+(y)*30+extra, BLACK, WHITE);
        break;
    }
  }
}

void updateBoard(int board[8][8], int last_squares[4], int current_squares[4]) { // turn all pieces to face correct direction
  
  int i = 1;
  for (int y=0;y<8;y+=1) {
    for (int x=0;x<8;x+=1) {
      if ((x == last_squares[0] and y == last_squares[1]) or (x == last_squares[2] and y == last_squares[3]) or (x == current_squares[0] and y == current_squares[1]) or (x == current_squares[2] and y == current_squares[3])) {i+=1;                                                            continue;}
      if (i%2==0) {
        tft.fillRect(80+(x)*30, (y)*30, 30, 30, DARK_BROWN);
      } else if (i%2==1) {
        tft.fillRect(80+(x)*30, (y)*30, 30, 30, LIGHT_BROWN);
      }
      
      drawPiece(board[y][x], x, y);
      
            
      i+=1;
    }
    i+=1;
  }
}

void drawBoard(int board[8][8]) {
  
  int i = 1;
  for (int y=0;y<8;y+=1) {
    for (int x=0;x<8;x+=1) {
      if (i%2==0) {
        tft.fillRect(80+(x)*30, (y)*30, 30, 30, DARK_BROWN);
      } else if (i%2==1) {
        tft.fillRect(80+(x)*30, (y)*30, 30, 30, LIGHT_BROWN);
      }
      
      drawPiece(board[y][x], x, y);
      
            
      i+=1;
    }
    i+=1;
  }
}


void _drawBoard(int board[8][8]) {
  
  int i = 1;
  for (int y=1;y<=8;y+=1) {
    for (int x=1;x<=8;x+=1) {
      if (i%2==0) {
        tft.fillRect(80+(x-1)*30, (y-1)*30, 30, 30, DARK_BROWN);
      } else if (i%2==1) {
        tft.fillRect(80+(x-1)*30, (y-1)*30, 30, 30, LIGHT_BROWN);
      }
      switch (board[y-1][x-1]) {
        case BLANK_SPACE:
          break;
        case WPAWN:
          drawPawn(85+(x-1)*30, 5+(y-1)*30, WHITE, BLACK);
          break;
        case WKNIGHT:
          drawKnight(85+(x-1)*30, 5+(y-1)*30, WHITE, BLACK);
          break;
        case WBISHOP:
          drawBishop(85+(x-1)*30, 5+(y-1)*30, WHITE, BLACK);
          break;
        case WROOK:
          drawRook(85+(x-1)*30, 5+(y-1)*30, WHITE, BLACK);
          break;
        case WQUEEN:
          drawQueen(85+(x-1)*30, 5+(y-1)*30, WHITE, BLACK);
          break;
        case WKING:
          drawKing(85+(x-1)*30, 5+(y-1)*30, WHITE, BLACK);
          break;
        case BPAWN:
          drawPawn(85+(x-1)*30, 5+(y-1)*30, BLACK, WHITE);
          break;
        case BKNIGHT:
          drawKnight(85+(x-1)*30, 5+(y-1)*30, BLACK, WHITE);
          break;
        case BBISHOP:
          drawBishop(85+(x-1)*30, 5+(y-1)*30, BLACK, WHITE);
          break;
        case BROOK:
          drawRook(85+(x-1)*30, 5+(y-1)*30, BLACK, WHITE);
          break;
        case BQUEEN:
          drawQueen(85+(x-1)*30, 5+(y-1)*30, BLACK, WHITE);
          break;
        case BKING:
          drawKing(85+(x-1)*30, 5+(y-1)*30, BLACK, WHITE);
          break;
      }
            
      i+=1;
    }
    i+=1;
  }
  
}
/*
  for (int y = 0; y < 221; y=y+20){
    for (int x = 0; x < 301; x=x+20) {
      if (x%120==0) {
        drawRook(x, y);
      } else if (x%120 == 20) {
        drawKnight(x, y);
      } else if (x%120 == 40) {
        drawQueen(x, y);
      } else if (x%120 == 60) {
        drawKing(x, y);
      } else if (x%120 == 80) {
        drawPawn(x, y);
      } else if (x%120 == 100) {
        drawBishop(x, y);
      }
    }
  }
*/
  
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define MINPRESSURE 5
#define MAXPRESSURE 1000 // possibly 100

#define TS_MINX 110
#define TS_MAXX 910

#define TS_MINY 70
#define TS_MAXY 900

#define MILLIS_BETWEEN_PRESS 250

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int current_king_location [2] = {-1,-1};// is only used for adding check colours
bool castle_alert = false;
bool passant_alert = false;
int px1 = 0;
int py1 = 0;
int px2 = 0;
int py2 = 0;
int current_squares[] = {0,0,0,0};
int last_squares[] = {-1,-1,-1,-1};  
unsigned long lastPress = millis();
int press_no = 1;
int xswitch[]  = {330, 329, 328, 327, 326, 325, 324, 323, 322, 321, 320, 319, 318, 317, 316, 315, 314, 313, 312, 311, 310, 309, 308, 307, 306, 305, 304, 303, 302, 301, 300, 299, 298, 297,
                  296, 295, 294, 293, 292, 291, 290, 289, 288, 287, 286, 285, 284, 283, 282, 281, 280, 279, 278, 277, 276, 275, 274, 273, 272, 271, 270, 269, 268, 267, 266, 265, 264, 263,
                  262, 261, 260, 259, 258, 257, 256, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229,
                  228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195,
                  194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161,
                  160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127,
                  126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91,
                  90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
                  47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3,
                  2, 1};
int yswitch[] = {250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 225, 224, 223, 222, 221, 220, 219, 218, 217,
                 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183,
                 182, 181, 180, 179, 178, 177, 176, 175, 174, 173, 172, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 153, 152, 151, 150, 149,
                 148, 147, 146, 145, 144, 143, 142, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115,
                 114, 113, 112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76,
                 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,
                 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};


void loop(void) {
  int tempx;
  int tempy;
  int color;
  bool cancel_move = false;
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    
    if (millis()-MILLIS_BETWEEN_PRESS > lastPress) {
      lastPress = millis();
      //Serial.println(lastPress);
      tempx = p.x;
      tempy = p.y;
      //Serial.print(p.x);Serial.print(" ");Serial.println(p.y);
    
      
      p.y = map(tempx, TS_MINX, TS_MAXX, 240, 0);
      
      p.x = xswitch[(320-map(tempy, TS_MINY, TS_MAXY, 320, 0))];
      //Serial.print(p.x);Serial.print(" ");Serial.println(p.y);
      int x, y;
      for (int sy=1;sy<=8;sy+=1) {
        for (int sx=1;sx<=8;sx+=1) {

          x = 80+(sx-1)*30;
          y = (sy-1)*30;
          if (x < p.x-5 and y < p.y+5 and x+30 > p.x-5 and y+30 > p.y+5) {
            switch (press_no) {
              case 1:
                if (not ((board[sy-1][sx-1] < BPAWN and turn == 1) or (board[sy-1][sx-1] > WKING and turn == 2))) {
                  break;
                }
                current_squares[0] = sx-1;
                current_squares[1] = sy-1;
                if (board[current_squares[1]][current_squares[0]] == BLANK_SPACE) {break;}

                draw_possible_moves(last_squares, current_squares, board);
                
                tft.fillRect(80+(sx-1)*30, (sy-1)*30, 30, 30, KHAKI);
                drawPiece(board[sy-1][sx-1], sx-1, sy-1);
                press_no = 2;
                
                break;
              case 2:
                delete_possible_moves(last_squares, current_squares, board);
                current_squares[2] = sx-1;
                current_squares[3] = sy-1;
                if ((board[current_squares[1]][current_squares[0]] == WPAWN and current_squares[3] == 0) or (board[current_squares[1]][current_squares[0]] == BPAWN and current_squares[3] == 7)) {
                  
                }
                
                if (validMove(current_squares[0], current_squares[1], current_squares[2], current_squares[3], last_squares, board)) {
                  if ((board[current_squares[1]][current_squares[0]] == WPAWN and current_squares[3] == 0) or (board[current_squares[1]][current_squares[0]] == BPAWN and current_squares[3] == 7)) {
                    bool run_promote  = true;

                    if (board[current_squares[1]][current_squares[0]] == WPAWN) {
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, BLACK);
                      drawPiece(WQUEEN, current_squares[2], current_squares[3]);
                      
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]+1)*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3]+1)*30, 30, 30, BLACK);
                      drawPiece(WROOK, current_squares[2], current_squares[3]+1);

                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]+2)*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3]+2)*30, 30, 30, BLACK);
                      drawPiece(WBISHOP, current_squares[2], current_squares[3]+2);

                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]+3)*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3]+3)*30, 30, 30, BLACK);
                      drawPiece(WKNIGHT, current_squares[2], current_squares[3]+3);
                    } else {
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, BLACK);
                      drawPiece(BQUEEN, current_squares[2], current_squares[3]);
                      
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]-1)*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3]-1)*30, 30, 30, BLACK);
                      drawPiece(BROOK, current_squares[2], current_squares[3]-1);

                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]-2)*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3]-2)*30, 30, 30, BLACK);
                      drawPiece(BBISHOP, current_squares[2], current_squares[3]-2);

                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]-3)*30, 30, 30, BRICK_RED);
                      tft.drawRect(80+(current_squares[2])*30, (current_squares[3]-3)*30, 30, 30, BLACK);
                      drawPiece(BKNIGHT, current_squares[2], current_squares[3]-3);
                    }


                    run_promote = true;
                    int nx = -1;int ny = -1;
                    while (run_promote) {
                      digitalWrite(13, HIGH);
                      TSPoint p = ts.getPoint();
                      digitalWrite(13, LOW);

                      pinMode(XM, OUTPUT);
                      pinMode(YP, OUTPUT);

                      if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
                        if (millis()-MILLIS_BETWEEN_PRESS > lastPress) {
                          lastPress = millis();
                          tempx = p.x;
                          tempy = p.y;
                          //Serial.print(p.x);Serial.print(" ");Serial.println(p.y);
                        
                          
                          p.y = map(tempx, TS_MINX, TS_MAXX, 240, 0);
                          
                          p.x = xswitch[(320-map(tempy, TS_MINY, TS_MAXY, 320, 0))];

                          for (int sy=1;sy<=8;sy+=1) {
                            for (int sx=1;sx<=8;sx+=1) {
                              x = 80+(sx-1)*30;
                              y = (sy-1)*30;
                              if (x < p.x-5 and y < p.y+5 and x+30 > p.x-5 and y+30 > p.y+5) {
                                nx=sx-1;
                                ny=sy-1;
                                run_promote = false;
                                sy = 9;// this breaks the for loop faster
                                break;
                              }
                            }
                          }
                          run_promote = false;
                        }
                      }
                    }
                    
                    if (board[current_squares[1]][current_squares[0]] == WPAWN) {
                      if ((current_squares[2]+current_squares[3])%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, color);
                      drawPiece(board[current_squares[3]][current_squares[2]], current_squares[2], current_squares[3]);
                      
                      if ((current_squares[2]+current_squares[3]+1)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]+1)*30, 30, 30, color);
                      drawPiece(board[current_squares[3]+1][current_squares[2]], current_squares[2], current_squares[3]+1);

                      if ((current_squares[2]+current_squares[3]+2)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]+2)*30, 30, 30, color);
                      drawPiece(board[current_squares[3]+2][current_squares[2]], current_squares[2], current_squares[3]+2);
                      
                      if ((current_squares[2]+current_squares[3]+3)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]+3)*30, 30, 30, color);
                      drawPiece(board[current_squares[3]+3][current_squares[2]], current_squares[2], current_squares[3]+3);
                    } else {
                      if ((current_squares[2]+current_squares[3])%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, color);
                      drawPiece(board[current_squares[3]][current_squares[2]], current_squares[2], current_squares[3]);
                      
                      if ((current_squares[2]+current_squares[3]-1)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]-1)*30, 30, 30, color);
                      drawPiece(board[current_squares[3]-1][current_squares[2]], current_squares[2], current_squares[3]-1);

                      if ((current_squares[2]+current_squares[3]-2)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]-2)*30, 30, 30, color);
                      drawPiece(board[current_squares[3]-2][current_squares[2]], current_squares[2], current_squares[3]-2);

                      if ((current_squares[2]+current_squares[3]-3)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                      tft.fillRect(80+(current_squares[2])*30, (current_squares[3]-3)*30, 30, 30, color);
                      drawPiece(board[current_squares[3]-3][current_squares[2]], current_squares[2], current_squares[3]-3);
                    }
                    
                    
                    // interpretation
                    if (nx == -1 or ny == -1 or nx != current_squares[2]) {cancel_move = true;/* user did not press a square or pressed a square on the wrong file */
                    
                    } else if (board[current_squares[1]][current_squares[0]] == WPAWN) {
                      Serial.println("White");
                      Serial.println(ny);
                      //if (ny == [current_squares[3] or ny == [current_squares[3]+1 or ny == [current_squares[3]+2 or ny == [current_squares[3]+3) {}
                      if (ny == 0){//current_squares[3]) {
                        Serial.println("White");
                        board[current_squares[1]][current_squares[0]] = WQUEEN; // cheap hack for changing the pawn ; see first line after promote menu
                      } else if (ny == current_squares[3]+1) {
                        board[current_squares[1]][current_squares[0]] = WROOK;
                      } else if (ny == current_squares[3]+2) {
                        board[current_squares[1]][current_squares[0]] = WBISHOP;
                      } else if (ny == current_squares[3]+3) {
                        board[current_squares[1]][current_squares[0]] = WKNIGHT;
                      }
                      Serial.println(board[current_squares[1]][current_squares[0]]);
                    } else if (board[current_squares[1]][current_squares[0]] == BPAWN) {
                      //if (ny == [current_squares[3] or ny == [current_squares[3]-1 or ny == [current_squares[3]-2 or ny == [current_squares[3]-3) {}
                      if (ny == current_squares[3]) {
                        board[current_squares[1]][current_squares[0]] = BQUEEN; // cheap hack for changing the pawn ; see first line after promote menu
                      } else if (ny == current_squares[3]-1) {
                        board[current_squares[1]][current_squares[0]] = BROOK;
                      } else if (ny == current_squares[3]-2) {
                        board[current_squares[1]][current_squares[0]] = BBISHOP;
                      } else if (ny == current_squares[3]-3) {
                        board[current_squares[1]][current_squares[0]] = BKNIGHT;
                      }
                    }
                    
                    
                  }
                  if (not cancel_move) {
                  if (turn == 1) {turn = 2;} else {turn = 1;}

                  // rook move check  -  hard-coded values                           left white rook                                                                                                   right white rook                                                                                                                                    black left rook                                                                                                                              black right rook
                  if (board[current_squares[1]][current_squares[0]] == WROOK and current_squares[0] == 0 and current_squares[1] == 7) {rook_movement[0] = true;} else if (board[current_squares[1]][current_squares[0]] == WROOK and current_squares[0] == 7 and current_squares[1] == 7) {rook_movement[1] = true;} else if (board[current_squares[1]][current_squares[0]] == BROOK and current_squares[0] == 0 and current_squares[1] == 0) {rook_movement[2] = true;} else if (board[current_squares[1]][current_squares[0]] == WROOK and current_squares[0] == 7 and current_squares[1] == 0) {rook_movement[3] = true;}

                  // king move check
                  if (board[current_squares[1]][current_squares[0]] == WKING and current_squares[0] == 4 and current_squares[1] == 7) {white_king_moved = true;} else if (board[current_squares[1]][current_squares[0]] == BKING and current_squares[0] == 4 and current_squares[1] == 0) {black_king_moved = true;}
                  
                  board[current_squares[3]][current_squares[2]] = board[current_squares[1]][current_squares[0]];
                  board[current_squares[1]][current_squares[0]] = BLANK_SPACE;

                  
                  
                  if (passant_alert == true) {
                    passant_alert = false;
                    board[last_squares[3]][last_squares[2]] = BLANK_SPACE;
                  }
                  if (castle_alert) {
                    castle_alert = false;
                    if (board[current_squares[3]][current_squares[2]] == WKING) {if (current_squares[0]-current_squares[2] > 0) {board[7][0] = BLANK_SPACE;board[7][3] = WROOK;} else { board[7][7] = BLANK_SPACE;board[7][5] = WROOK;}} else if (current_squares[0]-current_squares[2] > 0) {board[0][0] = BLANK_SPACE;board[0][3] = BROOK;} else { board[0][7] = BLANK_SPACE;board[0][5] = BROOK;}
                  } 
                  //drawBoard(board);

                  // remove last move just moved colors
                  if ((last_squares[0]+last_squares[1])%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                  tft.fillRect(80+(last_squares[0])*30, (last_squares[1])*30, 30, 30, color);
                  drawPiece(board[last_squares[1]][last_squares[0]], last_squares[0], last_squares[1]);
                  if ((last_squares[2]+last_squares[3])%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                  tft.fillRect(80+(last_squares[2])*30, (last_squares[3])*30, 30, 30, color);
                  drawPiece(board[last_squares[3]][last_squares[2]], last_squares[2], last_squares[3]);
                  
                  
                  // change squares to just moved colors
                  tft.fillRect(80+(current_squares[0])*30, (current_squares[1])*30, 30, 30, OLIVE);
                  drawPiece(board[current_squares[1]][current_squares[0]], current_squares[0], current_squares[1]);
                  tft.fillRect(80+(current_squares[2])*30, (current_squares[3])*30, 30, 30, LIGHT_OLIVE);
                  drawPiece(board[current_squares[3]][current_squares[2]], current_squares[2], current_squares[3]);
                  
                  
                  
                  
                  
                  last_squares[0] = current_squares[0];
                  last_squares[1] = current_squares[1];
                  last_squares[2] = current_squares[2];
                  last_squares[3] = current_squares[3];


                  int move_one[] = {current_squares[0], current_squares[1]}
                  int move_two[] = {current_squares[2], current_squares[3]}
                  
                  moves[move_no][0] = coord_to_uint8_t(move_one);
                  moves[move_no][1] = coord_to_uint8_t(move_two);
                  
                  move_no++;
                  
                  updateBoard(board, last_squares, current_squares);
                  
                  if (check_for_check(last_squares, board)) {
                    //if (turn == 1) {turn = 2;} else {turn = 1;}
                    find_king(turn, board);
                    
                    
                    tft.fillRect(80+(current_king_location[0])*30, (current_king_location[1])*30, 30, 30, RED);
                    drawPiece(board[current_king_location[1]][current_king_location[0]], current_king_location[0], current_king_location[1]);
                    
                    //if (turn == 1) {turn = 2;} else {turn = 1;}
                  }
                  
                  
                  
                  } else {
                    if (current_squares[0] == last_squares[2] and current_squares[1] == last_squares[3]) {color=LIGHT_OLIVE;} else if ((current_squares[0]+current_squares[1])%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                    tft.fillRect(80+(current_squares[0])*30, (current_squares[1])*30, 30, 30, color);
                    drawPiece(board[current_squares[1]][current_squares[0]], current_squares[0], current_squares[1]);
                  }
                } else {
                  if (check_for_check(last_squares, board) and (board[current_squares[1]][current_squares[0]] == BKING or board[current_squares[1]][current_squares[0]]==WKING)) {color=RED;} else if (current_squares[0] == last_squares[2] and current_squares[1] == last_squares[3]) {color=LIGHT_OLIVE;} else if ((current_squares[0]+current_squares[1])%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
                  tft.fillRect(80+(current_squares[0])*30, (current_squares[1])*30, 30, 30, color);
                  drawPiece(board[current_squares[1]][current_squares[0]], current_squares[0], current_squares[1]);
                }
                press_no = 1;
                
                break;
            }
          }

        }
      }
    }
    
  }
}

bool validMove(int x1, int y1, int x2, int y2, int last_moves[], int board[8][8]) { //                             VALIDATE                    MOVE
//                                             ^ en passant ^
  // initial checks                   |--------------------------------(WHITE)---------------------------------|     |--------------------(BLACK)----------------|
  //   check if first space blank                                                                      check if own capture  
  if (board[y1][x1] == BLANK_SPACE or (board[y1][x1]<BPAWN and board[y2][x2]<BPAWN and board[y2][x2]>BLANK_SPACE) or (board[y1][x1]>WKING and board[y2][x2]>WKING)) {return false;}

  // check if move is valid normally
  int id = board[y1][x1];
  
  if (id == WPAWN) { 
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) and y2+1 == y1 and board[y1-1][x1] == BLANK_SPACE or (y1 == 6 and ((y2+2 == y1 and board[y1-1][x1] == BLANK_SPACE and board[y1-2][x1] == BLANK_SPACE) or y2+1 == y1 and board[y1-1][x1] == BLANK_SPACE)) and (x1 == x2)) or
        (abs(x1-x2) == 1 and y2+1 == y1 and board[y2][x2] > WKING)) {} else if (
         abs(x1-x2) == 1 and y2+1 == y1 and y1 == 3 /*and board[y2+1][x2] > WKING*/ and last_moves[1] == 1 and last_moves[2] == x2 and last_moves[3] == 3 and board[last_moves[3]][last_moves[2]] == BPAWN) {
         passant_alert = true;
         } else {return false;}
  
  
  } else if (id == BPAWN) { 
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) and y2-1 == y1 and board[y1+1][x1] == BLANK_SPACE or (y1 == 1 and ((y2-2 == y1 and board[y1+1][x1] == BLANK_SPACE and board[y1+2][x1] == BLANK_SPACE) or y2-1 == y1 and board[y1+1][x1] == BLANK_SPACE)) and (x1 == x2)) or
        (abs(x1-x2) == 1 and y2-1 == y1 and board[y2][x2] < BPAWN and board[y2][x2] > BLANK_SPACE)) {} else if (
         abs(x1-x2) == 1 and y2-1 == y1 and y1 == 4 and last_moves[1] == 6 and last_moves[2] == x2 and last_moves[3] == 4 and board[last_moves[3]][last_moves[2]] == WPAWN) {
         passant_alert = true;
         } else {return false;}
        
  } else if (id == WKNIGHT or id == BKNIGHT) { 
    
    // Knights move by changing x or y by positive or negative 1 and 2
    // 8 possibilitites: (x+1, y+2)  (x+2, y+1)  (x-1, y-2)  (x-2, y-1)  (x+1, y-2)  (x-2, y+1)  (x-1, y+2)  (x+2, y-1)
    //               1                              2                              3                              4                              5                              6                              7                              8                
    if ((x1+1 == x2 and y1+2 == y2) or (x1+2 == x2 and y1+1 == y2) or (x1-1 == x2 and y1-2 == y2) or (x1-2 == x2 and y1-1 == y2) or (x1+1 == x2 and y1-2 == y2) or (x1-2 == x2 and y1+1 == y2) or (x1-1 == x2 and y1+2 == y2) or (x1+2 == x2 and y1-1 == y2)) {
    } else {return false;}
    
  } else if (id == WBISHOP or id == BBISHOP) {

    // Bishops move with equal changes in x and y but not passing over any other pieces
    if (abs(x1-x2) != abs(y1-y2)) {return false;}
    if (x1>x2 and y1>y2) {int s;int t;for (s=x1-1,t=y1-1; s>x2; s--,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}
    } else if (x1<x2 and y1>y2) {int s;int t;for (s=x1+1,t=y1-1; s<x2; s++,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}
    } else if (x1<x2 and y1<y2) {int s;int t;for (s=x1+1,t=y1+1; s<x2; s++,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}
    } else if (x1>x2 and y1<y2) {int s;int t;for (s=x1-1,t=y1+1; s>x2; s--,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}}
    
  } else if (id == WROOK or id == BROOK) {
    
    // Rooks move by changing x or y in any direction but not passing over any other pieces
    if (x2 != x1 and y2 == y1) { // row
      if (x2 < x1) {for (int i = x1-1;i>x2;i--) {if (board[y1][i] != BLANK_SPACE) {return false;}}} else {for (int i = x1+1;i<x2;i++) {if (board[y1][i] != BLANK_SPACE) {return false;}}};
    } else if (x2 == x1 and y2 != y1) { // column
      if (y2 < y1) {for (int i = y1-1;i>y2;i--) {if (board[i][x1] != BLANK_SPACE) {return false;}}} else {for (int i = y1+1;i<y2;i++) {if (board[i][x1] != BLANK_SPACE) {return false;}}};
    } else {return false;/* not moved in only x OR y */}
    
  } else if (id == WQUEEN or id == BQUEEN) {
    
    // Queens move in a combination of the rook and bishop's moves
    if (x2 != x1 and y2 == y1) { // row
      if (x2 < x1) {for (int i = x1-1;i>x2;i--) {if (board[y1][i] != BLANK_SPACE) {return false;}}} else {for (int i = x1+1;i<x2;i++) {if (board[y1][i] != BLANK_SPACE) {return false;}}};
    } else if (x2 == x1 and y2 != y1) { // column //                       ROOK  MOVE
      if (y2 < y1) {for (int i = y1-1;i>y2;i--) {if (board[i][x1] != BLANK_SPACE) {return false;}}} else {for (int i = y1+1;i<y2;i++) {if (board[i][x1] != BLANK_SPACE) {return false;}}};
    
    } else if (abs(x1-x2) == abs(y1-y2)) {
      if (x1>x2 and y1>y2) {int s;int t;for (s=x1-1,t=y1-1; s>x2; s--,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}
      } else if (x1<x2 and y1>y2) {int s;int t;for (s=x1+1,t=y1-1; s<x2; s++,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}  // BISHOP  MOVE
      } else if (x1<x2 and y1<y2) {int s;int t;for (s=x1+1,t=y1+1; s<x2; s++,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}  
      } else if (x1>x2 and y1<y2) {int s;int t;for (s=x1-1,t=y1+1; s>x2; s--,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}}
    
    } else {return false;/* not moved in only x OR y */}
    int white_king_moved = false;
    int black_king_moved = false;
    
  } else if (id == WKING) {
    // The king can move a maximum of one in any direction
    if (not (abs(x1-x2) > 1 or abs(y1-y2) > 1)) {} else if ((not white_king_moved) and abs(x1-x2) == 2 and abs(y1-y2) == 0) {
      if ((x1-x2 > 0 and not rook_movement[0] and board[7][3] == BLANK_SPACE and board[7][2] == BLANK_SPACE and board[7][1] == BLANK_SPACE) or (x1-x2 < 0 and not rook_movement[1] and board[7][5] == BLANK_SPACE)) {castle_alert = true;} else {return false;} 
    } else {return false;}
   
  } else if (id == BKING) {
    // The king can move a maximum of one in any direction
    if (not (abs(x1-x2) > 1 or abs(y1-y2) > 1)) {} else if ((not black_king_moved) and abs(x1-x2) == 2 and abs(y1-y2) == 0) {
      if ((x1-x2 > 0 and not rook_movement[2] and board[0][3] == BLANK_SPACE and board[0][2] == BLANK_SPACE and board[0][1] == BLANK_SPACE) or (x1-x2 < 0 and not rook_movement[3] and board[0][5] == BLANK_SPACE)) {castle_alert = true;} else {return false;} 
    } else {return false;}
    
  }
  
  // check if king is capturable after move

  
  if (castle_alert) {
    if (check_for_check(last_moves, board)) {return false;} // starts in check
    if (x1-x2 > 0) {
      update_hypthetical(board);
      hypthetical_board[y2][x2+1] = hypthetical_board[y1][x1];
      hypthetical_board[y1][x1] = BLANK_SPACE;
      if (check_for_check(last_moves, hypthetical_board)) {return false;} /// passes through check
    } else if (x1-x2 < 0) {
      update_hypthetical(board);
      hypthetical_board[y2][x2-1] = hypthetical_board[y1][x1];
      hypthetical_board[y1][x1] = BLANK_SPACE;
      if (check_for_check(last_moves, hypthetical_board)) {return false;}
    }
  }
  update_hypthetical(board);
  
  //memcpy(hypthetical_board, board, 8);
  hypthetical_board[y2][x2] = hypthetical_board[y1][x1];
  hypthetical_board[y1][x1] = BLANK_SPACE;
  if (passant_alert == true) {
    hypthetical_board[last_squares[3]][last_squares[2]] = BLANK_SPACE;
  }
  Serial.print(hypthetical_board[0][0]);Serial.println(" ");
  //Serial.println(ky);

  
  
  int kx = 0;
  int ky = 0;
  
  if (hypthetical_board[y2][x2] < BPAWN) {
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        
        if (hypthetical_board[y][x] == WKING) { // locate king
          kx=x;
          ky=y;
          Serial.print(kx);Serial.print(" ");
          Serial.println(ky);
        } 
      }//                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypthetical_board[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypthetical_board)) {
            passant_alert = false;
            castle_alert = false;
            return false;}
        } 
      }
    }
  } else {
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypthetical_board[y][x] == BKING) { // locate king
          kx=x;
          ky=y;
        } 
      }//                                                             BLACK
    }
    //Serial.println(kx, ky);
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypthetical_board[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypthetical_board)) {
            passant_alert = false;
            castle_alert = false;
            return false;}
        } 
      }
    }
  }
  
  

  return true; // return true if all checks say the move could be valid
}


bool check_for_check(int last_moves[4], int board[8][8]) {
  update_hypthetical(board);
  int kx = 0;
  int ky = 0;
  
  if (turn == 1) {
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        
        if (hypthetical_board[y][x] == WKING) { // locate king
          kx=x;
          ky=y;

        } 
      }//                                                             WHITE
    }
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypthetical_board[y][x] > WKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypthetical_board)) {
            castle_alert = false;
            return true;}
        } 
      }
    }
  } else {
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypthetical_board[y][x] == BKING) { // locate king
          kx=x;
          ky=y;
        } 
      }//                                                             BLACK
    }
    //Serial.println(kx, ky);
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (hypthetical_board[y][x] < BKING) {
          if (validMoveWithoutCheck(x, y, kx, ky, last_moves, hypthetical_board)) {
            castle_alert = false;
            return true;}
        } 
      }
    }
  }
  return false;
}








bool validMoveWithoutCheck(int x1, int y1, int x2, int y2, int last_moves[], int board[8][8]) { //                             VALIDATE                    MOVE
//                                             ^ en passant ^
  // initial checks                   |--------------------------------(WHITE)---------------------------------|     |--------------------(BLACK)----------------|
  //   check if first space blank                                                                      check if own capture  
  if (board[y1][x1] == BLANK_SPACE or (board[y1][x1]<BPAWN and board[y2][x2]<BPAWN and board[y2][x2]>BLANK_SPACE) or (board[y1][x1]>WKING and board[y2][x2]>WKING)) {return false;}

  // check if move is valid normally
  int id = board[y1][x1];
  
  if (id == WPAWN) { 
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) and y2+1 == y1 and board[y1-1][x1] == BLANK_SPACE or (y1 == 6 and ((y2+2 == y1 and board[y1-1][x1] == BLANK_SPACE and board[y1-2][x1] == BLANK_SPACE) or y2+1 == y1 and board[y1-1][x1] == BLANK_SPACE)) and (x1 == x2)) or
        (abs(x1-x2) == 1 and y2+1 == y1 and board[y2][x2] > WKING)) {} else if (
         abs(x1-x2) == 1 and y2+1 == y1 and y1 == 3 /*and board[y2+1][x2] > WKING*/ and last_moves[1] == 1 and last_moves[2] == x2 and last_moves[3] == 3 and board[last_moves[3]][last_moves[2]] == BPAWN) {
         passant_alert = true;
         } else {return false;}
  
  
  } else if (id == BPAWN) { 
    // The pawn has loads of move rules (e.g. en passant, first square 2 move rule, NO captures forwards...)
    if (((x1 == x2) and y2-1 == y1 and board[y1+1][x1] == BLANK_SPACE or (y1 == 1 and ((y2-2 == y1 and board[y1+1][x1] == BLANK_SPACE and board[y1+2][x1] == BLANK_SPACE) or y2-1 == y1 and board[y1+1][x1] == BLANK_SPACE)) and (x1 == x2)) or
        (abs(x1-x2) == 1 and y2-1 == y1 and board[y2][x2] < BPAWN and board[y2][x2] > BLANK_SPACE)) {} else if (
         abs(x1-x2) == 1 and y2-1 == y1 and y1 == 4 and last_moves[1] == 6 and last_moves[2] == x2 and last_moves[3] == 4 and board[last_moves[3]][last_moves[2]] == WPAWN) {
         passant_alert = true;
         } else {return false;}
        
  } else if (id == WKNIGHT or id == BKNIGHT) { 
    
    // Knights move by changing x or y by positive or negative 1 and 2
    // 8 possibilitites: (x+1, y+2)  (x+2, y+1)  (x-1, y-2)  (x-2, y-1)  (x+1, y-2)  (x-2, y+1)  (x-1, y+2)  (x+2, y-1)
    //               1                              2                              3                              4                              5                              6                              7                              8                
    if ((x1+1 == x2 and y1+2 == y2) or (x1+2 == x2 and y1+1 == y2) or (x1-1 == x2 and y1-2 == y2) or (x1-2 == x2 and y1-1 == y2) or (x1+1 == x2 and y1-2 == y2) or (x1-2 == x2 and y1+1 == y2) or (x1-1 == x2 and y1+2 == y2) or (x1+2 == x2 and y1-1 == y2)) {
    } else {return false;}
    
  } else if (id == WBISHOP or id == BBISHOP) {

    // Bishops move with equal changes in x and y but not passing over any other pieces
    if (abs(x1-x2) != abs(y1-y2)) {return false;}
    if (x1>x2 and y1>y2) {int s;int t;for (s=x1-1,t=y1-1; s>x2; s--,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}
    } else if (x1<x2 and y1>y2) {int s;int t;for (s=x1+1,t=y1-1; s<x2; s++,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}
    } else if (x1<x2 and y1<y2) {int s;int t;for (s=x1+1,t=y1+1; s<x2; s++,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}
    } else if (x1>x2 and y1<y2) {int s;int t;for (s=x1-1,t=y1+1; s>x2; s--,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}}
    
  } else if (id == WROOK or id == BROOK) {
    
    // Rooks move by changing x or y in any direction but not passing over any other pieces
    if (x2 != x1 and y2 == y1) { // row
      if (x2 < x1) {for (int i = x1-1;i>x2;i--) {if (board[y1][i] != BLANK_SPACE) {return false;}}} else {for (int i = x1+1;i<x2;i++) {if (board[y1][i] != BLANK_SPACE) {return false;}}};
    } else if (x2 == x1 and y2 != y1) { // column
      if (y2 < y1) {for (int i = y1-1;i>y2;i--) {if (board[i][x1] != BLANK_SPACE) {return false;}}} else {for (int i = y1+1;i<y2;i++) {if (board[i][x1] != BLANK_SPACE) {return false;}}};
    } else {return false;/* not moved in only x OR y */}
    
  } else if (id == WQUEEN or id == BQUEEN) {
    
    // Queens move in a combination of the rook and bishop's moves
    if (x2 != x1 and y2 == y1) { // row
      if (x2 < x1) {for (int i = x1-1;i>x2;i--) {if (board[y1][i] != BLANK_SPACE) {return false;}}} else {for (int i = x1+1;i<x2;i++) {if (board[y1][i] != BLANK_SPACE) {return false;}}};
    } else if (x2 == x1 and y2 != y1) { // column //                       ROOK  MOVE
      if (y2 < y1) {for (int i = y1-1;i>y2;i--) {if (board[i][x1] != BLANK_SPACE) {return false;}}} else {for (int i = y1+1;i<y2;i++) {if (board[i][x1] != BLANK_SPACE) {return false;}}};
    
    } else if (abs(x1-x2) == abs(y1-y2)) {
      if (x1>x2 and y1>y2) {int s;int t;for (s=x1-1,t=y1-1; s>x2; s--,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}
      } else if (x1<x2 and y1>y2) {int s;int t;for (s=x1+1,t=y1-1; s<x2; s++,t--) {if (board[t][s] != BLANK_SPACE) {return false;}}  // BISHOP  MOVE
      } else if (x1<x2 and y1<y2) {int s;int t;for (s=x1+1,t=y1+1; s<x2; s++,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}  
      } else if (x1>x2 and y1<y2) {int s;int t;for (s=x1-1,t=y1+1; s>x2; s--,t++) {if (board[t][s] != BLANK_SPACE) {return false;}}}
    
    } else {return false;/* not moved in only x OR y */}
    
    
  } else if (id == WKING or id == BKING) {
    // The king can move a maximum of one in any direction
    if (abs(x1-x2) > 1 or abs(y1-y2) > 1) {return false;} 
  }
  
  // check if king is capturable after move

  return true; // return true if all checks say the move could be valid
}

void update_hypthetical(int from[8][8]) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      hypthetical_board[y][x] = from[y][x];
    }  
  }  
}

void find_king(int turn, int board[8][8]) {
  int kx = 0;int ky = 0;

  if (turn == 1) {
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        
        if (board[y][x] == WKING) { // locate king
          kx=x;
          ky=y;

        } 
      }//                                                             WHITE
    }
  }  else {
    
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (board[y][x] == BKING) { // locate king
          kx=x;
          ky=y;
        } 
      }//                                                             BLACK
    }
  }
  current_king_location[0] = kx;
  current_king_location[1] = ky;
  
}



// Chess notation
// Required characters: KQNBRabcdefgh12345678-+=#
void updateNotation() {
  
}









void draw_possible_moves(int last_squares[4], int current_squares[4], int board[8][8]) {
  int color;
  int capture_pic[30][30] = {{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                           {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
                           {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
                           {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                           {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
                           {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
                           {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}};
  for (int y = 0;y<8;y++) {
    for (int x = 0;x<8;x++) {
      if (validMove(current_squares[0], current_squares[1], x, y, last_squares, board)) {
        if (board[y][x] == BLANK_SPACE) {
          tft.fillCircle(80+(x)*30+15, (y)*30+15, 4, DARK_OLIVE);

        } else {
          if (last_squares[2] == x and last_squares[3] == y) {color=LIGHT_OLIVE;} else if ((x+y)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
          //tft.fillRect(80+(x)*30, (y)*30, 30, 30, color);
          for (int j = 0; j < 30;j++) {for (int i = 0; i < 30;i++) {if (capture_pic[j][i]==1){tft.drawPixel((80+(x)*30) + i, (y)*30 + j, DARK_OLIVE);}}}
          drawPiece(board[y][x], x, y);
        }
      }
    }
  }
  passant_alert = false;
  castle_alert = false;
}

void delete_possible_moves(int last_squares[4], int current_squares[4], int board[8][8]) {
  int color;
  int capture_pic[30][30] = {{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                           {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
                           {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
                           {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
                           {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
                           {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
                           {1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
                           {1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1}};
  for (int y = 0;y<8;y++) {
    for (int x = 0;x<8;x++) {
      if (validMove(current_squares[0], current_squares[1], x, y, last_squares, board)) {
        if (board[y][x] == BLANK_SPACE) {
          if (last_squares[0] == x and last_squares[1] == y) {color=OLIVE;} else if ((x+y)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
          tft.fillCircle(80+(x)*30+15, (y)*30+15, 4, color);

        } else {
          if (last_squares[2] == x and last_squares[3] == y) {color=LIGHT_OLIVE;} else if ((x+y)%2==1) {color=DARK_BROWN;} else {color=LIGHT_BROWN;}
          //tft.fillRect(80+(x)*30, (y)*30, 30, 30, color);
          for (int j = 0; j < 30;j++) {for (int i = 0; i < 30;i++) {if (capture_pic[j][i]==1){tft.drawPixel((80+(x)*30) + i, (y)*30 + j, color);}}}
          drawPiece(board[y][x], x, y);
        
        }
      }
    }
  }
  passant_alert = false;
  castle_alert = false;
}
// Space to differentiate draw functions













































// Space to differenciate with draw functions

void drawKnight(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x+7, y, color_2);
  tft.drawPixel(x+9, y, color_2);
  
  tft.drawPixel(x+6, y+1, color_2);
  tft.drawPixel(x+7, y+1, color_1);
  tft.drawPixel(x+8, y+1, color_2);
  tft.drawPixel(x+9, y+1, color_1);
  tft.drawPixel(x+10, y+1, color_2);
  
  tft.drawPixel(x+5, y+2, color_2);
  tft.drawPixel(x+6, y+2, color_1);
  tft.drawPixel(x+7, y+2, color_1);
  tft.drawPixel(x+8, y+2, color_1);
  tft.drawPixel(x+9, y+2, color_1);
  tft.drawPixel(x+10, y+2, color_1);
  tft.drawPixel(x+11, y+2, color_2);

  tft.drawPixel(x+4, y+3, color_2);
  tft.drawPixel(x+5, y+3, color_1);
  tft.drawPixel(x+6, y+3, color_1);
  tft.drawPixel(x+7, y+3, color_1);
  tft.drawPixel(x+8, y+3, color_1);
  tft.drawPixel(x+9, y+3, color_1);
  tft.drawPixel(x+10, y+3, color_1);
  tft.drawPixel(x+11, y+3, color_1);
  tft.drawPixel(x+12, y+3, color_2);

  tft.drawPixel(x+4, y+4, color_2);
  tft.drawPixel(x+5, y+4, color_1);
  tft.drawPixel(x+6, y+4, color_1);
  tft.drawPixel(x+7, y+4, color_1);
  tft.drawPixel(x+8, y+4, color_1);
  tft.drawPixel(x+9, y+4, color_1);
  tft.drawPixel(x+10, y+4, color_1);
  tft.drawPixel(x+11, y+4, color_1);
  tft.drawPixel(x+12, y+4, color_1);
  tft.drawPixel(x+13, y+4, color_2);

  tft.drawPixel(x+3, y+5, color_2);
  tft.drawPixel(x+4, y+5, color_1);
  tft.drawPixel(x+5, y+5, color_1);
  tft.drawPixel(x+6, y+5, color_1);
  tft.drawPixel(x+7, y+5, color_1);
  tft.drawPixel(x+8, y+5, color_1);
  tft.drawPixel(x+9, y+5, color_1);
  tft.drawPixel(x+10, y+5, color_1);
  tft.drawPixel(x+11, y+5, color_1);
  tft.drawPixel(x+12, y+5, color_2);
  tft.drawPixel(x+13, y+5, color_1);
  tft.drawPixel(x+14, y+5, color_2);

  tft.drawPixel(x+3, y+6, color_2);
  tft.drawPixel(x+4, y+6, color_1);
  tft.drawPixel(x+5, y+6, color_2);
  tft.drawPixel(x+6, y+6, color_1);
  tft.drawPixel(x+7, y+6, color_1);
  tft.drawPixel(x+8, y+6, color_1);
  tft.drawPixel(x+9, y+6, color_1);
  tft.drawPixel(x+10, y+6, color_1);
  tft.drawPixel(x+11, y+6, color_1);
  tft.drawPixel(x+12, y+6, color_1);
  tft.drawPixel(x+13, y+6, color_2);
  tft.drawPixel(x+14, y+6, color_1);
  tft.drawPixel(x+15, y+6, color_2);

  tft.drawPixel(x+2, y+7, color_2);
  tft.drawPixel(x+3, y+7, color_1);
  tft.drawPixel(x+4, y+7, color_1);
  tft.drawPixel(x+5, y+7, color_1);
  tft.drawPixel(x+6, y+7, color_1);
  tft.drawPixel(x+7, y+7, color_1);
  tft.drawPixel(x+8, y+7, color_1);
  tft.drawPixel(x+9, y+7, color_1);
  tft.drawPixel(x+10, y+7, color_1);
  tft.drawPixel(x+11, y+7, color_1);
  tft.drawPixel(x+12, y+7, color_1);
  tft.drawPixel(x+13, y+7, color_2);
  tft.drawPixel(x+14, y+7, color_1);
  tft.drawPixel(x+15, y+7, color_2);

  tft.drawPixel(x+1, y+8, color_2);
  tft.drawPixel(x+2, y+8, color_2);
  tft.drawPixel(x+3, y+8, color_1);
  tft.drawPixel(x+4, y+8, color_1);
  tft.drawPixel(x+5, y+8, color_1);
  tft.drawPixel(x+6, y+8, color_1);
  tft.drawPixel(x+7, y+8, color_1);
  tft.drawPixel(x+8, y+8, color_1);
  tft.drawPixel(x+9, y+8, color_2);
  tft.drawPixel(x+10, y+8, color_1);
  tft.drawPixel(x+11, y+8, color_1);
  tft.drawPixel(x+12, y+8, color_1);
  tft.drawPixel(x+13, y+8, color_1);
  tft.drawPixel(x+14, y+8, color_2);
  tft.drawPixel(x+15, y+8, color_1);
  tft.drawPixel(x+16, y+8, color_2);

  tft.drawPixel(x, y+9, color_2);
  tft.drawPixel(x+1, y+9, color_2);
  tft.drawPixel(x+2, y+9, color_1);
  tft.drawPixel(x+3, y+9, color_1);
  tft.drawPixel(x+4, y+9, color_1);
  tft.drawPixel(x+5, y+9, color_1);
  tft.drawPixel(x+6, y+9, color_1);
  tft.drawPixel(x+7, y+9, color_1);
  tft.drawPixel(x+8, y+9, color_2);
  tft.drawPixel(x+9, y+9, color_2);
  tft.drawPixel(x+10, y+9, color_2);
  tft.drawPixel(x+11, y+9, color_1);
  tft.drawPixel(x+12, y+9, color_1);
  tft.drawPixel(x+13, y+9, color_1);
  tft.drawPixel(x+14, y+9, color_2);
  tft.drawPixel(x+15, y+9, color_1);
  tft.drawPixel(x+16, y+9, color_2);

  tft.drawPixel(x, y+10, color_2);
  tft.drawPixel(x+1, y+10, color_1);
  tft.drawPixel(x+2, y+10, color_1);
  tft.drawPixel(x+3, y+10, color_1);
  tft.drawPixel(x+4, y+10, color_1);
  tft.drawPixel(x+5, y+10, color_1);
  tft.drawPixel(x+6, y+10, color_2);
  tft.drawPixel(x+7, y+10, color_2);
  //tft.drawPixel(x+8, y+10, color_2);
  //tft.drawPixel(x+9, y+10, color_2);
  tft.drawPixel(x+10, y+10, color_2);
  tft.drawPixel(x+11, y+10, color_1);
  tft.drawPixel(x+12, y+10, color_1);
  tft.drawPixel(x+13, y+10, color_1);
  tft.drawPixel(x+14, y+10, color_1);
  tft.drawPixel(x+15, y+10, color_2);
  tft.drawPixel(x+16, y+10, color_1);
  tft.drawPixel(x+17, y+10, color_2);

  //tft.drawPixel(x, y+11, color_2);
  tft.drawPixel(x+1, y+11, color_2);
  tft.drawPixel(x+2, y+11, color_1);
  tft.drawPixel(x+3, y+11, color_2);
  tft.drawPixel(x+4, y+11, color_1);
  tft.drawPixel(x+5, y+11, color_2);
  //tft.drawPixel(x+6, y+11, color_2);
  //tft.drawPixel(x+7, y+11, color_2);
  //tft.drawPixel(x+8, y+11, color_2);
  tft.drawPixel(x+9, y+11, color_2);
  tft.drawPixel(x+10, y+11, color_1);
  tft.drawPixel(x+11, y+11, color_1);
  tft.drawPixel(x+12, y+11, color_1);
  tft.drawPixel(x+13, y+11, color_1);
  tft.drawPixel(x+14, y+11, color_1);
  tft.drawPixel(x+15, y+11, color_2);
  tft.drawPixel(x+16, y+11, color_1);
  tft.drawPixel(x+17, y+11, color_2);

  //tft.drawPixel(x, y+12, color_2);
  tft.drawPixel(x+1, y+12, color_2);
  tft.drawPixel(x+2, y+12, color_2);
  tft.drawPixel(x+3, y+12, color_2);
  tft.drawPixel(x+4, y+12, color_2);
  //tft.drawPixel(x+5, y+12, color_2);
  //tft.drawPixel(x+6, y+12, color_2);
  //tft.drawPixel(x+7, y+12, color_2);
  tft.drawPixel(x+8, y+12, color_2);
  tft.drawPixel(x+9, y+12, color_1);
  tft.drawPixel(x+10, y+12, color_1);
  tft.drawPixel(x+11, y+12, color_1);
  tft.drawPixel(x+12, y+12, color_1);
  tft.drawPixel(x+13, y+12, color_1);
  tft.drawPixel(x+14, y+12, color_1);
  tft.drawPixel(x+15, y+12, color_2);
  tft.drawPixel(x+16, y+12, color_1);
  tft.drawPixel(x+17, y+12, color_2);

  //tft.drawPixel(x, y+13, color_2);
  //tft.drawPixel(x+1, y+13, color_2);
  //tft.drawPixel(x+2, y+13, color_2);
  //tft.drawPixel(x+3, y+13, color_2);
  //tft.drawPixel(x+4, y+13, color_2);
  //tft.drawPixel(x+5, y+13, color_2);
  //tft.drawPixel(x+6, y+13, color_2);
  tft.drawPixel(x+7, y+13, color_2);
  tft.drawPixel(x+8, y+13, color_1);
  tft.drawPixel(x+9, y+13, color_1);
  tft.drawPixel(x+10, y+13, color_1);
  tft.drawPixel(x+11, y+13, color_1);
  tft.drawPixel(x+12, y+13, color_1);
  tft.drawPixel(x+13, y+13, color_1);
  tft.drawPixel(x+14, y+13, color_1);
  tft.drawPixel(x+15, y+13, color_2);
  tft.drawPixel(x+16, y+13, color_1);
  tft.drawPixel(x+17, y+13, color_2);

  //tft.drawPixel(x, y+14, color_2);
  //tft.drawPixel(x+1, y+14, color_2);
  //tft.drawPixel(x+2, y+14, color_2);
  //tft.drawPixel(x+3, y+14, color_2);
  //tft.drawPixel(x+4, y+14, color_2);
  //tft.drawPixel(x+5, y+14, color_2);
  tft.drawPixel(x+6, y+14, color_2);
  tft.drawPixel(x+7, y+14, color_1);
  tft.drawPixel(x+8, y+14, color_1);
  tft.drawPixel(x+9, y+14, color_1);
  tft.drawPixel(x+10, y+14, color_1);
  tft.drawPixel(x+11, y+14, color_1);
  tft.drawPixel(x+12, y+14, color_1);
  tft.drawPixel(x+13, y+14, color_1);
  tft.drawPixel(x+14, y+14, color_1);
  tft.drawPixel(x+15, y+14, color_1);
  tft.drawPixel(x+16, y+14, color_1);
  tft.drawPixel(x+17, y+14, color_2);

  //tft.drawPixel(x, y+15, color_2);
  //tft.drawPixel(x+1, y+15, color_2);
  //tft.drawPixel(x+2, y+15, color_2);
  //tft.drawPixel(x+3, y+15, color_2);
  //tft.drawPixel(x+4, y+15, color_2);
  tft.drawPixel(x+5, y+15, color_2);
  tft.drawPixel(x+6, y+15, color_1);
  tft.drawPixel(x+7, y+15, color_1);
  tft.drawPixel(x+8, y+15, color_1);
  tft.drawPixel(x+9, y+15, color_1);
  tft.drawPixel(x+10, y+15, color_1);
  tft.drawPixel(x+11, y+15, color_1);
  tft.drawPixel(x+12, y+15, color_1);
  tft.drawPixel(x+13, y+15, color_1);
  tft.drawPixel(x+14, y+15, color_1);
  tft.drawPixel(x+15, y+15, color_1);
  tft.drawPixel(x+16, y+15, color_1);
  tft.drawPixel(x+17, y+15, color_1);
  tft.drawPixel(x+18, y+15, color_2);
  
  //tft.drawPixel(x, y+16, color_2);
  //tft.drawPixel(x+1, y+16, color_2);
  //tft.drawPixel(x+2, y+16, color_2);
  //tft.drawPixel(x+3, y+16, color_2);
  //tft.drawPixel(x+4, y+16, color_2);
  tft.drawPixel(x+5, y+16, color_2);
  tft.drawPixel(x+6, y+16, color_1);
  tft.drawPixel(x+7, y+16, color_1);
  tft.drawPixel(x+8, y+16, color_1);
  tft.drawPixel(x+9, y+16, color_1);
  tft.drawPixel(x+10, y+16, color_1);
  tft.drawPixel(x+11, y+16, color_1);
  tft.drawPixel(x+12, y+16, color_1);
  tft.drawPixel(x+13, y+16, color_1);
  tft.drawPixel(x+14, y+16, color_1);
  tft.drawPixel(x+15, y+16, color_1);
  tft.drawPixel(x+16, y+16, color_1);
  tft.drawPixel(x+17, y+16, color_1);
  tft.drawPixel(x+18, y+16, color_2);

  //tft.drawPixel(x, y+17, color_2);
  //tft.drawPixel(x+1, y+17, color_2);
  //tft.drawPixel(x+2, y+17, color_2);
  //tft.drawPixel(x+3, y+17, color_2);
  tft.drawPixel(x+4, y+17, color_2);
  tft.drawPixel(x+5, y+17, color_2);
  tft.drawPixel(x+6, y+17, color_2);
  tft.drawPixel(x+7, y+17, color_2);
  tft.drawPixel(x+8, y+17, color_2);
  tft.drawPixel(x+9, y+17, color_2);
  tft.drawPixel(x+10, y+17, color_2);
  tft.drawPixel(x+11, y+17, color_2);
  tft.drawPixel(x+12, y+17, color_2);
  tft.drawPixel(x+13, y+17, color_2);
  tft.drawPixel(x+14, y+17, color_2);
  tft.drawPixel(x+15, y+17, color_2);
  tft.drawPixel(x+16, y+17, color_2);
  tft.drawPixel(x+17, y+17, color_2);
  tft.drawPixel(x+18, y+17, color_2);
  tft.drawPixel(x+19, y+17, color_2);

  //tft.drawPixel(x, y+18, color_2);
  //tft.drawPixel(x+1, y+18, color_2);
  //tft.drawPixel(x+2, y+18, color_2);
  //tft.drawPixel(x+3, y+18, color_2);
  tft.drawPixel(x+4, y+18, color_2);
  tft.drawPixel(x+5, y+18, color_1);
  tft.drawPixel(x+6, y+18, color_1);
  tft.drawPixel(x+7, y+18, color_1);
  tft.drawPixel(x+8, y+18, color_1);
  tft.drawPixel(x+9, y+18, color_1);
  tft.drawPixel(x+10, y+18, color_1);
  tft.drawPixel(x+11, y+18, color_1);
  tft.drawPixel(x+12, y+18, color_1);
  tft.drawPixel(x+13, y+18, color_1);
  tft.drawPixel(x+14, y+18, color_1);
  tft.drawPixel(x+15, y+18, color_1);
  tft.drawPixel(x+16, y+18, color_1);
  tft.drawPixel(x+17, y+18, color_1);
  tft.drawPixel(x+18, y+18, color_1);
  tft.drawPixel(x+19, y+18, color_2);

  //tft.drawPixel(x, y+19, color_2);
  //tft.drawPixel(x+1, y+19, color_2);
  //tft.drawPixel(x+2, y+19, color_2);
  //tft.drawPixel(x+3, y+19, color_2);
  tft.drawPixel(x+4, y+19, color_2);
  tft.drawPixel(x+5, y+19, color_2);
  tft.drawPixel(x+6, y+19, color_2);
  tft.drawPixel(x+7, y+19, color_2);
  tft.drawPixel(x+8, y+19, color_2);
  tft.drawPixel(x+9, y+19, color_2);
  tft.drawPixel(x+10, y+19, color_2);
  tft.drawPixel(x+11, y+19, color_2);
  tft.drawPixel(x+12, y+19, color_2);
  tft.drawPixel(x+13, y+19, color_2);
  tft.drawPixel(x+14, y+19, color_2);
  tft.drawPixel(x+15, y+19, color_2);
  tft.drawPixel(x+16, y+19, color_2);
  tft.drawPixel(x+17, y+19, color_2);
  tft.drawPixel(x+18, y+19, color_2);
  tft.drawPixel(x+19, y+19, color_2);
}










void drawPawn(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x+8, y+0, color_2);
  tft.drawPixel(x+9, y+0, color_2);
  tft.drawPixel(x+10, y+0, color_2);
  tft.drawPixel(x+11, y+0, color_2);

  tft.drawPixel(x+7, y+1, color_2);
  tft.drawPixel(x+8, y+1, color_1);
  tft.drawPixel(x+9, y+1, color_1);
  tft.drawPixel(x+10, y+1, color_1);
  tft.drawPixel(x+11, y+1, color_1);
  tft.drawPixel(x+12, y+1, color_2);

  tft.drawPixel(x+6, y+2, color_2);
  tft.drawPixel(x+7, y+2, color_1);
  tft.drawPixel(x+8, y+2, color_1);
  tft.drawPixel(x+9, y+2, color_1);
  tft.drawPixel(x+10, y+2, color_1);
  tft.drawPixel(x+11, y+2, color_1);
  tft.drawPixel(x+12, y+2, color_1);
  tft.drawPixel(x+13, y+2, color_2);

  tft.drawPixel(x+6, y+3, color_2);
  tft.drawPixel(x+7, y+3, color_1);
  tft.drawPixel(x+8, y+3, color_1);
  tft.drawPixel(x+9, y+3, color_1);
  tft.drawPixel(x+10, y+3, color_1);
  tft.drawPixel(x+11, y+3, color_1);
  tft.drawPixel(x+12, y+3, color_1);
  tft.drawPixel(x+13, y+3, color_2);

  tft.drawPixel(x+6, y+4, color_2);
  tft.drawPixel(x+7, y+4, color_1);
  tft.drawPixel(x+8, y+4, color_1);
  tft.drawPixel(x+9, y+4, color_1);
  tft.drawPixel(x+10, y+4, color_1);
  tft.drawPixel(x+11, y+4, color_1);
  tft.drawPixel(x+12, y+4, color_1);
  tft.drawPixel(x+13, y+4, color_2);

  tft.drawPixel(x+6, y+5, color_2);
  tft.drawPixel(x+7, y+5, color_1);
  tft.drawPixel(x+8, y+5, color_1);
  tft.drawPixel(x+9, y+5, color_1);
  tft.drawPixel(x+10, y+5, color_1);
  tft.drawPixel(x+11, y+5, color_1);
  tft.drawPixel(x+12, y+5, color_1);
  tft.drawPixel(x+13, y+5, color_2);

  tft.drawPixel(x+5, y+6, color_2);
  tft.drawPixel(x+6, y+6, color_1);
  tft.drawPixel(x+7, y+6, color_1);
  tft.drawPixel(x+8, y+6, color_1);
  tft.drawPixel(x+9, y+6, color_1);
  tft.drawPixel(x+10, y+6, color_1);
  tft.drawPixel(x+11, y+6, color_1);
  tft.drawPixel(x+12, y+6, color_1);
  tft.drawPixel(x+13, y+6, color_1);
  tft.drawPixel(x+14, y+6, color_2);

  tft.drawPixel(x+5, y+7, color_2);
  tft.drawPixel(x+6, y+7, color_2);
  tft.drawPixel(x+7, y+7, color_2);
  tft.drawPixel(x+8, y+7, color_2);
  tft.drawPixel(x+9, y+7, color_2);
  tft.drawPixel(x+10, y+7, color_2);
  tft.drawPixel(x+11, y+7, color_2);
  tft.drawPixel(x+12, y+7, color_2);
  tft.drawPixel(x+13, y+7, color_2);
  tft.drawPixel(x+14, y+7, color_2);

  tft.drawPixel(x+6, y+8, color_2);
  tft.drawPixel(x+7, y+8, color_2);
  tft.drawPixel(x+8, y+8, color_1);
  tft.drawPixel(x+9, y+8, color_1);
  tft.drawPixel(x+10, y+8, color_1);
  tft.drawPixel(x+11, y+8, color_1);
  tft.drawPixel(x+12, y+8, color_2);
  tft.drawPixel(x+13, y+8, color_2);

  tft.drawPixel(x+7, y+9, color_2);
  tft.drawPixel(x+8, y+9, color_1);
  tft.drawPixel(x+9, y+9, color_1);
  tft.drawPixel(x+10, y+9, color_1);
  tft.drawPixel(x+11, y+9, color_1);
  tft.drawPixel(x+12, y+9, color_2);

  tft.drawPixel(x+6, y+10, color_2);
  tft.drawPixel(x+7, y+10, color_2);
  tft.drawPixel(x+8, y+10, color_1);
  tft.drawPixel(x+9, y+10, color_1);
  tft.drawPixel(x+10, y+10, color_1);
  tft.drawPixel(x+11, y+10, color_1);
  tft.drawPixel(x+12, y+10, color_2);
  tft.drawPixel(x+13, y+10, color_2);

  tft.drawPixel(x+6, y+11, color_2);
  tft.drawPixel(x+7, y+11, color_1);
  tft.drawPixel(x+8, y+11, color_1);
  tft.drawPixel(x+9, y+11, color_1);
  tft.drawPixel(x+10, y+11, color_1);
  tft.drawPixel(x+11, y+11, color_1);
  tft.drawPixel(x+12, y+11, color_1);
  tft.drawPixel(x+13, y+11, color_2);

  tft.drawPixel(x+6, y+12, color_2);
  tft.drawPixel(x+7, y+12, color_1);
  tft.drawPixel(x+8, y+12, color_1);
  tft.drawPixel(x+9, y+12, color_1);
  tft.drawPixel(x+10, y+12, color_1);
  tft.drawPixel(x+11, y+12, color_1);
  tft.drawPixel(x+12, y+12, color_1);
  tft.drawPixel(x+13, y+12, color_2);

  tft.drawPixel(x+5, y+13, color_2);
  tft.drawPixel(x+6, y+13, color_1);
  tft.drawPixel(x+7, y+13, color_1);
  tft.drawPixel(x+8, y+13, color_1);
  tft.drawPixel(x+9, y+13, color_1);
  tft.drawPixel(x+10, y+13, color_1);
  tft.drawPixel(x+11, y+13, color_1);
  tft.drawPixel(x+12, y+13, color_1);
  tft.drawPixel(x+13, y+13, color_1);
  tft.drawPixel(x+14, y+13, color_2);

  tft.drawPixel(x+4, y+14, color_2);
  tft.drawPixel(x+5, y+14, color_2);
  tft.drawPixel(x+6, y+14, color_1);
  tft.drawPixel(x+7, y+14, color_1);
  tft.drawPixel(x+8, y+14, color_1);
  tft.drawPixel(x+9, y+14, color_1);
  tft.drawPixel(x+10, y+14, color_1);
  tft.drawPixel(x+11, y+14, color_1);
  tft.drawPixel(x+12, y+14, color_1);
  tft.drawPixel(x+13, y+14, color_1);
  tft.drawPixel(x+14, y+14, color_2);
  tft.drawPixel(x+15, y+14, color_2);

  tft.drawPixel(x+4, y+15, color_2);
  tft.drawPixel(x+5, y+15, color_1);
  tft.drawPixel(x+6, y+15, color_1);
  tft.drawPixel(x+7, y+15, color_1);
  tft.drawPixel(x+8, y+15, color_1);
  tft.drawPixel(x+9, y+15, color_1);
  tft.drawPixel(x+10, y+15, color_1);
  tft.drawPixel(x+11, y+15, color_1);
  tft.drawPixel(x+12, y+15, color_1);
  tft.drawPixel(x+13, y+15, color_1);
  tft.drawPixel(x+14, y+15, color_1);
  tft.drawPixel(x+15, y+15, color_2);

  tft.drawPixel(x+3, y+16, color_2);
  tft.drawPixel(x+4, y+16, color_1);
  tft.drawPixel(x+5, y+16, color_1);
  tft.drawPixel(x+6, y+16, color_1);
  tft.drawPixel(x+7, y+16, color_1);
  tft.drawPixel(x+8, y+16, color_1);
  tft.drawPixel(x+9, y+16, color_1);
  tft.drawPixel(x+10, y+16, color_1);
  tft.drawPixel(x+11, y+16, color_1);
  tft.drawPixel(x+12, y+16, color_1);
  tft.drawPixel(x+13, y+16, color_1);
  tft.drawPixel(x+14, y+16, color_1);
  tft.drawPixel(x+15, y+16, color_1);
  tft.drawPixel(x+16, y+16, color_2);

  tft.drawPixel(x+2, y+17, color_2);
  tft.drawPixel(x+3, y+17, color_2);
  tft.drawPixel(x+4, y+17, color_2);
  tft.drawPixel(x+5, y+17, color_2);
  tft.drawPixel(x+6, y+17, color_2);
  tft.drawPixel(x+7, y+17, color_2);
  tft.drawPixel(x+8, y+17, color_2);
  tft.drawPixel(x+9, y+17, color_2);
  tft.drawPixel(x+10, y+17, color_2);
  tft.drawPixel(x+11, y+17, color_2);
  tft.drawPixel(x+12, y+17, color_2);
  tft.drawPixel(x+13, y+17, color_2);
  tft.drawPixel(x+14, y+17, color_2);
  tft.drawPixel(x+15, y+17, color_2);
  tft.drawPixel(x+16, y+17, color_2);
  tft.drawPixel(x+17, y+17, color_2);

  tft.drawPixel(x+1, y+18, color_2);
  tft.drawPixel(x+2, y+18, color_1);
  tft.drawPixel(x+3, y+18, color_1);
  tft.drawPixel(x+4, y+18, color_1);
  tft.drawPixel(x+5, y+18, color_1);
  tft.drawPixel(x+6, y+18, color_1);
  tft.drawPixel(x+7, y+18, color_1);
  tft.drawPixel(x+8, y+18, color_1);
  tft.drawPixel(x+9, y+18, color_1);
  tft.drawPixel(x+10, y+18, color_1);
  tft.drawPixel(x+11, y+18, color_1);
  tft.drawPixel(x+12, y+18, color_1);
  tft.drawPixel(x+13, y+18, color_1);
  tft.drawPixel(x+14, y+18, color_1);
  tft.drawPixel(x+15, y+18, color_1);
  tft.drawPixel(x+16, y+18, color_1);
  tft.drawPixel(x+17, y+18, color_1);
  tft.drawPixel(x+18, y+18, color_2);

  tft.drawPixel(x+1, y+19, color_2);
  tft.drawPixel(x+2, y+19, color_2);
  tft.drawPixel(x+3, y+19, color_2);
  tft.drawPixel(x+4, y+19, color_2);
  tft.drawPixel(x+5, y+19, color_2);
  tft.drawPixel(x+6, y+19, color_2);
  tft.drawPixel(x+7, y+19, color_2);
  tft.drawPixel(x+8, y+19, color_2);
  tft.drawPixel(x+9, y+19, color_2);
  tft.drawPixel(x+10, y+19, color_2);
  tft.drawPixel(x+11, y+19, color_2);
  tft.drawPixel(x+12, y+19, color_2);
  tft.drawPixel(x+13, y+19, color_2);
  tft.drawPixel(x+14, y+19, color_2);
  tft.drawPixel(x+15, y+19, color_2);
  tft.drawPixel(x+16, y+19, color_2);
  tft.drawPixel(x+17, y+19, color_2);
  tft.drawPixel(x+18, y+19, color_2);
}


















void drawQueen(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x+6, y+0, color_2);
  tft.drawPixel(x+7, y+0, color_2);
  tft.drawPixel(x+8, y+0, color_2);
  tft.drawPixel(x+11, y+0, color_2);
  tft.drawPixel(x+12, y+0, color_2);
  tft.drawPixel(x+13, y+0, color_2);

  tft.drawPixel(x+6, y+1, color_2);
  tft.drawPixel(x+7, y+1, color_1);
  tft.drawPixel(x+8, y+1, color_2);
  tft.drawPixel(x+11, y+1, color_2);
  tft.drawPixel(x+12, y+1, color_1);
  tft.drawPixel(x+13, y+1, color_2);

  tft.drawPixel(x+0, y+2, color_2);
  tft.drawPixel(x+1, y+2, color_2);
  tft.drawPixel(x+2, y+2, color_2);
  tft.drawPixel(x+6, y+2, color_2);
  tft.drawPixel(x+7, y+2, color_2);
  tft.drawPixel(x+8, y+2, color_2);
  tft.drawPixel(x+11, y+2, color_2);
  tft.drawPixel(x+12, y+2, color_2);
  tft.drawPixel(x+13, y+2, color_2);
  tft.drawPixel(x+17, y+2, color_2);
  tft.drawPixel(x+18, y+2, color_2);
  tft.drawPixel(x+19, y+2, color_2);

  tft.drawPixel(x+0, y+3, color_2);
  tft.drawPixel(x+1, y+3, color_1);
  tft.drawPixel(x+2, y+3, color_2);
  tft.drawPixel(x+7, y+3, color_2);
  tft.drawPixel(x+12, y+3, color_2);
  tft.drawPixel(x+17, y+3, color_2);
  tft.drawPixel(x+18, y+3, color_1);
  tft.drawPixel(x+19, y+3, color_2);

  tft.drawPixel(x+0, y+4, color_2);
  tft.drawPixel(x+1, y+4, color_2);
  tft.drawPixel(x+2, y+4, color_2);
  tft.drawPixel(x+6, y+4, color_2);
  tft.drawPixel(x+7, y+4, color_1);
  tft.drawPixel(x+8, y+4, color_2);
  tft.drawPixel(x+11, y+4, color_2);
  tft.drawPixel(x+12, y+4, color_1);
  tft.drawPixel(x+13, y+4, color_2);
  tft.drawPixel(x+17, y+4, color_2);
  tft.drawPixel(x+18, y+4, color_2);
  tft.drawPixel(x+19, y+4, color_2);

  tft.drawPixel(x+1, y+5, color_2);
  tft.drawPixel(x+2, y+5, color_2);
  tft.drawPixel(x+6, y+5, color_2);
  tft.drawPixel(x+7, y+5, color_1);
  tft.drawPixel(x+8, y+5, color_2);
  tft.drawPixel(x+11, y+5, color_2);
  tft.drawPixel(x+12, y+5, color_1);
  tft.drawPixel(x+13, y+5, color_2);
  tft.drawPixel(x+17, y+5, color_2);
  tft.drawPixel(x+18, y+5, color_2);

  tft.drawPixel(x+2, y+6, color_2);
  tft.drawPixel(x+5, y+6, color_2);
  tft.drawPixel(x+6, y+6, color_2);
  tft.drawPixel(x+7, y+6, color_1);
  tft.drawPixel(x+8, y+6, color_2);
  tft.drawPixel(x+11, y+6, color_2);
  tft.drawPixel(x+12, y+6, color_1);
  tft.drawPixel(x+13, y+6, color_2);
  tft.drawPixel(x+14, y+6, color_2);
  tft.drawPixel(x+17, y+6, color_2);

  tft.drawPixel(x+2, y+7, color_2);
  tft.drawPixel(x+3, y+7, color_2);
  tft.drawPixel(x+5, y+7, color_2);
  tft.drawPixel(x+6, y+7, color_1);
  tft.drawPixel(x+7, y+7, color_1);
  tft.drawPixel(x+8, y+7, color_1);
  tft.drawPixel(x+9, y+7, color_2);
  tft.drawPixel(x+10, y+7, color_2);
  tft.drawPixel(x+11, y+7, color_1);
  tft.drawPixel(x+12, y+7, color_1);
  tft.drawPixel(x+13, y+7, color_1);
  tft.drawPixel(x+14, y+7, color_2);
  tft.drawPixel(x+16, y+7, color_2);
  tft.drawPixel(x+17, y+7, color_2);

  tft.drawPixel(x+2, y+8, color_2);
  tft.drawPixel(x+3, y+8, color_1);
  tft.drawPixel(x+4, y+8, color_2);
  tft.drawPixel(x+5, y+8, color_2);
  tft.drawPixel(x+6, y+8, color_1);
  tft.drawPixel(x+7, y+8, color_1);
  tft.drawPixel(x+8, y+8, color_1);
  tft.drawPixel(x+9, y+8, color_2);
  tft.drawPixel(x+10, y+8, color_2);
  tft.drawPixel(x+11, y+8, color_1);
  tft.drawPixel(x+12, y+8, color_1);
  tft.drawPixel(x+13, y+8, color_1);
  tft.drawPixel(x+14, y+8, color_2);
  tft.drawPixel(x+15, y+8, color_2);
  tft.drawPixel(x+16, y+8, color_1);
  tft.drawPixel(x+17, y+8, color_2);

  tft.drawPixel(x+2, y+9, color_2);
  tft.drawPixel(x+3, y+9, color_1);
  tft.drawPixel(x+4, y+9, color_1);
  tft.drawPixel(x+5, y+9, color_1);
  tft.drawPixel(x+6, y+9, color_1);
  tft.drawPixel(x+7, y+9, color_1);
  tft.drawPixel(x+8, y+9, color_1);
  tft.drawPixel(x+9, y+9, color_1);
  tft.drawPixel(x+10, y+9, color_1);
  tft.drawPixel(x+11, y+9, color_1);
  tft.drawPixel(x+12, y+9, color_1);
  tft.drawPixel(x+13, y+9, color_1);
  tft.drawPixel(x+14, y+9, color_1);
  tft.drawPixel(x+15, y+9, color_1);
  tft.drawPixel(x+16, y+9, color_1);
  tft.drawPixel(x+17, y+9, color_2);

  tft.drawPixel(x+3, y+10, color_2);
  tft.drawPixel(x+4, y+10, color_1);
  tft.drawPixel(x+5, y+10, color_1);
  tft.drawPixel(x+6, y+10, color_1);
  tft.drawPixel(x+7, y+10, color_1);
  tft.drawPixel(x+8, y+10, color_1);
  tft.drawPixel(x+9, y+10, color_1);
  tft.drawPixel(x+10, y+10, color_1);
  tft.drawPixel(x+11, y+10, color_1);
  tft.drawPixel(x+12, y+10, color_1);
  tft.drawPixel(x+13, y+10, color_1);
  tft.drawPixel(x+14, y+10, color_1);
  tft.drawPixel(x+15, y+10, color_1);
  tft.drawPixel(x+16, y+10, color_2);

  tft.drawPixel(x+3, y+11, color_2);
  tft.drawPixel(x+4, y+11, color_1);
  tft.drawPixel(x+5, y+11, color_1);
  tft.drawPixel(x+6, y+11, color_1);
  tft.drawPixel(x+7, y+11, color_1);
  tft.drawPixel(x+8, y+11, color_1);
  tft.drawPixel(x+9, y+11, color_1);
  tft.drawPixel(x+10, y+11, color_1);
  tft.drawPixel(x+11, y+11, color_1);
  tft.drawPixel(x+12, y+11, color_1);
  tft.drawPixel(x+13, y+11, color_1);
  tft.drawPixel(x+14, y+11, color_1);
  tft.drawPixel(x+15, y+11, color_1);
  tft.drawPixel(x+16, y+11, color_2);

  tft.drawPixel(x+3, y+12, color_2);
  tft.drawPixel(x+4, y+12, color_2);
  tft.drawPixel(x+5, y+12, color_1);
  tft.drawPixel(x+6, y+12, color_1);
  tft.drawPixel(x+7, y+12, color_1);
  tft.drawPixel(x+8, y+12, color_1);
  tft.drawPixel(x+9, y+12, color_1);
  tft.drawPixel(x+10, y+12, color_1);
  tft.drawPixel(x+11, y+12, color_1);
  tft.drawPixel(x+12, y+12, color_1);
  tft.drawPixel(x+13, y+12, color_1);
  tft.drawPixel(x+14, y+12, color_1);
  tft.drawPixel(x+15, y+12, color_2);
  tft.drawPixel(x+16, y+12, color_2);

  tft.drawPixel(x+4, y+13, color_2);
  tft.drawPixel(x+5, y+13, color_2);
  tft.drawPixel(x+6, y+13, color_2);
  tft.drawPixel(x+7, y+13, color_2);
  tft.drawPixel(x+8, y+13, color_2);
  tft.drawPixel(x+9, y+13, color_2);
  tft.drawPixel(x+10, y+13, color_2);
  tft.drawPixel(x+11, y+13, color_2);
  tft.drawPixel(x+12, y+13, color_2);
  tft.drawPixel(x+13, y+13, color_2);
  tft.drawPixel(x+14, y+13, color_2);
  tft.drawPixel(x+15, y+13, color_2);

  tft.drawPixel(x+3, y+14, color_2);
  tft.drawPixel(x+4, y+14, color_2);
  tft.drawPixel(x+5, y+14, color_1);
  tft.drawPixel(x+6, y+14, color_1);
  tft.drawPixel(x+7, y+14, color_1);
  tft.drawPixel(x+8, y+14, color_1);
  tft.drawPixel(x+9, y+14, color_1);
  tft.drawPixel(x+10, y+14, color_1);
  tft.drawPixel(x+11, y+14, color_1);
  tft.drawPixel(x+12, y+14, color_1);
  tft.drawPixel(x+13, y+14, color_1);
  tft.drawPixel(x+14, y+14, color_1);
  tft.drawPixel(x+15, y+14, color_2);
  tft.drawPixel(x+16, y+14, color_2);

  tft.drawPixel(x+3, y+15, color_2);
  tft.drawPixel(x+4, y+15, color_1);
  tft.drawPixel(x+5, y+15, color_1);
  tft.drawPixel(x+6, y+15, color_1);
  tft.drawPixel(x+7, y+15, color_1);
  tft.drawPixel(x+8, y+15, color_1);
  tft.drawPixel(x+9, y+15, color_1);
  tft.drawPixel(x+10, y+15, color_1);
  tft.drawPixel(x+11, y+15, color_1);
  tft.drawPixel(x+12, y+15, color_1);
  tft.drawPixel(x+13, y+15, color_1);
  tft.drawPixel(x+14, y+15, color_1);
  tft.drawPixel(x+15, y+15, color_1);
  tft.drawPixel(x+16, y+15, color_2);

  tft.drawPixel(x+3, y+16, color_2);
  tft.drawPixel(x+4, y+16, color_2);
  tft.drawPixel(x+5, y+16, color_2);
  tft.drawPixel(x+6, y+16, color_2);
  tft.drawPixel(x+7, y+16, color_2);
  tft.drawPixel(x+8, y+16, color_2);
  tft.drawPixel(x+9, y+16, color_2);
  tft.drawPixel(x+10, y+16, color_2);
  tft.drawPixel(x+11, y+16, color_2);
  tft.drawPixel(x+12, y+16, color_2);
  tft.drawPixel(x+13, y+16, color_2);
  tft.drawPixel(x+14, y+16, color_2);
  tft.drawPixel(x+15, y+16, color_2);
  tft.drawPixel(x+16, y+16, color_2);

  tft.drawPixel(x+2, y+17, color_2);
  tft.drawPixel(x+3, y+17, color_1);
  tft.drawPixel(x+4, y+17, color_1);
  tft.drawPixel(x+5, y+17, color_1);
  tft.drawPixel(x+6, y+17, color_1);
  tft.drawPixel(x+7, y+17, color_1);
  tft.drawPixel(x+8, y+17, color_1);
  tft.drawPixel(x+9, y+17, color_1);
  tft.drawPixel(x+10, y+17, color_1);
  tft.drawPixel(x+11, y+17, color_1);
  tft.drawPixel(x+12, y+17, color_1);
  tft.drawPixel(x+13, y+17, color_1);
  tft.drawPixel(x+14, y+17, color_1);
  tft.drawPixel(x+15, y+17, color_1);
  tft.drawPixel(x+16, y+17, color_1);
  tft.drawPixel(x+17, y+17, color_2);

  tft.drawPixel(x+2, y+18, color_2);
  tft.drawPixel(x+3, y+18, color_1);
  tft.drawPixel(x+4, y+18, color_1);
  tft.drawPixel(x+5, y+18, color_1);
  tft.drawPixel(x+6, y+18, color_1);
  tft.drawPixel(x+7, y+18, color_1);
  tft.drawPixel(x+8, y+18, color_1);
  tft.drawPixel(x+9, y+18, color_1);
  tft.drawPixel(x+10, y+18, color_1);
  tft.drawPixel(x+11, y+18, color_1);
  tft.drawPixel(x+12, y+18, color_1);
  tft.drawPixel(x+13, y+18, color_1);
  tft.drawPixel(x+14, y+18, color_1);
  tft.drawPixel(x+15, y+18, color_1);
  tft.drawPixel(x+16, y+18, color_1);
  tft.drawPixel(x+17, y+18, color_2);

  tft.drawPixel(x+2, y+19, color_2);
  tft.drawPixel(x+3, y+19, color_2);
  tft.drawPixel(x+4, y+19, color_2);
  tft.drawPixel(x+5, y+19, color_2);
  tft.drawPixel(x+6, y+19, color_2);
  tft.drawPixel(x+7, y+19, color_2);
  tft.drawPixel(x+8, y+19, color_2);
  tft.drawPixel(x+9, y+19, color_2);
  tft.drawPixel(x+10, y+19, color_2);
  tft.drawPixel(x+11, y+19, color_2);
  tft.drawPixel(x+12, y+19, color_2);
  tft.drawPixel(x+13, y+19, color_2);
  tft.drawPixel(x+14, y+19, color_2);
  tft.drawPixel(x+15, y+19, color_2);
  tft.drawPixel(x+16, y+19, color_2);
  tft.drawPixel(x+17, y+19, color_2);
}


















void drawKing(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x+9, y+0, color_2);
  tft.drawPixel(x+10, y+0, color_2);

  tft.drawPixel(x+8, y+1, color_2);
  tft.drawPixel(x+9, y+1, color_2);
  tft.drawPixel(x+10, y+1, color_2);
  tft.drawPixel(x+11, y+1, color_2);

  tft.drawPixel(x+9, y+2, color_2);
  tft.drawPixel(x+10, y+2, color_2);

  tft.drawPixel(x+9, y+3, color_2);
  tft.drawPixel(x+10, y+3, color_2);

  tft.drawPixel(x+3, y+4, color_2);
  tft.drawPixel(x+4, y+4, color_2);
  tft.drawPixel(x+5, y+4, color_2);
  tft.drawPixel(x+6, y+4, color_2);
  tft.drawPixel(x+8, y+4, color_2);
  tft.drawPixel(x+9, y+4, color_2);
  tft.drawPixel(x+10, y+4, color_2);
  tft.drawPixel(x+11, y+4, color_2);
  tft.drawPixel(x+13, y+4, color_2);
  tft.drawPixel(x+14, y+4, color_2);
  tft.drawPixel(x+15, y+4, color_2);
  tft.drawPixel(x+16, y+4, color_2);

  tft.drawPixel(x+1, y+5, color_2);
  tft.drawPixel(x+2, y+5, color_2);
  tft.drawPixel(x+3, y+5, color_2);
  tft.drawPixel(x+4, y+5, color_1);
  tft.drawPixel(x+5, y+5, color_1);
  tft.drawPixel(x+6, y+5, color_2);
  tft.drawPixel(x+7, y+5, color_2);
  tft.drawPixel(x+8, y+5, color_2);
  tft.drawPixel(x+9, y+5, color_1);
  tft.drawPixel(x+10, y+5, color_1);
  tft.drawPixel(x+11, y+5, color_2);
  tft.drawPixel(x+12, y+5, color_2);
  tft.drawPixel(x+13, y+5, color_2);
  tft.drawPixel(x+14, y+5, color_1);
  tft.drawPixel(x+15, y+5, color_1);
  tft.drawPixel(x+16, y+5, color_2);
  tft.drawPixel(x+17, y+5, color_2);
  tft.drawPixel(x+18, y+5, color_2);

  tft.drawPixel(x+0, y+6, color_2);
  tft.drawPixel(x+1, y+6, color_2);
  tft.drawPixel(x+2, y+6, color_1);
  tft.drawPixel(x+3, y+6, color_1);
  tft.drawPixel(x+4, y+6, color_1);
  tft.drawPixel(x+5, y+6, color_1);
  tft.drawPixel(x+6, y+6, color_1);
  tft.drawPixel(x+7, y+6, color_2);
  tft.drawPixel(x+8, y+6, color_1);
  tft.drawPixel(x+9, y+6, color_1);
  tft.drawPixel(x+10, y+6, color_1);
  tft.drawPixel(x+11, y+6, color_1);
  tft.drawPixel(x+12, y+6, color_2);
  tft.drawPixel(x+13, y+6, color_1);
  tft.drawPixel(x+14, y+6, color_1);
  tft.drawPixel(x+15, y+6, color_1);
  tft.drawPixel(x+16, y+6, color_1);
  tft.drawPixel(x+17, y+6, color_1);
  tft.drawPixel(x+18, y+6, color_2);
  tft.drawPixel(x+19, y+6, color_2);

  tft.drawPixel(x+0, y+7, color_2);
  tft.drawPixel(x+1, y+7, color_1);
  tft.drawPixel(x+2, y+7, color_1);
  tft.drawPixel(x+3, y+7, color_1);
  tft.drawPixel(x+4, y+7, color_1);
  tft.drawPixel(x+5, y+7, color_1);
  tft.drawPixel(x+6, y+7, color_1);
  tft.drawPixel(x+7, y+7, color_2);
  tft.drawPixel(x+8, y+7, color_2);
  tft.drawPixel(x+9, y+7, color_1);
  tft.drawPixel(x+10, y+7, color_1);
  tft.drawPixel(x+11, y+7, color_2);
  tft.drawPixel(x+12, y+7, color_2);
  tft.drawPixel(x+13, y+7, color_1);
  tft.drawPixel(x+14, y+7, color_1);
  tft.drawPixel(x+15, y+7, color_1);
  tft.drawPixel(x+16, y+7, color_1);
  tft.drawPixel(x+17, y+7, color_1);
  tft.drawPixel(x+18, y+7, color_1);
  tft.drawPixel(x+19, y+7, color_2);

  tft.drawPixel(x+0, y+8, color_2);
  tft.drawPixel(x+1, y+8, color_1);
  tft.drawPixel(x+2, y+8, color_1);
  tft.drawPixel(x+3, y+8, color_1);
  tft.drawPixel(x+4, y+8, color_1);
  tft.drawPixel(x+5, y+8, color_1);
  tft.drawPixel(x+6, y+8, color_1);
  tft.drawPixel(x+7, y+8, color_1);
  tft.drawPixel(x+8, y+8, color_2);
  tft.drawPixel(x+9, y+8, color_2);
  tft.drawPixel(x+10, y+8, color_2);
  tft.drawPixel(x+11, y+8, color_2);
  tft.drawPixel(x+12, y+8, color_1);
  tft.drawPixel(x+13, y+8, color_1);
  tft.drawPixel(x+14, y+8, color_1);
  tft.drawPixel(x+15, y+8, color_1);
  tft.drawPixel(x+16, y+8, color_1);
  tft.drawPixel(x+17, y+8, color_1);
  tft.drawPixel(x+18, y+8, color_1);
  tft.drawPixel(x+19, y+8, color_2);

  tft.drawPixel(x+0, y+9, color_2);
  tft.drawPixel(x+1, y+9, color_1);
  tft.drawPixel(x+2, y+9, color_1);
  tft.drawPixel(x+3, y+9, color_1);
  tft.drawPixel(x+4, y+9, color_1);
  tft.drawPixel(x+5, y+9, color_1);
  tft.drawPixel(x+6, y+9, color_1);
  tft.drawPixel(x+7, y+9, color_1);
  tft.drawPixel(x+8, y+9, color_1);
  tft.drawPixel(x+9, y+9, color_2);
  tft.drawPixel(x+10, y+9, color_2);
  tft.drawPixel(x+11, y+9, color_1);
  tft.drawPixel(x+12, y+9, color_1);
  tft.drawPixel(x+13, y+9, color_1);
  tft.drawPixel(x+14, y+9, color_1);
  tft.drawPixel(x+15, y+9, color_1);
  tft.drawPixel(x+16, y+9, color_1);
  tft.drawPixel(x+17, y+9, color_1);
  tft.drawPixel(x+18, y+9, color_1);
  tft.drawPixel(x+19, y+9, color_2);

  tft.drawPixel(x+0, y+10, color_2);
  tft.drawPixel(x+1, y+10, color_2);
  tft.drawPixel(x+2, y+10, color_1);
  tft.drawPixel(x+3, y+10, color_1);
  tft.drawPixel(x+4, y+10, color_1);
  tft.drawPixel(x+5, y+10, color_1);
  tft.drawPixel(x+6, y+10, color_1);
  tft.drawPixel(x+7, y+10, color_1);
  tft.drawPixel(x+8, y+10, color_1);
  tft.drawPixel(x+9, y+10, color_2);
  tft.drawPixel(x+10, y+10, color_2);
  tft.drawPixel(x+11, y+10, color_1);
  tft.drawPixel(x+12, y+10, color_1);
  tft.drawPixel(x+13, y+10, color_1);
  tft.drawPixel(x+14, y+10, color_1);
  tft.drawPixel(x+15, y+10, color_1);
  tft.drawPixel(x+16, y+10, color_1);
  tft.drawPixel(x+17, y+10, color_1);
  tft.drawPixel(x+18, y+10, color_2);
  tft.drawPixel(x+19, y+10, color_2);

  tft.drawPixel(x+1, y+11, color_2);
  tft.drawPixel(x+2, y+11, color_1);
  tft.drawPixel(x+3, y+11, color_1);
  tft.drawPixel(x+4, y+11, color_1);
  tft.drawPixel(x+5, y+11, color_1);
  tft.drawPixel(x+6, y+11, color_1);
  tft.drawPixel(x+7, y+11, color_1);
  tft.drawPixel(x+8, y+11, color_1);
  tft.drawPixel(x+9, y+11, color_2);
  tft.drawPixel(x+10, y+11, color_2);
  tft.drawPixel(x+11, y+11, color_1);
  tft.drawPixel(x+12, y+11, color_1);
  tft.drawPixel(x+13, y+11, color_1);
  tft.drawPixel(x+14, y+11, color_1);
  tft.drawPixel(x+15, y+11, color_1);
  tft.drawPixel(x+16, y+11, color_1);
  tft.drawPixel(x+17, y+11, color_1);
  tft.drawPixel(x+18, y+11, color_2);

  tft.drawPixel(x+1, y+12, color_2);
  tft.drawPixel(x+2, y+12, color_2);
  tft.drawPixel(x+3, y+12, color_1);
  tft.drawPixel(x+4, y+12, color_1);
  tft.drawPixel(x+5, y+12, color_1);
  tft.drawPixel(x+6, y+12, color_1);
  tft.drawPixel(x+7, y+12, color_1);
  tft.drawPixel(x+8, y+12, color_1);
  tft.drawPixel(x+9, y+12, color_2);
  tft.drawPixel(x+10, y+12, color_2);
  tft.drawPixel(x+11, y+12, color_1);
  tft.drawPixel(x+12, y+12, color_1);
  tft.drawPixel(x+13, y+12, color_1);
  tft.drawPixel(x+14, y+12, color_1);
  tft.drawPixel(x+15, y+12, color_1);
  tft.drawPixel(x+16, y+12, color_1);
  tft.drawPixel(x+17, y+12, color_2);
  tft.drawPixel(x+18, y+12, color_2);

  tft.drawPixel(x+2, y+13, color_2);
  tft.drawPixel(x+3, y+13, color_2);
  tft.drawPixel(x+4, y+13, color_1);
  tft.drawPixel(x+5, y+13, color_1);
  tft.drawPixel(x+6, y+13, color_1);
  tft.drawPixel(x+7, y+13, color_1);
  tft.drawPixel(x+8, y+13, color_1);
  tft.drawPixel(x+9, y+13, color_2);
  tft.drawPixel(x+10, y+13, color_2);
  tft.drawPixel(x+11, y+13, color_1);
  tft.drawPixel(x+12, y+13, color_1);
  tft.drawPixel(x+13, y+13, color_1);
  tft.drawPixel(x+14, y+13, color_1);
  tft.drawPixel(x+15, y+13, color_1);
  tft.drawPixel(x+16, y+13, color_2);
  tft.drawPixel(x+17, y+13, color_2);

  tft.drawPixel(x+3, y+14, color_2);
  tft.drawPixel(x+4, y+14, color_2);
  tft.drawPixel(x+5, y+14, color_2);
  tft.drawPixel(x+6, y+14, color_2);
  tft.drawPixel(x+7, y+14, color_2);
  tft.drawPixel(x+8, y+14, color_2);
  tft.drawPixel(x+9, y+14, color_2);
  tft.drawPixel(x+10, y+14, color_2);
  tft.drawPixel(x+11, y+14, color_2);
  tft.drawPixel(x+12, y+14, color_2);
  tft.drawPixel(x+13, y+14, color_2);
  tft.drawPixel(x+14, y+14, color_2);
  tft.drawPixel(x+15, y+14, color_2);
  tft.drawPixel(x+16, y+14, color_2);

  tft.drawPixel(x+2, y+15, color_2);
  tft.drawPixel(x+3, y+15, color_2);
  tft.drawPixel(x+4, y+15, color_1);
  tft.drawPixel(x+5, y+15, color_1);
  tft.drawPixel(x+6, y+15, color_1);
  tft.drawPixel(x+7, y+15, color_1);
  tft.drawPixel(x+8, y+15, color_1);
  tft.drawPixel(x+9, y+15, color_1);
  tft.drawPixel(x+10, y+15, color_1);
  tft.drawPixel(x+11, y+15, color_1);
  tft.drawPixel(x+12, y+15, color_1);
  tft.drawPixel(x+13, y+15, color_1);
  tft.drawPixel(x+14, y+15, color_1);
  tft.drawPixel(x+15, y+15, color_1);
  tft.drawPixel(x+16, y+15, color_2);
  tft.drawPixel(x+17, y+15, color_2);

  tft.drawPixel(x+2, y+16, color_2);
  tft.drawPixel(x+3, y+16, color_1);
  tft.drawPixel(x+4, y+16, color_1);
  tft.drawPixel(x+5, y+16, color_2);
  tft.drawPixel(x+6, y+16, color_2);
  tft.drawPixel(x+7, y+16, color_2);
  tft.drawPixel(x+8, y+16, color_2);
  tft.drawPixel(x+9, y+16, color_2);
  tft.drawPixel(x+10, y+16, color_2);
  tft.drawPixel(x+11, y+16, color_2);
  tft.drawPixel(x+12, y+16, color_2);
  tft.drawPixel(x+13, y+16, color_2);
  tft.drawPixel(x+14, y+16, color_2);
  tft.drawPixel(x+15, y+16, color_1);
  tft.drawPixel(x+16, y+16, color_1);
  tft.drawPixel(x+17, y+16, color_2);

  tft.drawPixel(x+2, y+17, color_2);
  tft.drawPixel(x+3, y+17, color_2);
  tft.drawPixel(x+4, y+17, color_2);
  tft.drawPixel(x+5, y+17, color_1);
  tft.drawPixel(x+6, y+17, color_1);
  tft.drawPixel(x+7, y+17, color_1);
  tft.drawPixel(x+8, y+17, color_1);
  tft.drawPixel(x+9, y+17, color_1);
  tft.drawPixel(x+10, y+17, color_1);
  tft.drawPixel(x+11, y+17, color_1);
  tft.drawPixel(x+12, y+17, color_1);
  tft.drawPixel(x+13, y+17, color_1);
  tft.drawPixel(x+14, y+17, color_1);
  tft.drawPixel(x+15, y+17, color_2);
  tft.drawPixel(x+16, y+17, color_2);
  tft.drawPixel(x+17, y+17, color_2);

  tft.drawPixel(x+2, y+18, color_2);
  tft.drawPixel(x+3, y+18, color_1);
  tft.drawPixel(x+4, y+18, color_1);
  tft.drawPixel(x+5, y+18, color_1);
  tft.drawPixel(x+6, y+18, color_1);
  tft.drawPixel(x+7, y+18, color_1);
  tft.drawPixel(x+8, y+18, color_1);
  tft.drawPixel(x+9, y+18, color_1);
  tft.drawPixel(x+10, y+18, color_1);
  tft.drawPixel(x+11, y+18, color_1);
  tft.drawPixel(x+12, y+18, color_1);
  tft.drawPixel(x+13, y+18, color_1);
  tft.drawPixel(x+14, y+18, color_1);
  tft.drawPixel(x+15, y+18, color_1);
  tft.drawPixel(x+16, y+18, color_1);
  tft.drawPixel(x+17, y+18, color_2);

  tft.drawPixel(x+2, y+19, color_2);
  tft.drawPixel(x+3, y+19, color_2);
  tft.drawPixel(x+4, y+19, color_2);
  tft.drawPixel(x+5, y+19, color_2);
  tft.drawPixel(x+6, y+19, color_2);
  tft.drawPixel(x+7, y+19, color_2);
  tft.drawPixel(x+8, y+19, color_2);
  tft.drawPixel(x+9, y+19, color_2);
  tft.drawPixel(x+10, y+19, color_2);
  tft.drawPixel(x+11, y+19, color_2);
  tft.drawPixel(x+12, y+19, color_2);
  tft.drawPixel(x+13, y+19, color_2);
  tft.drawPixel(x+14, y+19, color_2);
  tft.drawPixel(x+15, y+19, color_2);
  tft.drawPixel(x+16, y+19, color_2);
  tft.drawPixel(x+17, y+19, color_2);
}


















void drawRook(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x+2, y+0, color_2);
  tft.drawPixel(x+3, y+0, color_2);
  tft.drawPixel(x+4, y+0, color_2);
  tft.drawPixel(x+5, y+0, color_2);
  tft.drawPixel(x+8, y+0, color_2);
  tft.drawPixel(x+9, y+0, color_2);
  tft.drawPixel(x+10, y+0, color_2);
  tft.drawPixel(x+11, y+0, color_2);
  tft.drawPixel(x+14, y+0, color_2);
  tft.drawPixel(x+15, y+0, color_2);
  tft.drawPixel(x+16, y+0, color_2);
  tft.drawPixel(x+17, y+0, color_2);

  tft.drawPixel(x+2, y+1, color_2);
  tft.drawPixel(x+3, y+1, color_1);
  tft.drawPixel(x+4, y+1, color_1);
  tft.drawPixel(x+5, y+1, color_2);
  tft.drawPixel(x+8, y+1, color_2);
  tft.drawPixel(x+9, y+1, color_1);
  tft.drawPixel(x+10, y+1, color_1);
  tft.drawPixel(x+11, y+1, color_2);
  tft.drawPixel(x+14, y+1, color_2);
  tft.drawPixel(x+15, y+1, color_1);
  tft.drawPixel(x+16, y+1, color_1);
  tft.drawPixel(x+17, y+1, color_2);

  tft.drawPixel(x+2, y+2, color_2);
  tft.drawPixel(x+3, y+2, color_1);
  tft.drawPixel(x+4, y+2, color_1);
  tft.drawPixel(x+5, y+2, color_2);
  tft.drawPixel(x+6, y+2, color_2);
  tft.drawPixel(x+7, y+2, color_2);
  tft.drawPixel(x+8, y+2, color_2);
  tft.drawPixel(x+9, y+2, color_1);
  tft.drawPixel(x+10, y+2, color_1);
  tft.drawPixel(x+11, y+2, color_2);
  tft.drawPixel(x+12, y+2, color_2);
  tft.drawPixel(x+13, y+2, color_2);
  tft.drawPixel(x+14, y+2, color_2);
  tft.drawPixel(x+15, y+2, color_1);
  tft.drawPixel(x+16, y+2, color_1);
  tft.drawPixel(x+17, y+2, color_2);

  tft.drawPixel(x+2, y+3, color_2);
  tft.drawPixel(x+3, y+3, color_1);
  tft.drawPixel(x+4, y+3, color_1);
  tft.drawPixel(x+5, y+3, color_1);
  tft.drawPixel(x+6, y+3, color_1);
  tft.drawPixel(x+7, y+3, color_1);
  tft.drawPixel(x+8, y+3, color_1);
  tft.drawPixel(x+9, y+3, color_1);
  tft.drawPixel(x+10, y+3, color_1);
  tft.drawPixel(x+11, y+3, color_1);
  tft.drawPixel(x+12, y+3, color_1);
  tft.drawPixel(x+13, y+3, color_1);
  tft.drawPixel(x+14, y+3, color_1);
  tft.drawPixel(x+15, y+3, color_1);
  tft.drawPixel(x+16, y+3, color_1);
  tft.drawPixel(x+17, y+3, color_2);

  tft.drawPixel(x+2, y+4, color_2);
  tft.drawPixel(x+3, y+4, color_2);
  tft.drawPixel(x+4, y+4, color_1);
  tft.drawPixel(x+5, y+4, color_1);
  tft.drawPixel(x+6, y+4, color_1);
  tft.drawPixel(x+7, y+4, color_1);
  tft.drawPixel(x+8, y+4, color_1);
  tft.drawPixel(x+9, y+4, color_1);
  tft.drawPixel(x+10, y+4, color_1);
  tft.drawPixel(x+11, y+4, color_1);
  tft.drawPixel(x+12, y+4, color_1);
  tft.drawPixel(x+13, y+4, color_1);
  tft.drawPixel(x+14, y+4, color_1);
  tft.drawPixel(x+15, y+4, color_1);
  tft.drawPixel(x+16, y+4, color_2);
  tft.drawPixel(x+17, y+4, color_2);

  tft.drawPixel(x+3, y+5, color_2);
  tft.drawPixel(x+4, y+5, color_2);
  tft.drawPixel(x+5, y+5, color_1);
  tft.drawPixel(x+6, y+5, color_1);
  tft.drawPixel(x+7, y+5, color_1);
  tft.drawPixel(x+8, y+5, color_1);
  tft.drawPixel(x+9, y+5, color_1);
  tft.drawPixel(x+10, y+5, color_1);
  tft.drawPixel(x+11, y+5, color_1);
  tft.drawPixel(x+12, y+5, color_1);
  tft.drawPixel(x+13, y+5, color_1);
  tft.drawPixel(x+14, y+5, color_1);
  tft.drawPixel(x+15, y+5, color_2);
  tft.drawPixel(x+16, y+5, color_2);

  tft.drawPixel(x+4, y+6, color_2);
  tft.drawPixel(x+5, y+6, color_2);
  tft.drawPixel(x+6, y+6, color_2);
  tft.drawPixel(x+7, y+6, color_2);
  tft.drawPixel(x+8, y+6, color_2);
  tft.drawPixel(x+9, y+6, color_2);
  tft.drawPixel(x+10, y+6, color_2);
  tft.drawPixel(x+11, y+6, color_2);
  tft.drawPixel(x+12, y+6, color_2);
  tft.drawPixel(x+13, y+6, color_2);
  tft.drawPixel(x+14, y+6, color_2);
  tft.drawPixel(x+15, y+6, color_2);

  tft.drawPixel(x+5, y+7, color_2);
  tft.drawPixel(x+6, y+7, color_1);
  tft.drawPixel(x+7, y+7, color_1);
  tft.drawPixel(x+8, y+7, color_1);
  tft.drawPixel(x+9, y+7, color_1);
  tft.drawPixel(x+10, y+7, color_1);
  tft.drawPixel(x+11, y+7, color_1);
  tft.drawPixel(x+12, y+7, color_1);
  tft.drawPixel(x+13, y+7, color_1);
  tft.drawPixel(x+14, y+7, color_2);

  tft.drawPixel(x+5, y+8, color_2);
  tft.drawPixel(x+6, y+8, color_1);
  tft.drawPixel(x+7, y+8, color_1);
  tft.drawPixel(x+8, y+8, color_1);
  tft.drawPixel(x+9, y+8, color_1);
  tft.drawPixel(x+10, y+8, color_1);
  tft.drawPixel(x+11, y+8, color_1);
  tft.drawPixel(x+12, y+8, color_1);
  tft.drawPixel(x+13, y+8, color_1);
  tft.drawPixel(x+14, y+8, color_2);

  tft.drawPixel(x+5, y+9, color_2);
  tft.drawPixel(x+6, y+9, color_1);
  tft.drawPixel(x+7, y+9, color_1);
  tft.drawPixel(x+8, y+9, color_1);
  tft.drawPixel(x+9, y+9, color_1);
  tft.drawPixel(x+10, y+9, color_1);
  tft.drawPixel(x+11, y+9, color_1);
  tft.drawPixel(x+12, y+9, color_1);
  tft.drawPixel(x+13, y+9, color_1);
  tft.drawPixel(x+14, y+9, color_2);

  tft.drawPixel(x+5, y+10, color_2);
  tft.drawPixel(x+6, y+10, color_1);
  tft.drawPixel(x+7, y+10, color_1);
  tft.drawPixel(x+8, y+10, color_1);
  tft.drawPixel(x+9, y+10, color_1);
  tft.drawPixel(x+10, y+10, color_1);
  tft.drawPixel(x+11, y+10, color_1);
  tft.drawPixel(x+12, y+10, color_1);
  tft.drawPixel(x+13, y+10, color_1);
  tft.drawPixel(x+14, y+10, color_2);

  tft.drawPixel(x+5, y+11, color_2);
  tft.drawPixel(x+6, y+11, color_1);
  tft.drawPixel(x+7, y+11, color_1);
  tft.drawPixel(x+8, y+11, color_1);
  tft.drawPixel(x+9, y+11, color_1);
  tft.drawPixel(x+10, y+11, color_1);
  tft.drawPixel(x+11, y+11, color_1);
  tft.drawPixel(x+12, y+11, color_1);
  tft.drawPixel(x+13, y+11, color_1);
  tft.drawPixel(x+14, y+11, color_2);

  tft.drawPixel(x+5, y+12, color_2);
  tft.drawPixel(x+6, y+12, color_1);
  tft.drawPixel(x+7, y+12, color_1);
  tft.drawPixel(x+8, y+12, color_1);
  tft.drawPixel(x+9, y+12, color_1);
  tft.drawPixel(x+10, y+12, color_1);
  tft.drawPixel(x+11, y+12, color_1);
  tft.drawPixel(x+12, y+12, color_1);
  tft.drawPixel(x+13, y+12, color_1);
  tft.drawPixel(x+14, y+12, color_2);

  tft.drawPixel(x+5, y+13, color_2);
  tft.drawPixel(x+6, y+13, color_1);
  tft.drawPixel(x+7, y+13, color_1);
  tft.drawPixel(x+8, y+13, color_1);
  tft.drawPixel(x+9, y+13, color_1);
  tft.drawPixel(x+10, y+13, color_1);
  tft.drawPixel(x+11, y+13, color_1);
  tft.drawPixel(x+12, y+13, color_1);
  tft.drawPixel(x+13, y+13, color_1);
  tft.drawPixel(x+14, y+13, color_2);

  tft.drawPixel(x+4, y+14, color_2);
  tft.drawPixel(x+5, y+14, color_2);
  tft.drawPixel(x+6, y+14, color_2);
  tft.drawPixel(x+7, y+14, color_2);
  tft.drawPixel(x+8, y+14, color_2);
  tft.drawPixel(x+9, y+14, color_2);
  tft.drawPixel(x+10, y+14, color_2);
  tft.drawPixel(x+11, y+14, color_2);
  tft.drawPixel(x+12, y+14, color_2);
  tft.drawPixel(x+13, y+14, color_2);
  tft.drawPixel(x+14, y+14, color_2);
  tft.drawPixel(x+15, y+14, color_2);

  tft.drawPixel(x+3, y+15, color_2);
  tft.drawPixel(x+4, y+15, color_2);
  tft.drawPixel(x+5, y+15, color_1);
  tft.drawPixel(x+6, y+15, color_1);
  tft.drawPixel(x+7, y+15, color_1);
  tft.drawPixel(x+8, y+15, color_1);
  tft.drawPixel(x+9, y+15, color_1);
  tft.drawPixel(x+10, y+15, color_1);
  tft.drawPixel(x+11, y+15, color_1);
  tft.drawPixel(x+12, y+15, color_1);
  tft.drawPixel(x+13, y+15, color_1);
  tft.drawPixel(x+14, y+15, color_1);
  tft.drawPixel(x+15, y+15, color_2);
  tft.drawPixel(x+16, y+15, color_2);

  tft.drawPixel(x+2, y+16, color_2);
  tft.drawPixel(x+3, y+16, color_2);
  tft.drawPixel(x+4, y+16, color_1);
  tft.drawPixel(x+5, y+16, color_1);
  tft.drawPixel(x+6, y+16, color_1);
  tft.drawPixel(x+7, y+16, color_1);
  tft.drawPixel(x+8, y+16, color_1);
  tft.drawPixel(x+9, y+16, color_1);
  tft.drawPixel(x+10, y+16, color_1);
  tft.drawPixel(x+11, y+16, color_1);
  tft.drawPixel(x+12, y+16, color_1);
  tft.drawPixel(x+13, y+16, color_1);
  tft.drawPixel(x+14, y+16, color_1);
  tft.drawPixel(x+15, y+16, color_1);
  tft.drawPixel(x+16, y+16, color_2);
  tft.drawPixel(x+17, y+16, color_2);

  tft.drawPixel(x+1, y+17, color_2);
  tft.drawPixel(x+2, y+17, color_2);
  tft.drawPixel(x+3, y+17, color_2);
  tft.drawPixel(x+4, y+17, color_2);
  tft.drawPixel(x+5, y+17, color_2);
  tft.drawPixel(x+6, y+17, color_2);
  tft.drawPixel(x+7, y+17, color_2);
  tft.drawPixel(x+8, y+17, color_2);
  tft.drawPixel(x+9, y+17, color_2);
  tft.drawPixel(x+10, y+17, color_2);
  tft.drawPixel(x+11, y+17, color_2);
  tft.drawPixel(x+12, y+17, color_2);
  tft.drawPixel(x+13, y+17, color_2);
  tft.drawPixel(x+14, y+17, color_2);
  tft.drawPixel(x+15, y+17, color_2);
  tft.drawPixel(x+16, y+17, color_2);
  tft.drawPixel(x+17, y+17, color_2);
  tft.drawPixel(x+18, y+17, color_2);

  tft.drawPixel(x+1, y+18, color_2);
  tft.drawPixel(x+2, y+18, color_1);
  tft.drawPixel(x+3, y+18, color_1);
  tft.drawPixel(x+4, y+18, color_1);
  tft.drawPixel(x+5, y+18, color_1);
  tft.drawPixel(x+6, y+18, color_1);
  tft.drawPixel(x+7, y+18, color_1);
  tft.drawPixel(x+8, y+18, color_1);
  tft.drawPixel(x+9, y+18, color_1);
  tft.drawPixel(x+10, y+18, color_1);
  tft.drawPixel(x+11, y+18, color_1);
  tft.drawPixel(x+12, y+18, color_1);
  tft.drawPixel(x+13, y+18, color_1);
  tft.drawPixel(x+14, y+18, color_1);
  tft.drawPixel(x+15, y+18, color_1);
  tft.drawPixel(x+16, y+18, color_1);
  tft.drawPixel(x+17, y+18, color_1);
  tft.drawPixel(x+18, y+18, color_2);

  tft.drawPixel(x+1, y+19, color_2);
  tft.drawPixel(x+2, y+19, color_2);
  tft.drawPixel(x+3, y+19, color_2);
  tft.drawPixel(x+4, y+19, color_2);
  tft.drawPixel(x+5, y+19, color_2);
  tft.drawPixel(x+6, y+19, color_2);
  tft.drawPixel(x+7, y+19, color_2);
  tft.drawPixel(x+8, y+19, color_2);
  tft.drawPixel(x+9, y+19, color_2);
  tft.drawPixel(x+10, y+19, color_2);
  tft.drawPixel(x+11, y+19, color_2);
  tft.drawPixel(x+12, y+19, color_2);
  tft.drawPixel(x+13, y+19, color_2);
  tft.drawPixel(x+14, y+19, color_2);
  tft.drawPixel(x+15, y+19, color_2);
  tft.drawPixel(x+16, y+19, color_2);
  tft.drawPixel(x+17, y+19, color_2);
  tft.drawPixel(x+18, y+19, color_2);
}

















void drawBishop(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x+9, y+0, color_2);
  tft.drawPixel(x+10, y+0, color_2);

  tft.drawPixel(x+8, y+1, color_2);
  tft.drawPixel(x+9, y+1, color_1);
  tft.drawPixel(x+10, y+1, color_1);
  tft.drawPixel(x+11, y+1, color_2);

  tft.drawPixel(x+9, y+2, color_2);
  tft.drawPixel(x+10, y+2, color_2);

  tft.drawPixel(x+7, y+3, color_2);
  tft.drawPixel(x+8, y+3, color_2);
  tft.drawPixel(x+9, y+3, color_1);
  tft.drawPixel(x+10, y+3, color_1);
  tft.drawPixel(x+11, y+3, color_2);
  tft.drawPixel(x+12, y+3, color_2);

  tft.drawPixel(x+7, y+4, color_2);
  tft.drawPixel(x+8, y+4, color_1);
  tft.drawPixel(x+9, y+4, color_1);
  tft.drawPixel(x+10, y+4, color_2);
  tft.drawPixel(x+11, y+4, color_2);
  tft.drawPixel(x+12, y+4, color_2);

  tft.drawPixel(x+6, y+5, color_2);
  tft.drawPixel(x+7, y+5, color_2);
  tft.drawPixel(x+8, y+5, color_1);
  tft.drawPixel(x+9, y+5, color_1);
  tft.drawPixel(x+10, y+5, color_2);
  tft.drawPixel(x+11, y+5, color_2);
  tft.drawPixel(x+12, y+5, color_2);
  tft.drawPixel(x+13, y+5, color_2);

  tft.drawPixel(x+6, y+6, color_2);
  tft.drawPixel(x+7, y+6, color_1);
  tft.drawPixel(x+8, y+6, color_1);
  tft.drawPixel(x+9, y+6, color_1);
  tft.drawPixel(x+10, y+6, color_2);
  tft.drawPixel(x+11, y+6, color_2);
  tft.drawPixel(x+12, y+6, color_1);
  tft.drawPixel(x+13, y+6, color_2);

  tft.drawPixel(x+6, y+7, color_2);
  tft.drawPixel(x+7, y+7, color_1);
  tft.drawPixel(x+8, y+7, color_1);
  tft.drawPixel(x+9, y+7, color_2);
  tft.drawPixel(x+10, y+7, color_2);
  tft.drawPixel(x+11, y+7, color_1);
  tft.drawPixel(x+12, y+7, color_1);
  tft.drawPixel(x+13, y+7, color_2);

  tft.drawPixel(x+6, y+8, color_2);
  tft.drawPixel(x+7, y+8, color_1);
  tft.drawPixel(x+8, y+8, color_1);
  tft.drawPixel(x+9, y+8, color_2);
  tft.drawPixel(x+10, y+8, color_2);
  tft.drawPixel(x+11, y+8, color_1);
  tft.drawPixel(x+12, y+8, color_1);
  tft.drawPixel(x+13, y+8, color_2);

  tft.drawPixel(x+6, y+9, color_2);
  tft.drawPixel(x+7, y+9, color_1);
  tft.drawPixel(x+8, y+9, color_1);
  tft.drawPixel(x+9, y+9, color_1);
  tft.drawPixel(x+10, y+9, color_1);
  tft.drawPixel(x+11, y+9, color_1);
  tft.drawPixel(x+12, y+9, color_1);
  tft.drawPixel(x+13, y+9, color_2);

  tft.drawPixel(x+6, y+10, color_2);
  tft.drawPixel(x+7, y+10, color_1);
  tft.drawPixel(x+8, y+10, color_1);
  tft.drawPixel(x+9, y+10, color_1);
  tft.drawPixel(x+10, y+10, color_1);
  tft.drawPixel(x+11, y+10, color_1);
  tft.drawPixel(x+12, y+10, color_1);
  tft.drawPixel(x+13, y+10, color_2);

  tft.drawPixel(x+6, y+11, color_2);
  tft.drawPixel(x+7, y+11, color_1);
  tft.drawPixel(x+8, y+11, color_1);
  tft.drawPixel(x+9, y+11, color_1);
  tft.drawPixel(x+10, y+11, color_1);
  tft.drawPixel(x+11, y+11, color_1);
  tft.drawPixel(x+12, y+11, color_1);
  tft.drawPixel(x+13, y+11, color_2);

  tft.drawPixel(x+6, y+12, color_2);
  tft.drawPixel(x+7, y+12, color_2);
  tft.drawPixel(x+8, y+12, color_1);
  tft.drawPixel(x+9, y+12, color_1);
  tft.drawPixel(x+10, y+12, color_1);
  tft.drawPixel(x+11, y+12, color_1);
  tft.drawPixel(x+12, y+12, color_2);
  tft.drawPixel(x+13, y+12, color_2);

  tft.drawPixel(x+7, y+13, color_2);
  tft.drawPixel(x+8, y+13, color_2);
  tft.drawPixel(x+9, y+13, color_1);
  tft.drawPixel(x+10, y+13, color_1);
  tft.drawPixel(x+11, y+13, color_2);
  tft.drawPixel(x+12, y+13, color_2);

  tft.drawPixel(x+8, y+14, color_2);
  tft.drawPixel(x+9, y+14, color_1);
  tft.drawPixel(x+10, y+14, color_1);
  tft.drawPixel(x+11, y+14, color_2);

  tft.drawPixel(x+7, y+15, color_2);
  tft.drawPixel(x+8, y+15, color_2);
  tft.drawPixel(x+9, y+15, color_1);
  tft.drawPixel(x+10, y+15, color_1);
  tft.drawPixel(x+11, y+15, color_2);
  tft.drawPixel(x+12, y+15, color_2);

  tft.drawPixel(x+6, y+16, color_2);
  tft.drawPixel(x+7, y+16, color_2);
  tft.drawPixel(x+8, y+16, color_1);
  tft.drawPixel(x+9, y+16, color_1);
  tft.drawPixel(x+10, y+16, color_1);
  tft.drawPixel(x+11, y+16, color_1);
  tft.drawPixel(x+12, y+16, color_2);
  tft.drawPixel(x+13, y+16, color_2);

  tft.drawPixel(x+3, y+17, color_2);
  tft.drawPixel(x+4, y+17, color_2);
  tft.drawPixel(x+5, y+17, color_2);
  tft.drawPixel(x+6, y+17, color_2);
  tft.drawPixel(x+7, y+17, color_2);
  tft.drawPixel(x+8, y+17, color_2);
  tft.drawPixel(x+9, y+17, color_2);
  tft.drawPixel(x+10, y+17, color_2);
  tft.drawPixel(x+11, y+17, color_2);
  tft.drawPixel(x+12, y+17, color_2);
  tft.drawPixel(x+13, y+17, color_2);
  tft.drawPixel(x+14, y+17, color_2);
  tft.drawPixel(x+15, y+17, color_2);
  tft.drawPixel(x+16, y+17, color_2);

  tft.drawPixel(x+3, y+18, color_2);
  tft.drawPixel(x+4, y+18, color_1);
  tft.drawPixel(x+5, y+18, color_1);
  tft.drawPixel(x+6, y+18, color_1);
  tft.drawPixel(x+7, y+18, color_1);
  tft.drawPixel(x+8, y+18, color_1);
  tft.drawPixel(x+9, y+18, color_1);
  tft.drawPixel(x+10, y+18, color_1);
  tft.drawPixel(x+11, y+18, color_1);
  tft.drawPixel(x+12, y+18, color_1);
  tft.drawPixel(x+13, y+18, color_1);
  tft.drawPixel(x+14, y+18, color_1);
  tft.drawPixel(x+15, y+18, color_1);
  tft.drawPixel(x+16, y+18, color_2);

  tft.drawPixel(x+3, y+19, color_2);
  tft.drawPixel(x+4, y+19, color_2);
  tft.drawPixel(x+5, y+19, color_2);
  tft.drawPixel(x+6, y+19, color_2);
  tft.drawPixel(x+7, y+19, color_2);
  tft.drawPixel(x+8, y+19, color_2);
  tft.drawPixel(x+9, y+19, color_2);
  tft.drawPixel(x+10, y+19, color_2);
  tft.drawPixel(x+11, y+19, color_2);
  tft.drawPixel(x+12, y+19, color_2);
  tft.drawPixel(x+13, y+19, color_2);
  tft.drawPixel(x+14, y+19, color_2);
  tft.drawPixel(x+15, y+19, color_2);
  tft.drawPixel(x+16, y+19, color_2);
}





















































































































































void drawKnight2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x-7, y, color_2);
  tft.drawPixel(x-9, y, color_2);
  
  tft.drawPixel(x-6, y-1, color_2);
  tft.drawPixel(x-7, y-1, color_1);
  tft.drawPixel(x-8, y-1, color_2);
  tft.drawPixel(x-9, y-1, color_1);
  tft.drawPixel(x-10, y-1, color_2);
  
  tft.drawPixel(x-5, y-2, color_2);
  tft.drawPixel(x-6, y-2, color_1);
  tft.drawPixel(x-7, y-2, color_1);
  tft.drawPixel(x-8, y-2, color_1);
  tft.drawPixel(x-9, y-2, color_1);
  tft.drawPixel(x-10, y-2, color_1);
  tft.drawPixel(x-11, y-2, color_2);

  tft.drawPixel(x-4, y-3, color_2);
  tft.drawPixel(x-5, y-3, color_1);
  tft.drawPixel(x-6, y-3, color_1);
  tft.drawPixel(x-7, y-3, color_1);
  tft.drawPixel(x-8, y-3, color_1);
  tft.drawPixel(x-9, y-3, color_1);
  tft.drawPixel(x-10, y-3, color_1);
  tft.drawPixel(x-11, y-3, color_1);
  tft.drawPixel(x-12, y-3, color_2);

  tft.drawPixel(x-4, y-4, color_2);
  tft.drawPixel(x-5, y-4, color_1);
  tft.drawPixel(x-6, y-4, color_1);
  tft.drawPixel(x-7, y-4, color_1);
  tft.drawPixel(x-8, y-4, color_1);
  tft.drawPixel(x-9, y-4, color_1);
  tft.drawPixel(x-10, y-4, color_1);
  tft.drawPixel(x-11, y-4, color_1);
  tft.drawPixel(x-12, y-4, color_1);
  tft.drawPixel(x-13, y-4, color_2);

  tft.drawPixel(x-3, y-5, color_2);
  tft.drawPixel(x-4, y-5, color_1);
  tft.drawPixel(x-5, y-5, color_1);
  tft.drawPixel(x-6, y-5, color_1);
  tft.drawPixel(x-7, y-5, color_1);
  tft.drawPixel(x-8, y-5, color_1);
  tft.drawPixel(x-9, y-5, color_1);
  tft.drawPixel(x-10, y-5, color_1);
  tft.drawPixel(x-11, y-5, color_1);
  tft.drawPixel(x-12, y-5, color_2);
  tft.drawPixel(x-13, y-5, color_1);
  tft.drawPixel(x-14, y-5, color_2);

  tft.drawPixel(x-3, y-6, color_2);
  tft.drawPixel(x-4, y-6, color_1);
  tft.drawPixel(x-5, y-6, color_2);
  tft.drawPixel(x-6, y-6, color_1);
  tft.drawPixel(x-7, y-6, color_1);
  tft.drawPixel(x-8, y-6, color_1);
  tft.drawPixel(x-9, y-6, color_1);
  tft.drawPixel(x-10, y-6, color_1);
  tft.drawPixel(x-11, y-6, color_1);
  tft.drawPixel(x-12, y-6, color_1);
  tft.drawPixel(x-13, y-6, color_2);
  tft.drawPixel(x-14, y-6, color_1);
  tft.drawPixel(x-15, y-6, color_2);

  tft.drawPixel(x-2, y-7, color_2);
  tft.drawPixel(x-3, y-7, color_1);
  tft.drawPixel(x-4, y-7, color_1);
  tft.drawPixel(x-5, y-7, color_1);
  tft.drawPixel(x-6, y-7, color_1);
  tft.drawPixel(x-7, y-7, color_1);
  tft.drawPixel(x-8, y-7, color_1);
  tft.drawPixel(x-9, y-7, color_1);
  tft.drawPixel(x-10, y-7, color_1);
  tft.drawPixel(x-11, y-7, color_1);
  tft.drawPixel(x-12, y-7, color_1);
  tft.drawPixel(x-13, y-7, color_2);
  tft.drawPixel(x-14, y-7, color_1);
  tft.drawPixel(x-15, y-7, color_2);

  tft.drawPixel(x-1, y-8, color_2);
  tft.drawPixel(x-2, y-8, color_2);
  tft.drawPixel(x-3, y-8, color_1);
  tft.drawPixel(x-4, y-8, color_1);
  tft.drawPixel(x-5, y-8, color_1);
  tft.drawPixel(x-6, y-8, color_1);
  tft.drawPixel(x-7, y-8, color_1);
  tft.drawPixel(x-8, y-8, color_1);
  tft.drawPixel(x-9, y-8, color_2);
  tft.drawPixel(x-10, y-8, color_1);
  tft.drawPixel(x-11, y-8, color_1);
  tft.drawPixel(x-12, y-8, color_1);
  tft.drawPixel(x-13, y-8, color_1);
  tft.drawPixel(x-14, y-8, color_2);
  tft.drawPixel(x-15, y-8, color_1);
  tft.drawPixel(x-16, y-8, color_2);

  tft.drawPixel(x, y-9, color_2);
  tft.drawPixel(x-1, y-9, color_2);
  tft.drawPixel(x-2, y-9, color_1);
  tft.drawPixel(x-3, y-9, color_1);
  tft.drawPixel(x-4, y-9, color_1);
  tft.drawPixel(x-5, y-9, color_1);
  tft.drawPixel(x-6, y-9, color_1);
  tft.drawPixel(x-7, y-9, color_1);
  tft.drawPixel(x-8, y-9, color_2);
  tft.drawPixel(x-9, y-9, color_2);
  tft.drawPixel(x-10, y-9, color_2);
  tft.drawPixel(x-11, y-9, color_1);
  tft.drawPixel(x-12, y-9, color_1);
  tft.drawPixel(x-13, y-9, color_1);
  tft.drawPixel(x-14, y-9, color_2);
  tft.drawPixel(x-15, y-9, color_1);
  tft.drawPixel(x-16, y-9, color_2);

  tft.drawPixel(x, y-10, color_2);
  tft.drawPixel(x-1, y-10, color_1);
  tft.drawPixel(x-2, y-10, color_1);
  tft.drawPixel(x-3, y-10, color_1);
  tft.drawPixel(x-4, y-10, color_1);
  tft.drawPixel(x-5, y-10, color_1);
  tft.drawPixel(x-6, y-10, color_2);
  tft.drawPixel(x-7, y-10, color_2);
  //tft.drawPixel(x-8, y-10, color_2);
  //tft.drawPixel(x-9, y-10, color_2);
  tft.drawPixel(x-10, y-10, color_2);
  tft.drawPixel(x-11, y-10, color_1);
  tft.drawPixel(x-12, y-10, color_1);
  tft.drawPixel(x-13, y-10, color_1);
  tft.drawPixel(x-14, y-10, color_1);
  tft.drawPixel(x-15, y-10, color_2);
  tft.drawPixel(x-16, y-10, color_1);
  tft.drawPixel(x-17, y-10, color_2);

  //tft.drawPixel(x, y-11, color_2);
  tft.drawPixel(x-1, y-11, color_2);
  tft.drawPixel(x-2, y-11, color_1);
  tft.drawPixel(x-3, y-11, color_2);
  tft.drawPixel(x-4, y-11, color_1);
  tft.drawPixel(x-5, y-11, color_2);
  //tft.drawPixel(x-6, y-11, color_2);
  //tft.drawPixel(x-7, y-11, color_2);
  //tft.drawPixel(x-8, y-11, color_2);
  tft.drawPixel(x-9, y-11, color_2);
  tft.drawPixel(x-10, y-11, color_1);
  tft.drawPixel(x-11, y-11, color_1);
  tft.drawPixel(x-12, y-11, color_1);
  tft.drawPixel(x-13, y-11, color_1);
  tft.drawPixel(x-14, y-11, color_1);
  tft.drawPixel(x-15, y-11, color_2);
  tft.drawPixel(x-16, y-11, color_1);
  tft.drawPixel(x-17, y-11, color_2);

  //tft.drawPixel(x, y-12, color_2);
  tft.drawPixel(x-1, y-12, color_2);
  tft.drawPixel(x-2, y-12, color_2);
  tft.drawPixel(x-3, y-12, color_2);
  tft.drawPixel(x-4, y-12, color_2);
  //tft.drawPixel(x-5, y-12, color_2);
  //tft.drawPixel(x-6, y-12, color_2);
  //tft.drawPixel(x-7, y-12, color_2);
  tft.drawPixel(x-8, y-12, color_2);
  tft.drawPixel(x-9, y-12, color_1);
  tft.drawPixel(x-10, y-12, color_1);
  tft.drawPixel(x-11, y-12, color_1);
  tft.drawPixel(x-12, y-12, color_1);
  tft.drawPixel(x-13, y-12, color_1);
  tft.drawPixel(x-14, y-12, color_1);
  tft.drawPixel(x-15, y-12, color_2);
  tft.drawPixel(x-16, y-12, color_1);
  tft.drawPixel(x-17, y-12, color_2);

  //tft.drawPixel(x, y-13, color_2);
  //tft.drawPixel(x-1, y-13, color_2);
  //tft.drawPixel(x-2, y-13, color_2);
  //tft.drawPixel(x-3, y-13, color_2);
  //tft.drawPixel(x-4, y-13, color_2);
  //tft.drawPixel(x-5, y-13, color_2);
  //tft.drawPixel(x-6, y-13, color_2);
  tft.drawPixel(x-7, y-13, color_2);
  tft.drawPixel(x-8, y-13, color_1);
  tft.drawPixel(x-9, y-13, color_1);
  tft.drawPixel(x-10, y-13, color_1);
  tft.drawPixel(x-11, y-13, color_1);
  tft.drawPixel(x-12, y-13, color_1);
  tft.drawPixel(x-13, y-13, color_1);
  tft.drawPixel(x-14, y-13, color_1);
  tft.drawPixel(x-15, y-13, color_2);
  tft.drawPixel(x-16, y-13, color_1);
  tft.drawPixel(x-17, y-13, color_2);

  //tft.drawPixel(x, y-14, color_2);
  //tft.drawPixel(x-1, y-14, color_2);
  //tft.drawPixel(x-2, y-14, color_2);
  //tft.drawPixel(x-3, y-14, color_2);
  //tft.drawPixel(x-4, y-14, color_2);
  //tft.drawPixel(x-5, y-14, color_2);
  tft.drawPixel(x-6, y-14, color_2);
  tft.drawPixel(x-7, y-14, color_1);
  tft.drawPixel(x-8, y-14, color_1);
  tft.drawPixel(x-9, y-14, color_1);
  tft.drawPixel(x-10, y-14, color_1);
  tft.drawPixel(x-11, y-14, color_1);
  tft.drawPixel(x-12, y-14, color_1);
  tft.drawPixel(x-13, y-14, color_1);
  tft.drawPixel(x-14, y-14, color_1);
  tft.drawPixel(x-15, y-14, color_1);
  tft.drawPixel(x-16, y-14, color_1);
  tft.drawPixel(x-17, y-14, color_2);

  //tft.drawPixel(x, y-15, color_2);
  //tft.drawPixel(x-1, y-15, color_2);
  //tft.drawPixel(x-2, y-15, color_2);
  //tft.drawPixel(x-3, y-15, color_2);
  //tft.drawPixel(x-4, y-15, color_2);
  tft.drawPixel(x-5, y-15, color_2);
  tft.drawPixel(x-6, y-15, color_1);
  tft.drawPixel(x-7, y-15, color_1);
  tft.drawPixel(x-8, y-15, color_1);
  tft.drawPixel(x-9, y-15, color_1);
  tft.drawPixel(x-10, y-15, color_1);
  tft.drawPixel(x-11, y-15, color_1);
  tft.drawPixel(x-12, y-15, color_1);
  tft.drawPixel(x-13, y-15, color_1);
  tft.drawPixel(x-14, y-15, color_1);
  tft.drawPixel(x-15, y-15, color_1);
  tft.drawPixel(x-16, y-15, color_1);
  tft.drawPixel(x-17, y-15, color_1);
  tft.drawPixel(x-18, y-15, color_2);
  
  //tft.drawPixel(x, y-16, color_2);
  //tft.drawPixel(x-1, y-16, color_2);
  //tft.drawPixel(x-2, y-16, color_2);
  //tft.drawPixel(x-3, y-16, color_2);
  //tft.drawPixel(x-4, y-16, color_2);
  tft.drawPixel(x-5, y-16, color_2);
  tft.drawPixel(x-6, y-16, color_1);
  tft.drawPixel(x-7, y-16, color_1);
  tft.drawPixel(x-8, y-16, color_1);
  tft.drawPixel(x-9, y-16, color_1);
  tft.drawPixel(x-10, y-16, color_1);
  tft.drawPixel(x-11, y-16, color_1);
  tft.drawPixel(x-12, y-16, color_1);
  tft.drawPixel(x-13, y-16, color_1);
  tft.drawPixel(x-14, y-16, color_1);
  tft.drawPixel(x-15, y-16, color_1);
  tft.drawPixel(x-16, y-16, color_1);
  tft.drawPixel(x-17, y-16, color_1);
  tft.drawPixel(x-18, y-16, color_2);

  //tft.drawPixel(x, y-17, color_2);
  //tft.drawPixel(x-1, y-17, color_2);
  //tft.drawPixel(x-2, y-17, color_2);
  //tft.drawPixel(x-3, y-17, color_2);
  tft.drawPixel(x-4, y-17, color_2);
  tft.drawPixel(x-5, y-17, color_2);
  tft.drawPixel(x-6, y-17, color_2);
  tft.drawPixel(x-7, y-17, color_2);
  tft.drawPixel(x-8, y-17, color_2);
  tft.drawPixel(x-9, y-17, color_2);
  tft.drawPixel(x-10, y-17, color_2);
  tft.drawPixel(x-11, y-17, color_2);
  tft.drawPixel(x-12, y-17, color_2);
  tft.drawPixel(x-13, y-17, color_2);
  tft.drawPixel(x-14, y-17, color_2);
  tft.drawPixel(x-15, y-17, color_2);
  tft.drawPixel(x-16, y-17, color_2);
  tft.drawPixel(x-17, y-17, color_2);
  tft.drawPixel(x-18, y-17, color_2);
  tft.drawPixel(x-19, y-17, color_2);

  //tft.drawPixel(x, y-18, color_2);
  //tft.drawPixel(x-1, y-18, color_2);
  //tft.drawPixel(x-2, y-18, color_2);
  //tft.drawPixel(x-3, y-18, color_2);
  tft.drawPixel(x-4, y-18, color_2);
  tft.drawPixel(x-5, y-18, color_1);
  tft.drawPixel(x-6, y-18, color_1);
  tft.drawPixel(x-7, y-18, color_1);
  tft.drawPixel(x-8, y-18, color_1);
  tft.drawPixel(x-9, y-18, color_1);
  tft.drawPixel(x-10, y-18, color_1);
  tft.drawPixel(x-11, y-18, color_1);
  tft.drawPixel(x-12, y-18, color_1);
  tft.drawPixel(x-13, y-18, color_1);
  tft.drawPixel(x-14, y-18, color_1);
  tft.drawPixel(x-15, y-18, color_1);
  tft.drawPixel(x-16, y-18, color_1);
  tft.drawPixel(x-17, y-18, color_1);
  tft.drawPixel(x-18, y-18, color_1);
  tft.drawPixel(x-19, y-18, color_2);

  //tft.drawPixel(x, y-19, color_2);
  //tft.drawPixel(x-1, y-19, color_2);
  //tft.drawPixel(x-2, y-19, color_2);
  //tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x-4, y-19, color_2);
  tft.drawPixel(x-5, y-19, color_2);
  tft.drawPixel(x-6, y-19, color_2);
  tft.drawPixel(x-7, y-19, color_2);
  tft.drawPixel(x-8, y-19, color_2);
  tft.drawPixel(x-9, y-19, color_2);
  tft.drawPixel(x-10, y-19, color_2);
  tft.drawPixel(x-11, y-19, color_2);
  tft.drawPixel(x-12, y-19, color_2);
  tft.drawPixel(x-13, y-19, color_2);
  tft.drawPixel(x-14, y-19, color_2);
  tft.drawPixel(x-15, y-19, color_2);
  tft.drawPixel(x-16, y-19, color_2);
  tft.drawPixel(x-17, y-19, color_2);
  tft.drawPixel(x-18, y-19, color_2);
  tft.drawPixel(x-19, y-19, color_2);
}










void drawPawn2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x-8, y-0, color_2);
  tft.drawPixel(x-9, y-0, color_2);
  tft.drawPixel(x-10, y-0, color_2);
  tft.drawPixel(x-11, y-0, color_2);

  tft.drawPixel(x-7, y-1, color_2);
  tft.drawPixel(x-8, y-1, color_1);
  tft.drawPixel(x-9, y-1, color_1);
  tft.drawPixel(x-10, y-1, color_1);
  tft.drawPixel(x-11, y-1, color_1);
  tft.drawPixel(x-12, y-1, color_2);

  tft.drawPixel(x-6, y-2, color_2);
  tft.drawPixel(x-7, y-2, color_1);
  tft.drawPixel(x-8, y-2, color_1);
  tft.drawPixel(x-9, y-2, color_1);
  tft.drawPixel(x-10, y-2, color_1);
  tft.drawPixel(x-11, y-2, color_1);
  tft.drawPixel(x-12, y-2, color_1);
  tft.drawPixel(x-13, y-2, color_2);

  tft.drawPixel(x-6, y-3, color_2);
  tft.drawPixel(x-7, y-3, color_1);
  tft.drawPixel(x-8, y-3, color_1);
  tft.drawPixel(x-9, y-3, color_1);
  tft.drawPixel(x-10, y-3, color_1);
  tft.drawPixel(x-11, y-3, color_1);
  tft.drawPixel(x-12, y-3, color_1);
  tft.drawPixel(x-13, y-3, color_2);

  tft.drawPixel(x-6, y-4, color_2);
  tft.drawPixel(x-7, y-4, color_1);
  tft.drawPixel(x-8, y-4, color_1);
  tft.drawPixel(x-9, y-4, color_1);
  tft.drawPixel(x-10, y-4, color_1);
  tft.drawPixel(x-11, y-4, color_1);
  tft.drawPixel(x-12, y-4, color_1);
  tft.drawPixel(x-13, y-4, color_2);

  tft.drawPixel(x-6, y-5, color_2);
  tft.drawPixel(x-7, y-5, color_1);
  tft.drawPixel(x-8, y-5, color_1);
  tft.drawPixel(x-9, y-5, color_1);
  tft.drawPixel(x-10, y-5, color_1);
  tft.drawPixel(x-11, y-5, color_1);
  tft.drawPixel(x-12, y-5, color_1);
  tft.drawPixel(x-13, y-5, color_2);

  tft.drawPixel(x-5, y-6, color_2);
  tft.drawPixel(x-6, y-6, color_1);
  tft.drawPixel(x-7, y-6, color_1);
  tft.drawPixel(x-8, y-6, color_1);
  tft.drawPixel(x-9, y-6, color_1);
  tft.drawPixel(x-10, y-6, color_1);
  tft.drawPixel(x-11, y-6, color_1);
  tft.drawPixel(x-12, y-6, color_1);
  tft.drawPixel(x-13, y-6, color_1);
  tft.drawPixel(x-14, y-6, color_2);

  tft.drawPixel(x-5, y-7, color_2);
  tft.drawPixel(x-6, y-7, color_2);
  tft.drawPixel(x-7, y-7, color_2);
  tft.drawPixel(x-8, y-7, color_2);
  tft.drawPixel(x-9, y-7, color_2);
  tft.drawPixel(x-10, y-7, color_2);
  tft.drawPixel(x-11, y-7, color_2);
  tft.drawPixel(x-12, y-7, color_2);
  tft.drawPixel(x-13, y-7, color_2);
  tft.drawPixel(x-14, y-7, color_2);

  tft.drawPixel(x-6, y-8, color_2);
  tft.drawPixel(x-7, y-8, color_2);
  tft.drawPixel(x-8, y-8, color_1);
  tft.drawPixel(x-9, y-8, color_1);
  tft.drawPixel(x-10, y-8, color_1);
  tft.drawPixel(x-11, y-8, color_1);
  tft.drawPixel(x-12, y-8, color_2);
  tft.drawPixel(x-13, y-8, color_2);

  tft.drawPixel(x-7, y-9, color_2);
  tft.drawPixel(x-8, y-9, color_1);
  tft.drawPixel(x-9, y-9, color_1);
  tft.drawPixel(x-10, y-9, color_1);
  tft.drawPixel(x-11, y-9, color_1);
  tft.drawPixel(x-12, y-9, color_2);

  tft.drawPixel(x-6, y-10, color_2);
  tft.drawPixel(x-7, y-10, color_2);
  tft.drawPixel(x-8, y-10, color_1);
  tft.drawPixel(x-9, y-10, color_1);
  tft.drawPixel(x-10, y-10, color_1);
  tft.drawPixel(x-11, y-10, color_1);
  tft.drawPixel(x-12, y-10, color_2);
  tft.drawPixel(x-13, y-10, color_2);

  tft.drawPixel(x-6, y-11, color_2);
  tft.drawPixel(x-7, y-11, color_1);
  tft.drawPixel(x-8, y-11, color_1);
  tft.drawPixel(x-9, y-11, color_1);
  tft.drawPixel(x-10, y-11, color_1);
  tft.drawPixel(x-11, y-11, color_1);
  tft.drawPixel(x-12, y-11, color_1);
  tft.drawPixel(x-13, y-11, color_2);

  tft.drawPixel(x-6, y-12, color_2);
  tft.drawPixel(x-7, y-12, color_1);
  tft.drawPixel(x-8, y-12, color_1);
  tft.drawPixel(x-9, y-12, color_1);
  tft.drawPixel(x-10, y-12, color_1);
  tft.drawPixel(x-11, y-12, color_1);
  tft.drawPixel(x-12, y-12, color_1);
  tft.drawPixel(x-13, y-12, color_2);

  tft.drawPixel(x-5, y-13, color_2);
  tft.drawPixel(x-6, y-13, color_1);
  tft.drawPixel(x-7, y-13, color_1);
  tft.drawPixel(x-8, y-13, color_1);
  tft.drawPixel(x-9, y-13, color_1);
  tft.drawPixel(x-10, y-13, color_1);
  tft.drawPixel(x-11, y-13, color_1);
  tft.drawPixel(x-12, y-13, color_1);
  tft.drawPixel(x-13, y-13, color_1);
  tft.drawPixel(x-14, y-13, color_2);

  tft.drawPixel(x-4, y-14, color_2);
  tft.drawPixel(x-5, y-14, color_2);
  tft.drawPixel(x-6, y-14, color_1);
  tft.drawPixel(x-7, y-14, color_1);
  tft.drawPixel(x-8, y-14, color_1);
  tft.drawPixel(x-9, y-14, color_1);
  tft.drawPixel(x-10, y-14, color_1);
  tft.drawPixel(x-11, y-14, color_1);
  tft.drawPixel(x-12, y-14, color_1);
  tft.drawPixel(x-13, y-14, color_1);
  tft.drawPixel(x-14, y-14, color_2);
  tft.drawPixel(x-15, y-14, color_2);

  tft.drawPixel(x-4, y-15, color_2);
  tft.drawPixel(x-5, y-15, color_1);
  tft.drawPixel(x-6, y-15, color_1);
  tft.drawPixel(x-7, y-15, color_1);
  tft.drawPixel(x-8, y-15, color_1);
  tft.drawPixel(x-9, y-15, color_1);
  tft.drawPixel(x-10, y-15, color_1);
  tft.drawPixel(x-11, y-15, color_1);
  tft.drawPixel(x-12, y-15, color_1);
  tft.drawPixel(x-13, y-15, color_1);
  tft.drawPixel(x-14, y-15, color_1);
  tft.drawPixel(x-15, y-15, color_2);

  tft.drawPixel(x-3, y-16, color_2);
  tft.drawPixel(x-4, y-16, color_1);
  tft.drawPixel(x-5, y-16, color_1);
  tft.drawPixel(x-6, y-16, color_1);
  tft.drawPixel(x-7, y-16, color_1);
  tft.drawPixel(x-8, y-16, color_1);
  tft.drawPixel(x-9, y-16, color_1);
  tft.drawPixel(x-10, y-16, color_1);
  tft.drawPixel(x-11, y-16, color_1);
  tft.drawPixel(x-12, y-16, color_1);
  tft.drawPixel(x-13, y-16, color_1);
  tft.drawPixel(x-14, y-16, color_1);
  tft.drawPixel(x-15, y-16, color_1);
  tft.drawPixel(x-16, y-16, color_2);

  tft.drawPixel(x-2, y-17, color_2);
  tft.drawPixel(x-3, y-17, color_2);
  tft.drawPixel(x-4, y-17, color_2);
  tft.drawPixel(x-5, y-17, color_2);
  tft.drawPixel(x-6, y-17, color_2);
  tft.drawPixel(x-7, y-17, color_2);
  tft.drawPixel(x-8, y-17, color_2);
  tft.drawPixel(x-9, y-17, color_2);
  tft.drawPixel(x-10, y-17, color_2);
  tft.drawPixel(x-11, y-17, color_2);
  tft.drawPixel(x-12, y-17, color_2);
  tft.drawPixel(x-13, y-17, color_2);
  tft.drawPixel(x-14, y-17, color_2);
  tft.drawPixel(x-15, y-17, color_2);
  tft.drawPixel(x-16, y-17, color_2);
  tft.drawPixel(x-17, y-17, color_2);

  tft.drawPixel(x-1, y-18, color_2);
  tft.drawPixel(x-2, y-18, color_1);
  tft.drawPixel(x-3, y-18, color_1);
  tft.drawPixel(x-4, y-18, color_1);
  tft.drawPixel(x-5, y-18, color_1);
  tft.drawPixel(x-6, y-18, color_1);
  tft.drawPixel(x-7, y-18, color_1);
  tft.drawPixel(x-8, y-18, color_1);
  tft.drawPixel(x-9, y-18, color_1);
  tft.drawPixel(x-10, y-18, color_1);
  tft.drawPixel(x-11, y-18, color_1);
  tft.drawPixel(x-12, y-18, color_1);
  tft.drawPixel(x-13, y-18, color_1);
  tft.drawPixel(x-14, y-18, color_1);
  tft.drawPixel(x-15, y-18, color_1);
  tft.drawPixel(x-16, y-18, color_1);
  tft.drawPixel(x-17, y-18, color_1);
  tft.drawPixel(x-18, y-18, color_2);

  tft.drawPixel(x-1, y-19, color_2);
  tft.drawPixel(x-2, y-19, color_2);
  tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x-4, y-19, color_2);
  tft.drawPixel(x-5, y-19, color_2);
  tft.drawPixel(x-6, y-19, color_2);
  tft.drawPixel(x-7, y-19, color_2);
  tft.drawPixel(x-8, y-19, color_2);
  tft.drawPixel(x-9, y-19, color_2);
  tft.drawPixel(x-10, y-19, color_2);
  tft.drawPixel(x-11, y-19, color_2);
  tft.drawPixel(x-12, y-19, color_2);
  tft.drawPixel(x-13, y-19, color_2);
  tft.drawPixel(x-14, y-19, color_2);
  tft.drawPixel(x-15, y-19, color_2);
  tft.drawPixel(x-16, y-19, color_2);
  tft.drawPixel(x-17, y-19, color_2);
  tft.drawPixel(x-18, y-19, color_2);
}


















void drawQueen2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x-6, y-0, color_2);
  tft.drawPixel(x-7, y-0, color_2);
  tft.drawPixel(x-8, y-0, color_2);
  tft.drawPixel(x-11, y-0, color_2);
  tft.drawPixel(x-12, y-0, color_2);
  tft.drawPixel(x-13, y-0, color_2);

  tft.drawPixel(x-6, y-1, color_2);
  tft.drawPixel(x-7, y-1, color_1);
  tft.drawPixel(x-8, y-1, color_2);
  tft.drawPixel(x-11, y-1, color_2);
  tft.drawPixel(x-12, y-1, color_1);
  tft.drawPixel(x-13, y-1, color_2);

  tft.drawPixel(x-0, y-2, color_2);
  tft.drawPixel(x-1, y-2, color_2);
  tft.drawPixel(x-2, y-2, color_2);
  tft.drawPixel(x-6, y-2, color_2);
  tft.drawPixel(x-7, y-2, color_2);
  tft.drawPixel(x-8, y-2, color_2);
  tft.drawPixel(x-11, y-2, color_2);
  tft.drawPixel(x-12, y-2, color_2);
  tft.drawPixel(x-13, y-2, color_2);
  tft.drawPixel(x-17, y-2, color_2);
  tft.drawPixel(x-18, y-2, color_2);
  tft.drawPixel(x-19, y-2, color_2);

  tft.drawPixel(x-0, y-3, color_2);
  tft.drawPixel(x-1, y-3, color_1);
  tft.drawPixel(x-2, y-3, color_2);
  tft.drawPixel(x-7, y-3, color_2);
  tft.drawPixel(x-12, y-3, color_2);
  tft.drawPixel(x-17, y-3, color_2);
  tft.drawPixel(x-18, y-3, color_1);
  tft.drawPixel(x-19, y-3, color_2);

  tft.drawPixel(x-0, y-4, color_2);
  tft.drawPixel(x-1, y-4, color_2);
  tft.drawPixel(x-2, y-4, color_2);
  tft.drawPixel(x-6, y-4, color_2);
  tft.drawPixel(x-7, y-4, color_1);
  tft.drawPixel(x-8, y-4, color_2);
  tft.drawPixel(x-11, y-4, color_2);
  tft.drawPixel(x-12, y-4, color_1);
  tft.drawPixel(x-13, y-4, color_2);
  tft.drawPixel(x-17, y-4, color_2);
  tft.drawPixel(x-18, y-4, color_2);
  tft.drawPixel(x-19, y-4, color_2);

  tft.drawPixel(x-1, y-5, color_2);
  tft.drawPixel(x-2, y-5, color_2);
  tft.drawPixel(x-6, y-5, color_2);
  tft.drawPixel(x-7, y-5, color_1);
  tft.drawPixel(x-8, y-5, color_2);
  tft.drawPixel(x-11, y-5, color_2);
  tft.drawPixel(x-12, y-5, color_1);
  tft.drawPixel(x-13, y-5, color_2);
  tft.drawPixel(x-17, y-5, color_2);
  tft.drawPixel(x-18, y-5, color_2);

  tft.drawPixel(x-2, y-6, color_2);
  tft.drawPixel(x-5, y-6, color_2);
  tft.drawPixel(x-6, y-6, color_2);
  tft.drawPixel(x-7, y-6, color_1);
  tft.drawPixel(x-8, y-6, color_2);
  tft.drawPixel(x-11, y-6, color_2);
  tft.drawPixel(x-12, y-6, color_1);
  tft.drawPixel(x-13, y-6, color_2);
  tft.drawPixel(x-14, y-6, color_2);
  tft.drawPixel(x-17, y-6, color_2);

  tft.drawPixel(x-2, y-7, color_2);
  tft.drawPixel(x-3, y-7, color_2);
  tft.drawPixel(x-5, y-7, color_2);
  tft.drawPixel(x-6, y-7, color_1);
  tft.drawPixel(x-7, y-7, color_1);
  tft.drawPixel(x-8, y-7, color_1);
  tft.drawPixel(x-9, y-7, color_2);
  tft.drawPixel(x-10, y-7, color_2);
  tft.drawPixel(x-11, y-7, color_1);
  tft.drawPixel(x-12, y-7, color_1);
  tft.drawPixel(x-13, y-7, color_1);
  tft.drawPixel(x-14, y-7, color_2);
  tft.drawPixel(x-16, y-7, color_2);
  tft.drawPixel(x-17, y-7, color_2);

  tft.drawPixel(x-2, y-8, color_2);
  tft.drawPixel(x-3, y-8, color_1);
  tft.drawPixel(x-4, y-8, color_2);
  tft.drawPixel(x-5, y-8, color_2);
  tft.drawPixel(x-6, y-8, color_1);
  tft.drawPixel(x-7, y-8, color_1);
  tft.drawPixel(x-8, y-8, color_1);
  tft.drawPixel(x-9, y-8, color_2);
  tft.drawPixel(x-10, y-8, color_2);
  tft.drawPixel(x-11, y-8, color_1);
  tft.drawPixel(x-12, y-8, color_1);
  tft.drawPixel(x-13, y-8, color_1);
  tft.drawPixel(x-14, y-8, color_2);
  tft.drawPixel(x-15, y-8, color_2);
  tft.drawPixel(x-16, y-8, color_1);
  tft.drawPixel(x-17, y-8, color_2);

  tft.drawPixel(x-2, y-9, color_2);
  tft.drawPixel(x-3, y-9, color_1);
  tft.drawPixel(x-4, y-9, color_1);
  tft.drawPixel(x-5, y-9, color_1);
  tft.drawPixel(x-6, y-9, color_1);
  tft.drawPixel(x-7, y-9, color_1);
  tft.drawPixel(x-8, y-9, color_1);
  tft.drawPixel(x-9, y-9, color_1);
  tft.drawPixel(x-10, y-9, color_1);
  tft.drawPixel(x-11, y-9, color_1);
  tft.drawPixel(x-12, y-9, color_1);
  tft.drawPixel(x-13, y-9, color_1);
  tft.drawPixel(x-14, y-9, color_1);
  tft.drawPixel(x-15, y-9, color_1);
  tft.drawPixel(x-16, y-9, color_1);
  tft.drawPixel(x-17, y-9, color_2);

  tft.drawPixel(x-3, y-10, color_2);
  tft.drawPixel(x-4, y-10, color_1);
  tft.drawPixel(x-5, y-10, color_1);
  tft.drawPixel(x-6, y-10, color_1);
  tft.drawPixel(x-7, y-10, color_1);
  tft.drawPixel(x-8, y-10, color_1);
  tft.drawPixel(x-9, y-10, color_1);
  tft.drawPixel(x-10, y-10, color_1);
  tft.drawPixel(x-11, y-10, color_1);
  tft.drawPixel(x-12, y-10, color_1);
  tft.drawPixel(x-13, y-10, color_1);
  tft.drawPixel(x-14, y-10, color_1);
  tft.drawPixel(x-15, y-10, color_1);
  tft.drawPixel(x-16, y-10, color_2);

  tft.drawPixel(x-3, y-11, color_2);
  tft.drawPixel(x-4, y-11, color_1);
  tft.drawPixel(x-5, y-11, color_1);
  tft.drawPixel(x-6, y-11, color_1);
  tft.drawPixel(x-7, y-11, color_1);
  tft.drawPixel(x-8, y-11, color_1);
  tft.drawPixel(x-9, y-11, color_1);
  tft.drawPixel(x-10, y-11, color_1);
  tft.drawPixel(x-11, y-11, color_1);
  tft.drawPixel(x-12, y-11, color_1);
  tft.drawPixel(x-13, y-11, color_1);
  tft.drawPixel(x-14, y-11, color_1);
  tft.drawPixel(x-15, y-11, color_1);
  tft.drawPixel(x-16, y-11, color_2);

  tft.drawPixel(x-3, y-12, color_2);
  tft.drawPixel(x-4, y-12, color_2);
  tft.drawPixel(x-5, y-12, color_1);
  tft.drawPixel(x-6, y-12, color_1);
  tft.drawPixel(x-7, y-12, color_1);
  tft.drawPixel(x-8, y-12, color_1);
  tft.drawPixel(x-9, y-12, color_1);
  tft.drawPixel(x-10, y-12, color_1);
  tft.drawPixel(x-11, y-12, color_1);
  tft.drawPixel(x-12, y-12, color_1);
  tft.drawPixel(x-13, y-12, color_1);
  tft.drawPixel(x-14, y-12, color_1);
  tft.drawPixel(x-15, y-12, color_2);
  tft.drawPixel(x-16, y-12, color_2);

  tft.drawPixel(x-4, y-13, color_2);
  tft.drawPixel(x-5, y-13, color_2);
  tft.drawPixel(x-6, y-13, color_2);
  tft.drawPixel(x-7, y-13, color_2);
  tft.drawPixel(x-8, y-13, color_2);
  tft.drawPixel(x-9, y-13, color_2);
  tft.drawPixel(x-10, y-13, color_2);
  tft.drawPixel(x-11, y-13, color_2);
  tft.drawPixel(x-12, y-13, color_2);
  tft.drawPixel(x-13, y-13, color_2);
  tft.drawPixel(x-14, y-13, color_2);
  tft.drawPixel(x-15, y-13, color_2);

  tft.drawPixel(x-3, y-14, color_2);
  tft.drawPixel(x-4, y-14, color_2);
  tft.drawPixel(x-5, y-14, color_1);
  tft.drawPixel(x-6, y-14, color_1);
  tft.drawPixel(x-7, y-14, color_1);
  tft.drawPixel(x-8, y-14, color_1);
  tft.drawPixel(x-9, y-14, color_1);
  tft.drawPixel(x-10, y-14, color_1);
  tft.drawPixel(x-11, y-14, color_1);
  tft.drawPixel(x-12, y-14, color_1);
  tft.drawPixel(x-13, y-14, color_1);
  tft.drawPixel(x-14, y-14, color_1);
  tft.drawPixel(x-15, y-14, color_2);
  tft.drawPixel(x-16, y-14, color_2);

  tft.drawPixel(x-3, y-15, color_2);
  tft.drawPixel(x-4, y-15, color_1);
  tft.drawPixel(x-5, y-15, color_1);
  tft.drawPixel(x-6, y-15, color_1);
  tft.drawPixel(x-7, y-15, color_1);
  tft.drawPixel(x-8, y-15, color_1);
  tft.drawPixel(x-9, y-15, color_1);
  tft.drawPixel(x-10, y-15, color_1);
  tft.drawPixel(x-11, y-15, color_1);
  tft.drawPixel(x-12, y-15, color_1);
  tft.drawPixel(x-13, y-15, color_1);
  tft.drawPixel(x-14, y-15, color_1);
  tft.drawPixel(x-15, y-15, color_1);
  tft.drawPixel(x-16, y-15, color_2);

  tft.drawPixel(x-3, y-16, color_2);
  tft.drawPixel(x-4, y-16, color_2);
  tft.drawPixel(x-5, y-16, color_2);
  tft.drawPixel(x-6, y-16, color_2);
  tft.drawPixel(x-7, y-16, color_2);
  tft.drawPixel(x-8, y-16, color_2);
  tft.drawPixel(x-9, y-16, color_2);
  tft.drawPixel(x-10, y-16, color_2);
  tft.drawPixel(x-11, y-16, color_2);
  tft.drawPixel(x-12, y-16, color_2);
  tft.drawPixel(x-13, y-16, color_2);
  tft.drawPixel(x-14, y-16, color_2);
  tft.drawPixel(x-15, y-16, color_2);
  tft.drawPixel(x-16, y-16, color_2);

  tft.drawPixel(x-2, y-17, color_2);
  tft.drawPixel(x-3, y-17, color_1);
  tft.drawPixel(x-4, y-17, color_1);
  tft.drawPixel(x-5, y-17, color_1);
  tft.drawPixel(x-6, y-17, color_1);
  tft.drawPixel(x-7, y-17, color_1);
  tft.drawPixel(x-8, y-17, color_1);
  tft.drawPixel(x-9, y-17, color_1);
  tft.drawPixel(x-10, y-17, color_1);
  tft.drawPixel(x-11, y-17, color_1);
  tft.drawPixel(x-12, y-17, color_1);
  tft.drawPixel(x-13, y-17, color_1);
  tft.drawPixel(x-14, y-17, color_1);
  tft.drawPixel(x-15, y-17, color_1);
  tft.drawPixel(x-16, y-17, color_1);
  tft.drawPixel(x-17, y-17, color_2);

  tft.drawPixel(x-2, y-18, color_2);
  tft.drawPixel(x-3, y-18, color_1);
  tft.drawPixel(x-4, y-18, color_1);
  tft.drawPixel(x-5, y-18, color_1);
  tft.drawPixel(x-6, y-18, color_1);
  tft.drawPixel(x-7, y-18, color_1);
  tft.drawPixel(x-8, y-18, color_1);
  tft.drawPixel(x-9, y-18, color_1);
  tft.drawPixel(x-10, y-18, color_1);
  tft.drawPixel(x-11, y-18, color_1);
  tft.drawPixel(x-12, y-18, color_1);
  tft.drawPixel(x-13, y-18, color_1);
  tft.drawPixel(x-14, y-18, color_1);
  tft.drawPixel(x-15, y-18, color_1);
  tft.drawPixel(x-16, y-18, color_1);
  tft.drawPixel(x-17, y-18, color_2);

  tft.drawPixel(x-2, y-19, color_2);
  tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x-4, y-19, color_2);
  tft.drawPixel(x-5, y-19, color_2);
  tft.drawPixel(x-6, y-19, color_2);
  tft.drawPixel(x-7, y-19, color_2);
  tft.drawPixel(x-8, y-19, color_2);
  tft.drawPixel(x-9, y-19, color_2);
  tft.drawPixel(x-10, y-19, color_2);
  tft.drawPixel(x-11, y-19, color_2);
  tft.drawPixel(x-12, y-19, color_2);
  tft.drawPixel(x-13, y-19, color_2);
  tft.drawPixel(x-14, y-19, color_2);
  tft.drawPixel(x-15, y-19, color_2);
  tft.drawPixel(x-16, y-19, color_2);
  tft.drawPixel(x-17, y-19, color_2);
}
















void drawKing2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x-9, y-0, color_2);
  tft.drawPixel(x-10, y-0, color_2);

  tft.drawPixel(x-8, y-1, color_2);
  tft.drawPixel(x-9, y-1, color_2);
  tft.drawPixel(x-10, y-1, color_2);
  tft.drawPixel(x-11, y-1, color_2);

  tft.drawPixel(x-9, y-2, color_2);
  tft.drawPixel(x-10, y-2, color_2);

  tft.drawPixel(x-9, y-3, color_2);
  tft.drawPixel(x-10, y-3, color_2);

  tft.drawPixel(x-3, y-4, color_2);
  tft.drawPixel(x-4, y-4, color_2);
  tft.drawPixel(x-5, y-4, color_2);
  tft.drawPixel(x-6, y-4, color_2);
  tft.drawPixel(x-8, y-4, color_2);
  tft.drawPixel(x-9, y-4, color_2);
  tft.drawPixel(x-10, y-4, color_2);
  tft.drawPixel(x-11, y-4, color_2);
  tft.drawPixel(x-13, y-4, color_2);
  tft.drawPixel(x-14, y-4, color_2);
  tft.drawPixel(x-15, y-4, color_2);
  tft.drawPixel(x-16, y-4, color_2);

  tft.drawPixel(x-1, y-5, color_2);
  tft.drawPixel(x-2, y-5, color_2);
  tft.drawPixel(x-3, y-5, color_2);
  tft.drawPixel(x-4, y-5, color_1);
  tft.drawPixel(x-5, y-5, color_1);
  tft.drawPixel(x-6, y-5, color_2);
  tft.drawPixel(x-7, y-5, color_2);
  tft.drawPixel(x-8, y-5, color_2);
  tft.drawPixel(x-9, y-5, color_1);
  tft.drawPixel(x-10, y-5, color_1);
  tft.drawPixel(x-11, y-5, color_2);
  tft.drawPixel(x-12, y-5, color_2);
  tft.drawPixel(x-13, y-5, color_2);
  tft.drawPixel(x-14, y-5, color_1);
  tft.drawPixel(x-15, y-5, color_1);
  tft.drawPixel(x-16, y-5, color_2);
  tft.drawPixel(x-17, y-5, color_2);
  tft.drawPixel(x-18, y-5, color_2);

  tft.drawPixel(x-0, y-6, color_2);
  tft.drawPixel(x-1, y-6, color_2);
  tft.drawPixel(x-2, y-6, color_1);
  tft.drawPixel(x-3, y-6, color_1);
  tft.drawPixel(x-4, y-6, color_1);
  tft.drawPixel(x-5, y-6, color_1);
  tft.drawPixel(x-6, y-6, color_1);
  tft.drawPixel(x-7, y-6, color_2);
  tft.drawPixel(x-8, y-6, color_1);
  tft.drawPixel(x-9, y-6, color_1);
  tft.drawPixel(x-10, y-6, color_1);
  tft.drawPixel(x-11, y-6, color_1);
  tft.drawPixel(x-12, y-6, color_2);
  tft.drawPixel(x-13, y-6, color_1);
  tft.drawPixel(x-14, y-6, color_1);
  tft.drawPixel(x-15, y-6, color_1);
  tft.drawPixel(x-16, y-6, color_1);
  tft.drawPixel(x-17, y-6, color_1);
  tft.drawPixel(x-18, y-6, color_2);
  tft.drawPixel(x-19, y-6, color_2);

  tft.drawPixel(x-0, y-7, color_2);
  tft.drawPixel(x-1, y-7, color_1);
  tft.drawPixel(x-2, y-7, color_1);
  tft.drawPixel(x-3, y-7, color_1);
  tft.drawPixel(x-4, y-7, color_1);
  tft.drawPixel(x-5, y-7, color_1);
  tft.drawPixel(x-6, y-7, color_1);
  tft.drawPixel(x-7, y-7, color_2);
  tft.drawPixel(x-8, y-7, color_2);
  tft.drawPixel(x-9, y-7, color_1);
  tft.drawPixel(x-10, y-7, color_1);
  tft.drawPixel(x-11, y-7, color_2);
  tft.drawPixel(x-12, y-7, color_2);
  tft.drawPixel(x-13, y-7, color_1);
  tft.drawPixel(x-14, y-7, color_1);
  tft.drawPixel(x-15, y-7, color_1);
  tft.drawPixel(x-16, y-7, color_1);
  tft.drawPixel(x-17, y-7, color_1);
  tft.drawPixel(x-18, y-7, color_1);
  tft.drawPixel(x-19, y-7, color_2);

  tft.drawPixel(x-0, y-8, color_2);
  tft.drawPixel(x-1, y-8, color_1);
  tft.drawPixel(x-2, y-8, color_1);
  tft.drawPixel(x-3, y-8, color_1);
  tft.drawPixel(x-4, y-8, color_1);
  tft.drawPixel(x-5, y-8, color_1);
  tft.drawPixel(x-6, y-8, color_1);
  tft.drawPixel(x-7, y-8, color_1);
  tft.drawPixel(x-8, y-8, color_2);
  tft.drawPixel(x-9, y-8, color_2);
  tft.drawPixel(x-10, y-8, color_2);
  tft.drawPixel(x-11, y-8, color_2);
  tft.drawPixel(x-12, y-8, color_1);
  tft.drawPixel(x-13, y-8, color_1);
  tft.drawPixel(x-14, y-8, color_1);
  tft.drawPixel(x-15, y-8, color_1);
  tft.drawPixel(x-16, y-8, color_1);
  tft.drawPixel(x-17, y-8, color_1);
  tft.drawPixel(x-18, y-8, color_1);
  tft.drawPixel(x-19, y-8, color_2);

  tft.drawPixel(x-0, y-9, color_2);
  tft.drawPixel(x-1, y-9, color_1);
  tft.drawPixel(x-2, y-9, color_1);
  tft.drawPixel(x-3, y-9, color_1);
  tft.drawPixel(x-4, y-9, color_1);
  tft.drawPixel(x-5, y-9, color_1);
  tft.drawPixel(x-6, y-9, color_1);
  tft.drawPixel(x-7, y-9, color_1);
  tft.drawPixel(x-8, y-9, color_1);
  tft.drawPixel(x-9, y-9, color_2);
  tft.drawPixel(x-10, y-9, color_2);
  tft.drawPixel(x-11, y-9, color_1);
  tft.drawPixel(x-12, y-9, color_1);
  tft.drawPixel(x-13, y-9, color_1);
  tft.drawPixel(x-14, y-9, color_1);
  tft.drawPixel(x-15, y-9, color_1);
  tft.drawPixel(x-16, y-9, color_1);
  tft.drawPixel(x-17, y-9, color_1);
  tft.drawPixel(x-18, y-9, color_1);
  tft.drawPixel(x-19, y-9, color_2);

  tft.drawPixel(x-0, y-10, color_2);
  tft.drawPixel(x-1, y-10, color_2);
  tft.drawPixel(x-2, y-10, color_1);
  tft.drawPixel(x-3, y-10, color_1);
  tft.drawPixel(x-4, y-10, color_1);
  tft.drawPixel(x-5, y-10, color_1);
  tft.drawPixel(x-6, y-10, color_1);
  tft.drawPixel(x-7, y-10, color_1);
  tft.drawPixel(x-8, y-10, color_1);
  tft.drawPixel(x-9, y-10, color_2);
  tft.drawPixel(x-10, y-10, color_2);
  tft.drawPixel(x-11, y-10, color_1);
  tft.drawPixel(x-12, y-10, color_1);
  tft.drawPixel(x-13, y-10, color_1);
  tft.drawPixel(x-14, y-10, color_1);
  tft.drawPixel(x-15, y-10, color_1);
  tft.drawPixel(x-16, y-10, color_1);
  tft.drawPixel(x-17, y-10, color_1);
  tft.drawPixel(x-18, y-10, color_2);
  tft.drawPixel(x-19, y-10, color_2);

  tft.drawPixel(x-1, y-11, color_2);
  tft.drawPixel(x-2, y-11, color_1);
  tft.drawPixel(x-3, y-11, color_1);
  tft.drawPixel(x-4, y-11, color_1);
  tft.drawPixel(x-5, y-11, color_1);
  tft.drawPixel(x-6, y-11, color_1);
  tft.drawPixel(x-7, y-11, color_1);
  tft.drawPixel(x-8, y-11, color_1);
  tft.drawPixel(x-9, y-11, color_2);
  tft.drawPixel(x-10, y-11, color_2);
  tft.drawPixel(x-11, y-11, color_1);
  tft.drawPixel(x-12, y-11, color_1);
  tft.drawPixel(x-13, y-11, color_1);
  tft.drawPixel(x-14, y-11, color_1);
  tft.drawPixel(x-15, y-11, color_1);
  tft.drawPixel(x-16, y-11, color_1);
  tft.drawPixel(x-17, y-11, color_1);
  tft.drawPixel(x-18, y-11, color_2);

  tft.drawPixel(x-1, y-12, color_2);
  tft.drawPixel(x-2, y-12, color_2);
  tft.drawPixel(x-3, y-12, color_1);
  tft.drawPixel(x-4, y-12, color_1);
  tft.drawPixel(x-5, y-12, color_1);
  tft.drawPixel(x-6, y-12, color_1);
  tft.drawPixel(x-7, y-12, color_1);
  tft.drawPixel(x-8, y-12, color_1);
  tft.drawPixel(x-9, y-12, color_2);
  tft.drawPixel(x-10, y-12, color_2);
  tft.drawPixel(x-11, y-12, color_1);
  tft.drawPixel(x-12, y-12, color_1);
  tft.drawPixel(x-13, y-12, color_1);
  tft.drawPixel(x-14, y-12, color_1);
  tft.drawPixel(x-15, y-12, color_1);
  tft.drawPixel(x-16, y-12, color_1);
  tft.drawPixel(x-17, y-12, color_2);
  tft.drawPixel(x-18, y-12, color_2);

  tft.drawPixel(x-2, y-13, color_2);
  tft.drawPixel(x-3, y-13, color_2);
  tft.drawPixel(x-4, y-13, color_1);
  tft.drawPixel(x-5, y-13, color_1);
  tft.drawPixel(x-6, y-13, color_1);
  tft.drawPixel(x-7, y-13, color_1);
  tft.drawPixel(x-8, y-13, color_1);
  tft.drawPixel(x-9, y-13, color_2);
  tft.drawPixel(x-10, y-13, color_2);
  tft.drawPixel(x-11, y-13, color_1);
  tft.drawPixel(x-12, y-13, color_1);
  tft.drawPixel(x-13, y-13, color_1);
  tft.drawPixel(x-14, y-13, color_1);
  tft.drawPixel(x-15, y-13, color_1);
  tft.drawPixel(x-16, y-13, color_2);
  tft.drawPixel(x-17, y-13, color_2);

  tft.drawPixel(x-3, y-14, color_2);
  tft.drawPixel(x-4, y-14, color_2);
  tft.drawPixel(x-5, y-14, color_2);
  tft.drawPixel(x-6, y-14, color_2);
  tft.drawPixel(x-7, y-14, color_2);
  tft.drawPixel(x-8, y-14, color_2);
  tft.drawPixel(x-9, y-14, color_2);
  tft.drawPixel(x-10, y-14, color_2);
  tft.drawPixel(x-11, y-14, color_2);
  tft.drawPixel(x-12, y-14, color_2);
  tft.drawPixel(x-13, y-14, color_2);
  tft.drawPixel(x-14, y-14, color_2);
  tft.drawPixel(x-15, y-14, color_2);
  tft.drawPixel(x-16, y-14, color_2);

  tft.drawPixel(x-2, y-15, color_2);
  tft.drawPixel(x-3, y-15, color_2);
  tft.drawPixel(x-4, y-15, color_1);
  tft.drawPixel(x-5, y-15, color_1);
  tft.drawPixel(x-6, y-15, color_1);
  tft.drawPixel(x-7, y-15, color_1);
  tft.drawPixel(x-8, y-15, color_1);
  tft.drawPixel(x-9, y-15, color_1);
  tft.drawPixel(x-10, y-15, color_1);
  tft.drawPixel(x-11, y-15, color_1);
  tft.drawPixel(x-12, y-15, color_1);
  tft.drawPixel(x-13, y-15, color_1);
  tft.drawPixel(x-14, y-15, color_1);
  tft.drawPixel(x-15, y-15, color_1);
  tft.drawPixel(x-16, y-15, color_2);
  tft.drawPixel(x-17, y-15, color_2);

  tft.drawPixel(x-2, y-16, color_2);
  tft.drawPixel(x-3, y-16, color_1);
  tft.drawPixel(x-4, y-16, color_1);
  tft.drawPixel(x-5, y-16, color_2);
  tft.drawPixel(x-6, y-16, color_2);
  tft.drawPixel(x-7, y-16, color_2);
  tft.drawPixel(x-8, y-16, color_2);
  tft.drawPixel(x-9, y-16, color_2);
  tft.drawPixel(x-10, y-16, color_2);
  tft.drawPixel(x-11, y-16, color_2);
  tft.drawPixel(x-12, y-16, color_2);
  tft.drawPixel(x-13, y-16, color_2);
  tft.drawPixel(x-14, y-16, color_2);
  tft.drawPixel(x-15, y-16, color_1);
  tft.drawPixel(x-16, y-16, color_1);
  tft.drawPixel(x-17, y-16, color_2);

  tft.drawPixel(x-2, y-17, color_2);
  tft.drawPixel(x-3, y-17, color_2);
  tft.drawPixel(x-4, y-17, color_2);
  tft.drawPixel(x-5, y-17, color_1);
  tft.drawPixel(x-6, y-17, color_1);
  tft.drawPixel(x-7, y-17, color_1);
  tft.drawPixel(x-8, y-17, color_1);
  tft.drawPixel(x-9, y-17, color_1);
  tft.drawPixel(x-10, y-17, color_1);
  tft.drawPixel(x-11, y-17, color_1);
  tft.drawPixel(x-12, y-17, color_1);
  tft.drawPixel(x-13, y-17, color_1);
  tft.drawPixel(x-14, y-17, color_1);
  tft.drawPixel(x-15, y-17, color_2);
  tft.drawPixel(x-16, y-17, color_2);
  tft.drawPixel(x-17, y-17, color_2);

  tft.drawPixel(x-2, y-18, color_2);
  tft.drawPixel(x-3, y-18, color_1);
  tft.drawPixel(x-4, y-18, color_1);
  tft.drawPixel(x-5, y-18, color_1);
  tft.drawPixel(x-6, y-18, color_1);
  tft.drawPixel(x-7, y-18, color_1);
  tft.drawPixel(x-8, y-18, color_1);
  tft.drawPixel(x-9, y-18, color_1);
  tft.drawPixel(x-10, y-18, color_1);
  tft.drawPixel(x-11, y-18, color_1);
  tft.drawPixel(x-12, y-18, color_1);
  tft.drawPixel(x-13, y-18, color_1);
  tft.drawPixel(x-14, y-18, color_1);
  tft.drawPixel(x-15, y-18, color_1);
  tft.drawPixel(x-16, y-18, color_1);
  tft.drawPixel(x-17, y-18, color_2);

  tft.drawPixel(x-2, y-19, color_2);
  tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x-4, y-19, color_2);
  tft.drawPixel(x-5, y-19, color_2);
  tft.drawPixel(x-6, y-19, color_2);
  tft.drawPixel(x-7, y-19, color_2);
  tft.drawPixel(x-8, y-19, color_2);
  tft.drawPixel(x-9, y-19, color_2);
  tft.drawPixel(x-10, y-19, color_2);
  tft.drawPixel(x-11, y-19, color_2);
  tft.drawPixel(x-12, y-19, color_2);
  tft.drawPixel(x-13, y-19, color_2);
  tft.drawPixel(x-14, y-19, color_2);
  tft.drawPixel(x-15, y-19, color_2);
  tft.drawPixel(x-16, y-19, color_2);
  tft.drawPixel(x-17, y-19, color_2);
}


















void drawRook2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x-2, y-0, color_2);
  tft.drawPixel(x-3, y-0, color_2);
  tft.drawPixel(x-4, y-0, color_2);
  tft.drawPixel(x-5, y-0, color_2);
  tft.drawPixel(x-8, y-0, color_2);
  tft.drawPixel(x-9, y-0, color_2);
  tft.drawPixel(x-10, y-0, color_2);
  tft.drawPixel(x-11, y-0, color_2);
  tft.drawPixel(x-14, y-0, color_2);
  tft.drawPixel(x-15, y-0, color_2);
  tft.drawPixel(x-16, y-0, color_2);
  tft.drawPixel(x-17, y-0, color_2);

  tft.drawPixel(x-2, y-1, color_2);
  tft.drawPixel(x-3, y-1, color_1);
  tft.drawPixel(x-4, y-1, color_1);
  tft.drawPixel(x-5, y-1, color_2);
  tft.drawPixel(x-8, y-1, color_2);
  tft.drawPixel(x-9, y-1, color_1);
  tft.drawPixel(x-10, y-1, color_1);
  tft.drawPixel(x-11, y-1, color_2);
  tft.drawPixel(x-14, y-1, color_2);
  tft.drawPixel(x-15, y-1, color_1);
  tft.drawPixel(x-16, y-1, color_1);
  tft.drawPixel(x-17, y-1, color_2);

  tft.drawPixel(x-2, y-2, color_2);
  tft.drawPixel(x-3, y-2, color_1);
  tft.drawPixel(x-4, y-2, color_1);
  tft.drawPixel(x-5, y-2, color_2);
  tft.drawPixel(x-6, y-2, color_2);
  tft.drawPixel(x-7, y-2, color_2);
  tft.drawPixel(x-8, y-2, color_2);
  tft.drawPixel(x-9, y-2, color_1);
  tft.drawPixel(x-10, y-2, color_1);
  tft.drawPixel(x-11, y-2, color_2);
  tft.drawPixel(x-12, y-2, color_2);
  tft.drawPixel(x-13, y-2, color_2);
  tft.drawPixel(x-14, y-2, color_2);
  tft.drawPixel(x-15, y-2, color_1);
  tft.drawPixel(x-16, y-2, color_1);
  tft.drawPixel(x-17, y-2, color_2);

  tft.drawPixel(x-2, y-3, color_2);
  tft.drawPixel(x-3, y-3, color_1);
  tft.drawPixel(x-4, y-3, color_1);
  tft.drawPixel(x-5, y-3, color_1);
  tft.drawPixel(x-6, y-3, color_1);
  tft.drawPixel(x-7, y-3, color_1);
  tft.drawPixel(x-8, y-3, color_1);
  tft.drawPixel(x-9, y-3, color_1);
  tft.drawPixel(x-10, y-3, color_1);
  tft.drawPixel(x-11, y-3, color_1);
  tft.drawPixel(x-12, y-3, color_1);
  tft.drawPixel(x-13, y-3, color_1);
  tft.drawPixel(x-14, y-3, color_1);
  tft.drawPixel(x-15, y-3, color_1);
  tft.drawPixel(x-16, y-3, color_1);
  tft.drawPixel(x-17, y-3, color_2);

  tft.drawPixel(x-2, y-4, color_2);
  tft.drawPixel(x-3, y-4, color_2);
  tft.drawPixel(x-4, y-4, color_1);
  tft.drawPixel(x-5, y-4, color_1);
  tft.drawPixel(x-6, y-4, color_1);
  tft.drawPixel(x-7, y-4, color_1);
  tft.drawPixel(x-8, y-4, color_1);
  tft.drawPixel(x-9, y-4, color_1);
  tft.drawPixel(x-10, y-4, color_1);
  tft.drawPixel(x-11, y-4, color_1);
  tft.drawPixel(x-12, y-4, color_1);
  tft.drawPixel(x-13, y-4, color_1);
  tft.drawPixel(x-14, y-4, color_1);
  tft.drawPixel(x-15, y-4, color_1);
  tft.drawPixel(x-16, y-4, color_2);
  tft.drawPixel(x-17, y-4, color_2);

  tft.drawPixel(x-3, y-5, color_2);
  tft.drawPixel(x-4, y-5, color_2);
  tft.drawPixel(x-5, y-5, color_1);
  tft.drawPixel(x-6, y-5, color_1);
  tft.drawPixel(x-7, y-5, color_1);
  tft.drawPixel(x-8, y-5, color_1);
  tft.drawPixel(x-9, y-5, color_1);
  tft.drawPixel(x-10, y-5, color_1);
  tft.drawPixel(x-11, y-5, color_1);
  tft.drawPixel(x-12, y-5, color_1);
  tft.drawPixel(x-13, y-5, color_1);
  tft.drawPixel(x-14, y-5, color_1);
  tft.drawPixel(x-15, y-5, color_2);
  tft.drawPixel(x-16, y-5, color_2);

  tft.drawPixel(x-4, y-6, color_2);
  tft.drawPixel(x-5, y-6, color_2);
  tft.drawPixel(x-6, y-6, color_2);
  tft.drawPixel(x-7, y-6, color_2);
  tft.drawPixel(x-8, y-6, color_2);
  tft.drawPixel(x-9, y-6, color_2);
  tft.drawPixel(x-10, y-6, color_2);
  tft.drawPixel(x-11, y-6, color_2);
  tft.drawPixel(x-12, y-6, color_2);
  tft.drawPixel(x-13, y-6, color_2);
  tft.drawPixel(x-14, y-6, color_2);
  tft.drawPixel(x-15, y-6, color_2);

  tft.drawPixel(x-5, y-7, color_2);
  tft.drawPixel(x-6, y-7, color_1);
  tft.drawPixel(x-7, y-7, color_1);
  tft.drawPixel(x-8, y-7, color_1);
  tft.drawPixel(x-9, y-7, color_1);
  tft.drawPixel(x-10, y-7, color_1);
  tft.drawPixel(x-11, y-7, color_1);
  tft.drawPixel(x-12, y-7, color_1);
  tft.drawPixel(x-13, y-7, color_1);
  tft.drawPixel(x-14, y-7, color_2);

  tft.drawPixel(x-5, y-8, color_2);
  tft.drawPixel(x-6, y-8, color_1);
  tft.drawPixel(x-7, y-8, color_1);
  tft.drawPixel(x-8, y-8, color_1);
  tft.drawPixel(x-9, y-8, color_1);
  tft.drawPixel(x-10, y-8, color_1);
  tft.drawPixel(x-11, y-8, color_1);
  tft.drawPixel(x-12, y-8, color_1);
  tft.drawPixel(x-13, y-8, color_1);
  tft.drawPixel(x-14, y-8, color_2);

  tft.drawPixel(x-5, y-9, color_2);
  tft.drawPixel(x-6, y-9, color_1);
  tft.drawPixel(x-7, y-9, color_1);
  tft.drawPixel(x-8, y-9, color_1);
  tft.drawPixel(x-9, y-9, color_1);
  tft.drawPixel(x-10, y-9, color_1);
  tft.drawPixel(x-11, y-9, color_1);
  tft.drawPixel(x-12, y-9, color_1);
  tft.drawPixel(x-13, y-9, color_1);
  tft.drawPixel(x-14, y-9, color_2);

  tft.drawPixel(x-5, y-10, color_2);
  tft.drawPixel(x-6, y-10, color_1);
  tft.drawPixel(x-7, y-10, color_1);
  tft.drawPixel(x-8, y-10, color_1);
  tft.drawPixel(x-9, y-10, color_1);
  tft.drawPixel(x-10, y-10, color_1);
  tft.drawPixel(x-11, y-10, color_1);
  tft.drawPixel(x-12, y-10, color_1);
  tft.drawPixel(x-13, y-10, color_1);
  tft.drawPixel(x-14, y-10, color_2);

  tft.drawPixel(x-5, y-11, color_2);
  tft.drawPixel(x-6, y-11, color_1);
  tft.drawPixel(x-7, y-11, color_1);
  tft.drawPixel(x-8, y-11, color_1);
  tft.drawPixel(x-9, y-11, color_1);
  tft.drawPixel(x-10, y-11, color_1);
  tft.drawPixel(x-11, y-11, color_1);
  tft.drawPixel(x-12, y-11, color_1);
  tft.drawPixel(x-13, y-11, color_1);
  tft.drawPixel(x-14, y-11, color_2);

  tft.drawPixel(x-5, y-12, color_2);
  tft.drawPixel(x-6, y-12, color_1);
  tft.drawPixel(x-7, y-12, color_1);
  tft.drawPixel(x-8, y-12, color_1);
  tft.drawPixel(x-9, y-12, color_1);
  tft.drawPixel(x-10, y-12, color_1);
  tft.drawPixel(x-11, y-12, color_1);
  tft.drawPixel(x-12, y-12, color_1);
  tft.drawPixel(x-13, y-12, color_1);
  tft.drawPixel(x-14, y-12, color_2);

  tft.drawPixel(x-5, y-13, color_2);
  tft.drawPixel(x-6, y-13, color_1);
  tft.drawPixel(x-7, y-13, color_1);
  tft.drawPixel(x-8, y-13, color_1);
  tft.drawPixel(x-9, y-13, color_1);
  tft.drawPixel(x-10, y-13, color_1);
  tft.drawPixel(x-11, y-13, color_1);
  tft.drawPixel(x-12, y-13, color_1);
  tft.drawPixel(x-13, y-13, color_1);
  tft.drawPixel(x-14, y-13, color_2);

  tft.drawPixel(x-4, y-14, color_2);
  tft.drawPixel(x-5, y-14, color_2);
  tft.drawPixel(x-6, y-14, color_2);
  tft.drawPixel(x-7, y-14, color_2);
  tft.drawPixel(x-8, y-14, color_2);
  tft.drawPixel(x-9, y-14, color_2);
  tft.drawPixel(x-10, y-14, color_2);
  tft.drawPixel(x-11, y-14, color_2);
  tft.drawPixel(x-12, y-14, color_2);
  tft.drawPixel(x-13, y-14, color_2);
  tft.drawPixel(x-14, y-14, color_2);
  tft.drawPixel(x-15, y-14, color_2);

  tft.drawPixel(x-3, y-15, color_2);
  tft.drawPixel(x-4, y-15, color_2);
  tft.drawPixel(x-5, y-15, color_1);
  tft.drawPixel(x-6, y-15, color_1);
  tft.drawPixel(x-7, y-15, color_1);
  tft.drawPixel(x-8, y-15, color_1);
  tft.drawPixel(x-9, y-15, color_1);
  tft.drawPixel(x-10, y-15, color_1);
  tft.drawPixel(x-11, y-15, color_1);
  tft.drawPixel(x-12, y-15, color_1);
  tft.drawPixel(x-13, y-15, color_1);
  tft.drawPixel(x-14, y-15, color_1);
  tft.drawPixel(x-15, y-15, color_2);
  tft.drawPixel(x-16, y-15, color_2);

  tft.drawPixel(x-2, y-16, color_2);
  tft.drawPixel(x-3, y-16, color_2);
  tft.drawPixel(x-4, y-16, color_1);
  tft.drawPixel(x-5, y-16, color_1);
  tft.drawPixel(x-6, y-16, color_1);
  tft.drawPixel(x-7, y-16, color_1);
  tft.drawPixel(x-8, y-16, color_1);
  tft.drawPixel(x-9, y-16, color_1);
  tft.drawPixel(x-10, y-16, color_1);
  tft.drawPixel(x-11, y-16, color_1);
  tft.drawPixel(x-12, y-16, color_1);
  tft.drawPixel(x-13, y-16, color_1);
  tft.drawPixel(x-14, y-16, color_1);
  tft.drawPixel(x-15, y-16, color_1);
  tft.drawPixel(x-16, y-16, color_2);
  tft.drawPixel(x-17, y-16, color_2);

  tft.drawPixel(x-1, y-17, color_2);
  tft.drawPixel(x-2, y-17, color_2);
  tft.drawPixel(x-3, y-17, color_2);
  tft.drawPixel(x-4, y-17, color_2);
  tft.drawPixel(x-5, y-17, color_2);
  tft.drawPixel(x-6, y-17, color_2);
  tft.drawPixel(x-7, y-17, color_2);
  tft.drawPixel(x-8, y-17, color_2);
  tft.drawPixel(x-9, y-17, color_2);
  tft.drawPixel(x-10, y-17, color_2);
  tft.drawPixel(x-11, y-17, color_2);
  tft.drawPixel(x-12, y-17, color_2);
  tft.drawPixel(x-13, y-17, color_2);
  tft.drawPixel(x-14, y-17, color_2);
  tft.drawPixel(x-15, y-17, color_2);
  tft.drawPixel(x-16, y-17, color_2);
  tft.drawPixel(x-17, y-17, color_2);
  tft.drawPixel(x-18, y-17, color_2);

  tft.drawPixel(x-1, y-18, color_2);
  tft.drawPixel(x-2, y-18, color_1);
  tft.drawPixel(x-3, y-18, color_1);
  tft.drawPixel(x-4, y-18, color_1);
  tft.drawPixel(x-5, y-18, color_1);
  tft.drawPixel(x-6, y-18, color_1);
  tft.drawPixel(x-7, y-18, color_1);
  tft.drawPixel(x-8, y-18, color_1);
  tft.drawPixel(x-9, y-18, color_1);
  tft.drawPixel(x-10, y-18, color_1);
  tft.drawPixel(x-11, y-18, color_1);
  tft.drawPixel(x-12, y-18, color_1);
  tft.drawPixel(x-13, y-18, color_1);
  tft.drawPixel(x-14, y-18, color_1);
  tft.drawPixel(x-15, y-18, color_1);
  tft.drawPixel(x-16, y-18, color_1);
  tft.drawPixel(x-17, y-18, color_1);
  tft.drawPixel(x-18, y-18, color_2);

  tft.drawPixel(x-1, y-19, color_2);
  tft.drawPixel(x-2, y-19, color_2);
  tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x-4, y-19, color_2);
  tft.drawPixel(x-5, y-19, color_2);
  tft.drawPixel(x-6, y-19, color_2);
  tft.drawPixel(x-7, y-19, color_2);
  tft.drawPixel(x-8, y-19, color_2);
  tft.drawPixel(x-9, y-19, color_2);
  tft.drawPixel(x-10, y-19, color_2);
  tft.drawPixel(x-11, y-19, color_2);
  tft.drawPixel(x-12, y-19, color_2);
  tft.drawPixel(x-13, y-19, color_2);
  tft.drawPixel(x-14, y-19, color_2);
  tft.drawPixel(x-15, y-19, color_2);
  tft.drawPixel(x-16, y-19, color_2);
  tft.drawPixel(x-17, y-19, color_2);
  tft.drawPixel(x-18, y-19, color_2);
}

















void drawBishop2(uint16_t x, uint16_t y, uint16_t color_1, uint16_t color_2) {
  tft.drawPixel(x-9, y-0, color_2);
  tft.drawPixel(x-10, y-0, color_2);

  tft.drawPixel(x-8, y-1, color_2);
  tft.drawPixel(x-9, y-1, color_1);
  tft.drawPixel(x-10, y-1, color_1);
  tft.drawPixel(x-11, y-1, color_2);

  tft.drawPixel(x-9, y-2, color_2);
  tft.drawPixel(x-10, y-2, color_2);

  tft.drawPixel(x-7, y-3, color_2);
  tft.drawPixel(x-8, y-3, color_2);
  tft.drawPixel(x-9, y-3, color_1);
  tft.drawPixel(x-10, y-3, color_1);
  tft.drawPixel(x-11, y-3, color_2);
  tft.drawPixel(x-12, y-3, color_2);

  tft.drawPixel(x-7, y-4, color_2);
  tft.drawPixel(x-8, y-4, color_1);
  tft.drawPixel(x-9, y-4, color_1);
  tft.drawPixel(x-10, y-4, color_2);
  tft.drawPixel(x-11, y-4, color_2);
  tft.drawPixel(x-12, y-4, color_2);

  tft.drawPixel(x-6, y-5, color_2);
  tft.drawPixel(x-7, y-5, color_2);
  tft.drawPixel(x-8, y-5, color_1);
  tft.drawPixel(x-9, y-5, color_1);
  tft.drawPixel(x-10, y-5, color_2);
  tft.drawPixel(x-11, y-5, color_2);
  tft.drawPixel(x-12, y-5, color_2);
  tft.drawPixel(x-13, y-5, color_2);

  tft.drawPixel(x-6, y-6, color_2);
  tft.drawPixel(x-7, y-6, color_1);
  tft.drawPixel(x-8, y-6, color_1);
  tft.drawPixel(x-9, y-6, color_1);
  tft.drawPixel(x-10, y-6, color_2);
  tft.drawPixel(x-11, y-6, color_2);
  tft.drawPixel(x-12, y-6, color_1);
  tft.drawPixel(x-13, y-6, color_2);

  tft.drawPixel(x-6, y-7, color_2);
  tft.drawPixel(x-7, y-7, color_1);
  tft.drawPixel(x-8, y-7, color_1);
  tft.drawPixel(x-9, y-7, color_2);
  tft.drawPixel(x-10, y-7, color_2);
  tft.drawPixel(x-11, y-7, color_1);
  tft.drawPixel(x-12, y-7, color_1);
  tft.drawPixel(x-13, y-7, color_2);

  tft.drawPixel(x-6, y-8, color_2);
  tft.drawPixel(x-7, y-8, color_1);
  tft.drawPixel(x-8, y-8, color_1);
  tft.drawPixel(x-9, y-8, color_2);
  tft.drawPixel(x-10, y-8, color_2);
  tft.drawPixel(x-11, y-8, color_1);
  tft.drawPixel(x-12, y-8, color_1);
  tft.drawPixel(x-13, y-8, color_2);

  tft.drawPixel(x-6, y-9, color_2);
  tft.drawPixel(x-7, y-9, color_1);
  tft.drawPixel(x-8, y-9, color_1);
  tft.drawPixel(x-9, y-9, color_1);
  tft.drawPixel(x-10, y-9, color_1);
  tft.drawPixel(x-11, y-9, color_1);
  tft.drawPixel(x-12, y-9, color_1);
  tft.drawPixel(x-13, y-9, color_2);

  tft.drawPixel(x-6, y-10, color_2);
  tft.drawPixel(x-7, y-10, color_1);
  tft.drawPixel(x-8, y-10, color_1);
  tft.drawPixel(x-9, y-10, color_1);
  tft.drawPixel(x-10, y-10, color_1);
  tft.drawPixel(x-11, y-10, color_1);
  tft.drawPixel(x-12, y-10, color_1);
  tft.drawPixel(x-13, y-10, color_2);

  tft.drawPixel(x-6, y-11, color_2);
  tft.drawPixel(x-7, y-11, color_1);
  tft.drawPixel(x-8, y-11, color_1);
  tft.drawPixel(x-9, y-11, color_1);
  tft.drawPixel(x-10, y-11, color_1);
  tft.drawPixel(x-11, y-11, color_1);
  tft.drawPixel(x-12, y-11, color_1);
  tft.drawPixel(x-13, y-11, color_2);

  tft.drawPixel(x-6, y-12, color_2);
  tft.drawPixel(x-7, y-12, color_2);
  tft.drawPixel(x-8, y-12, color_1);
  tft.drawPixel(x-9, y-12, color_1);
  tft.drawPixel(x-10, y-12, color_1);
  tft.drawPixel(x-11, y-12, color_1);
  tft.drawPixel(x-12, y-12, color_2);
  tft.drawPixel(x-13, y-12, color_2);

  tft.drawPixel(x-7, y-13, color_2);
  tft.drawPixel(x-8, y-13, color_2);
  tft.drawPixel(x-9, y-13, color_1);
  tft.drawPixel(x-10, y-13, color_1);
  tft.drawPixel(x-11, y-13, color_2);
  tft.drawPixel(x-12, y-13, color_2);

  tft.drawPixel(x-8, y-14, color_2);
  tft.drawPixel(x-9, y-14, color_1);
  tft.drawPixel(x-10, y-14, color_1);
  tft.drawPixel(x-11, y-14, color_2);

  tft.drawPixel(x-7, y-15, color_2);
  tft.drawPixel(x-8, y-15, color_2);
  tft.drawPixel(x-9, y-15, color_1);
  tft.drawPixel(x-10, y-15, color_1);
  tft.drawPixel(x-11, y-15, color_2);
  tft.drawPixel(x-12, y-15, color_2);

  tft.drawPixel(x-6, y-16, color_2);
  tft.drawPixel(x-7, y-16, color_2);
  tft.drawPixel(x-8, y-16, color_1);
  tft.drawPixel(x-9, y-16, color_1);
  tft.drawPixel(x-10, y-16, color_1);
  tft.drawPixel(x-11, y-16, color_1);
  tft.drawPixel(x-12, y-16, color_2);
  tft.drawPixel(x-13, y-16, color_2);

  tft.drawPixel(x-3, y-17, color_2);
  tft.drawPixel(x-4, y-17, color_2);
  tft.drawPixel(x-5, y-17, color_2);
  tft.drawPixel(x-6, y-17, color_2);
  tft.drawPixel(x-7, y-17, color_2);
  tft.drawPixel(x-8, y-17, color_2);
  tft.drawPixel(x-9, y-17, color_2);
  tft.drawPixel(x-10, y-17, color_2);
  tft.drawPixel(x-11, y-17, color_2);
  tft.drawPixel(x-12, y-17, color_2);
  tft.drawPixel(x-13, y-17, color_2);
  tft.drawPixel(x-14, y-17, color_2);
  tft.drawPixel(x-15, y-17, color_2);
  tft.drawPixel(x-16, y-17, color_2);

  tft.drawPixel(x-3, y-18, color_2);
  tft.drawPixel(x-4, y-18, color_1);
  tft.drawPixel(x-5, y-18, color_1);
  tft.drawPixel(x-6, y-18, color_1);
  tft.drawPixel(x-7, y-18, color_1);
  tft.drawPixel(x-8, y-18, color_1);
  tft.drawPixel(x-9, y-18, color_1);
  tft.drawPixel(x-10, y-18, color_1);
  tft.drawPixel(x-11, y-18, color_1);
  tft.drawPixel(x-12, y-18, color_1);
  tft.drawPixel(x-13, y-18, color_1);
  tft.drawPixel(x-14, y-18, color_1);
  tft.drawPixel(x-15, y-18, color_1);
  tft.drawPixel(x-16, y-18, color_2);

  tft.drawPixel(x-3, y-19, color_2);
  tft.drawPixel(x-4, y-19, color_2);
  tft.drawPixel(x-5, y-19, color_2);
  tft.drawPixel(x-6, y-19, color_2);
  tft.drawPixel(x-7, y-19, color_2);
  tft.drawPixel(x-8, y-19, color_2);
  tft.drawPixel(x-9, y-19, color_2);
  tft.drawPixel(x-10, y-19, color_2);
  tft.drawPixel(x-11, y-19, color_2);
  tft.drawPixel(x-12, y-19, color_2);
  tft.drawPixel(x-13, y-19, color_2);
  tft.drawPixel(x-14, y-19, color_2);
  tft.drawPixel(x-15, y-19, color_2);
  tft.drawPixel(x-16, y-19, color_2);
}
