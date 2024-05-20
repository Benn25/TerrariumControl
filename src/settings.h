//the buttons OK and cancel
#define but_W 100 //OK but W
#define but_H 30 //OK but H
#define but_X 80 //OK but X pos
#define but_Y tft.height() - but_H  - 5 //OK but Y pos

#define simpleBut_W 58
#define simpleBut_H 30

///////buttons on the specific settings pages
#define nuribut_Wid  60
#define nuribut_Hei  35
#define nuriBut_X  tft.width()-10-5*2-nuribut_Wid*3/2//tft.width() / 2+40
#define nuriBut_Y  tft.height() / 2+12


#define LowGraphPos 142  // Y coordinate of the lower part of the graph
#define GraphH 130       // height of the graph
#define mettersYpos 197  // Y position of the round metters

#define MAXBL 255    // max backlight
#define MINBL 60     // min backlight
#define max_temp 45  // maximum reachable temperature
#define FanWorkTime 60 //duration you want the fan to work for
#define timeLine_Y 45 //Y position of the timeline for settings

#define DEG2RAD 0.0174532925

// some principal color definitions
// RGB 565 color picker at
// https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
// in the form ((R << 11) | (G << 5) | B), max vales are R=31, G=63, B=31

#define WHITE 0xFFFF
#define BLACK 0xFFFF //0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define GREY 0x2108
#define SCALE0 0xC655  // accent color for unused scale segments
#define SCALE1 0x5DEE  // accent color for unused scale segments
#define TEXT_COLOR TFT_DEEPGREY//0xEF3B  // 0xFFFF
#define INV_TEXT_COLOR TFT_WHITE 
#define BACKGROUND_COLOR TFT_OFFWHITE//0x18a3//0x20E4  //((3 << 11) | (8 << 5) | 3)     //sort of dark grey super dark
#define TEMP_COLOR 0xF6F4  //((20 << 11) | (40 << 5) | 20)
#define HYGRO_COLOR 0x7DF8
#define DARK_GOLD 0xACA4
#define DARK_RED 0xb000

// char* label[] = {"","Celsius","%","AMP", "VOLT"};            // some custom
// gauge labels
char lab[2][10] = { "temp", "hygro" };
//output labels
char outLab[5][10] = { "light", "pump","mist","fan","sunset" };
char outTitleLab[5][22] = {
"---= MAIN LIGHT =---",
"---= PUMP =---",
"---= MIST =---",
"---= FAN =---",
"---= SUNSET =---" };
//labels for ON and OFF
char butlab[2][10] = { "OFF", "ON" };

//def the colors of the outputs for the timeline
//the order is light, pump, mist, fan, secLight
int IOcolors[] = {
TFT_GOLD,
TFT_WHITE,
HYGRO_COLOR,
TFT_GREEN,
TFT_RED
    };
int IODcolors[] = { //dark colors for the timeline
    DARK_GOLD,
    TFT_DARKGREY,
    TFT_DARKCYAN,
    TFT_DARKGREEN,
    DARK_RED
    };

static int temp_cursor;

float Maxdurations[5] = { //max duration for each parameters, in h
    25/*main light, full day max*/,
    4/*pump*/,
    2/*mist*/,
    1/*fan*/,
    1/*unset*/
    };

#define SunriseWinter 421 //the min of day of sunrise on dec 21
#define SunriseSummer 283 //the min of day of sunrise on jun 21
#define SunsetWinter 1009 //the min of day of sunset on dec 21
#define SunsetSummer 1154 //the min of day of sunset on jun 21

#define Pin_Fan 5
#define Pin_Fan 5
#define Pin_Fan 5
#define Pin_Fan 5
#define Pin_Fan 5

int Out_Pin[5] = { 5,6,7,8,9 }; //pins for all the outputs