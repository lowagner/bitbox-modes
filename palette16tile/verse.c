#include "bitbox.h"
#include "common.h"
#include "common.h"
#include "chiptune.h"
#include "font.h"
#include "io.h"

#include <string.h> // memset

uint8_t verse_color_names[16][3] = {
    { 'S', 0, 0 }, // black, stop/silence
    { 'V', 0, 0 }, // gray, volume fade
    { 'R', 0, 0 }, // white, repeat note
    { 'P', 0, 0 }, // pink
    { 'C', 0, 0 }, // red
    { 'C', '#', 0 }, 
    { 'D', 0, 0 }, 
    { 'E', 'b', 0 }, 
    { 'E', 0, 0 }, 
    { 'F', 0, 0 }, 
    { 'F', '#', 0 }, 
    { 'G', 0, 0 }, 
    { 'A', 'b', 0 }, 
    { 'A', 0, 0 }, 
    { 'B', 'b', 0 }, 
    { 'B', 0, 0 }
};

#define BG_COLOR 192
#define NUMBER_LINES 12

uint8_t verse_note CCM_MEMORY;
uint8_t verse_track CCM_MEMORY;
uint8_t verse_instrument CCM_MEMORY;
uint8_t verse_position CCM_MEMORY;
uint8_t verse_edit_track_not_instrument CCM_MEMORY;

void verse_init()
{
    verse_track = 0;
    verse_instrument = 0;
    verse_position = 0;
    verse_edit_track_not_instrument = 1;
}

void verse_reset()
{
    int i=0; // isntrument
    int ci = 0; // command index
    instrument[i].track_octave = 2;
    instrument[i].cmd[ci] = SIDE | (3<<4); 
    instrument[i].cmd[++ci] = VOLUME | (15<<4); 
    instrument[i].cmd[++ci] = WAVEFORM | (WF_SAW<<4); 
    instrument[i].cmd[++ci] = NOTE | (0<<4); 
    instrument[i].cmd[++ci] = WAIT | (15<<4); 
    instrument[i].cmd[++ci] = SIDE | (1<<4); 
    instrument[i].cmd[++ci] = NOTE | (4<<4); 
    instrument[i].cmd[++ci] = WAIT | (4<<4); 
    instrument[i].cmd[++ci] = SIDE | (2<<4); 
    instrument[i].cmd[++ci] = WAIT | (4<<4); 
    instrument[i].cmd[++ci] = NOTE | (7<<4); 
    instrument[i].cmd[++ci] = WAIT | (4<<4); 
    instrument[i].cmd[++ci] = FADE_OUT | (15<<4); 
    
    i = 1;
    ci = 0;
    instrument[i].track_octave = 3;
    instrument[i].cmd[ci] = SIDE | (3<<4); 
    instrument[i].cmd[++ci] = INERTIA | (15<<4); 
    instrument[i].cmd[++ci] = VOLUME | (15<<4); 
    instrument[i].cmd[++ci] = WAVEFORM | (WF_SINE<<4); 
    instrument[i].cmd[++ci] = NOTE | (0<<4); 
    instrument[i].cmd[++ci] = WAIT | (15<<4); 
    instrument[i].cmd[++ci] = VIBRATO | (3<<4); 
    instrument[i].cmd[++ci] = WAIT | (15<<4); 
    instrument[i].cmd[++ci] = WAIT | (15<<4); 
    instrument[i].cmd[++ci] = FADE_OUT | (1<<4); 
    
    i = 2;
    ci = 0;
    instrument[i].track_octave = 4;
    instrument[i].cmd[ci] = SIDE | (3<<4); 
    instrument[i].cmd[++ci] = VOLUME | (15<<4); 
    instrument[i].cmd[++ci] = WAVEFORM | (WF_NOISE<<4); 
    instrument[i].cmd[++ci] = WAIT | (3<<4); 
    instrument[i].cmd[++ci] = WAVEFORM | (WF_PULSE<<4); 
    instrument[i].cmd[++ci] = DUTY_DELTA | (6<<4); 
    instrument[i].cmd[++ci] = FADE_OUT | (1<<4); 
    instrument[i].cmd[++ci] = NOTE | (12<<4); 
    instrument[i].cmd[++ci] = WAIT | (3<<4); 
    instrument[i].cmd[++ci] = NOTE | (7<<4); 
    instrument[i].cmd[++ci] = WAIT | (3<<4); 
    instrument[i].cmd[++ci] = NOTE | (3<<4); 
    instrument[i].cmd[++ci] = WAIT | (3<<4); 
    instrument[i].cmd[++ci] = NOTE | (0<<4); 
    instrument[i].cmd[++ci] = WAIT | (3<<4); 
    instrument[i].cmd[++ci] = JUMP | (7<<4); 
    
    i = 3;
    ci = 0; 
    instrument[i].track_octave = 3;
    instrument[i].is_drum = 1; // drums get MAX_INSTRUMENT_LENGTH/4 commands for each sub-instrument
    instrument[i].cmd[ci] = WAIT | (5<<4); 
    instrument[i].cmd[++ci] = WAVEFORM | (WF_SINE<<4); 
    instrument[i].cmd[++ci] = NOTE | (0 << 4); 
    instrument[i].cmd[++ci] = WAIT | (6<<4); 
    instrument[i].cmd[++ci] = SIDE | (1 << 4);  
    instrument[i].cmd[++ci] = WAIT | (6<<4); 
    instrument[i].cmd[++ci] = NOTE | (2 << 4); 
    instrument[i].cmd[++ci] = FADE_OUT | (13<<4); // the first sub-instrument is long (8 commands) 
    instrument[i].cmd[++ci] = SIDE | (1<<4); 
    instrument[i].cmd[++ci] = WAIT | (15<<4); 
    instrument[i].cmd[++ci] = SIDE | (2<<4); 
    instrument[i].cmd[++ci] = FADE_OUT | (10<<4);  // that was the second sub-instrument
    
    instrument[i].cmd[++ci] = WAIT | (3<<4); 
    instrument[i].cmd[++ci] = FADE_OUT | (15<<4); 
    instrument[i].cmd[++ci] = 0; 
    instrument[i].cmd[++ci] = 0;  // that was the third (last) sub-instrument
}

void verse_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0 || vga_line/2 == 10)
        {
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
            return;
        }
        return;
    }
    else if (vga_line >= SCREEN_H - 22)
    {
        if (vga_line/2 == (SCREEN_H - 20)/2)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
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
            if (verse_edit_track_not_instrument)
            {
                uint8_t msg[] = { 'e', 'd', 'i', 't', ' ', 
                    't', 'r', 'a', 'c', 'k', ' ', hex[verse_track],
                0 };
                font_render_line_doubled(msg, 16, internal_line, 65535, BG_COLOR*257);
            }
            else
            {
                uint8_t msg[] = { 'e', 'd', 'i', 't', ' ', 
                    'i', 'n', 's', 't', 'r', 'u', 'm', 'e', 'n', 't', 
                    ' ', hex[verse_instrument],
                0 };
                font_render_line_doubled(msg, 16, internal_line, 65535, BG_COLOR*257);
            }
            break;
        case 1:
            break;
        }

    }
}

void verse_controls()
{
    if (GAMEPAD_PRESS(0, B))
    {
        if (verse_note)
            chip_note(verse_instrument, --verse_note, 240); 
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        if (verse_note < MAX_NOTE-1)
            chip_note(verse_instrument, ++verse_note, 240); 
    }
    if (GAMEPAD_PRESS(0, L))
    {
        verse_instrument = (verse_instrument-1)&3;
    }
    if (GAMEPAD_PRESS(0, R))
    {
        verse_instrument = (verse_instrument+1)&3;
    }
    
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        previous_visual_mode = None;
        game_switch(SaveLoadScreen);
        return;
    } 
    
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        verse_edit_track_not_instrument = 1 - verse_edit_track_not_instrument;
        return;
    } 
}
