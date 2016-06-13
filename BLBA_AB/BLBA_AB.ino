/*
  Blob Attack: http://www.team-arg.org/blba-manual.html

  Arduboy version 1.6:  http://www.team-arg.org/blba-downloads.html

  MADE by TEAM a.r.g. : http://www.team-arg.org/more-about.html

  2015 - Game by JO3RI - All art by CastPixel: https://twitter.com/castpixel

  Game License: MIT : https://opensource.org/licenses/MIT

*/

//determine the game
#define GAME_ID 28

#include "Arglib.h"
#include "globals.h"
#include "menu.h"
#include "game.h"

typedef void (*FunctionPointer) ();

const FunctionPointer PROGMEM  mainGameLoop[] = {
  stateMenuIntro,
  stateMenuMain,
  stateMenuHelp,
  stateMenuPlay,
  stateMenuInfo,
  stateMenuSoundfx,
  stateGamePlaying,
  stateGamePause,
  stateGameOver,
};


void setup() {
  arduboy.start();
  arduboy.setFrameRate(60);                                 // set the frame rate of the game at 60 fps
  arduboy.initRandomSeed();
}

void loop() {
  if (!(arduboy.nextFrame())) return;
  arduboy.poll();
  arduboy.clearDisplay();
  ((FunctionPointer) pgm_read_word (&mainGameLoop[gameState]))();
  if (arduboy.everyXFrames(1)) elfPausedFrame++;
  if (elfPausedFrame > 20)elfPausedFrame = 0;
  arduboy.display();
}

