#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

uint16_t row_color1[Nx+2] CCM_MEMORY;
uint16_t row_color2[Nx+2] CCM_MEMORY;
uint16_t row_color3[Nx+2] CCM_MEMORY;

uint16_t *row_current CCM_MEMORY, *row_above CCM_MEMORY, *row_below CCM_MEMORY;

uint8_t box_count;

uint8_t y_draw_order[MAX_BOXES]; // index of boxes sorted by increasing y value
uint8_t y_first_box_index; // index of the box which appeared first and is still being drawn
uint8_t y_last_box_index; // index+1 of the box which is last appeared...

uint8_t current_box_count; // number of boxes on screen = y_last_box_index - y_first_box_index
uint8_t x_draw_order[MAX_BOXES]; // index of currently active boxes, sorted by increasing x

/*
 y draw order:
    
    box[y_draw_order[0]] is the first box to be drawn on screen
    box[y_draw_order[1]] is the second box
    y_draw_order[x_draw_order[0]] to y_draw_order[x_draw_order[current_box_count-1]]
        are the currently drawn boxes in order
*/

void_fn_2int* graph_line_callback;
void_fn_2int* partial_graph_line_callback;

extern const uint8_t font88_data[256][8];
uint8_t font8_data_cached[256][8] CCM_MEMORY;

uint16_t superpixel[Ny][Nx] CCM_MEMORY;
uint16_t bg_color CCM_MEMORY;
uint32_t palette[16] CCM_MEMORY;
uint8_t cascade CCM_MEMORY;


char text[1024];
uint8_t text_attr[1024];
struct box box[MAX_BOXES];

// default palettes

void graph_frame() 
{
//    if (vga_frame % 60 == 0)
//        message("new frame:\n");
    if (!partial_graph_line_callback)
    {
        if (graph_line_callback)
            partial_graph_line_callback = graph_line_callback;
    }

    if (!cascade)
    {
        row_above = row_color1+1; // offset so we don't have to check for i > 0
        row_current = row_color2+1; // or i < Nx-1 things in the background evolution functions
        row_below = row_color3+1;
        memset(row_above, 0, 2*Nx);
        memcpy(row_current, superpixel[0], 2*Nx);
        memcpy(row_below, superpixel[1], 2*Nx);
    }
    else
    {
        row_above = row_color1+1;
        row_current = row_color1+1;
        row_below = row_color2+1;
        memset(row_above, 0, 2*Nx);
        if (cascade < 2)
            memset(row_below, 0, 2*Nx);
        //memcpy(row_below, superpixel[0], 2*Nx);
    }

    // sort the draw orders by y appearing first!
    for (int j=0; j<box_count-1; ++j)
    {
        uint8_t y_tos = y_draw_order[j+1]; // top of current stack, push it down if...
        int16_t tos_top = box[y_tos].y; // this value is smaller than others below it
        int k=j;
        for (; k>=0 && (box[y_draw_order[k]].y > tos_top ||
            (box[y_draw_order[k]].y == tos_top && box[y_draw_order[k]].x > box[y_tos].x));
            --k)
        {
            // move tos_top down, and move other draw orders up:
            y_draw_order[k+1] = y_draw_order[k];
        }
        // put tos_top where it belongs:  --k hits once more than expected!
        y_draw_order[k+1] = y_tos;
    }
    y_first_box_index = 0;
    y_last_box_index = 0;
    current_box_count = 0; // reset to zero each frame!
//    if (vga_frame % 60 == 0)
//    {
//        message("ordering:\n");
//        for (int j=0; j<box_count; ++j)
//            message(" box %d\n", y_draw_order[j]);
//    }
}
// --------------------------------------------------------------

static inline void draw_line()
{
    if (!vga_odd) // draw to the buffer
    {
        uint32_t *dst = (uint32_t*) draw_buffer;
        for (int i=0; i<Nx; ++i)
        {
            uint16_t current_color = row_current[i]; //superpixel[vga_line/2][i];
            *dst++ = current_color | (current_color<<16);
        }
    }
    else // not a drawing time...
    {
        if (vga_line % 2 == 0)
        {
            if (graph_line_callback)
                graph_line_callback(0, Nx);
        }
        else
        {
            if (!cascade)
            {
                // above
                // current
                // below

                // no cascade, make rows behave as normal
                // do this switcheroo:
                //   below -> current
                //   current -> above
                //   above -> below (and reset it)
                uint16_t *ptr = row_current;
                row_current = row_below;
                row_below = row_above;
                row_above = ptr;
                
                if (vga_line/2 < Ny-2)
                    memcpy(row_below, superpixel[vga_line/2+2], 2*Nx);
                else if (vga_line/2 == Ny-2)
                    memset(row_below, 0, 2*Nx);
            }
            else
            {
                // (above)  above
                // (current)above
                // (below)  current

                // make cascade happen...
                uint16_t *ptr = row_current;
                row_current = row_below;
                row_above = row_below;
                row_below = ptr;
                if (vga_line/2 < Ny-1)
                    memcpy(row_below, superpixel[vga_line/2+1], 2*Nx);
            }
        }
    }
}


void graph_line() 
{
    int draw_changed = 0;
    int16_t vga16 = (int16_t) vga_line;
    while (y_last_box_index < box_count)
    {
        if (box[y_draw_order[y_last_box_index]].y <= vga16) 
        {
            draw_changed = 1;
            // add last_box_index to x_draw_order:
            x_draw_order[current_box_count] = y_last_box_index;
            //if (vga_frame % 60 == 0)
            //    message("adding box %d -> %d at y = %d\n", (int)y_last_box_index, (int)y_draw_order[y_last_box_index], (int)box[y_draw_order[y_last_box_index]].y);
            ++y_last_box_index;
            ++current_box_count;
            //if (vga_frame % 60 == 0)
            //{
            //    message("set of boxes:\n");
            //    for (int k=0; k<current_box_count; ++k)
            //    message("   %d -> %d\n", (int)x_draw_order[k], (int)y_draw_order[x_draw_order[k]]);
            //    message("got current box count %d, first/last = %d/%d\n", (int)current_box_count, (int)y_first_box_index, (int)y_last_box_index);
            //}
        }
        else
            break;
    }
    while (y_first_box_index < box_count)
    {
        uint8_t box_index = y_draw_order[y_first_box_index];
        if (box[box_index].y+box[box_index].height*8 <= vga16) 
        {
            draw_changed = 1;
            // remove first_box_index from x_draw_order
            int k=0;
            while (k<current_box_count)
            {
                if (x_draw_order[k] == y_first_box_index)
                    break;
                else
                    ++k;
            }
            // knock all the other indices down, e.g.:
            // 0 1 2 3 4    # draw count = 5
            // 0 2 3 4      # draw count = 4
            --current_box_count;
            //if (vga_frame % 60 == 0)
            //    message("killing off box %d -> %d at k=%d\n", (int)x_draw_order[k], (int)y_draw_order[x_draw_order[k]], k);
            if (current_box_count)
            {
                for (; k<(int)(current_box_count); ++k)
                {
//                    if (vga_frame % 60 == 0)
//                        message(" moving %d ", k);
                    x_draw_order[k] = x_draw_order[k+1];
                }
                //if (vga_frame % 60 == 0)
                //{
                //    message("\nremaining boxes:\n");
                //    for (k=0; k<current_box_count; ++k)
                //    message("   %d -> %d\n", (int)x_draw_order[k], (int)y_draw_order[x_draw_order[k]]);
                //}
            }

            ++y_first_box_index;
        }
        else
            break;
    }
    if (current_box_count == 0 || y_first_box_index == y_last_box_index) // none to draw...
    {
        draw_line();
        return;
    }
    else if (draw_changed)
    {
        //if (vga_frame % 60 == 0)
        //message("first and last %d - %d\n",(int) y_first_box_index, (int)y_last_box_index);
        // sort the x_draw_order correctly!
        // sort the draw orders by x appearing first!
        //if (vga_frame % 60 == 0)
        //message("new draw count = %d\n", current_box_count);
        for (int j=0; j<current_box_count-1; ++j)
        {
            uint8_t x_tos = x_draw_order[j+1]; // top of x stack, push it down if
            uint8_t tos_left = box[y_draw_order[x_tos]].x; // this is less than others
            int k=j;
            for (; k>=0 && box[y_draw_order[x_draw_order[k]]].x > tos_left; --k)
            {
                // move tos_left down, and move other draw orders up:
                x_draw_order[k+1] = x_draw_order[k];
            }
            // put box_top where it belongs:  (k-- hits once more than expected)
            x_draw_order[k+1] = x_tos;
        }
        //if (vga_frame % 60 == 0)
        //{
        //    message("boxes after sort:\n");
        //    for (int k=0; k<current_box_count; ++k)
        //    message("   %d:  %d -> %d\n", k, (int)x_draw_order[k], (int)y_draw_order[x_draw_order[k]]);
        //}
    }

    // 
    // the following logic checks if we should put any text boxes on screen.
    // text-boxes are assumed to come one after the other, vertically, so
    // no two text boxes should be on the same horizontal line.

    // need to worry about drawing the text boxes and the line.

    // start with superpixels on the left
    int i=0; // super pixel i
    if (!vga_odd) // draw to the buffer
    {
        uint32_t *dst = ((uint32_t*) draw_buffer); 
        for (int k=0; k<current_box_count; ++k)
        {
            // draw superpixels left of the text box
            uint8_t current_box_index = y_draw_order[x_draw_order[k]];
            for (; i<((int)box[current_box_index].x); ++i)
            {
                // write every superpixel up to the text wall...
                uint16_t current_color = row_current[i]; 
                *dst++ = current_color | (current_color<<16);
            }
            int16_t txtj = vga16 - box[current_box_index].y;

            // TEXT DRAWING ALGORITHM
            // borrowed from bitbox/lib/simple.c VGA_SIMPLE_MODE=10,
            // with some extra bits to make background color of text impress upon superpixels
            static uint32_t lut_data[4]; // cache couples for faster drawing
            static uint8_t prev_attr = 0xff;
            int text_index = box[current_box_index].offset +
                box[current_box_index].width*(txtj/8);
            uint8_t *text_pos = (uint8_t*) &text[text_index];
            uint8_t *attr_pos = &text_attr[text_index];
            // check if we need to draw into the superpixel background
            if ((txtj == 0) || (txtj == 8*((int)box[current_box_index].height)-1))
            {
                // yes, update superpixels all the way across, 
                // we are at the top and bottom of the text box:
                txtj %= 8; // now figure out which of the 8 horizontal lines of a character it should be
                for (int txti=0; txti<box[current_box_index].width; ++txti)
                {
                    uint8_t p = font8_data_cached[*text_pos++][txtj];
                    uint8_t attr = *attr_pos++;
                    if (attr != prev_attr)
                    {   // update the cached colors
                        uint32_t c = palette[attr];

                        lut_data[0] = (c&0xffff)*0x10001; // AA
                        lut_data[1] = c; // AB
                        lut_data[2] = (c<<16 | c>>16); // BA
                        lut_data[3] = (c>>16)*0x10001; // BB

                        prev_attr = attr;

                        // set the background here...
                        uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
                        uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
                        *setsuperpixel++ = lut_data[0];
                        *setsuperpixel++ = lut_data[0];
                        *setrowcurrent++ = lut_data[0];
                        *setrowcurrent++ = lut_data[0]; 
                    }
                    else 
                    {
                        // set the background here...
                        uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
                        uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
                        *setsuperpixel++ = lut_data[0];
                        *setsuperpixel++ = lut_data[0];
                        *setrowcurrent++ = lut_data[0];
                        *setrowcurrent++ = lut_data[0]; 
                    }
                    *dst++ = lut_data[(p>>6) & 3];
                    *dst++ = lut_data[(p>>4) & 3];
                    *dst++ = lut_data[(p>>2) & 3];
                    *dst++ = lut_data[(p>>0) & 3];
                    i+=4; // increment four superpixels at a time
                }
            }
            else if (vga_line % 2 == 0) 
            {
                // we are drawing somewhere in the middle (vertically speaking) of the textbox, 
                // but we should update the superpixels at the edges

                txtj %= 8; // now figure out which of the 8 horizontal lines of a character it should be
                // do first and last char separately:  here the first one:
                int txti = 0;
                uint8_t p = font8_data_cached[*text_pos++][txtj];
                uint8_t attr = *attr_pos++;
                if (attr != prev_attr)
                {   // update the cached colors
                    uint32_t c = palette[attr];

                    lut_data[0] = (c&0xffff)*0x10001; // AA
                    lut_data[1] = c; // AB
                    lut_data[2] = (c<<16 | c>>16); // BA
                    lut_data[3] = (c>>16)*0x10001; // BB

                    prev_attr = attr;

                    // set the background here...
                    uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
                    uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
                    *setsuperpixel = lut_data[0];
                    *setrowcurrent = lut_data[0];
                }
                else
                {
                    // set the background here...
                    uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
                    uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
                    *setsuperpixel = lut_data[0];
                    *setrowcurrent = lut_data[0];
                }
                *dst++ = lut_data[(p>>6) & 3];
                *dst++ = lut_data[(p>>4) & 3];
                *dst++ = lut_data[(p>>2) & 3];
                *dst++ = lut_data[(p>>0) & 3];
                
                i+=4; // increment four superpixels at a time

                // now do the middle chars, but don't update superpixels...
                for (txti=1; txti<box[current_box_index].width-1; ++txti)
                {
                    p = font8_data_cached[*text_pos++][txtj];
                    attr = *attr_pos++;
                    if (attr != prev_attr)
                    {   // update the cached colors
                        uint32_t c = palette[attr];

                        lut_data[0] = (c&0xffff)*0x10001; // AA
                        lut_data[1] = c; // AB
                        lut_data[2] = (c<<16 | c>>16); // BA
                        lut_data[3] = (c>>16)*0x10001; // BB

                        prev_attr = attr;
                    }
                    *dst++ = lut_data[(p>>6) & 3];
                    *dst++ = lut_data[(p>>4) & 3];
                    *dst++ = lut_data[(p>>2) & 3];
                    *dst++ = lut_data[(p>>0) & 3];
                    i+=4; // increment two superpixels at a time
                }

                // now finally do the last char:
                p = font8_data_cached[*text_pos++][txtj];
                attr = *attr_pos++;
                i+=2; // increment two superpixels at a time, to get to the last bit...
                if (attr != prev_attr)
                {   // update the cached colors
                    uint32_t c = palette[attr];

                    lut_data[0] = (c&0xffff)*0x10001; // AA
                    lut_data[1] = c; // AB
                    lut_data[2] = (c<<16 | c>>16); // BA
                    lut_data[3] = (c>>16)*0x10001; // BB

                    prev_attr = attr;

                    // set the background here...
                    uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
                    uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
                    *setsuperpixel = lut_data[0];
                    *setrowcurrent = lut_data[0];
                }
                else
                {
                    // set the background here...
                    uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
                    uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
                    *setsuperpixel = lut_data[0];
                    *setrowcurrent = lut_data[0];
                }
                *dst++ = lut_data[(p>>6) & 3];
                *dst++ = lut_data[(p>>4) & 3];
                *dst++ = lut_data[(p>>2) & 3];
                *dst++ = lut_data[(p>>0) & 3];
                i += 2; // necessary to get to the right superpixels correctly
            }
            else // somewhere in the middle of the text block, don't need to update superpixels
            {
                txtj %= 8; // now figure out which of the 8 horizontal lines of a character it should be
                for (int txti=0; txti<box[current_box_index].width; ++txti)
                {
                    uint8_t p = font8_data_cached[*text_pos++][txtj];
                    uint8_t attr = *attr_pos++;
                    if (attr != prev_attr)
                    {   // update the cached colors
                        uint32_t c = palette[attr];

                        lut_data[0] = (c&0xffff)*0x10001; // AA
                        lut_data[1] = c; // AB
                        lut_data[2] = (c<<16 | c>>16); // BA
                        lut_data[3] = (c>>16)*0x10001; // BB

                        prev_attr = attr;
                    }
                    *dst++ = lut_data[(p>>6) & 3];
                    *dst++ = lut_data[(p>>4) & 3];
                    *dst++ = lut_data[(p>>2) & 3];
                    *dst++ = lut_data[(p>>0) & 3];
                    i+=4; // increment two superpixels at a time
                }
            }
        }

        // finished TEXT.  now move onto right superpixels
        for (; i<Nx; ++i)
        {
            // write after the text wall
            uint16_t current_color = row_current[i]; //superpixel[vga_line/2][i];
            *dst++ = current_color | (current_color<<16);
        }
    }
    else // don't draw this round, just run bg stuff
    {
        if (vga_line % 2 == 0)
        {
            if (partial_graph_line_callback)
            {
                uint8_t current_box_index = y_draw_order[x_draw_order[0]];
                i = ((int)box[current_box_index].x); 
                // update the left pixels:
                if (i)
                    partial_graph_line_callback(0, i);

                // the right side of the first text box becomes
                // the left side of what we next need to update:
                int i0 = i + 4*((int)box[current_box_index].width);
        
                // update between each text box
                for (int k=1; k<current_box_count; ++k)
                {
                    current_box_index = y_draw_order[x_draw_order[k]];
                    i = ((int)box[current_box_index].x); 
                    
                    if (i > i0)
                        partial_graph_line_callback(i0, i);

                    i0 = i + 4*((int)box[current_box_index].width);
                }
               
                // update the right pixels
                if (i0 < Nx)
                    partial_graph_line_callback(i0, Nx);
            }
        }
        else
        {
            if (!cascade)
            {
                // above
                // current
                // below

                // no cascade, make rows behave as normal
                // do this switcheroo:
                //   below -> current
                //   current -> above
                //   above -> below (and reset it)
                uint16_t *ptr = row_current;
                row_current = row_below;
                row_below = row_above;
                row_above = ptr;
               
                // DO SOMETHING SMARTER HERE IF WE STILL CAN'T WING IT!
                // i.e. only copy over rows that are used
                if (vga_line/2 < Ny-2)
                {
                    memcpy(row_below, superpixel[vga_line/2+2], 2*Nx);
                }
                else if (vga_line/2 == Ny-2)
                    memset(row_below, 0, 2*Nx);
            }
            else
            {
                // (above)  above
                // (current)above
                // (below)  current

                // make cascade happen...
                uint16_t *ptr = row_current;
                row_current = row_below;
                row_above = row_below;
                row_below = ptr;
                if (vga_line/2 < Ny-1)
                    memcpy(row_below, superpixel[vga_line/2+1], 2*Nx);
            }

        }
    }
}

// --------------------------------------------------------------
// utilities 

void clear_screen() 
{
   memset(superpixel, 0, sizeof(superpixel));
   memcpy(font8_data_cached, font88_data, sizeof(font8_data_cached));
   for (int i=0; i<MAX_BOXES; ++i)
   {
        y_draw_order[i] = i;
   }
}

void background_decay(int istart, int iend)
{
    int j = vga_line/2;
    if (vga_frame % 2 == j % 2) // timing
    for (int i=istart; i<iend; ++i)
    {
        int newcolor = 1*(row_current[i]-bg_color);
        newcolor += (row_current[i-1]-bg_color);
        newcolor += (row_current[i+1]-bg_color);
        newcolor += (row_above[i]-bg_color);
        newcolor += (row_below[i]-bg_color);
        superpixel[j][i] = newcolor/5 + bg_color;
    }
}

void background_color_decay(int istart, int iend)
{
    int j = vga_line/2;
    // skip every other superpixel to render faster (i += 2 down below),
    // but start odd/even every other time:
    istart += (vga_frame + j)%2; 

    switch ((vga_frame/2) % 3) // only do one color each line...
    {
    case 0:
        for (int i=istart; i<iend; i+=2) // here's where we skip...
        {
            uint8_t bg_r = (bg_color >> 10)&31;

            int r = 4*(((row_current[i] >> 10)&31) - bg_r);
        
            r += ((row_current[i-1] >> 10)&31) - bg_r;
            r += ((row_current[i+1] >> 10)&31) - bg_r;
            r += ((row_above[i] >> 10)&31) - bg_r;
            r += ((row_below[i] >> 10)&31) - bg_r;

            r = r/8 + bg_r;
            if (r < 0)
                r = 0;
            else if (r > 31)
                r = 31;
            
            superpixel[j][i] = (r<<10) | (superpixel[j][i] & 1023);
        }
        break;
    case 1:

        for (int i=istart; i<iend; i+=2)
        {
            uint8_t bg_g = (bg_color >> 5)&31;

            int g = 4*(((row_current[i] >> 5)&31) - bg_g);
           
            g += ((row_current[i-1] >> 5)&31) - bg_g;
            g += ((row_current[i+1] >> 5)&31) - bg_g;
            g += ((row_above[i] >> 5)&31) - bg_g;
            g += ((row_below[i] >> 5)&31) - bg_g;

            g = g/8 + bg_g;
            if (g < 0)
                g = 0;
            else if (g > 31)
                g = 31;

            superpixel[j][i] = (g<<5) | (superpixel[j][i] & 31775);
        }
        break;
    case 2:
        for (int i=istart; i<iend; i+=2)
        {
            uint8_t bg_b = (bg_color)&31;

            int b = 4*(((row_current[i])&31) - bg_b);
           
            b += ((row_current[i-1])&31) - bg_b;
            b += ((row_current[i+1])&31) - bg_b;
            b += ((row_above[i])&31) - bg_b;
            b += ((row_below[i])&31) - bg_b;

            b = b/8 + bg_b;
            if (b < 0)
                b = 0;
            else if (b > 31)
                b = 31;

            superpixel[j][i] = (superpixel[j][i] & 32736) | b;
        }
        break;
    }
}

