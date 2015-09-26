// vview3d.h: vertex view 3d
#ifndef VERTEX_VIEW3D_H
#define VERTEX_VIEW3D_H
#include "bitbox.h"

#define SCREEN_W 640
#define SCREEN_H 400
#include "bb3d.h" // need to put SCREEN_W/H before this include.

#define MAX_VERTICES 64

extern vertex v[MAX_VERTICES]; // array of vertices
extern int numv; // number of vertices
extern Camera camera;

void heap_sort_vertices();
void insertion_sort_vertices();

inline void get_coordinates(vertex *vv)
{
    float view[3]; // coordinates of the vertex within the camera's reference frame
    // you get this by matrix multiplication:
    matrix_multiply_vector0(view, camera.view_matrix, vv->world);
    if (view[2] <= 0.0f) // vertex is on/behind the camera
    {
        // put the vertex below the screen, so it won't get drawn.
        vv->image[1] = 10000;
    }
    else
    {
        // apply perspective to the coordinates:
        view[0] *= camera.magnification / view[2]; 
        view[1] *= camera.magnification / view[2]; 
        vv->image[0] = SCREEN_W/2 + round(view[0]);
        vv->image[1] = SCREEN_H/2 + round(view[1]);
    }
    vv->image_z = view[2]; // allow for testing behindness
}

#endif
