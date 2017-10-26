#include "Control.h"
#include "UI.h"



CRGB leds[NUM_LEDS];
Control Control(leds, NUM_LEDS);
UI ui(&Control);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  
  Control.setupControl();
  ui.setupUI();
  Serial.println("Setup Complete");
}

void loop() {
  // put your main code here, to run repeatedly:
  Control.handleControl();
  ui.handleUI();
}
