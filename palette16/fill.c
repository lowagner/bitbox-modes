#include "fill.h"
#include "nonsimple.h"
#include <stdint.h>
#include <stdlib.h> // rand
#include <string.h> // memset

#define MAX_STACK_INDEX (SCREEN_H*2-1)

int fill_old_color, fill_new_color;
uint8_t fill_matrix[SCREEN_H][SCREEN_W/8];
// 0 - not checked
// 1 - checked

int fill_stack[MAX_STACK_INDEX+1];
// 2^9 2^9 2^8
// x_min + y*SCREEN_W + (x_max<<17)
int fill_stack_index = -1;
// value = fill_x + fill_y * SCREEN_W

void set_fill_matrix(int x, int y) 
{
    fill_matrix[y][x/8] |= 1 << (x % 8);
}

int get_fill_matrix(int x, int y)
{
    return (fill_matrix[y][x/8] & (1<<(x%8)) );
}

void fill_add_row_to_stack(int y, int xmin, int xmax)
{
    if (fill_stack_index >= MAX_STACK_INDEX)
    {
        message("ran out of memory\n");
        // shuffle the fill stack and drop the last half of it
        int i=0;
        for (; i<=MAX_STACK_INDEX/2; ++i) // don't shuffle all the way up, you drop it anyway
        {
            int j = i + rand()%(MAX_STACK_INDEX+1 - i);
            int old_value_i = fill_stack[i];
            fill_stack[i] = fill_stack[j];
            fill_stack[j] = old_value_i;
        }
        fill_stack_index = i;
    }
    fill_stack[++fill_stack_index] = xmin + y*SCREEN_W + (xmax<<17);
}

void fill_row(int y, int xmin, int xmax, int c)
{
    if (xmax - xmin >= 4)
    {
        if (xmin % 2)
        {
            set_color(xmin, y, c);
            set_fill_matrix(xmin, y);
            ++xmin;
        }
        if (xmax % 2 == 0)
        {
            set_color(xmax, y, c);
            set_fill_matrix(xmax, y);
            --xmax;
        }
        c = (c & 15) | ((c & 15) << 4);
        // for example, if xmin == 3 and xmax == 6
        // then xmin -> 4 and xmax -> 5
        // then we need to set row[xmin/2 = 2] to row[xmax/2 = 2], inclusive
        // so we need to set 1 byte now

        // another example, xmin = 3 and xmax == 9
        // then xmin -> 4, xmin/2 -> 2, and xmax/2 -> 4.
        // need to set bytes 2, 3, and 4, so counting 3 in total
        memset(&superpixel[y][xmin/2], c, 1 + xmax/2-xmin/2);
        
        while (xmin <= xmax && (xmin % 8))
        {
            set_fill_matrix(xmin, y);
            ++xmin;
        }
        while (xmin <= xmax && (xmax % 8))
        {
            set_fill_matrix(xmax, y);
            --xmax;
        }
        if (xmin < xmax)
        {
            memset(&fill_matrix[y][xmin/8], 255, (xmax-xmin)/8);
        }
    }
    else
    {
        for (int x=xmin; x<=xmax; ++x)
        {
            set_fill_matrix(x, y);
            set_color(x, y, c);
        }
    }
}

int fill_can_start()
{
    return fill_stack_index < 0; 
}

void fill_init(int old_c, int x, int y, int new_c)
{
    memset(fill_matrix, 0, sizeof(fill_matrix));
    // so that fill color will work:
    int xmin = x;
    while (xmin > 0 && get_color(xmin-1, y) == old_c)
        --xmin;
    int xmax = x;
    while (xmax < SCREEN_W-1 && get_color(xmax+1, y) == old_c)
        ++xmax;
    
    if (y > 0)
        fill_add_row_to_stack(y-1, xmin, xmax);
    if (y < SCREEN_H-1)
        fill_add_row_to_stack(y+1, xmin, xmax);
    
    fill_old_color = old_c;
    fill_new_color = new_c;
    
    fill_row(y, xmin, xmax, fill_new_color);
}

// // Recursive works fine on emulator, not on bitbox:
//void fill_color(int this_c, int x, int y, int fill_c) 
//{
//    // set anything neighboring (x,y) that is "this_c" to color "fill_c" 
//    // but first check if (x,y) is this_c, otherwise break:
//    if (get_color(x, y) != this_c) 
//        return;
//
//    // set current color
//    set_color(x, y, fill_c);
//
//    // do neighbors recursively
//    if (x > 0)
//        fill_color(this_c, x-1, y, fill_c);
//    if (x < SCREEN_W-1)
//        fill_color(this_c, x+1, y, fill_c);
//    if (y > 0)
//        fill_color(this_c, x, y-1, fill_c);
//    if (y < SCREEN_H-1)
//        fill_color(this_c, x, y+1, fill_c);
//}

int fill_test(int x, int y)
{
    return (get_color(x, y) == fill_old_color && get_fill_matrix(x, y) == 0);
}

void fill_frame()
{for (int step=0; step<1; ++step) {
    if (fill_stack_index < 0)
        return;
    int z = fill_stack[fill_stack_index--];
    // unravel the xmin, xmax, and y values in fill_stack:
    int xmax = (z >> 17)&511;
    z &= (1<<17)-1;
    int xmin = z % SCREEN_W;
    int y = z / SCREEN_W;

    // the rows will be tested from xmin to xmax_current,
    // then you can add [y, xmin, xmax_current] to the stack
    int xmax_current = -1;
    if (fill_test(xmin, y))
    {
        // try moving backwards from here
        xmax_current = xmin;
        while (xmin > 0 && fill_test(xmin-1, y))
            --xmin;
    }
    else
    {
        // try moving forwards
        while (xmin < xmax)
        {
            ++xmin;
            if (fill_test(xmin, y))
                break;
        }
        if (xmin == xmax && fill_test(xmax, y) == 0)
        {
            // this stack value was a bust, nothing paintable in this row
            continue;
        }
        else
        {
            // we know for sure that xmin is possible
            xmax_current = xmin; 
        }
    }
    while (xmin <= xmax)
    {
        // xmin to xmax_current is currently a valid paintable row,
        // push the right side of the as far as possible:
        while (xmax_current < SCREEN_W-1 && fill_test(xmax_current+1, y))
            ++xmax_current;
       
        if (xmin == xmax_current)
        {
            // not much of a row to paint
            set_color(xmin, y, fill_new_color);
            set_fill_matrix(xmin, y);
            // but it may leak above and below:
            if (y > 0)
            {
                // do a pre check here since it will save room on the stack:
                if (fill_test(xmin, y-1))
                    fill_add_row_to_stack(y-1, xmin, xmin);
            }
            if (y < SCREEN_H-1)
            {
                if (fill_test(xmin, y+1))
                    fill_add_row_to_stack(y+1, xmin, xmin);
            }
        }
        else
        {
            fill_row(y, xmin, xmax_current, fill_new_color);
            // add stuff to the stack
            if (y > 0)
                fill_add_row_to_stack(y-1, xmin, xmax_current);
            if (y < SCREEN_H-1)
                fill_add_row_to_stack(y+1, xmin, xmax_current);
        }

        // since we know that xmax_current+1 is not a valid spot to fill,
        // start searching here:
        xmin = xmax_current + 2;

        if (xmin > xmax)
        {
            // nothing more can be painted, it doesn't connect
            break;
        }
        
        // try moving forwards
        while (xmin < xmax)
        {
            if (fill_test(xmin, y))
                break;
            ++xmin;
        }
        if (xmin == xmax && fill_test(xmax, y) == 0)
        {
            // nothing left paintable in this row
            ++xmin;
            break;
        }
        else
        {
            // we know for sure that xmin is possible
            xmax_current = xmin; 
            // so repeat the whole shindig
        }
    }
}}
