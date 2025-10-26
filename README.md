# Embedded Chess

This is my first large scale project attempt on an Arduino, starting back in 2022. My original code (which I have added to the version history) is not very readable, but you are free to give it a go. 

## How to setup and run this project
This project was designed in the Arduino IDE, with the .zip files in the libraries folder installed (separately extracted and added to the include path).


## Requirements
* An Arduino Mega 2560 or equivalent with at least 80KB program storage space and 2.5KB of dynamic memory
* A TFTLCD touchscreen - [like this one](https://www.amazon.co.uk/Elegoo-EL-SM-004-Inches-Technical-Arduino/dp/B01EUVJYME)

## Program Structure
Program is split into the following sections
- the main "chess.ino" file; contains the setup() and loop() functions, the required functions in an arduino project; files: chess.ino
- the Draw section: contains the interface between the code and the TFT hardware; Draw.h, Draw.cpp
- the Game section: contains the implementation of the game of chess specifically; Game.h, Game.cpp, Zobrist.h, Zobrist.cpp, Notation.h, Notation.cpp
- the Engine section: contains the implementation of the chess engine; Engine.h, Engine.cpp
- other helper files: Types.h, Setup.h, Setup.cpp
