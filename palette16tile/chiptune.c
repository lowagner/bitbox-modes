/* Simple soundengine for the BitBox, modified
 * Copyright 2016, Lucas Wagner <lowagner@gmail.com>
 * Copyright 2015, Makapuf <makapuf2@gmail.com>
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project
 *
 * This main file is a player for the packed music format used in the original
 * "hardware chiptune"
 * http://www.linusakesson.net/hardware/chiptune.php
 * There is a tracker for this but the format here is slightly different (mostly
 * because of the different replay rate - 32KHz instead of 16KHz).
 *
 * Because of this the sound in the tracker will be a bit different, but it can
 * easily be tweaked. This version has a somewhat bigger but much simplified song format.
 */
#include "bitbox.h"
#include "chiptune.h"

#include <stdint.h>
#include <stdlib.h>

uint8_t chip_play CCM_MEMORY;
uint8_t chip_play_track CCM_MEMORY;
uint8_t chip_repeat CCM_MEMORY;
uint8_t chip_volume CCM_MEMORY;

/*
    oscillator

    the stuff that gets called multiple times a frame,
    to generate the samples.
*/

struct oscillator oscillator[CHIP_PLAYERS] CCM_MEMORY;

/* 
    instruments
    you get one for each channel.
    you have a max of MAX_INSTRUMENT_LENGTH commands with parameters 

    there's a command (&15) and a parameter (>>4) embedded in each uint8_t cmd[].

    TODO:  multiply parameter by 16 to get full range for certain commands.
 */

struct instrument instrument[CHIP_PLAYERS] CCM_MEMORY;

/* 
    tracks

    chip_track[t][i][0] -> first command of track t with instrument i
    chip_track[t][i][1] -> second command...
 */

uint8_t track_pos CCM_MEMORY;
uint8_t track_length CCM_MEMORY;
uint8_t chip_track[16][CHIP_PLAYERS][MAX_TRACK_LENGTH] CCM_MEMORY;

/*
    chip_player
    the walkers that run through commands for an instrument
    as well as for a track
*/

struct chip_player chip_player[CHIP_PLAYERS] CCM_MEMORY;

/* 
    chip song
    is made of 16 different tracks for each instrument/channel.
 */

uint16_t chip_song[MAX_SONG_LENGTH]; // a nibble for the track to play for each channel
uint8_t song_wait CCM_MEMORY; // >0 means wait N frames, 0 means play now. 
uint8_t song_speed CCM_MEMORY;
uint8_t song_pos CCM_MEMORY;
uint8_t song_length CCM_MEMORY; // capped at MAX_SONG_LENGTH
uint8_t song_transpose CCM_MEMORY;

// At each sample the phase is incremented by frequency/4. It is then used to
// compute the output of the oscillator depending on the waveform.
// This means the frequency unit is 65536*4/31000 or about 8.456Hz
// and the frequency range is 0 to 554180Hz. Maybe it would be better to adjust
// the scaling factor to allow less high frequencies (they are useless) but
// more fine grained resolution. Not only we could play notes more in tune,
// but also we would get a more subtle vibrato effect.

// ... and that's it for the engine, which is very simple as you see.
// The parameters for the oscillators can be updated in your game_frame callback.
// Since the audio buffer is generated in one go it is useless to try to tweak
// the parameters more often than that.

static const uint16_t freq_table[MAX_NOTE] = {
    0x010b, 0x011b, 0x012c, 0x013e, 0x0151, 0x0165, 0x017a, 0x0191, 0x01a9,
    0x01c2, 0x01dd, 0x01f9, 0x0217, 0x0237, 0x0259, 0x027d, 0x02a3, 0x02cb,
    0x02f5, 0x0322, 0x0352, 0x0385, 0x03ba, 0x03f3, 0x042f, 0x046f, 0x04b2,
    0x04fa, 0x0546, 0x0596, 0x05eb, 0x0645, 0x06a5, 0x070a, 0x0775, 0x07e6,
    0x085f, 0x08de, 0x0965, 0x09f4, 0x0a8c, 0x0b2c, 0x0bd6, 0x0c8b, 0x0d4a,
    0x0e14, 0x0eea, 0x0fcd, 0x10be, 0x11bd, 0x12cb, 0x13e9, 0x1518, 0x1659,
    0x17ad, 0x1916, 0x1a94, 0x1c28, 0x1dd5, 0x1f9b, 0x217c, 0x237a, 0x2596,
    0x27d3, 0x2a31, 0x2cb3, 0x2f5b, 0x322c, 0x3528, 0x3851, 0x3bab, 0x3f37,
    0x42f9, 0x46f5, 0x4b2d, 0x4fa6, 0x5462, 0x5967, 0x5eb7, 0x6459, 0x6a51,
    0x70a3, 0x7756, 0x7e6f
}; // 84 long.

static const int8_t sine_table[] = {
    0, 12, 25, 37, 49, 60, 71, 81, 90, 98, 106, 112, 117, 122, 125, 126,
    127, 126, 125, 122, 117, 112, 106, 98, 90, 81, 71, 60, 49, 37, 25, 12,
    0, -12, -25, -37, -49, -60, -71, -81, -90, -98, -106, -112, -117, -122,
    -125, -126, -127, -126, -125, -122, -117, -112, -106, -98, -90, -81,
    -71, -60, -49, -37, -25, -12
};



static void instrument_run_command(uint8_t i, uint8_t inst, uint8_t cmd) 
{
    uint8_t param = cmd >> 4;
    switch (cmd&15) 
    {
        case BREAK:
            chip_player[i].cmd_index = MAX_INSTRUMENT_LENGTH; // end instrument commmands
            break;
        case SIDE: // s = switch side (L/R)
            oscillator[i].side = param; // 0 = no side / silence!, 1 = L, 2 = R, 3 = L/R
            break;
        case VOLUME: // v = volume
            chip_player[i].volume = param<<4;
            break;
        case WAVEFORM: // w = select waveform
            oscillator[i].waveform = param;
            break;
        case NOTE: // + = set relative note
            chip_player[i].note = (param + chip_player[i].track_note)%MAX_NOTE;
            if (chip_player[i].note >= MAX_NOTE)
                chip_player[i].note -= 12;
            break;
        case WAIT: // t = timing 
            chip_player[i].wait = param;
            break;
        case FADE_IN: // < = fade in, or crescendo
            chip_player[i].volumed = param + param*param/15;
            break;
        case FADE_OUT: // > = fade out, or decrescendo
            chip_player[i].volumed = -param - param*param/15;
            break;
        case INERTIA: // i = inertia (auto note slides)
            chip_player[i].inertia = param << 4;
            break;
        case VIBRATO_DEPTH: // ~ = vibrato depth
            chip_player[i].vibrato_depth = param;
            break;
        case VIBRATO_RATE: // x = vibrato rate
            chip_player[i].vibrato_rate = param;
            break;
        case BITCRUSH: // b = bitcrush
            oscillator[i].bitcrush = param;
            break;
        case DUTY: // d = duty cycle.  param==8 makes for a square wave if waveform is WF_PULSE
            oscillator[i].duty = 16384 + (param << 11);
            break;
        case DUTY_DELTA: // m = duty variation
            chip_player[i].dutyd = param << 4;
            break;
        case RANDOMIZE:
            switch (instrument[inst].cmd[param]&15)
            {
                case BREAK:
                    break;
                case SIDE:
                    instrument[inst].cmd[param] = SIDE | ((rand()%4)<<4);
                    break;
                case WAVEFORM:
                    instrument[inst].cmd[param] = WAVEFORM | ((rand()%(WF_VIOLET+1))<<4);
                    break;
                case VOLUME:
                    instrument[inst].cmd[param] = VOLUME | ((rand()%16)<<4);
                    break;
                case NOTE:
                    instrument[inst].cmd[param] = NOTE | ((rand()%16)<<4);
                    break;
                case WAIT:
                    instrument[inst].cmd[param] = WAIT | ((1+rand()%15)<<4);
                    break;
                case FADE_IN:
                    instrument[inst].cmd[param] = FADE_IN | ((1 + rand()%15)<<4);
                    break;
                case FADE_OUT:
                    instrument[inst].cmd[param] = FADE_OUT | ((1 + rand()%15)<<4);
                    break;
                case INERTIA:
                    instrument[inst].cmd[param] = INERTIA | ((rand()%16)<<4);
                    break;
                case VIBRATO_DEPTH:
                    instrument[inst].cmd[param] = VIBRATO_DEPTH | ((rand()%16)<<4);
                    break;
                case VIBRATO_RATE:
                    instrument[inst].cmd[param] = VIBRATO_RATE | ((1+rand()%15)<<4);
                    break;
                case BITCRUSH:
                    instrument[inst].cmd[param] = BITCRUSH | ((rand()%16)<<4);
                    break;
                case DUTY:
                    instrument[inst].cmd[param] = DUTY | ((rand()%16)<<4);
                    break;
                case DUTY_DELTA:
                    instrument[inst].cmd[param] = DUTY_DELTA | ((rand()%16)<<4);
                    break;
                case RANDOMIZE:
                    instrument[inst].cmd[param] = RANDOMIZE | ((rand()%16)<<4);
                    break;
                case JUMP:
                {
                    // jump randomly to a wait, 
                    // or before the wait 
                    // where there is no
                    // jumps in between.
                    
                    // TODO:  probably can create this once, per instrument,
                    // and re-use it.  as long as other commands in the instrument
                    // don't get modified, it would be fine to keep it.
                    uint8_t work[16] = { -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, -1
                    };
                    int work_size = 0;
                    int probable_start_point = 0;
                    for (int j=0; j<16; ++j)
                    {
                        switch (instrument[inst].cmd[j]&15)
                        {
                        case WAIT:
                            if (instrument[inst].cmd[j]>>4)
                            {
                                for (int k=probable_start_point; k<=j; ++k)
                                    work[work_size++] = k;
                                
                                probable_start_point = j+1;
                            }
                            break;
                        case JUMP:
                            probable_start_point = j+1;
                        }
                    }
                    #ifdef EMULATOR
                    if (work_size > 16)
                    {
                        message("got something to overflow in JUMP randomizer.\n");
                        break;
                    }
                    message("got randomized JUMP:\n [");
                    for (int j=0; j<work_size; ++j)
                        message("%d, ", work[j]);
                    message("]\n");
                    #endif
                   
                    if (work_size)
                        instrument[inst].cmd[param] = JUMP | (work[rand()%work_size]<<4);
                    else
                        message("should have some ability to work a JUMP out!\n");
                    break;
                }
            }
            break;
        case JUMP: // j = instrument jump
            chip_player[i].cmd_index = param;
            break;
    }
}

void chip_init()
{
    chip_volume = 5;
}

void chip_reset()
{
    song_length = 16;
    track_length = 32;
    song_speed = 8;
}

void reset_player(int i)
{
    chip_player[i].instrument = i;
    chip_player[i].track_cmd_index = 0;
    chip_player[i].volume = 0;
    chip_player[i].track_volume = 0;
    chip_player[i].track_volumed = 0;
    chip_player[i].track_inertia = 0;
    chip_player[i].track_vibrato_rate = 0;
    chip_player[i].track_vibrato_depth = 0;
    chip_player[i].octave = instrument[i].octave;
}

void chip_kill()
{
    chip_play = 0;
    chip_play_track = 0;
    for (int i=0; i<4; ++i)
    {
        oscillator[i].volume = 0;
        chip_player[i].track_volume = 0;
        chip_player[i].track_volumed = 0;
    }
}

void chip_play_init(int pos) 
{
    song_wait = 0;
    track_pos = 0;
    chip_play = 1;
    song_pos = pos % song_length;
    
    for (int i=0; i<4; ++i)
    {
        reset_player(i);
    }

    uint16_t tracks = chip_song[song_pos];
    chip_player[0].track_index = tracks & 15;
    chip_player[1].track_index = (tracks >> 4) & 15;
    chip_player[2].track_index = (tracks >> 8) & 15;
    chip_player[3].track_index = tracks >> 12;
    tracks = chip_song[(song_pos+1)%song_length];
    chip_player[0].next_track_index = tracks & 15;
    chip_player[1].next_track_index = (tracks >> 4) & 15;
    chip_player[2].next_track_index = (tracks >> 8) & 15;
    chip_player[3].next_track_index = tracks >> 12;
}

void chip_play_track_init(int track)
{
    song_wait = 0;
    track_pos = 0;
    chip_play = 0;
    chip_play_track = 1;
   
    track &= 15;
    for (int i=0; i<4; ++i)
    {
        reset_player(i);
        chip_player[i].next_track_index = track;
        chip_player[i].track_index = track;
    }
}

void _chip_note(uint8_t i, uint8_t note)
{
    #ifdef CHIPTUNE_DEBUG
    message("note %d on player %d\n", (note+12*chip_player[i].octave), i);
    #endif
    // now set some defaults and startup the command index
    if (instrument[chip_player[i].instrument].is_drum)
    {
        // a drum instrument has 3 sub instruments.
        note %= 12;
        // first subinstrument is 2*MAX_DRUM_LENGTH commands long, and takes up first 10 notes.
        if (note < 10)
        {
            chip_player[i].cmd_index = 0;
            chip_player[i].max_drum_index = 2*MAX_DRUM_LENGTH;
        }
        else if (note == 10)
        {
            chip_player[i].cmd_index = 2*MAX_DRUM_LENGTH;
            chip_player[i].max_drum_index = 3*MAX_DRUM_LENGTH;
        }
        else
        {
            chip_player[i].cmd_index = 3*MAX_DRUM_LENGTH;
            chip_player[i].max_drum_index = 4*MAX_DRUM_LENGTH;
        }
        oscillator[i].waveform = WF_NOISE; // by default
        chip_player[i].volume = 240; 
    }
    else
    {
        chip_player[i].cmd_index = 0;
        oscillator[i].waveform = WF_TRIANGLE; // by default
        chip_player[i].volume = 10<<4; 
    }
    chip_player[i].track_note = note + chip_player[i].octave*12 + song_transpose;
    chip_player[i].volumed = 0;
    chip_player[i].inertia = 0;
    chip_player[i].wait = 0;
    chip_player[i].dutyd = 0;
    chip_player[i].vibrato_depth = 0;
    chip_player[i].vibrato_rate = 5;
    oscillator[i].side = 3; // default to output both L/R
    oscillator[i].duty = 0x8000; // default to square wave
    //oscillator[i].volume = chip_player[i].track_volume * chip_player[i].volume / 255;
}

void chip_note(uint8_t i, uint8_t note, uint8_t track_volume)
{
    _chip_note(i, note);
    chip_player[i].track_volume = track_volume;
    chip_player[i].track_volumed = 0;
    //oscillator[i].volume = chip_player[i].track_volume * chip_player[i].volume / 255;
}

static void track_run_command(uint8_t i, uint8_t cmd) 
{
    uint8_t param = cmd >> 4;
    switch(cmd&15) 
    {
        case TRACK_BREAK:
            chip_player[i].track_cmd_index = MAX_TRACK_LENGTH; // end track commmands
            break;
        case TRACK_OCTAVE: // O = octave, or + or - for relative octave
            if (param < 7)
                chip_player[i].octave = param;
            else if (param == 7)
                chip_player[i].octave = instrument[chip_player[i].instrument].octave;
            else if (param < 12)
            {
                chip_player[i].octave += (param-6)/2;
                if (chip_player[i].octave > 6)
                {
                    if (param%2) // additive, no wrap around
                        chip_player[i].octave = 6;
                    else // wrap around
                        chip_player[i].octave %= 7;
                }
            }
            else
            {
                uint8_t delta = (17-param)/2;
                if (delta > chip_player[i].octave)
                {
                    if (param%2) // wrap around
                        chip_player[i].octave = 7 - delta + chip_player[i].octave;
                    else // subtractive only, no wrap around
                        chip_player[i].octave = 0;
                }
                else
                    chip_player[i].octave -= delta;
            }
            break;
        case TRACK_INSTRUMENT:
            chip_player[i].instrument = param % 4;
            //chip_player[i].octave = instrument[param % 4].octave;
            break;
        case TRACK_VOLUME: // v = volume
            chip_player[i].track_volume = param<<4;
            break;
        case TRACK_NOTE: // 
            _chip_note(i, param);
            break;
        case TRACK_WAIT: // w = wait 
            chip_player[i].track_wait = param;
            break;
        case TRACK_FILL: // f = wait til a given quarter note
            if (4*param > track_pos)
                chip_player[i].track_wait = 4*param - track_pos;
            else
                chip_player[i].track_wait = 0;
            break;
        case TRACK_FADE_IN: // < = fade in, or crescendo
            chip_player[i].track_volumed = param + param*param/15;
            break;
        case TRACK_FADE_OUT: // > = fade out, or decrescendo
            chip_player[i].track_volumed = -param - param*param/15;
            break;
        case TRACK_INERTIA: // i = inertia (auto note slides)
            chip_player[i].track_inertia = param << 4;
            break;
        case TRACK_VIBRATO: // ~ = vibrato depth
            chip_player[i].track_vibrato_rate = (param&3)<<2;
            chip_player[i].track_vibrato_depth = param & 12;
            break;
        case TRACK_TRANSPOSE: // T = global transpose
            if (param == 0) // reset song transpose
                song_transpose = 0;
            else
                song_transpose = (song_transpose+param)%12;
            break;
        case TRACK_SPEED: // 
            song_speed = 16 - param;
            break;
        case TRACK_LENGTH: // M - measure length, in quarter notes
            if (param)
                track_length = 4*param;
            else // zero is a special case:  16 quarter notes
                track_length = 64;
            break;
        case TRACK_RANDOMIZE1: // 
            param += 16;
            // DO NOT BREAK
        case TRACK_RANDOMIZE0:
        {
            uint8_t *memory = &chip_track[chip_player[i].track_index][i][param];
            switch ((*memory)&15)
            {
                case TRACK_BREAK:
                    break;
                case TRACK_OCTAVE:
                    *memory = TRACK_OCTAVE | ((rand()%16)<<4);
                    break;
                case TRACK_INSTRUMENT:
                    *memory = TRACK_INSTRUMENT | ((rand()%4)<<4);
                    break;
                case TRACK_VOLUME:
                    *memory = TRACK_VOLUME | ((rand()%16)<<4);
                    break;
                case TRACK_NOTE:
                    *memory = TRACK_NOTE | ((rand()%16)<<4);
                    break;
                case TRACK_WAIT:
                    *memory = TRACK_WAIT | ((1+rand()%15)<<4);
                    break;
                case TRACK_FILL:
                    *memory = TRACK_FILL | ((1+rand()%15)<<4);
                    break;
                case TRACK_FADE_IN:
                    *memory = TRACK_FADE_IN | ((1 + rand()%15)<<4);
                    break;
                case TRACK_FADE_OUT:
                    *memory = TRACK_FADE_OUT | ((1 + rand()%15)<<4);
                    break;
                case TRACK_INERTIA:
                    *memory = TRACK_INERTIA | ((rand()%16)<<4);
                    break;
                case TRACK_VIBRATO:
                    *memory = TRACK_VIBRATO | ((rand()%16)<<4);
                    break;
                case TRACK_TRANSPOSE:
                    *memory = TRACK_TRANSPOSE | ((rand()%16)<<4);
                    break;
                case TRACK_SPEED:
                    *memory = TRACK_SPEED | ((rand()%16)<<4);
                    break;
                case TRACK_LENGTH:
                    *memory = TRACK_LENGTH | ((rand()%16)<<4);
                    break;
                case TRACK_RANDOMIZE0:
                case TRACK_RANDOMIZE1:
                {
                    uint8_t pos = rand()%32;
                    if (pos >= 16)
                        *memory = TRACK_RANDOMIZE1 | (pos-16)<<4;
                    else
                        *memory = TRACK_RANDOMIZE0 | (pos<<4);
                    break;
                }
            }
            break;
        }
    }
}

static void chip_track_update()
{
    #ifdef DEBUG_CHIPTUNE
    message("%02d", track_pos);
    #endif

    for (int i=0; i<4; ++i) 
    {
        #ifdef DEBUG_CHIPTUNE
        message(" | t: %d (%02d/%02d) ", i, chip_player[i].track_cmd_index, track_pos, track_length);
        #endif
        while (!chip_player[i].track_wait && chip_player[i].track_cmd_index < MAX_TRACK_LENGTH) 
            track_run_command(i, chip_track[chip_player[i].track_index][i][chip_player[i].track_cmd_index++]);

        if (chip_player[i].track_wait)
            --chip_player[i].track_wait;
    }

    #ifdef DEBUG_CHIPTUNE
    message("\n");
    #endif

    if (++track_pos == track_length)
    {
        for (int i=0; i<4; ++i)
        {
            chip_player[i].track_cmd_index = 0;
            chip_player[i].track_index = chip_player[i].next_track_index;
        }
        track_pos = 0;
    }
}

static void chip_song_update()
// this shall be called each 1/60 sec, but only if chip_play is true.
// one buffer is 512 samples @32kHz, which is ~ 62.5 Hz,
// calling each song frame should be OK
{
    if (song_wait) 
    {
        --song_wait;
        return;
    } 
    song_wait = song_speed;

    if (!track_pos) // == 0.  load the next track.
    {
        if (song_pos >= song_length) 
        {
            if (chip_repeat)
                song_pos = 0;
            else
            {
                chip_play = 0;
                return;
            }
        } 
        #ifdef DEBUG_CHIPTUNE
        message("Now at position %d of song\n", song_pos);
        #endif
       
        song_pos = (song_pos+1)%song_length;
        uint16_t tracks = chip_song[song_pos];
        chip_player[0].next_track_index = tracks & 15;
        chip_player[1].next_track_index = (tracks >> 4) & 15;
        chip_player[2].next_track_index = (tracks >> 8) & 15;
        chip_player[3].next_track_index = tracks >> 12;
    }
    
    chip_track_update();
}

static void chip_update()
{
    for (int i=0; i<4; ++i) 
    {
        if (!chip_player[i].track_volume && chip_player[i].track_volumed <= 0)
        {
            oscillator[i].volume = 0;
            continue;
        }
        int16_t vol;
        uint16_t slur;
        int16_t inertia;
        uint8_t inst = chip_player[i].instrument;
    
        // run through instrument instructions when note is playing or held
        if (instrument[inst].is_drum)
        {
            // is_drum holds which end point to avoid, as well:
            while (!chip_player[i].wait && chip_player[i].cmd_index < chip_player[i].max_drum_index)
                instrument_run_command(i, inst, instrument[inst].cmd[chip_player[i].cmd_index++]);
        }
        else
        {
            while (!chip_player[i].wait && chip_player[i].cmd_index < MAX_INSTRUMENT_LENGTH) 
                instrument_run_command(i, inst, instrument[inst].cmd[chip_player[i].cmd_index++]);
        }

        if (chip_player[i].wait)
            --chip_player[i].wait;
        
        // calculate instrument frequency
        inertia = 2*(chip_player[i].inertia + chip_player[i].track_inertia);
        if (inertia) // if sliding around
        {
            slur = chip_player[i].slur;
            int16_t diff = freq_table[chip_player[i].note] - slur;
            if (diff > inertia) 
                diff = inertia;
            else if (diff < -inertia) 
                diff = -inertia;
            slur += diff;
            chip_player[i].slur = slur;
        } 
        else 
        {
            slur = freq_table[chip_player[i].note];
        }
        oscillator[i].freq = slur +
            (((chip_player[i].vibrato_depth + chip_player[i].track_vibrato_depth) * 
                sine_table[chip_player[i].vibrato_phase & 63]) >> 2);
        chip_player[i].vibrato_phase += chip_player[i].vibrato_rate + chip_player[i].track_vibrato_rate;

        vol = chip_player[i].volume + chip_player[i].volumed;
        if (vol < 0) vol = 0;
        else if (vol > 255) vol = 255;
        chip_player[i].volume = vol;
       
        if (song_wait == song_speed)
        {
            vol = chip_player[i].track_volume + chip_player[i].track_volumed;
            if (vol < 0) vol = 0;
            else if (vol > 255) vol = 255;
            chip_player[i].track_volume = vol;
        }

        oscillator[i].volume = (chip_player[i].volume * chip_player[i].track_volume) >> 8;

        // not sure if it's necessary to check duty, but we can put it back if necessary.
        //duty = oscillator[i].duty + chip_player[i].dutyd;
        //if (duty > 0xe000) duty = 0x2000; 
        //if (duty < 0x2000) duty = 0xe000; 
        //oscillator[i].duty = duty;
        // here we just let it roll over mod 2**16
        oscillator[i].duty += chip_player[i].dutyd << 6;
    }
}


// This function generates one audio sample for all 8 oscillators. The returned
// value is a 2*8bit stereo audio sample ready for putting in the audio buffer.
static inline uint16_t gen_sample()
{
    // This is a simple noise generator based on an LFSR (linear feedback shift
    // register). It is fast and simple and works reasonably well for audio.
    // Note that we always run this so the noise is not dependent on the
    // oscillators frequencies.
    static uint32_t noiseseed = 1;
    static uint32_t rednoise = 0;
    static uint32_t violetnoise = 0;
    uint32_t newbit;
    newbit = 0;
    if (noiseseed & 0x80000000L) newbit ^= 1;
    if (noiseseed & 0x01000000L) newbit ^= 1;
    if (noiseseed & 0x00000040L) newbit ^= 1;
    if (noiseseed & 0x00000200L) newbit ^= 1;
    noiseseed = (noiseseed << 1) | newbit;
    rednoise = 3*rednoise/4 + (noiseseed&255)/4;
    // violet should be the derivative of white noise, but that wasn't nice:
    // this gives some higher freqs, and a metallic ring too:
    violetnoise = violetnoise/6 + ((noiseseed&255)-128); 

    int16_t acc[2] = { 0, 0 }; // accumulators for each channel
    // Now compute the value of each oscillator and mix them
    for (int i=0; i<4; i++) 
    {
        if (!oscillator[i].side || !oscillator[i].volume)
            continue;
        
        int8_t value; // [-128, 127]

        switch (oscillator[i].waveform) 
        {
            case WF_SINE:
                //value = sine_table[oscillator[i].phase>>10]>>2;
                value = sine_table[oscillator[i].phase>>10];
                break;
            case WF_TRIANGLE:
                // Triangle: the part before 0x8000 raises, then it goes back
                // down.
                if (oscillator[i].phase < 0x8000) 
                    value = -128 + (oscillator[i].phase >> 7);
                else
                    value = 127 - ((oscillator[i].phase - 0x8000) >> 7);
                break;
            case WF_SAW:
                // Sawtooth: always raising.
                value = -128 + (oscillator[i].phase >> 8);
                break;
            case WF_PULSE:
                // Pulse: max value until we reach "duty", then min value.
                value = (oscillator[i].phase > oscillator[i].duty)? -128 : 127;
                break;
            case WF_NOISE:
                // Noise: from the generator. Only the low order bits are used.
                value = (noiseseed & 255) - 128;
                break;
            case WF_RED:
                // Red Noise, integrated from white noise..
                value = (rednoise & 255) - 128;
                break;
            case WF_VIOLET:
                // Violet Noise, derivative of white noise, at least supposedly.
                value = (violetnoise & 255) - 128;
                break;
            default:
                value = 0;
                break;
        }
        // Compute the oscillator phase (position in the waveform) for next time
        oscillator[i].phase += oscillator[i].freq / 4;

        // bit crusher effect
        value |= ((1<<oscillator[i].bitcrush) - 1); // if bitcrush == 0, does nothing

        // addition has range [-8160,7905], roughly +- 2**13
        int16_t add = (((oscillator[i].volume * value) >> 2)*chip_volume)>>8;
        
        // Mix it in the appropriate output channel
        if (oscillator[i].side & 1)
            acc[0] += add;
        if (oscillator[i].side & 2)
            acc[1] += add;
    }
    // Now put the two channels together in the output word
    // acc has roughly +- (4 instr)*2**13  needs to return as 2*[1,251],  (roughly 128 +- 2**7)
    return (128 + (acc[0] >> 8))|(((128 + (acc[1] >> 8))) << 8);  // 2*[1,251]
}

void game_snd_buffer(uint16_t* buffer, int len) 
{
    if (chip_play)
        chip_song_update();
    else if (chip_play_track)
    {
        if (song_wait) 
            --song_wait;
        else
        {
            song_wait = song_speed;
            chip_track_update();
        }
    }
    // Even if song is not playing, update oscillators in case a "chip_note" gets called.
    chip_update();
    // Generate enough samples to fill the buffer.
    for (int i=0; i<len; i++)
        buffer[i] = gen_sample();
}
