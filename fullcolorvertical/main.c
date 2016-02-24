#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"

#include <string.h> // memset

void game_init()
{ 
    for (int i=0; i<SCREEN_W; ++i)
    {
        v_color[i][0] = RGB((255+i)%255, (255-i)%255, 50);
        v_y[i][0] = 0;
        for (int j=1; j<MAX_V; ++j)
        {
            v_color[i][j] = RGB(rand()%256, rand()%256, rand()%256);
            v_y[i][j] = v_y[i][j-1] + 1 + rand()%16;
        }
    }
}

void game_frame()
{
    kbd_emulate_gamepad();
}
