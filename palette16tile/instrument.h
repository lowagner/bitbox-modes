#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <stdint.h> // uint

extern const uint8_t note_name[12][2];
extern uint8_t instrument_i;

void instrument_init();
void instrument_reset();
void instrument_controls();
void instrument_line();

#endif
