//// Control.h
// Base class for controlling the totem pole
//
// Provides interface to control of the lights and storage of paramaters
// Derivative classes for UI control and LED control
#ifndef CONTROL_H
#define CONTROL_H

//#define DEBUG
#ifdef DEBUG
  #define DEBUG(x)    Serial.print(x)
  #define DEBUG_L(x)  Serial.println(x)
#else
  #define DEBUG(x)
  #define DEBUG_L(x)
#endif
#define DISPLAY
#ifdef DISPLAY
  #define DISPLAY(x)    Serial.print(x)
  #define DISPLAY_L(x)  Serial.println(x)
#endif

#include <FastLED.h>

// Information about the LED strip itself
#define LED_PIN     9
#define NUM_COLS    9
#define NUM_ROWS    9
#define NUM_LEDS    (NUM_COLS * NUM_ROWS) //using an 8x8 matrix of LEDS eventually...
//#define NUM_LEDS    8
const bool MatrixSerpentineLayout = true; //if LEDs are snaking or not (most likely, yes)
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
#define TEMPERATURE OvercastSky

#define TAP_PIN           A3    //for tap tempo


// todo MORE PATTERNS
// sync all patterns to BPM using bool beatNow (see rolling_rows() for example)
// implement hue speed changes

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
    
    void set_pattern(uint8_t pattern) {currentPatternNumber = pattern;}
    String getPatternName() {return String::String(patternNames[currentPatternNumber]);}
    void inc_pattern();
    void dec_pattern();
    void setHueSpeed(uint8_t speeed) {speed_m = speeed;}    // control speed at which hue changes
    String getHueSpeed() {return String::String(speed_m);}  // between 5 - 95msec for hue change
    void incHueSpeed(uint8_t i = 3) {speed_m = max(speed_m - i, 5);}
    void decHueSpeed(uint8_t i = 3) {speed_m = min(speed_m + i, 95);}
    void set_tempo(unsigned short tempo) {tempo_m = tempo;}

    //tap tempo functions
    unsigned short get_tempo() {return constrain(tempo_m, 200, 2000) ; }
    uint8_t get_BPM() {return (60000/tempo_m);}
    void tap();
    void tap_toggle();

  private:
    //Varibles for FPS
    const uint8_t FPS = 60; 
    unsigned long lastUpdate;

    //UI related variables
    uint8_t brightness_m;
    uint8_t speed_m; 
    uint8_t currentPatternNumber;
    unsigned short tempo_m; // recorded as the miliseconds between two beats e.g. 120BPM = 500msec between beats

    //LEDS and helper functions
    CRGB *leds_m;
    uint8_t nLeds_m;
    //LEDS are arranged in an 8X8 design around a globe
    //array has bottom to top, clockwise fashion. 
    // e.g. 0-7 is 12o'clock, bottom to top, 8-15 is next column (~1.20o'clock) bottom to top, etc. 
    void selectRow(uint8_t row, CRGB *leds[8]);   // for manipulating a whole row at once
    CRGB * selectRow2(uint8_t row);
    void selectCol(uint8_t col, CRGB *leds[8]);   // for manipulating a whole column at once
    CRGB * selectCol2(uint8_t col);
    uint8_t atRowCol(uint8_t row, uint8_t col);     // returns index i of led at row & col (accounting for Serpentine order)

    void addGlitter(fract8 chanceOfGlitter = 80);
    void pulseToBeat(); 

    /******************************/
    /*        PATTERNS            */
    /******************************/
    //Pattern variables
    uint8_t hue_m;      //rotating 'base colour' used by patterns
    bool newPattern_m;    //used to initialise new patterns
    
    //Pattern array
    static const uint8_t numPatterns = 7;
    typedef void (Control::*PatternList[numPatterns])();
    PatternList patterns_m = { &rainbow, &confetti, &rolling_rows_diag, &rolling_rows, &scroll_rows, &BPM_boogie, &randomLights  };;   // BPM_boogie, scroll_rows
    // Pattern names for display, max 14 chars
    const char *patternNames[numPatterns] = { "Rainbow", "Confetti", "Roll Rows (D)", "Roll Rows", "Scroll Rows", "BPM Boogie", "Random Lights" };
    
    //Patterns
    void rainbow();
    void confetti();
    void rolling_rows_diag();
    void rolling_rows();
    void BPM_boogie();
    void scroll_rows();
    void randomLights();

    /******************************/
    /*      TAP TEMPO CONTROL     */
    /******************************/
    //variables
    int lastTapState = LOW;  /* the last tap button state */
    bool beatNow = false;
    bool tapOn = true;
    unsigned long currentTimer[2] = { 500, 500 };  /* array of most recent tap counts */
    unsigned long timeoutTime = 0;  /* this is when the timer will trigger next */

    unsigned long indicatorTimeout; /* for our fancy "blink" tempo indicator */
    unsigned long lastTap = 0; /* when the last tap happened */
    //functions
    void updateTap();
    void rescheduleTimer();
};

#endif /* LEDControl_H */
