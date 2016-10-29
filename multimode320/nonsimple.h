// nonsimple.h : a simple frmebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

#define SCREEN_W 320 // number of regular pixels
#define SCREEN_H 240
#define Nx 160      // number of super pixels (2x2 pixels)
#define Ny 120
#define MAX_BOXES 8

typedef void (void_fn)(void);
typedef void (void_fn_2int)(int, int);

struct box {
    uint8_t x; // be careful with your x, it can't send text offscreen.
    uint8_t next; // singly linked active drawing list
    int16_t y; // it's ok to put y offscreen.
    // x addresses by superpixel (160 of them)
    // and y addresses by regular pixel (240 of those)
    int16_t offset; 
    uint8_t width; // max is 40.  x + 8*width < SCREEN_W or segfault!
    uint8_t height; // max is 30.
};

void clear_screen();

extern uint16_t superpixel[Ny][Nx];
extern uint16_t bg_color;
extern uint32_t palette[16];
extern char text[1024];
extern uint8_t text_attr[1024];
extern struct box box[MAX_BOXES];
extern int box_count;
extern uint8_t cascade;

extern void_fn_2int* graph_line_callback;
extern void_fn_2int* partial_graph_line_callback;

void background_decay(int, int);

void background_color_decay(int, int);

#endif
