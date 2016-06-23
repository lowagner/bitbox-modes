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

uint8_t verse_note CCM_MEMORY;
uint8_t verse_track CCM_MEMORY;
uint8_t verse_instrument CCM_MEMORY;
uint8_t verse_position CCM_MEMORY;

void verse_init()
{
    verse_track = 0;
    verse_instrument = 0;
    verse_position = 0;
}

void verse_reset()
{
    int i=0;
    instrument[i].track_octave = 2;
    instrument[i].cmd[0] = 1 | (3<<4); // play both sides L/R
    instrument[i].cmd[1] = 2 | (15<<4); // volume 
    instrument[i].cmd[2] = 3 | (WF_SAW<<4); // waveform 
    instrument[i].cmd[3] = 4 | (0<<4); // relative note
    instrument[i].cmd[4] = 6 | (4<<4); // timing
    instrument[i].cmd[5] = 8 | (4<<4); // fade out
    instrument[i].cmd[6] = 6 | (4<<4); // timing
    instrument[i].cmd[7] = 2 | (15<<4); // volume 
    instrument[i].cmd[8] = 4 | (0<<4); // relative note 
    instrument[i].cmd[9] = 8 | (0<<4); // fade out
    instrument[i].cmd[10] = 6 | (4<<4); // timing
    instrument[i].cmd[9] = 8 | (4<<4); // fade out
}

void verse_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= SCREEN_H - 22)
    {
        if (vga_line/2 == (SCREEN_H - 20)/2)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
}

void verse_controls()
{
    if (GAMEPAD_PRESS(0, B))
    {
        if (verse_note)
            chip_note(verse_instrument, --verse_note); 
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        if (verse_note < MAX_NOTE-1)
            chip_note(verse_instrument, ++verse_note); 
    }
    
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        previous_visual_mode = None;
        game_switch(SaveLoadScreen);
        return;
    } 
}
