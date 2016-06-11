// nonsimple.h : a simple framebuffer based engine
#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#define TILE_MAP_MEMORY 8192
#define MAX_OBJECTS 64 // no more than 255
#define SCREEN_W 320
#define SCREEN_H 240

typedef enum {
    Passable=0,
    PassableDamage=1,
    Breakable=2,
    BreakableDamage=3,
    Hard=4,
    HardDamage=5,
    Slippery=6,
    SlipperyDamage=7,
    Sticky=8,
    StickyDamage=9,
    Bouncy=10,
    BouncyDamage=11
} SideType; // no more than 16 total types, so from 0 to 15, with damage as odd.

#include "tiles.h"
#include "sprites.h"
#include "palette.h"
#include "map.h"

#define GAMEPAD_PRESS_WAIT 8

typedef enum {
    None=0,
    TilesAndSprites,
    EditTileOrSprite,
    EditTileOrSpriteProperties,
    EditPalette,
    SaveLoadScreen,
    ChooseFilename
} VisualMode;

extern VisualMode visual_mode;
extern VisualMode previous_visual_mode;

#define DOWN 0
#define RIGHT 1
#define LEFT 2
#define UP 3

#define GAMEPAD_PRESS(id, key) ((gamepad_buttons[id]) & (~old_gamepad[id]) & (gamepad_##key))
#define GAMEPAD_PRESSING(id, key) ((gamepad_buttons[id]) & (gamepad_##key) & (~old_gamepad[id] | ((gamepad_press_wait == 0)*gamepad_##key)))
extern uint16_t old_gamepad[2];
extern uint8_t gamepad_press_wait;

extern uint8_t game_message[32];
extern const uint8_t hex[32]; // not exactly hex but ok!
extern const uint8_t direction[4];

void draw_parade(int line, uint8_t bg_color);
#endif
