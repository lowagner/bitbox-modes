#include <stdlib.h> // rand
#include "bitbox.h"
#include "nonsimple.h"

void game_init()
{
    offset_x = 10;
    offset_y = 10;

    bg_color = 0;
    bg_wrap_color = 10;

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
    kbd_emulate_gamepad();

    if (vga_frame % 2 == 1)
    {
        if (GAMEPAD_PRESSED(0, left))
        {
            if (offset_x > SQUARE_PIXELS)
                --offset_x;
        }
        else if (GAMEPAD_PRESSED(0, right))
        {
            if (offset_x < 64 - SQUARE_PIXELS)
                ++offset_x;
        }
        if (GAMEPAD_PRESSED(0, up))
        {
            if (offset_y > SQUARE_PIXELS)
                --offset_y;
        }
        else if (GAMEPAD_PRESSED(0, down))
        {
            if (offset_y < 72 - SQUARE_PIXELS)
                ++offset_y;
        }
    } 
}
