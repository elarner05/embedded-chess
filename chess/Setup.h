
// File created: Sept 2025

#pragma once
#include <Arduino.h>

#include <Elegoo_GFX.h>     // Core graphics library
#include <Elegoo_TFTLCD.h>  // Hardware-specific library
#include <TouchScreen.h>    // Touch-screen library

constexpr uint8_t LCD_CS = A3;  // Chip Select goes to Analog 3
constexpr uint8_t LCD_CD = A2;  // Command/Data goes to Analog 2
constexpr uint8_t LCD_WR = A1;  // LCD Write goes to Analog 1
constexpr uint8_t LCD_RD = A0;  // LCD Read goes to Analog 0

constexpr int SD_SCK_PIN = 13;

constexpr uint8_t LCD_RESET = A4;  // Can alternately connect to reset pin



constexpr uint8_t YP = A3;  // Must be an analog pin
constexpr uint8_t XM = A2;  // Must be an analog pin
constexpr int YM = 9;   // Can be a digital pin
constexpr int XP = 8;   // Can be a digital pin

constexpr int MINPRESSURE = 5;
constexpr int MAXPRESSURE = 1000;

// Some calibration numbers for the touchscreen screen, found through testing
constexpr int TS_MINX = 110;
constexpr int TS_MAXX = 910;

constexpr int TS_MINY = 70;
constexpr int TS_MAXY = 900;

extern Elegoo_TFTLCD tft;

extern TouchScreen ts;  // Setup the TouchScreen Object

constexpr int MILLIS_BETWEEN_PRESS = 250;  // The delay required between presses on the screen. Prevents double pressing the same spot accidently
extern unsigned long lastPress;

