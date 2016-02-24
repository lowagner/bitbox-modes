#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#define FASTMEM __attribute__ ((section (".ccm")))

uint8_t v_y[SCREEN_W][MAX_V] FASTMEM;
uint16_t v_color[SCREEN_W][MAX_V] FASTMEM;
uint8_t v_index[SCREEN_W] FASTMEM; 

void graph_frame() 
{
}
// --------------------------------------------------------------

void graph_line() 
{
    if (vga_odd)
        return;

    uint16_t *dst = draw_buffer;

    uint8_t vga8 = (uint8_t) vga_line;
    switch (vga8)
    {
    case 0:
        // need to run all the colors through...
        for (int i=0; i<SCREEN_W; ++i) 
        {
            v_index[i] = 0; // would need to be 1 according to lines 2 through SCREEN_H, but we can be more efficient here and check at line 1 for stuff, too
            *dst++ = v_color[i][0];
        }
        return;
    case 1:
        // need to run all the colors through for double buffering
        for (int i=0; i<SCREEN_W; ++i) 
        {
            if (vga8 >= v_y[i][1])
            {
                *dst++ = v_color[i][1];
                v_index[i] = 3; // encode the fact that we need to double buffer here
            }
            else
                // no need to double buffer this next color
                *dst++ = v_color[i][0];
        }
        return;
    }

    for (int i=0; i<SCREEN_W; ++i)
    {
        if (v_index[i] % 2) // odd guy, may need to double buffer what happened last line
        {
            if (v_index[i]/2 < MAX_V-1 && vga8 >= v_y[i][v_index[i]/2+1])
            {
                // but there's a new color, so don't repeat old color
                *dst = v_color[i][v_index[i]/2+1];
                v_index[i] += 2;
            }
            else
            {
                // there's no old color...
                *dst = v_color[i][v_index[i]/2];
                // make it even, but don't increase the color
                --v_index[i];
            }
        }
        else // odd, no need to re-double what happened
        {
            // but do need to add in a color if necessary
            if (v_index[i]/2 < MAX_V-1 && vga8 >= v_y[i][v_index[i]/2+1])
            {
                *dst = v_color[i][v_index[i]/2+1];
                v_index[i] += 3;
            }

        }
        dst++;
    }
}
// --------------------------------------------------------------
// utilities 

void clear() 
{
}

