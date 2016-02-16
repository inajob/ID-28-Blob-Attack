/*
 Blob Attack: http://www.team-arg.org/blba-manual.html

 Arduboy version 1.5:  http://www.team-arg.org/blba-downloads.html

 MADE by TEAM a.r.g. : http://www.team-arg.org/more-about.html

 2015 - Game by JO3RI - All art by CastPixel: https://twitter.com/castpixel

 Game License: MIT : https://opensource.org/licenses/MIT

 */

//determine the game
#define GAME_ID 28

#include <SPI.h>
#include <EEPROM.h>
#include "Arglib.h"
#include "menu_bitmaps.h"
#include "blobs_bitmaps.h"
#include "number_bitmaps.h"
#include "playfield_bitmaps.h"

//define menu states (on main menu)
#define STATE_MENU_MAIN          0
#define STATE_MENU_HELP          1
#define STATE_MENU_INFO          3
#define STATE_MENU_SOUNDFX       4

//define menu choices (on main menu)
#define CHOOSE_HELP              1
#define CHOOSE_PLAY              2
#define CHOOSE_INFO              3
#define CHOOSE_CONF              4

//define game states (on main menu)
#define STATE_GAME_INIT          2
#define STATE_GAME_PLAYING       8
#define STATE_GAME_PAUSE         9
#define STATE_GAME_OVER          10

#define PLAYFIELD_WIDTH          8         // Playfield width in blobs
#define PLAYFIELD_HEIGHT         10        // Playfield height in blobs
#define PLAYFIELD_ZERO_X         2         // zero x-position of the playfield in pixel position on the screen
#define PLAYFIELD_ZERO_Y         3         // zero x-position of the playfield in pixel position on the screen
#define BLOB_FREE                0         // an empty place in the playfield

#define BLOB_PIXELS              6
#define TILES_IN_BLOBS           3      // number of vertical or horizontal tiles in blobsgrid

#define BLOB_CURRENT             0
#define BLOB_NEXT                2
#define BLOB_WAITING             4

#define NO_FLAG_ON_FIELD         0
#define FLAG_ON_FIELD            1

#define ELF_NORMAL               0
#define ELF_THUMBSUP             1
#define ELF_STRESSED             2
#define ELF_PAUSED               3

Arduboy arduboy;
SimpleButtons buttons (arduboy);

unsigned char gameState = STATE_MENU_MAIN;
boolean soundYesNo;
boolean giveExtraScore;
boolean canMoveBlobsDown;
boolean showCombo;
byte elfState;

int menuSelection;

unsigned long scorePlayer;
unsigned long extraScoreForChain;

int field[PLAYFIELD_WIDTH][PLAYFIELD_HEIGHT];
int fieldFlags[PLAYFIELD_WIDTH][PLAYFIELD_HEIGHT];

int randomBlobPit[6];
int blobNumbers;
byte blobFrame;
byte bouncingBallFrame;
byte bouncingBallSequence[] = {0, 1, 2, 0, 0, 0, 0, 0};
byte bouncingHeight[] = {0, 0, 0, 1, 0, 0, 0, 0};
byte elfStressedBodySequenceY[] = {34, 32, 33};
byte elfStressedHeadSequenceY[] = {0, 4, 2};
byte elfStressedWandSequenceX[] = {51, 51, 50};
byte elfStressedWandSequenceY[] = {26, 24, 26};
byte elfStressedFrame;
byte elfPausedBodySequenceY[] = {38, 37, 37, 37, 37, 37, 37, 38, 40, 41, 42, 42, 42, 42, 42, 42, 42, 42, 41, 40, 39,};
byte elfPausedHeadSequenceX[] = {50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 52, 52, 52, 52, 52, 52, 52, 51, 51, 50, 50,};
byte elfPausedHeadSequenceY[] = { 4, 4, 4, 4, 4, 4, 4, 4, 3, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4,};
byte elfPausedWandSequenceX[] = {100, 100, 100, 100, 100, 100, 100, 101, 100, 99, 98, 97, 97, 97, 97, 97, 97, 97, 97, 98, 99, 100,};
byte elfPausedWandSequenceY[] = { 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 2, 1, 1, 1, 1, 1, 1, 0, 1, 2, 3,};
byte elfPausedMouthSequence[] = {0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 4, 3, 2, 2, 5};
byte elfPausedFrame;
byte elfNormalEyesSequence[] = {0, 1, 2, 3, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,};
byte elfNormalFrame;
byte infoSequence[] = {1,2,3,4,5,4,3,2};
byte thumbsUpFrame;
byte chain;
int currentBlobs[] =
{
  // array for current blob
  0, 0, 0,
  0, 0, 0,
  0, 0, 0,
};

int blobsXY[2];          // X and Y coordinates for the small 3x3 blob grid


void setup()
{
  arduboy.start();
  arduboy.setFrameRate(60);
  arduboy.drawCompressed(0, 8, TEAMarg, WHITE);
  arduboy.display();
  delay(3000);
  gameState = STATE_MENU_MAIN;
  if (EEPROM.read(EEPROM_AUDIO_ON_OFF)) soundYesNo = true;
  if (soundYesNo == true) arduboy.audio.on();
  else arduboy.audio.off();
  arduboy.initRandomSeed();
  menuSelection = CHOOSE_PLAY;
}



void loop()
{
  if (!(arduboy.nextFrame())) return;
  buttons.poll();
  arduboy.clearDisplay();
  switch (gameState)
  {
    case STATE_MENU_MAIN:
      if (arduboy.everyXFrames(1)) bouncingBallFrame++;
      if (bouncingBallFrame > 7) bouncingBallFrame = 0;
      // show the splash art
      arduboy.drawCompressed(0, 0, splashScreen_compressed, WHITE);
      arduboy.drawSprite(17 + (24 * (menuSelection - 1)), 52, selectorMask_bitmap, 25, 11, 0, BLACK);
      arduboy.drawSprite(17 + (24 * (menuSelection - 1)), 52, selector_bitmap, 25, 11, 0, WHITE);
      arduboy.drawSprite(19, 54, mainMenu_bitmap, 93, 8, 0, BLACK);
      arduboy.drawSprite(25 + (24 * (menuSelection - 1)), 44 - bouncingHeight[bouncingBallFrame], bouncingBallMask_bitmaps, 9, 8, bouncingBallSequence[bouncingBallFrame], WHITE);
      arduboy.drawSprite(25 + (24 * (menuSelection - 1)), 44 - bouncingHeight[bouncingBallFrame], bouncingBall_bitmaps, 9, 8, bouncingBallSequence[bouncingBallFrame], BLACK);
      if (buttons.justPressed(RIGHT_BUTTON) && (menuSelection < 4))menuSelection++;
      if (buttons.justPressed(LEFT_BUTTON) && (menuSelection > 1))menuSelection--;
      if (buttons.justPressed(A_BUTTON | B_BUTTON)) gameState = menuSelection;
      break;
    case STATE_MENU_HELP: // QR code
      arduboy.drawCompressed(32, 0, qrcode_compressed, WHITE);
      if (buttons.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_MENU_MAIN;
      break;
    case STATE_MENU_INFO: // infoscreen
      if (arduboy.everyXFrames(2)) bouncingBallFrame++;
      if (bouncingBallFrame > 7)bouncingBallFrame = 0;
      arduboy.drawCompressed(19, 27, madeBy_compressed, WHITE);
      arduboy.drawCompressed(5, infoSequence[bouncingBallFrame], gameTitle_compressed, WHITE);
      if (buttons.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_MENU_MAIN;
      break;
    case STATE_MENU_SOUNDFX: // soundconfig screen
      if (arduboy.everyXFrames(1)) bouncingBallFrame++;
      if (bouncingBallFrame > 7)bouncingBallFrame = 0;
      arduboy.drawCompressed(0, 0, splashScreen_compressed, WHITE);
      arduboy.drawSprite(31, 54, soundMenu_bitmap, 66, 8, 0, BLACK);
      arduboy.drawRect(61 + (19 * soundYesNo), 52, 20, 11, BLACK);
      arduboy.drawSprite(67 + (19 * soundYesNo), 44 - bouncingHeight[bouncingBallFrame], bouncingBallMask_bitmaps, 9, 8, bouncingBallSequence[bouncingBallFrame], WHITE);
      arduboy.drawSprite(67 + (19 * soundYesNo), 44 - bouncingHeight[bouncingBallFrame], bouncingBall_bitmaps, 9, 8, bouncingBallSequence[bouncingBallFrame], BLACK);
      if (buttons.justPressed(RIGHT_BUTTON))soundYesNo = true;
      if (buttons.justPressed(LEFT_BUTTON))soundYesNo = false;
      if (soundYesNo == true) arduboy.audio.on();
      else arduboy.audio.off();
      if (buttons.justPressed(A_BUTTON | B_BUTTON))
      {
        arduboy.audio.save_on_off();
        gameState = STATE_MENU_MAIN;
      }
      break;
    case STATE_GAME_INIT:
      scorePlayer = 0;
      blobNumbers = 0;
      blobFrame = 0;
      initPlayfield();                        // let's clean up the playfield and start with fresh ones
      removeFlag();
      fillBlobPit();                          // fill the pit with random blobs.
      createCurrentBlobs();
      giveExtraScore = false;
      canMoveBlobsDown = true;
      showCombo = false;
      chain = 0;
      elfState = ELF_NORMAL;
      elfStressedFrame = 0;
      elfPausedFrame = 0;
      thumbsUpFrame = 0;
      gameState = STATE_GAME_PLAYING;
      break;
    case STATE_GAME_PLAYING:
      if (arduboy.everyXFrames(30))dropBlobs();
      updateStage();
      if (buttons.justPressed(RIGHT_BUTTON)) moveBlobsRight();
      if (buttons.justPressed(LEFT_BUTTON)) moveBlobsLeft();
      if (buttons.justPressed(DOWN_BUTTON)) dropBlobs();
      if (buttons.justPressed(B_BUTTON)) rotateBlobsRight();
      if (buttons.justPressed(A_BUTTON)) rotateBlobsLeft();
      if (buttons.justPressed(UP_BUTTON))
      {
        gameState = STATE_GAME_PAUSE;
        elfState = ELF_PAUSED;
      }
      break;
    case STATE_GAME_OVER:
      drawDitherBackground();
      drawStressedElf();
      arduboy.drawRect(0, 0, 51, 64, WHITE);
      arduboy.drawSprite(4, 16, youLose_bitmap, 43, 16, 0, WHITE);
      arduboy.fillRect(4, 32, 43, 9, WHITE);
      scoreDraw(6, 34);
      if (buttons.justPressed(A_BUTTON | B_BUTTON))
      {
        gameState = STATE_MENU_MAIN;
      }
      break;
    case STATE_GAME_PAUSE:
      elfState = ELF_PAUSED;
      updateStage();
      if (buttons.justPressed(UP_BUTTON))
      {
        gameState = STATE_GAME_PLAYING;
        elfState = ELF_NORMAL;
      }
      break;
  }
  if (arduboy.everyXFrames(1)) elfPausedFrame++;
  if (elfPausedFrame > 20)elfPausedFrame = 0;
  arduboy.display();
}


// playfield
//-----------

void initPlayfield()
{
  for (int x = 0; x < PLAYFIELD_WIDTH; x++)
  {
    for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
    {
      field[x][y] = BLOB_FREE;
    }
  }
}

void removeFlag()
{
  for (int x = 0; x < PLAYFIELD_WIDTH; x++)
  {
    for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
    {
      fieldFlags[x][y] = NO_FLAG_ON_FIELD;
    }
  }
}

void storeBlob(int array_x, int array_y)
{
  int draw_pointer = 0;
  for (int y = array_y; y < array_y + TILES_IN_BLOBS; y++)
  {
    for (int x = array_x; x < array_x + TILES_IN_BLOBS; x++)
    {
      if (currentBlobs[draw_pointer] != 0) field[x][y] = currentBlobs[draw_pointer];
      draw_pointer++;
    }
  }
}

void storeOneBlob(int array_x, int array_y)
{
  // if the blob is not on the floor
  if ((array_y) < PLAYFIELD_HEIGHT - 2)
  {

    for (int x = 0; x < TILES_IN_BLOBS; x++)
    {
      if ((!isTileFree(array_x + x, array_y + 2)) && (!isOnlyOneBlob()) && (currentBlobs[3 + x] != 0))
      {
        field [array_x + x][array_y + 1] = currentBlobs[3 + x];
        currentBlobs[3 + x] = 0;
      }
    }
  }
}

void drawField()
{
  for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
  {
    for (int x = 0 ; x < PLAYFIELD_WIDTH; x++)
    {
      // draw every tile in the playfield
      if (field [x][y] != BLOB_FREE) arduboy.drawSprite((x * BLOB_PIXELS) + PLAYFIELD_ZERO_X, (y * BLOB_PIXELS) + PLAYFIELD_ZERO_Y, animatedBlobs_bitmaps, 5, 8, 0, WHITE);
      arduboy.drawSprite((x * BLOB_PIXELS) + PLAYFIELD_ZERO_X, (y * BLOB_PIXELS) + PLAYFIELD_ZERO_Y, animatedBlobs_bitmaps, 5, 8, (4 * field[x][y]) + blobFrame, BLACK);
    }
  }
}

boolean isTileFree(int array_x, int array_y)
{
  if (field [array_x][array_y] == BLOB_FREE) return true;
  else return false;
}

boolean aboveIsSameBlob(int array_x, int array_y)
{
  if ((array_y - 1 > 0) && (field [array_x][array_y] == field [array_x][array_y - 1])) return true;
  else return false;
}

boolean underIsSameBlob(int array_x, int array_y)
{
  if ((array_y + 1 < PLAYFIELD_HEIGHT) && (field [array_x][array_y] == field [array_x][array_y + 1])) return true;
  else return false;
}

boolean rightIsSameBlob(int array_x, int array_y)
{
  if ((array_x + 1 < PLAYFIELD_WIDTH ) && (field [array_x][array_y] == field [array_x + 1][array_y])) return true;
  else return false;
}

boolean leftIsSameBlob(int array_x, int array_y)
{
  if ((array_x - 1 > 0 ) && (field [array_x][array_y] == field [array_x - 1][array_y])) return true;
  else return false;
}



boolean isMovePossible (int array_x, int array_y)
{
  // checks collision with blocks already stored in the playfield
  // check if the 3x3 tiles of a blob with the correct area in the playfield provide by draw_x and draw_y
  int draw_pointer = 0;
  for (int y = array_y; y < array_y + TILES_IN_BLOBS; y++)
  {
    for (int x = array_x; x < array_x + TILES_IN_BLOBS; x++)
    {
      // check if the block is outside the limits of the playfield
      if (x < 0 || x > PLAYFIELD_WIDTH - 1 || y > PLAYFIELD_HEIGHT - 1)
      {
        byte temp = currentBlobs[draw_pointer];
        if ( temp != 0) return false;
      }

      // check if the block has collided with tile already stored in the playfield array
      byte temp = currentBlobs[draw_pointer];
      if ((temp != 0) && (isTileFree(x, y) == false)) return false;
      draw_pointer++;
    }
  }
  return true;
}
boolean isOneBlobDropPossible(int array_x, int array_y)
{
  // checks 1 blob collision with blobs already stored in the playfield
  if ((currentBlobs[1] == 0) && (currentBlobs[7] == 0))
  {
    for (byte temp = 3; temp < 6; temp++)
    {
      if ((currentBlobs[temp] != 0) && (isTileFree(array_x, array_y) == false)) return true;
    }
  }
  else return false;
}

boolean isOnlyOneBlob()
{
  byte temp = 0;
  for (int i = 0; i < 9; i++)
  {
    if (currentBlobs[i] != 0)temp++;
  }
  if (temp < 2) return true;
  else return false;
}

void deletePossibleBlobs()
{
  while (canMoveBlobsDown)
  {
    updateStage();
    fourInPack();
    fourInColumn();
    fourInRow();
    removeGroups();
    moveBlobsDown();
    //delay(200);
  }
  canMoveBlobsDown = true;
}

void moveBlobsDown()
{
  canMoveBlobsDown = false;
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (isTileFree(column, row))
      {
        if (!isTileFree(column, row - 1))
        {
          field [column][row] = field [column][row - 1];
          field [column][row - 1] = BLOB_FREE;
          drawField();
          arduboy.drawSprite((column * BLOB_PIXELS) + PLAYFIELD_ZERO_X, (row * BLOB_PIXELS) + PLAYFIELD_ZERO_Y, smallBlobs_bitmaps, 5, 8, 0, BLACK);
          arduboy.drawSprite((column * BLOB_PIXELS) + PLAYFIELD_ZERO_X, (row * BLOB_PIXELS) + PLAYFIELD_ZERO_Y, smallBlobsInverted_bitmaps, 5, 8, field [column][row], WHITE);
          arduboy.display();
          canMoveBlobsDown = true;
        }
      }
    }
  }
}

void removeGroups()
{
  for (int x = 0; x < PLAYFIELD_WIDTH; x++)
  {
    for (int y = 0; y < PLAYFIELD_HEIGHT; y++)
    {
      if (fieldFlags[x][y] == FLAG_ON_FIELD)
      {
        giveExtraScore = true;
        field[x][y] = BLOB_FREE;
        scorePlayer += 50;
      }
    }
  }
  if (giveExtraScore == true)
  {
    scorePlayer += extraScoreForChain;
    extraScoreForChain += 500;
    chain++;
    if (chain > 1) showCombo = true;
    arduboy.tunes.tone(440, 100);
    delay(100);
    arduboy.tunes.tone(1047, 200);
  }
  giveExtraScore = false;
  removeFlag();
}


void fourInPack()
{
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (!isTileFree(column, row))
      {
        if (aboveIsSameBlob(column, row) && rightIsSameBlob(column, row) && aboveIsSameBlob(column + 1, row))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 1] = FLAG_ON_FIELD;
        }
        if (rightIsSameBlob(column, row) && aboveIsSameBlob(column + 1, row) && rightIsSameBlob(column + 1, row - 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 2][row - 1] = FLAG_ON_FIELD;
        }
        if (rightIsSameBlob(column, row) && underIsSameBlob(column + 1, row) && rightIsSameBlob(column + 1, row + 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row + 1] = FLAG_ON_FIELD;
          fieldFlags[column + 2][row + 1] = FLAG_ON_FIELD;
        }
        if (aboveIsSameBlob(column, row) && rightIsSameBlob(column, row - 1) && aboveIsSameBlob(column + 1, row - 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column + 1][row - 2] = FLAG_ON_FIELD;
        }
        if (aboveIsSameBlob(column, row) && leftIsSameBlob(column, row - 1) && aboveIsSameBlob(column - 1, row - 1))
        {
          fieldFlags[column][row] = FLAG_ON_FIELD;
          fieldFlags[column][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column - 1][row - 1] = FLAG_ON_FIELD;
          fieldFlags[column - 1][row - 2] = FLAG_ON_FIELD;
        }
      }
    }
  }
}


void fourInColumn()
{
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (!isTileFree(column, row))
      {
        if (aboveIsSameBlob(column, row) && aboveIsSameBlob(column, row - 1))
        {
          if (aboveIsSameBlob(column, row - 2))
          {
            fieldFlags[column][row] = FLAG_ON_FIELD;
            fieldFlags[column][row - 1] = FLAG_ON_FIELD;
            fieldFlags[column][row - 2] = FLAG_ON_FIELD;
            fieldFlags[column][row - 3] = FLAG_ON_FIELD;
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (rightIsSameBlob(column, row - temp))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column][row - 1] = FLAG_ON_FIELD;
              fieldFlags[column][row - 2] = FLAG_ON_FIELD;
              fieldFlags[column + 1][row - temp] = FLAG_ON_FIELD;
            }
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (leftIsSameBlob(column, row - temp))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column][row - 1] = FLAG_ON_FIELD;
              fieldFlags[column][row - 2] = FLAG_ON_FIELD;
              fieldFlags[column - 1][row - temp] = FLAG_ON_FIELD;
            }
          }
        }
      }
    }
  }
}


void fourInRow()
{
  //check if 4 or more blobs are equal in the same row
  for (byte column = 0; column < PLAYFIELD_WIDTH; column++)
  {
    for (byte row = PLAYFIELD_HEIGHT - 1; row > 0; row--)
    {
      if (!isTileFree(column, row))
      {
        if (rightIsSameBlob(column, row) && rightIsSameBlob(column + 1, row))
        {
          if (rightIsSameBlob(column + 2, row))
          {
            fieldFlags[column][row] = FLAG_ON_FIELD;
            fieldFlags[column + 1][row] = FLAG_ON_FIELD;
            fieldFlags[column + 2][row] = FLAG_ON_FIELD;
            fieldFlags[column + 3][row] = FLAG_ON_FIELD;
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (aboveIsSameBlob(column + temp, row))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column + 1][row] = FLAG_ON_FIELD;
              fieldFlags[column + 2][row] = FLAG_ON_FIELD;
              fieldFlags[column + temp][row - 1] = FLAG_ON_FIELD;
            }
          }
          for (byte temp = 0; temp < 3; temp++)
          {
            if (underIsSameBlob(column + temp, row))
            {
              fieldFlags[column][row] = FLAG_ON_FIELD;
              fieldFlags[column + 1][row] = FLAG_ON_FIELD;
              fieldFlags[column + 2][row] = FLAG_ON_FIELD;
              fieldFlags[column + temp][row + 1] = FLAG_ON_FIELD;
            }
          }
        }
      }
    }
  }
}




// blobs
//-------

void clearBlob(int array_x, int array_y)
{
  field [array_x][array_y] = BLOB_FREE;
}

void moveBlobsRight()
{
  if (!isOnlyOneBlob() && isMovePossible(blobsXY[0] + 1, blobsXY[1])) blobsXY[0]++;
}


void moveBlobsLeft()
{
  if (!isOnlyOneBlob() && isMovePossible(blobsXY[0] - 1, blobsXY[1])) blobsXY[0]--;
}


void dropBlobs()
{
  if (isOneBlobDropPossible(blobsXY[0], blobsXY[1] + 1)) storeOneBlob(blobsXY[0], blobsXY[1]);
  //move down is no longer possible because the field is full, the game is over
  if ((blobsXY[1] == 0) && !isTileFree(blobsXY[0] + 1, 0)) gameState = STATE_GAME_OVER;
  if (isMovePossible(blobsXY[0], blobsXY[1] + 1))
  {
    blobsXY[1]++;
    arduboy.tunes.tone(104, 10);
    extraScoreForChain = 0;
    chain = 0;
  }
  else if (gameState != STATE_GAME_OVER)
  {
    storeBlob(blobsXY[0], blobsXY[1]);
    scorePlayer += 10;
    deletePossibleBlobs();
    createCurrentBlobs();
  }
}

void rotateBlobsRight()
{
  if (!isOnlyOneBlob())
  {
    byte temp = currentBlobs[1];
    currentBlobs[1] = currentBlobs[3];
    currentBlobs[3] = currentBlobs[7];
    currentBlobs[7] = currentBlobs[5];
    currentBlobs[5] = temp;
    arduboy.tunes.tone(330, 25);
  }
  if (!isMovePossible(blobsXY[0], blobsXY[1]))
  {
    byte temp = currentBlobs[1];
    currentBlobs[1] = currentBlobs[5];
    currentBlobs[5] = currentBlobs[7];
    currentBlobs[7] = currentBlobs[3];
    currentBlobs[3] = temp;
  }
}

void rotateBlobsLeft()
{
  if (!isOnlyOneBlob())
  {
    byte temp = currentBlobs[1];
    currentBlobs[1] = currentBlobs[5];
    currentBlobs[5] = currentBlobs[7];
    currentBlobs[7] = currentBlobs[3];
    currentBlobs[3] = temp;
    arduboy.tunes.tone(330, 25);
  }
  if (!isMovePossible(blobsXY[0], blobsXY[1]))
  {
    byte temp = currentBlobs[1];
    currentBlobs[1] = currentBlobs[3];
    currentBlobs[3] = currentBlobs[7];
    currentBlobs[7] = currentBlobs[5];
    currentBlobs[5] = temp;
  }
}


void fillBlobPit()
{
  for (byte x = 0; x < 6; x++)
  {
    randomBlobPit[x] = random(1, 4);
  }
}

void drawBlobs (int draw_x, int draw_y, int which_blobs)
{
  switch (which_blobs)
  {
    case BLOB_CURRENT:
      {
        int draw_pointer = 0;
        for (int y = draw_y; y < draw_y + 18; y = y + BLOB_PIXELS)
        {
          for (int x = draw_x; x < draw_x + 18; x = x + BLOB_PIXELS)
          {
            int temp = currentBlobs[draw_pointer];
            if (temp > 0)
            {
              arduboy.drawSprite(x, y, animatedBlobsInverted_bitmaps, 5, 8, (4 * temp) + blobFrame, WHITE);
            }
            draw_pointer++;
          }
        }

        break;
      }
    case BLOB_NEXT:
      arduboy.drawSprite(draw_x, draw_y, bigBlobs_bitmaps, 10, 16, randomBlobPit[0], WHITE); //randomBlobPit[0]
      arduboy.drawSprite(draw_x, draw_y + (2 * BLOB_PIXELS), bigBlobs_bitmaps, 10, 16, randomBlobPit[1], WHITE); //randomBlobPit[1]
      break;
    case BLOB_WAITING:
      arduboy.drawSprite(draw_x, draw_y, smallBlobs_bitmaps, 5, 8, randomBlobPit[2], WHITE);
      arduboy.drawSprite(draw_x, draw_y + BLOB_PIXELS, smallBlobs_bitmaps, 5, 8, randomBlobPit[3], WHITE);
      break;

  }
}

void createCurrentBlobs ()
{
  blobsXY[0] = 2;     //player X
  blobsXY[1] = 0;     //player Y
  for (byte i = 0; i < 9; i++)
  {
    currentBlobs[i] = 0;
  }
  currentBlobs[1] = randomBlobPit[0];
  currentBlobs[4] = randomBlobPit[1];

  for (byte i = 0; i < 4; i++)
  {
    randomBlobPit[i] = randomBlobPit[i + 2];
  }

  randomBlobPit[4] = random(1, 6);
  randomBlobPit[5] = random(1, 6);
}

boolean isRowThreeFree()
{
  for (byte array_x = 0; array_x < 8; array_x++)
  {
    if (field [array_x][3] == BLOB_FREE) return true;
    else return false;
  }

}

void checkIfBlobsAreGettingToHigh()
{
  for (byte array_x = 0; array_x < 8; array_x++)
  {
    if (field [array_x][4] == BLOB_FREE)
    {
      if (showCombo) elfState = ELF_THUMBSUP;
      else elfState = ELF_NORMAL;
    }
    else
    {
      elfState = ELF_STRESSED;
      break;
    }
  }
}

// stage
//-------
void updateStage()
{
  if (gameState != STATE_GAME_PAUSE) checkIfBlobsAreGettingToHigh();
  if (showCombo) elfState = ELF_THUMBSUP;
  switch (elfState)
  {
    case ELF_NORMAL:
      if (arduboy.everyXFrames(6)) blobFrame++;
      if (blobFrame > 3)blobFrame = 0;
      drawNormalElf();
      break;
    case ELF_THUMBSUP:
      if (arduboy.everyXFrames(6)) blobFrame++;
      if (blobFrame > 3)blobFrame = 0;
      if (arduboy.everyXFrames(3)) thumbsUpFrame++;
      if (thumbsUpFrame > 7)
      {
        thumbsUpFrame = 0;
        showCombo = false;
      }
      drawThumbsUpElf();
      break;
    case ELF_STRESSED:
      if (arduboy.everyXFrames(3)) blobFrame++;
      elfStressedFrame++;
      if (blobFrame > 3)blobFrame = 0;
      if (elfStressedFrame > 2)elfStressedFrame = 0;
      drawDitherBackground();
      drawStressedElf();
      break;
    case ELF_PAUSED:
      drawDitherBackground();
      drawPausedElf();
      break;
  }
  arduboy.drawRect(0, 0, 51, 64, WHITE);
  drawNextBlobs();

  switch (elfState)
  {
    case ELF_NORMAL:
    case ELF_STRESSED:
    case ELF_THUMBSUP:
      drawField();
      drawCurrentBlobs();
      drawNextAndWaitingBlobs();
      break;
    case ELF_PAUSED:
      arduboy.fillRect(56, 4, 10, 22, WHITE);
      arduboy.fillRect(70, 10, 5, 11, WHITE);
      arduboy.drawSprite(56, 13, pause_bitmap, 19, 5, 0, BLACK);
      break;
  }
  
  arduboy.fillRect(53, 52, 47, 9, WHITE);
  scoreDraw(57, 54);
}

void drawNextBlobs()
{
  arduboy.drawSprite(54, 2, nextBlobsMask_bitmap, 23, 27, 0, WHITE);
  arduboy.drawSprite(54, 2, nextBlobs_bitmap, 23, 27, 0, BLACK);
}

void drawNextAndWaitingBlobs()
{
  drawBlobs((PLAYFIELD_WIDTH * BLOB_PIXELS) + 8, 4, BLOB_NEXT);
  drawBlobs((PLAYFIELD_WIDTH * BLOB_PIXELS) + 22, 10, BLOB_WAITING);
}

void drawCurrentBlobs()
{
  drawBlobs((blobsXY[0]*BLOB_PIXELS) + PLAYFIELD_ZERO_X, (blobsXY[1]*BLOB_PIXELS) + PLAYFIELD_ZERO_Y, BLOB_CURRENT);
}

void scoreDraw(int scoreX, int scoreY)
{
  arduboy.drawSprite(scoreX - 3, scoreY - 2, scoreBackground_bitmap, 45, 9, 0, BLACK);
  char buf[10];
  ltoa(scorePlayer, buf, 10);
  char charLen = strlen(buf);
  char pad = 10 - charLen;

  //draw 0 padding
  for (byte i = 0; i < pad; i++)
  {
    arduboy.drawSprite(scoreX + (4 * i), scoreY, numbers_bitmaps, 3, 8, 0, WHITE);
  }

  for (byte i = 0; i < charLen; i++)
  {
    char digit = buf[i];
    byte j;
    if (digit <= 48)
    {
      digit = 0;
    }
    else {
      digit -= 48;
      if (digit > 9) digit = 0;
    }

    for (byte z = 0; z < 10; z++)
    {
      if (digit == z) j = z;
    }
    arduboy.drawSprite(scoreX + (pad * 4) + (4 * i), scoreY, numbers_bitmaps, 3, 8, digit, WHITE);
  }
}

void drawNormalElf()
{
  arduboy.drawCompressed(51, 0, elfNormal_compressed, WHITE);
  arduboy.drawSprite(79, 24, elfNormalEyes_bitmaps, 31, 16, elfNormalEyesSequence[elfPausedFrame], WHITE);
}

void drawStressedElf()
{
  arduboy.drawCompressed(58, elfStressedBodySequenceY[elfStressedFrame], elfBodyMask_compressed, WHITE);
  arduboy.drawCompressed(58, elfStressedBodySequenceY[elfStressedFrame], elfBody_compressed, BLACK);
  arduboy.drawCompressed(61, - elfStressedHeadSequenceY[elfStressedFrame], elfHeadMask_compressed, WHITE);
  arduboy.drawCompressed(61, - elfStressedHeadSequenceY[elfStressedFrame], elfHead_compressed, BLACK);
  arduboy.drawCompressed(elfStressedWandSequenceX[elfStressedFrame], elfStressedWandSequenceY[elfStressedFrame], elfWandMask_compressed, WHITE);
  arduboy.drawCompressed(elfStressedWandSequenceX[elfStressedFrame], elfStressedWandSequenceY[elfStressedFrame], elfWand_compressed, BLACK);
}

void drawPausedElf()
{
  arduboy.drawCompressed(50,3+ elfPausedBodySequenceY[elfPausedFrame], elfPauseBodyMask_compressed, WHITE);
  arduboy.drawCompressed(50,3+ elfPausedBodySequenceY[elfPausedFrame], elfPauseBody_compressed, BLACK);
  arduboy.drawCompressed(elfPausedHeadSequenceX[elfPausedFrame], -elfPausedHeadSequenceY[elfPausedFrame], elfPauseHeadMask_compressed, WHITE);
  arduboy.drawCompressed(elfPausedHeadSequenceX[elfPausedFrame], -elfPausedHeadSequenceY[elfPausedFrame], elfPauseHead_compressed, BLACK);
  arduboy.drawCompressed(elfPausedWandSequenceX[elfPausedFrame], elfPausedWandSequenceY[elfPausedFrame], elfPauseWandMask_compressed, WHITE);
  arduboy.drawCompressed(elfPausedWandSequenceX[elfPausedFrame], elfPausedWandSequenceY[elfPausedFrame], elfPauseWand_compressed, BLACK);
  arduboy.drawSprite(elfPausedHeadSequenceX[elfPausedFrame] + 31, -elfPausedHeadSequenceY[elfPausedFrame] + 43, elfPauseMouth_bitmap, 8, 8, elfPausedMouthSequence[elfPausedFrame], BLACK);
}

void drawThumbsUpElf()
{
  arduboy.drawCompressed(51, 0, thumbsUpMask_compressed, BLACK);
  arduboy.drawCompressed(51, 0, thumbsUp_compressed[thumbsUpFrame], WHITE);
}

void drawDitherBackground()
{
  for (byte y = 0; y < 8; y++)
  {
    for (byte x = 0; x < 77; x += 2)
    {
      arduboy.drawSprite(51 + x, 8 * y, elfBackground_bitmap, 2, 8, 0, WHITE);
    }
  }
}

