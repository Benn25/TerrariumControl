//#include "C:\Users\benn2\OneDrive\arduino_16\TerrariumControl\.pio\libdeps\ESP32\TFT_eSPI\examples\320 x 240\TFT_Pie_Chart\TFT_Pie_Chart.ino"

// ESP32_WROOM_TFT_eSPI_ILI9341_rainbow_scale
// note: rainbow scale original sketch by Bodmer 
//
// March 1, 2021
// Floris Wouterlood 
// public domain
//
// 240*320 TFT 7-pins SPI interface ILI 9341 controller
// microcontroller ESP32 WROOM32 DEV board
// NOTE the display is a 3.3V device. 
// pin wiring to WROOM32 as follows
// GND ----- GND
// VCC ----- 3V3
// SCL ----- D18 yellow wire
// SDA ----- D23 green wire
// RES ----- D4
// DC ------ D2
// CS ------ D15
// BLK ----- D32
//
// uses TFT _eSPI library with modified setup.h file (in the library folder)
// note - a backup of User_setup.h is named Setup_FW_WROOM32_ILI9341_TFT_018_OK.h
// and saved in the User_Setups folder
// note: colors are inverted
//
// in User_Setup uncomment in section 2:


#include <dhtnew.h>

DHTNEW mySensor(16);   //  ESP 16    UNO 5    MKR1010 5

float temp, hum;
float dispTemp, dispHum;

#define TFT_BL 32             // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH // Level to turn ON back-light (HIGH or LOW)


//   #define TFT_RGB_ORDER TFT_BGR                      // color order Blue-Green-Red - for this specific display  
   #include <TFT_eSPI.h>                              // hardware-specific library
   #include <SPI.h>
   TFT_eSPI tft = TFT_eSPI();                         // invoke custom library

#define DEG2RAD 0.0174532925

uint16_t RGB888toRGB565(const char *rgb32_str_)
   {
     long rgb32 = strtoul(rgb32_str_, 0, 16);
     return (rgb32 >> 8 & 0xf800) | (rgb32 >> 5 & 0x07e0) | (rgb32 >> 3 & 0x001f);
}

// some principal color definitions
// RGB 565 color picker at https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
// in the form ((R << 11) | (G << 5) | B), max vales are R=31, G=63, B=31

   #define WHITE       0xFFFF
   #define BLACK       0x0000
   #define BLUE        0x001F
   #define RED         0xF800
   #define GREEN       0x07E0
   #define CYAN        0x07FF
   #define MAGENTA     0xF81F
   #define YELLOW      0xFFE0
   #define GREY        0x2108 
   #define SCALE0      0xC655                                                  // accent color for unused scale segments                                   
   #define SCALE1      0x5DEE                                                  // accent color for unused scale segments
   #define TEXT_COLOR  0xEF3B//0xFFFF                                                  // is currently white
   #define BACKGROUND_COLOR 0x20E4 //((3 << 11) | (8 << 5) | 3)     //sort of dark grey super dark
   #define TEMP_COLOR  0xF6F4//((20 << 11) | (40 << 5) | 20)
   #define HYGRO_COLOR 0x7DF8

   //char Kawauso = 0xCD5C08;
   //char* label[] = {"","Celsius","%","AMP", "VOLT"};            // some custom gauge labels


   int gaugeposition_x = 20;                                                   // these two variables govern the position
   int gaugeposition_y = 20;                                                   // of the square + gauge on the display

   char lab[2][10] = {"temp", "hygro"};

   int graphLine[310][2] = {};

   

// ################################################################################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// ################################################################################################################################

float sineWave(long phase, int upperBound) {
  float val = sin(phase * DEG2RAD);
  val += 1;
  val *= upperBound/2;
  return int(val);
}

/////////////// GRAPH FUNCTIONS ////////////////
void metter(int x,int y, int lowBound, int highBound, float value, unsigned int colour,unsigned int emptyColour, float text, int labindex){
  lowBound *= 100;
  highBound *= 100;
  value *= 100;

  value = constrain(value, lowBound, highBound);              // clamp the values to the set bounds
  int endAngle = map(value, lowBound, highBound, 50, 360-50); //convert the values to angles

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
  tft.setTextPadding(50);
  tft.drawFloat(text,0, x, y, 4);
  tft.drawString(lab[labindex], x, y - 23, 2);
  tft.drawArc(x, y, 45/*outer rad*/, 45 - 3/*inner rad*/, 50/*start angle*/, endAngle, colour, BACKGROUND_COLOR, true);
  tft.drawArc(x, y, 45, 45 - 3, endAngle, 360-50, emptyColour, BACKGROUND_COLOR, true);
}

void graph(int x, int y, int pos /*where to draw*/, int val1, int val2){ 

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
long debugval = 0; // shit to increase to simulate changing values 
void setup() {
                                                      
   Serial.begin (115200);
   Serial.println ("starting TFT display");
                                                              
   tft.init();  
   tft.setRotation (3);                                                        // display in portrait
  // Kawauso = convert32To16(Kawauso);
   
   tft.fillScreen (BACKGROUND_COLOR); 
   //tft.drawRect (gaugeposition_x, gaugeposition_y, 200,200, YELLOW);
//Serial.println(BACKGROUND_COLOR, HEX);

 

  //mySensor.setHumOffset(10);
  mySensor.setTempOffset(-6.0);

 tft.drawRoundRect(0, 240-95, 320 - 1, 94, 6, TFT_SILVER);
 //tft.fillRect(5, 120, 314, 20, TFT_BLACK);
 tft.setTextDatum(MC_DATUM);
tft.setTextColor(HYGRO_COLOR);
tft.drawString("100%",307,12,1);
tft.setTextColor(TEMP_COLOR);
tft.drawString("50C",12,12,1);
tft.setTextColor(TEXT_COLOR);
tft.drawString("24h",26,127,1);
tft.drawString("now",308,127,1);


}

void loop() {

static uint32_t lastTime = 0;     // holds its value after every iteration of loop
  if (millis() - lastTime >= 2000)  // print every 2000 milliseconds
  {
    lastTime = millis();
    //read DHT data every 2 sec here
    mySensor.read();
  hum = mySensor.getHumidity();
  temp = mySensor.getTemperature();
  Serial.println(temp, 1);
  Serial.print("\t");
  Serial.print(hum, 1);

//tft.fillScreen (BACKGROUND_COLOR); 

for(int a=0 ; a<309 ; a++){ //record the new data
  if(a==0){
    graphLine[a][0] = temp;
    graphLine[a][1] = hum;
  }
  //slide the whole arrays
  graphLine[309-a][0] = graphLine[309-(a+1)][0];
  graphLine[309-a][1] = graphLine[309-(a+1)][1];

}

//////////draw the graph////////////
for(int a = 0 ; a < 309 ; a++){
  tft.drawLine(314 - a, 120,314-a,120 -100, TFT_BLACK);//clear the Vline
  if(a % 12 == 0) tft.drawLine(314 - a, 120,314-a,120 -100, ((1 << 11) | (2 << 5) | 1));//add vert lines each hour
  if(a % 36 == 0) tft.drawLine(314 - a, 120,314-a,120 -100, BACKGROUND_COLOR);//add vert line every 3 hours

 if(graphLine[a][0] != 0)  tft.drawLine(314 - a, 120, 314-a, 120-2*graphLine[a][0], 0x5aa7);
//  tft.drawPixel(314 - a, 120 - graphLine[a][1], HYGRO_COLOR);
}
for(int a = 0 ; a < 309 ; a++){
 if(graphLine[a][0] != 0)   tft.drawLine(314 - a, 120 - 2*graphLine[a][0], 314 - (a + 1), 120 - 2*graphLine[a + 1][0], TEMP_COLOR);
 if(graphLine[a][1] != 0)   tft.drawLine(314 - a, 120 - graphLine[a][1], 314 - (a + 1), 120 - graphLine[a + 1][1], HYGRO_COLOR);
}
  }

  debugval++;
  int test = sineWave(debugval, 255);

  analogWrite(TFT_BL, test);

  dispTemp > temp * 100 ? dispTemp=dispTemp-abs((temp * 100)-dispTemp)/70 : dispTemp = dispTemp+abs((temp * 100)-dispTemp)/70;
  dispHum > hum * 100 ? dispHum = dispHum - abs((hum*100)-dispHum)/70 : dispHum = dispHum+abs((hum * 100)-dispHum)/70;

  Serial.println(dispHum);

  metter(60/*x coord*/,240-40/*y coord*/,0/*min val*/, 50/*max*/, dispTemp/100, TEMP_COLOR,((3 << 11) | (5 << 5) | 3), temp,0);
  metter(320-60,240-40,0, 100, dispHum/100, HYGRO_COLOR, ((3 << 11) | (5 << 5) | 3), hum,1);

  //delay(25);



}