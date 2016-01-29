#include "common.h"
#include "simple.h"

struct cursor cursor;

void place_cursor(uint8_t x, uint8_t y)
{
    cursor.x = x;
    cursor.y = y;
    cursor.saved_attr = vram_attr[y][x];
    vram_attr[y][x] = cursor.attr;
}

void move_cursor(uint8_t x, uint8_t y)
{
    vram_attr[cursor.y][cursor.x] = cursor.saved_attr;
    place_cursor(x, y);
}

void move_cursor_to_x(uint8_t x)
{
    // relinquish attribute of current cursor position:
    vram_attr[cursor.y][cursor.x] = cursor.saved_attr;
    // update cursor
    cursor.x = x % SCREEN_W;
    // save this location's original attribute:
    cursor.saved_attr = vram_attr[cursor.y][cursor.x];
    // put in the cursor's:
    vram_attr[cursor.y][cursor.x] = cursor.attr;
}

void move_cursor_to_y(uint8_t y)
{
    // relinquish attribute of current cursor position:
    vram_attr[cursor.y][cursor.x] = cursor.saved_attr;
    // update cursor
    cursor.y = y % SCREEN_H;
    // save this location's original attribute:
    cursor.saved_attr = vram_attr[cursor.y][cursor.x];
    // put in the cursor's:
    vram_attr[cursor.y][cursor.x] = cursor.attr;
}

void move_cursor_y(uint8_t direction)
{
    // relinquish attribute of current cursor position:
    vram_attr[cursor.y][cursor.x] = cursor.saved_attr;
    cursor.y = (cursor.y + direction) % SCREEN_H; // move in y dir
    // save this location's original attribute:
    cursor.saved_attr = vram_attr[cursor.y][cursor.x];
    // put in the cursor's:
    vram_attr[cursor.y][cursor.x] = cursor.attr;
}

void move_cursor_up()
{
    if (cursor.y)
    {
        // relinquish attribute of current cursor position:
        vram_attr[cursor.y--][cursor.x] = cursor.saved_attr;
        // save this location's original attribute:
        cursor.saved_attr = vram_attr[cursor.y][cursor.x];
        // put in the cursor's:
        vram_attr[cursor.y][cursor.x] = cursor.attr;
    }
}

void move_cursor_down()
{
    if (cursor.y < SCREEN_H-1)
    {
        // relinquish attribute of current cursor position:
        vram_attr[cursor.y++][cursor.x] = cursor.saved_attr;
        // save this location's original attribute:
        cursor.saved_attr = vram_attr[cursor.y][cursor.x];
        // put in the cursor's:
        vram_attr[cursor.y][cursor.x] = cursor.attr;
    }
}

void move_cursor_x(uint8_t direction)
{
    // relinquish attribute of current cursor position:
    vram_attr[cursor.y][cursor.x] = cursor.saved_attr;
    cursor.x = (cursor.x + direction) % SCREEN_W; // move in x dir
    // save this location's original attribute:
    cursor.saved_attr = vram_attr[cursor.y][cursor.x];
    // put in the cursor's:
    vram_attr[cursor.y][cursor.x] = cursor.attr;
}

void move_cursor_left()
{
    if (cursor.x)
    {
        // relinquish attribute of current cursor position:
        vram_attr[cursor.y][cursor.x--] = cursor.saved_attr;
        // save this location's original attribute:
        cursor.saved_attr = vram_attr[cursor.y][cursor.x];
        // put in the cursor's:
        vram_attr[cursor.y][cursor.x] = cursor.attr;
    }
}

void move_cursor_right()
{
    if (cursor.x < SCREEN_W-1)
    {
        // relinquish attribute of current cursor position:
        vram_attr[cursor.y][cursor.x++] = cursor.saved_attr;
        // save this location's original attribute:
        cursor.saved_attr = vram_attr[cursor.y][cursor.x];
        // put in the cursor's:
        vram_attr[cursor.y][cursor.x] = cursor.attr;
    }
}

