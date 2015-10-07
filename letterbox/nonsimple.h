// nonsimple.h : a simple frmebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

#define SCREEN_W 320
#define SCREEN_H 200

#define SQUARE_PIXELS 4     // number of pixels per side of a drawing-square.
                            // may need to use 8 here, if 4 is too much. 
#define OFFSET_X 32 // should not be more than 64.  also should probably be even.
#define OFFSET_Y 2 // should not be more than 32.  also probably should make it even.

void clear();
extern uint16_t vram[(SCREEN_H-32)/SQUARE_PIXELS][(SCREEN_W-64)/SQUARE_PIXELS];

#endif
