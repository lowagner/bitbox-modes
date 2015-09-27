#include <stdlib.h> // rand
#include "simple.h"
#include "bb3d.h"

vertex v[128]; // array of vertices
int numv; // number of vertices
int vertices_sorted;

Camera camera;

inline void get_coordinates(vertex *vv)
{
    float view[3]; // coordinates of the vertex within the camera's reference frame
    // you get this by matrix multiplication:
    matrix_multiply_vector0(view, camera.view_matrix, vv->world);
    if (view[2] <= 0.0f) // vertex is on/behind the camera
    {
        // put the vertex below the screen, so it won't get drawn.
        vv->iy = 10000;
    }
    else
    {
        // apply perspective to the coordinates:
        view[0] *= camera.magnification / view[2]; 
        view[1] *= camera.magnification / view[2]; 
        vv->ix = SCREEN_W/2 + round(view[0]);
        vv->iy = SCREEN_H/2 + round(view[1]);
        // TODO:  
        // could make vertex brighter or bigger if it's closer.
        if (vv->ix >= 0 && vv->ix < SCREEN_W &&
            vv->iy >= 0 && vv->iy < SCREEN_H)
            draw_pixel(vv->ix, vv->iy, vv->color_index);

    }
    vv->iz = view[2]; // allow for testing behindness
}

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
    numv = 64; 
    for (int i=0; i<numv; ++i)
    {
        v[i] = (vertex) { 
            .world = { 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024), 0.001*(rand()%2048) },
            .color_index = i % (1 << BPP)
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
        // clear screen
        clear();
        // fix the camera at two units away from the origin
        normalize(camera.viewer, camera.viewer);
        for (int i=0; i<3; ++i)
            camera.viewer[i] *= 2;
        // need to still update the view matrix of the camera,
        get_view(&camera);
        // and then apply the matrix to all vertex positions:
        get_all_coordinates();
    }
}
