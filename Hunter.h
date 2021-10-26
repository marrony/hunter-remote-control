#ifndef HUNTER_H
#define HUNTER_H

#include "Arduino.h"

void hunter_enable_receive(int8_t interrupt_pin);
void hunter_disable_receive();
void hunter_wait_two_gaps();

bool hunter_available(uint32_t* codes);
void hunter_reset();

void hunter_send_command(int8_t tx_pin, const uint32_t* code);

void hunter_debug(Stream& serial);

#endif //HUNTER_H

