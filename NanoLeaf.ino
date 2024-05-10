#include <Arduino.h>
#include <FastLED.h>

#define LEAFS 18   // how many panels
#define LEDPLEAF 9 // 5*3

#define NUM_LEDS LEAFS *LEDPLEAF + 1
#define DATA_PIN 7
#define CLOCK_PIN 8
#define POT A1
#define BUT1 2 // button 1
#define BUT2 3 // button 2

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

DEFINE_GRADIENT_PALETTE(Tertiary_01b_gp){0,  0,   1,   80,  63,  20,  1,
                                         34, 127, 120, 1,   2,   191, 177,
                                         66, 1,   255, 247, 255, 0};

DEFINE_GRADIENT_PALETTE(Pastel_Colors_gp){
    0,   120, 122, 56,  12,  179, 90,  69,  25,  255, 62,  84,  38,  179,
    118, 122, 51,  120, 193, 168, 63,  179, 166, 145, 76,  255, 141, 125,
    89,  179, 101, 87,  102, 120, 69,  56,  114, 179, 127, 55,  127, 255,
    205, 54,  140, 179, 127, 55,  153, 120, 69,  56,  165, 179, 131, 123,
    178, 255, 217, 223, 191, 179, 136, 168, 204, 120, 75,  122, 216, 74,
    131, 123, 229, 41,  205, 125, 242, 74,  161, 87,  255, 120, 122, 56};

DEFINE_GRADIENT_PALETTE(Radial_Eyeball_Green_gp){
    0,   0,   0,   0,   22,  0,  0,   0,  43,  0, 0,  0,
    43,  39,  128, 55,  89,  12, 53,  19, 182, 1, 11, 2,
    182, 255, 255, 255, 222, 71, 146, 71, 255, 4, 68, 4};

DEFINE_GRADIENT_PALETTE(Sunset_Yellow_gp){
    0,   10,  62,  123, 36,  56,  130, 103, 87,  153, 225, 85,  100, 199, 217,
    68,  107, 255, 207, 54,  115, 247, 152, 57,  120, 239, 107, 61,  128, 247,
    152, 57,  180, 255, 207, 54,  223, 255, 227, 48,  255, 255, 248, 42};

DEFINE_GRADIENT_PALETTE(Thinky_Pinky_gp){
    0,   255, 171, 245, 58,  252, 191, 247, 90,  249, 213, 250,
    96,  252, 233, 252, 102, 255, 255, 255, 117, 215, 201, 205,
    157, 179, 156, 160, 216, 125, 88,  98,  255, 83,  42,  52};

DEFINE_GRADIENT_PALETTE(sky_21_gp){0,   255, 164, 49, 40, 227, 141, 72, 87, 125,
                                   149, 135, 178, 0,  31, 52,  255, 1,  10, 22};

DEFINE_GRADIENT_PALETTE(bhw3_37_gp){
    0,  2,  1,   7,   56,  7,   13,  44, 119, 169, 22,  12, 153, 229,
    62, 24, 188, 232, 138, 137, 237, 83, 68,  114, 255, 83, 68,  114};

DEFINE_GRADIENT_PALETTE(art_deco_04_gp){
    0,   157, 7,   13,  42,  194, 45,  44, 84,  234, 117, 98, 127, 194,
    133, 125, 170, 157, 151, 156, 212, 87, 103, 111, 255, 39, 66,  74};

DEFINE_GRADIENT_PALETTE(occ092_gp){0,  179, 33,  5,   127, 242,
                                   56, 31,  255, 255, 191, 127};

DEFINE_GRADIENT_PALETTE(purple_orange_d06_gp){
    0,   58,  29,  92,  42,  58,  29,  92,  42,  139, 66,  115,
    84,  139, 66,  115, 84,  188, 138, 151, 127, 188, 138, 151,
    127, 217, 173, 99,  170, 217, 173, 99,  170, 237, 108, 17,
    212, 237, 108, 17,  212, 171, 44,  1,   255, 171, 44,  1};

DEFINE_GRADIENT_PALETTE(blue_tan_d08_gp){
    0,   7,   77,  210, 31,  7,   77,  210, 31,  21,  112, 216, 63,
    21,  112, 216, 63,  53,  149, 207, 95,  53,  149, 207, 95,  123,
    180, 192, 127, 123, 180, 192, 127, 186, 186, 127, 159, 186, 186,
    127, 159, 182, 159, 50,  191, 182, 159, 50,  191, 155, 117, 14,
    223, 155, 117, 14,  223, 115, 72,  2,   255, 115, 72,  2};

DEFINE_GRADIENT_PALETTE(brightsong2_gp){0,   10, 199, 26,  73, 255,
                                        255, 8,  255, 255, 11, 71};

DEFINE_GRADIENT_PALETTE(samba_rose_gp){
    0,   203, 169, 145, 35,  10,  0,   0,   79,  194, 13,  2,   107, 121,
    31,  14,  132, 103, 23,  0,   158, 103, 23,  0,   183, 229, 25,  2,
    214, 255, 255, 8,   244, 255, 244, 240, 255, 255, 244, 240};

DEFINE_GRADIENT_PALETTE(peacerose_gp){/*
                                        150, 239, 244, 221,
                                        221, 239, 47, 105,
                                        200, 222, 184, 145
                                      */
                                      0, 0, 0, 0, 22, 71, 146, 71, 43, 0, 0, 0,
                                      // 43,  239, 244, 221,
                                      // 89,  239, 244, 221,
                                      182, 255, 47, 105, 200, 0, 146, 255,
                                      // 222,  222, 184, 145,
                                      255, 71, 146, 71

};

DEFINE_GRADIENT_PALETTE(pulpfiction_gp){0,   18, 7, 230, 255,
                                        224, 10, 9

};

DEFINE_GRADIENT_PALETTE(tropical_beach_gp){
    0,   1,  61,  85,  38,  9, 176, 255, 63,  255, 97,  5,   96, 255,
    26,  19, 130, 206, 201, 6, 165, 255, 29,  16,  191, 255, 97, 5,
    221, 9,  176, 255, 247, 1, 61,  85,  255, 1,   61,  85};

DEFINE_GRADIENT_PALETTE(Complementary_01a_gp){0,  0,  33,  255, 127, 42,
                                              33, 45, 255, 255, 33,  0};

CRGBPalette16 gPal[] = {
    peacerose_gp,            // all white? *
    Radial_Eyeball_Green_gp, // lot of bk *
    pulpfiction_gp,          // good subtle violet blue?? **
    OceanColors_p,           // only blue ? *
    RainbowColors_p,
    ForestColors_p, // green subtle ****
    CloudColors_p,
    Complementary_01a_gp, //****
    tropical_beach_gp,    // good orange (lagune blue?) ***
    samba_rose_gp,        // red bk grey ***
    brightsong2_gp,       // kimidori green *****
    blue_tan_d08_gp,      // subtle sky blue*****
    purple_orange_d06_gp, // pink violet*****
    occ092_gp,            // only red ? *
    bhw3_37_gp,           // pink blu super dark *

    Sunset_Yellow_gp, // light yellow sky *****
    spring_gp,        // pink light violet *****
    Primary_01_gp,    // light yellow sky *****
    Tertiary_01b_gp,  // pink bk blue dark ***
    Pastel_Colors_gp, // pink wh light orange*****
    Thinky_Pinky_gp,  // all wh?*
    sky_21_gp,        // light lagune wgrey yell light orange****
    art_deco_04_gp    // wh pink red *****
};
byte thisPal = 0; // the index of the palette we are using
int l;            // the leaf we want to manipulate the color
bool release1;

unsigned long counter;
int countTarget;

void setup() {
  Serial.begin(57600);
  Serial.println("resetting");
  // LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(
      leds, NUM_LEDS); //.setCorrection( TypicalSMD5050 );

  LEDS.setBrightness(255);

  pinMode(POT, INPUT);
  pinMode(BUT1, INPUT_PULLUP);
}

void loop() {

  if (digitalRead(BUT1) == LOW && release1 == true) {
    release1 = false;
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
      leds[lea * LEDPLEAF + a] =
          nblend(leds[lea * LEDPLEAF + a], color[lea], 3);
      //  leds[lea * LEDPLEAF + a] =color[lea]; //for testing
    }
  }

  fadeall();

  // fill_palette(leds, NUM_LEDS, 0, 1, gPal[thisPal], 255, LINEARBLEND);
  // Serial.println(counter);
  FastLED.show();

  // periodically set random pixel to a random color, to show the fading
  if (counter + countTarget < millis()) { ////////////////
    countTarget = random(5, 50);
    counter = millis();
    l = random8(LEAFS);
    // l > LEAFS-2 ? l = 0 : l++;
    uint8_t pos = random8(LEAFS);
    // color = CRGB( random8(255),random8(255),random8(255));

    color[l] = ColorFromPalette(gPal[thisPal], random8()); //,255,LINEARBLEND);
    leaf[l] = color[l];
  } /////////////

  FastLED.delay(30);
}

void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    // leds[i].nscale8(250);
    leds[i].nscale8_video(245);
  }
}
