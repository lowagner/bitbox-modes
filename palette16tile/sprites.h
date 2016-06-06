#ifndef SPRITES_H
#define SPRITES_H

#include "nonsimple.h"

// break objects up into 16x16 tiles:
extern uint8_t sprite_draw[16][8][16][8]; 

struct object {
    // first 32 bits:
    int16_t y, x;
    // second
    int16_t iy, ix;
    // third 32 bits
    uint8_t draw_index;
    uint8_t z;
    uint8_t invisible_color;
    uint8_t properties;
    // fourth
    uint8_t sprite_index;
    uint8_t sprite_frame;
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
void make_unseen_object_viewable(int i);
int on_screen(int16_t x, int16_t y);
void update_objects();

void sprite_line();
void sprite_frame();

void sprites_reset();

#endif
