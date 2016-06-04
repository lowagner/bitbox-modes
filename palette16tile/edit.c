#include "bitbox.h"
#include "nonsimple.h"
#include "edit.h"
#include "tiles.h"
#include "fill.h"
#include "font.h"
#include "io.h"
#include "sprites.h"
#include <string.h> // memset

uint8_t edit_tile CCM_MEMORY;
uint8_t edit_sprite CCM_MEMORY;

int edit_x CCM_MEMORY;
int edit_y CCM_MEMORY;
uint8_t edit_color CCM_MEMORY;
uint8_t previous_canvas_color CCM_MEMORY;
uint8_t edit_paint_mode CCM_MEMORY;

static const char hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

void edit_tile_line() 
{
    if (vga_line < 32)
    {
        if (vga_line < 4)
        {
            if (vga_line/2 == 0)
                for (int i=0; i<SCREEN_W; ++i)
                    draw_buffer[i] = palette[edit_color];
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
        else if (vga_line/2 == 10 || vga_line/2 == 15)
            memset(draw_buffer, 0, 2*SCREEN_W);
        else // goes from 22 to 30
        {
            int line = vga_line - 22;
            uint8_t text[] = { 't', 'i', 'l', 'e', ':', ' ', hex[edit_tile], 
                ' ', '<', 'L', '/', 'R', '>',
                0 };
            font_render_line_doubled(text, 8, line, 2);
        }
        return;
    }
    else if (vga_line >= SCREEN_H - 32)
    {
        if (vga_line >= SCREEN_H - 4)
        {
            if (vga_line/2 == (SCREEN_H-4)/2)
            {
                for (int i=0; i<SCREEN_W; ++i)
                    draw_buffer[i] = palette[edit_color];
            }
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
        else if (vga_line/2 == (SCREEN_H-32)/2 || vga_line/2 == (SCREEN_H-22)/2)
            memset(draw_buffer, 0, 2*SCREEN_W);
        else // goes from SCREEN_H-30 to SCREEN_H-22
        {
            int line = vga_line - (SCREEN_H-30);
            /*
                controls:
                edit_paint_mode:
                    X/Y cycle
                    A: mode
                    B: fill
                else:
                    X/Y/ cycle
                    A: mode
                    B: paint

            */
            if (edit_paint_mode)
            {
                uint8_t text[] = { 
                    'X', '/', 'Y', ':', 'c', 'y', 'c', 'l', 'e', ' ', 'c', 'o', 'l', 'o', 'r', 's', ' ',
                    'A', ':', 'm', 'o', 'd', 'e', ' ',
                    'B', ':', 'f', 'i', 'l', 'l',
                    0 };
                font_render_line_doubled(text, 20, line, 2);
            }
            else
            {
                uint8_t text[] = { 
                    'X', '/', 'Y', ':', 'c', 'y', 'c', 'l', 'e', ' ', 'c', 'o', 'l', 'o', 'r', 's', ' ',
                    'A', ':', 'm', 'o', 'd', 'e', ' ',
                    'B', ':', 'p', 'a', 'i', 'n', 't',
                    0 };
                font_render_line_doubled(text, 20, line, 2);
            }
        }
        return;
    }

    // separate block to limit scope of variables therein:
    {
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
            for (int step=0; step<4; ++step)
            {
                uint8_t *tile_color = &tile_draw[(tile_j/16)&15][tile_j&15][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
        }
        else
        {
            for (int step=0; step<4; ++step)
            {
                uint8_t *tile_color = &tile_draw[edit_tile][tile_j&15][0] - 1;
                for (int l=0; l<8; ++l) 
                {
                    uint32_t color = palette[(*(++tile_color))&15];
                    color |= palette[(*tile_color)>>4] << 16;
                    *dst++ = color;
                }
            }
        }
    }
}

void edit_sprite_line() 
{
    
}

void edit_spot_paint()
{
    if (edit_x % 2)
    {
        int color = tile_draw[edit_tile][edit_y][edit_x/2];
        previous_canvas_color = color >> 4;
        tile_draw[edit_tile][edit_y][edit_x/2] = (color & 15) | (edit_color<<4);
    }
    else
    {
        int color = tile_draw[edit_tile][edit_y][edit_x/2];
        previous_canvas_color = color & 15;
        tile_draw[edit_tile][edit_y][edit_x/2] = edit_color | (color & 240);
    }
}

static inline int edit_spot_color()
{
    if (edit_x % 2)
        return tile_draw[edit_tile][edit_y][edit_x/2] >> 4;
    else
        return tile_draw[edit_tile][edit_y][edit_x/2] & 15;
}

void edit_tile_controls()
{
    if (GAMEPAD_PRESS(0, select))
    {
        fill_stop();
        visual_mode = TilesAndSprites;
        return;
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
        fill_stop();
        edit_tile = (edit_tile + movement)&15; 
        return;
        //edit_sprite = (edit_sprite + movement)&15; 
    }
   
    int moved = 0, paint_if_moved = 0;
    
    if (GAMEPAD_PRESSING(0, Y))
    {
        edit_color = (edit_color + 1)&15;
        moved = 1;
    }
    else if (GAMEPAD_PRESSING(0, X))
    {
        edit_color = (edit_color - 1)&15;
        moved = 1;
    }
    if (edit_paint_mode)
    {
        if (GAMEPAD_PRESS(0, A))
        {
            edit_paint_mode = 0;
            return;
        }
        
        if (GAMEPAD_PRESSING(0, B))
        {
            if (fill_can_start() && previous_canvas_color != edit_color)
            {
                fill_init(tile_draw[edit_tile][0], 16, 16, 
                    previous_canvas_color, edit_x, edit_y, edit_color);
                gamepad_press_wait = GAMEPAD_PRESS_WAIT;
                return;
            }
        }
    }
    else
    {
        if (GAMEPAD_PRESS(0, A))
        {
            edit_paint_mode = 1;
            edit_spot_paint();
            return;
        }
        
        if (GAMEPAD_PRESSING(0, B))
        {
            edit_spot_paint();
            paint_if_moved = 1;
        }        
    }
    
    if (GAMEPAD_PRESSING(0, left))
    {
        edit_x = (edit_x - 1)&15;
        moved = 1;
    }
    else if (GAMEPAD_PRESSING(0, right))
    {
        edit_x = (edit_x + 1)&15;
        moved = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        edit_y = (edit_y - 1)&15;
        moved = 1;
    }
    else if (GAMEPAD_PRESSING(0, down))
    {
        edit_y = (edit_y + 1)&15;
        moved = 1;
    }
    if (moved)
    {
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        if (edit_paint_mode || paint_if_moved)
            edit_spot_paint();
        return;
    }
    else if (paint_if_moved)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
}
