#include "bitbox.h"
#include "sprites.h"
#include "tiles.h"

#include <stdlib.h> // rand

// break sprites up into 16x16 tiles:
uint8_t sprite_draw[16][8][16][8] CCM_MEMORY; // 16 sprites, 8 frames, 16x16 pixels...
// info about a sprite:
//   5 bits for "what color is invisible in this sprite"
//   3 bits for pattern of sprite
//   4 bits for initial health
//   4 bits for speed of sprite
//   4 bits for damage/passable property, x 4 sides 
//      first bit indicates damaging (or not if it's zero)
//      second three bits:  passable, breakable, hard, slippery, sticky, bouncey, 
uint32_t sprite_info[16][8] CCM_MEMORY; 

struct object object[MAX_OBJECTS] CCM_MEMORY;
uint8_t first_free_object CCM_MEMORY;
uint8_t first_used_object CCM_MEMORY;
uint8_t object_count CCM_MEMORY; 

uint8_t draw_order[MAX_OBJECTS] CCM_MEMORY;
uint8_t drawing_count CCM_MEMORY; 
uint8_t first_drawing_index CCM_MEMORY; 
uint8_t last_drawing_index CCM_MEMORY; 

void make_unseen_object_viewable(int i)
{
    // object is viewable, but hasn't gone on the drawing list:
    object[i].iy = object[i].y - tile_map_y;
    object[i].ix = object[i].x - tile_map_x;
    object[i].draw_index = drawing_count;
    draw_order[drawing_count] = i;

    ++drawing_count;
}

int on_screen(int16_t x, int16_t y)
{
    if (x > tile_map_x-16 && x < tile_map_x + SCREEN_W &&
        y > tile_map_y-16 && y < tile_map_y + SCREEN_H)
        return 1;
    return 0;
}

int create_object(int sprite_draw_index, int16_t x, int16_t y, uint8_t z)
{
    if (object_count >= MAX_OBJECTS)
        return -1;

    int i = first_free_object;
    // setup head of list for both free and used: 
    first_free_object = object[i].next_free_object;
    object[i].next_used_object = first_used_object;
    first_used_object = i;

    // add in object properties
    object[i].y = y;
    object[i].x = x;
    object[i].z = z;
    if (on_screen(object[i].x, object[i].y))
        make_unseen_object_viewable(i);
    else
        object[i].draw_index = -1;

    object[i].sprite_index = sprite_draw_index;
    ++object_count;
    return i;
}

void move_object(int i, int16_t x, int16_t y)
{
    if (object[i].draw_index < 255) // object was visible...
    {
        if (on_screen(x, y))
        {
            // object is still visible, need to sort draw_order.. but do it later!
            object[i].iy = y - tile_map_y;
            object[i].ix = x - tile_map_x;
        }
        else // object is no longer visible
        {
            for (int k=object[i].draw_index+1; k<drawing_count; ++k)
            {
                // move the pointers around for the objects draw_index:
                // object at draw_order k should go to spot k-1:
                int object_k = draw_order[k];
                // move down draw_order:
                object[object_k].draw_index = k-1;
                draw_order[k-1] = object_k;
            }
            object[i].iy = SCREEN_H;
            object[i].ix = SCREEN_W;
            object[i].draw_index = -1;
            --drawing_count;
        }
    }
    else // wasn't visible
    {
        if (on_screen(x, y))
        {
            // object has become visible
            make_unseen_object_viewable(i);
        }
        else // object is still not visible
        {
            // do nothing!
        }
    }
    object[i].y = y;
    object[i].x = x;
}

void update_objects()
{
    uint8_t ok = first_used_object; // object index
    while (ok < 255)
    {
        move_object(ok, object[ok].x, object[ok].y);
        ok = object[ok].next_used_object;
    }
}

void sprites_line()
{
    if (!drawing_count) // none to draw...
        return;
    int16_t vga16 = (int16_t) vga_line;
    // add any new sprites to the draw line:
    while (last_drawing_index < drawing_count)
    {
        if (object[draw_order[last_drawing_index]].iy <= vga16) 
            ++last_drawing_index;
        else
            break;
    }
    // subtract any early sprites no longer on the line
    vga16-=16;
    while (first_drawing_index < drawing_count)
    {
        if (object[draw_order[first_drawing_index]].iy <= vga16) 
            ++first_drawing_index;
        else
            break;
    }
    // reset vga16
    vga16+=16;
    // draw visible sprites
    for (int k=first_drawing_index; k<last_drawing_index; ++k)
    {
        struct object *o = &object[draw_order[k]];
        int sprite_draw_row = vga16 - o->iy;
        if (o->ix < 0) // object left of screen but still visible
        {
            uint16_t *dst = draw_buffer;
            uint8_t *src = &sprite_draw[o->sprite_index][o->sprite_frame][sprite_draw_row][-o->ix/2];
            uint8_t invisible_color = sprite_info[o->sprite_index][o->sprite_frame] & 31;
            if ((-o->ix)%2)
            {
                // if -o->ix == 15
                // this executes no matter what:
                uint8_t color = ((*src)>>4); //&15; //unnecessary!
                if (color != invisible_color)
                    *dst = palette[color]; // &65535; // unnecessary...
                ++dst;
                // if (o->ix == -13)
                // we run from 7 to <8, which is just once:
                for (int pxl=(-o->ix+1)/2; pxl<16/2; ++pxl) // 
                {
                    color = ((*(++src)))&15;
                    if (color != invisible_color)
                        *dst = palette[color]; // &65535; // unnecessary...
                    ++dst; 
                    color = ((*src)>>4); //&15; //unnecessary!
                    if (color != invisible_color)
                        *dst = palette[color]; // &65535; // unnecessary...
                    ++dst; 
                }
            }
            else // not odd
            {
                --src;
                for (int pxl=-o->ix/2; pxl<16/2; ++pxl)
                {
                    uint8_t color = (*(++src))&15;
                    if (color != invisible_color)
                        *dst = palette[color]; // &65535; // unnecessary...
                    ++dst; 
                    color = ((*src)>>4); //&15; //unnecessary!
                    if (color != invisible_color)
                        *dst = palette[color]; // &65535; // unnecessary...
                    ++dst; 
                }

            }
            //int odd = ((-o->ix)%2);
            //for (int pxl=-o->ix; pxl<16; ++pxl)
            //{
            //    uint8_t color;
            //    if (odd)
            //    {
            //        color = ((*src++)>>4); //&15; //unnecessary!
            //        odd = 0;
            //    }
            //    else
            //    {
            //        color = ((*src))&15;
            //        odd = 1;
            //    }
            //    if (color != invisible_color)
            //        *dst = palette[color]; // &65535; // unnecessary...
            //    ++dst; 
            //}
        }
        else if (o->ix > SCREEN_W-16)
        {
            // object right of screen but still visible
            int num_pxls = 1 + (SCREEN_W-1) - o->ix;
            uint16_t *dst = draw_buffer + o->ix;
            uint8_t *src = &sprite_draw[o->sprite_index][o->sprite_frame][sprite_draw_row][0]-1;
            uint8_t invisible_color = sprite_info[o->sprite_index][o->sprite_frame] & 31;
            uint8_t color;
            for (int pxl=0; pxl<num_pxls/2; ++pxl)
            {
                color = (*(++src))&15;
                if (color != invisible_color)
                    *dst = palette[color]; // &65535; // unnecessary...
                ++dst; 
                color = ((*src)>>4); //&15; //unnecessary!
                if (color != invisible_color)
                    *dst = palette[color]; // &65535; // unnecessary...
                ++dst; 
            }
            if (num_pxls%2)
            {
                color = (*(++src))&15;
                if (color != invisible_color)
                    *dst = palette[color]; // &65535; // unnecessary...
            }
        }
        else
        {
            uint16_t *dst = draw_buffer + o->ix;
            uint8_t *src = &sprite_draw[o->sprite_index][o->sprite_frame][sprite_draw_row][0]-1;
            uint8_t invisible_color = sprite_info[o->sprite_index][o->sprite_frame] & 31;
            for (int pxl=0; pxl<8; ++pxl)
            {
                uint8_t color = (*(++src))&15;
                if (color != invisible_color)
                    *dst = palette[color]; // &65535; // unnecessary...
                ++dst; 
                color = ((*src)>>4); //&15; //unnecessary!
                if (color != invisible_color)
                    *dst = palette[color]; // &65535; // unnecessary...
                ++dst; 
            }
            //int odd = 0;
            //// process through the nibbles (half bytes) individually:
            //for (int pxl=0; pxl<16; ++pxl)
            //{
            //    uint8_t color;
            //    if (odd)
            //    {
            //        color = ((*src++)>>4); //&15; //unnecessary!
            //        odd = 0;
            //    }
            //    else
            //    {
            //        color = ((*src))&15;
            //        odd = 1;
            //    }
            //    if (color != invisible_color)
            //        *dst = palette[color]; // &65535; // unnecessary...
            //    ++dst; 
            //}
        }
    }
}

static inline void swap_draw_order_k_kminus1(int k)
{
    int object_kminus1 = draw_order[k-1];
    int object_k = draw_order[k];

    object[object_kminus1].draw_index = k;
    object[object_k].draw_index = k-1;

    draw_order[k-1] = object_k;
    draw_order[k] = object_kminus1;
}

void sprites_frame()
{
    // insertion sort all the drawing objects on the map
    for (int j=0; j<drawing_count-1; ++j)
    {
        for (int k=j+1; k > 0 && ((object[draw_order[k-1]].iy > object[draw_order[k]].iy) ||    
            ((object[draw_order[k-1]].iy == object[draw_order[k]].iy) && 
             (object[draw_order[k-1]].z > object[draw_order[k]].z)) ); --k)
        {
            swap_draw_order_k_kminus1(k);
        }
    }
    
    first_drawing_index = 0;
    last_drawing_index = 0;
}

void sprites_reset()
{
    // create some random sprites...
    uint8_t *sc = sprite_draw[0][0][0];
    int color_index = 0;
    for (int tile=0; tile<15; ++tile)
    {
        for (int frame=0; frame<8; ++frame)
        {
            sprite_info[tile][frame] = 16;
            int forward = frame % 2;
            for (int line=0; line<16; ++line)
            {
                for (int col=0; col<8; ++col)
                    *sc++ = (((color_index+1)>>(tile/4))&15)|((((color_index - forward)>>(tile/4))&15)<<4);
                ++color_index;
            }
            color_index -= 16;
        }
        ++color_index;
    }
    // 16th sprite is random
    for (int l=0; l<8; ++l)
    {
        sprite_info[15][l] = 0; // with black as invisible for all frames
        for (int k=0; k<256/2; ++k)
            *sc++ = rand()%256;
    }
}
