#include "Hunter.h"

#define MAX_CHANGES 132
#define PULSE_LONG  850
#define PULSE_SHORT 350

#define STATE_LONG_GAP0 0
#define STATE_LONG_GAP1 1
#define STATE_SYNC      2
#define STATE_SHORT_GAP 3
#define STATE_DATA      4
#define STATE_AVAILABLE 5

static inline int16_t diff(long a, long b) {
  return abs(a - b);
}

static inline void write(int pin, int value, int micros) {
  digitalWrite(pin, value);
  delayMicroseconds(micros - 3);
}

static void send_sync_code(int pin) {
  for (int i = 0; i < 11; i++) {
    write(pin, HIGH, 400);
    write(pin, LOW, 400);
  }

  write(pin, HIGH, 400);
  write(pin, LOW, 5200);
}

static void send_code(int pin, uint32_t code) {
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

static int8_t _interrupt_pin = -1;

static volatile uint8_t _start_state = STATE_LONG_GAP1;
static volatile uint8_t _state = _start_state;
static volatile uint16_t _change_count = 0;
static volatile long _last_time = 0;
static volatile long _last_duration = 0;
static volatile uint32_t _codes[3];

//this is for debuging purposes
static long _timings[MAX_CHANGES];

static void interrupt_handler();

void hunter_enable_receive(int8_t interrupt_pin) {
  _interrupt_pin = interrupt_pin;
  if (_interrupt_pin != -1) {
    attachInterrupt(digitalPinToInterrupt(_interrupt_pin), interrupt_handler, CHANGE);
  }
}

void hunter_disable_receive() {
  detachInterrupt(digitalPinToInterrupt(_interrupt_pin));
}

void hunter_wait_two_gaps() {
  _start_state = STATE_LONG_GAP0;
}

bool hunter_available(uint32_t* codes) {
  if (_state == STATE_AVAILABLE) {
    codes[0] = _codes[0];
    codes[1] = _codes[1];
    codes[2] = _codes[2];
    return true;
  } else {
    return false;
  }
}

void hunter_reset() {
  _state = _start_state;
  _change_count = 0;
}

static void interrupt_handler() {
  long time = micros();
  long duration = time - _last_time;

  switch (_state) {
    case STATE_LONG_GAP0:
      if (diff(duration, 25000) < 2000) {
        _state = STATE_LONG_GAP1;
      }

      break;

    case STATE_LONG_GAP1:
      if (diff(duration, 25000) < 2000) {
        _state = STATE_SYNC;
        _change_count = 0;
      }

      break;

    case STATE_SYNC:
      _change_count++;

      if (_change_count % 2 == 0) {
        if (diff(_last_duration + duration, 800) > 300) {
          _state = _start_state;
          _change_count = 0;
        }
      }

      if (_change_count >= 23) {
        if (diff(duration, 400) > 300) {
          _state = _start_state;
        } else {
          _state = STATE_SHORT_GAP;
        }
      }

      break;

    case STATE_SHORT_GAP:
      if (diff(duration, 5200) > 500) {
        _state = _start_state;
      } else {
        _state = STATE_DATA;
        _change_count = 0;
      }

      break;

    case STATE_DATA:
      _timings[_change_count++] = duration;

      if (_change_count % 2 == 0) {
        if (duration > 1200) {
          duration = diff(_last_duration, 1200);
          _timings[_change_count - 1] = duration;
        }

        if (diff(_last_duration + duration, 1200) > 1000) {
          _state = _start_state;
        } else {
          int index = (_change_count - 1) / 44;

          _codes[index] <<= 1;
          if (_last_duration > duration) {
            _codes[index] |= 1;
          }
        }
      }

      if (_change_count >= MAX_CHANGES) {
        _codes[0] &= 0x3FFFFF;
        _codes[1] &= 0x3FFFFF;
        _codes[2] &= 0x3FFFFF;
        _state = STATE_AVAILABLE;
      }

      break;

    case STATE_AVAILABLE:
      //keep in this state until reset
      break;
  }

  _last_duration = duration;
  _last_time = time;
}

void hunter_send_command(int8_t tx_pin, const uint32_t* code) {
  send_sync_code(tx_pin);
  send_code(tx_pin, code[0]);
  send_code(tx_pin, code[1]);
  send_code(tx_pin, code[2]);
}

void hunter_debug(Stream& serial) {
  serial.print("Timings: ");
  serial.print(5200);
  serial.print(",");

  for (int i = 0; i < MAX_CHANGES; i++) {
    serial.print(_timings[i]);
    serial.print(",");
  }

  serial.println();

  serial.print("Codes: ");
  serial.print("0x"); serial.print(_codes[0], HEX); serial.print(", ");
  serial.print("0x"); serial.print(_codes[1], HEX); serial.print(", ");
  serial.print("0x"); serial.println(_codes[2], HEX);
}

