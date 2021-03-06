#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"
#include "fatfs/ff.h"

// for saving/loading the background image
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
    bg_color = RGB(255,0,0);

    // color in some of the super pixels:
    draw_supersuperpixel(0,0,RGB(0,0,255));
    draw_supersuperpixel(0,1,RGB(0,255,255));
    draw_supersuperpixel(0,2,RGB(0,255,0));

    graph_line_callback = &background_decay;

    fat_result = f_mount(&fat_fs, "", 1); // mount now...
}

void game_frame()
{
    kbd_emulate_gamepad();

    // draw some other fun things on the screen,
    // like a circling cyan object...
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

    // and some other things wizzing around:
    superpixel[60+(x-80)/4][80+3*(x-80)/2] = RGB(0,0,0);
    superpixel[60+(y-60)/4][80-3*(y-60)/2] = RGB(255,255,255);


    if (GAMEPAD_PRESSED(0, Y))
        bg_color = RGB(255,0,0);
    else
        bg_color = RGB(10,10,10);

    if (GAMEPAD_PRESSED(0, A))
        graph_line_callback = &background_decay;
    if (GAMEPAD_PRESSED(0, B))
        graph_line_callback = &background_color_decay;

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
                for (int j=0; j<SCREEN_H; j++)
                for (int i=0; i<SCREEN_W; i++)
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
                for (int j=0; j<SCREEN_H; j++)
                for (int i=0; i<SCREEN_W; i++)
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
