#ifndef WORLD_H
#define WORLD_H
#include "bb3d.h"

extern float camera_distance;

extern struct vertex vertex[256]; // array of vertices
extern struct edge edge[256];
extern struct face face[256];

extern Camera camera;

extern int numv; // number of vertices
extern int nume; // number of edges
extern int numf; // number of faces

void world_update();
void world_init();

#endif

