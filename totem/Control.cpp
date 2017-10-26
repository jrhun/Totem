#include "Control.h"

// ********************************
//  Control Implementation
// ********************************

// constructor
Control::Control(CRGB *l, uint8_t nLeds) 
  : brightness_m(96), newPattern_m(true), lastUpdate(0)
{ 
  leds_m = l;
  nLeds_m = nLeds;
}

void Control::setupControl()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds_m, nLeds_m).setCorrection( TypicalSMD5050 );
  FastLED.setBrightness( brightness_m );
  FastLED.setTemperature( TEMPERATURE );
}
  
void Control::handleControl() 
{
  // Monitors for UI input
  // Updates UI components

  static uint8_t gHue = 0;   //rotating base colour used by patterns
  
  // Calls render if ready for it, then updates LEDs
  unsigned long lastInterval = millis() - lastUpdate;      //essetnailly currentMillis - lastUpdate
  if (lastInterval >= (1000/FPS))
  {
    // Call the current pattern function once, updating the 'leds' array
    //gPatterns[gCurrentPatternNumber]();
    
//    static uint8_t starthue = 0;
//    fill_rainbow( leds_m, nLeds_m, --starthue, 20);

    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8( this->get_BPM(), 64, 255);
    for( int i = 0; i < nLeds_m; i++) { //9948
      leds_m[i] = ColorFromPalette(palette, gHue+(i*2), beat/*-gHue*/+(i*10));
    }
    FastLED.show();
  }

  updateTap();
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}

//void set_brightness(uint8_t brightness)
//{
//  brightness_m = brightness;
//}
//void set_pattern(uint8_t pattern);// {pattern_m = pattern};
//void set_speed(uint8_t speeed);// {speed_m = speeed};
//
//    void set_tempo(unsigned short tempo_m); 

//LED HELPER FUNCTIONS
void Control::selectRow(CRGB *leds[8], uint8_t row) 
{
  // pass it a pointer to a CRGB pointer array[8], and it'll populate it with the pointers of the selected row
  // MUST PASS A SIZE 8 ARRAY OF POINTERS
  for (uint8_t i = 0; i < 8; i++)
  {
    leds[i] = &leds_m[row + i*8];
    DEBUG("Led number in array:\t");
    DEBUG_L(row + i*8);
  }
}
void Control::selectCol(CRGB *leds[8], uint8_t col) 
{
  // pass it a pointer to a CRGB pointer array[8], and it'll populate it with the pointers of the selected col
  // MUST PASS A SIZE 8 ARRAY OF POINTERS
  for (uint8_t i = 0; i < 8; i++)
  {
    leds[i] = &leds_m[col*8 + i];
    DEBUG("Led number in array:\t");
    DEBUG_L(col*8 + i);
  }
}

void Control::updateTap() 
{
  /* check for timer timeout */
  if( millis() >= timeoutTime )
  {
    /* timeout happened.  clock tick! */
    indicatorTimeout = millis() + 50;  /* this sets the time when LED 13 goes off */
    /* and reschedule the timer to keep the pace */
    rescheduleTimer();
  }
  
  //display tap tempo
  if( millis() < indicatorTimeout ) {
    digitalWrite( TAP_PIN, HIGH );
  } else {
    digitalWrite( TAP_PIN, LOW );
  }
}

void Control::tap()
{
  /* we keep two of these around to average together later */
  currentTimer[1] = currentTimer[0];
  currentTimer[0] = millis() - lastTap;
  lastTap = millis();
  timeoutTime = 0; /* force the trigger to happen immediately - sync and blink! */
  
  // only update tempo if we have two valid taps, greater than 6BPM
//  if ( (currentTimer[0] + currentTimer[1])/2 < 10000) )
//  {
    tempo_m = (currentTimer[0] + currentTimer[1])/2;
//  }
  DEBUG("\tmsec b/w beats:\t");
  DEBUG(Control::get_tempo());
  DEBUG("\tBPM:\t");
  DEBUG_L(Control::get_BPM());

  CRGB *row[8];
  Control::selectCol(row, 2);
  DEBUG("Col");
  for (uint8_t i = 0; i < 8; i++)
  {
    DEBUG(i);
    DEBUG(":\tR:"); DEBUG(row[i]->r);
    DEBUG("\tG:");  DEBUG(row[i]->g);
    DEBUG("\tB:");  DEBUG_L(row[i]->b);
  }
}

void Control::rescheduleTimer()
{
    /* set the timer to go off again when the time reaches the 
       timeout.  The timeout is all of the "currentTimer" values averaged
       together, then added onto the current time.  When that time has been
       reached, the next tick will happen...
    */
    timeoutTime = millis() + ((currentTimer[0] + currentTimer[1])/2);
}

