// nonsimple.h : a simple framebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"
#define MAX_V 32
#define SCREEN_W 320
#define SCREEN_H 240

extern uint8_t v_y[SCREEN_W][MAX_V];
extern uint16_t v_color[SCREEN_W][MAX_V];
extern uint8_t v_index[SCREEN_W]; 


void clear();

#endif
