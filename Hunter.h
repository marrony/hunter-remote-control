#ifndef HUNTER_H
#define HUNTER_H

#include "Arduino.h"

#define MAX_CHANGES 156

struct Command {
  uint32_t code0;
  uint32_t code1;
  uint32_t code2;
};

struct Decoder {
  static bool decodeCode(const long* timings, uint32_t* out);
  static bool decodeTimings(const long* timings, Command* out);
};

class HunterReceiver {
public:
  HunterReceiver();

  bool available() const;
  void resetAvailable();

  Command getCommand() const;

  void enableReceive(int8_t interruptPin);
  void disableReceive();

  void interruptHandler();

  void debug(Stream& serial) const;
private:
  uint16_t _changeCount;
  int8_t _interruptPin;
  bool _active;
  bool _available;

  long _lastTime;
  long _timings[MAX_CHANGES];
  Command _cmd;

  void enableReceive();
};

class HunterSender {
public:
  HunterSender(int8_t pin);

  void sendCommand(const Command& cmd) const;

private:
  int8_t _pin;
};

void debug(Stream& serial, const long* timings, const Command* cmd);

#endif //HUNTER_H

