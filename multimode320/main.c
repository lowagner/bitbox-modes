#include <stdlib.h> // rand
#include <math.h>
#include "nonsimple.h"
#include "fatfs/ff.h"
#include <stdio.h>
#include <string.h>

FATFS fat_fs;
FIL fat_file;
FRESULT fat_result;

int taking_picture, getting_picture;

void game_init()
{ 
    clear_screen();
    bg_color = RGB(255,0,0);

    graph_line_callback = &background_decay;

    fat_result = f_mount(&fat_fs, "", 1); // mount now...

    //palette:  bg | (fg << 16)
    palette[0] = RGB(30,50,100) | (RGB(255,235,205) << 16);//RGB(255,255,255) | (0 << 16);
    palette[1] = 0 | (RGB(255,200,0) <<16);
    palette[2] = RGB(255,255,0) | (RGB(150,0,0) <<16);
    palette[3] = 0 | (RGB(255,255,255) <<16);
    palette[4] = RGB(200,230,255) | (RGB(0,50,0) <<16);

    // don't use box[0], that's for administrative purposes.
    // the rest of the boxes MUST NOT OVERLAP.  that's bad.
    // box_count is the index of the highest box, since box[0] is not visible.
    box[1].y = 5;
    box[1].width = 32; // MUST BE NONZERO or there will be problems.
    box[1].x = 18;
    box[1].height = 8;
    box[1].offset = 0;
    for (int i=0; i<256; ++i)
        text[i] = (uint8_t) i;
    memset(text_attr, 0, sizeof(text_attr));  // use palette 0 for first quarter

    box[2].x = 10;
    box[2].y = 86;
    box[2].width = 10; // MUST BE NONZERO or there will be problems.
    box[2].height = 1;
    box[2].offset = 256;
   
    sprintf(text+box[2].offset, "OOOOOOOOOOO");
    memset(text_attr+box[2].offset, 1, sizeof(text_attr)/4); // use palette 1 for second

    box[3].x = 10;
    box[3].y = 165;
    box[3].width = 9; // MUST BE NONZERO or there will be problems.
    box[3].height = 7;
    box[3].offset = 512;

    sprintf(text+box[3].offset, "  Ness             HP 999   PP NNN             ++-0+  ");
    memset(text_attr+box[3].offset, 2, 9); // use palette 2
    
    box[4].x = 60;
    box[4].y = 165;
    box[4].width = 9; // MUST BE NONZERO or there will be problems.
    box[4].height = 7;
    box[4].offset = 640;

    sprintf(text+box[4].offset, "  |x\231dx            HP 234   PP 9XN            +-+-0+  ");
    memset(text_attr+box[4].offset, 3, 9); // use palette 3
    
    box[5].x = 110;
    box[5].y = 155;
    box[5].width = 9; // MUST BE NONZERO or there will be problems.
    box[5].height = 7;
    box[5].offset = 768;

    sprintf(text+box[5].offset, "Kristoph!           HP 99    PP XX              ---");
    memset(text_attr+box[5].offset, 4, 9); // use palette 4

    box_count = 5;

}

void game_frame()
{
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
        for (int i=0; i<=box_count; ++i)
            if (box[i].x > 0)
                box[i].x -= 1;
    }
    else if (GAMEPAD_PRESSED(0, right))
    {
        for (int i=0; i<=box_count; ++i)
            if (box[i].x + 4*box[i].width + 4 < Nx)
                box[i].x += 1;
    }


    if (GAMEPAD_PRESSED(0, down))
    {
        for (int i=0; i<=box_count; ++i)
            box[i].y += 1;
    }
    else if (GAMEPAD_PRESSED(0, up))
    {
        for (int i=0; i<=box_count; ++i)
            box[i].y -= 1;
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
