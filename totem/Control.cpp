#include "Control.h"

/********************************/
/*  Control Implementation      */
/********************************/
// constructor
Control::Control(CRGB *l, uint8_t nLeds) 
  : brightness_m(96), newPattern_m(true), lastUpdate(0), currentPatternNumber(0), hue_m(0), beatNow(false)
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
  // Monitors for UI input???
  // Updates UI components??? (done in UI - could reconsolidate)

  // Calls patterns once to render if ready for it, then updates LEDs
  unsigned long lastInterval = millis() - lastUpdate;      //essetnailly currentMillis - lastUpdate
  if (lastInterval >= (1000/FPS))
  {
    lastUpdate = millis();
    // Call the current pattern function once, updating the 'leds' array
    (this->*patterns_m[currentPatternNumber])();

    //update the leds
    FastLED.show();
  }

  updateTap();    // update tap tempo display
  
  EVERY_N_MILLISECONDS( 20 ) { hue_m++; }
}

void Control::inc_pattern(){
  currentPatternNumber = (currentPatternNumber + 1) % numPatterns;
}
void Control::dec_pattern(){
  if (currentPatternNumber == 0) {currentPatternNumber = numPatterns - 1;}
  else                           {currentPatternNumber--;}
}

void Control::incHueSpeed(){
  //
}
void Control::decHueSpeed() {
  //
}

/******************************/
/*   LED HELPER FUNCTIONS     */
/******************************/
void Control::selectRow(uint8_t row,CRGB *leds[NUM_COLS]) 
{
  // pass it a pointer to a CRGB pointer array[NUM_COLS], and it'll populate it with the pointers to leds in the column of the selected row
  // MUST PASS A SIZE NUM_COL ARRAY OF POINTERS
  for (uint8_t col = 0; col < NUM_ROWS; col++){
    leds[col] = &leds_m[ atRowCol(row,col) ];
//    DEBUG("Led number in array:\t");
//    DEBUG_L(row + i*8);
  }
}

CRGB * Control::selectRow2(uint8_t row) 
{
  CRGB *leds[NUM_COLS];
  for (uint8_t col = 0; col < NUM_ROWS; col++) {
    leds[col] = &leds_m[ atRowCol(row,col)]; 
  }
  return *leds;
}

void Control::selectCol(uint8_t col,CRGB *leds[NUM_ROWS]) 
{
  // pass it a pointer to a CRGB pointer array[8], and it'll populate it with the pointers of the selected col
  // MUST PASS A SIZE NUM_ROWS ARRAY OF POINTERS
  for (uint8_t row = 0; row < NUM_ROWS; row++) {
    // i = number in row
    
    leds[row] = &leds_m[atRowCol(row, col)];
//    DEBUG("Led number in array:\t");
//    DEBUG_L(col*8 + i);
  }
}

CRGB * Control::selectCol2(uint8_t col) 
{
  CRGB *leds[NUM_ROWS];
  for (uint8_t row = 0; row < NUM_ROWS; row++) {
    leds[row] = &leds_m[ atRowCol(row,col)]; 
  }
  return *leds;
}

uint8_t Control::atRowCol(uint8_t row, uint8_t col) 
{
  // returns the index of the led at row/col
  uint8_t i;

  if (MatrixSerpentineLayout == false) {
    i = (col * NUM_ROWS) + row;
  }
  
  if (MatrixSerpentineLayout == true) {
    
    if (col & 0x01) {
      //odd columns run backwards
      uint8_t reverseRow = (NUM_ROWS - 1) - row;
      i = (col * NUM_ROWS) + reverseRow;
    } else {
      // Even rows run forwards
      i = (col * NUM_ROWS) + row;
    }
  }
  
  return i; 
}

void addGlitter( fract8 changeOfGlitter = 80)
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void Control::pulseToBeat()
{
  // helper function, called every update to pulse lights to beat
  uint8_t wave_bright = beatsin8(get_BPM(), brightness_m/6, brightness_m);
  leds_m.setBrightness(wave_bright); 
}


/******************************/
/*        PATTERNS            */
/******************************/
// Patterns
void Control::BPM_boogie()
{
  // all strips pulsing at a defined BPM, no ofset
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( this->get_BPM(), 64, 255, 0, 90);
  for( int i = 0; i < nLeds_m; i++) { //9948
    leds_m[i] = ColorFromPalette(palette, hue_m+(i*2), beat/*-hue_m*/+(i*10));
  }
}

void Control::scroll_rows() 
{
  // scrolls through each of the rows to the top, then back down
  fadeToBlackBy( leds_m, NUM_LEDS, 20);
  uint8_t pos = beatsin16( this->get_BPM()/NUM_ROWS, 0, NUM_ROWS);  //rises up rows one row per beat
  for (uint8_t col = 0; col < NUM_COLS; col++){
    CRGB *col_leds[8];
    selectCol(col, col_leds); //col_leds now contains pointers to LEDS
    *col_leds[pos] += CHSV( hue_m+15*col, 255, 192);
  }
}
void Control::rolling_rows()
{
  // scrolls through each of the rows to the top, then starts from the bottom again
  static uint8_t currentRow = NUM_ROWS; //since we pre-increment
  
//  fadeToBlackBy( leds_m, NUM_LEDS, 20);
  //if ( millis() >= timeoutTime + 1000/FPS ) {
    // only run if we are on the beat (give leeway as we only check every once every FPS)
  if (beatNow) {
    // set every time we have a beat
    beatNow = false;
    currentRow = (currentRow + 1) % NUM_ROWS; // select next row and wrap around if at top
    for (uint8_t col = 0; col < NUM_COLS; col++) {
//      CRGB *col_leds[8];
//      selectCol(col, col_leds);
//      *col_leds[currentRow] += CHSV( hue_m+15*col, 255, 192);  
      leds_m[atRowCol(currentRow, col)] += CHSV( hue_m+15*col, 255, 192);  
    }
    
  }
  // fade every led except current row (not efficient, but I guess the fastled code would do this anyway...)
  for (uint8_t row = 0; row < NUM_ROWS; row++) {
    for (uint8_t col = 0; col < NUM_COLS; col++) {
      if ( row != currentRow) {
        leds_m[atRowCol(row,col)].fadeToBlackBy (10);
      }
    }
  }
}
void Control::rolling_rows_diag()
{
  // scrolls through each of the rows to the top, then starts from the bottom again, in a diagnoal
  static uint8_t currentRow = NUM_ROWS; //since we pre-increment
  
//  fadeToBlackBy( leds_m, NUM_LEDS, 20);
  //if ( millis() >= timeoutTime + 1000/FPS ) {
    // only run if we are on the beat (give leeway as we only check every once every FPS)
  if (beatNow) {
    // set every time we have a beat
    beatNow = false;
    currentRow = (currentRow + 1) % NUM_ROWS; // select next row and wrap around if at top
    for (uint8_t col = 0; col < NUM_COLS; col++) {
      CRGB *col_leds[8];
      selectCol(col, col_leds);
      *col_leds[(currentRow+col)%NUM_ROWS] += CHSV( hue_m+15*col, 255, 192);  
    }
    
  }
  // fade every led except current row (not efficient, but I guess the fastled code would do this anyway...)
  for (uint8_t row = 0; row < NUM_ROWS; row++) {
    for (uint8_t col = 0; col < NUM_COLS; col++) {
      if ( ((row+col)%NUM_ROWS) != currentRow) {
//        if (leds_m[atRowCol(row,col)] > CHSV (0,256,brightness_m/5)) {
          leds_m[atRowCol(row,col)].fadeToBlackBy (10);
          
//        }
      }
    }
  }
}

void Control::rainbow()
{
  // Classic rainbow, maybe have the width represent the speed or something?
  uint8_t width = 7; 
  fill_rainbow( leds_m, NUM_LEDS, gHue, width);
  
  // could also have it pulse according to beat 
  pulseToBeat();
}

void Control::confetti()
{
  // randomo coloured speckles that blink in and fade smoothly
  fadeToBlackBy( leds_m, NUM_LEDS, 10);
  uint8_t pos = random16(NUM_LEDS); 
  led[pos] += CHSV( hue_m + random8(64), 200, 255);
}




/******************************/
/*        TAP TEMPO           */
/******************************/
void Control::updateTap() 
{
  /* check for timer timeout */
  if( millis() >= timeoutTime ) {
    /* timeout happened.  clock tick! */
    beatNow = true;
    indicatorTimeout = millis() + 30;  /* this sets the time when LED 13 goes off */
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
//  if ( (currentTimer[0] + currentTimer[1])/2 < 10000) ) {
    tempo_m = (currentTimer[0] + currentTimer[1])/2;
//  }
  DEBUG("\tmsec b/w beats:\t");
  DEBUG(this->get_tempo());
  DEBUG("\tBPM:\t");
  DEBUG_L(this->get_BPM());

  CRGB *row[NUM_COLS];
  this->selectCol(2, row);
  DEBUG("Col");
  #ifdef DEBUG
  for (uint8_t i = 0; i < NUM_COLS; i++){
    DEBUG(i);
    DEBUG(":\tR:"); DEBUG(row[i]->r);
    DEBUG("\tG:");  DEBUG(row[i]->g);
    DEBUG("\tB:");  DEBUG_L(row[i]->b);
  }
  #endif
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

