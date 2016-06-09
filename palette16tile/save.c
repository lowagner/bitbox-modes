#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "io.h"

#include <string.h> // memset

#define OFFSET_X 230 // offset for alphabet square
#define SAVE_COLOR 5 // a uint8_t, uint16_t color is (SAVE_COLOR)|(SAVE_COLOR<<8)

uint8_t save_position CCM_MEMORY; // position in the base filename
uint8_t save_not_load CCM_MEMORY; // whether to be in save (1) or load (0)
uint8_t save_only CCM_MEMORY; // 0 - everything, 1 - tiles, 2 - sprites, 3 - map, 4 - palette
int save_text_offset CCM_MEMORY;

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
            if (save_text_offset < 0 || save_text_offset > SCREEN_W - 20)
                save_text_offset = 0;
            // spot in the filename to write to
            uint16_t *dst = draw_buffer + save_text_offset + 1 + save_position * 9;
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
        else if (save_y+1 == line)
        {
            uint16_t *dst = draw_buffer + 1 + save_x * 9 + OFFSET_X;
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
            uint16_t *dst = draw_buffer + 1 + save_x * 9 + OFFSET_X;
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
            save_text_offset = 16 + 5*9;
            if (save_not_load)
                font_render_line_doubled((const uint8_t *)"save", 16, internal_line, 65535, SAVE_COLOR*257);
            else
                font_render_line_doubled((const uint8_t *)"load", 16, internal_line, 65535, 0);
            switch (save_only)
            {
            case 0:
                font_render_line_doubled((const uint8_t *)"all:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257*save_not_load);
                save_text_offset += 5*9;
                break;
            case 1:
                font_render_line_doubled((const uint8_t *)"tiles:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257*save_not_load);
                save_text_offset += 7*9;
                break;
            case 2:
                font_render_line_doubled((const uint8_t *)"sprites:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257*save_not_load);
                save_text_offset += 9*9;
                break;
            case 3:
                font_render_line_doubled((const uint8_t *)"map:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257*save_not_load);
                save_text_offset += 5*9;
                break;
            case 4:
                font_render_line_doubled((const uint8_t *)"palette:", 16 + 5*9, internal_line, 65535, SAVE_COLOR*257*save_not_load);
                save_text_offset += 9*9;
                break;
                
            }
            font_render_line_doubled((uint8_t *)base_filename, save_text_offset, internal_line, 65535, SAVE_COLOR*257*save_not_load);
        }
        else if (line <= 6)
        {
            font_render_line_doubled((const uint8_t *)allowed_chars[line-1], OFFSET_X, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            if (line-1 == save_y)
            {
                if (save_x < 5)
                {
                    {
                    uint16_t *dst = draw_buffer + (save_x * 9 + OFFSET_X);
                    const uint16_t color = save_colors[save_not_load][0];
                    *dst = color;
                    dst += 9;
                    *dst = color;
                    }
                    {
                    uint32_t *dst = (uint32_t *)draw_buffer + (1 + 6 * 9 + OFFSET_X)/2;
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
                    uint16_t *dst = draw_buffer + (save_x * 9 + OFFSET_X);
                    const uint16_t color = save_colors[save_not_load][0];
                    *dst = color;
                    }
                    {
                    uint32_t *dst = (uint32_t *)draw_buffer + (1 + 6 * 9 + OFFSET_X)/2;
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
            font_render_line_doubled((const uint8_t *)"X:delete  Y:overwrite", 16, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        case 8:
            font_render_line_doubled((const uint8_t *)"A:advance B:backtrack", 16, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        case 9:
            if (save_not_load)
                font_render_line_doubled((const uint8_t *)"<L/R>:selective save", 16, internal_line, 65535, SAVE_COLOR*257);
            else
                font_render_line_doubled((const uint8_t *)"<L/R>:selective load", 16, internal_line, 65535, 0);
            break;
        case 10:
            break;
        case 11:
            font_render_line_doubled(game_message, 32, internal_line, 65535, SAVE_COLOR*257*save_not_load);
            break;
        }
    }
}

void save_overwrite_character()
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
}

void save_backspace_character()
{
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
}

void save_delete_character()
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
}

void save_insert_character()
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

    if (GAMEPAD_PRESS(0, A))
    {
        if (base_filename[save_position] != 0 && save_position < 7)
            ++save_position;
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, B))
    {
        if (save_position)
            --save_position;
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, X))
    {
        save_delete_character();
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        save_overwrite_character();
        game_message[0] = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, L))
    {
        if (save_not_load) // consider SAVE to the left of LOAD
        {
            if (save_only < 4) // and save_only is "absolute value"
                ++save_only; // go from tiles to sprites to map to palette 
        }
        else
        {
            if (save_only)
                --save_only;
            else
                save_not_load = 1; // switch over into save
        }
        return;
    } 
    if (GAMEPAD_PRESS(0, R))
    {
        if (save_not_load) // consider SAVE to the left of LOAD
        {
            if (save_only)
                --save_only;
            else
                save_not_load = 0; // switch over into load
        }
        else
        {
            if (save_only < 4)
                ++save_only; // go from tiles to sprites to map to palette 
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        save_only = 0;
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
        FileError result = BotchedIt;
        int offset = 0;
        switch (save_only)
        {
        case 0: // save all
            result = save_not_load ? io_save_tile(16) : io_load_tile(16);
            if (result != NoError)
            {
                strcpy((char *)game_message, "tiles ");
                offset = 6;
                break;
            }
            result = save_not_load ? io_save_sprite(16) : io_load_sprite(16);
            if (result != NoError)
            {
                strcpy((char *)game_message, "sprites ");
                offset = 8;
                break;
            }
            result = save_not_load ? io_save_map() : io_load_map();
            if (result != NoError)
            {
                strcpy((char *)game_message, "map ");
                offset = 4;
                break;
            }
            result = save_not_load ? io_save_palette() : io_load_palette();
            if (result != NoError)
            {
                strcpy((char *)game_message, "palette ");
                offset = 8;
                break;
            }
            break;
        case 1:
            result = save_not_load ? io_save_tile(16) : io_load_tile(16);
            if (result != NoError)
            {
                strcpy((char *)game_message, "tiles ");
                offset = 6;
            }
            break;
        case 2:
            result = save_not_load ? io_save_sprite(16) : io_load_sprite(16);
            if (result != NoError)
            {
                strcpy((char *)game_message, "sprites ");
                offset = 8;
            }
            break;
        case 3:
            result = save_not_load ? io_save_map() : io_load_map();
            if (result != NoError)
            {
                strcpy((char *)game_message, "map ");
                offset = 4;
            }
            break;
        case 4:
            result = save_not_load ? io_save_palette() : io_load_palette();
            if (result != NoError)
            {
                strcpy((char *)game_message, "palette ");
                offset = 8;
            }
            break;
        }
       
        switch (result)
        {
        case NoError:
            if (save_not_load)
                strcpy((char *)game_message + offset, "saved!");
            else
                strcpy((char *)game_message + offset, "loaded!");
            break;
        case MountError:
            strcpy((char *)game_message + offset, "fs unmounted!");
            break;
        case ConstraintError:
            strcpy((char *)game_message + offset, "unconstrained!");
            break;
        case OpenError:
            strcpy((char *)game_message + offset, "no open!");
            break;
        case ReadError:
            strcpy((char *)game_message + offset, "no read!");
            break;
        case WriteError:
            strcpy((char *)game_message + offset, "no write!");
            break;
        case NoDataError:
            strcpy((char *)game_message + offset, "no data!");
            break;
        case MissingDataError:
            strcpy((char *)game_message + offset, "miss data!");
            break;
        case BotchedIt:
            strcpy((char *)game_message + offset, "fully bungled.");
            break;
        }
        return;
    }
}
