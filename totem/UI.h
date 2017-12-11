#ifndef UI_H
#define UI_H

#include "Control.h"

#define NUM_BUTTONS       4
#define UI_LEDS   3

//TODO
// VISUAL FEEDBACK FOR MODES
// EXTRA MODES (e.g. hue speed select, single hue select)
// refactor button handle code (single handleButton fn that takes (button pressed and press type) and calls relevant fns)

// LCD display
/*
  
*/
#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 7, 6, 5, 4, 3, 2);
// LCD screen updates are called after each button press


// Enum definitions
enum UIState_t : uint8_t {pattern, brightness, hue, manual /*, rhythm*/}; 
const uint8_t UINumModes = 4;
enum buttonPress_t : uint8_t {shortPress, longPress};
struct UIDescriptions_s { int index; String text; };
UIDescriptions_s UIDescriptions[UINumModes] = 
//Custom array, takes UiMode and display string (could F(0) string to save space)
//two variables, uimode index, ui display text (printed on line 0)
{
 {0, "Pattern Sel:"},
 {1, "Brightness: "},
 {2, "Hue Speed:  "},
 {3, "Manual mode:"}/*,
 {4, "Rhythm input:"}*/
};

// Generalised class for handling user interface
class UI
{
 // take a copy of Control when constructing as needs to edit values in it
 public:
    // Pass a reference to Control for constructor as need to access members. 
    UI(Control *c);
   
    void setupUI();
    void handleUI();
    void renderUI();      //potentially use LCD display or OSC messages
 private:
    Control *Control_m; 
    UIState_t UIState;

    //lcd display funtions (displays pattern, brightness, hue speed, etc)
    void LCDDisplay();
    
    // Functions for controlling UI state
    void toggleButton(buttonPress_t b);
      // Changes UIState. button press can be shortPress or longPress
      // inputPins[0]
    void incButton(buttonPress_t b);
      // Increments either pattern, brightness scale or speed depending on UIState
      // inputPins[1]
    void decButton(buttonPress_t b);
      // Decrements either pattern, brightness scale or speed depending on UIState
      // inputPins[2]
    void fnButton(buttonPress_t b);
      // Used for tap tempo, possibly other fns... (See below)
      // inputPins[3]

    void (UI::*buttonFunctions[NUM_BUTTONS])(buttonPress_t b) = {&toggleButton, &incButton, &decButton, &fnButton};
      // pointer to each of the button functions (each take a buttonPress variable (shortPress/longPress))

      
    // Pin definitions
    const uint8_t outputPins[UI_LEDS] = {11,12,13};     // output pins for feedback of UIState: pattern, brightness, hue
    const uint8_t inputPins[NUM_BUTTONS] = {16,14,15,10};   //In order: toggle, inc, dec, fn
    
    // Button related declarations
    unsigned long lastDebounceTime[NUM_BUTTONS];  //used to debounce buttons. last time pin output toggled
    uint8_t lastPinValue[NUM_BUTTONS];      // last button state
    uint8_t buttonState[NUM_BUTTONS];       // current button state
    
    const unsigned short debounceDelay = 20;
    const unsigned short holdTime[NUM_BUTTONS] = {2000,600,600,600};
    const unsigned short repeatRate[NUM_BUTTONS] = {4000,200,200,4000}; //repeat rate different for different buttons
    // TODO, repeatRate increases if you hold down for longer
    bool trigger[NUM_BUTTONS] = {true, true, true, true};
    unsigned long lastTriggered[NUM_BUTTONS] = {0,0,0,0};
};

#endif /* UI_H */

UI::UI(Control *c)
  : UIState(pattern)
{
  Control_m = c;
  
  unsigned long lastDebounceTime[NUM_BUTTONS] = {0,0,0,0};  //used to debounce buttons. last time pin output toggled
  unsigned short debounceDelay = 20;

  uint8_t lastPinValue[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH}; // pills are pulled high
  uint8_t buttonState[NUM_BUTTONS] = {HIGH, HIGH, HIGH, HIGH};
}

void UI::setupUI()
{
  // setup input buttons
  for (uint8_t i = 0; i < NUM_BUTTONS; i++)
  {
    pinMode(inputPins[i], INPUT_PULLUP);
  } 
  
  // setup output leds 
  for (uint8_t i = 0; i < UI_LEDS; i++)
  {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW);
  }

  // Setup LCD panel 
  // RS, En, D4, D5, D6, D7 (Rw to ground)
//  LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
  lcd.begin(16, 2);
  delay(50);
  lcd.print("Setup complete!");
  delay(500);
  LCDDisplay();
  

  DEBUG("Pattern Name: ");
  DEBUG_L(Control_m->getPatternName());
  
}

void UI::handleUI()
{
  // Includes jitter protection (i.e. won't register button repress if within 20msec)
  // Inc and dec buttons keep firing after a period of time (700msec, sends button down every 200msec?)

  // Hangle Output indictor lights
  for (uint8_t i = 0; i < UI_LEDS; i++)
  {
    if (UIState == i){
      digitalWrite(outputPins[i], HIGH);  
    }
    else {
      digitalWrite(outputPins[i], LOW);
    }
  }

  // handle button presses for each button
  for (uint8_t i = 0; i < NUM_BUTTONS; i++)
  {
    // Looking for switch that goes low as we have pullup resistors on
    uint8_t reading = digitalRead(inputPins[i]);

    // If the switch changed, due to noise or pressing
    if ( reading != lastPinValue[i] )
    {
      // reset debouncing timer
      lastDebounceTime[i] = millis();
    }

    if ( (millis() - lastDebounceTime[i]) > debounceDelay )
    {
      // Whatever the reading is it, it's been there for longer than debounce delay
      // So take it as current state

      // if button has changed state
      if (reading != buttonState[i]) 
      {
        buttonState[i] = reading; 
        // if button has changed state to LOW reading
        if (reading == LOW)
        { // triger short press
          (this->*buttonFunctions[i])(shortPress);
        }
      }
      // This triggers after a button has been low for > 700msec
      if ( (reading == LOW) and (( millis() - lastDebounceTime[i] ) > holdTime[i]) )
      {
        // If trigger is true, repeat function inc
        if (trigger[i])
        {
          (this->*buttonFunctions[i])(longPress);
          trigger[i] = false;
          lastTriggered[i] = millis();
        }
        // this checks if we have gone past repeatrate
        if ( (millis() - lastTriggered[i]) > repeatRate[i])
        {
          trigger[i] = true; 
        }
      }
    }
    lastPinValue[i] = reading; // save the reading
  }

//  updateTap();
//  renderUI();
}

void UI::LCDDisplay() 
{
  lcd.clear();
  lcd.print(UIDescriptions[UIState].text);
  lcd.setCursor(0,1); 
  float bright = Control_m->getBrightness();
  bright = bright * 0.39216;
  switch(UIState) {
    case pattern :
      lcd.print( String::String("> " + Control_m->getPatternName()) );
      break;
    case brightness :
      //max brightness is 255
      lcd.print( String::String("> " + String::String(bright, 1) + "%") );
      break;
    case hue :
      lcd.print( String::String("> " + Control_m->getHueSpeed() + " msec/d"));
      break;
    case manual :
      break;
  }
}

void UI::toggleButton(buttonPress_t p)
{
  DEBUG("Toggle button Pressed\n\tUI State: ");
  // Short press
  if (p == shortPress)
  {
    // change UIState
    switch(UIState) {
      // LEDS A (pattern) LED A (brightness) LED A Speed
      case pattern : 
        UIState = brightness;   
        DEBUG_L("Brightness");
        break;
      case brightness :
        UIState = hue;        
        DEBUG_L("Speed");
        break;
      case hue : 
        UIState = manual;      
        DEBUG_L("Manual");
        break;
      case manual :
        UIState = pattern;
        DEBUG_L("Pattern");
        break;
    }
//    lcd.clear();
//    lcd.print(UIDescriptions[UIState].text);
//    LCDDisplayValues();
  }
  // Long press ??
  // Possible modes: pattern (default), brightness, speed. Inditacted by lights
  if (p == longPress)
  {
    // do something fun. Maybe default/reset/lasers on&off?
    DEBUG("\t(long press)");
  }
  
  LCDDisplay();
}

void UI::incButton(buttonPress_t p)
{
  DEBUG("Inc button Pressed");
  if (p == shortPress)
  {
//    Serial.println("\t(Short press)");
    switch(UIState){
    case pattern :
      // increment pattern
      DEBUG_L("\t(inc pattern)");
      Control_m->inc_pattern();
      break;
    case brightness :
      // inc brightness
      DEBUG_L("\t(inc brightness)");
      Control_m->incBrightness();
      DEBUG("Brightness:\t");
      DEBUG_L(Control_m->getBrightness());
      break;
    case hue :
      // inc speed
      DEBUG_L("\t(inc speed)");
      Control_m->incHueSpeed();
      break;
    case manual :
      //
      break;
    }
    
  } else if (p == longPress)
  {
    // long press
    DEBUG_L("\t(Long press)");
    uint8_t multiplier = 1;
    switch(UIState){
    case pattern :
      // increment pattern
      DEBUG_L("\t(inc pattern)");
      //Control_m->incPattern();
      break;
    case brightness :
      // inc brightness
      DEBUG_L("\t(inc brightness)");
      // speed up inc if held for longer
      if (( millis() - lastDebounceTime[1] ) > 2000)       {multiplier = 2;} 
      Control_m->incBrightness(5*multiplier);
      DEBUG("Brightness:\t");
      DEBUG_L(Control_m->getBrightness());
      break;
    case hue :
      // inc speed
      DEBUG_L("\t(inc hue speed)");
      Control_m->incHueSpeed();
      break;
    case manual :
      // manual mode
      break;
    }
  }
  // update LCD values
  LCDDisplay();
}

void UI::decButton(buttonPress_t p)
{
  DEBUG("Dec button");
  if (p == shortPress)
  {
//    Serial.println("\t(Short press)");
    switch(UIState){
    case pattern :
      // decrement pattern
      DEBUG_L("\t(dec pattern)");
      Control_m->dec_pattern();
      break;
    case brightness :
      // dec brightness
      DEBUG_L("\t(dec brightness)");
      Control_m->decBrightness();
      DEBUG("Brightness:\t");
      DEBUG_L(Control_m->getBrightness());
      break;
    case hue :
      // dec hue speed
      DEBUG_L("\t(dec speed)");
      Control_m->decHueSpeed();
      break;
    case manual :
      // manual mode
      break;
    }
  } else if (p == longPress)
  {
    // long press
    DEBUG_L("\t(Long press)");
    uint8_t multiplier = 1;
    switch(UIState){
      case pattern :
        // decrement pattern
        DEBUG_L("\t(inc pattern)");
        //Control_m->decPattern();
        break;
      case brightness :
        // dec brightness
        DEBUG_L("\t(dec brightness)");
        // speed up decrement if button held for longer
        if (( millis() - lastDebounceTime[2] ) > 2000)       {multiplier = 2;} 
        Control_m->decBrightness(5*multiplier);
        DEBUG("Brightness:\t");
        DEBUG_L(Control_m->getBrightness());
        break;
      case hue :
        // inc hue speed
        DEBUG_L("\t(dec hue speed)");
        Control_m->decHueSpeed();
        break;
      case manual :
        // manual mode
        break;
    }
  }
  // update lcd values
  LCDDisplay();
}

void UI::fnButton(buttonPress_t p)
{
  if (p == shortPress)
  {
    DEBUG_L("Function Button (Short press)");
    
    switch(UIState){
      case pattern :
        // temporary flash pattern while held down?
//        break;
      case brightness :
        // ?change max brightness
//        break;
      case hue :
        // tap tempo
        //tap();
        Control_m->tap();
        break;
      case manual :
        // Manual mode
        // fx hit
        break;
    }
  } else if (p == longPress)
  {
    DEBUG_L("Function Button (Long press)");
    switch(UIState){
      case pattern :
        // temporary flash pattern while held down?
//        break;
      case brightness :
        // ?change max brightness
//        break;
      case hue :
        // tap tempo
        //tap();
        Control_m->tap_toggle();
        break;
      case manual :
        // Manual mode
        // fx hit
        break;
    }
  }
}


void UI::renderUI() {
  //potentially output to OSC
    
}

