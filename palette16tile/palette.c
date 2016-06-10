#include "palette.h"
#include "common.h"
#include "bitbox.h"
#include "font.h"
#include "name.h"
#include "io.h"

#include "string.h" //memcpy

uint16_t palette[16] CCM_MEMORY; 

void palette_reset()
{
    static const uint16_t colors[16] = {
        [BLACK]=RGB(0, 0, 0),
        [GRAY]=RGB(157, 157, 157),
        [WHITE]=(1<<15) - 1,
        [PINK]=RGB(224, 111, 139),
        [RED]=RGB(190, 38, 51),
        [BLAZE]=RGB(235, 137, 49),
        [BROWN]=RGB(164, 100, 34),
        [DULLBROWN]=RGB(73, 60, 43),
        [GINGER]=RGB(247, 226, 107),
        [SLIME]=RGB(163, 206, 39),
        [GREEN]=RGB(68, 137, 26),
        [BLUEGREEN]=RGB(47, 72, 78),
        [CLOUDBLUE]=RGB(178, 220, 239),
        [SKYBLUE]=RGB(49, 162, 242),
        [SEABLUE]=RGB(0, 87, 132),
        [INDIGO]=RGB(28, 20, 40),
    };
    memcpy(palette, colors, sizeof(colors));
}

#define NUMBER_LINES 11

uint8_t palette_index CCM_MEMORY;
uint8_t palette_selector CCM_MEMORY; 
uint16_t palette_copying CCM_MEMORY;

void palette_init()
{
    palette_index = 0;
    palette_selector = 0;
    palette_copying = 32768;
}

void palette_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    else if (vga_line/2 == (SCREEN_H - 20)/2)
    {
        memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + NUMBER_LINES*10)
    {
        if (vga_line/2 == (22 + NUMBER_LINES*10)/2)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, 0, 2*SCREEN_W);
    }
    else
    {
        --internal_line;
        switch (line)
        {
        case 0:
            font_render_line_doubled((uint8_t *)"palette in", 16, internal_line, 65535, 0);
            font_render_line_doubled((uint8_t *)base_filename, 16+9*11, internal_line, 65535, 0);
            break;
        case 2:
            font_render_line_doubled((const uint8_t *)"L/R:cycle index", 16, internal_line, 65535, 0);
            {
            uint8_t label[3] = { hex[palette_index], ':', 0 };
            font_render_line_doubled(label, 16 + 9*22, internal_line, 65535, 0);
            }
            break;
        case 3:
            if (palette_copying < 32768)
                font_render_line_doubled((const uint8_t *)"A:cancel copy", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"A:save all", 16+2*9, internal_line, 65535, 0);
            break;
        case 4:
            if (palette_copying < 32768)
                font_render_line_doubled((const uint8_t *)"X:  \"     \"", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"X:load all", 16+2*9, internal_line, 65535, 0);
            break;
        case 5:
            if (palette_copying < 32768)
                font_render_line_doubled((const uint8_t *)"B:  \"     \"", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"B:copy", 16+2*9, internal_line, 65535, 0);
            break;
        case 6:
            if (palette_copying < 32768)
                font_render_line_doubled((const uint8_t *)"Y:paste", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"Y:change filename", 16+2*9, internal_line, 65535, 0);
        case 7:
            break;
        case 8:
            font_render_line_doubled((const uint8_t *)"start: return", 16, internal_line, 65535, 0);
            {
            uint8_t label[] = { 'r', ':', hex[(palette[palette_index]>>10)&31], ' ', 'g', ':', hex[(palette[palette_index]>>5)&31], ' ', 'b', ':', hex[(palette[palette_index])&31], 0 };
            font_render_line_doubled(label, 16+21*9, internal_line, 65535, 0);
            }
            break;
        case 10:
            font_render_line_doubled(game_message, 16, internal_line, 65535, 0);
            break;
        }
    }
    if (vga_line < 22 + 2*16)
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (SCREEN_W - 24 - 16*2*2)/2 - 1;
        uint32_t color = palette[palette_index] | (palette[palette_index]<<16);
        for (int l=0; l<16; ++l) 
            *(++dst) = color;
    }
    else if (vga_line/2 == (22 + 2*16)/2)
    {
        memset(draw_buffer+(SCREEN_W - 24 - 16*2*2), 0, 64);
    }
    else if (vga_line < 22 + 2 + 2*16 + 2*16 && palette_copying < 32768)
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (SCREEN_W - 24 - 16*2)/2;
        uint32_t color = palette_copying | (palette_copying << 16);
        for (int l=0; l<16; ++l) 
            *(++dst) = color;
    }
}

void palette_controls()
{
    int moved = 0;
    if (GAMEPAD_PRESSING(0, R))
    {
        ++moved;
    }
    if (GAMEPAD_PRESSING(0, L))
    {
        --moved;
    }
    if (moved)
    {
        game_message[0] = 0;
        palette_index = (palette_index + moved)&15;
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        return;
    }
    int make_wait = 0;
    if (GAMEPAD_PRESSING(0, left))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        make_wait = 1;
    }
    if (make_wait)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;

    if (GAMEPAD_PRESS(0, B))
    {
        // copy or uncopy
        if (palette_copying < 32768)
            palette_copying = 32768;
        else
            palette_copying = palette[palette_index];
        return;
    }
    int save_or_load = 0;
    if (GAMEPAD_PRESS(0, A))
    {
        // save
        save_or_load = 1;
    }
    else if (GAMEPAD_PRESS(0, X))
    {
        // load
        save_or_load = 2;
    }
    if (save_or_load)
    {
        if (palette_copying < 32768)
        {
            // or cancel a copy
            palette_copying = 32768;
            return;
        }
            
        FileError error;
        if (save_or_load == 1) // save
        {
            error = io_save_palette();
        }
        else // load
        {
            error = io_load_palette();
        }
        switch (error)
        {
        case NoError:
            if (save_or_load == 1)
                strcpy((char *)game_message, "saved!");
            else
                strcpy((char *)game_message, "loaded!");
            break;
        case MountError:
            strcpy((char *)game_message, "fs unmounted!");
            break;
        case ConstraintError:
            strcpy((char *)game_message, "unconstrained!");
            break;
        case OpenError:
            strcpy((char *)game_message, "no open!");
            break;
        case ReadError:
            strcpy((char *)game_message, "no read!");
            break;
        case WriteError:
            strcpy((char *)game_message, "no write!");
            break;
        case NoDataError:
            strcpy((char *)game_message, "no data!");
            break;
        case MissingDataError:
            strcpy((char *)game_message, "miss data!");
            break;
        case BotchedIt:
            strcpy((char *)game_message, "fully bungled.");
            break;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        if (palette_copying < 32768)
        {
            // paste:
            strcpy((char *)game_message, "pasted.");
            palette[palette_index] = palette_copying;
            palette_copying = 32768;
        }
        else
        {
            // go to filename chooser
            game_message[0] = 0;
            previous_visual_mode = EditPalette;
            visual_mode = ChooseFilename;
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        visual_mode = EditTileOrSprite;
        previous_visual_mode = None;
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        if (previous_visual_mode)
        {
            visual_mode = previous_visual_mode;
            previous_visual_mode = None;
        }
        else
        {
            visual_mode = EditTileOrSprite;
        }
        return;
    }
}
