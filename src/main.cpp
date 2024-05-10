// #include
// "C:\Users\benn2\OneDrive\arduino_16\TerrariumControl\.pio\libdeps\ESP32\TFT_eSPI\examples\320
// x 240\TFT_Pie_Chart\TFT_Pie_Chart.ino"

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
// note - a backup of User_setup.h is named
// Setup_FW_WROOM32_ILI9341_TFT_018_OK.h and saved in the User_Setups folder
// note: colors are inverted
//
// in User_Setup uncomment in section 2:

#include <ESP32Time.h>
ESP32Time rtc(0);  // offset in param, but dont use it
int HoldHour[24]; //the hour to hold when setting up a time
int HoldMin[60]; //same for the minute
char const* HoldDayOW[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" }; //hold the days of the week in string
char const* HoldMonth[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

/*/////////////Usefull RTC get shit /////////////////
//  Serial.println(rtc.getTime());          //  (String) 15:24:38
//  Serial.println(rtc.getDate());          //  (String) Sun, Jan 17 2021
//  Serial.println(rtc.getDate(true));      //  (String) Sunday, January 17 2021
//  Serial.println(rtc.getDateTime());      //  (String) Sun, Jan 17 2021
15:24:38
//  Serial.println(rtc.getDateTime(true));  //  (String) Sunday, January 17 2021
15:24:38
//  Serial.println(rtc.getTimeDate());      //  (String) 15:24:38 Sun, Jan 17
2021
//  Serial.println(rtc.getTimeDate(true));  //  (String) 15:24:38 Sunday,
January 17 2021
//
//  Serial.println(rtc.getMicros());        //  (long)    723546
//  Serial.println(rtc.getMillis());        //  (long)    723
//  Serial.println(rtc.getEpoch());         //  (long)    1609459200
//  Serial.println(rtc.getSecond());        //  (int)     38    (0-59)
//  Serial.println(rtc.getMinute());        //  (int)     24    (0-59)
//  Serial.println(rtc.getHour());          //  (int)     3     (1-12)
//  Serial.println(rtc.getHour(true));      //  (int)     15    (0-23)
//  Serial.println(rtc.getAmPm());          //  (String)  pm
//  Serial.println(rtc.getAmPm(true));      //  (String)  PM
//  Serial.println(rtc.getDay());           //  (int)     17    (1-31)
//  Serial.println(rtc.getDayofWeek());     //  (int)     0     (0-6)
//  Serial.println(rtc.getDayofYear());     //  (int)     16    (0-365)
//  Serial.println(rtc.getMonth());         //  (int)     0     (0-11)
//  Serial.println(rtc.getYear());          //  (int)     2021*/

#include <Preferences.h>
Preferences preferences;  // initiate the preference for saving shit
#include <dhtnew.h>

DHTNEW mySensor(16);  //  ESP 16    UNO 5    MKR1010 5

float temp, hum;
float dispTemp, dispHum;
unsigned long lastPress = 0;  // counter for the time since last press for dimming
int page = 0;       // the page to display, 0 is main screen
int last_page = 0;  // track the change of page, to redraw everything

#define TFT_BL 32              // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>  // hardware-specific library
#include <TFT_eWidget.h>

#include "Free_Fonts.h"
TFT_eSPI tft = TFT_eSPI();             // invoke custom library
TFT_eSprite knob = TFT_eSprite(&tft);  // Sprite for the slide knob

SliderWidget s1 = SliderWidget(&tft, &knob);  // Slider  widget for hour
SliderWidget s2 = SliderWidget(&tft, &knob);  // Slider  widget for min
SliderWidget sy = SliderWidget(&tft, &knob);  // Slider  widget for year 
SliderWidget sm = SliderWidget(&tft, &knob);  // Slider  widget for month
SliderWidget sd = SliderWidget(&tft, &knob);  // Slider  widget for day

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

#define BUTTON_W 60
#define BUTTON_H 30

#define LowGraphPos 142  // Y coordinate of the lower part of the graph
#define GraphH 130       // height of the graph
#define mettersYpos 200  // Y position of the round metters

#define MAXBL 255    // max backlight
#define MINBL 80     // min backlight
#define max_temp 45  // maximum reachable

#define DEG2RAD 0.0174532925

///////////output states//////////////
bool FanOut = 0;






uint16_t RGB888toRGB565(const char* rgb32_str_) {
  long rgb32 = strtoul(rgb32_str_, 0, 16);
  return (rgb32 >> 8 & 0xf800) | (rgb32 >> 5 & 0x07e0) | (rgb32 >> 3 & 0x001f);
  }

// some principal color definitions
// RGB 565 color picker at
// https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
// in the form ((R << 11) | (G << 5) | B), max vales are R=31, G=63, B=31

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define GREY 0x2108
#define SCALE0 0xC655  // accent color for unused scale segments
#define SCALE1 0x5DEE  // accent color for unused scale segments
#define TEXT_COLOR 0xEF3B  // 0xFFFF        
#define BACKGROUND_COLOR 0x18a3//0x20E4  //((3 << 11) | (8 << 5) | 3)     //sort of dark grey super dark
#define TEMP_COLOR 0xF6F4  //((20 << 11) | (40 << 5) | 20)
#define HYGRO_COLOR 0x7DF8

// char Kawauso = 0xCD5C08;
// char* label[] = {"","Celsius","%","AMP", "VOLT"};            // some custom
// gauge labels

char lab[2][10] = { "temp", "hygro" };

int graphLine[320][4] = {};  // building the array that saves the data for the graph
                             // 0:temp, 1:hygro, 2:fan ON, 3:light ON

void touch_calibrate() {

  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!LittleFS.begin()) {
    Serial.println("formatting file system");
    LittleFS.format();
    LittleFS.begin();
    }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE);
      }
    else {
      File f = LittleFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char*)calData, 14) == 14) calDataOK = 1;
        f.close();
        }
      }
    }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
    }
  else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
      }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char*)calData, 14);
      f.close();
      }
    }
  }
float sineWave(long phase, int upperBound) {
// ################################################################################################################################
// Return a value in range -1 to +1 for a given phase angle in degrees
// ################################################################################################################################
  float val = sin(phase * DEG2RAD);
  val += 1;
  val *= upperBound / 2;

  return int(val);
  }

/////////////// GRAPH FUNCTIONS ////////////////

void metter(int x, int y, int lowBound, int highBound, float value,
  unsigned int colour, unsigned int emptyColour, float text,
  int labindex) { //draw the 2 metters
  lowBound *= 100;
  highBound *= 100;
  value *= 100;
  value = constrain(value, lowBound,
    highBound);  // clamp the values to the set bound
  int endAngle = map(value, lowBound, highBound, 50, 360 - 50);  // convert the values to angles

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
  tft.setTextPadding(50);
  tft.drawFloat(text, 0, x, y, 4);
  tft.drawString(lab[labindex], x, y - 23, 2);
  tft.drawArc(x, y, 45 /*outer rad*/, 45 - 3 /*inner rad*/, 50 /*start angle*/,
    endAngle, colour, BACKGROUND_COLOR, true);
  tft.drawArc(x, y, 45, 45 - 3, endAngle, 360 - 50, emptyColour, BACKGROUND_COLOR, true);
  }

void drawSplash(int p) {  // draw the splash coresponding to the page
  // tft.setTextSize(1);
  if (p == 0) {  //////////////////////////////// Splash on main page
    // tft.setTextSize(1);
    //  tft.drawRoundRect(0, 240-95, 320 - 1, 94, 6, TFT_SILVER);
    //  tft.fillRect(5, 120, 314, 20, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(HYGRO_COLOR);
    tft.drawString("100%", 307, LowGraphPos - GraphH - 1, 1);
    tft.setTextColor(TEMP_COLOR);
    tft.drawString("50C", 12, LowGraphPos - GraphH - 1, 1);
    tft.setTextColor(TEXT_COLOR);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("24h", 1, LowGraphPos + 4, 1);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("now", tft.width() - 2, LowGraphPos + 4, 1);
    tft.drawFastHLine(0, LowGraphPos + 1, 319, TFT_WHITE);
    }

  if (p == 1) { //////////////////////////////////// Splash on option screen
    //  Create slider using parameters and plot at UL
    // s1.drawSlider(3, 28);
    // s2.drawSlider(3, 49);
    // sy.drawSlider(3, 28+21*2);
    // sm.drawSlider(3, 28+21*3);
    // sd.drawSlider(3, 28+21*4);
    int16_t x, y;                         // x and y can be negative
    uint16_t w, h;                        // Width and height
    s1.getBoundingRect(&x, &y, &w, &h);   // Update x,y,w,h with bounding box
    tft.fillRect(x, y, w, h, TFT_BLACK);  // Draw rectangle outline
    s2.getBoundingRect(&x, &y, &w, &h);   // Update x,y,w,h with bounding box
    tft.fillRect(x, y, w, h, TFT_BLACK);  // Draw rectangle outline
    sy.getBoundingRect(&x, &y, &w, &h);   // Update x,y,w,h with bounding box
    tft.fillRect(x, y, w, h, TFT_BLACK);  // Draw rectangle outline
    sm.getBoundingRect(&x, &y, &w, &h);   // Update x,y,w,h with bounding box
    tft.fillRect(x, y, w, h, TFT_BLACK);  // Draw rectangle outline
    sd.getBoundingRect(&x, &y, &w, &h);   // Update x,y,w,h with bounding box
    tft.fillRect(x, y, w, h, TFT_BLACK);  // Draw rectangle outline
    // tft.fillSmoothRoundRect(tft.width() / 2 - 42, LowGraphPos + 1, 84, 32, 3,
    // TFT_WHITE, BACKGROUND_COLOR);
    //OK button bien phat
    tft.setTextDatum(MC_DATUM);
    tft.fillSmoothRoundRect(50, 200, 50, 40, 3, TEXT_COLOR, BACKGROUND_COLOR);
    tft.setTextColor(TFT_BLACK, TEXT_COLOR);
    tft.drawString("OK", 50 + 50 / 2, 200 + 40 / 2 + 2, 4);
    s1.getSliderPosition();
    s2.getSliderPosition();
    sy.getSliderPosition();
    sm.getSliderPosition();
    sd.getSliderPosition();

    s1.setSliderPosition(rtc.getHour());
    s2.setSliderPosition(rtc.getMinute());
    sy.setSliderPosition(rtc.getYear());
    sm.setSliderPosition(rtc.getMonth()+1);
    sd.setSliderPosition(rtc.getDay());

    tft.setTextColor(TEXT_COLOR, TFT_BLACK);
    tft.setTextDatum(CL_DATUM);
    tft.drawString("hour:", 0, 29 + 22 / 2, 1);
    tft.drawString("min:", 0, 29 +28 + 22 / 2, 1);
    tft.drawString("year:", 0, 29 +28*2 + 22 / 2, 1);
    tft.drawString("month:", 0, 29 +28*3 + 22 / 2, 1);
    tft.drawString("day:", 0, 29 +28*4 + 22 / 2, 1);
    }
  }

void drawGraph() {
  //Serial.println("enrtering the drawgraph funct");
  for (int a = 0; a < 319; a++) {
    tft.drawLine(319 - a, LowGraphPos, 319 - a, LowGraphPos - GraphH,
      TFT_BLACK);  // clear the graph line by line
    if (a % 20 == 0)
      tft.drawLine(319 - a, LowGraphPos, 319 - a, LowGraphPos - GraphH,
      ((1 << 11) | (2 << 5) | 1));  // add vert lines each hour
    if (a % 40 == 0)
      tft.drawLine(319 - a, LowGraphPos, 319 - a, LowGraphPos - GraphH,
      BACKGROUND_COLOR);  // add vert line every 3 hours

    if (graphLine[a][0] != 0)
      tft.drawLine(319 - a, LowGraphPos, 319 - a,
      LowGraphPos - map(graphLine[a][0], 0, max_temp, 0, GraphH),
      0x5aa7);  // draw the fill first (for temp)
    //  tft.drawPixel(314 - a, 120 - graphLine[a][1], HYGRO_COLOR);
    }
  for (int a = 0; a < 319; a++) {  // draw the lines for temp and hygro
    if (graphLine[a][0] != 0)
      tft.drawLine(
      319 - a, LowGraphPos - map(graphLine[a][0], 0, max_temp, 0, GraphH),
      319 - (a + 1),
      LowGraphPos - map(graphLine[a + 1][0], 0, max_temp, 0, GraphH),
      TEMP_COLOR);
    if (graphLine[a][1] != 0)
      tft.drawLine(319 - a,
      LowGraphPos - map(graphLine[a][1], 0, 100, 0, GraphH),
      319 - (a + 1),
      LowGraphPos - map(graphLine[a + 1][1], 0, 100, 0, GraphH),
      HYGRO_COLOR);
    }
  tft.fillSmoothRoundRect(tft.width() / 2 - 42, mettersYpos - 16 - 5, 84, 32, 3,
    TFT_WHITE, BACKGROUND_COLOR);
  tft.setTextColor(TFT_BLACK);
  // tft.setFreeFont(TT1);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(rtc.getTime("%a, %b %d"), tft.width() / 2, mettersYpos - 1 - 5,
    2);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(rtc.getTime("%H:%M"), tft.width() / 2, mettersYpos - 5, 2);
  }

void drawTags(int x) {  // draw the values labels on the graph
  tft.setTextPadding(25);
  int indexOfData =
    map(x, 319, 0, 0, 319);  // select the corresponding bar in the array of data
  float tempTag = graphLine[indexOfData][0];
  float hygTag = graphLine[indexOfData][1];
  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_BLACK, TEMP_COLOR);
  tft.drawFloat(tempTag, 1, tft.width() / 2, LowGraphPos + 1, 2 /*font*/);
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TFT_BLACK, HYGRO_COLOR, true);
  tft.drawFloat(hygTag, 1, tft.width() / 2, LowGraphPos - GraphH, 2 /*font*/);
  tft.setTextDatum(MC_DATUM);
  }

void toggleSW(int x, int y, bool state, const char* label) {
  static bool oldState = state;
  static int augment = 2; //size to add to the contener to be bigger that the dot
  static int rad = 9; //size of the round in px
  static int interSpace = rad;
  static int wide = 4 * rad + interSpace + augment * 2; //wideness L to R(max)
  static int disp;
  static int R = 31;
  static int G = 10;
  static int B = 8;

  if (oldState != state) {
    //state changed, nee to draw
    Serial.println("refreshed state");
    }

  if (disp != state * 10 + 1) {//do a cool animation
    disp < state * 10 + 1 ? disp++ : disp--;
    tft.setTextPadding(1);
    tft.setTextDatum(CR_DATUM);
    tft.setTextColor(TFT_BLACK);
    tft.fillSmoothRoundRect(x - wide / 2 - tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1) - 7, y - rad - 2 + 4, wide + 1 + tft.drawString(label, 10, 10, 1), rad * 2 + augment * 2 + 1 - 8, rad + augment, TFT_SILVER, BACKGROUND_COLOR);
    tft.drawString(label, x - rad*2 -interSpace/2-5, y + 1, 1);
    //Serial.println(disp);
    tft.setTextColor(TEXT_COLOR);
    //tft.setFreeFont(TT1);
    tft.setTextSize(1);
    tft.fillSmoothRoundRect(x - wide / 2, y - rad - 2, wide + 1, rad * 2 + augment * 2 + 1, rad + augment, ((map(disp, 1, 11, 8, 28) << 11) | G | (map(disp, 11, 1, 8, 31) << 5)), BACKGROUND_COLOR);
    tft.setTextDatum(CR_DATUM);
    if (disp > 8) tft.drawString("OFF", x+2, y + 1, 1);
    tft.setTextDatum(CL_DATUM);
    if (disp < 3) tft.drawString("ON", x+3, y+1, 1);
    tft.fillSmoothCircle(x - rad - interSpace / 2 + map(disp, 1, 11, 0, 2 * rad + interSpace), y, rad, TEXT_COLOR, ((map(disp, 1, 11, 8, 28) << 11) | G | (map(disp, 11, 1, 8, 31) << 5)));
    tft.setFreeFont(GLCD);
    tft.setTextSize(1);
    }
  oldState = state;
  }



long debugval = 0;  // shit to increase to simulate changing values
void setup() {
  Serial.begin(115200);
  rtc.setTime(00, 10, 22, 6, 5, 2024);  // 17th Jan 2021 15:24:30

  Serial.println("starting TFT display");
  // preferences.begin("Settings", false);
  //  save shit with preferences.putUInt("NameOfTheThing", val);
  //  Get it back with unsigned int val = preferences.getUInt("NameOfTheThing",
  //  0); 0 is the def val, when nothing is saved yet

  tft.init();
  tft.setRotation(3);  // display in portrait
  tft.fillScreen(BACKGROUND_COLOR);

  // mySensor.setHumOffset(10);
  mySensor.setTempOffset(-6.0);

  tft.setTextSize(1);

  analogWrite(TFT_BL, 255);

  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();

  // Slider slot parameters
  s1.createSlider(4, 319 - 30, BACKGROUND_COLOR, BACKGROUND_COLOR, H_SLIDER);
  s1.createKnob(9, 22, 4, TEXT_COLOR, TFT_DARKGREY);
  s1.setSliderScale(0, 23, 2000);

  s2.createSlider(4, 319 - 30, BACKGROUND_COLOR, BACKGROUND_COLOR, H_SLIDER);
  s2.createKnob(9, 22, 4, TEXT_COLOR, TFT_DARKGREY);
  s2.setSliderScale(0, 59, 2000);

  sy.createSlider(4, 319 - 30, BACKGROUND_COLOR, BACKGROUND_COLOR, H_SLIDER);
  sy.createKnob(9, 22, 4, TEXT_COLOR, TFT_DARKGREY);
  sy.setSliderScale(2024, 2050, 2000);

  sm.createSlider(4, 319 - 30, BACKGROUND_COLOR, BACKGROUND_COLOR, H_SLIDER);
  sm.createKnob(9, 22, 4, TEXT_COLOR, TFT_DARKGREY);
  sm.setSliderScale(1, 12, 2000);

  sd.createSlider(4, 319-30, BACKGROUND_COLOR, BACKGROUND_COLOR, H_SLIDER);
  sd.createKnob(9, 22, 4, TEXT_COLOR, TFT_DARKGREY);
  sd.setSliderScale(1, 31, 2000);

  s1.drawSlider(30, 28);
  s2.drawSlider(30, 28+28);
  sy.drawSlider(30, 28 + 28 * 2);
  sm.drawSlider(30, 28 + 28 * 3);
  sd.drawSlider(30, 28 + 28 * 4);
  
  drawSplash(page);  // draw the static shit of the current
  }

void loop() {
  if (last_page != page) {  // the page changed, refresh the screen
    Serial.println("switch page");
    dispHum = 0;                       // just for the style
    dispTemp = 0;                      // juste for the style
    tft.fillScreen(BACKGROUND_COLOR);  // errase all
    drawSplash(page);  // draw the splash of the coresponding page
    }

   debugval++;
   int test = sineWave(debugval, 2);
   FanOut = test;
   //Serial.println(test);

   static uint32_t lastTime = 0;  // holds its value after every iteration of loop
  if (millis() - lastTime >= 2000 || lastTime == 0){  // read sensor every 2000 milliseconds
    Serial.println((String)"page: " + page);
    lastTime = millis();
    // read DHT data every 2 sec here
    mySensor.read();
    hum = mySensor.getHumidity();
    temp = mySensor.getTemperature();
    hum = constrain(hum, 0, 100);         // clamp values
    temp = constrain(temp, 0, max_temp);  // clamp values

    for (int a = 0; a < 319; a++) {  // record the new data as array
      if (a == 0) {
        graphLine[a][0] = temp;
        graphLine[a][1] = hum;
        }
      // slide the whole arrays
      graphLine[319 - a][0] = graphLine[319 - (a + 1)][0];
      graphLine[319 - a][1] = graphLine[319 - (a + 1)][1];
      }
    if (page == 0) {  // if we are on main page
      }
    }
  // fluidify the move of the metters
  dispTemp > temp * 100
    ? dispTemp = dispTemp - abs((temp * 100) - dispTemp) / 70
    : dispTemp = dispTemp + abs((temp * 100) - dispTemp) / 70;
  dispHum > hum * 100 ? dispHum = dispHum - abs((hum * 100) - dispHum) / 70
    : dispHum = dispHum + abs((hum * 100) - dispHum) / 70;

  // Serial.println(dispHum);
  static uint32_t lastTimeForRef = 0;  // holds its value after every iteration of loop
  if (page == 0) {  // refresh every loop on main screen
    if (millis() - lastTimeForRef >= 1000 || lastTimeForRef == 0 || last_page !=page) {  // low refresh rate
      lastTimeForRef = millis();
      //Serial.println("refresh the graph");
      //////////draw the graph////////////
      drawGraph();
      }
    //////////draw the 2 metters////////////
    metter(60 /*x coord*/, mettersYpos /*y coord*/, 0 /*min val*/, 50 /*max*/,
      dispTemp / 100, TEMP_COLOR, TFT_BLACK, temp, 0);
    metter(320 - 60, mettersYpos, 0, 100, dispHum / 100, HYGRO_COLOR,
      TFT_BLACK, hum, 1);

    //////////draw toggle buttons ////////////
    toggleSW(tft.width()/2, LowGraphPos+15,FanOut, "FAN");

    
    } //end shit to draw on main page

  if (page == 1) {  //things to refresh every loop on Options screen
    if (millis() - lastTimeForRef >= 200 || lastTimeForRef == 0 || last_page != page) {  // low refresh rate of this page
      lastTimeForRef = millis();
      //probably nothing to do at low refresh rate here....
      tft.setTextPadding(tft.width());
      rtc.setTime(00, s2.getSliderPosition(), s1.getSliderPosition(), sd.getSliderPosition(), sm.getSliderPosition(), sy.getSliderPosition());
      }
    // option page, this will refresh every loop
    //  tft.setTextDatum(MC_DATUM);
    //  tft.drawString("SETTINGS PAGE", tft.width() / 2, mettersYpos - 25, 2);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    //tft.drawNumber(s1.getSliderPosition(), 80, 1, 4);
    s1.getSliderPosition();
    s2.getSliderPosition();
    sy.getSliderPosition();
    sm.getSliderPosition();
    sd.getSliderPosition();
    tft.drawString(rtc.getDateTime(), tft.width()/2, 15, 4);
    }

  // delay(25);
  last_page = page;  // updating the last page, for comparison

  static uint32_t scanTime = millis();
  uint16_t t_x = 9999, t_y = 9999;  // To store the touch coordinates
  // Scan keys every 50ms at most
  if (millis() - scanTime >=
    50) {  // Pressed will be set true if there is a valid touch on the screen
    bool pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    if (pressed) {  // to check and do when pressed
      lastPress = millis();
      if (page == 0) {            // to do when pressed on main page
        if (t_y < LowGraphPos) {  // touch on the graph
          // drawGraph();
          drawTags(t_x);
          }
        else {  // press outside of the graph
          while (
            tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything
            }
          if (page++ > 0) page = 0;  // skip to option screen
          }
        }
      if (page == 1) {  // to do on pressed on page options

        if (tft.getTouch(&t_x, &t_y, 250)) {
          if (s1.checkTouch(t_x, t_y)) {
            s1.getSliderPosition();
            }
          if (s2.checkTouch(t_x, t_y)) {
            s2.getSliderPosition();
            }
          if (sy.checkTouch(t_x, t_y)) {
            sy.getSliderPosition();
            }
          if (sm.checkTouch(t_x, t_y)) {
            sm.getSliderPosition();
            }
          if (sd.checkTouch(t_x, t_y)) {
            sd.getSliderPosition();
            }
          }
        
        if (t_x > 50 && t_x < 100 && t_y > 200 &&
          t_y < 200 + 40) {  // check if the return button is pressed
          while (
            tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything
            }
          if (page++ > 0) page = 0;  // skip to option screen
          }
        }
      }
    else  // when no touch detected
      {
      }
    }

  /////////////Updating the LCD brightness/////////////////
  int BLintens = 500 - (millis() - lastPress) / 10;
  if (BLintens > MAXBL) BLintens = MAXBL;
  if (BLintens < MINBL) BLintens = MINBL;
  analogWrite(TFT_BL, BLintens);
  }