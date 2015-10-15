#ifndef WORLD_H
#define WORLD_H
#include "bb3d.h"

extern struct vertex vertex[64]; // array of vertices
extern struct edge edge[128];
extern struct face face[96];

extern Camera camera;

extern int numv; // number of vertices
extern int nume; // number of edges
extern int numf; // number of faces

void world_draw();
void world_init();

#endif

