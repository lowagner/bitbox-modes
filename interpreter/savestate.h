#ifndef SAVESTATE_H
#define SAVESTATE_H

typedef struct _yx {
    uint8_t y, x;
} yx;

typedef struct _cyx {
    uint8_t y, x;
    uint8_t y0, x0;
    uint8_t saved_c, c;
    uint16_t data;
} cyx;

extern char a_game_buffer[2][64];
extern uint16_t current_a_game;
extern uint16_t attempting_a_game;
extern cyx cursor[2];

/*
 * SCREEN_W = 80, H = 30
 */

#endif
