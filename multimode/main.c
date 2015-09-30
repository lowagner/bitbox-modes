#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"
#include "fatfs/ff.h"
#include <stdio.h>
#include <string.h>


char text[1024];
uint8_t text_attr[1024];
struct text_box text_box[NUM_TEXT_BOXES];

FATFS fat_fs;
FIL fat_file;
FRESULT fat_result;

int taking_picture, getting_picture;

inline void draw_supersuperpixel(int J, int I, uint16_t color)
{
    for (int di=0; di<8; di++)
    for (int dj=0; dj<6; dj++)
        superpixel[J*6+dj][I*8+di] = color; 
}

void game_init()
{ 
    clear_screen();
    bg_color = RGB(255,0,0);

    draw_supersuperpixel(0,0,RGB(0,0,255));
    graph_line_callback = &background_decay;

    fat_result = f_mount(&fat_fs, "", 1); // mount now...

    //palette:  bg | (fg << 16)
    palette[0] = RGB(255,255,255) | (0 << 16);
    palette[1] = 0 | (RGB(255,200,0) <<16);

    text_box[0].x = 4;
    text_box[0].y = 10;
    text_box[0].width = 50; // MUST BE NONZERO or there will be problems.
    text_box[0].height = 10;
    sprintf(text, "who are you?  i dare say not understandable til the last drop.  that is absurd and unintelligible i am afraid, quoth the raven.  NO way hosea.");
    memset(text_attr, 0, sizeof(text_attr)/2);  // use palette 0 for first half

    text_box[1].x = 50;
    text_box[1].y = 60;
    text_box[1].width = 10; // MUST BE NONZERO or there will be problems.
    text_box[1].height = 1;
    text_box[1].offset = 512;
    
    sprintf(text+512, "next box.");
    memset(text_attr+512, 1, sizeof(text_attr)/2); // use palette 1 for second

}

void game_frame()
{
    kbd_emulate_gamepad();
    
    int x = 80 + round(40*cos(vga_frame*0.05));;
    int y = 60 + round(40*sin(vga_frame*0.05));;
    superpixel[y][x] = RGB(0,0,255);
    superpixel[y][++x] = RGB(0,0,255);
    superpixel[y][++x] = RGB(0,0,255);
    superpixel[++y][x] = RGB(0,0,255);
    superpixel[y][--x] = RGB(0,255,255);
    superpixel[y][--x] = RGB(0,0,255);
    superpixel[++y][x] = RGB(0,0,255);
    superpixel[y][++x] = RGB(0,0,255);
    superpixel[y][++x] = RGB(0,0,255);

    superpixel[60+(x-80)/4][80+3*(x-80)/2] = RGB(0,0,0);
    superpixel[60+(y-60)/4][80-3*(y-60)/2] = RGB(255,255,255);


    if (GAMEPAD_PRESSED(0, X))
    {
        partial_graph_line_callback = &background_color_decay;
    }
    if (GAMEPAD_PRESSED(0, Y))
    {
        graph_line_callback = &background_color_decay;
        bg_color = RGB(255,0,0);
    }
    else
        bg_color = RGB(10,10,10);

    if (GAMEPAD_PRESSED(0, A))
    {
        graph_line_callback = &background_decay;
    }
    if (GAMEPAD_PRESSED(0, B))
    {
        partial_graph_line_callback = &background_decay;
    }
    
    if (GAMEPAD_PRESSED(0, R))
    {
        if (GAMEPAD_PRESSED(0, down))
            cascade = 2;
        else
            cascade = 1;
    }
    if (GAMEPAD_PRESSED(0, L))
    {
        cascade = 0;
    }

    if (GAMEPAD_PRESSED(0, left))
    {
        if (text_box[0].x > 0) // assumes text_box[0] is furthest left.
            for (int i=0; i<NUM_TEXT_BOXES; ++i)
                text_box[i].x -= 1;
    }
    else if (GAMEPAD_PRESSED(0, right))
    {
        if (2*text_box[NUM_TEXT_BOXES-1].x + 2*text_box[NUM_TEXT_BOXES-1].width < Nx-1) // assumes text_box[-1] is furthest right.
            for (int i=0; i<NUM_TEXT_BOXES; ++i)
                text_box[i].x += 1;

    }


    if (GAMEPAD_PRESSED(0, down))
    {
        for (int i=0; i<NUM_TEXT_BOXES; ++i)
            text_box[i].y += 1;
    }
    else if (GAMEPAD_PRESSED(0, up))
    {
        for (int i=0; i<NUM_TEXT_BOXES; ++i)
            text_box[i].y -= 1;
    }


    if (GAMEPAD_PRESSED(0, select))
    {
        if (!taking_picture)
        {
            message("taking picture\n");
            taking_picture = 1;
            fat_result = f_open(&fat_file,"hello.ppm", FA_WRITE | FA_OPEN_ALWAYS);
            f_lseek(&fat_file, 0);
            if (fat_result == FR_OK)
            {
                UINT bytes_get;
                f_write(&fat_file, "P6\n160 120 31\n", 14, &bytes_get);
                //uint32_t *src = (uint32_t*) superpixel;
                for (int j=0; j<Ny; j++)
                for (int i=0; i<Nx; i++)
                {
                    uint16_t C = superpixel[j][i];
                    char msg[3];
                    msg[0] = (C >> 10)&31; // red
                    msg[1] = (C >> 5)&31; // green
                    msg[2] = (C)&31; // blue
                    f_write(&fat_file, msg, 3, &bytes_get);
                }
                f_close(&fat_file);
            }
        }
    }
    else
        taking_picture = 0;
    
    if (GAMEPAD_PRESSED(0, start))
    {
        if (!getting_picture)
        {
            message("resetting picture\n");
            getting_picture = 1;
            fat_result = f_open(&fat_file,"hello.ppm", FA_READ);
            if (fat_result == FR_OK)
            {
                UINT bytes_get;
                char msg[14];
                f_read(&fat_file, msg, 14, &bytes_get); 
                if (bytes_get != 14) { f_close(&fat_file); return; }
                for (int j=0; j<Ny; j++)
                for (int i=0; i<Nx; i++)
                {
                    f_read(&fat_file, msg, 3, &bytes_get);
                    if (bytes_get != 3) { f_close(&fat_file); return; }

                    superpixel[j][i]=((msg[0]&31)<<10) | ((msg[1]&31)<<5) | (msg[2]&31);
                }
                f_close(&fat_file);
            }
        }
    }
    else
        getting_picture = 0;
}
