// nonsimple.h : a simple framebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"
#define TILE_MAP_MEMORY 19200
#define SCREEN_W 320
#define SCREEN_H 240

extern uint16_t bg_color;
extern uint8_t tile_map[TILE_MAP_MEMORY];
extern uint8_t tile_translator[16];
extern uint16_t tile_draw[16][16][16];
extern uint16_t tile_map_x, tile_map_y;
extern uint16_t tile_map_width, tile_map_height;

void clear();

#endif
