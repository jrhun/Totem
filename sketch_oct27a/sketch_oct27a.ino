void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);
  uint8_t a = 214;
  Serial.print(min(a + 5, 255));
}

void loop() {
  // put your main code here, to run repeatedly:

}
