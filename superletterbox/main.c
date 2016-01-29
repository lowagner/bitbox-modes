#include <stdlib.h> // rand
#include "nonsimple.h"

void game_init()
{
    offset_y = 0;
    for (int j=0; j<Ny-1; ++j)
    for (int i=0; i<Nx; ++i)
        vram[j][i] = RGB((0xff*j/Ny), 0xff, (0xff*i/Nx));

    vram[Ny-1][0] = RGB(0xff, 0, 0);
    vram[Ny-1][1] = RGB(0xff, 0xff, 0);
    vram[Ny-1][2] = RGB(0xff, 0xff, 0xff);
    for (int i=3; i<Nx; ++i)
        vram[Ny-1][i] = RGB((0xff), (0xff*(Nx-(i-3))/Nx), 0);
}

void game_frame()
{
    kbd_emulate_gamepad();

    if (vga_frame % 2)
    {
        if (GAMEPAD_PRESSED(0, up))
        {
            if (offset_y > 0)
                --offset_y;
        }
        else if (GAMEPAD_PRESSED(0, down))
        {
            if (offset_y < SCREEN_H - Ny - 1)
                ++offset_y;
        }
    }
}
