#include "bitbox.h"
#include "common.h"
#include "common.h"
#include "chiptune.h"
#include "instrument.h"
#include "font.h"
#include "io.h"

#include <stdlib.h> // rand
#include <string.h> // memset

#define BG_COLOR 164
#define BOX_COLOR (RGB(180, 200, 250)|(RGB(180, 200, 250)<<16))
#define MATRIX_WING_COLOR (RGB(30, 20, 0) | (RGB(30, 20, 0)<<16))
#define NUMBER_LINES 20

uint8_t verse_track CCM_MEMORY;
uint8_t verse_track_pos CCM_MEMORY;
uint8_t verse_track_offset CCM_MEMORY;

void key_name(uint8_t *name, uint8_t key)
{
    if (key < 12)
    {
        name[0] = note_name[key][0];
        name[1] = note_name[key][1];
    }
    else if (key < 24)
    {
        name[0] = note_name[key-12][0] - 32;
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
}

void verse_reset()
{
    track_length = MAX_TRACK_LENGTH;
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
    switch (line)
    {
        case 0:
        {
            // edit track
            uint8_t msg[] = {  't', 'r', 'a', 'c', 'k', ' ', hex[verse_track], 
                ':', ' ', 'X', '0' + verse_track_pos/10, '0' + verse_track_pos%10, '/',
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
                for (int j=0; j<16; ++j)
                {
                    int value = chip_track[verse_track][i][j+verse_track_offset];
                    uint8_t row = (font[hex[value]] >> y) & 15;
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
            font_render_line_doubled((uint8_t *)"dpad:move cursor", 12, internal_line, 65535, BG_COLOR*257);
            break;
        case 9:
            font_render_line_doubled((uint8_t *)"X:edit instrument", 12, internal_line, 65535, BG_COLOR*257);
            break;
    }
}

void verse_controls()
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
        --verse_track_pos;
        if (verse_track_pos == 255)
        {
            verse_track_pos = track_length;
            verse_track_offset = verse_track_pos - 15; 
        }
        else if (verse_track_pos == 0)
        {
            verse_track_offset = 1;
        }
        else if (verse_track_pos < verse_track_offset)
        {
            verse_track_offset = verse_track_pos;
        }
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        ++verse_track_pos;
        if (verse_track_pos > track_length)
        {
            verse_track_pos = 0;
            verse_track_offset = 1;
        }
        else if (verse_track_pos >= verse_track_offset+16)
        {
            verse_track_offset = verse_track_pos - 15;
        }
        movement = 1;
    }
    if (movement)
    {
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        return;
    }

    if (GAMEPAD_PRESS(0, X))
    { 
        game_switch(EditInstrument);
        return;
    }
    if (GAMEPAD_PRESS(0, A))
    {
    } 
    if (GAMEPAD_PRESS(0, L))
    {
    }
    if (GAMEPAD_PRESS(0, R))
    {
    } 
    if (GAMEPAD_PRESS(0, Y))
    {
    }
    if (GAMEPAD_PRESS(0, B))
    {
    }
    
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        // probably return to EditSong
        return;
    } 
    
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        previous_visual_mode = None;
        game_switch(SaveLoadScreen);
        return;
    } 
}
