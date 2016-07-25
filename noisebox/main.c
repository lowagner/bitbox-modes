#include "bitbox.h" 
#include <stdlib.h> // rand
#include <math.h>
#include <stdio.h>
#include <string.h>

uint16_t color CCM_MEMORY;
uint8_t noise_type CCM_MEMORY;

void game_init()
{
    color = 0;
}

void game_frame()
{
    static uint16_t press=0;
    static uint16_t press_hold=0;
    kbd_emulate_gamepad();
    if (press)
    {
        ++press_hold;
        press = button_state() || gamepad_buttons[0] || gamepad_buttons[1];
        if (!press)
        {
            // button release, do something fancier for holding down the button
            noise_type = (noise_type+1)%3;
        }
    }
    else
    {
        press_hold = 0;
        press = button_state() || gamepad_buttons[0] || gamepad_buttons[1];
    }
}

#ifndef NO_VGA
void graph_frame() 
{
}

void graph_line() 
{
    uint8_t max_color = 8*(noise_type+1);
    color = ((rand()%max_color)<<10)|((rand()%max_color)<<5)|(rand()%max_color);
    draw_buffer[rand()%320] = color;
}
#endif

static inline uint16_t gen_sample()
{
    // This is a simple noise generator based on an LFSR (linear feedback shift
    // register). It is fast and simple and works reasonably well for audio.
    // Note that we always run this so the noise is not dependent on the
    // oscillators frequencies.
    static uint32_t noiseseed[2] = {1, 150};
    static uint32_t rednoise[2] = {0, 3};
    static uint32_t violetnoise[2] = {0, 5};
    for (int i=0; i<2; ++i)
    {
        uint32_t newbit = 0;
        if (noiseseed[i] & 0x80000000L) newbit ^= 1;
        if (noiseseed[i] & 0x01000000L) newbit ^= 1;
        if (noiseseed[i] & 0x00000040L) newbit ^= 1;
        if (noiseseed[i] & 0x00000200L) newbit ^= 1;
        noiseseed[i] = (noiseseed[i] << 1) | newbit;
        rednoise[i] = 3*rednoise[i]/4 + (noiseseed[i]&255)/4;
        violetnoise[i] = violetnoise[i]/6 + ((noiseseed[i]&255)-128); 
    }

    uint8_t value[2];
    // Now compute the value of each oscillator and mix them
    switch (noise_type) 
    {
        case 0:
            // Red Noise, integrated from white noise..
            value[0] = (rednoise[0]&255);
            value[1] = (rednoise[1]&255);
            break;
        case 1:
            // Noise: from the generator. Only the low order bits are used.
            value[0] = (noiseseed[0]&255);
            value[1] = (noiseseed[1]&255);
            break;
        case 2:
            // Violet Noise, derivative of white noise, at least supposedly.
            value[0] = (violetnoise[0] & 255);
            value[1] = (violetnoise[1] & 255);
            break;
        default:
            value[0] = 0;
            value[1] = 0;
            break;
    }
    // Now put the two channels together in the output word
    return *((uint16_t *)value);
}

void game_snd_buffer(uint16_t *buffer, int len) 
{
    for (int i=0; i<len; i++)
        buffer[i] = gen_sample();
}
