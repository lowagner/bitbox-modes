// nonsimple.h : a simple frmebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define SQUARE_PIXELS 2     // number of pixels per side of a drawing-square.
                            // may need to use 8 here, if 4 is too much. 

void clear();
extern uint16_t vram[(SCREEN_H-72)/SQUARE_PIXELS][(SCREEN_W-64)/SQUARE_PIXELS];
extern const uint8_t n_x;
extern const uint8_t n_y;

extern uint8_t offset_x; // should not be more than 64.  also should probably be even.
extern uint8_t offset_y; // should not be more than 32.  also probably should make it even.

extern uint8_t bg_color;
extern uint8_t bg_wrap_color;

#endif
