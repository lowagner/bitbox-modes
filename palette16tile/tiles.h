#ifndef TILES_H
#define TILES_H

#include "common.h"

extern uint8_t tile_draw[16][16][8];
extern uint8_t tile_map[TILE_MAP_MEMORY];
extern uint32_t tile_info[16]; 
extern uint8_t tile_translator[16];
extern int16_t tile_map_x, tile_map_y;
extern uint16_t tile_map_width, tile_map_height;

void tiles_init();
void tiles_line();
void tiles_reset();

uint32_t pack_tile_info(uint8_t hiding, uint8_t strength, 
    uint8_t translation, uint8_t timing, const SideType *sides);

void unpack_tile_info(uint32_t value, uint8_t *hiding, uint8_t *strength, 
    uint8_t *translation, uint8_t *timing, SideType *sides);

#endif
