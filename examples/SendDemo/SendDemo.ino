#include <Hunter.h>

constexpr int8_t ledPin = 13;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
}

const Command toggleLight = {0x3FFEC2, 0x2A8AF0, 0x201DFE}; //10 0000 0001 1101 1110
const Command offFan      = {0x3FFEC2, 0x2A8AF0, 0x045FBA}; //00 0100 1111 1011 1010
const Command minFan      = {0x3FFEC2, 0x2A8AF0, 0x005FFA}; //00 0101 1111 1111 1010
const Command midFan      = {0x3FFEC2, 0x2A8AF0, 0x041FBE}; //00 0100 1111 1011 1110
const Command maxFan      = {0x3FFEC2, 0x2A8AF0, 0x081F7E}; //00 1000 0001 0111 1110

void loop() {
  Serial.println("Sending ...");
  hunter_send_command(ledPin, toggleLight);
  Serial.println("Reset your Arduino to send again.");

  while (1) {}
}

