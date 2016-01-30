#include "nonsimple.h"

#include <stdint.h>
#include <bitbox.h>
#include <stdlib.h> // abs
#include <string.h> // memset

#define FASTMEM __attribute__ ((section (".ccm")))

uint16_t row_color1[Nx+2] FASTMEM;
uint16_t row_color2[Nx+2] FASTMEM;
uint16_t row_color3[Nx+2] FASTMEM;

uint16_t *row_current FASTMEM, *row_above FASTMEM, *row_below FASTMEM;

uint8_t current_text_box_index;

void_fn_2int* graph_line_callback;
void_fn_2int* partial_graph_line_callback;

extern const uint8_t font16_data[256][16];
uint8_t font16_data_cached[256][16] FASTMEM;

uint16_t superpixel[Ny][Nx] FASTMEM;
uint16_t bg_color FASTMEM;
uint32_t palette[16] FASTMEM;
uint8_t cascade;

// default palettes

void graph_frame() 
{
    current_text_box_index = 0;
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
}
// --------------------------------------------------------------

inline void draw_line()
{
    switch (vga_line % 4)
    {
        case 0:
        case 1:
        {
            uint32_t *dst = (uint32_t*)draw_buffer;
            for (int i=0; i<Nx; ++i)
            {
                uint32_t current_color = row_current[i]; //superpixel[vga_line/4][i];
                current_color |= (current_color<<16);

                *dst++ = current_color;
                *dst++ = current_color;
            }
            break;
        }
        case 2:
        {
            if (graph_line_callback)
                graph_line_callback(0, Nx);
            break;
        }
        case 3:
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
                
                if (vga_line/4 < Ny-2)
                    memcpy(row_below, superpixel[vga_line/4+2], 2*Nx);
                else if (vga_line/4 == Ny-2)
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
                if (vga_line/4 < Ny-1)
                    memcpy(row_below, superpixel[vga_line/4+1], 2*Nx);
            }
            break;
        }
    }
}


void graph_line() 
{
    // 
    int txtj = vga_line - 4*(text_box[current_text_box_index].y);
    // the following logic checks if we should put any text boxes on screen.
    // text-boxes are assumed to come one after the other, vertically, so
    // no two text boxes should be on the same horizontal line.
    if (text_box[current_text_box_index].offset < 0 ||
        txtj < 0 || 
        ((current_text_box_index == NUM_TEXT_BOXES-1) && 
         (txtj >= 16*((int)text_box[current_text_box_index].height))) ||
        ((txtj >= 16*((int)text_box[current_text_box_index].height)) && 
         (txtj=vga_line-4*(text_box[++current_text_box_index].y), txtj<0))
    )
    {
        // no need to worry about text boxes, just draw the line as normal.
        draw_line();
        return;
    }

    // need to worry about drawing the text boxes and the line.

    // start with line
    uint32_t *dst = ((uint32_t*) draw_buffer); // + 4*(int)text_box[current_text_box_index].x; 
    int i=0; // super pixel i
  
    // first draw all the super pixels left of the text box:
    switch (vga_line % 4)
    {
        case 0:
        case 1:
        {
            for (; i<2*((int)text_box[current_text_box_index].x); ++i)
            {
                // write every superpixel up to the text wall...
                uint32_t current_color = row_current[i]; //superpixel[vga_line/4][i];
                current_color |= (current_color<<16);

                *dst++ = current_color;
                *dst++ = current_color;
            }
            break;
        }
        case 2:
            i = 2*((int)text_box[current_text_box_index].x);
            if (partial_graph_line_callback)
            {
                if (i)
                    partial_graph_line_callback(0, i);
            }
            else if (graph_line_callback)
            {
                partial_graph_line_callback = graph_line_callback;
                if (i)
                    partial_graph_line_callback(0, i);
            }
            dst += 2*i;
            break;
        case 3:
            i = 2*((int)text_box[current_text_box_index].x);
            dst += 2*i;
            break;
    }

    // TEXT DRAWING ALGORITHM
    // borrowed from bitbox/lib/simple.c VGA_SIMPLE_MODE=10,
    // with some extra bits to make background color of text impress upon superpixels
    static uint32_t lut_data[4]; // cache couples for faster drawing
    static uint8_t prev_attr = 0xff;
    uint8_t *text_pos = (uint8_t*) &text[text_box[current_text_box_index].offset+text_box[current_text_box_index].width*(txtj/16)];
    uint8_t *attr_pos = &text_attr[text_box[current_text_box_index].offset+text_box[current_text_box_index].width*(txtj/16)];
    // check if we need to draw into the superpixel background
    if ((txtj == 0) || (txtj == 16*((int)text_box[current_text_box_index].height)-4))
    {
        // yes, update superpixels all the way across, 
        // we are at the top and bottom of the text box:
        txtj %= 16; // now figure out which of the 16 horizontal lines of a character it should be
        for (int txti=0; txti<text_box[current_text_box_index].width; ++txti)
        {
            uint8_t p = font16_data_cached[*text_pos++][txtj];
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
                superpixel[vga_line/4][i] = superpixel[vga_line/4][i+1] = row_current[i] = row_current[i+1] = c&65535;
                
            }
            else 
            {
                // set the background here...
                superpixel[vga_line/4][i] = superpixel[vga_line/4][i+1] = row_current[i] = row_current[i+1] = palette[attr]&65535;
            }
            *dst++ = lut_data[(p>>6) & 3];
            *dst++ = lut_data[(p>>4) & 3];
            *dst++ = lut_data[(p>>2) & 3];
            *dst++ = lut_data[(p>>0) & 3];
            i+=2; // increment two superpixels at a time
        }
    }
    else if (vga_line % 4 == 0) 
    {
        // we are drawing somewhere in the middle (vertically speaking) of the textbox, 
        // but we should update the superpixels at the edges

        txtj %= 16; // now figure out which of the 16 horizontal lines of a character it should be
        // do first and last char separately:  here the first one:
        int txti = 0;
        uint8_t p = font16_data_cached[*text_pos++][txtj];
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
            superpixel[vga_line/4][i] = superpixel[vga_line/4][i+1] = row_current[i] = row_current[i+1] = c&65535;
            
        }
        else
        {
            // set the background here...
            superpixel[vga_line/4][i] = superpixel[vga_line/4][i+1] = row_current[i] = row_current[i+1] = palette[attr]&65535;
        }
        *dst++ = lut_data[(p>>6) & 3];
        *dst++ = lut_data[(p>>4) & 3];
        *dst++ = lut_data[(p>>2) & 3];
        *dst++ = lut_data[(p>>0) & 3];
        
        i+=2; // increment two superpixels at a time

        // now do the middle chars, but don't update superpixels...
        for (txti=1; txti<text_box[current_text_box_index].width-1; ++txti)
        {
            p = font16_data_cached[*text_pos++][txtj];
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
            i+=2; // increment two superpixels at a time
        }

        // now finally do the last char:
        p = font16_data_cached[*text_pos++][txtj];
        attr = *attr_pos++;
        if (attr != prev_attr)
        {   // update the cached colors
            uint32_t c = palette[attr];

            lut_data[0] = (c&0xffff)*0x10001; // AA
            lut_data[1] = c; // AB
            lut_data[2] = (c<<16 | c>>16); // BA
            lut_data[3] = (c>>16)*0x10001; // BB

            prev_attr = attr;

            // set the background here...
            superpixel[vga_line/4][i] = superpixel[vga_line/4][i+1] = row_current[i] = row_current[i+1] = c&65535;
            
        }
        else
        {
            // set the background here...
            superpixel[vga_line/4][i] = superpixel[vga_line/4][i+1] = row_current[i] = row_current[i+1] = palette[attr]&65535;
        }
        *dst++ = lut_data[(p>>6) & 3];
        *dst++ = lut_data[(p>>4) & 3];
        *dst++ = lut_data[(p>>2) & 3];
        *dst++ = lut_data[(p>>0) & 3];
        
        i+=2; // increment two superpixels at a time
    }
    else // somewhere in the middle of the text block, don't need to update superpixels
    {
        txtj %= 16; // now figure out which of the 16 horizontal lines of a character it should be
        for (int txti=0; txti<text_box[current_text_box_index].width; ++txti)
        {
            uint8_t p = font16_data_cached[*text_pos++][txtj];
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
            i+=2; // increment two superpixels at a time
        }
    }

    // other side of text box, draw super pixels:
    switch (vga_line % 4)
    {
        case 0:
        case 1:
        {
            for (; i<Nx; ++i)
            {
                // write after the text wall
                uint32_t current_color = row_current[i]; //superpixel[vga_line/4][i];
                current_color |= (current_color<<16);

                *dst++ = current_color;
                *dst++ = current_color;
            }
            break;
        }
        case 2:
        {
            if (partial_graph_line_callback)
            {
                if (i < Nx)
                    partial_graph_line_callback(i, Nx);
            }
            break;
        }
        case 3:
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
               
                // DO SOMETHING HERE IF WE STILL CAN'T WING IT!
                if (vga_line/4 < Ny-2)
                    memcpy(row_below, superpixel[vga_line/4+2], 2*Nx);
                else if (vga_line/4 == Ny-2)
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
                if (vga_line/4 < Ny-1)
                    memcpy(row_below, superpixel[vga_line/4+1], 2*Nx);
            }
            break;
        }
    }
}

// --------------------------------------------------------------
// utilities 

void clear_screen() 
{
   memset(superpixel, 0, sizeof(superpixel));
   memcpy(font16_data_cached, font16_data, sizeof(font16_data_cached));
}

void background_decay(int istart, int iend)
{
    int j = vga_line/4;
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
    int j = vga_line/4;
    // skip every other superpixel to render faster (i += 2 down below),
    // but start odd/even every other time:
    istart += (vga_frame + j)%2; 
    switch ((vga_frame/2) % 3) // only do one color each line...
    {
    case 0:
        for (int i=istart; i<iend; i+=2)
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

