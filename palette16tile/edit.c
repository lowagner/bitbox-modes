#include "bitbox.h"
#include "edit.h"
#include "nonsimple.h"
#include "tiles.h"
#include "sprites.h"
#include <string.h> // memset

uint8_t edit_tile CCM_MEMORY;
uint8_t edit_sprite CCM_MEMORY;

uint8_t edit_spot CCM_MEMORY;
uint8_t edit_color CCM_MEMORY;

void edit_tile_line() 
{
    if (vga_line < 32)
    {
        if (vga_line < 4)
        {
            if (vga_line/2 == 0)
            for (int i=0; i<SCREEN_W; ++i)
                draw_buffer[i] = palette[(vga_frame/(60*5))&15];
        }
        else if (vga_line < 20)
        {
            int tile_j = vga_line-4;
            uint32_t *dst = (uint32_t *)draw_buffer;
            
            uint8_t *tile_color;
           
            // draw a border, some of the edited tile, plus all other tiles
            for (int step=0; step<2; ++step)
            {
                tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
    
            // alternate edit tile and other tiles:
            for (int tile=0; tile<8; ++tile)
            {
                tile_color = &tile_draw[tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
                tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
            
            for (int step=0; step<2; ++step)
            {
                tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
        }
        else if (vga_line/2 == 10)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= SCREEN_H - 32)
    {
        if (vga_line >= SCREEN_H - 4)
        {
            if (vga_line/2 == (SCREEN_H-4)/2)
            for (int i=0; i<SCREEN_W; ++i)
                draw_buffer[i] = palette[(vga_frame/(60*5))&15];
        }
        else if (vga_line >= SCREEN_H - 20)
        {
            int tile_j = vga_line-(SCREEN_H - 20);
            uint32_t *dst = (uint32_t *)draw_buffer;
            
            uint8_t *tile_color;
            // draw a border, some of the edited tile, plus all other tiles
            for (int step=0; step<2; ++step)
            {
                tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
    
            // alternate edit tile and other tiles:
            for (int tile=8; tile<16; ++tile)
            {
                tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
                tile_color = &tile_draw[tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
            
            for (int step=0; step<2; ++step)
            {
                tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
        }
        else if (vga_line/2 == (SCREEN_H-32)/2)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }

    // separate block to limit scope of variables therein:
    {
        int edit_x = edit_spot&15;
        int edit_y = edit_spot>>4;

        // draw big tile
        int tile_j = (vga_line-32)/11;
        uint32_t *dst = ((uint32_t *)draw_buffer) + 32/2;
        uint8_t *tile_color = &tile_draw[edit_tile][tile_j][0] - 1;
        int draw_crosshair = (edit_y == tile_j) && (((vga_line-32)%11)/2 == 2);
        
        for (int l=0; l<8; ++l)
        {
            uint32_t color = palette[(*(++tile_color))&15];
            color |= (color << 16);
            if (draw_crosshair && edit_x == l*2)
            {
                *dst++ = color;
                *dst++ = color;
                *dst++ = ~color;
                *dst++ = color;
                *dst++ = color;
            }
            else
            {
                for (int k=0; k<5; ++k)
                    *dst++ = color;
            }
           
            // in the middle
            color = (color&65535) | (palette[(*tile_color)>>4]<<16);
            *dst++ = color;

            color = ((color>>16)&65535) | (color&(65535<<16));
            if (draw_crosshair && edit_x == l*2+1)
            {
                *dst++ = color;
                *dst++ = color;
                *dst++ = ~color;
                *dst++ = color;
                *dst++ = color;
            }
            else
            {
                for (int k=0; k<5; ++k)
                    *dst++ = color;
            }
        }
    }

    {
        // draw other tiles scrolling
        uint32_t *dst = ((uint32_t *)draw_buffer) + 32/2 + 8*11 + 16/2;
        int tile_j = vga_line-32 + vga_frame/20;
   
        if (tile_j/16 % 2)
        {
            uint8_t *tile_color = &tile_draw[(tile_j/16)&15][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
            tile_color = &tile_draw[(tile_j/16)&15][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
            tile_color = &tile_draw[edit_tile][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
            tile_color = &tile_draw[edit_tile][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
        }
        else
        {
            uint8_t *tile_color = &tile_draw[edit_tile][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
            tile_color = &tile_draw[edit_tile][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
            tile_color = &tile_draw[(tile_j/16)&15][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }
            tile_color = &tile_draw[(tile_j/16)&15][tile_j&15][0] - 1;
            for (int l=0; l<8; ++l) 
            {
                uint32_t color = palette[(*(++tile_color))&15];
                color |= palette[(*tile_color)>>4] << 16;
                *dst++ = color;
            }

        }
    }
}

void edit_sprite_line() 
{
    
}


void edit_tile_controls()
{
    if (vga_frame % 4 == 0)
    {
        if (GAMEPAD_PRESSED(0, left))
        {
            if ((edit_spot & 15) > 0)
            {
                --edit_spot;
            }
        }
        else if (GAMEPAD_PRESSED(0, right))
        {
            if ((edit_spot & 15) < 15)
            {
                ++edit_spot;
            }
        }
        if (GAMEPAD_PRESSED(0, up))
        {
            if (edit_spot >= 16)
            {
                edit_spot -= 16;
            }
        }
        else if (GAMEPAD_PRESSED(0, down))
        {
            if (edit_spot < 240)
            {
                edit_spot += 16;
            }
        }
    }
    if (GAMEPAD_PRESS(0, select))
    {
        visual_mode = TilesAndSprites;
    }
    int movement = 0;
    if (GAMEPAD_PRESS(0, R))
    {
        movement = 1;
    }
    else if (GAMEPAD_PRESS(0, L))
    {
        movement = 15;
    }
    
    if (movement)
    {
        edit_tile = (edit_tile + movement)&15; 
        //edit_sprite = (edit_sprite + movement)&15; 
    }
}
