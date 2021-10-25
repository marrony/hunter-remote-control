#include "Hunter.h"

#define PULSE_LONG 850
#define PULSE_SHORT 350

static inline int16_t diff(long a, long b) {
  return abs(a - b);
}

bool Decoder::decodeCode(const long* timings, uint32_t* out) {
  uint32_t code = 0;
  for (uint16_t i = 0; i < 44; i += 2) {
    code <<= 1;

    long s = timings[i] + timings[i+1];
    long p0 = (timings[i] * 100 + s - 1) / s;
    long p1 = s - p0;

    if (p0 > 50) {
      code |= 1;
    } else if (p1 > 50) {
      code |= 0;
    } else {
      return false;
    }
  }

  *out = code;
  return true;
}

bool Decoder::decodeTimings(const long* timings, Command* cmd) {
  //12 sync pulses
  for (int i = 0; i < 11*2; i += 2) {
    long s = timings[i] + timings[i+1];

    if (diff(s, 800) > 100) {
      return false;
    }
  }

  if (diff(timings[22], 300) > 100) {
    return false;
  }

  //5ms gap
  if (diff(timings[23], 5200) > 200) {
    return false;
  }

  for (int i = 24; i < 156; i += 2) {
    long s = timings[i] + timings[i+1];

    if (i < 154 && diff(s, 1200) > 200) {
      return false;
    }
  }

  if (!decodeCode(timings+24, &cmd->code0)) {
    return false;
  }

  if (!decodeCode(timings+68, &cmd->code1)) {
    return false;
  }

  if (!decodeCode(timings+112, &cmd->code2)) {
    return false;
  }

  return true;
}

static inline void write(int pin, int value, int micros) {
  digitalWrite(pin, value);
  delayMicroseconds(micros - 3);
}

static void sendSyncCode(int pin) {
  for (int i = 0; i < 11; i++) {
    write(pin, HIGH, 300);
    write(pin, LOW, 500);
  }

  write(pin, HIGH, 300);
  write(pin, LOW, 5292);
}

static void sendCode(int pin, uint32_t code) {
  for (int i = 21; i >= 0; i--) {
    if (((code >> i) & 0x1) != 0) {
      write(pin, HIGH, PULSE_LONG);
      write(pin, LOW, PULSE_SHORT);
    } else {
      write(pin, HIGH, PULSE_SHORT);
      write(pin, LOW, PULSE_LONG);
    }
  }
}

static HunterReceiver* thisHunter = nullptr;

HunterReceiver::HunterReceiver()
  : _changeCount(0), _interruptPin(-1), _active(false), _available(false), _lastTime(0) {
  thisHunter = this;
}

bool HunterReceiver::available() const {
  return _available;
}

void HunterReceiver::resetAvailable() {
  _available = false;
  _cmd = {0, 0, 0};
}

static void _interruptHandler() {
  thisHunter->interruptHandler();
}

void HunterReceiver::enableReceive() {
  if (_interruptPin != -1) {
    attachInterrupt(digitalPinToInterrupt(_interruptPin), _interruptHandler, CHANGE);
  }
}

Command HunterReceiver::getCommand() const {
  return _cmd;
}

void HunterReceiver::enableReceive(int8_t interruptPin) {
  _interruptPin = interruptPin;
  enableReceive();
}

void HunterReceiver::disableReceive() {
  detachInterrupt(digitalPinToInterrupt(_interruptPin));
}

void HunterReceiver::interruptHandler() {
  const long time = micros();

  const long duration = time - _lastTime;

  if (_active) {
    _timings[_changeCount++] = duration;

    if (_changeCount >= MAX_CHANGES) {
      if (_timings[MAX_CHANGES - 1] > 1200) {
        _timings[MAX_CHANGES - 1] = diff(_timings[MAX_CHANGES - 2], 1200);
      }

      if (Decoder::decodeTimings(_timings, &_cmd)) {
        _available = true;
      }

      _active = false;
      _changeCount = 0;
    }
  }

  if (diff(duration, 25000) <= 2000) {
    _active = true;
    _changeCount = 0;
  }

  _lastTime = time;
}

void HunterReceiver::debug(Stream& serial) const {
  ::debug(serial, _timings, &_cmd);
}

HunterSender::HunterSender(int8_t pin)
: _pin(pin)
{ }

void HunterSender::sendCommand(const Command& cmd) const {
  sendSyncCode(_pin);
  sendCode(_pin, cmd.code0);
  sendCode(_pin, cmd.code1);
  sendCode(_pin, cmd.code2);
}

void debug(Stream& serial, const long* timings, const Command* cmd) {
  serial.print("Timings: ");
  serial.print(25000);
  serial.print(",");

  for (int i = 0; i < MAX_CHANGES; i++) {
    serial.print(timings[i]);
    serial.print(",");
  }

  serial.println();

  serial.print("Codes: ");
  serial.print("0x"); serial.print(cmd->code0, HEX); serial.print(", ");
  serial.print("0x"); serial.print(cmd->code1, HEX); serial.print(", ");
  serial.print("0x"); serial.println(cmd->code2, HEX);
}

