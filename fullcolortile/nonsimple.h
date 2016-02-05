// nonsimple.h : a simple framebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"
#define TILE_MAP_MEMORY 19200
#define MAX_OBJECTS 64 // no more than 255
#define SCREEN_W 320
#define SCREEN_H 240

extern uint16_t bg_color;
extern uint8_t tile_map[TILE_MAP_MEMORY];
extern uint8_t tile_translator[16];
//extern uint16_t tile_properties[16];
extern uint16_t tile_draw[16][16][16];
extern int16_t tile_map_x, tile_map_y;
extern uint16_t tile_map_width, tile_map_height;


// break objects up into 16x16 tiles:
extern uint16_t sprite_frames[16][8][16][16]; 
extern uint16_t sprite_draw[16][16][16];
struct object {
    // first 32 bits:
    int16_t y, x;
    // second
    int16_t iy, ix;
    // third 32 bits
    uint8_t draw_index;
    uint8_t z;
    uint8_t destroy_me;
    uint8_t properties;
    // fourth
    uint8_t sprite_frame;
    uint8_t sprite_index;
    uint8_t next_free_object; // for creating new objects
    uint8_t next_used_object; // for keeping track of objects
};

extern uint8_t draw_order[MAX_OBJECTS];

extern struct object object[MAX_OBJECTS];
extern uint8_t first_free_object;
extern uint8_t first_used_object;
extern uint8_t object_count; // keep track of how many objects you have
// keep the object list sorted in increasing y, 
// and break ties by bringing to the front lower z:
extern uint8_t drawing_count; // keep track of how many objects you can see
// since every sprite is the same size, 
// we just need to keep a first and last active index
extern uint8_t first_drawing_index; 
extern uint8_t last_drawing_index; 

int create_object(int sprite_draw_index, int16_t x, int16_t y, uint8_t z);
void move_object(int i, int16_t x, int16_t y);
void update_objects();

void clear();

#endif
