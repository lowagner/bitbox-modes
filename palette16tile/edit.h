#ifndef EDIT_H
#define EDIT_H

#include <stdint.h>

extern uint8_t edit_color;
extern uint8_t edit_tile, edit_sprite;
extern uint8_t edit_sprite_not_tile;

void edit_tile_line();
void edit_tile_controls();

#endif