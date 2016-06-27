#include "bitbox.h"
#include "common.h"
#include "common.h"
#include "chiptune.h"
#include "font.h"
#include "io.h"

#include <stdlib.h> // rand
#include <string.h> // memset

// TODO:  allow setting initial_track_octave and is_drum from line 0

const uint8_t note16_name[16][2] = {
    { 'S', ' ' }, // black, stop/silence
    { 'V', ' ' }, // gray, volume fade
    { 'R', ' ' }, // white, repeat note
    { 'P', ' ' }, // pink
    { 'C', ' ' }, // red
    { 'C', '#' }, 
    { 'D', ' ' }, 
    { 'E', 'b' }, 
    { 'E', ' ' }, 
    { 'F', ' ' }, 
    { 'F', '#' }, 
    { 'G', ' ' }, 
    { 'A', 'b' }, 
    { 'A', ' ' }, 
    { 'B', 'b' }, 
    { 'B', ' ' }
};

#define BG_COLOR 192
#define NUMBER_LINES 20

uint8_t instrument_note CCM_MEMORY;
uint8_t instrument_save_not_load CCM_MEMORY;
uint8_t instrument_i CCM_MEMORY;
uint8_t instrument_j CCM_MEMORY;
uint8_t show_instrument CCM_MEMORY;
uint8_t instrument_bad CCM_MEMORY;

void instrument_init()
{
    instrument_i = 0;
    instrument_j = 0;
    instrument_bad = 0;
    instrument_save_not_load = 1;
}

void instrument_reset()
{
    int i=0; // isntrument
    int ci = 0; // command index
    instrument[i].initial_track_octave = 2;
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
    instrument[i].initial_track_octave = 3;
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
    instrument[i].initial_track_octave = 4;
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
    instrument[i].initial_track_octave = 3;
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

void instrument_short_command_message(uint8_t *buffer, uint8_t cmd)
{
    switch (cmd&15)
    {
        case BREAK:
            strcpy((char *)buffer, "break");
            break;
        case SIDE:
            strcpy((char *)buffer, "stereo");
            break;
        case WAVEFORM:
            strcpy((char *)buffer, "waveform");
            break;
        case VOLUME:
            strcpy((char *)buffer, "volume");
            break;
        case NOTE:
            strcpy((char *)buffer, "note");
            break;
        case RANDOMIZE:
            strcpy((char *)buffer, "randomize");
            break;
        case WAIT:
            strcpy((char *)buffer, "wait");
            break;
        case FADE_IN:
            strcpy((char *)buffer, "fade in");
            break;
        case FADE_OUT:
            strcpy((char *)buffer, "fade out");
            break;
        case VIBRATO:
            strcpy((char *)buffer, "vibrato");
            break;
        case VIBRATO_RATE:
            strcpy((char *)buffer, "vibrato rate");
            break;
        case INERTIA:
            strcpy((char *)buffer, "inertia");
            break;
        case BITCRUSH:
            strcpy((char *)buffer, "bitcrush");
            break;
        case DUTY:
            strcpy((char *)buffer, "duty");
            break;
        case DUTY_DELTA:
            strcpy((char *)buffer, "change duty");
            break;
        case JUMP:
            strcpy((char *)buffer, "jump");
            break;
    }
}

void instrument_render_cmd(int i, int j, int y)
{
    int x = 32;
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

    if (j != instrument_j)
    {
        color_choice[1] = 65535u*65537u;
    }
    else
    {
        color_choice[1] = RGB(190, 245, 255)|(RGB(190, 245, 255)<<16);
        if ((y+1)/2 == 1)
        {
            dst -= 4;
            *dst = color_choice[1];
            ++dst;
            *dst = color_choice[1];
            dst += 4 - 1;
        }
        else if ((y+1)/2 == 3)
        {
            dst -= 4;
            *dst = 16843009u*BG_COLOR;
            ++dst;
            *dst = 16843009u*BG_COLOR;
            dst += 4 - 1;
        }
    }
    
    if (cmd == 0)
    {
        cmd = '0';
        param = '0'; 
        if (y == 7)
            show_instrument = 0;
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
            cmd = note16_name[4+param][0];
            param = note16_name[4+param][1];
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

int _check_instrument(int i);

void check_instrument(int i)
{
    // check if that parameter broke something
    if (_check_instrument(i))
    {
        instrument_bad = 1; 
        instrument[i].track_volume = 0; // shut it down now!
        strcpy((char *)game_message, "bad jump, need wait in loop.");
    }
    else
    {
        instrument_bad = 0; 
        game_message[0] = 0;
    }
    instrument[instrument_i].track_volume = 0;
}

void instrument_adjust_parameter(int direction)
{
    if (!direction)
        return;
    uint8_t cmd = instrument[instrument_i].cmd[instrument_j];
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
    instrument[instrument_i].cmd[instrument_j] = cmd | (param<<4);

    check_instrument(instrument_i);
}

int _check_instrument(int i)
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
    message("couldn't finish after 32 iterations. congratulations.\nprobably looping back on self, but with waits.");
    return 0;
}

void instrument_line()
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
        return;
    }
    --internal_line;
    uint8_t buffer[24];
    switch (line)
    {
    case 0:
    {
        // edit instrument
        uint8_t msg[] = { 'i', 'n', 's', 't', 'r', 'u', 'm', 'e', 'n', 't', 
            ' ', hex[instrument_i],
            ' ',  'o', 'c', 't', 'a', 'v', 'e',
            ' ',  hex[instrument[instrument_i].initial_track_octave],
            ' ', 'd', 'r', 'u', 'm', ' ', (instrument[instrument_i].is_drum ? 'Y' : 'N'),
        0 };
        font_render_line_doubled(msg, 16, internal_line, 65535, BG_COLOR*257);
        break;
    }
    case 1:
        break;
    case 2:
    {
        show_instrument = 1; 
        instrument_render_cmd(instrument_i, line-2, internal_line);
        // command
        uint8_t msg[] = { 'c', 'o', 'm', 'm', 'a', 'n', 'd', ' ', hex[instrument_j], ':', 0 };
        font_render_line_doubled(msg, 96, internal_line, 65535, BG_COLOR*257);
    }
    case 10:
    case 14:
        if (instrument[instrument_i].is_drum || show_instrument)
        {
            show_instrument = 1; 
            instrument_render_cmd(instrument_i, line-2, internal_line);
        }
        break;
    case 18:
        break;
    case 19:
        font_render_line_doubled(game_message, 36, internal_line, 65535, BG_COLOR*257);
        break;
    case 3:
        switch (instrument[instrument_i].cmd[instrument_j]&15)
        {
            case BREAK:
                strcpy((char *)buffer, "end of instrument");
                break;
            case SIDE:
                strcpy((char *)buffer, "stereo left/right");
                break;
            case WAVEFORM:
                strcpy((char *)buffer, "waveform");
                break;
            case VOLUME:
                strcpy((char *)buffer, "volume");
                break;
            case NOTE:
                strcpy((char *)buffer, "relative note from C");
                break;
            case RANDOMIZE:
                strcpy((char *)buffer, "randomize command");
                break;
            case WAIT:
                strcpy((char *)buffer, "wait");
                break;
            case FADE_IN:
                strcpy((char *)buffer, "fade in");
                break;
            case FADE_OUT:
                strcpy((char *)buffer, "fade out");
                break;
            case VIBRATO:
                strcpy((char *)buffer, "vibrato depth");
                break;
            case VIBRATO_RATE:
                strcpy((char *)buffer, "vibrato rate");
                break;
            case INERTIA:
                strcpy((char *)buffer, "note inertia");
                break;
            case BITCRUSH:
                strcpy((char *)buffer, "bitcrush");
                break;
            case DUTY:
                strcpy((char *)buffer, "duty");
                break;
            case DUTY_DELTA:
                strcpy((char *)buffer, "change in duty");
                break;
            case JUMP:
                strcpy((char *)buffer, "jump to command");
                break;
        }
        font_render_line_doubled(buffer, 102, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 5:
        font_render_line_doubled((uint8_t *)"switch to:", 102, internal_line, 65535, BG_COLOR*257); 
        goto maybe_show_instrument;
    case 6:
        buffer[0] = 'L'; buffer[1] = ':';
        instrument_short_command_message(buffer+2, instrument[instrument_i].cmd[instrument_j]-1);
        font_render_line_doubled(buffer, 112, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 7:
        buffer[0] = 'R'; buffer[1] = ':';
        instrument_short_command_message(buffer+2, instrument[instrument_i].cmd[instrument_j]+1);
        font_render_line_doubled(buffer, 112, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 8:
        font_render_line_doubled((uint8_t *)"left/right:", 102, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 9:
        font_render_line_doubled((uint8_t *)"adjust parameter", 112, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 11:
        font_render_line_doubled((uint8_t *)"A:insert cmd", 96, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 12:
        font_render_line_doubled((uint8_t *)"X:delete cmd", 96, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 13:
        if (!instrument_bad)
            font_render_line_doubled((uint8_t *)"Y:play note", 96, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 15:
        font_render_line_doubled((uint8_t *)"B:toggle save/load", 96, internal_line, 65535, BG_COLOR*257);
        goto maybe_show_instrument;
    case 17:
        if (instrument_save_not_load)
        {
            if (!instrument_bad)
                font_render_line_doubled((uint8_t *)"start:save instrument", 96, internal_line, 65535, BG_COLOR*257);
        }
        else
        {
            font_render_line_doubled((uint8_t *)"start:load instrument", 96, internal_line, 65535, BG_COLOR*257);
        }
        goto maybe_show_instrument;
    default:
      maybe_show_instrument:
        if (show_instrument)
            instrument_render_cmd(instrument_i, line-2, internal_line);
        break; 
    }
}

void instrument_controls()
{
    int movement = 0;
    if (GAMEPAD_PRESSING(0, down))
    {
        if (!instrument[instrument_i].is_drum)
        {
            if (instrument_j < MAX_INSTRUMENT_LENGTH-1 &&
                instrument[instrument_i].cmd[instrument_j])
            {
                ++instrument_j;
            }
            else
                instrument_j = 0;
        }
        else
        {
            int next_j;
            if (instrument_j < 2*MAX_DRUM_LENGTH)
                next_j = 2*MAX_DRUM_LENGTH;
            else if (instrument_j < 3*MAX_DRUM_LENGTH)
                next_j = 3*MAX_DRUM_LENGTH;
            else
                next_j = 0;

            if (instrument_j < MAX_INSTRUMENT_LENGTH-1)
            {
                if ((instrument[instrument_i].cmd[instrument_j]&15) == BREAK)
                {
                    instrument_j = next_j;
                }
                else
                {
                    ++instrument_j;
                }
            }
            else
                instrument_j = 0;
        }
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        if (!instrument[instrument_i].is_drum)
        {
            if (instrument_j)
                --instrument_j;
            else
            {
                while (instrument_j < MAX_INSTRUMENT_LENGTH-1 && 
                    (instrument[instrument_i].cmd[instrument_j]&15) != BREAK)
                {
                    ++instrument_j;
                }
            }
        }
        else 
        {
            int move_here_then_up = -1;
            int but_no_further_than;
            switch (instrument_j)
            {
                case 0:
                    move_here_then_up = 3*MAX_DRUM_LENGTH; 
                    but_no_further_than = 4*MAX_DRUM_LENGTH;
                    break;
                case 2*MAX_DRUM_LENGTH:
                    move_here_then_up = 0; 
                    but_no_further_than = 2*MAX_DRUM_LENGTH;
                    break;
                case 3*MAX_DRUM_LENGTH:
                    move_here_then_up = 2*MAX_DRUM_LENGTH; 
                    but_no_further_than = 3*MAX_DRUM_LENGTH;
                    break;
                default:
                    --instrument_j;
            }
            if (move_here_then_up >= 0)
            {
                instrument_j = move_here_then_up;
                while (instrument_j < but_no_further_than-1 && 
                    (instrument[instrument_i].cmd[instrument_j]&15) != BREAK)
                {
                    ++instrument_j;
                }
            }
        }
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, left))
    {
        instrument_adjust_parameter(-1);
        movement = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        instrument_adjust_parameter(+1);
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
        if (!instrument[instrument_i].is_drum)
        {
            for (int j=instrument_j; j<MAX_INSTRUMENT_LENGTH-1; ++j)
            {
                if ((instrument[instrument_i].cmd[j] = instrument[instrument_i].cmd[j+1]) == 0)
                    break;
            }
            instrument[instrument_i].cmd[MAX_INSTRUMENT_LENGTH-1] = BREAK;
        }
        else
        {
            int max_j;
            if (instrument_j < 2*MAX_DRUM_LENGTH)
                max_j = 2*MAX_DRUM_LENGTH;
            else if (instrument_j < 3*MAX_DRUM_LENGTH)
                max_j = 3*MAX_DRUM_LENGTH;
            else
                max_j = 4*MAX_DRUM_LENGTH;
            
            for (int j=instrument_j; j<max_j-1; ++j)
            {
                if ((instrument[instrument_i].cmd[j] = instrument[instrument_i].cmd[j+1]) == 0)
                    break;
            }
            instrument[instrument_i].cmd[max_j-1] = BREAK;
        }
        check_instrument(instrument_i);
        return;
    }

    if (GAMEPAD_PRESS(0, A))
    {
        // insert
        if ((instrument[instrument_i].cmd[MAX_INSTRUMENT_LENGTH-1]&15) != BREAK)
        {
            strcpy((char *)game_message, "list full, can't insert.");
            return;
        }
        for (int j=MAX_INSTRUMENT_LENGTH-1; j>instrument_j; --j)
        {
            // TODO: could do fancier things here, like check for JUMP indices to correct
            instrument[instrument_i].cmd[j] = instrument[instrument_i].cmd[j-1];
        }
        instrument[instrument_i].cmd[instrument_j] = WAIT | ((rand()%16)<<4);
        check_instrument(instrument_i);
    }
    
    if (GAMEPAD_PRESS(0, L))
    {
        uint8_t *cmd = &instrument[instrument_i].cmd[instrument_j];
        *cmd = ((*cmd - 1)&15) | ((*cmd)&240);
        check_instrument(instrument_i);
    }
    if (GAMEPAD_PRESS(0, R))
    {
        uint8_t *cmd = &instrument[instrument_i].cmd[instrument_j];
        *cmd = ((*cmd + 1)&15) | ((*cmd)&240);
        check_instrument(instrument_i);
    }
    
    if (GAMEPAD_PRESS(0, B))
    {
        instrument_save_not_load = 1 - instrument_save_not_load; 
        return;
    }
    
    if (GAMEPAD_PRESS(0, start))
    {
        FileError error;
        if (instrument_save_not_load)
        {
            if (!instrument_bad)
                error = io_save_instrument(instrument_i);
            else
            {
                strcpy((char *)game_message, "can't save bad jump.");
                return;
            }
        }
        else
        {
            error = io_load_instrument(instrument_i);
            check_instrument(instrument_i);
        }
        io_message_from_error(game_message, error, instrument_save_not_load);
        return;
    } 
   
    if (instrument_bad) // can't do anything else until you fix this
        return;

    if (GAMEPAD_PRESS(0, Y))
    {
        // play a note
        if (instrument_save_not_load)
            instrument_note = (instrument_note + 1)%24;
        else if (--instrument_note > 23)
            instrument_note = 23;
        instrument[instrument_i].track_octave = instrument[instrument_i].initial_track_octave;
        chip_note(instrument_i, instrument_note, 240); 
    }

    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        instrument_j = 0;
        if (previous_visual_mode)
        {
            game_switch(previous_visual_mode);
        }
        else
        {
            previous_visual_mode = None;
            game_switch(EditVerse);
        }
        return;
    } 
}
