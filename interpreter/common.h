#ifndef COMMON_H
#define COMMON_H
#include "stdint.h"
#include "simple.h"

struct cursor {
    uint8_t y, x;
    uint8_t attr, saved_attr;
};

extern struct cursor cursor;

void place_cursor(uint8_t x, uint8_t y);
void move_cursor(uint8_t x, uint8_t y);
void move_cursor_to_x(uint8_t x);
void move_cursor_to_y(uint8_t y);
void move_cursor_y(uint8_t direction);
void move_cursor_up();
void move_cursor_down();
void move_cursor_x(uint8_t direction);
void move_cursor_right();
void move_cursor_left();


#endif
