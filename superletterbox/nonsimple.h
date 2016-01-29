// nonsimple.h : a simple frmebuffer based engine
#ifndef NONSIMPLE_H
#define NONSIMPLE_H
#include "bitbox.h"

#define SCREEN_W 320
#define SCREEN_H 240

#define Nx 320
#define Ny 100

void clear();
extern uint16_t vram[Ny][Nx];
extern uint16_t offset_y;

#endif
