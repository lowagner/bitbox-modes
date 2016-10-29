#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

uint16_t row_color1[Nx+2] CCM_MEMORY;
uint16_t row_color2[Nx+2] CCM_MEMORY;
uint16_t row_color3[Nx+2] CCM_MEMORY;

uint16_t *row_current CCM_MEMORY, *row_above CCM_MEMORY, *row_below CCM_MEMORY;

int box_count CCM_MEMORY;
uint8_t y_draw_order[MAX_BOXES] CCM_MEMORY; 
#define Y_DRAW_INDEX y_draw_order[0]

/*
 y draw order:
    
    box[y_draw_order[1]] is the first box to be drawn on screen
    box[y_draw_order[2]] is the second box
*/

void_fn_2int* graph_line_callback;
void_fn_2int* partial_graph_line_callback;

extern const uint8_t font88_data[256][8];
uint8_t font8_data_cached[256][8] CCM_MEMORY;

uint16_t superpixel[Ny][Nx] CCM_MEMORY;
uint16_t bg_color CCM_MEMORY;
uint32_t palette[16] CCM_MEMORY;
uint8_t cascade CCM_MEMORY;


char text[1024] CCM_MEMORY;
uint8_t text_attr[1024] CCM_MEMORY;
struct box box[MAX_BOXES] CCM_MEMORY;

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
    for (int j=1; j<box_count; ++j)
    {
        uint8_t y_tos = y_draw_order[j+1]; // top of current stack, push it down if...
        int16_t tos_top = box[y_tos].y; // this value is smaller than others below it
        int k=j;
        for (; k>=1 && (box[y_draw_order[k]].y > tos_top ||
            (box[y_draw_order[k]].y == tos_top && box[y_draw_order[k]].x > box[y_tos].x));
            --k)
        {
            // move tos_top down, and move other draw orders up:
            y_draw_order[k+1] = y_draw_order[k];
        }
        // put tos_top where it belongs:  --k hits once more than expected!
        y_draw_order[k+1] = y_tos;
    }
    //message("ordering:\n");
    //for (int j=1; j<=MAX_BOXES; ++j)
    //    message(" box %d\n", y_draw_order[j]);
    box[0].next = 0; // set head of list
    Y_DRAW_INDEX = 1;
    while (Y_DRAW_INDEX <= box_count && (box[y_draw_order[Y_DRAW_INDEX]].y+box[y_draw_order[Y_DRAW_INDEX]].height*10 <= 0))
    {
        ++Y_DRAW_INDEX;
    }
}
// --------------------------------------------------------------

void graph_line() 
{
    if (vga_odd)
    {
        // do background update stuff
        if (vga_line % 2 == 0)
        {
            uint8_t current = box[0].next;
            if (!current)
            {
                if (graph_line_callback)
                    graph_line_callback(0, Nx);
            }
            else if (partial_graph_line_callback)
            {
                int iR = ((int)box[current].x); 
                // update the left pixels:
                if (iR)
                    partial_graph_line_callback(0, iR);

                // the right side of the first text box becomes
                // the left side of what we next need to update:
                int iL = iR + 4*((int)box[current].width) + 4;
        
                // update between each text box
                while ((current=box[current].next))
                {
                    iR = ((int)box[current].x); 
                    
                    if (iR > iL)
                        partial_graph_line_callback(iL, iR);

                    iL = iR + 4*((int)box[current].width) + 4;
                }
               
                // update the right pixels
                if (iL < Nx)
                    partial_graph_line_callback(iL, Nx);
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
        return;
    }
    
    int16_t vga16 = (int16_t) vga_line;
    // add any boxes which should be drawn
    while (Y_DRAW_INDEX <= box_count)
    {
        uint8_t current = y_draw_order[Y_DRAW_INDEX];
        if (box[current].y > vga16)
            break;
        // current needs to be added to singly linked list
        uint8_t previous = 0;
        uint8_t next = box[0].next;
        while ((next) && (box[next].x < box[current].x))
        {
            previous = next;
            next = box[next].next;
        }
        box[previous].next = current;
        box[current].next = next;
        ++Y_DRAW_INDEX;
    }
    // remove any boxes which have finished drawing
    if (box[0].next)
    {
        uint8_t previous = 0;
        uint8_t current = box[0].next;
        while (current)
        {
            if ((box[current].y+box[current].height*10 <= vga_line))
                // remove current from the draw list
                box[previous].next = box[current].next;
            else
                previous = current;
            current = box[current].next;
        }
        if (!box[0].next)
            goto no_boxes;
    }
    else // none to draw...
    no_boxes:
    {
        uint32_t *dst = (uint32_t*) draw_buffer;
        for (int i=0; i<Nx; ++i)
        {
            uint16_t current_color = row_current[i]; //superpixel[vga_line/2][i];
            *dst++ = current_color | (current_color<<16);
            // a superpixel is 2x a regular pixel, i.e. it is written
            // at the word level (32 bits), not the pixel level (16 bits)...
        }
        return;
    }

    // the following logic checks if we should put any text boxes on screen.
    // text-boxes are assumed to come one after the other, vertically
    // need to worry about drawing the text boxes and the line.
    
    // start with superpixels on the left
    int i=0; // super pixel i
    // draw to the buffer
    uint32_t *dst = ((uint32_t*) draw_buffer);
    uint8_t current = box[0].next;
    do // we know that current != 0 since we would have gone to "no_boxes" above.
    {
        // draw superpixels left of the text box
        for (; i<((int)box[current].x); ++i)
        {
            // write every superpixel up to the text wall...
            uint16_t current_color = row_current[i]; 
            *dst++ = current_color | (current_color<<16);
        }
        int16_t txtj = vga16 - box[current].y;

        // TEXT DRAWING ALGORITHM
        // borrowed from bitbox/lib/simple.c VGA_SIMPLE_MODE=10,
        // with some extra bits to make background color of text impress upon superpixels
        static uint32_t lut_data[4]; // cache couples for faster drawing
        int text_index = box[current].offset + box[current].width*(txtj/10);
        uint8_t *text_pos = (uint8_t*) &text[text_index];
        uint8_t *attr_pos = &text_attr[text_index];
        uint8_t attr = *attr_pos;
        static uint8_t prev_attr = 0xff;

        if (attr != prev_attr)
        {
            uint32_t c = palette[attr];

            lut_data[0] = (c&0xffff)*0x10001; // AA
            lut_data[1] = c; // AB
            lut_data[2] = (c<<16 | c>>16); // BA
            lut_data[3] = (c>>16)*0x10001; // BB

            prev_attr = attr;
        }

        // check if we need to draw into the superpixel background
        if ((txtj == 0) || (txtj == 10*((int)box[current].height)-1))
        {
            //message("drawing box %d top or bottom at line %d\n", current, vga_line);
            // yes, update superpixels all the way across, 
            // we are at the top and bottom of the text box:

            uint32_t *setsuperpixel = (uint32_t*)&superpixel[vga_line/2][i];
            *setsuperpixel++ = lut_data[0];
            uint32_t *setrowcurrent = (uint32_t*)&row_current[i];
            *setrowcurrent++ = lut_data[0];
            *dst++ = lut_data[0];
            *dst++ = lut_data[0];
            i += 2;
            for (int txti=0; txti<box[current].width; ++txti)
            {
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
                *setsuperpixel++ = lut_data[0];
                *setsuperpixel++ = lut_data[0];
                *setrowcurrent++ = lut_data[0];
                *setrowcurrent++ = lut_data[0]; 
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i += 4;
            }
            *dst++ = lut_data[0];
            *dst++ = lut_data[0];
            *setsuperpixel++ = lut_data[0];
            *setrowcurrent++ = lut_data[0];
            i += 2;
            //message("top of box %d\n", i);
        }
        else if (vga_line % 2 == 0) 
        {
            // we are drawing somewhere in the middle (vertically speaking) of the textbox, 
            // but we should update the superpixels at the edges

            txtj %= 10; // now figure out which of the 8 horizontal lines of a character it should be
            if (txtj == 0 || txtj == 9)
            {
                int txti = 0;
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
                // set the background here...
                superpixel[vga_line/2][i] = lut_data[0];
                superpixel[vga_line/2][i+1] = lut_data[0];
                row_current[i] = lut_data[0];
                row_current[i+1] = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                
                i+=6; // increment four superpixels at a time

                // now do the middle chars, but don't update superpixels...
                for (txti=1; txti+1<box[current].width; ++txti)
                {
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
                    *dst++ = lut_data[0];
                    *dst++ = lut_data[0];
                    *dst++ = lut_data[0];
                    *dst++ = lut_data[0];
                    i+=4; // increment two superpixels at a time
                }

                // now finally do the last char:
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
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i+=4; // 
                // set the background here...
                superpixel[vga_line/2][i] = lut_data[0];
                superpixel[vga_line/2][i+1] = lut_data[0];
                row_current[i] = lut_data[0];
                row_current[i+1] = lut_data[0];
                i += 2; // necessary to get to the right superpixels correctly
            }
            else
            {
                // do first and last char separately:  here the first one:
                txtj -= 1;
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
                }
                // set the background here...
                superpixel[vga_line/2][i] = lut_data[0];
                superpixel[vga_line/2][i+1] = lut_data[0];
                row_current[i] = lut_data[0];
                row_current[i+1] = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                *dst++ = lut_data[(p>>6) & 3];
                *dst++ = lut_data[(p>>4) & 3];
                *dst++ = lut_data[(p>>2) & 3];
                *dst++ = lut_data[(p>>0) & 3];
                
                i+=6; // increment four superpixels at a time

                // now do the middle chars, but don't update superpixels...
                for (txti=1; txti+1<box[current].width; ++txti)
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
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i+=4; // increment four superpixels at a time, to get to the last bit...
                superpixel[vga_line/2][i] = lut_data[0];
                superpixel[vga_line/2][i+1] = lut_data[0];
                row_current[i] = lut_data[0];
                row_current[i+1] = lut_data[0];
                i += 2; // necessary to get to the right superpixels correctly
            }
        }
        else // somewhere in the middle of the text block, don't need to update superpixels
        {
            txtj %= 10; // now figure out which of the 8 horizontal lines of a character it should be
            if (txtj == 0 || txtj == 9)
            {
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i += 2;
                for (int txti=0; txti<box[current].width; ++txti)
                {
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
                    *dst++ = lut_data[0];
                    *dst++ = lut_data[0];
                    *dst++ = lut_data[0];
                    *dst++ = lut_data[0];
                    i+=4; // increment two superpixels at a time
                }
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i += 2;
            }
            else
            {
                txtj -= 1;
                
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i += 2;

                for (int txti=0; txti<box[current].width; ++txti)
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
                
                *dst++ = lut_data[0];
                *dst++ = lut_data[0];
                i += 2;
            }
        }
        current = box[current].next;
    } while (current);

    // finished TEXT.  now move onto right superpixels
    for (; i<Nx; ++i)
    {
        // write after the text wall
        uint16_t current_color = row_current[i]; //superpixel[vga_line/2][i];
        *dst++ = current_color | (current_color<<16);
    }
}

// --------------------------------------------------------------
// utilities 

void clear_screen() 
{
   memset(superpixel, 0, sizeof(superpixel));
   memcpy(font8_data_cached, font88_data, sizeof(font8_data_cached));
   for (int i=0; i<MAX_BOXES; ++i)
        y_draw_order[i] = i;
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

