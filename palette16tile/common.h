// nonsimple.h : a simple framebuffer based engine
#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#define TILE_MAP_MEMORY 8192
#define MAX_OBJECTS 64 // no more than 255
#define SCREEN_W 320
#define SCREEN_H 240

#include "tiles.h"
#include "sprites.h"
#include "palette.h"
#include "map.h"

#define GAMEPAD_PRESS_WAIT 8

typedef enum {
    TilesAndSprites=0,
    EditTile,
    EditSprite,
    SaveScreen
} VisualMode;

extern VisualMode visual_mode;

#define GAMEPAD_PRESS(id, key) ((gamepad_buttons[id]) & (~old_gamepad[id]) & (gamepad_##key))
#define GAMEPAD_PRESSING(id, key) ((gamepad_buttons[id]) & (gamepad_##key) & (~old_gamepad[id] | ((gamepad_press_wait == 0)*gamepad_##key)))
extern uint16_t old_gamepad[2];
extern uint8_t gamepad_press_wait;
#endif
