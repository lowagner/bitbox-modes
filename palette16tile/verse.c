#include "bitbox.h"
#include "common.h"
#include "common.h"
#include "chiptune.h"
#include "instrument.h"
#include "name.h"
#include "font.h"
#include "io.h"

#include <stdlib.h> // rand
#include <string.h> // memset

#define BG_COLOR 132
#define BOX_COLOR (RGB(200, 200, 230)|(RGB(200, 200, 230)<<16))
#define MATRIX_WING_COLOR (RGB(30, 90, 90) | (RGB(30, 90, 90)<<16))
#define NUMBER_LINES 20

uint8_t verse_track CCM_MEMORY;
uint8_t verse_track_pos CCM_MEMORY;
uint8_t verse_track_offset CCM_MEMORY;
uint8_t verse_menu_not_edit CCM_MEMORY;
uint8_t verse_copying CCM_MEMORY;
uint8_t verse_color[2] CCM_MEMORY;
uint8_t verse_last_painted CCM_MEMORY;

void key_name(uint8_t *name, uint8_t key)
{
    if (key < 12)
    {
        name[0] = note_name[key][0];
        name[1] = note_name[key][1];
    }
    else if (key < 24)
    {
        name[0] = note_name[key-12][0] + 32;
        name[1] = note_name[key-12][1];
    }
    else
    {
        name[0] = 'n';
        name[1] = 'o';
    }
}

void verse_init()
{
    verse_track = 0;
    verse_track_pos = 1;
    verse_track_offset = 1;
    verse_color[1] = 0;
    verse_color[0] = 4;
}

void verse_reset()
{
}

void render_command(uint8_t value, int x, int y)
{
    value &= 15;
    uint32_t *dst = (uint32_t *)draw_buffer + x/2;
    uint8_t row = (font[hex[value]] >> (((y/2)*4))) & 15;
    const uint32_t color_choice[2] = { 
          palette[value] | (palette[value]<<16),
        ~(palette[value] | (palette[value]<<16))
    };
    *(++dst) = color_choice[0];
    *(++dst) = color_choice[0];
    for (int k=0; k<4; ++k)
    {
        *(++dst) = color_choice[row&1];
        row >>= 1;
    }
    *(++dst) = color_choice[0];
    *(++dst) = color_choice[0];
}

void verse_line()
{
    if (vga_line < 16)
    {
        if (vga_line/2 == 0)
        {
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
            return;
        }
        return;
    }
    else if (vga_line >= 16 + NUMBER_LINES*10)
    {
        if (vga_line/2 == (16 +NUMBER_LINES*10)/2)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-16) / 10;
    int internal_line = (vga_line-16) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        if (line > 2 && line < 7)
        {
            if (line - 3 == instrument_i)
            {
                uint32_t *dst = (uint32_t *)draw_buffer + 25 + 8*verse_track_pos - 8*verse_track_offset;
                (*dst) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
                (*(++dst)) = BOX_COLOR;
            }
            else if (line == 6 && internal_line == 9)
            {
                uint32_t *dst = (uint32_t *)draw_buffer + 25 + 8*verse_track_pos - 8*verse_track_offset;
                (*dst) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
                (*(++dst)) = MATRIX_WING_COLOR;
            }
        }
        return;
    }
    --internal_line;
    uint8_t buffer[24];
    switch (line)
    {
        case 0:
        {
            // edit track
            uint8_t msg[] = {  't', 'r', 'a', 'c', 'k', ' ', hex[verse_track], 
                '.', hex[instrument_i], 
                ' ', 'X', '0' + verse_track_pos/10, '0' + verse_track_pos%10, '/',
                '0' + track_length/10, '0' + track_length%10,
            0 };
            font_render_line_doubled(msg, 12, internal_line, 65535, BG_COLOR*257);
            break;
        }
        case 2:
            font_render_line_doubled((uint8_t *)"I:key", 12, internal_line, 65535, BG_COLOR*257);
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        {
            int i = line - 3;
            if (i == instrument_i)
            {
                uint32_t *dst = (uint32_t *)draw_buffer + 3;
                if ((internal_line+1)/2 == 1)
                {
                    *dst = ~(*dst);
                    ++dst;
                    *dst = ~(*dst);
                }
                else if ((internal_line+1)/2 == 3)
                {
                    *dst = 16843009u*BG_COLOR;
                    ++dst;
                    *dst = 16843009u*BG_COLOR;
                }
            }
            {
                uint8_t key[2];
                key_name(key, chip_track[verse_track][i][0]);
                uint8_t msg[] = { hex[i], ':', key[0], key[1], 0 };
                font_render_line_doubled(msg, 13, internal_line, 65535, BG_COLOR*257);
                uint32_t *dst = (uint32_t *)draw_buffer + 24;
                uint8_t y = ((internal_line/2))*4; // how much to shift for font row
                if (verse_track_offset%2)
                {
                    // this is normal unpacking of nibbles, 
                    // since verse_track_offset should be 1 for the start of the track values.
                    uint8_t *value = &chip_track[verse_track][i][verse_track_offset/2];
                    for (int j=0; j<8; ++j)
                    {
                        uint8_t command = (*(++value))&15;
                        uint8_t row = (font[hex[command]] >> y) & 15;
                        uint32_t color_choice[2];
                        color_choice[0] = palette[command] | (palette[command]<<16);
                        color_choice[1] = ~color_choice[0];

                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
                        for (int k=0; k<4; ++k)
                        {
                            *(++dst) = color_choice[row&1];
                            row >>= 1;
                        }
                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];

                        command = (*value)>>4;
                        row = (font[hex[command]] >> y) & 15;
                        color_choice[0] = palette[command] | (palette[command]<<16);
                        color_choice[1] = ~color_choice[0];
                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
                        for (int k=0; k<4; ++k)
                        {
                            *(++dst) = color_choice[row&1];
                            row >>= 1;
                        }
                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
                    }
                }
                else
                {
                    // this is offset unpacking of nibbles.
                    uint8_t *value = &chip_track[verse_track][i][verse_track_offset/2];
                    for (int j=0; j<8; ++j)
                    {
                        uint8_t command = (*value)>>4;
                        uint8_t row = (font[hex[command]] >> y) & 15;
                        uint32_t color_choice[2];
                        color_choice[0] = palette[command] | (palette[command]<<16);
                        color_choice[1] = ~color_choice[0];

                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
                        for (int k=0; k<4; ++k)
                        {
                            *(++dst) = color_choice[row&1];
                            row >>= 1;
                        }
                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
    
                        command = (*(++value))&15;
                        row = (font[hex[command]] >> y) & 15;
                        color_choice[0] = palette[command] | (palette[command]<<16);
                        color_choice[1] = ~color_choice[0];
                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
                        for (int k=0; k<4; ++k)
                        {
                            *(++dst) = color_choice[row&1];
                            row >>= 1;
                        }
                        *(++dst) = color_choice[0];
                        *(++dst) = color_choice[0];
                    }
                }
            }
            if (i == instrument_i)
            {
                const uint16_t color = (verse_track_offset + 15 == verse_track_pos) ? 
                    BOX_COLOR :
                    MATRIX_WING_COLOR;
                uint16_t *dst = draw_buffer + 24*2 + 16*8*2;
                *(++dst) = color;
                ++dst;
                *(++dst) = color;
                ++dst;
                *(++dst) = color;
                ++dst;
                *(++dst) = color;
                ++dst;
                *(++dst) = color;
            }

            break;
        }
        case 8:
            if (verse_menu_not_edit)
                font_render_line_doubled((uint8_t *)"dpad:adjust key", 12, internal_line, 65535, BG_COLOR*257);
            else
                font_render_line_doubled((uint8_t *)"dpad:move cursor", 12, internal_line, 65535, BG_COLOR*257);
            break;
        case 9:
            if (verse_menu_not_edit)
            {
                if (verse_copying < 16)
                    font_render_line_doubled((uint8_t *)"A:cancel copy", 12+3*9, internal_line, 65535, BG_COLOR*257);
                else
                    font_render_line_doubled((uint8_t *)"A:save to file", 12+3*9, internal_line, 65535, BG_COLOR*257);
            }
            else
            {
                if (chip_play_track)
                    font_render_line_doubled((uint8_t *)"A:stop", 12+3*9, internal_line, 65535, BG_COLOR*257);
                else
                    font_render_line_doubled((uint8_t *)"A:play", 12+3*9, internal_line, 65535, BG_COLOR*257);
            }
            break;
        case 10:
            if (verse_menu_not_edit)
            {
                if (verse_copying < 16)
                    font_render_line_doubled((uint8_t *)"X:  \"     \"", 12+3*9, internal_line, 65535, BG_COLOR*257);
                else
                    font_render_line_doubled((uint8_t *)"X:load from file", 12+3*9, internal_line, 65535, BG_COLOR*257);
            }
            else
                font_render_line_doubled((uint8_t *)"X:edit instrument", 12+3*9, internal_line, 65535, BG_COLOR*257);
            break;
        case 11:
            if (verse_menu_not_edit)
            {
                if (verse_copying < 16)
                    font_render_line_doubled((uint8_t *)"B:  \"     \"", 12+3*9, internal_line, 65535, BG_COLOR*257);
                else
                    font_render_line_doubled((uint8_t *)"B:copy lines", 12+3*9, internal_line, 65535, BG_COLOR*257);
            }
            else
            {
                font_render_line_doubled((uint8_t *)"B:put", 12+3*9, internal_line, 65535, BG_COLOR*257);
                render_command(verse_color[1], 12+3*9+6*9, internal_line);
                if (verse_last_painted)
                {
                    font_render_line_doubled((uint8_t *)"L:", 150, internal_line, 65535, BG_COLOR*257);
                    render_command(verse_color[1]-1, 150+2*9, internal_line);
                    font_render_line_doubled((uint8_t *)"R:", 200, internal_line, 65535, BG_COLOR*257);
                    render_command(verse_color[1]+1, 200+2*9, internal_line);
                }
            }
            break;
        case 12:
            if (verse_menu_not_edit)
            {
                if (verse_copying < 16)
                    font_render_line_doubled((uint8_t *)"Y:paste", 12+3*9, internal_line, 65535, BG_COLOR*257);
                else
                {
                    strcpy((char *)buffer, "Y:file ");
                    strcpy((char *)(buffer+7), base_filename);
                    font_render_line_doubled(buffer, 12+3*9, internal_line, 65535, BG_COLOR*257);
                }
            }
            else
            {
                font_render_line_doubled((uint8_t *)"Y:put", 12+3*9, internal_line, 65535, BG_COLOR*257);
                render_command(verse_color[0], 12+3*9+6*9, internal_line);
                if (!verse_last_painted)
                {
                    font_render_line_doubled((uint8_t *)"L:", 150, internal_line, 65535, BG_COLOR*257);
                    render_command(verse_color[0]-1, 150+2*9, internal_line);
                    font_render_line_doubled((uint8_t *)"R:", 200, internal_line, 65535, BG_COLOR*257);
                    render_command(verse_color[0]+1, 200+2*9, internal_line);
                }
            }
            break;
        case 14:
            if (verse_menu_not_edit)
                font_render_line_doubled((uint8_t *)"start:edit track", 12, internal_line, 65535, BG_COLOR*257);
            else
                font_render_line_doubled((uint8_t *)"start:track menu", 12, internal_line, 65535, BG_COLOR*257);
            break;
        case 15:
            font_render_line_doubled((uint8_t *)"select:back to anthem", 12, internal_line, 65535, BG_COLOR*257);
            break;
        case 17:
            render_command(verse_color[verse_last_painted], 12, internal_line);
            switch (verse_color[verse_last_painted])
            {
                case 0:
                    font_render_line_doubled((uint8_t *)":silence", 30, internal_line, 65535, BG_COLOR*257);
                    break;
                case 1:
                    font_render_line_doubled((uint8_t *)":fade in/out", 30, internal_line, 65535, BG_COLOR*257);
                    break;
                case 2:
                    font_render_line_doubled((uint8_t *)":repeat note", 30, internal_line, 65535, BG_COLOR*257);
                    break;
                case 3:
                    font_render_line_doubled((uint8_t *)":control note", 30, internal_line, 65535, BG_COLOR*257);
                    break;
                default:
                {
                    uint8_t msg[12] = { ':', 'n', 'o', 't', 'e', ' ', 0, 0, 0 };
                    key_name(msg+6, verse_color[verse_last_painted]-4);
                    
                    font_render_line_doubled(msg, 30, internal_line, 65535, BG_COLOR*257);
                    break;
                }
            }
            break;
        case 19:
            font_render_line_doubled(game_message, 12, internal_line, 65535, BG_COLOR*257);
            break;
            
    }
}

uint8_t verse_track_color()
{
    if (verse_track_pos == 0)
        message ("ERROR!  shouldn't get pos= 0 in verse_track_color\n");
    uint8_t value = chip_track[verse_track][instrument_i][(verse_track_pos+1)/2];
    if (verse_track_pos % 2) // note the following are switched because of the pos-1 offset.
        return value & 15;
    else
        return value >> 4;
}

void verse_track_paint(uint8_t p)
{
    if (verse_track_pos == 0)
        message ("ERROR!  shouldn't get pos= 0 in verse_track_paint\n");
    verse_last_painted = p;

    uint8_t *memory = &chip_track[verse_track][instrument_i][(verse_track_pos+1)/2];
    if (verse_track_pos % 2) // note the following are switched because of the pos-1 offset.
        *memory = (verse_color[p]) | ((*memory) & 240);
    else
        *memory = ((*memory)&15) | (verse_color[p]<<4);
}

void verse_controls()
{
    if (verse_menu_not_edit)
    {
        int movement = 0;
        if (GAMEPAD_PRESSING(0, down))
        {
            instrument_i = (instrument_i+1)&3;
            movement = 1;
        }
        if (GAMEPAD_PRESSING(0, up))
        {
            instrument_i = (instrument_i-1)&3;
            movement = 1;
        }
        if (GAMEPAD_PRESSING(0, left))
        {
            if (--chip_track[verse_track][instrument_i][0] == 255)
                chip_track[verse_track][instrument_i][0] = 24;
            movement = 1;
        }
        if (GAMEPAD_PRESSING(0, right))
        {
            if (++chip_track[verse_track][instrument_i][0] > 24)
                chip_track[verse_track][instrument_i][0] = 0;
            movement = 1;
        }
        if (movement)
        {
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;
            return;
        }
        
        int save_or_load = 0;
        if (GAMEPAD_PRESS(0, A))
            save_or_load = 1; // save
        if (GAMEPAD_PRESS(0, X))
            save_or_load = 2; // load
        if (save_or_load)
        {
            if (verse_copying < 16)
            {
                // cancel a copy 
                verse_copying = 16;
                game_message[0] = 0;
                return;
            }

            FileError error = BotchedIt;
            if (save_or_load == 1)
                error = io_save_verse(verse_track);
            else
                error = io_load_verse(verse_track);
            io_message_from_error(game_message, error, save_or_load);
            return;
        }
        int switched = 0;
        if (GAMEPAD_PRESSING(0, L))
            --switched;
        if (GAMEPAD_PRESSING(0, R))
            ++switched;
        if (switched)
        {
            verse_track = (verse_track+switched)&15;
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;
            return;
        }
        if (GAMEPAD_PRESS(0, Y))
        {
            if (verse_copying < 16)
            {
                // paste
                if (verse_copying == verse_track)
                {
                    verse_copying = 16;
                    strcpy((char *)game_message, "pasting to same thing"); 
                    return;
                }
                uint8_t *src, *dst;
                src = &chip_track[verse_copying][0][0];
                dst = &chip_track[verse_track][0][0];
                memcpy(dst, src, sizeof(chip_track[0]));
                strcpy((char *)game_message, "pasted."); 
                verse_copying = 16;
            }
            else
            {
                // switch to choose name and hope to come back
                game_message[0] = 0;
                game_switch(ChooseFilename);
                previous_visual_mode = EditVerse;
            }
            return;
        }
        if (GAMEPAD_PRESS(0, B))
        {
            if (verse_copying < 16)
            {
                verse_copying = 16;
                game_message[0] = 0;
            }
            else
            {
                verse_copying = verse_track;
                strcpy((char *)game_message, "copied.");
            }
        }
    }
    else // editing, not menu
    {
        int paint_if_moved = 0;
        if (GAMEPAD_PRESSING(0, Y))
        {
            verse_track_paint(0);
            paint_if_moved = 1;
        }
        if (GAMEPAD_PRESSING(0, B))
        {
            verse_track_paint(1);
            paint_if_moved = 2;
        }

        int switched = 0;
        if (GAMEPAD_PRESSING(0, L))
            --switched;
        if (GAMEPAD_PRESSING(0, R))
            ++switched;
        if (switched)
            verse_color[verse_last_painted] = (verse_color[verse_last_painted]+switched)&15;

        int moved = 0;
        if (GAMEPAD_PRESSING(0, down))
        {
            instrument_i = (instrument_i+1)&3;
            moved = 1;
        }
        if (GAMEPAD_PRESSING(0, up))
        {
            instrument_i = (instrument_i-1)&3;
            moved = 1;
        }
        if (GAMEPAD_PRESSING(0, left))
        {
            if (--verse_track_pos == 0)
            {
                verse_track_pos = track_length;
                verse_track_offset = verse_track_pos - 15; 
            }
            else if (verse_track_pos < verse_track_offset)
            {
                verse_track_offset = verse_track_pos;
            }
            moved = 1;
        }
        if (GAMEPAD_PRESSING(0, right))
        {
            if (++verse_track_pos > track_length)
            {
                verse_track_pos = 1;
                verse_track_offset = 1;
            }
            else if (verse_track_pos > verse_track_offset+15)
            {
                verse_track_offset = verse_track_pos - 15;
            }
            moved = 1;
        }
        if (moved)
        {
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;
            if (paint_if_moved)
                verse_track_paint(paint_if_moved-1);
        }
        else if (switched || paint_if_moved)
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;

        if (GAMEPAD_PRESS(0, A))
        {
            track_pos = 0;
            if (chip_play_track)
            {
                chip_play_track = 0;
                for (int i=0; i<4; ++i)
                    instrument[i].track_volume = 0;
            }
            else
            {
                // play this instrument track.
                // after the repeat, all tracks will sound.
                chip_play_track_init(verse_track);
                // avoid playing other instruments for now:
                for (int i=0; i<instrument_i; ++i)
                    instrument[i].track_read_pos = track_length;
                for (int i=instrument_i+1; i<4; ++i)
                    instrument[i].track_read_pos = track_length;
            }
        } 
        if (moved)
        {
            gamepad_press_wait = GAMEPAD_PRESS_WAIT;
            return;
        }
        if (GAMEPAD_PRESS(0, X))
        { 
            game_switch(EditInstrument);
            return;
        }
    }
    
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        verse_menu_not_edit = 1 - verse_menu_not_edit; 
        if (verse_menu_not_edit)
            verse_track_pos = 0;
        else
            verse_track_pos = 1;
        verse_track_offset = 1;
        verse_copying = 16;
        for (int i=0; i<4; ++i)
            instrument[i].track_volume = 0;
        chip_play_track = 0;
        return;
    }

    if (GAMEPAD_PRESS(0, select))
    {
        verse_copying = 16;
        game_message[0] = 0;
        previous_visual_mode = None;
        game_switch(EditAnthem);
        return;
    } 
}
