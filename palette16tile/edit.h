#ifndef EDIT_H
#define EDIT_H

#include <stdint.h>

extern uint8_t edit_color;
extern uint8_t edit_tile, edit_sprite;
void edit_tile_line();
void edit_sprite_line();
void edit_tile_controls();

extern uint8_t edit_spot;
extern uint8_t edit_color;

#endif
