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
#define NUMBER_LINES 20

uint8_t verse_track CCM_MEMORY;
uint8_t verse_track_pos CCM_MEMORY;

void verse_init()
{
    verse_track = 0;
    verse_track_pos = 0;
}

void verse_reset()
{
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
    }
    else
    {
        --internal_line;
        switch (line)
        {
            case 0:
            {
                // edit track
                uint8_t msg[] = { 'e', 'd', 'i', 't', ' ', 
                    't', 'r', 'a', 'c', 'k', ' ', hex[verse_track], ':', ' ', 'I',
                    hex[instrument_i], ' ', 'X', hex[verse_track_pos],
                0 };
                font_render_line_doubled(msg, 16, internal_line, 65535, BG_COLOR*257);
                break;
            }
            case 2:
                font_render_line_doubled((uint8_t *)"dpad:", 16, internal_line, 65535, BG_COLOR*257);
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            {
                uint8_t msg[] = { (line-3) == instrument_i ? '*' : ' ', hex[line-3], ':', 0 };
                font_render_line_doubled(msg, 16, internal_line, 65535, BG_COLOR*257);
                break;
            }
            case 8:
            {
                font_render_line_doubled((uint8_t *)"X: edit instrument", 16, internal_line, 65535, BG_COLOR*257);
                break;
            }
        }
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
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
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
