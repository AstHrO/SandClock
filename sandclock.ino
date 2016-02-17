// Sandclock for Arduino
// V1.6 -  february 2016
// Blenderlab (http://www.blenderlab.fr / http://www.asthrolab.fr)
// Fab4U (http://www.fab4U.de)
//
// Hardware needed :
// Arduino (Uno,Mini,Micro...)
// 2 Led Matrix with MAX7219 controller
// DS103 RTC for realtime &keeping time in memory.
// Wires, powersupply ...;-D
//
//
// Connect the first matrix to the arduino (5 pins, as follow)
// Connect (chain) the second matrix to the first one : All same pins except (DOUT->DIN)
// Taht's it !
//
// Matrix pinout
// CS = 10
// CL = 13
// DI = 11
//
// SW350D : 12 & GND (Vibration sensor) - (We use internal pullup)


#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include "font_diagonal.h";
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

#define HAUT 0
#define BAS 1
#define ON 1
#define OFF 0
#define SHAKE 12 // Vibration sensor pin

#define MODE 2
// 1 = normal - random length loop
// 2 = 1 minute per loop.

int wait = 20; // In milliseconds
int ln = 5; // Filling level of the globe
int pinCS = 10;
// One second between each marble drop :
int htimeout = 1000;
  
Max72xxPanel matrix = Max72xxPanel(pinCS, 1, 2);
RTC_Millis rtc;

// Timer for benchmarking
unsigned long startms = millis();

// Variable for the timing :
unsigned long timer = 0;

//  Timeout for a next-grain drop
int next_grain = 0;

//Array of pixels (2 matrices of 8x8)
int pixels[8][16];

// Emptying led array :
void cleanup_pixels() {
  for (int i = 0; i < 8 ; i++) {
    for (int j = 0; j < 16; j++) {
      pixels[i][j] = 0;
    }
  }
}

// Filling the Upper glass with some sand :
void fill_hourglass(int niv = 5, int globe = HAUT) {
  cleanup_pixels();
  byte minu = 0;
  niv = 0;

#if MODE==1

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if ((x + y) > niv) {
        set_pixel(x, y, globe, ON);
        draw_hourglass();
        minu++;
        delay(10);
      }
    }
  }

# else
  while (minu <= 48) {
    int x = random(8);
    int y = random(8);
    if (get_pixel(x, y, HAUT) == OFF) {
      set_pixel(x, y, HAUT, ON);
      minu++;
      draw_hourglass();
      delay(10);
    }
  }



# endif
  Serial.println("====");
  Serial.println((millis() - timer) / 1000);
  timer = millis();
}

// cleanupp the lower glass slowly  :
void empty_hourglass( int globe = BAS) {
  for (int i = 0; i <= 256; i++) {
    set_pixel(random(8), random(8), random(2), OFF);
    animer(random(8), random(8) , 1);
    draw_hourglass();
    delay(10);
  }

}

// Routine to print the led array on the matrix
void draw_hourglass() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 16; j++) {
      matrix.drawPixel(i, j, pixels[i][j]);
    }
  }
  matrix.write();
}

// Routine to move 1 pixel from X1/Y1 to X2/Y2 in the same globe :
int move_grain(int x1, int y1, int x2, int y2, int globe) {
  if (get_pixel(x2, y2, globe) == OFF) {   // there's no pixel here !
    set_pixel(x1, y1,   globe, OFF);
    set_pixel(x2, y2, globe, ON);
    delay(10); // Wait a bit, it's so cute !
    return (1);
  }
  return (0);
}

// Routine to get the led's state :
int get_pixel(int x, int y, int globe) {
  if ((x > 7) || (y > 7)) {
    return (255); // Outcote scope... sorry !
  }

  if (globe == BAS) { // Adding ofset for the lower matrix :
    y += 8;
  }
  return ( pixels[x][y] );
}

// Setting led to selected state :
// X,Y,HAUT/BAS,ON/OFF
void set_pixel(int x, int y, int globe, const int allume) {
  if ((x > 7) || (y > 7)) {
    return;
  }
  if ((x < 0) || (y < 0)) {
    return;
  }
  if (globe == BAS) {
    y += 8; // Offset for the lower matrix
  }
  pixels[x][y] = allume;
}

// How pixels move (moving the pixel at X,Y from the HAUT/BAS globe)
int animer(int x, int y, int globe) {
  static int  cote = 0; // From the left ? To the right ...
  static int  gs = 0; // Number of grains that were NOT moved (fails)

  // trying not to move pixels away from the matrix :
  x = constrain(x, 0, 7);
  y = constrain(y, 0, 7);

  // If pixel is OFF : nothing to do & add a fail to counter :
  if (get_pixel(x, y, globe) == OFF) {
    gs++;
    return (gs);
  }

  // If we managed to move the pixel by 1 pixel on X & Y( vertical move ) : reset fail counter :
  if ( move_grain(x, y, x + 1, y + 1, globe) ) {
    gs = 0;
    return (gs);
  }

  // If we didn't managed last move, trying to move on the side :
  if (cote) {
    cote ^= 1;    // change cote & trying to move :
    if ( move_grain(x, y, x + 1, y, globe) ) {
      gs = 0;   // reset failing counter
      return (gs);
    } // Other side :
    if ( move_grain(x, y, x, y + 1, globe) ) {
      gs = 0;   // reset failing counter
      return (gs);
    }
  }
  else {
    cote ^= 1;    // change cote
    if ( move_grain(x, y, x, y + 1, globe) ) {
      gs = 0;   // reset failing counter
      return (gs);
    }
    if ( move_grain(x, y, x + 1, y, globe) ) {
      gs = 0;   // reset failing counter
      return (gs);
    }
  }
  // If we try to move a pixel from the lower globe : failing again.
  if (globe == BAS) {
    gs++;
  }
  else {
    gs = 0;
  }
  // Setting the pixel to the newer position :
  set_pixel(x, y, globe, ON);

  // Drawing the result :
  draw_hourglass();

  return (gs);
}

// Removing the last pixel of the upper globe, to the first of the lower :
void drop() {
  if (get_pixel(7, 7, HAUT) == OFF) {
    return;
  }
  if (get_pixel(0, 0, BAS) == OFF) {
    set_pixel(7, 7, HAUT, OFF);
    set_pixel(0, 0, BAS, ON);
  }
  draw_hourglass();
}

// Function to draw integers on the screen
void drawInt(int16_t x, int16_t y, int c,  byte globe) {
  for (int8_t i = 0 ; i < 8; i++ ) {
    uint8_t line;
    line = pgm_read_byte(font + (i + 8 * c));
    for (int8_t j = 0; j < 8; j++, line >>= 1 ) {
      if (line & 0x1) {

        set_pixel(x + j, y + 8 - i, globe, ON);
      }
    }
  }
}

// Function tha display current time
void displaytime() {
  // Get current ime
  DateTime now = rtc.now();
  int ho = now.hour();
  int mi = now.minute();
  // Moving from out the screen to the center :
  for (int k = 8; k >= 0; k--) {
    cleanup_pixels();
    drawInt(k, -k, ho / 10, HAUT);
    drawInt(k, -k, ho - (ho / 10) * 10, BAS);
    draw_hourglass();
    delay(100);
  }
  delay(1000);
  
  // Moving the hours out & let the minutes come in :
  for (int k = 0; k >= -8; k--) {
    cleanup_pixels();
    drawInt(k, -k, ho / 10, HAUT);
    drawInt(k, -k, ho - (ho / 10) * 10, BAS);
    drawInt(k + 8,  -k - 8, mi / 10, HAUT);
    drawInt(k + 8,  -k - 8, mi - (mi / 10) * 10, BAS);
    draw_hourglass();
    delay(100);
  }
  delay(1000);
  
  // Push the minutes out :
  for (int k = 0; k >= -8; k--) {
    cleanup_pixels();
    drawInt(k, -k, mi / 10, HAUT);
    drawInt(k, -k, mi - (mi / 10) * 10, BAS);
    draw_hourglass();
    delay(100);
  }

}

// Setu p :What elese ?
void setup() {
  randomSeed(analogRead(A0)); // Initialize random generator
  pinMode(12, INPUT); // SW350D Pinout
  digitalWrite(12, HIGH); // Turnon Internal Pullup
  matrix.setIntensity(0); // Set brightness between 0 and 15
  matrix.setPosition(1, 0, 0); // The first display is at <0, 0>
  matrix.setPosition(0, 1, 0); // The second display is at <1, 0>
  matrix.setRotation(0, 3); // In case you glued the screens upside down
  matrix.setRotation(1, 1);
  matrix.fillScreen(LOW); // Turn of the screen
  Wire.begin(); // Init SPI coms
  rtc.begin(DateTime(F(__DATE__), F(__TIME__))); // Set date/time to software RTC
  displaytime(); // To be sure ...
  fill_hourglass(ln, HAUT); // First refill
  startms = millis(); // init counter
  }

void reset_hourglass(){
    empty_hourglass();
    ln = random(8) + 1;
    displaytime();
    fill_hourglass(ln);
    startms = now;
    drop();
  }

// Main loop :
void loop() {
  // TIme since poweron :
  long nowms = millis();
  // timeout reached ?  Dropping 1 marble :
  if ((nowms - startms) >= htimeout) {
    startms = nowms;
    drop();
  }
  
  if (digitalRead(12) == HIGH) { // If vibration recorded : reset !
    reset_hourglass();
  }
  
  // If we cannot manage to move 512 pixels, considering that the globe is empty.
  if (animer(random(8), random(8) , random(2) ) > 512) {
    reset_hourglass();
  }
}
