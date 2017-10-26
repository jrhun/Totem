//// Control.h
// Base class for controlling the totem pole
//
// Provides interface to control of the lights and storage of paramaters
// Derivative classes for UI control and LED control
#ifndef CONTROL_H
#define CONTROL_H

#define DEBUG
#ifdef DEBUG
  #define DEBUG(x)    Serial.print(x)
  #define DEBUG_L(x)  Serial.println(x)
#else
  #define DEBUG(x)
#endif

#include <FastLED.h>

// Information about the LED strip itself
#define LED_PIN     9
#define NUM_LEDS    20
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
#define TEMPERATURE OvercastSky

//using an 8x8 matrix of LEDS eventually...
//#define ROWS        8
//#define COLS        8
#define TAP_PIN           A0    //for tap tempo

class Control
{
  public:
    //Pass the led array to control for management
    Control(CRGB *l, uint8_t nLeds);
    
    void setupControl();
    
    void render();
      // This is where changes to the LED array occur
    void handleControl();
      // Monitors for UI input
      // Updates UI components
      // Calls render if ready for it, then updates LEDs

    void set_brightness(uint8_t brightness) {brightness_m = constrain(brightness,0,255); FastLED.setBrightness(brightness_m);}
    void decBrightness(uint8_t i = 3) {brightness_m = max(brightness_m - i, 0);   FastLED.setBrightness(brightness_m);}
    void incBrightness(uint8_t i = 3) {brightness_m = min(brightness_m + i, 255); FastLED.setBrightness(brightness_m);}
    uint8_t getBrightness() {return brightness_m;};
    
    void set_pattern(uint8_t pattern) {pattern_m = pattern;}
    void set_speed(uint8_t speeed) {speed_m = speeed;}
    void set_tempo(unsigned short tempo) {tempo_m = tempo;}

    //tap tempo functions
    unsigned short get_tempo() {return constrain(tempo_m, 200, 2000) ; }
    uint8_t get_BPM() {return (60000/tempo_m);}
    void tap();

  private:
    const uint8_t FPS = 60; 
    unsigned long lastUpdate;

    //TODO intensity setting for effects
    uint8_t brightness_m;
    uint8_t speed_m; 
    unsigned short tempo_m; // recorded as the miliseconds between two beats e.g. 120BPM = 500msec between beats
    uint8_t pattern_m;
    uint8_t numPatterns_m;
    bool newPattern_m;    //used to initialise new patterns

    //LEDS and helper functions
    CRGB *leds_m;
    uint8_t nLeds_m;
    //LEDS are arranged in an 8X8 design around a globe
    //array has bottom to top, clockwise fashion. 
    // e.g. 0-7 is 12o'clock, bottom to top, 8-15 is next column (~1.20o'clock) bottom to top, etc. 
    void selectRow(CRGB *leds[8], uint8_t row);
    void selectCol(CRGB *leds[8], uint8_t col);

    //tap tempo control
    //variables
    int lastTapState = LOW;  /* the last tap button state */
    unsigned long currentTimer[2] = { 500, 500 };  /* array of most recent tap counts */
    unsigned long timeoutTime = 0;  /* this is when the timer will trigger next */

    unsigned long indicatorTimeout; /* for our fancy "blink" tempo indicator */
    unsigned long lastTap = 0; /* when the last tap happened */
    //functions
    void updateTap();
    void rescheduleTimer();
};

#endif /* LEDControl_H */
