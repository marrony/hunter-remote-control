#include "Hunter.h"

constexpr int8_t interruptPin = 2;

HunterReceiver receiver;

void setup() {
  Serial.begin(9600);
  pinMode(interruptPin, INPUT);

  receiver.enableReceive(interruptPin);
}

void loop() {
  if (receiver.available()) {
    Command cmd = receiver.getCommand();
    Serial.print(cmd.code0, HEX); Serial.print(" ");
    Serial.print(cmd.code1, HEX); Serial.print(" ");
    Serial.print(cmd.code2, HEX); Serial.println();

    receiver.resetAvailable();
  }
}

