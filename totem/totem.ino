#include "Control.h"
#include "UI.h"



CRGB leds[NUM_LEDS];
Control Control(leds, NUM_LEDS);
UI ui(&Control);

void setup() {
  // put your setup code here, to run once:
//  #ifdef DEBUG
//    Serial.begin(115200);
//    while (!Serial);
//  #endif
  
  Control.setupControl();
  ui.setupUI();
  
//  DEBUG_L("Setup Complete");
}

void loop() {
  // put your main code here, to run repeatedly:
  Control.handleControl();
  ui.handleUI();
}
