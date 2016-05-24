#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"
#include <stdio.h>
#include <string.h>

int player_x, player_y, player_c;

void set_color(int x, int y, int c) 
{
    if (x % 2) 
        superpixel[y][x/2] = (superpixel[y][x/2]&15)|((c&15)<<4);
    else 
        superpixel[y][x/2] = (c&15)|(superpixel[y][x/2]&(15<<4));
}

void draw_box(int x, int y, int c) 
{
    x /= 2;
    c &= 15;
    c |= (c<<4);
    for (int i=x; i<x+8; ++i) 
    for (int j=y; j<y+16; ++j)
        superpixel[j][i] = c;

}

void game_init()
{ 
    player_x = 0;
    player_y = 0;
    player_c = 1;
    clear_screen();
    set_color(player_x, player_y, player_c);
    int k=0;
    for (int i=0; i<SCREEN_W/16; ++i) 
    for (int j=0; j<SCREEN_H/16; ++j)
        draw_box(i*16, j*16, k++);
}


void game_frame()
{
    kbd_emulate_gamepad();

    if (vga_frame % 10 == 0) 
    {
        if (GAMEPAD_PRESSED(0, X))
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
        }
        if (GAMEPAD_PRESSED(0, down)) 
        {
            ++player_y;
            if (player_y >= SCREEN_H)
                player_y = 0;
        }
        if (GAMEPAD_PRESSED(0, left)) 
        {
            --player_x;
            if (player_x < 0)
                player_x = SCREEN_W-1;
        }
        if (GAMEPAD_PRESSED(0, right)) 
        {
            ++player_x;
            if (player_x >= SCREEN_W)
                player_x = 0;
        }
        set_color(player_x, player_y, player_c);
    }

    if (GAMEPAD_PRESSED(0, A)) { 
    }
    
    if (GAMEPAD_PRESSED(0, R))
    {
    }
    if (GAMEPAD_PRESSED(0, L))
    {
    }

    if (GAMEPAD_PRESSED(0, select))
    {
    }
    
    if (GAMEPAD_PRESSED(0, start))
    {
    }
}
