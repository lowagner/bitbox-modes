#include <stdlib.h> // rand
#include "nonsimple.h"

uint16_t vram[(SCREEN_H-32)/SQUARE_PIXELS][(SCREEN_W-64)/SQUARE_PIXELS];
const uint8_t n_x = (SCREEN_W-64)/SQUARE_PIXELS;
const uint8_t n_y = (SCREEN_H-32)/SQUARE_PIXELS;


void game_init()
{
    for (uint8_t j=0; j<n_y-1; ++j)
    for (uint8_t i=0; i<n_x; ++i)
        vram[j][i] = RGB((0xff*j/n_y), 0xff, (0xff*i/n_x));

    vram[n_y-1][0] = RGB(0xff, 0, 0);
    vram[n_y-1][1] = RGB(0xff, 0xff, 0);
    vram[n_y-1][2] = RGB(0xff, 0xff, 0xff);
    for (uint8_t i=3; i<n_x; ++i)
        vram[n_y-1][i] = RGB((0xff), (0xff*(n_x-(i-3))/n_x), 0);
}

void game_frame()
{
}
