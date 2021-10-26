#include <Hunter.h>

const byte interruptPin = 2;

void setup() {
  Serial.begin(9600);
  pinMode(interruptPin, INPUT);

  hunter_enable_receive(interruptPin);
//  hunter_wait_two_gaps();
}

void loop() {
  uint32_t code[3];

  if (hunter_available(code)) {
    Serial.print("Codes: 0x");
    Serial.print(code[0], HEX); Serial.print(", 0x");
    Serial.print(code[1], HEX); Serial.print(", 0x");
    Serial.print(code[2], HEX); Serial.println();

    //hunter_debug(Serial);
    hunter_reset();
  }
}

