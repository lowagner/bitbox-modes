#include "bitbox.h"
#include "common.h"
#include "common.h"
#include "chiptune.h"
#include "font.h"
#include "io.h"

#include <string.h> // memset

uint8_t verse_color_names[16][3] = {
    { 'S', ' ', 0 }, // black, stop/silence
    { 'V', ' ', 0 }, // gray, volume fade
    { 'R', ' ', 0 }, // white, repeat note
    { 'P', ' ', 0 }, // pink
    { 'C', ' ', 0 }, // red
    { 'C', '#', 0 }, 
    { 'D', ' ', 0 }, 
    { 'E', 'b', 0 }, 
    { 'E', ' ', 0 }, 
    { 'F', ' ', 0 }, 
    { 'F', '#', 0 }, 
    { 'G', ' ', 0 }, 
    { 'A', 'b', 0 }, 
    { 'A', ' ', 0 }, 
    { 'B', 'b', 0 }, 
    { 'B', ' ', 0 }
};

#define BG_COLOR 192
#define NUMBER_LINES 18

uint8_t verse_note CCM_MEMORY;
uint8_t verse_track CCM_MEMORY;
uint8_t verse_instrument CCM_MEMORY;
uint8_t verse_instrument_pos CCM_MEMORY;
uint8_t verse_position CCM_MEMORY;
uint8_t verse_edit_track_not_instrument CCM_MEMORY;
uint8_t verse_show_instrument CCM_MEMORY;

void verse_init()
{
    verse_track = 0;
    verse_instrument = 0;
    verse_position = 0;
    verse_edit_track_not_instrument = 0;
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

void verse_render_cmd(int i, int j, int x, int y)
{
    #ifdef EMULATOR
    if (y < 0 || y >= 8)
    {
        message("got too big a line count for instrument %d, %d\n", (int)i, y);
        return;
    }
    if (x < 0 || x + 10 >= SCREEN_W)
    {
        message("instrument %d goes off screen!\n", (int)i);
        return;
    }
    #endif
    
    uint8_t cmd = instrument[i].cmd[j];
    uint8_t param = cmd>>4;
    cmd &= 15;
    int smash_together = 0;

    uint32_t *dst = (uint32_t *)draw_buffer + x/2;
    uint32_t color_choice[2];
    if (!instrument[i].is_drum || j < 2*MAX_DRUM_LENGTH)
    {
        if (j % 2)
            color_choice[0] = 16843009u*BG_COLOR;
        else
            color_choice[0] = 16843009u*149;
    }
    else if (j < 3*MAX_DRUM_LENGTH)
    {
        if (j % 2)
            color_choice[0] = 16843009u*41;
        else
            color_choice[0] = 16843009u*45;
    }
    else
    {
        if (j % 2)
            color_choice[0] = 16843009u*BG_COLOR;
        else
            color_choice[0] = 16843009u*9;
    }
    if (j != verse_instrument_pos || verse_edit_track_not_instrument)
    {
        color_choice[1] = 65535u*65537u;
    }
    else
    {
        color_choice[1] = RGB(190, 245, 255)|(RGB(190, 245, 255)<<16);
        if ((y+1)/2 == 1)
        {
            dst -= x/4;
            *dst = color_choice[1];
            ++dst;
            *dst = color_choice[1];
            dst += x/4 - 1;
        }
        else if ((y+1)/2 == 3)
        {
            dst -= x/4;
            *dst = 16843009u*BG_COLOR;
            ++dst;
            *dst = 16843009u*BG_COLOR;
            dst += x/4 - 1;
        }
    }

    
    if (cmd == 0)
    {
        cmd = '0';
        param = '?'; 
        if (y == 7)
            verse_show_instrument = 0;
    }
    else 
    switch (cmd)
    {
        case SIDE:
            switch (param)
            {
                case 0:
                    cmd = 's';
                    param = 'h';
                    break;
                case 1:
                    cmd = 'L';
                    param = ' ';
                    break;
                case 2:
                    cmd = ' ';
                    param = 'R';
                    break;
                case 3:
                    cmd = 'L';
                    param = 'R';
                    break;
            }
            break;
        case VOLUME:
            cmd = 'V';
            param = hex[param];
            break;
        case WAVEFORM:
            switch (param)
            {
                case WF_NOISE:
                    cmd = 0;
                    param = 0;
                    break;
                case WF_TRIANGLE:
                    cmd = '/';
                    param = '\\';
                    break;
                case WF_SAW:
                    cmd = 3;
                    param = 4;
                    break;
                case WF_PULSE:
                    cmd = 5;
                    param = 6;
                    break;
                case WF_SINE:
                    cmd = 1;
                    param = 2;
                    break;
                default:
                    cmd = ' ';
                    param = ' ';
                    break;
            }
            smash_together = 1;
            break;
        case NOTE:
            if (param >= 12)
                color_choice[1] = RGB(150,150,255)|(65535<<16);
            param %= 12;
            cmd = verse_color_names[4+param][0];
            param = verse_color_names[4+param][1];
            break;
        case WAIT:
            cmd = 'W';
            param = hex[param];
            break;
        case FADE_IN:
            cmd = '<';
            param = hex[param];
            break;
        case FADE_OUT:
            cmd = '>';
            param = hex[param];
            break;
        case VIBRATO:
            cmd = '~';
            param = hex[param];
            break;
        case VIBRATO_RATE:
            cmd = 128+32+13; // nu
            param = hex[param];
            break;
        case INERTIA:
            cmd = 'i';
            param = hex[param];
            break;
        case BITCRUSH:
            cmd = 7;
            param = hex[param];
            break;
        case DUTY:
            cmd = 129; // Gamma
            param = hex[param];
            break;
        case DUTY_DELTA:
            cmd = 130; // Delta
            param = hex[param];
            break;
        case JUMP:
            cmd = 'J';
            param = hex[param];
            break;
    }
    
    y = ((y/2))*4; // make y now how much to shift for font row
    uint8_t row = (font[hex[j]] >> y) & 15;
    *(++dst) = color_choice[0];
    for (int k=0; k<4; ++k)
    {
        *(++dst) = color_choice[row&1];
        row >>= 1;
    }
    *(++dst) = color_choice[0];
    row = (font[':'] >> y) & 15;
    for (int k=0; k<4; ++k)
    {
        *(++dst) = color_choice[row&1];
        row >>= 1;
    }
    *(++dst) = color_choice[0];
    *(++dst) = color_choice[0];
    row = (font[cmd] >> y) & 15;
    if (smash_together)
    {
        *(++dst) = color_choice[0];
        for (int k=0; k<4; ++k)
        {
            *(++dst) = color_choice[row&1];
            row >>= 1;
        }
        row = (font[param] >> y) & 15;
        for (int k=0; k<4; ++k)
        {
            *(++dst) = color_choice[row&1];
            row >>= 1;
        }
    }
    else
    {
        for (int k=0; k<4; ++k)
        {
            *(++dst) = color_choice[row&1];
            row >>= 1;
        }
        *(++dst) = color_choice[0];
        
        row = (font[param] >> y) & 15;
        for (int k=0; k<4; ++k)
        {
            *(++dst) = color_choice[row&1];
            row >>= 1;
        }
    }
    *(++dst) = color_choice[0];
}

void verse_adjust_parameter(int direction)
{
    if (!direction)
        return;
    uint8_t cmd = instrument[verse_instrument].cmd[verse_instrument_pos];
    uint8_t param = cmd>>4;
    cmd &= 15;
    
    switch (cmd)
    {
        case BREAK:
            return;
        case SIDE:
            param = (param+direction)&3;
            break;
        case WAVEFORM:
            param = param+direction;
            if (param > 240)
                param = WF_SINE;
            else if (param > WF_SINE)
                param = WF_NOISE;
            break;
        case VOLUME:
        case NOTE:
        case WAIT:
        case FADE_IN:
        case FADE_OUT:
        case VIBRATO:
        case VIBRATO_RATE:
        case INERTIA:
        case BITCRUSH:
        case DUTY:
        case DUTY_DELTA:
        case JUMP:
            param = (param+direction)&15;
            break;
    }
    instrument[verse_instrument].cmd[verse_instrument_pos] = cmd | (param<<4);
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
        case 2:
            verse_show_instrument = 1; 
            // purposeful fall through here
        case 10:
        case 14:
            if (instrument[verse_instrument].is_drum)
               verse_show_instrument = 1; 
            // purposeful fall through here
        default:
            if (verse_show_instrument)
                verse_render_cmd(verse_instrument, line-2, 16, internal_line);
            //verse_render_instrument(verse_instrument, 16, internal_line);
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
        verse_instrument_pos = 0;
    }
    if (GAMEPAD_PRESS(0, R))
    {
        verse_instrument = (verse_instrument+1)&3;
        verse_instrument_pos = 0;
    }
    int movement = 0;
    if (GAMEPAD_PRESSING(0, down))
    {
        if (verse_edit_track_not_instrument)
        {
        }
        else
        {
            if (verse_instrument_pos < 15 && 
                instrument[verse_instrument].cmd[verse_instrument_pos])
                ++verse_instrument_pos;
        }
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        if (verse_edit_track_not_instrument)
        {
        }
        else
        {
            if (verse_instrument_pos)
                --verse_instrument_pos;
        }
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, left))
    {
        if (verse_edit_track_not_instrument)
        {
        }
        else
        {
            verse_adjust_parameter(-1);
        }
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        if (verse_edit_track_not_instrument)
        {
        }
        else
        {
            verse_adjust_parameter(+1);
        }
        movement = 1;
    }
    if (movement)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
    
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
