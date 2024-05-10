#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <FastLED.h>


extern "C" {
#include <os_type.h>
#include <osapi.h>
}


#define LEAFS 18   // how many panels
#define LEDPLEAF 9 // 5*3

#define NUM_LEDS LEAFS *LEDPLEAF // + 1
#define DATA_PIN D6//6 on nano, Dx on wemos
#define CLOCK_PIN D7//7 on nano Dx on wemos
#define ANAL A0
#define BUT1 D0 // button 1,pin 0 on Wemo, 2 on nano
#define BUT2 3 // button 2, pin 3 on nano 

int leaf[LEAFS]; // the leaf we are changing the color
CRGB leds[NUM_LEDS];
CRGB leafs[LEAFS];
CRGB color[LEAFS];

DEFINE_GRADIENT_PALETTE(spring_gp){
    0,   255, 0,   255, 17,  255, 1,   212, 33,  255, 2,   178, 51,
    255, 7,   145, 68,  255, 13,  115, 84,  255, 22,  92,  102, 255,
    33,  71,  119, 255, 47,  52,  135, 255, 62,  37,  153, 255, 82,
    25,  170, 255, 104, 15,  186, 255, 127, 9,   204, 255, 156, 4,
    221, 255, 186, 1,   237, 255, 217, 1,   255, 255, 255, 0};

DEFINE_GRADIENT_PALETTE(Primary_01_gp){0,   16,  82,  255, 63, 91,  156,
                                       103, 127, 255, 255, 25, 191, 255,
                                       75,  18,  255, 255, 3,  12};

DEFINE_GRADIENT_PALETTE(Radial_Eyeball_Green_gp){
    0,   0,   0,   0,   22,  1,  11,   2,  43,  0, 0,  0,
    43,  39,  128, 55,  89,  12, 53,  19, 182, 1, 11, 2,
    182, 255, 255, 255, 222, 71, 146, 71, 255, 4, 68, 4};

DEFINE_GRADIENT_PALETTE(Sunset_Yellow_gp){
    0,   10,  62,  123, 36,  56,  130, 103, 87,  153, 225, 85,  100, 199, 217,
    68,  107, 255, 207, 54,  115, 247, 152, 57,  120, 239, 107, 61,  128, 247,
    152, 57,  180, 255, 207, 54,  223, 255, 227, 48,  255, 255, 248, 42};

DEFINE_GRADIENT_PALETTE(sky_21_gp){0,   255, 164, 49, 40, 227, 141, 72, 87, 125,
                                   149, 135, 178, 0,  31, 52,  255, 1,  10, 22};

DEFINE_GRADIENT_PALETTE(bhw3_37_gp){
    0,  2,  1,   7,   56,  7,   13,  44, 119, 169, 22,  12, 153, 229,
    62, 24, 188, 232, 138, 137, 237, 83, 68,  114, 255, 83, 68,  114};

DEFINE_GRADIENT_PALETTE(purple_orange_d06_gp){
    0,   58,  29,  92, 
     42,  58,  29,  92,
       43,  139, 66,  115,
    84,  139, 66,  115,
     85,  188, 138, 151,
      127, 188, 138, 151,
    128, 217, 173, 99,
      170, 217, 173, 99,
        171, 237, 108, 17,
    212, 237, 108, 17,
      213, 171, 44,  1,
         255, 171, 44,  1};

DEFINE_GRADIENT_PALETTE(blue_tan_d08_gp){
    0,   7,   77,  210, 31,  7,   77,  210, 31,  21,  112, 216, 63,
    21,  112, 216, 63,  53,  149, 207, 95,  53,  149, 207, 95,  123,
    180, 192, 127, 123, 180, 192, 127, 186, 186, 127, 159, 186, 186,
    127, 159, 182, 159, 50,  191, 182, 159, 50,  191, 155, 117, 14,
    223, 155, 117, 14,  223, 115, 72,  2,   255, 115, 72,  2};

DEFINE_GRADIENT_PALETTE(brightsong2_gp){0,   10, 199, 26,  73, 255,
                                        255, 8,  255, 255, 11, 71};

DEFINE_GRADIENT_PALETTE(peacerose_gp){0,  239, 244, 221, 221, 239,
                                      47, 105, 255, 222, 184, 145};

DEFINE_GRADIENT_PALETTE(pulpfiction_gp2){0, 18, 7, 230, 255, 224, 10, 9};


DEFINE_GRADIENT_PALETTE(tropical_beach_gp){
    0,   1,  61,  85,  38,  9, 176, 255, 63,  255, 97,  5,   96, 255,
    26,  19, 130, 206, 201, 6, 165, 255, 29,  16,  191, 255, 97, 5,
    221, 9,  176, 255, 247, 1, 61,  85,  255, 1,   61,  85};

DEFINE_GRADIENT_PALETTE(Complementary_01a_gp){0,  0,  33,  255, 127, 42,
                                              33, 45, 255, 255, 33,  0};

DEFINE_GRADIENT_PALETTE(YlGnBu_03_gp){0,   210, 239, 102, 84,  210, 239, 102,
                                      84,  41,  157, 117, 170, 41,  157, 117,
                                      170, 2,   55,  112, 255, 2,   55,  112};
// Gradient palette "ib33_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/general/tn/ib33.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE(springbird_gp){0,   10,  45,  147, 38,  10,  45,  147,
                                       216, 192, 121, 162, 255, 192, 121, 162};

DEFINE_GRADIENT_PALETTE(i_hedvajz_gp){
    0,   21,  142, 130, 137, 21,  142, 130, 137, 107, 217, 174, 150, 107,
    217, 174, 150, 194, 255, 190, 181, 194, 255, 190, 181, 132, 244, 38,
    193, 132, 244, 38,  193, 87,  173, 2,   255, 87,  173, 2};

// Gradient palette "Need_I_Say_More_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/sugar/tn/Need_I_Say_More.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(Need_I_Say_More_gp){
    0,   224, 49,  13,  84,  224, 49,  13,  84,  239, 103, 34, 114, 239,
    103, 34,  114, 229, 176, 68,  137, 229, 176, 68,  137, 82, 142, 83,
    168, 82,  142, 83,  168, 25,  111, 89,  255, 25,  111, 89};


// Gradient palette "Alive_And_Kicking_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/rphnick/tn/Alive_And_Kicking.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(Alive_And_Kicking_gp){
    0,   192, 50,  207, 51,  192, 50,  207, 51,  87,  50,  207, 102, 87,
    50,  207, 102, 37,  73,  207, 153, 37,  73,  207, 153, 37,  127, 207,
    204, 37,  127, 207, 204, 37,  213, 140, 255, 37,  213, 140};

// Gradient palette "Water_Polo_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/rphnick/tn/Water_Polo.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(Water_Polo_gp){
    0,   104, 213, 201, 51,  104, 213, 201, 51,  56,  182, 166, 102, 56,
    182, 166, 102, 27,  147, 128, 153, 27,  147, 128, 153, 11,  105, 87,
    204, 11,  105, 87,  204, 4,   73,  57,  255, 4,   73,  57};

// Gradient palette "bhw1_sunset1_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_sunset1.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE(bhw1_sunset1_gp){0,   33,  21,  25,  38,  125, 29,  20,
                                         71,  222, 59,  30,  145, 190, 147, 127,
                                         178, 88,  136, 203, 255, 3,   24,  78};

// Gradient palette "4k_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/alpen/tn/4k.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.


DEFINE_GRADIENT_PALETTE(textures_in_blue_gp){
    0,   12, 47,  77, 51,  12, 47,  77, 51,  3,  29,  60, 102, 3,
    29,  60, 102, 1,  19,  47, 153, 1,  19,  47, 153, 1,  15,  36,
    204, 1,  15,  36, 204, 1,  8,   23, 255, 1,  8,   23};

// Gradient palette "l_o_v_e_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/colo/tvr/tn/l_o_v_e.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 40 bytes of program space.

DEFINE_GRADIENT_PALETTE(l_o_v_e_gp){
    0,  12,  1,  1,   17, 12,  1,  1,   17,  64,  1,  1,   35, 64,
    1,  1,   35, 128, 1,  1,   56, 128, 1,   1,   56, 210, 32, 0,
    76, 210, 32, 0,   76, 104, 95, 39,  255, 104, 95, 39};

DEFINE_GRADIENT_PALETTE(Adrift_in_Dreams_gp){
    0,  148, 223, 77,  25, 148, 223, 77,  51, 115, 201, 83, 76, 86,  182,
    89, 102, 57,  156, 80, 127, 36,  131, 72, 153, 17,  93, 61, 178, 5,
    61, 51,  204, 1,   34, 38,  229, 1,   15, 29,  255, 1,  15, 29};

// Gradient palette "bhw1_16_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/bhw/bhw1/tn/bhw1_16.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE(bhw1_16_gp){0, 252, 255, 102, 255, 155, 213, 250};

// Gradient palette "xmas_04_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/xmas/tn/xmas_04.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE(green_gp){0,   8,  66,  31,  68,  14,
                                    105, 59, 255, 115, 223, 102};

// Gradient palette "moon_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/pn/tn/moon.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 188 bytes of program space.

DEFINE_GRADIENT_PALETTE(moon_gp){
    0,   53,  99,  145, 99,  107, 147,
    166, 255, 244, 248, 205};

// Gradient palette "YlOrRd_09_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/cb/seq/tn/YlOrRd_09.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 72 bytes of program space.

DEFINE_GRADIENT_PALETTE(YlOrRd_09_gp){
    0,   180, 180, 75, 28,  255, 255, 145, 28,  255, 217, 79,  56,  255, 217,
    79,  56,  252, 178, 37,  84,  252, 178, 37,  84,  252, 115, 12,  113, 252,
    115, 12,  113, 249, 69,  6,   141, 249, 69,  6,   141, 247, 18,  2,   170,
    247, 18,  2,   170, 188, 1,   1,   188, 188, 1,   1,   188, 117, 0,   2,
    210, 117, 0,   2,   210, 42,  0,   2,   255, 10,  0,   1};

CRGBPalette16 gPal[] = {
    pulpfiction_gp2,
     YlOrRd_09_gp, green_gp, moon_gp,
    // Adrift_in_Dreams_gp, l_o_v_e_gp, textures_in_blue_gp,
    //  bhw1_sunset1_gp,
    Need_I_Say_More_gp, Alive_And_Kicking_gp,
    // Water_Polo_gp,
    i_hedvajz_gp,
    sky_21_gp, // light lagune wgrey yell light orange****
    springbird_gp, YlGnBu_03_gp,
    // OceanColors_p,  // only blue ? *
    ForestColors_p, // green subtle ****
    CloudColors_p,
    Complementary_01a_gp,    //****
    tropical_beach_gp,       // good orange (lagune blue?) ***
    brightsong2_gp,          // kimidori green *****
    blue_tan_d08_gp,         // subtle sky blue*****
    purple_orange_d06_gp,    // pink violet*****
    bhw3_37_gp,              // pink blu super dark *
    Radial_Eyeball_Green_gp, // lot of bk *
    Sunset_Yellow_gp,        // light yellow sky *****
    spring_gp,               // pink light violet *****
    // Primary_01_gp,           // light yellow sky *****
};
byte thisPal = 0; // the index of the palette we are using
int l;            // the leaf we want to manipulate the color
bool release1;
bool NEWSTATE = true; //whole change of every leafs on palette change
byte MODE = 0; //0=mode randonm , 1=mode susumu
uint8_t pos;   //the position of the leaf we want to change the color

unsigned long counter;
int countTarget;
float potVal;

int Zone = 255; //the wideness of the zone where to take the color in the palette
byte Target = 128; //the center of the zone
byte colorPick; //the location of the color

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    // leds[i].nscale8(250);
    leds[i].nscale8_video(240);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("resetting");
  // LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS)
      .setTemperature(0xB5D6FF)
  .setCorrection(TypicalSMD5050);

  LEDS.setBrightness(255);

  // pinMode(POT, INPUT);
  pinMode(BUT1, INPUT_PULLUP);
  pinMode(BUT2, INPUT_PULLUP);
}

// Function from Mark's example...
//
// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.

void nblendU8TowardU8(uint8_t &cur, const uint8_t target, uint8_t amount) {
  if (cur == target)
    return;

  if (cur < target) {
    uint8_t delta = target - cur;
    delta = scale8_video(delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video(delta, amount);
    cur -= delta;
  }
}

CRGB fadeTowardColor(CRGB &cur, CRGB &target, uint8_t amount) {
  nblendU8TowardU8(cur.red, target.red, amount);
  nblendU8TowardU8(cur.green, target.green, amount);
  nblendU8TowardU8(cur.blue, target.blue, amount);
  return cur;
}
// Helper function that blends one uint8_t toward another by a given amount

void loop() {
  if (digitalRead(BUT2) == LOW){//mode button is pressed
    Serial.print("MODE=");
    Serial.println(MODE);
    delay(250);
    MODE == 1 ? MODE = 0 : MODE++;
    Serial.print("MODE=");
    Serial.println(MODE);
  }

  potVal = map(analogRead(ANAL), 0, 1023, 15, 30);
  potVal /= 20;

  Serial.println(potVal);

  if (digitalRead(BUT1) == LOW && release1 == true) { // button is pressed
    release1 = false;
    NEWSTATE = true;
    thisPal >= (sizeof(gPal) / sizeof(gPal[0])) - 1 ? thisPal = 0 : thisPal++;
    delay(40);
    Serial.println(thisPal);
    }
  if (digitalRead(BUT1) == HIGH) {
    release1 = true;
  }

  for (int lea = 0; lea < LEAFS; lea++) {
    leafs[lea] = color[lea];

    for (int a = 0; a < LEDPLEAF; a++) {
      //leds[lea * LEDPLEAF + a] = blend(leds[lea * LEDPLEAF + a], color[lea], random(40));
      // leds[lea * LEDPLEAF + a] =color[lea];
      leds[lea * LEDPLEAF + a] =
          fadeTowardColor(leds[lea * LEDPLEAF + a], color[lea], 1);
    }
  }

  //fadeall();
  
/*
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
      
      map(i, 0, NUM_LEDS - 1, 0, 240); // index is now based on pixel number
      leds[i] =
          ColorFromPalette(gPal[thisPal], paletteIndex++, 100, LINEARBLEND);
      // fill_palette(leds, NUM_LEDS, paletteIndex++, 4, gPal[thisPal], 100,
      // LINEARBLEND); //
      FastLED.delay(20);
    }
    */
    //
    // LINEARBLEND or NOBLEND
    // Serial.println(counter);
 


 
 //static uint8_t paletteIndex = 0;
 FastLED.show();

 EVERY_N_SECONDS(2) {
   //Zone = min(random16(70, 400), 255);
   int randVal = random16(70, 400) > 255 ? Zone = 255 : Zone = randVal;

   Target = random8(35, 255 - 35);
}

if(NEWSTATE){//fillup the full panels with the new colors when the palette is changed.
  for (int a = 0 ; a < LEAFS ; a++) {
    color[a] = ColorFromPalette(gPal[thisPal],random8(), 70, NOBLEND);
  }
  NEWSTATE = false;
}

  // periodically set a new color to a random leaf
  if (counter + countTarget < millis()) { ////////////////
    countTarget = random(20, 250);
    countTarget *= potVal;
    counter = millis();

    if(MODE==0){
      l = random8(LEAFS); // pick a leaf to change the color
      pos = random8(LEAFS);
    }
    else{
      l > LEAFS - 2 ? l = 0 : l++;
      pos = l;
    }
    
     
     // colorPick = constrain(random8(Target - Zone / 2, Target + Zone / 2), 0,
     // 255);

     color[l] = ColorFromPalette(gPal[thisPal], colorPick+=3, 65, LINEARBLEND);
     leaf[pos] = color[l];
  } /////////////

//  FastLED.delay(25);
  FastLED.delay(20);
}
