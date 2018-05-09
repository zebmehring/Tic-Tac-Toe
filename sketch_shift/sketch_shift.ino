/*
 * File:    sketch_shift.ino
 *
 * Author 1:  Sanat Khurana (sanat.khurana@yale.edu)
 * Author 2:  Zeb Mehring (zeb.mehring@yale.edu)
 * Date:      Spring 2018
 * Course:    EENG 348
 *
 * Summary of File:
 *
 *   This is the implementation of a 2-player game of tic-tac-toe on Arduino,
 *   using a custom-built circuit specifically for this project. The board
 *   consists of two 9x9 arrays of: LEDs and buttons. When a button is pressed,
 *   the corresponding LED lights up to the color of the player whose turn it
 *   is (green or blue). The colors are then internally toggled and the next
 *   player may take their turn. When the game is over, any button may be pressed
 *   to reset the board. If a player wins, their winning triplet will blink
 *   twice to indicate their victory.
 *
 */

int latchPin = 8; // pin connected to ST_CP of 74HC595
int clockPin = 12; // pin connected to SH_CP of 74HC595
int dataPin = 11; // pin connected to DS of 74HC595

byte dataROW; // information for the shifting function
byte dataArrayROW[27]; // information for the shifting function
int board[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; // 3x3 matrix representing the game board
bool s_open[9] = {true, true, true, true, true, true, true, true, true}; // booleans to indicate when a button is pressed
int turn = 1; // keeps track of whose turn it is
bool gameOver = false; // whether or not the game is over
bool tie = false; // whether or not the game is tied

void setup()
{
  pinMode(latchPin, OUTPUT);
  // set button pins to input
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  pinMode(4, INPUT);
  digitalWrite(4, HIGH);
  pinMode(5, INPUT);
  digitalWrite(5, HIGH);
  pinMode(6, INPUT);
  digitalWrite(6, HIGH);
  pinMode(7, INPUT);
  digitalWrite(7, HIGH);
  pinMode(9, INPUT);
  digitalWrite(9, HIGH);
  pinMode(10, INPUT);
  digitalWrite(10, HIGH);
  pinMode(13, INPUT);
  digitalWrite(13, HIGH);

  Serial.begin(9600);

  // set information for the shift function
  dataArrayROW[0] = 0b00000000;
  dataArrayROW[1] = 0b00000001;
  dataArrayROW[2] = 0b00000010;
  dataArrayROW[3] = 0b00000100;
  dataArrayROW[4] = 0b00000101;
  dataArrayROW[5] = 0b00000110;
  dataArrayROW[6] = 0b00001000;
  dataArrayROW[7] = 0b00001001;
  dataArrayROW[8] = 0b00001010;
  dataArrayROW[9] = 0b00010000;
  dataArrayROW[10] = 0b00010001;
  dataArrayROW[11] = 0b00010010;
  dataArrayROW[12] = 0b00010100;
  dataArrayROW[13] = 0b00010101;
  dataArrayROW[14] = 0b00010110;
  dataArrayROW[15] = 0b00011000;
  dataArrayROW[16] = 0b00011001;
  dataArrayROW[17] = 0b00011010;
  dataArrayROW[18] = 0b00100000;
  dataArrayROW[19] = 0b00100001;
  dataArrayROW[20] = 0b00100010;
  dataArrayROW[21] = 0b00100100;
  dataArrayROW[22] = 0b00100101;
  dataArrayROW[23] = 0b00100110;
  dataArrayROW[24] = 0b00101000;
  dataArrayROW[25] = 0b00101001;
  dataArrayROW[26] = 0b00101010;

  // blink all the LEDs to signal the start of the game
  blinkAll_2Bytes(2, 500);
}

void loop()
{
  // wait for input via polling
  while (s_open[0] == ((PIND >> 2) & 1) &&
         s_open[1] == ((PIND >> 3) & 1) &&
         s_open[2] == ((PIND >> 4) & 1) &&
         s_open[3] == ((PIND >> 5) & 1) &&
         s_open[4] == ((PIND >> 6) & 1) &&
         s_open[5] == ((PIND >> 7) & 1) &&
         s_open[6] == ((PINB >> 1) & 1) &&
         s_open[7] == ((PINB >> 2) & 1) &&
         s_open[8] == ((PINB >> 5) & 1));

  // toggle the pin status of the pressed button
  for (int i = 0; i < 6; i++)
  {
    if (s_open[i] != ((PIND >> (i + 2)) & 1))
      s_open[i] = !s_open[i];
  }
  if (s_open[6] != ((PINB >> 1) & 1))
    s_open[6] = !s_open[6];
  if (s_open[7] != ((PINB >> 2) & 1))
    s_open[7] = !s_open[7];
  if (s_open[8] != ((PINB >> 5) & 1))
    s_open[8] = !s_open[8];

  // find the pressed button
  for (int i = 0; i < 9; i++)
  {
    if (s_open[i] == false)
    {
      Serial.print("The switch is: closed"); Serial.println(i);
      if (!gameOver)
      {
        buttonPressed(i); // update game state
      }
      else // restart game
      {
        for (int i = 0; i < 9; i++) board[i / 3][i % 3] = 0; // turn all LEDs off
        turn = 1; // player 1's turn at start
        gameOver = false; // reset gameOver status
        tie = false; // reset tie status
      }
      break;
    }
  }
  drawBoard(board); // update the LEDs to reflect the move
  delay(500); // debounce the signal by waiting
}

/*
 * void buttonPressed(int i)
 *
 * Summary:
 *
 *    Updates the game state to reflect the move made by the player specified by
 *    the global variable 'turn'.
 *
 * Parameters   - i: index of the pressed button
 *
 * Return Value - none.
 */
void buttonPressed(int i)
{
  int x = board[i / 3][i % 3]; // get the status of the corresponding LED
  bool turnSwitch = true; // whether or not to switch to next player for next call

  if (x == 0) // if LED is off, turn on LED
    board[i / 3][i % 3] = turn;
  else // otherwise button was pressed in error -- don't do anything
    turnSwitch = false;

  drawBoard(board); // update the LEDs
  checkForGameOver(); // assert game can continue

  if (!gameOver && turnSwitch) // switch turns if game not over and move is valid
  {
    turn = (turn % 2) + 1;
  }
  else if (tie) // game is over and result is a tie
  {
    Serial.println("Winner is: TIE");
  }
  else if (gameOver) // game is over and a winner emerges
  {
    Serial.print("Winner is: "); Serial.println(turn);
  }
}

/*
 * void checkForGameOver()
 *
 * Summary:
 *
 *    Checks the board to see if a winning move has been made, or if no more
 *    moves are possible. If a winner exists, their winning combination blinks
 *    and the global game status 'gameOver' is set to indicate that it's time
 *    to clean up. If the result is a tie, nothing is blinked, but the game
 *    is still over. If the global 'gameOver' status is set to true, then the
 *    next button press will reset the board.
 *
 * Parameters   - none
 *
 * Return Value - none.
 */
void checkForGameOver()
{
  // check all possible winning sequences
  for (int i = 0; i < 3; i++)
  {
    if (checkTripletForEquality(3 * i, 3 * i + 1, 3 * i + 2)) // horizontal sequences
    {
      blinkTriplet(3 * i, 3 * i + 1, 3 * i + 2);
      gameOver = true;
      return;
    }
    else if (checkTripletForEquality(i, i + 3, i + 6)) // vertical sequences
    {
      blinkTriplet(i, i + 3, i + 6);
      gameOver = true;
      return;
    }
  }
  if (checkTripletForEquality(0, 4, 8)) // first diagonal
  {
    blinkTriplet(0, 4, 8);
    gameOver = true;
    return;
  }
  else if (checkTripletForEquality(2, 4, 6)) // second diagonal
  {
    blinkTriplet(2, 4, 6);
    gameOver = true;
    return;
  }

  // check for a tie by finding unplayed spaces
  tie = true;
  gameOver = true;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if (board[i][j] == 0)
      {
        tie = false;
        gameOver = false;
      }
}

/*
 * void checkTripletForEquality(int x, int y, int z)
 *
 * Summary:
 *
 *    Checks the state of the three LEDs specified by the parameters to see if
 *    they're all the same color (i.e. if 'board' holds the same value for all)
 *    positions.
 *
 * Parameters   - x: index of first space
 *                y: index of second space
 *                z: index of third space
 *
 * Return Value - True if the three indices are the same color, false otherwise
 */
bool checkTripletForEquality(int x, int y, int z)
{
  if (board[x / 3][x % 3] == board[y / 3][y % 3] &&
      board[x / 3][x % 3] == board[z / 3][z % 3] &&
      board[y / 3][y % 3] == board[z / 3][z % 3] &&
      board[y / 3][y % 3] != 0)
    return true;
  return false;
}

/*
 * void blinkTriplet(int x, int y, int z)
 *
 * Summary:
 *
 *    Blinks the three LEDs at the indices specified by the arguments. Used
 *    to indicate the winning sequence when the game is over.
 *
 * Parameters   - x: index of first LED
 *                y: index of second LED
 *                z: index of third LED
 *
 * Return Value - none
 */
void blinkTriplet(int x, int y, int z)
{
  for (int i = 0; i < 5; i++) // blink 5 times
  {
    board[x / 3][x % 3] = 0;
    board[y / 3][y % 3] = 0;
    board[z / 3][z % 3] = 0;
    drawBoard(board);
    delay(300);
    board[x / 3][x % 3] = turn;
    board[y / 3][y % 3] = turn;
    board[z / 3][z % 3] = turn;
    drawBoard(board);
    delay(300);
  }
}

/*
 * void drawBoard(int b[3][3])
 *
 * Summary:
 *
 *    Updates the LEDs to reflect the state of the game, as represented by
 *    the global variable 'board'.
 *
 * Parameters   - b: 3x3 array representing the state of the game board
 *
 * Return Value - none
 */
void drawBoard(int b[3][3])
{
  // set the color of the LED corresponding to player 1 or player 2
  int rowIndices[3] = {0, 0, 0};
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      rowIndices[i] += ((int) pow(3.0, 2.0 - j) * b[i][j]); // sets the appropriate color
    }
  }

  // send data to the shift registers
  digitalWrite(latchPin, 0);
  for (int i = 2; i >= 0; i--)
  {
    shiftOut(dataPin, clockPin, dataArrayROW[rowIndices[i]]); // broadcast board to LEDs
  }
  digitalWrite(latchPin, 1);
}

/*
 * void shiftOut(int myDataPin, int myClockPin, byte myDataOut)
 *
 * Summary:
 *
 *    Shifts out a byte of data to chained shift registers.
 *
 * Parameters   - myDataPin: data pin
 *                myClockPin: clock pin
 *                myDataOut: data to send
 *
 * Return Value - none.
 *
 * Source: https://www.arduino.cc/en/Tutorial/ShftOut13
 */
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first,
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i = 0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights.
  for (i = 7; i >= 0; i--)
  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1 << i) )
    {
      pinState = 0;
    }
    else
    {
      pinState = 1;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}

/*
 * void blinkAll_2Bytes(int n, int d)
 *
 * Summary:
 *
 *    Toggles power on/off to all the output pins of a shift register n
 *    times, with a delay of d between each blink.
 *
 * Parameters   - n: number of blinks to perform
 *                d: delay between successive blinks
 *
 * Return Value - none.
 *
 * Source: https://www.arduino.cc/en/Tutorial/ShftOut13
 */
void blinkAll_2Bytes(int n, int d) {
  // toggle power off initially to signal start
  digitalWrite(latchPin, 0);
  shiftOut(dataPin, clockPin, 0);
  shiftOut(dataPin, clockPin, 0);
  digitalWrite(latchPin, 1);
  delay(200);
  // blink on and off n times
  for (int x = 0; x < n; x++)
  {
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, 255);
    shiftOut(dataPin, clockPin, 255);
    shiftOut(dataPin, clockPin, 255);
    digitalWrite(latchPin, 1);
    delay(d);
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);
    digitalWrite(latchPin, 1);
    delay(d);
  }
}
