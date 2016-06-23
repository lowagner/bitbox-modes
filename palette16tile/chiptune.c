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
uint8_t chip_repeat CCM_MEMORY;

/* 
    instruments
    you get one for each channel.
    you have a max of MAX_INSTRUMENT_LENGTH commands with parameters 

    there's a command (&15) and a parameter (>>4) embedded in each uint8_t cmd[].

    TODO:  multiply parameter by 16 to get full range for certain commands.
 */

struct instrument instrument[CHANNELS] CCM_MEMORY;

/* 
    tracks

    chip_track[t][i][0] -> key for track t with instrument i
    chip_track[t][i][1] -> first note and second note
 */

uint8_t track_pos CCM_MEMORY;
uint8_t track_length CCM_MEMORY;
uint8_t chip_track[16][4][1 + MAX_TRACK_LENGTH/2] CCM_MEMORY; 

/* 
    chip song
    is made of 16 different tracks for each instrument/channel.
 */

uint16_t chip_song[MAX_SONG_LENGTH]; // a nibble for the track to play for each channel
uint8_t song_wait CCM_MEMORY; // >0 means wait N frames, 0 means play now. 
uint8_t song_speed CCM_MEMORY;
uint8_t song_pos CCM_MEMORY;
uint8_t song_length CCM_MEMORY; // capped at MAX_SONG_LENGTH
int8_t song_transpose CCM_MEMORY;

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



static void instrument_run_cmd(uint8_t i, uint8_t cmd) 
{
    uint8_t param = cmd >> 4;
    switch(cmd&15) 
    {
        case 0:
            instrument[i].cmd_index = MAX_INSTRUMENT_LENGTH; // end instrument commmands
            break;
        case 1: // s = switch side (L/R)
            instrument[i].side = param; // 0 = no side / silence!, 1 = L, 2 = R, 3 = L/R
            break;
        case 2: // v = volume
            instrument[i].volume = (param<<4) + 1;
            break;
        case 3: // w = select waveform
            instrument[i].waveform = param;
            break;
        case 4: // + = set relative note
            instrument[i].note = param + instrument[i].track_note + instrument[i].track_octave*12 + song_transpose;
            break;
        case 5: // = = set absolute note 
            instrument[i].note = param + instrument[i].track_octave*12 + song_transpose;
            break;
        case 6: // t = timing 
            instrument[i].wait = param;
            break;
        case 7: // < = fade in, or crescendo
            instrument[i].volumed = param<<4;
            break;
        case 8: // > = fade out, or decrescendo
            instrument[i].volumed = -(param<<4);
            break;
        case 9: // ~ = vibrato depth
            instrument[i].vibrato_depth = param;
            break;
        case 10: // x = vibrato rate
            instrument[i].vibrato_rate = param;
            break;
        case 11: // i = inertia (auto note slides)
            instrument[i].inertia = param << 5;
            break;
        case 12: // b = bitcrush
            instrument[i].bitcrush = param;
            break;
        case 13: // d = duty cycle.  param==8 makes for a square wave if waveform is WF_PUL
            instrument[i].duty = param << 12;
            break;
        case 14: // m = duty variation
            instrument[i].dutyd = param << 10;
            break;
        case 15: // j = instrument jump
            instrument[i].cmd_index = param;
            break;
    }
}

//static void track_run_cmd(uint8_t ch, uint8_t cmd) 
//{
//    switch(cmd) {
//        case 1: // 0 = note off
//            instrument[ch].track_volume = 0;
//            break;
//        case 2: // d = duty cycle
//            instrument[ch].duty = param << 8;
//            break;
//        case 3: // f = volume slide
//            instrument[ch].volumed = param;
//            break;
//        case 4: // i = inertia (auto note slides)
//            instrument[ch].inertia = param << 1;
//            break;
//        case 5: // j = instrument jump
//            instrument[ch].cmd_index = param;
//            break;
//        case 6: // used to be slide
//            break;
//        case 7: // m = duty variation
//            channel[ch].dutyd = param << 6;
//            break;
//        case 8: // t = timing (for instrument, song speed for track context)
//            if (!context)
//                channel[ch].instrument_wait = param;
//            else
//                song_speed = param;
//            break;
//        case 9: // v = volume
//            channel[ch].volume = param;
//            break;
//        case 10: // w = select waveform
//            channel[ch].waveform = param;
//            break;
//        case 11: // ~ = vibrato
//            if (channel[ch].vibrato_depth != (param >> 4)) {
//                channel[ch].vibrato_phase = 0;
//            }
//            channel[ch].vibrato_depth = param >> 4;
//            channel[ch].vibrato_rate = param & 15;
//            break;
//        case 12: // + = set relative note
//            instrument[ch].note = param + instrument[ch].track_note - 12 * 4;
//            break;
//        case 13: // = = set absolute note in instrument context or instrument in pattern context
//            if (!context)
//                channel[ch].instrument_note = param;
//            else
//                channel[ch].lastinstr = param;
//            break;
//        case 14:
//            channel[ch].bitcrush=param;
//    }
//}


void chip_init()
{
}

void chip_reset()
{
    song_length = 16;
    track_length = 32;

}

void chip_switch() 
{
    song_wait = 0;
    track_pos = 0;
    chip_play = 1;
    song_pos = 0;
    song_speed = 8; // default speed

    for (int i=0; i<4; i++) 
    {
        instrument[i].volume = 0;
        instrument[i].bitcrush = 5;
    }
}

void chip_note(uint8_t i, uint8_t note)
{
    instrument[i].cmd_index = 0;
    instrument[i].previous_track_note = instrument[i].track_note;
    instrument[i].track_note = note;
    instrument[i].volumed = 0;
    instrument[i].track_volume = 240;
    instrument[i].track_volumed = 0;
    instrument[i].wait = 0;
    instrument[i].dutyd = 0;
    instrument[i].vibrato_depth = 0;
    if (instrument[i].track_emphasis)
        --instrument[i].track_emphasis;
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
        
        uint16_t tracks = chip_song[song_pos];
        instrument[0].track_num = tracks & 15;
        instrument[1].track_num = (tracks >> 4) & 15;
        instrument[2].track_num = (tracks >> 8) & 15;
        instrument[3].track_num = tracks >> 12;

        for (int i=0; i<4; ++i)
            instrument[i].track_read_pos = 0;
        
        song_pos++;
    }

    #ifdef DEBUG_CHIPTUNE
    message ("%d |", track_pos);
    #endif

    for (int i=0; i<4; ++i) 
    {
        if (instrument[i].track_read_pos > track_pos)
            continue;

        uint8_t fields = chip_track[instrument[i].track_num][i][1+instrument[i].track_read_pos/2];
        if (instrument[i].track_read_pos % 2)
        {
            uint8_t note = fields >> 4;
            if (note >= 4)
            {
                chip_note(i, note-4 + instrument[i].track_octave*12);
                continue;
            }
        }
        else
        {
            uint8_t note = fields & 15;
            if (note >= 4)
            {
                chip_note(i, note-4 + instrument[i].track_octave*12);
                continue;
            }
        }
        
        ++instrument[i].track_read_pos;
    }
    #ifdef DEBUG_CHIPTUNE
    message("\n");
    #endif

    if (++track_pos == track_length)
        track_pos = 0;
}

static void chip_update()
{
    for (int i=0; i<4; ++i) 
    {
        if (!instrument[i].track_volume)
            continue;
        int16_t vol;
        uint16_t slur;
        int16_t inertia;

        while (!instrument[i].wait && instrument[i].cmd_index < MAX_INSTRUMENT_LENGTH) {
            // run through instrument instructions when note is playing or held
            instrument_run_cmd(i, instrument[i].cmd[instrument[i].cmd_index++]);
        }

        if (instrument[i].wait)
            --instrument[i].wait;
        
        // calculate instrument frequency
        inertia = instrument[i].inertia + instrument[i].inertia;
        if (inertia) // if sliding around
        {
            slur = instrument[i].slur;
            int16_t diff = freq_table[instrument[i].note] - slur;
            if (diff > 0) 
            {
                if (diff > inertia) 
                    diff = inertia;
            } 
            else if (diff < 0) 
            {
                if (diff < -inertia) 
                    diff = -inertia;
            }
            slur += diff;
            instrument[i].slur = slur;
        } 
        else 
        {
            slur = freq_table[instrument[i].note];
        }
        instrument[i].freq = slur +
            (((instrument[i].vibrato_depth + instrument[i].track_vibrato_depth) * 
                sine_table[instrument[i].vibrato_phase & 63]) >> 2);

        vol = instrument[i].volume + instrument[i].volumed;
        if (vol < 0) vol = 0;
        if (vol > 255) vol = 255;
        instrument[i].volume = vol;
        
        vol = instrument[i].track_volume + instrument[i].track_volumed;
        if (vol < 0) vol = 0;
        if (vol > 240) vol = 240;
        instrument[i].track_volume = vol;

        // not sure if it's necessary to check duty, but we can put it back if necessary.
        //duty = instrument[i].duty + instrument[i].dutyd;
        //if (duty > 0xe000) duty = 0x2000; 
        //if (duty < 0x2000) duty = 0xe000; 
        //instrument[i].duty = duty;
        // here we just let it roll over mod 2**16
        instrument[i].duty += instrument[i].dutyd;

        instrument[i].vibrato_phase += instrument[i].vibrato_rate + instrument[i].track_vibrato_rate;
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
    uint32_t newbit;
    newbit = 0;
    if (noiseseed & 0x80000000L) newbit ^= 1;
    if (noiseseed & 0x01000000L) newbit ^= 1;
    if (noiseseed & 0x00000040L) newbit ^= 1;
    if (noiseseed & 0x00000200L) newbit ^= 1;
    noiseseed = (noiseseed << 1) | newbit;

    int16_t acc[2] = { 0, 0 }; // accumulators for each channel
    uint8_t div[2] = { 0, 0 }; // dividers, how many instruments have put stuff into which channels

    // Now compute the value of each oscillator and mix them
    for (int i=0; i<4; i++) 
    {
        if (!instrument[i].side || !instrument[i].track_volume)
            continue;
        
        int8_t value; // [-32,31]

        switch(instrument[i].waveform) 
        {
            case WF_TRI:
                // Triangle: the part before 0x8000 raises, then it goes back
                // down.
                if (instrument[i].phase < 0x8000) 
                    value = -32 + (instrument[i].phase >> 9);
                else
                    value = 31 - ((instrument[i].phase - 0x8000) >> 9);
                break;
            case WF_SAW:
                // Sawtooth: always raising.
                value = -32 + (instrument[i].phase >> 10);
                break;
            case WF_PUL:
                // Pulse: max value until we reach "duty", then min value.
                value = (instrument[i].phase > instrument[i].duty)? -32 : 31;
                break;
            case WF_NOI:
                // Noise: from the generator. Only the low order bits are used.
                value = (noiseseed & 63) - 32;
                break;
            case WF_SIN:
                value = sine_table[instrument[i].phase>>10]>>2;
                break;
            default:
                value = 0;
                break;
        }
        // Compute the oscillator phase (position in the waveform) for next time
        instrument[i].phase += instrument[i].freq / 4;

        // bit crusher effect
        value |= ((1<<instrument[i].bitcrush) - 1); // if bitcrush == 0, does nothing

        // addition has range [-8160,7905], roughly +- 2**13
        int16_t add = instrument[i].track_emphasis ? 
            (instrument[i].track_volume+15) * instrument[i].volume / 255 * value :
            instrument[i].track_volume * instrument[i].volume / 255 * value;
        // Mix it in the appropriate output channel
        if (instrument[i].side & 1)
        {
            acc[0] += add;
            ++div[0];
        }
        if (instrument[i].side & 2)
        {
            acc[1] += add;
            ++div[1];
        }
    }
    // Now put the two channels together in the output word
    // acc has +- div[c]*2**13  needs to return as 2*[1,251],  (roughly 128 +- 2**7)
    // so shift down 
    uint16_t result = 0;
    if (div[0])
        result |= (128 + (acc[0] >> 6)/div[0]);
    if (div[1])
        result |= ((128 + (acc[1] >> 6)/div[1])) << 8;
    // if it sounds bad to promote channels, then just shift down by 8 and call it a day.
    return result;  // 2*[1,251]
}

void game_snd_buffer(uint16_t* buffer, int len) 
{
    if (chip_play)
        chip_song_update();
    // Even if song is not playing, update oscillators in case a "chip_note" gets called.
    chip_update();
    // Generate enough samples to fill the buffer.
    for (int i=0; i<len; i++)
        buffer[i] = gen_sample();
}
