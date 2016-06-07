#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "io.h"

#include <string.h> // memset

#define OFFSET_X 40
#define SAVE_COLOR 5 // a uint8_t, uint16_t color is (SAVE_COLOR)|(SAVE_COLOR<<8)

char base_filename[9] CCM_MEMORY; // up to 8 characters, plus a zero
uint8_t save_position CCM_MEMORY; // position in the base filename
uint8_t save_not_load CCM_MEMORY; // whether to be in save (1) or load (0)

uint8_t save_x CCM_MEMORY, save_y CCM_MEMORY; // position in alphabet table (below):
static const uint8_t allowed_chars[6][7] = {
    {'E', 'T', 'A', 'O', 'I', 'N', 0},
    {'S', 'R', 'H', 'L', 'D', 'C', 0},
    {'U', 'M', 'W', 'F', 'G', 'P', 0},
    {'Y', 'B', 'V', 'K', 'X', 'J', 0},
    {'Q', 'Z', '0', '1', '2', '3', 0},
    {'4', '5', '6', '7', '8', '9', 0}
};

static const uint16_t save_colors[2][2] = {
    { RGB(255, 0, 0), RGB(0, 255, 0) }, // load colors 
    { RGB(0, 0, 0), RGB(0, 255, 255) }  // save colors
};

void save_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, SAVE_COLOR*save_not_load, 2*SCREEN_W);
        return;
    }
    else if (vga_line/2 == (SCREEN_H - 20)/2)
    {
        memset(draw_buffer, SAVE_COLOR*save_not_load, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + 12*10)
    {
        if (vga_line/2 == (22 + 12*10)/2)
            memset(draw_buffer, SAVE_COLOR*save_not_load, 2*SCREEN_W);
        else if (save_not_load == 0)
        {
            // load mode

        }
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, SAVE_COLOR*save_not_load, 2*SCREEN_W);
        // also check for character selector
        if (line == 0)
        {
            // spot in the filename to write to
            uint16_t *dst = draw_buffer + OFFSET_X + 11*9 + 1 + save_position * 9;
            const uint16_t color = save_colors[save_not_load][0];;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
        }
        else if (save_y+1 == line)
        {
            uint16_t *dst = draw_buffer + 20*9 + 1 + save_x * 9 + OFFSET_X;
            const uint16_t color = save_colors[save_not_load][0];
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
        }
        else if (line == 6 && internal_line == 9)
        {
            uint16_t *dst = draw_buffer + 20*9 + 1 + save_x * 9 + OFFSET_X;
            const uint16_t color = save_colors[save_not_load][1];
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
            *dst++ = color;
        }
    }
    else
    {
        --internal_line;
        if (line == 0)
        {
            if (save_not_load)
                font_render_line_doubled((const uint8_t *)"save file:", OFFSET_X, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            else
                font_render_line_doubled((const uint8_t *)"load file:", OFFSET_X, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            font_render_line_doubled((uint8_t *)base_filename, 9*11 + OFFSET_X, internal_line, 65535, SAVE_COLOR*257*save_not_load);
        }
        else if (line <= 6)
        {
            font_render_line_doubled((const uint8_t *)allowed_chars[line-1], OFFSET_X + 20*9, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            if (line-1 == save_y)
            {
                if (save_x < 5)
                {
                    {
                    uint16_t *dst = draw_buffer + (20*9 + save_x * 9 + OFFSET_X);
                    const uint16_t color = save_colors[save_not_load][0];
                    *dst = color;
                    dst += 9;
                    *dst = color;
                    }
                    {
                    uint32_t *dst = (uint32_t *)draw_buffer + (20*9 + 1 + 6 * 9 + OFFSET_X)/2;
                    const uint32_t color = save_not_load ? (save_colors[1][1] | ((SAVE_COLOR*257) << 16)) : save_colors[0][1];
                    *dst++ = color;
                    *dst++ = color;
                    *dst++ = color;
                    *dst++ = color;
                    *dst++ = color;
                    }
                }
                else
                {
                    {
                    uint16_t *dst = draw_buffer + (20*9 + save_x * 9 + OFFSET_X);
                    const uint16_t color = save_colors[save_not_load][0];
                    *dst = color;
                    }
                    {
                    uint32_t *dst = (uint32_t *)draw_buffer + (20*9 + 1 + 6 * 9 + OFFSET_X)/2;
                    const uint32_t color = save_not_load ? (save_colors[1][0] | ((SAVE_COLOR*257) << 16)) : save_colors[0][0];
                    *dst++ = color;
                    *dst++ = color;
                    *dst++ = color;
                    *dst++ = color;
                    *dst++ = color;
                    }
                }
            }
        }
        else
        switch (line)
        {
        case 7:
            font_render_line_doubled((const uint8_t *)"Y:insert    X:delete", 16, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        case 8:
            font_render_line_doubled((const uint8_t *)"A:overwrite B:backspace", 16, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        case 9:
            font_render_line_doubled((const uint8_t *)"<L/R>:move cursor", 16, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        case 10:
            break;
        case 11:
            font_render_line_doubled(game_message, 32, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        }
    }
}

void save_controls()
{
    int make_wait = 0;
    if (GAMEPAD_PRESSING(0, left))
    {
        if (save_x)
            --save_x;
        else
            save_x = 5;
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        if (save_x < 5)
            ++save_x;
        else
            save_x = 0;
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        if (save_y)
            --save_y;
        else
            save_y = 5;
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        if (save_y < 5)
            ++save_y;
        else
            save_y = 0;
        make_wait = 1;
    }
    if (make_wait)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;

    /*
    controls:
        A - overwrite (and advance)
        X - delete (and move everything back)
        Y - insert (and advance)
        B - backspace (and move everything back)
        select - switch to a different mode (without saving)
        start - save (NOT IMPLEMENTED)
     */
    if (GAMEPAD_PRESS(0, A))
    {
        char previous = base_filename[save_position];
        base_filename[save_position] = allowed_chars[save_y][save_x];
        if (save_position < 7) 
        {
            ++save_position;
            if (previous == 0)
                base_filename[save_position] = 0;
        }
        else
            base_filename[8] = 0;
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, B))
    {
        // backspace
        if (save_position == 7)
        {
            if (base_filename[save_position])
                base_filename[save_position] = 0;
            else
                base_filename[--save_position] = 0;
        }
        else
            // if inside, move everything back
        if (save_position) // somewhere in the middle
        {
            for (int i=save_position-1; i<7; ++i)
            {
                base_filename[i] = base_filename[i+1];
                if (base_filename[i] == 0)
                    break;
            }
            // ensure that we took something out, the end is zeroed.
            // shouldn't be necessary, but doesn't cost much:
            base_filename[7] = 0;
            // also move back
            --save_position;
        }
        else
        {
            for (int i=0; i<7; ++i)
            {
                base_filename[i] = base_filename[i+1];
                if (base_filename[i] == 0)
                    break;
            }
            // ensure that we took something out, the end is zeroed.
            // shouldn't be necessary, but doesn't cost much:
            base_filename[7] = 0;
        }
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, X))
    {
        // delete (and move other elements down), don't move cursor
        if (save_position == 7)
        {
            base_filename[save_position] = 0;
        }
        else
        {   
            for (int i=save_position; i<7; ++i)
            {
                base_filename[i] = base_filename[i+1];
                if (base_filename[i] == 0)
                    break;
            }
            // ensure that we took something out, the end is zeroed.
            // shouldn't be necessary, but doesn't cost much:
            base_filename[7] = 0;
        }
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        // insert (and move up everything else)
        if (save_position < 7)
        {
            base_filename[8] = 0; // override anything that would get moved up here
            for (int i=6; i>=save_position; --i)
            {
                base_filename[i+1] = base_filename[i];
            }
            base_filename[save_position++] = allowed_chars[save_y][save_x];
        }
        else
        {
            base_filename[save_position] = allowed_chars[save_y][save_x];
        }
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, L))
    {
        if (save_position)
            --save_position;
        return;
    } 
    if (GAMEPAD_PRESS(0, R))
    {
        if (base_filename[save_position] != 0 && save_position < 7)
            ++save_position;
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        if (save_not_load)
        {
            save_not_load = 0;
            game_message[0] = 0;
        }
        else
        {
            visual_mode = TilesAndSprites; // switch visual mode
            save_not_load = 1; // but next time we'll come back to save
            game_message[0] = 0;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        FileError result = save_not_load ? io_save_tile(16) : io_load_tile(16);
        switch (result)
        {
        case NoError:
            if (save_not_load)
                strcpy((char *)game_message, "tiles saved!");
            else
                strcpy((char *)game_message, "tiles loaded!");
            break;
        case MountError:
            strcpy((char *)game_message, "file-system not mounted!");
            break;
        case ConstraintError:
            strcpy((char *)game_message, "constraints not satisfied!");
            break;
        case OpenError:
            strcpy((char *)game_message, "could not open file!");
            break;
        case ReadError:
            strcpy((char *)game_message, "could not read file!");
            break;
        case WriteError:
            strcpy((char *)game_message, "could not write file!");
            break;
        case NoDataError:
            strcpy((char *)game_message, "no data read/written!");
            break;
        case MissingDataError:
            strcpy((char *)game_message, "not all data read/written!");
            break;
        }
        return;
    }
}