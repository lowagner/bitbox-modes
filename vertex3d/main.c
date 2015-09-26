#include <stdlib.h> // rand
#include "vview3d.h"
#include "bb3d.h"

vertex v[MAX_VERTICES]; // array of vertices
int numv; // number of vertices
int vertices_sorted;

Camera camera;

void get_all_coordinates()
{
    // determine the screen-coordinates of all vertices
    for (int i=0; i<numv; ++i)
    {
        get_coordinates(&v[i]);
        #ifdef DEBUG
        message("v[%d].world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, v[i].world[0], v[i].world[1], v[i].world[2], v[i].image[0], v[i].image[1], v[i].image_z);
        #endif
    }
}

void game_init()
{
    // setup the game with some random vertices
    numv = 32; 
    for (int i=0; i<numv; ++i)
    {
        v[i] = (vertex) { 
            .world = { 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024), 0.001*(rand()%2048) },
            .color = RGB(rand()%256, rand()%256, rand()%256) 
        };
        message("v[%d].(x,y,z) = (%f, %f, %f)\n", i, v[i].world[0], v[i].world[1], v[i].world[2]);
    }

    // setup the camera
    camera = (Camera) {
        .viewer = {0,0,2},
        .viewee = {0,0,0},
        .down = {0,1,0},
        .magnification = 300
    };
    // get the view of the camera:
    get_view(&camera);
    #ifdef DEBUG
    message("cam matrix: ");
    for (int i=0; i<12; ++i)
        message("%f, ", camera.view_matrix[i]);
    message("\n");
    #endif

    // get the vertices' screen positions:
    get_all_coordinates();

    // the graphing algorithm (for displaying on screen) benefits from the vertices
    // being sorted top to bottom, here we do it with heap sort, O(numv lg numv):
    heap_sort_vertices();
    #ifdef DEBUG
    for (int i=0; i<numv; ++i)
    {
        message("v[%d].world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, v[i].world[0], v[i].world[1], v[i].world[2], v[i].image[0], v[i].image[1], v[i].image_z);
    }
    #endif
}

void game_frame()
{
    kbd_emulate_gamepad();

    // move camera to arrow keys (or d-pad):
    const float delta = 0.1;
    int need_new_view = 0;
    if (GAMEPAD_PRESSED(0, left)) 
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] -= delta*camera.right[i];
        need_new_view = 1;
    }
    else if (GAMEPAD_PRESSED(0, right)) 
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] += delta*camera.right[i];
        need_new_view = 1;
    }
    if (GAMEPAD_PRESSED(0, down))
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] += delta*camera.down[i];
        need_new_view = 1;
    }
    else if (GAMEPAD_PRESSED(0, up))
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] -= delta*camera.down[i];
        need_new_view = 1;
    }
    if (need_new_view)
    {
        // fix the camera at two units away from the origin
        normalize(camera.viewer, camera.viewer);
        for (int i=0; i<3; ++i)
            camera.viewer[i] *= 2;
        // need to still update the view matrix of the camera,
        get_view(&camera);
        // and then apply the matrix to all vertex positions:
        get_all_coordinates();
        insertion_sort_vertices(); // close to O(numv) if we are nearly sorted
    }
}
