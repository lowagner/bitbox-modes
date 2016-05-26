#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"
#include "fill.h"
#include "io.h"
#include <stdio.h>
#include <string.h>

int player_x, player_y, player_c, original_c, player_s;

void game_init()
{ 
    player_x = 0;
    player_y = 0;
    player_c = 1;
    original_c = 0;
    player_s = 3;
    clear_screen();
    set_color(player_x, player_y, player_c);
    int k=0;
    for (int i=0; i<SCREEN_W/16; ++i) 
    for (int j=0; j<SCREEN_H/16; ++j)
        draw_box(i*16, j*16, 16, 16, k++);
    io_init();
}

void game_frame()
{
    static int previous_picture = 0;

    kbd_emulate_gamepad();

    if (vga_frame % player_s == 0) 
    {
        if (GAMEPAD_PRESSED(0, B))
        {
            ++player_c;
            if (player_c > 15)
                player_c = 0;
            message("player c = %d\n", player_c);
        }
        if (GAMEPAD_PRESSED(0, Y))
        {
            --player_c;
            if (player_c < 0)
                player_c = 15;
            message("player c = %d\n", player_c);
        }

        if (GAMEPAD_PRESSED(0, up)) 
        {
            --player_y;
            if (player_y < 0)
                player_y = SCREEN_H-1;
            original_c = get_color(player_x, player_y);
        }
        if (GAMEPAD_PRESSED(0, down)) 
        {
            ++player_y;
            if (player_y >= SCREEN_H)
                player_y = 0;
            original_c = get_color(player_x, player_y);
        }
        if (GAMEPAD_PRESSED(0, left)) 
        {
            --player_x;
            if (player_x < 0)
                player_x = SCREEN_W-1;
            original_c = get_color(player_x, player_y);
        }
        if (GAMEPAD_PRESSED(0, right)) 
        {
            ++player_x;
            if (player_x >= SCREEN_W)
                player_x = 0;
            original_c = get_color(player_x, player_y);
        }
        set_color(player_x, player_y, player_c);
    }

  
    if (vga_frame % 10 == 0)
    {
        if (GAMEPAD_PRESSED(0, A)) 
        {  
            // // fill the area with the current player color
            // if (player_c != original_c) // only fill if different than original color
            // {
            //     // temporarily set the previous background color
            //     set_color(player_x, player_y, original_c);
            //     // so that fill color will work:
            //     fill_color(original_c, player_x, player_y, fill_c);
            // }
            if (fill_can_start() && player_c != original_c) // only fill if different from original color
            {
                fill_init(original_c, player_x, player_y, player_c);
            }
        }
        
        if (GAMEPAD_PRESSED(0, R))
        {
            if (player_s > 1)
                --player_s;
        }
        if (GAMEPAD_PRESSED(0, L))
        {
            if (player_s < 10)
                ++player_s;
        }

        if (GAMEPAD_PRESSED(0, select))
        {
            load_picture(previous_picture++);
        } 
        else if (GAMEPAD_PRESSED(0, start))
        {
            previous_picture = 0;
            save_picture();            
        }
    }

    fill_frame();
}

