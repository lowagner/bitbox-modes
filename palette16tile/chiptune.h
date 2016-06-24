/** Initialize the player to play the given song.
 * - Oscillators are reset to silence
 * - Song position is reset to 0
 * - Tracks and instruments are loaded from the song.
 *
 * Pass NULL to stop playing and not load a new song.
 */
/* Simple soundengine for the BitBox
 * Copyright 2014, Adrien Destugues <pulkomandy@pulkomandy.tk>
 * Copyright 2007, Linus Akesson
 * Based on the "Hardware Chiptune" project */
#ifndef CHIPTUNE_H
#define CHIPTUNE_H
#include <stdint.h>

#define MAX_INSTRUMENT_LENGTH 16
#define MAX_DRUM_LENGTH (MAX_INSTRUMENT_LENGTH/4)
#define MAX_SONG_LENGTH 32
#define MAX_TRACK_LENGTH 38 // make this divisible by 2, with result+1 divisible by 4
#define MAX_NOTE 84
#define CHANNELS 4

extern uint8_t chip_play_song;

struct instrument 
{
    // properties related to things you can change from song
    uint8_t track_num;
    uint8_t track_read_pos; // because of control notes, you can be a bit ahead of the actual track_pos

    // commands which create the instrument sound
    // stuff in the cmd array can be modified externally.
    uint8_t cmd[MAX_INSTRUMENT_LENGTH];
    uint8_t cmd_index;
    uint8_t is_drum;
    
    // properties specific to the instrument (set/modified using cmd array above)
    // and properties which can be set by the track (prefixed by `track_`).
    uint8_t wait;

    uint8_t track_sounding; // whether playing a note or not
    uint8_t side; // 0 = no sound, 1 = left side, 2 = right side, 3 = left and right side

    uint8_t track_octave;
    uint8_t note; // actual note being played, chosen using cmd array and track_note:
    uint8_t track_note; // note supposed to be played
    uint8_t previous_track_note; // previous note which was supposed to be played

    uint8_t vibrato_depth;
    uint8_t track_vibrato_depth;
    uint8_t vibrato_rate;
    uint8_t track_vibrato_rate;
    uint8_t vibrato_phase;

    int16_t inertia;
    int16_t track_inertia;
    uint16_t slur; // internally how we keep track of note with inertia
    
    uint16_t duty; // duty cycle (pulse wave only)
    uint16_t dutyd;
    
    uint8_t volume; // 0-255
    uint8_t track_volume; // 0-240
    uint8_t track_emphasis; // +15 to track_volume, how many times to do this.
    int8_t volumed;
    int8_t track_volumed;
    
    uint16_t freq; // frequency (except for noise, unused)
    uint16_t phase; // phase (except for noise, unused)
    uint8_t waveform; // waveform (from the enum above)
    uint8_t bitcrush; // 0-f level of quantization (power of 2)
}; 

extern struct instrument instrument[CHANNELS];

void chip_init(); // initialize all variables at start of game (stuff that only happens once)
void chip_reset(); // put in a random tune.
void chip_switch(); // re-initialize current tune to start over.

// play a note of this instrument now - useful for FX !
void chip_note(uint8_t ch, uint8_t note, uint8_t track_volume);

// These are our possible waveforms. Any other value plays silence.
enum 
{
    WF_SINE = 0,  // 
    WF_TRIANGLE, // = /\/\,
    WF_SAW, // = /|/| 
    WF_PULSE, // = |_|- (adjustable duty)
    WF_NOISE, // = !*@?
};

#define BREAK 0
#define SIDE 1
#define VOLUME 2
#define WAVEFORM 3
#define NOTE 4
// define ?? 5
#define WAIT 6
#define FADE_IN 7
#define FADE_OUT 8
#define VIBRATO 9
#define VIBRATO_RATE 10
#define INERTIA 11
#define BITCRUSH 12
#define DUTY 13
#define DUTY_DELTA 14
#define JUMP 15

#endif
