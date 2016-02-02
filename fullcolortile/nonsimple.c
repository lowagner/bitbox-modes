#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#define FASTMEM __attribute__ ((section (".ccm")))

uint16_t bg_color;
uint8_t tile_map[TILE_MAP_MEMORY];
uint8_t tile_translator[16] FASTMEM;
//uint16_t tile_properties[16] FASTMEM;
uint16_t tile_draw[16][16][16] FASTMEM;
int16_t tile_map_x FASTMEM, tile_map_y FASTMEM;
uint16_t tile_map_width FASTMEM, tile_map_height FASTMEM;
// tile_map_width * tile_map_height <= TILE_MAP_MEMORY
// tile_map_x < tile_map_width - 320
// tile_map_y < tile_map_height - 240

// break sprites up into 16x16 tiles:
//uint8_t sprite_translator[16];
uint16_t sprite_draw[16][16][16];

struct object object[MAX_OBJECTS];
uint8_t first_free_object;
uint8_t first_used_object;
uint8_t object_count; 

uint8_t draw_order[MAX_OBJECTS];
uint8_t drawing_count; 
uint8_t first_drawing_index; 
uint8_t last_drawing_index; 


static inline void swap_draw_order_k_kminus1(int k);

void graph_frame() 
{
    // insertion sort all the drawing objects on the map
    for (int j=0; j<drawing_count-1; ++j)
    {
//        for (int k=j+1; k > 0 && ((draw_order[k-1].iy > draw_order[k].iy) ||    
//            ((draw_order[k-1].iy == draw_order[k].iy) && 
//             (draw_order[k-1].index_z&240 > draw_order[k].index_z&240)) ); --k)
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
// --------------------------------------------------------------

void graph_line() 
{
    if (vga_odd)
        return;
    int tile_j = tile_map_y + vga_line;
//    if (tile_j < 0 || tile_j >= tile_map_height*16)
//    {
//        memset(draw_buffer, bg_color, 2*SCREEN_W);
//        return;
//    }   
    uint16_t *dst = draw_buffer;
    if (tile_map_x % 16)
    {
        uint8_t *tile = &tile_map[(tile_j/16)*tile_map_width + tile_map_x / 16];
        tile_j %= 16;
        // draw the first tile (it's somehwat off screen)
        uint8_t trans = tile_translator[(*tile++)&15];
        for (int l=tile_map_x%16; l<16; ++l)
        {
            uint16_t color = tile_draw[trans][tile_j][l];
            if (color < (1 << 15))
                *dst++ = color;
            else
                *dst++ = bg_color;
        }
        // draw 19 un-broken tiles:
        for (int k=0; k<19; ++k)
        {
            // translate the tile into what tile it should be drawn as:
            trans = tile_translator[(*tile++)&15];
            for (int l=0; l<16; ++l)
            {
                uint16_t color = tile_draw[trans][tile_j][l];
                if (color < (1 << 15))
                    *dst++ = color;
                else
                    *dst++ = bg_color;
            }
        }
        // draw 22'nd broken tile:
        trans = tile_translator[(*tile)&15];
        for (int l=0; l<(tile_map_x%16); ++l)
        {
            uint16_t color = tile_draw[trans][tile_j][l];
            if (color < (1 << 15))
                *dst++ = color;
            else
                *dst++ = bg_color;
        }
    }
    else
    {
        uint8_t *tile = &tile_map[(tile_j/16)*tile_map_width + tile_map_x / 16];
        tile_j %= 16;
        for (int k=0; k<20; ++k)
        {
            // translate the tile into what tile it should be drawn as:
            uint8_t trans = tile_translator[(*tile++)&15];
            for (int l=0; l<16; ++l)
            {
                uint16_t color = tile_draw[trans][tile_j][l];
                if (color < (1 << 15))
                    *dst++ = color;
                else
                    *dst++ = bg_color;
            }
        }
    }

    // now that we've drawn the background, draw the sprites...
//    if (vga_frame % 60 == 0 && vga_line % 32 == 0)
//        message("drawing line %d:  drawing count %d\n", vga_line, drawing_count);

    if (!drawing_count) // none to draw...
        return;
    int16_t vga16 = (int16_t) vga_line;
    while (last_drawing_index < drawing_count)
    {
        if (object[draw_order[last_drawing_index]].iy <= vga16) 
            ++last_drawing_index;
        else
            break;
    }
    vga16-=16;
    while (first_drawing_index < drawing_count)
    {
        if (object[draw_order[first_drawing_index]].iy <= vga16) 
            ++first_drawing_index;
        else
            break;
    }
    vga16+=16;
    for (int k=first_drawing_index; k<last_drawing_index; ++k)
    {
        struct object *o = &object[draw_order[k]];
        int sprite_draw_row = vga16 - o->iy;
        if (o->ix < 0)
        {
            uint16_t *dst = draw_buffer;
            uint16_t *src = &sprite_draw[o->sprite_index][sprite_draw_row][-o->ix];
            for (int pxl=-o->ix; pxl<16; ++pxl)
            {
                if (*src < (1 << 15))
                    *dst = *src;
                ++dst; ++src;
            }
        }
        else if (o->ix > SCREEN_W-16)
        {
            int num_pxls = 1 + (SCREEN_W-1) - o->ix;
            uint16_t *dst = draw_buffer + o->ix;
            uint16_t *src = &sprite_draw[o->sprite_index][sprite_draw_row][0];
            for (int pxl=0; pxl<num_pxls; ++pxl)
            {
                if (*src < (1 << 15))
                    *dst = *src;
                ++dst; ++src;
            }
        }
        else
        {
            uint16_t *dst = draw_buffer + o->ix;
            uint16_t *src = &sprite_draw[o->sprite_index][sprite_draw_row][0];
            for (int pxl=0; pxl<16; ++pxl)
            {
                if (*src < (1 << 15))
                    *dst = *src;
                ++dst; ++src;
            }
        }
    }
}
// --------------------------------------------------------------
// utilities 

void clear() 
{
    memset(tile_map, 0, sizeof(tile_map));
    tile_map_x = 0;
    tile_map_y = 0;
    for (uint8_t i=0; i<16; ++i)
    {
        tile_translator[i] = i; 
        //sprite_translator[i] = i; 
    }
}

static inline void swap_draw_order_k_kminus1(int k)
{
//    // move the pointers around for the objects draw_index:
//    int object_kminus1 = draw_order[k-1].ix_object >> 10;
//    int object_k = draw_order[k].ix_object >> 10;
//    object[object_kminus1].draw_index = k;
//    object[object_k].draw_index = k-1;
//
//    // swap draw_order:
//    int old_k_value = draw_order[k].value;
//    draw_order[k].value = draw_order[k-1].value;
//    draw_order[k-1].value = old_k_value;
    // move the pointers around for the objects draw_index:

    int object_kminus1 = draw_order[k-1];
    int object_k = draw_order[k];

    object[object_kminus1].draw_index = k;
    object[object_k].draw_index = k-1;

    draw_order[k-1] = object_k;
    draw_order[k] = object_kminus1;
}

static inline void make_unseen_object_viewable(int i)
{
    // object is viewable, but hasn't gone on the drawing list:
    draw_order[drawing_count] = i;
    object[i].iy = object[i].y - tile_map_y;
    object[i].ix = object[i].x - tile_map_x;

    /*
    draw_order[drawing_count].iy = object[i].y - tile_map_y;
    draw_order[drawing_count].ix_object = ((object[i].x - tile_map_x) & 1023) | (i << 10);
    draw_order[drawing_count].index_z = object[i].index_z;
    */
    /*
    // assume everything is sorted from 0 to drawing_count-1
    int k=drawing_count;
    while ( k > 0 && (draw_order[k-1].iy > draw_order[k].iy) )
    {
        swap_draw_order_k_kminus1(k);
        --k;
    }
    
    while ( k > 0 && 
        (draw_order[k-1].iy == draw_order[k].iy) && 
        (draw_order[k-1].index_z&240 > draw_order[k].index_z&240) )
    {
        swap_draw_order_k_kminus1(k);
        --k;
    }
    */
    object[i].draw_index = drawing_count;
    ++drawing_count;
}

static inline int on_screen(int16_t x, int16_t y)
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
    // NEED TO CHECK FOR x < tile_map_x - 16... instead!
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
