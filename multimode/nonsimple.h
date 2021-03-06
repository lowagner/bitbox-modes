// nonsimple.h : a simple frmebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

#define SCREEN_W 640 // number of regular pixels
#define SCREEN_H 480
#define Nx 160      // number of super pixels (4x4 pixels)
#define Ny 120
#define NUM_TEXT_BOXES 2

typedef void (void_fn)(void);
typedef void (void_fn_2int)(int, int);

struct text_box {
    uint8_t x; // be careful with your x, it can't send text offscreen.
    int8_t y; // it's ok to put y offscreen.
    // x and y address by superpixel, which there are 160x120 of them.
    uint16_t info;
    int16_t offset; 
    uint8_t width; // max is 80.  MUST FIT ON SCREEN given the value for x
    uint8_t height; // max is 30.  MUST FIT ON SCREEN given value for y
};

void clear_screen();

extern uint16_t superpixel[Ny][Nx];
extern uint16_t bg_color;
extern uint32_t palette[16];
extern char text[1024];
extern uint8_t text_attr[1024];
extern struct text_box text_box[NUM_TEXT_BOXES];
extern uint8_t cascade;

extern void_fn_2int* graph_line_callback;
extern void_fn_2int* partial_graph_line_callback;

void background_decay(int, int);

void background_color_decay(int, int);

#endif
