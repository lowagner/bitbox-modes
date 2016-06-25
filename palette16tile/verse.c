#include "bitbox.h"
#include "common.h"
#include "common.h"
#include "chiptune.h"
#include "font.h"
#include "io.h"

#include <stdlib.h> // rand
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
#define NUMBER_LINES 20

uint8_t verse_note CCM_MEMORY;
uint8_t verse_track CCM_MEMORY;
uint8_t verse_instrument CCM_MEMORY;
uint8_t verse_instrument_pos CCM_MEMORY;
uint8_t verse_position CCM_MEMORY;
uint8_t verse_edit_track_not_instrument CCM_MEMORY;
uint8_t verse_show_instrument CCM_MEMORY;
uint8_t verse_bad_instrument CCM_MEMORY;

void verse_init()
{
    verse_track = 0;
    verse_instrument = 0;
    verse_position = 0;
    verse_edit_track_not_instrument = 0;
    verse_bad_instrument = 0;
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
        param = '0'; 
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
                case WF_SINE:
                    cmd = 1;
                    param = 2;
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
                case WF_NOISE:
                    cmd = 7;
                    param = 8;
                    break;
                case WF_RED:
                    cmd = 7;
                    param = 9;
                    break;
                case WF_VIOLET:
                    cmd = 7;
                    param = 10;
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
        case RANDOMIZE:
            cmd = 'R';
            param = hex[param];
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
            cmd = 157; // nu
            param = hex[param];
            break;
        case INERTIA:
            cmd = 'i';
            param = hex[param];
            break;
        case BITCRUSH:
            cmd = 9;
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

int _verse_check_instrument(int i);

void verse_check_instrument(int i)
{
    // check if that parameter broke something
    if (_verse_check_instrument(i))
    {
        verse_bad_instrument = 1; 
        instrument[i].track_volume = 0; // shut it down now!
        strcpy((char *)game_message, "bad jump, need wait in loop.");
    }
    else
    {
        verse_bad_instrument = 0; 
        game_message[0] = 0;
    }
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
            break;
        case SIDE:
            param = (param+direction)&3;
            break;
        case WAVEFORM:
            param = param+direction;
            if (param > 240)
                param = WF_VIOLET;
            else if (param > WF_VIOLET)
                param = WF_SINE;
            break;
        case VOLUME:
        case NOTE:
        case RANDOMIZE:
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

    verse_check_instrument(verse_instrument);
}

int _verse_check_instrument(int i)
{
    // check for a JUMP which loops back on itself without waiting at least a little bit.
    // return 1 if so, 0 if not.
    int j=0; // current command index
    int j_last_jump = -1;
    int found_wait = 0;
    for (int k=0; k<32; ++k)
    {
        if (j >= 16) // got to the end
        {
            message("made it to end, good!\n");
            return 0;
        }
        message("scanning instrument %d: line %d\n", i, j);
        if (j_last_jump >= 0)
        {
            int j_next_jump = -1;
            if (j == j_last_jump) // we found our loop-back point
            {
                message("returned to the jump\n");
                return !(found_wait); // did we find a wait?
            }
            switch (instrument[i].cmd[j]&15)
            {
                case JUMP:
                    j_next_jump = instrument[i].cmd[j]>>4;
                    if (j_next_jump == j_last_jump) // jumping forward to the original jump
                    {
                        message("jumped to the old jump\n");
                        return !(found_wait);
                    }
                    else if (j_next_jump > j_last_jump) // jumping past original jump
                    {
                        message("This probably shouldn't happen.\n");
                        j_last_jump = -1; // can't look for loops...
                    }
                    else
                    {
                        message("jumped backwards again?\n");
                        j_last_jump = j;
                        found_wait = 0;
                    }
                    j = j_next_jump;
                    break;
                case WAIT:
                    message("saw wait at j=%d\n", j);
                    if (instrument[i].cmd[j]>>4)
                        found_wait = 1;
                    ++j;
                    break;
                default:
                    ++j;
            }
        }
        else
        {
            if ((instrument[i].cmd[j]&15) != JUMP)
                ++j;
            else
            {
                j_last_jump = j;
                j = instrument[i].cmd[j]>>4;
                if (j > j_last_jump)
                {
                    message("This probably shouldn't happen??\n");
                    j_last_jump = -1; // don't care, we moved ahead
                }
                else if (j == j_last_jump)
                {
                    message("jumped to itself\n");
                    return 1; // this is bad
                }
                else
                    found_wait = 0;
            }
        }
    }
    message("couldn't finish after 32 iterations. congratulations\n");
    return 1;
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
        case 19:
            font_render_line_doubled(game_message, 36, internal_line, 65535, BG_COLOR*257);
            break;
        case 1:
        case 18:
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
            break; 
        }

    }
}

void verse_controls()
{
    int movement = 0;
    if (GAMEPAD_PRESSING(0, down))
    {
        if (verse_edit_track_not_instrument)
        {
        }
        else
        {
            if (verse_instrument_pos < MAX_INSTRUMENT_LENGTH && 
                instrument[verse_instrument].cmd[verse_instrument_pos])
            {
                ++verse_instrument_pos;
            }
            else
                verse_instrument_pos = 0;
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
            else
            {
                while (verse_instrument_pos < MAX_INSTRUMENT_LENGTH && 
                    (instrument[verse_instrument].cmd[verse_instrument_pos]&15) != BREAK)
                {
                    ++verse_instrument_pos;
                }
            }
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
    {
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        return;
    }

    if (GAMEPAD_PRESS(0, X))
    {
        // delete
        for (int j=verse_instrument_pos; j<MAX_INSTRUMENT_LENGTH; ++j)
        {
            // TODO: could do fancier things here, like check for JUMP indices to correct
            if ((instrument[verse_instrument].cmd[j] = instrument[verse_instrument].cmd[j+1]) == 0)
                break;
        }
        verse_check_instrument(verse_instrument);
        return;
    }

    if (GAMEPAD_PRESS(0, A))
    {
        // insert
        if ((instrument[verse_instrument].cmd[MAX_INSTRUMENT_LENGTH-1]&15) != BREAK)
        {
            strcpy((char *)game_message, "list full, can't insert.");
            return;
        }
        for (int j=MAX_INSTRUMENT_LENGTH-1; j>verse_instrument_pos; --j)
        {
            // TODO: could do fancier things here, like check for JUMP indices to correct
            instrument[verse_instrument].cmd[j] = instrument[verse_instrument].cmd[j-1];
        }
        instrument[verse_instrument].cmd[verse_instrument_pos] = rand()%16;
        verse_check_instrument(verse_instrument);
    }
    
    if (GAMEPAD_PRESS(0, B))
    {
        instrument[verse_instrument].cmd[verse_instrument_pos] =
        (instrument[verse_instrument].cmd[verse_instrument_pos] + 1)&15;
        verse_check_instrument(verse_instrument);
    }
   
    if (verse_bad_instrument) // can't leave until you fix this
        return;

    if (GAMEPAD_PRESS(0, Y))
    {
        chip_note(verse_instrument, verse_note++, 240); 
        verse_note %= 12;
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
