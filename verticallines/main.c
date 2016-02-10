#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"

#include <string.h> // memset

void game_init()
{ 
    for (int i=0; i<SCREEN_W; ++i)
    {
        v[i][0].color = 0;
        v[i][0].y = 0;
        for (int j=1; j<MAX_V; ++j)
        {
            v[i][j].color = rand()%256;
            v[i][j].y = v[i][j-1].y + 1 + rand()%16;
        }
    }
    clear();
}

void game_frame()
{
    kbd_emulate_gamepad();
}
