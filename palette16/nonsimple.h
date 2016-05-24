#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

#define SCREEN_W 320 // number of regular pixels
#define SCREEN_H 240

typedef void (void_fn)(void);

void clear_screen();

extern uint8_t superpixel[SCREEN_H][SCREEN_W/2];
extern int graph_debug;
extern int wind_y, wind_x;

extern void_fn* graph_line_callback;

void propagate();

#define BLACK 0
#define GRAY 1
#define WHITE 2
#define PINK 3
#define RED 4
#define BLAZE 5
#define BROWN 6
#define DULLBROWN 7
#define GINGER 8
#define SLIME 9
#define GREEN 10
#define BLUEGREEN 11
#define CLOUDBLUE 12
#define SKYBLUE 13
#define SEABLUE 14
#define INDIGO 15

#endif
