// This sketch includes a function to draw circle segments
// for pie charts in 1 degree increments

#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

#define DEG2RAD 0.0174532925

byte inc = 0;
unsigned int col = 0;


// #########################################################################
// Draw circle segments
// #########################################################################

// x,y == coords of centre of circle
// start_angle = 0 - 359
// sub_angle   = 0 - 360 = subtended angle
// r = radius
// colour = 16-bit colour value

void fillSegment(int x, int y, int start_angle, int sub_angle, int r, unsigned int colour)
{
  // Calculate first pair of coordinates for segment start
  float sx = cos((start_angle - 90) * DEG2RAD);
  float sy = sin((start_angle - 90) * DEG2RAD);
  uint16_t x1 = sx * r + x;
  uint16_t y1 = sy * r + y;

  uint16_t subx1 = sx * (r-5) + x;
  uint16_t suby1 = sy * (r-5) + y;


  // Draw colour blocks every inc degrees
  for (int i = start_angle; i < start_angle + sub_angle; i++) {

    // Calculate pair of coordinates for segment end
    int x2 = cos((i + 1 - 90) * DEG2RAD) * r + x;
    int y2 = sin((i + 1 - 90) * DEG2RAD) * r + y;

    int subx2 = cos((i + 1 - 90) * DEG2RAD) * (r-5) + x;
    int suby2 = sin((i + 1 - 90) * DEG2RAD) * (r-5) + y;

    //tft.fillTriangle(x1, y1, x2, y2, x, y, colour);
   // tft.fillTriangle(subx1, suby1, subx2, suby2, x, y, TFT_BLACK);
    //tft.drawArc(x, y, r, r - 3, i, i + 1, colour, TFT_BLACK);
  

    // Copy segment end to segment start for next segment
    x1 = x2;
    y1 = y2;

    subx1 = subx2;
    suby1 = suby2;
  }
//  tft.fillSmoothCircle(x, y, r - 5, TFT_LIGHTGREY, TFT_BLACK);
}

void metter(int x,int y, int lowBound, int highBound, int value, unsigned int colour,unsigned int comcolour){

  value = constrain(value, lowBound,highBound);
  int endAngle = map(value, lowBound, highBound, 50, 360-50);

  tft.drawArc(x, y, 45, 45 - 3, 50, endAngle, colour, TFT_BLACK, true);
  tft.drawCentreString("T", x, y - 10, 4);
  tft.drawArc(x, y, 45, 45 - 3, endAngle, 360-50, comcolour, TFT_BLACK, true);

}
// #########################################################################
// Return the 16-bit colour with brightness 0-100%
// #########################################################################
/*
unsigned int brightness(unsigned int colour, int brightness)
{
  byte red   = colour >> 11;
  byte green = (colour & 0x7E0) >> 5;
  byte blue  = colour & 0x1F;

  blue =  (blue * brightness)/100;
  green = (green * brightness)/100;
  red =   (red * brightness)/100;

  return (red << 11) + (green << 5) + blue;
}
*/

long factor = 0;

void setup(void)
{
  Serial.begin(115200);
  tft.begin();

  tft.setRotation(3);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);  // Text colour
}

void loop() {

  // Draw 4 pie chart segments
  //fillSegment(60, 240-60, 50, 270, 50, TFT_LIGHTGREY);
  //fillSegment(160, 120, 0, 30, 100, TFT_GREEN);
  //fillSegment(160, 120, 60 + 30, 120, 100, TFT_BLUE);
  //fillSegment(160, 120, 60 + 30 + 120, 150, 100, TFT_YELLOW);

  factor++;

  float end = sin((factor/2)* 0.0174532925);
  end += 1;
  end *= 130;

  Serial.println(int(end));

  metter(60,240-40,0, 350, int(end), 0xF0AF,0x7BEF );
  // tft.drawArc(60, 240 - 60, 51, 50 - 4, 50, 270, TFT_BLACK, TFT_BLACK, false);
  // tft.fillCircle(60, 240-60, 51, TFT_BLACK);

metter(320-60,240-40,-50, 250, 250-int(end), (65535/3)*2, 0x7800);

  delay(10);

  // Erase old chart with 360 degree black plot
  //fillSegment(160, 120, 0, 360, 100, TFT_BLACK);
}
