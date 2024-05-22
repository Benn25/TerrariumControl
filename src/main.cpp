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

//#include <Preferences.h>
//Preferences preferences;  // initiate the preference for saving shit
#include <dhtnew.h>

DHTNEW mySensor(16);  //  ESP 16    UNO 5    MKR1010 5

float temp, hum;
float dispTemp, dispHum;
unsigned long lastPress = 0;  // counter for the time since last press for dimming
int page = 0;       // the page to display, 0 is main screen
int last_page = 0;  // track the change of page, to redraw everything
bool nuristate = 1; //state of the buttons for setting the times

bool pressed; //track the pressed state of the screen
bool oldPressed; //old state of the screen

//////touch screen smoother ///////////
// Constants
const int numSamples = 20; // Number of samples to average

int parseMonth(const char* monthStr) {
  if (strcmp(monthStr, "Jan") == 0) return 1;
  if (strcmp(monthStr, "Feb") == 0) return 2;
  if (strcmp(monthStr, "Mar") == 0) return 3;
  if (strcmp(monthStr, "Apr") == 0) return 4;
  if (strcmp(monthStr, "May") == 0) return 5;
  if (strcmp(monthStr, "Jun") == 0) return 6;
  if (strcmp(monthStr, "Jul") == 0) return 7;
  if (strcmp(monthStr, "Aug") == 0) return 8;
  if (strcmp(monthStr, "Sep") == 0) return 9;
  if (strcmp(monthStr, "Oct") == 0) return 10;
  if (strcmp(monthStr, "Nov") == 0) return 11;
  if (strcmp(monthStr, "Dec") == 0) return 12;
  return 0;
  }

// Variables
int touchX[numSamples];
int touchY[numSamples];
int currentIndex = 0;
bool touchDetected[numSamples];
int avgX = 100;
int avgY;
int timer = 60; //the timer for the timespan
int SunriseMin;
int SunsetMin;
int IOdurations[5]={}; //all the ON durations, dynamically change depending on the settings of IOstates[]

//timestamps :
unsigned long fanTS;
unsigned long mistTS;
unsigned long lightTS;
unsigned long pumpTS;
unsigned long seclightTS;
unsigned long IOtimeStamps[5] = {};

#define TFT_BL 32              // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>  // hardware-specific library
#include <TFT_eWidget.h>
#include "Free_Fonts.h"
#include <settings.h>

TFT_eSPI tft = TFT_eSPI();             // invoke custom library
TFT_eSprite knob = TFT_eSprite(&tft);  // Sprite for the slide knob
uint16_t t_x = 9999, t_y = 9999;  // To store the touch coordinates

SliderWidget s1 = SliderWidget(&tft, &knob);  // Slider  widget for hour
SliderWidget s2 = SliderWidget(&tft, &knob);  // Slider  widget for min
SliderWidget sy = SliderWidget(&tft, &knob);  // Slider  widget for year 
SliderWidget sm = SliderWidget(&tft, &knob);  // Slider  widget for month
SliderWidget sd = SliderWidget(&tft, &knob);  // Slider  widget for day

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

///////////output states//////////////
bool FanOut = 0; //air fan ON/OFF
bool mistOut = 0; //mist ON/OFF
bool lightOut = 0; //main light ON/OFF
bool pumpOut = 0; //water pump ON/OFF
bool secLightOut = 0; //secondary light analog write 256 values
bool PinOutStates[5] = {};

//track the changing state (useless?)
bool oldFanOut = 0; //air fan ON/OFF
bool oldmistOut = 0; //mist ON/OFF
bool oldlightOut = 0; //main light ON/OFF
bool oldpumpOut = 0; //water pump ON/OFF
bool oldsecLightOut = 0; //secondary light analog write 256 values
bool OldPinOutStates[5] = {};

uint16_t RGB888toRGB565(const char* rgb32_str_) {
  long rgb32 = strtoul(rgb32_str_, 0, 16);
  return (rgb32 >> 8 & 0xf800) | (rgb32 >> 5 & 0x07e0) | (rgb32 >> 3 & 0x001f);
  }


int IOstates[288][5] = {}; //settings data, when to light up, when to start the pump etc...
//order, from top in the timeline : light, pump, mist, fan, secLight

int graphLine[320][6] = {};  // building the array that saves the data for the graph
// 0:temp, 1:hygro, 2:fan, 3:light, 4: mist, 5: secondary light

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


class classtoggleSW {
public:
  int x;
  int y;
  bool state;
  const char* label;
  int disp;

  void drawTW() { //draw the toggleSW
    static bool oldState = state;
    static int augment = 2; //size to add to the contener to be bigger that the dot
    static int rad = 7; //size of the round in px
    static int interSpace = rad / 2 + 1;
    static int wide = 4 * rad + interSpace + augment * 2; //wideness L to R(max)

    static int minR = 20;
    static int minG = 40;
    static int minB = 28;

    static int R = 28;
    static int G = 25;
    static int B = 10;

    if (page != last_page) {
      //state changed, nee to draw
     // Serial.println("redraw the button");
      }
    if (oldState != state) {
      //state changed, nee to draw
     // Serial.println("refreshed state");
      }
    if (disp != state * 10 + 1 || page != last_page) {//do a cool animation
      disp < state * 10 + 1 ? disp++ : disp--;
      tft.setTextPadding(1);
      tft.setTextDatum(CR_DATUM);
      tft.setTextColor(INV_TEXT_COLOR);
      //txt label
      tft.fillSmoothRoundRect(x - wide / 2 - tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1) - 7,
        y - rad - 2 + 4, wide + 1 + tft.drawString(label, 10, 10, 1), rad * 2 + augment * 2 + 1 - 8, rad + augment, TFT_MIDGREY, BACKGROUND_COLOR);
      tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1);
      //Serial.println(disp);
      tft.setTextColor(TEXT_COLOR);
      //tft.setFreeFont(TT1);
      tft.setTextSize(1);
      tft.fillSmoothRoundRect(x - wide / 2, y - rad - 2, wide + 1, rad * 2 + augment * 2 + 1, rad + augment,
        ((map(disp, 11, 1, minR, R) << 11) |
        (map(disp, 11, 1, minG, G) << 5) |
        map(disp, 11, 1, minB, B)),
        BACKGROUND_COLOR);

      tft.setTextDatum(CR_DATUM);
      if (disp > 8) tft.drawString("ON", x - 2, y + 1, 1);
      tft.setTextDatum(CL_DATUM);
      if (disp < 3) tft.drawString("OFF", x -1, y + 1, 1);
      tft.fillSmoothCircle(x - rad - interSpace / 2 + map(disp, 1, 11, 0, 2 * rad + interSpace), y, rad, TFT_LIGHTGREY,
        ((map(disp, 11, 1, 8, 28) << 11) | (G << 5) | map(disp, 1, 11, 8, B) << 5));
      tft.setFreeFont(GLCD);
      tft.setTextSize(1);
      }
    oldState = state;
    }
  };

classtoggleSW FanSW, lightSW, pumpSW, mistSW, seclightSW;

void getSunriseSunset(int val/*Day of year, from 0 to 364*/) {
  //int eventMin = 500 * sin(((354 - 80) / 365) * 3.14158 * 2) + 500; //range from 0 to 1000, need remap
  float eventMin = (val - 80);
  eventMin /= 365;
  eventMin *= PI * 2;
  eventMin = sin(eventMin);
  eventMin += 1;
  eventMin /= 2;
  SunsetMin = round(eventMin * (SunsetSummer - SunsetWinter) + SunsetWinter);

  eventMin = (val - 80);
  eventMin /= 365;
  eventMin *= PI * 2;
  eventMin += PI / 2;
  eventMin = cos(eventMin);
  eventMin += 1;
  eventMin /= 2;
  SunriseMin = round(eventMin * (SunriseWinter - SunriseSummer) + SunriseSummer);

  //  int eventMin = 500 * sin(val) + 500; //range from 0 to 1000, need remap
  //Serial.println(eventMin);
   //SunriseMin = map(eventMin, 0, 1000, SunriseSummer, SunriseWinter);
   //SunsetMin = map(eventMin, 0, 1000, SunsetWinter, SunsetSummer);
  }

String getTimeString(int minutesOfDay) {
  int hours = minutesOfDay / 60;
  int minutes = minutesOfDay % 60;
  // Format the hours and minutes as a string
  char timeStr[6];  // HH:MM format requires 5 characters + 1 for the null terminator
  sprintf(timeStr, "%02d:%02d", hours, minutes);
  return String(timeStr);
  }

String getHourMin() {
  int hours = rtc.getHour(true);
  int minutes = rtc.getMinute();
  // Format the hours and minutes as a string
  char timeStr[6];  // HH:MM format requires 5 characters + 1 for the null terminator
  sprintf(timeStr, "%02d:%02d", hours, minutes);
  return String(timeStr);
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

int remapExponential(int toTimer/*the value to evaluate*/, int upbound/*max value*/, float upTime) {
  //this function remaps a linear slider to an exp timer from 5 sec to upTime (in sec)
  // Ensure the toTimer is within the range [0, upbound]
  toTimer = constrain(toTimer, 0, upbound);
  // Calculate the remapped value
  float remappedValue = 5 * pow(upTime / 5, toTimer / float(upbound));
  return remappedValue;
  }

void clearTL(int timeLineID, int Val) {
  for (int a = 0; a < 288; a++) {
    IOstates[a][timeLineID] = Val;
    }
  }

int minOfDay() {
  int minOfDay;
  minOfDay = rtc.getHour(true) * 60;
  minOfDay += rtc.getMinute();
  return minOfDay;
  }

void outputStates() {
  //first, check if need to switch ON
  for (int IO = 0; IO < 5; IO++) {
    //first, switch ON if needed:
    if (rtc.getMinute() % 5 == 0 && rtc.getSecond() < 5) { //check for 5 sec every 5 min if we need to activate something
      if (IOstates[minOfDay()/5][IO] != 0) { //if a timestamp is set for this time of the day
        IOtimeStamps[IO] = millis();//reset the timestamps
        IOdurations[IO] = IOstates[minOfDay()/5][IO]; //set the corresponding duration in ms
        PinOutStates[IO] = HIGH; //put this pin ON
      Serial.print("pump duration : ");
      Serial.println(IOstates[minOfDay() / 5][1]);
        }
      }

    //next, check if need to switch OFF
    if (IOtimeStamps[IO] + (IOdurations[IO]*1000) < millis() && PinOutStates[IO] == HIGH) { //if ON and times up
      PinOutStates[IO] = LOW; //put the pin OFF
      Serial.print("order to go off on : ");
      Serial.println(IO);
      }
    
    
    if (PinOutStates[IO] != OldPinOutStates[IO]) { //if there was a change in state, update the toggle switch
      digitalWrite(Out_Pin[IO], PinOutStates[IO]); //switch ON or OFF the needed pins
      Serial.print("a change in state was detected in IO : ");
      Serial.println(IO);
      //update the ToggleS:
      lightSW.state = PinOutStates[0];//lightOut;
      pumpSW.state = PinOutStates[1];//pumpOut;
      Serial.print("set pump to : ");
      Serial.println(PinOutStates[1]);
      mistSW.state = PinOutStates[2];//mistOut;
      FanSW.state = PinOutStates[3];//FanOut;
      seclightSW.state = PinOutStates[4];//secLightOut;
      }
    }
  }

void saveInFS(int IOchan) {
  // Create a file name for the current IOchan
  String fileName = "/IOstates_" + String(IOchan) + ".txt";

  // Open the file for writing
  File file = LittleFS.open(fileName, FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file for writing: " + fileName);
    return;
    }

  // Write each element of the specified column to the file
  for (int i = 0; i < 288; i++) {
    file.print(IOstates[i][IOchan]);
    if (i < 287) {
      file.print(",");  // Add a comma delimiter
      }
    }

  file.println();  // New line after each column

  // Close the file
  file.close();
  Serial.println("Data saved to " + fileName);
  }

void readFromFS(int IOchan) {
  // Create a file name for the current IOchan
  String fileName = "/IOstates_" + String(IOchan) + ".txt";

  // Open the file for reading
  File file = LittleFS.open(fileName, FILE_READ);

  if (!file) {
    Serial.println("Failed to open file for reading: " + fileName);
    return;
    }

  // Read and parse the file contents
  String content = file.readString();
  file.close();

  int index = 0;
  int start = 0;

  for (int i = 0; i < content.length(); i++) {
    if (content[i] == ',' || content[i] == '\n') {
      IOstates[index][IOchan] = content.substring(start, i).toInt();
      start = i + 1;
      index++;
      }
    }

  Serial.println("Data read from " + fileName + ":");
  for (int i = 0; i < 288; i++) {
    Serial.print(IOstates[i][IOchan]);
    Serial.print(" ");
    }
  Serial.println();
  }

void saveDate() {
  // Get current date and time
  String datetime = rtc.getDate() + " " + rtc.getTime("%H:%M:%S");

  // Save date and time to file
  File file = LittleFS.open("/datetime.txt", "w");
  if (file) {
    file.println(datetime);
    file.close();
    //Serial.println("Saved datetime: " + datetime);
    }
  else {
    //Serial.println("Failed to open file for writing");
    }
  }

void recoverDate() {
  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
    }

  // Attempt to read saved date and time from file
  if (LittleFS.exists("/datetime.txt")) {
    File file = LittleFS.open("/datetime.txt", "r");
    if (file) {
      String datetime = file.readStringUntil('\n');
      file.close();
      Serial.println("Read datetime: " + datetime);

      // Parse and set date and time
      int year, month, day, hour, minute, second;
      char monthStr[4];
      if (sscanf(datetime.c_str(), "%*[^,], %3s %d %d %d:%d:%d", monthStr, &day, &year, &hour, &minute, &second) == 6) {
        month = parseMonth(monthStr);
        rtc.setTime(second, minute, hour, day, month, year);
        Serial.println("RTC set to saved datetime");
        }
      else {
        Serial.println("Failed to parse datetime");
        }
      }
    }
  else {
    Serial.println("No saved datetime found, setting default time");
    rtc.setTime(00, 10, 22, 6, 5, 2024); // Default time if no file exists
    }
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
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.setTextPadding(50);
  tft.drawFloat(text, 0, x, y+4, 4);
  tft.drawString(lab[labindex], x, y - 20, 2);
  tft.drawArc(x, y, 45 /*outer rad*/, 45 - 5 /*inner rad*/, 50 /*start angle*/,
    endAngle, colour, BACKGROUND_COLOR, true);
  tft.drawArc(x, y, 45, 45 - 5, endAngle, 360 - 50, emptyColour, BACKGROUND_COLOR, true);
  }

void drawOKandCancel() { //the OK and cancel buttons
  //OK button bien phat
  //tft.setFreeFont(FMB12);
  tft.setFreeFont(FSSB12);
  tft.setTextDatum(MC_DATUM);
  tft.fillSmoothRoundRect(but_X - but_W / 2, but_Y, but_W, but_H, but_H / 2, TFT_MIDGREY, BACKGROUND_COLOR);
  tft.setTextColor(INV_TEXT_COLOR);
  tft.setTextPadding(0);
  tft.drawString("OK", but_X, tft.height() - 6 - but_H / 2, 1);


  tft.fillSmoothRoundRect(tft.width() - but_X - but_W / 2, tft.height() - but_H - 5, but_W, but_H, but_H / 2, TFT_MIDGREY, BACKGROUND_COLOR);
  tft.setTextColor(INV_TEXT_COLOR);
  tft.drawString("Cancel", tft.width() - but_X, tft.height() - 6 - but_H / 2, 1);

  tft.setFreeFont();
  }

void simpleBut(const char* label, int x/*middle X*/, int y/*middle Y*/, int wide, int hei, int state) { //a simple button, frozen H and W with label on it
  int col;

  wide > 50 ? tft.setFreeFont(FMB9) : tft.setFreeFont(); // FSSB9, FMB12
  state == 0 ? col = TFT_MIDGREY : col = TFT_LIGHTGREY;
  tft.fillSmoothRoundRect(x - wide / 2, y - hei / 2, wide, hei, 8, col, BACKGROUND_COLOR);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TEXT_COLOR);
  tft.drawString(label, x, y, 1);
  if (wide >= 50) tft.setFreeFont(); //reset the font if needed
  }

void drawDateBloc() { //draw the date and time of the main page
  tft.setTextDatum(TC_DATUM);
  tft.fillSmoothRoundRect(
    tft.width() / 2 - (tft.drawString(rtc.getTime("%a, %b %d %H:%M"), tft.width() / 2, LowGraphPos + 2 + 1 - 3, 1) + 6) / 2, /*X pos*/
    LowGraphPos + 2 - 1 - 3, /*Y pos*/
    tft.drawString(rtc.getTime("%a, %b %d %H:%M"), tft.width() / 2, LowGraphPos + 2 + 1 - 3, 1) + 6,/*W*/
    10, /*H*/
    3,/*rad*/
    TFT_MIDGREY,
    BACKGROUND_COLOR);
  tft.setTextColor(INV_TEXT_COLOR);
  // tft.setFreeFont(TT1);
  tft.drawString(rtc.getTime("%a, %b %d %H:%M"), tft.width() / 2, LowGraphPos + 2 + 1 - 3, 1);
  //tft.setTextDatum(TC_DATUM);
  //tft.drawString(rtc.getTime("%H:%M"), tft.width() / 2, LowGraphPos+15, 2);
  }

void drawGraph() {
  //Serial.println("enrtering the drawgraph funct");
//erase the graph
  tft.fillRect(0, LowGraphPos - GraphH, tft.width(), GraphH, BLACK);
  for (int a = 0; a < 319; a++) {
    // tft.drawLine(319 - a, LowGraphPos, 319 - a, LowGraphPos - GraphH,
    //  BLACK);  // clear the graph line by line
    if (a % 20 == 0)
      tft.drawLine(319 - a, LowGraphPos, 319 - a, LowGraphPos - GraphH,
      TFT_MIDGREY);  // add vert lines each hour
    if (a % 40 == 0)
      tft.drawLine(319 - a, LowGraphPos, 319 - a, LowGraphPos - GraphH,
      BACKGROUND_COLOR);  // add vert line every 3 hours

    if (graphLine[a][0] != 0)
      tft.drawLine(319 - a, LowGraphPos, 319 - a,
      LowGraphPos - map(graphLine[a][0], 0, max_temp, 0, GraphH),
      IODcolors[0]);  // draw the fill first (for temp)
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
  // draw the max labels
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(HYGRO_COLOR, BLACK);
  tft.drawString("100%", 307, LowGraphPos - GraphH + 3, 1);
  tft.setTextColor(TEMP_COLOR, BLACK);
  //construct the string label for max temperature
  String labeltemp;
  labeltemp = max_temp;
  tft.drawString(labeltemp + "C", 12, LowGraphPos - GraphH + 3, 1);
  //redraw the sunrise sunset info
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawString("sunrise :", 70, 2, 1);
  tft.drawString(getTimeString(SunriseMin), 116, 2, 1);
  tft.drawString("sunset :", 162, 2, 1);
  tft.drawString(getTimeString(SunsetMin), 201, 2, 1);

  drawDateBloc();
  }

void drawTimeStringCursor(int val) {
  tft.fillRect(0, timeLine_Y + 10 + 11, tft.width(), 20, BACKGROUND_COLOR);//errase cursors
  tft.fillTriangle(val, timeLine_Y + 10 + 11, val - 3, timeLine_Y + 10 + 11 + 3, val + 3, timeLine_Y + 10 + 11 + 3, TEXT_COLOR); //draw cursor
  
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawString(getTimeString((val - 16) * 5), val, timeLine_Y + 10 + 11 + 10, 1); //draw the selected hour as string
  }

void drawDuration(int Tim) {
  timer = Tim;
  timer = remapExponential(timer, tft.width(), 3600.0 * Maxdurations[page - 3]);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  String message;
  if (timer < 60) {
    message = timer;
    tft.setTextPadding(tft.drawString(message + "s", 86, nuriBut_Y + 9, 4) + 10);
    }
  else if (timer >= 60 && timer < 3600 * 4) {
    message = (timer / 60);
    tft.setTextPadding(tft.drawString(message + "min", 86, nuriBut_Y + 9, 4) + 10);
    }
  else {
    message = (timer / 3600);
    tft.setTextPadding(tft.drawString(message + "h", 86, nuriBut_Y + 9, 4) + 10);
    }
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

void drawTimeLine() {
  tft.fillRect((tft.width() - 288) / 2, timeLine_Y, 288, 5 * 11, TFT_BLACK); //errase the timeline
  //int barCol;

  for (int hour = 0; hour < 24; hour++) {
    // Calculate x position of the hour label
    int x = map(hour * 60, 0, 1439, 0, 287);
    //draw a small line every hours
    if (hour % 1 == 0 && hour != 0) tft.drawFastVLine(x + (tft.width() - 288) / 2, timeLine_Y, 55, TFT_DEEPGREY);
    }

  for (int a = 0; a < 288; a++) { //draw the timeline of the day, min by min
    for (int chan = 0; chan < 5; chan++) {
      //              IOstates[a][chan] == 1 ? barCol = IOcolors[chan] : barCol = TFT_BLACK;
      if (IOstates[a][chan]) {
        //tft.drawFastVLine(a + (tft.width() - 288) / 2, timeLine_Y + chan * 3, 3, barCol);
        tft.drawRect(a + (tft.width() - 288) / 2, timeLine_Y + chan * 11 + 2, IOstates[a][chan] / 60 / 5, 2, IODcolors[chan]);
        tft.drawRect(a + (tft.width() - 288) / 2, timeLine_Y + chan * 11, 2, 6, IOcolors[chan]);
        }


      /*
      if (IOstates[a][chan] == 2) {
        tft.drawPixel(a + (tft.width() - 288) / 2, timeLine_Y + chan * 11 + 1, IODcolors[chan]);
        }
*/
      }
    }
  }

void drawSpecTimeLine(int page) { //the timelines for each output settings
  tft.fillRect((tft.width() - 288) / 2, timeLine_Y + 10, 289, 10, TFT_BLACK); //errase the timeline

  for (int a = 0; a < 288; a++) { //draw the timeline of the day, min by min
    if (IOstates[a][page]) {
      //first, draw the length of the activation
      tft.fillRect(a + (tft.width() - 288) / 2, timeLine_Y + 14, IOstates[a][page] / 60 / 5, 2, IODcolors[page]);
//then draw the starting line
      tft.drawFastVLine(a + (tft.width() - 288) / 2, timeLine_Y + 10, 10, IOcolors[page]);
      }
    }
  }

void drawSplash(int p) {  // draw the splash coresponding to the page
  // tft.setTextSize(1);
  if (p == 0) {  //////////////////////////////// Splash on main page
    // tft.setTextSize(1);
    //  tft.drawRoundRect(0, 240-95, 320 - 1, 94, 6, TFT_SILVER);
    //draw the 2 options buttons (clock and settings)
    //tft.fillRect(20, 0, 20, 9, TEXT_COLOR);
    tft.fillScreen(BACKGROUND_COLOR);
    getSunriseSunset(rtc.getDayofYear());
    tft.setTextPadding(0);
    tft.setTextColor(INV_TEXT_COLOR, TFT_MIDGREY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("settings", tft.width() - 20, 2, 1);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("clock", 20, 2, 1);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.drawString("sunrise :", 70, 2, 1);
    tft.drawString(getTimeString(SunriseMin), 116, 2, 1);
    tft.drawString("sunset :", 162, 2, 1);
    tft.drawString(getTimeString(SunsetMin), 201, 2, 1);

    tft.setTextColor(TEXT_COLOR);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("24h", 1, LowGraphPos + 4, 1);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("now", tft.width() - 2, LowGraphPos + 4, 1);
    tft.drawFastHLine(0, LowGraphPos + 1, 319, TFT_MIDGREY);
    drawGraph();
    }

  if (p == 1) { //////////////////////////////////// Splash on option screen
    tft.fillScreen(BACKGROUND_COLOR);
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
    drawOKandCancel();

    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.setTextDatum(CL_DATUM);
    tft.drawString("hour:", 1, 29 + 22 / 2, 1);
    tft.drawString("min:", 1, 29 + 28 + 22 / 2, 1);
    tft.drawString("year:", 1, 29 + 28 * 2 + 22 / 2, 1);
    tft.drawString("month:", 1, 29 + 28 * 3 + 22 / 2, 1);
    tft.drawString("day:", 1, 29 + 28 * 4 + 22 / 2, 1);

    s1.setSliderPosition(rtc.getHour() - 1);
    s2.setSliderPosition(rtc.getMinute() - 1);
    sy.setSliderPosition(rtc.getYear() + 1);
    sm.setSliderPosition(rtc.getMonth() + 1 - 1);
    sd.setSliderPosition(rtc.getDay() - 1);

    s1.setSliderPosition(rtc.getHour(true));
    s2.setSliderPosition(rtc.getMinute());
    sy.setSliderPosition(rtc.getYear());
    sm.setSliderPosition(rtc.getMonth() + 1);
    sd.setSliderPosition(rtc.getDay());
    }

  if (page == 2) { //////////////////////////////// Splash settings
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.drawString("---= SETTINGS =---", tft.width() / 2, 18, 4); //page top text
    //OK button bien phat
    drawOKandCancel();

    tft.setTextColor(TEXT_COLOR);
    tft.setTextDatum(TC_DATUM);

    for (int hour = 0; hour < 24; hour++) {
      // Calculate x position of the hour label
      int x = map(hour * 60, 0, 1439, 0, 287);
      // Draw hour label
      tft.setTextColor(TEXT_COLOR);
      tft.setTextSize(1);
      //      tft.setCursor(x + (tft.width() - 288) / 2, timeLine_Y + 35);
      if (hour % 3 == 0 && hour != 0) tft.drawNumber(hour, x + (tft.width() - 288) / 2, timeLine_Y + 10 * 6, 1);
      //draw a small line every hours
      if (hour % 1 == 0 && hour != 0) tft.drawFastVLine(x + (tft.width() - 288) / 2, timeLine_Y + 10 * 6 - 5, 3, TFT_DARKGREY);
      }
    //draw the labels of the outputs
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FF21);
    for (int a = 0; a < 5; a++) {
      tft.fillSmoothRoundRect(((tft.width() / 5) / 2 + (tft.width() / 5) * a) - 60 / 2,
        (tft.height() - but_H - 50) - 32 / 2,
        60, 32, 8, TFT_LIGHTGREY, BACKGROUND_COLOR);
      tft.setTextColor(TEXT_COLOR);
      tft.drawString(outLab[a], (tft.width() / 5) / 2 + (tft.width() / 5) * a, tft.height() - but_H - 50, 1);
      tft.setTextColor(IOcolors[a]);
      tft.drawString(outLab[a], (tft.width() / 5) / 2 + (tft.width() / 5) * a - 1, tft.height() - but_H - 50 - 1, 1);
      }
    tft.setFreeFont();
    // draw a cute frame
    tft.drawSmoothRoundRect(4, timeLine_Y - 10, 6, 6, tft.width() - 8, 10 * 6 + 25, TFT_MIDGREY, BACKGROUND_COLOR);

    //draw the timeline
    drawTimeLine();
    }

  if (page > 2) { //////////////////////////////// Splash for spec settings
    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(outTitleLab[page - 3], tft.width() / 2, 18, 4); //page top text
    //OK button bien phat
    drawOKandCancel();
    drawSpecTimeLine(page - 3);
    //draw the hours labels
    tft.setTextColor(TEXT_COLOR);
    tft.setTextDatum(BC_DATUM);
    for (int hour = 0; hour < 24; hour++) {
      // Calculate x position of the hour label
      int x = map(hour * 60, 0, 1439, 0, 287);
      // Draw hour label
      tft.setTextColor(TEXT_COLOR);
      tft.setTextSize(1);
      //      tft.setCursor(x + (tft.width() - 288) / 2, timeLine_Y + 35);
      if (hour % 3 == 0 && hour != 0) tft.drawNumber(hour, x + (tft.width() - 288) / 2 + 2, timeLine_Y - 5 + 10, 1);
      //draw a small line every hours
      if (hour % 1 == 0 && hour != 0) tft.drawFastVLine(x + (tft.width() - 288) / 2 + 1, timeLine_Y - 3 + 10, 3, TFT_DARKGREY);
      }
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(TFT_DARKGREY);
    tft.drawString("- - S  L  I  D  E    H  E  R  E - -", tft.width() / 2, 182 - 3, 2);
    tft.setTextColor(TEXT_COLOR);
    tft.fillRect(3, 182 - 1, tft.width() - 8, 2, TFT_LIGHTGREY);
    tft.drawFastVLine(25, 182 - 6, 3, TFT_LIGHTGREY);
    tft.drawFastVLine(tft.width() / 2, 182 - 8, 5, TEXT_COLOR);
    tft.drawFastVLine(tft.width() - 26, 182 - 6, 3, TFT_LIGHTGREY);
    tft.setTextDatum(BL_DATUM);
    tft.drawString("5s", 1, 182 - 3, 2);
    tft.setTextDatum(BR_DATUM);
    tft.drawString("max", tft.width() - 1, 182 - 3, 2);

    for (int b = 0; b < 2; b++) {
      if (b == 0) simpleBut("SET", nuriBut_X + b * nuribut_Wid + b * 5, nuriBut_Y, nuribut_Wid, nuribut_Hei, nuristate);
      if (b == 1) {
        simpleBut("Fill!", nuriBut_X + b * nuribut_Wid + b * 5, nuriBut_Y, nuribut_Wid, nuribut_Hei, nuristate);
        }
      }

    drawTimeStringCursor(avgX);
    int defaultDuration = 150; //the default timer for settings, roughly 5min
    drawDuration(defaultDuration); //start with default values

    //draw a few kazari
    tft.drawSmoothRoundRect(9, nuriBut_Y - nuribut_Hei / 2 - 4, 12, 12, tft.width() - 20, nuribut_Hei + 8, TFT_MIDGREY, BACKGROUND_COLOR);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_MIDGREY);
    tft.drawString("ON duration", 18, nuriBut_Y - nuribut_Hei / 2 + 1, 1);
    //tft.drawFastHLine(nuriBut_X - b * nuribut_Wid - b * 5 - nuribut_Wid / 2, nuriBut_Y - nuribut_Hei / 2 + 12, nuribut_Wid, TFT_MIDGREY);

    //draw the mode button (add or keshi)
    simpleBut(butlab[nuristate], tft.width() / 2, nuriBut_Y, nuriBut_X - tft.width() / 2 - 5 * 4, nuribut_Hei, nuristate);

    simpleBut("<", nuriBut_X, nuriBut_Y - 35, nuribut_Wid, nuribut_Hei / 2, 1);
    simpleBut(">", nuriBut_X + nuribut_Wid + 5, nuriBut_Y - 35, nuribut_Wid, nuribut_Hei / 2, 1);

    }
  }
/*
class classtoggleSW {
public:
  int x;
  int y;
  bool state;
  const char* label;
  int disp;

  void drawTW() { //draw the toggleSW
    static bool oldState = state;
    static int augment = 2; //size to add to the contener to be bigger that the dot
    static int rad = 7; //size of the round in px
    static int interSpace = rad / 2 + 1;
    static int wide = 4 * rad + interSpace + augment * 2; //wideness L to R(max)

    static int minR = 20;
    static int minG = 40;
    static int minB = 28;

    static int R = 28;
    static int G = 25;
    static int B = 10;

    if (page != last_page) {
      //state changed, nee to draw
     // Serial.println("redraw the button");
      }
    if (oldState != state) {
      //state changed, nee to draw
     // Serial.println("refreshed state");
      }
    if (disp != state * 10 + 1 || page != last_page) {//do a cool animation
      disp < state * 10 + 1 ? disp++ : disp--;
      tft.setTextPadding(1);
      tft.setTextDatum(CR_DATUM);
      tft.setTextColor(INV_TEXT_COLOR);
      //txt label
      tft.fillSmoothRoundRect(x - wide / 2 - tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1) - 7,
        y - rad - 2 + 4, wide + 1 + tft.drawString(label, 10, 10, 1), rad * 2 + augment * 2 + 1 - 8, rad + augment, TFT_MIDGREY, BACKGROUND_COLOR);
      tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1);
      //Serial.println(disp);
      tft.setTextColor(TEXT_COLOR);
      //tft.setFreeFont(TT1);
      tft.setTextSize(1);
      tft.fillSmoothRoundRect(x - wide / 2, y - rad - 2, wide + 1, rad * 2 + augment * 2 + 1, rad + augment,
        ((map(disp, 1, 11, minR, R) << 11) |
        (map(disp, 1, 11, minG, G) << 5) |
        map(disp, 1, 11, minB, B)),
        BACKGROUND_COLOR);

      tft.setTextDatum(CR_DATUM);
      if (disp > 8) tft.drawString("OFF", x + 2, y + 1, 1);
      tft.setTextDatum(CL_DATUM);
      if (disp < 3) tft.drawString("ON", x + 3, y + 1, 1);
      tft.fillSmoothCircle(x - rad - interSpace / 2 + map(disp, 1, 11, 0, 2 * rad + interSpace), y, rad, TFT_LIGHTGREY,
        ((map(disp, 1, 11, 8, 28) << 11) | (G << 5) | map(disp, 11, 1, 8, B) << 5));
      tft.setFreeFont(GLCD);
      tft.setTextSize(1);
      }
    oldState = state;
    }
  };
*/
void toggleSW(int x, int y, bool state, const char* label) {
  static bool oldState = state;
  static int augment = 2; //size to add to the contener to be bigger that the dot
  static int rad = 9; //size of the round in px
  static int interSpace = rad / 3;
  static int wide = 4 * rad + interSpace + augment * 2; //wideness L to R(max)
  static int disp;
  static int R = 31;
  static int G = 20;
  static int B = 23;

  if (page != last_page) {
    //state changed, nee to draw
    Serial.println("redraw the button");
    }

  if (oldState != state) {
    //state changed, nee to draw
   // Serial.println("refreshed state");
    }

  if (disp != state * 10 + 1) {//do a cool animation
    disp < state * 10 + 1 ? disp++ : disp--;
    tft.setTextPadding(1);
    tft.setTextDatum(CR_DATUM);
    tft.setTextColor(TFT_BLACK);
    tft.fillSmoothRoundRect(x - wide / 2 - tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1) - 7, y - rad - 2 + 4, wide + 1 + tft.drawString(label, 10, 10, 1), rad * 2 + augment * 2 + 1 - 8, rad + augment, TFT_SILVER, BACKGROUND_COLOR);
    tft.drawString(label, x - rad * 2 - interSpace / 2 - 5, y + 1, 1);
    //Serial.println(disp);
    tft.setTextColor(TEXT_COLOR);
    //tft.setFreeFont(TT1);
    tft.setTextSize(1);
    tft.fillSmoothRoundRect(x - wide / 2, y - rad - 2, wide + 1, rad * 2 + augment * 2 + 1, rad + augment, ((map(disp, 1, 11, 8, 28) << 11) | (G << 5) | map(disp, 11, 1, 8, B)), BACKGROUND_COLOR);
    tft.setTextDatum(CR_DATUM);
    if (disp > 8) tft.drawString("OFF", x + 2, y + 1, 1);
    tft.setTextDatum(CL_DATUM);
    if (disp < 3) tft.drawString("ON", x + 3, y + 1, 1);
    tft.fillSmoothCircle(x - rad - interSpace / 2 + map(disp, 1, 11, 0, 2 * rad + interSpace), y, rad, TEXT_COLOR, ((map(disp, 1, 11, 8, 28) << 11) | (G << 5) | map(disp, 11, 1, 8, B) << 5));
    tft.setFreeFont(GLCD);
    tft.setTextSize(1);
    }
  oldState = state;
  }

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(3);  // display in portrait
  tft.fillScreen(BACKGROUND_COLOR);
  tft.setTextSize(1);
  analogWrite(TFT_BL, 255);

  //  preferences.begin("Save", false);
    // "Save" is the namespace name, and `false` indicates read/write mode. 
    // If `true`, it opens in read-only mode.

    //preferences.begin("Save", false);
    //preferences.clear();  // Erase all keys in the namespace
    //preferences.end();

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
    }

  int ToggleSW_X = tft.width() / 2 + 20;

  FanSW.x = ToggleSW_X + 28;
  FanSW.y = LowGraphPos + 24 + 21 * 3;
  FanSW.state = FanOut;
  FanSW.label = "Fan";
  FanSW.disp = 0;

  pumpSW.x = ToggleSW_X - 40;
  pumpSW.y = LowGraphPos + 24 + 21 * 3;
  pumpSW.state = pumpOut;
  pumpSW.label = "Pump";
  pumpSW.disp = 0;

  lightSW.x = ToggleSW_X;
  lightSW.y = LowGraphPos + 24 + 21 * 1;
  lightSW.state = lightOut;
  lightSW.label = "Light";
  lightSW.disp = 0;

  seclightSW.x = ToggleSW_X;
  seclightSW.y = LowGraphPos + 24 + 21 * 2;
  seclightSW.state = secLightOut;
  seclightSW.label = "Sunset";
  seclightSW.disp = 0;

  mistSW.x = ToggleSW_X;
  mistSW.y = LowGraphPos + 24;
  mistSW.state = mistOut;
  mistSW.label = "Mist";
  mistSW.disp = 0;


//  rtc.setTime(00, 10, 22, 6, 5, 2024);  // 17th Jan 2021 15:24:30

  //read the date from FS
  recoverDate();

  Serial.println("starting TFT display");
  // preferences.begin("Settings", false);
  //  save shit with preferences.putUInt("NameOfTheThing", val);
  //  Get it back with unsigned int val = preferences.getUInt("NameOfTheThing",
  //  0); 0 is the def val, when nothing is saved yet

 // Initialize touch arrays
  for (int i = 0; i < numSamples; i++) {
    touchX[i] = 0;
    touchY[i] = 0;
    touchDetected[i] = false;
    }

  // mySensor.setHumOffset(10);
  mySensor.setTempOffset(-6.0);

  // Calibrate the touch screen and retrieve the scaling factors
  touch_calibrate();

  // Slider slot parameters
  s1.createSlider(4, 319 - 30, TFT_DEEPGREY, TFT_BLACK, H_SLIDER);
  s1.createKnob(9, 22, 4, TEXT_COLOR, TFT_MIDGREY);
  s1.setSliderScale(0, 23, 2000);

  s2.createSlider(4, 319 - 30, TFT_DEEPGREY, TFT_BLACK, H_SLIDER);
  s2.createKnob(9, 22, 4, TEXT_COLOR, TFT_MIDGREY);
  s2.setSliderScale(0, 59, 2000);

  sy.createSlider(4, 319 - 30, TFT_DEEPGREY, TFT_BLACK, H_SLIDER);
  sy.createKnob(9, 22, 4, TEXT_COLOR, TFT_MIDGREY);
  sy.setSliderScale(2024, 2050, 2000);

  sm.createSlider(4, 319 - 30, TFT_DEEPGREY, TFT_BLACK, H_SLIDER);
  sm.createKnob(9, 22, 4, TEXT_COLOR, TFT_MIDGREY);
  sm.setSliderScale(1, 12, 2000);

  sd.createSlider(4, 319 - 30, TFT_DEEPGREY, TFT_BLACK, H_SLIDER);
  sd.createKnob(9, 22, 4, TEXT_COLOR, TFT_MIDGREY);
  sd.setSliderScale(1, 31, 2000);

  s1.drawSlider(30, 28);
  s2.drawSlider(30, 28 + 28);
  sy.drawSlider(30, 28 + 28 * 2);
  sm.drawSlider(30, 28 + 28 * 3);
  sd.drawSlider(30, 28 + 28 * 4);


  for (int a = 0; a < 5; a++) {
    readFromFS(a);
    delay(20); //sanity delay
    }

  /////for debug ////

  drawSplash(page);  // draw the static shit of the current
  }

void loop() {
  if (last_page != page) {  // the page changed, refresh the screen
    // Serial.println("switch page");
    dispHum = 0;                       // just for the style
    dispTemp = 0;                      // juste for the style
    tft.fillScreen(BACKGROUND_COLOR);  // errase all
    drawSplash(page);  // draw the splash of the coresponding page
    }

  static uint32_t lastTime = 0;  // holds its value after every iteration of loop
  if (millis() - lastTime >= 20000 || lastTime == 0) {  // read sensor every 2000 milliseconds
    // Serial.println((String)"page: " + page);
    lastTime = millis();
    // read DHT data every 2 sec here
    mySensor.read();
    hum = mySensor.getHumidity();
    temp = mySensor.getTemperature();
    hum = constrain(hum, 0, 100);         // clamp values
    temp = constrain(temp, 0, max_temp);  // clamp values
    saveDate(); //save the time every once in a while
    for (int a = 0; a < 319; a++) {  // record the new data as array
      if (a == 0) {
        graphLine[a][0] = temp;
        graphLine[a][1] = hum;
        }
      // slide the whole arrays
      graphLine[319 - a][0] = graphLine[319 - (a + 1)][0];
      graphLine[319 - a][1] = graphLine[319 - (a + 1)][1];
      }
    if (page == 0) {
      drawGraph();// if we are on main page, refresh the graph
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
    if (millis() - lastTimeForRef >= 5000 || lastTimeForRef == 0 || last_page != page) {  // low refresh rate
      lastTimeForRef = millis();
      //Serial.println("refresh the graph");
      //////////draw the graph////////////
      //drawGraph();
      drawDateBloc();
      }
    //////////draw the 2 metters////////////
    metter(50 /*x coord*/,
      mettersYpos /*y coord*/,
      0 /*min val*/,
      50 /*max*/,
      dispTemp / 100,
      TEMP_COLOR,
      INV_TEXT_COLOR,
      temp,
      0);
    metter(tft.width() - 50,
      mettersYpos,
      0,
      100,
      dispHum / 100,
      HYGRO_COLOR,
      INV_TEXT_COLOR,
      hum,
      1);

    //////////draw toggle buttons ////////////
    //toggleSW(tft.width() / 2+15, LowGraphPos + 15, FanOut, "FAN");
    //toggleSW(tft.width() / 2+15, LowGraphPos + 15+10, lightOut, "LIGHT");
    //toggleSW(tft.width() / 2+15, LowGraphPos + 15+20, secLightOut, "SUNSET");
    pumpSW.drawTW();
    FanSW.drawTW();
    lightSW.drawTW();
    seclightSW.drawTW();
    mistSW.drawTW();


    } //end shit to draw each loop on main page

  if (page == 1) {  //things to refresh every loop on clock adjust screen
    if (millis() - lastTimeForRef >= 200 || lastTimeForRef == 0 || last_page != page) {  // low refresh rate of this page
      lastTimeForRef = millis();
      //probably nothing to do at low refresh rate here....
      tft.setTextPadding(tft.width());
      rtc.setTime(00,
        s2.getSliderPosition(),
        s1.getSliderPosition(),
        sd.getSliderPosition(),
        sm.getSliderPosition(),
        sy.getSliderPosition());
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
    tft.drawString(rtc.getDateTime(), tft.width() / 2, 15, 4);
    }

  if (page == 2) { //things to refresh every loop on settings screen 
    if (millis() - lastTimeForRef >= 1500 || lastTimeForRef == 0 || last_page != page) {  // low refresh rate of this page
      lastTimeForRef = millis();
      //draw a cute clock in the corner
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(TEXT_COLOR,BACKGROUND_COLOR);
      tft.drawString(getHourMin(), 2, 2,1);

      //draw a triangle to show the time on the timeline
      tft.fillRect(10, timeLine_Y-5, tft.width()-20, 5, BACKGROUND_COLOR);//errase cursors
      tft.fillTriangle(minOfDay() + 16, timeLine_Y-1, minOfDay() + 16 - 3, timeLine_Y-1 - 3, minOfDay() + 16 + 3, timeLine_Y-1- 3, TEXT_COLOR); //draw cursor

      }
    //draw the sliders for settings hours and min and get the min of the day
    //s1.getSliderPosition();
    //s2.getSliderPosition();
    }

  if (page > 2) { //things to refresh every loop on corresponding setting page
    if (millis() - lastTimeForRef >= 1500 || lastTimeForRef == 0 || last_page != page) {  // low refresh rate of this page
      lastTimeForRef = millis();
      //draw a cute clock in the corner
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      tft.drawString(getHourMin(), 2, 2, 1);
      }
    }
  // delay(25);

  last_page = page;  // updating the last page, for comparison
  /////////// touch detection routine ///////////////

  static uint32_t scanTime = millis();

  // Scan keys every 50ms at most
  if (millis() - scanTime >= 50) {  // Pressed will be set true if there is a valid touch on the screen
    pressed = tft.getTouch(&t_x, &t_y);
    scanTime = millis();
    if (pressed) {  // to check and do when pressed
      lastPress = millis();

      if (page == 0) {            // to do when pressed on main page
        if (t_y < LowGraphPos && t_y > LowGraphPos - GraphH) {  // touch on the graph
          // drawGraph();
          drawTags(t_x);
          }

        else {  // press outside of the graph
          if (t_y < 12 &&
            t_x < 70 &&
            t_x > 5) {
            page = 1; //go to clock setup page
            }

          if (t_x < tft.width() - 5 &&
            t_x > tft.width() - 70 &&
            t_y < 12) {
            page = 2; //go to main setup page
            }

          if (t_y < lightSW.y + 9 &&
            t_y > lightSW.y - 9 &&
            t_x < lightSW.x + 20 &&
            t_x > lightSW.x - 20) { //fan toggle Switch touched
            if (PinOutStates[0] == 0) { //we were OFF, do what is needed to switch ON
              IOtimeStamps[0] = millis();//set the timestamp
              IOdurations[0] = 3600 *6; //set the duration
              }
            PinOutStates[0] = !PinOutStates[0];
            }

          if (t_y < pumpSW.y + 9 &&
            t_y > pumpSW.y - 9 &&
            t_x < pumpSW.x + 20 &&
            t_x > pumpSW.x - 20) { //fan toggle Switch touched
            Serial.println("pump Switch touched");
            if (PinOutStates[1] == 0) { //we were OFF, do what is needed to switch ON
            IOtimeStamps[1] = millis();//set the timestamp
            IOdurations[1] = 3600 / 4; //set the duration
              }
            PinOutStates[1] = !PinOutStates[1];
            }
          
          if (t_y < mistSW.y + 9 &&
            t_y > mistSW.y - 9 &&
            t_x < mistSW.x + 20 &&
            t_x > mistSW.x - 20) { //fan toggle Switch touched
            if (PinOutStates[2] == 0) { //we were OFF, do what is needed to switch ON
              IOtimeStamps[2] = millis();//set the timestamp
              IOdurations[2] = 60*10; //set the duration
              }
            PinOutStates[2] = !PinOutStates[2];
            }
          
          if (t_y < FanSW.y + 9 &&
            t_y > FanSW.y - 9 &&
            t_x < FanSW.x + 20 &&
            t_x > FanSW.x - 20) { //fan toggle Switch touched
            if (PinOutStates[3] == 0) { //we were OFF, do what is needed to switch ON
              IOtimeStamps[3] = millis();//set the timestamp
              IOdurations[3] = 30; //set the duration
              }
            PinOutStates[3] = !PinOutStates[3];
            }

          if (t_y < seclightSW.y + 9 &&
            t_y > seclightSW.y - 9 &&
            t_x < seclightSW.x + 20 &&
            t_x > seclightSW.x - 20) { //fan toggle Switch touched
            if (PinOutStates[4] == 0) { //we were OFF, do what is needed to switch ON
              IOtimeStamps[4] = millis();//set the timestamp
              IOdurations[4] = 30; //set the duration
              }
            PinOutStates[4] = !PinOutStates[4];
            }

          while (
            tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything
            }
/*
          FanSW.state = FanOut;
          pumpSW.state = pumpOut;
          lightSW.state = lightOut;
          seclightSW.state = secLightOut;
          mistSW.state = mistOut;
*/

          }
        }
      if (page == 1) {  // to do on pressed on clock setup page
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
        }
      if (page == 2) {         //to do on pressed on settings page
        if (tft.getTouch(&t_x, &t_y, 250)) {

          for (int b = 0; b < 5; b++) {
            int butX = (tft.width() / 5) / 2 + (tft.width() / 5) * b;
            int butY = (tft.height() - but_H - 50);
            if (t_x > butX - 27 && t_x < butX + 27 && t_y > butY - 16 && t_y < butY + 16) { //visual feedback on button when press
              simpleBut(outLab[b], butX, butY, simpleBut_W, simpleBut_H, 0);
              page = 3 + b;
              while (
                tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything
                }
              }
            else {
              }
            }
          }// emd of to do when pressed
        }
      if (page > 2) { // setting for the pump
        if (tft.getTouch(&t_x, &t_y, 250)) { //touch detected

          if (t_x > nuriBut_X - nuribut_Wid / 2 + nuribut_Wid + 5 && t_x < nuriBut_X + nuribut_Wid / 2 + nuribut_Wid + 5 &&
            t_y > nuriBut_Y - nuribut_Hei / 2 && t_y < nuriBut_Y + nuribut_Hei / 2) {  // check if the fill button is pressed
            clearTL(page - 3, nuristate);//fill all timeline
            while (tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything to debounce
              }
            }

          if (t_x > tft.width() / 2 - 15 &&
            t_x < tft.width() / 2 + 15 &&
            t_y > nuriBut_Y - (nuribut_Hei) / 2 &&
            t_y < nuriBut_Y + (nuribut_Hei) / 2) { //the changing state buton is pressed

            simpleBut(butlab[!nuristate], tft.width() / 2, nuriBut_Y, nuriBut_X - tft.width() / 2 - 5 * 4, nuribut_Hei, !nuristate);
            simpleBut("SET", nuriBut_X, nuriBut_Y, nuribut_Wid, nuribut_Hei, !nuristate);
            simpleBut("Fill!", nuriBut_X + nuribut_Wid + 5, nuriBut_Y, nuribut_Wid, nuribut_Hei, !nuristate);

            while (tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything to debounce
              }
            nuristate = !nuristate;
            }
          if (t_y > 170 && t_y < 200) { //touch on the slider timespan zone

            drawDuration(t_x);

            }
          while (tft.getTouch(&t_x, &t_y) && t_y > timeLine_Y && t_y < timeLine_Y + 10 + 25) { //if touch on the timeline

            // Update touch readings
            touchX[currentIndex] = t_x;
            touchY[currentIndex] = t_y;
            touchDetected[currentIndex] = pressed;
            currentIndex = (currentIndex + 1) % numSamples;

            // Calculate the average touch position
            int sumX = 0, sumY = 0, count = 0;
            for (int i = 0; i < numSamples; i++) {
              if (touchDetected[i]) {
                sumX += touchX[i];
                sumY += touchY[i];
                count++;
                }
              }

            if (count > 0) {
              avgX = sumX / count;
              avgY = sumY / count;
              }

            drawTimeStringCursor(avgX);
            }

          if (t_x > nuriBut_X - nuribut_Wid / 2 && t_x < nuriBut_X + nuribut_Wid / 2 &&
            t_y > nuriBut_Y - 35 - nuribut_Hei / 2 && t_y < nuriBut_Y - 35 + nuribut_Hei / 2) { //touch on left arrow
            avgX--;
            tft.fillRect(0, timeLine_Y + 10 + 11, tft.width(), 20, BACKGROUND_COLOR);//errase cursors
            tft.fillTriangle(avgX, timeLine_Y + 10 + 11, avgX - 3, timeLine_Y + 10 + 11 + 3, avgX + 3, timeLine_Y + 10 + 11 + 3, TEXT_COLOR); //draw cursor
            drawTimeStringCursor(avgX);
            delay(10);
            }
          if (t_x > nuriBut_X - nuribut_Wid / 2 + nuribut_Wid + 5 && t_x < nuriBut_X + nuribut_Wid / 2 + nuribut_Wid + 5 &&
            t_y > nuriBut_Y - 35 - nuribut_Hei / 2 && t_y < nuriBut_Y - 35 + nuribut_Hei / 2) { //touch on R arrow
            avgX++;
            tft.fillRect(0, timeLine_Y + 10 + 11, tft.width(), 20, BACKGROUND_COLOR);//errase cursors
            tft.fillTriangle(avgX, timeLine_Y + 10 + 11, avgX - 3, timeLine_Y + 10 + 11 + 3, avgX + 3, timeLine_Y + 10 + 11 + 3, TEXT_COLOR); //draw cursor
            drawTimeStringCursor(avgX);
            delay(10);
            }

          if (t_x > nuriBut_X - nuribut_Wid / 2 && t_x < nuriBut_X + nuribut_Wid / 2 &&
            t_y > nuriBut_Y - nuribut_Hei / 2 && t_y < nuriBut_Y + nuribut_Hei / 2) {  // check if the set button is pressed
            while (tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything to debounce
              }
            nuristate == 0 ? IOstates[avgX - 16][page - 3] = 0 : IOstates[avgX - 16][page - 3] = timer;
            //            IOstates[avgX - 16][page - 3] = nuristate;
            }

          drawSpecTimeLine(page - 3);
          }
        }
      if (page != 0) { //common to page other than the main
        if (t_x > 50 && t_x < 100 && t_y > 200 && t_y < 200 + 40) {  // check if the OK button is pressed
          while (tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything
            }
          if (page == 1) {
            saveDate(); //save the date
            page = 0; //return to main
            }
          if (page > 2) {
            saveInFS(page - 3); //save the settings of the page
            page = 2;  // return to Settings screen
            }
          else {
            page = 0; //return to main page
            }
          }
        if (t_x > tft.width() - 100 && t_x < tft.width() - 50 && t_y > 200 && t_y < 200 + 40) {  // check if the Cancel button is pressed
          while (tft.getTouch(&t_x, &t_y)) {  // screen is pressed, stop everything
            }
          if (page == 1) {
            page = 0; //return to main, saving nothing
            }
          if (page > 2) {
            readFromFS(page-3);
            page = 2;  // return to Settings screen, save nothing adn retreive the last saved states
            }
          else {
            page = 0; //return to main page
            }
          }        
        }
      else  // when no touch detected
        {
        }
      } //end of to do when pressed
    }

  if (oldPressed != pressed) { //touch has been released
    if (page == 4) {
     // drawTimeStringCursor(avgX);
     // drawSpecTimeLine(page - 3);
      }
    }

  oldPressed = pressed;

    outputStates(); //change the outputs according to what is needed

    for (int a = 0; a < 5; a++) {
      OldPinOutStates[a] = PinOutStates[a];
      }
    
    /////////////Updating the LCD brightness/////////////////
  int BLintens = 1000 - (millis() - lastPress) / 10;
  if (BLintens > MAXBL) BLintens = MAXBL;
  if (BLintens < MINBL) BLintens = MINBL;
  page == 0 ? analogWrite(TFT_BL, BLintens) : analogWrite(TFT_BL, 255); //set BL adaptive on main page only 
  }
