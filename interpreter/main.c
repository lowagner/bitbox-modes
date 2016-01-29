// test simple by default
#include "simple.h"
#include "screen.h"
#include "bsod.h"

typedef void (void_fn)(void);

#define INIT(x) x##_init(); a_game_frame = &x##_frame

void_fn *a_game_frame;

void a_game_init(uint16_t game)
{
    // setup a game, start the right a_game_frame
    clear();
    switch (game)
    {
    case 1:
        INIT(screen);
        break;
    default:
        INIT(bsod);
        break;
    }
}


void game_init() {
    a_game_init(1);
}


void game_frame() {
    if (a_game_frame)
        a_game_frame();
}
