// nonsimple.h : a simple framebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"
#define MAX_V 32
#define SCREEN_W 320
#define SCREEN_H 240

struct vertical {
    // first 16 bits:
    uint8_t y;
    uint8_t color;
};

extern struct vertical v[SCREEN_W][MAX_V];
extern uint8_t v_index[SCREEN_W]; 
extern uint16_t palette[256]; 


void clear();

#endif
