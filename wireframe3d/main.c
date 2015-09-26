#include <stdlib.h> // rand
#include "wview3d.h"
#include "bb3d.h"

edge e[MAX_EDGES]; // array of edges
int nume; // number of vertices
#ifdef EMULATOR
extern int debug_draw;
#endif

Camera camera;

void game_init()
{
    // setup the game with some random vertices
    nume = 20;
    for (int i=0; i<4; ++i)
    {
        e[i] = (edge) { 
          .p1 = (vertex) { 
            .world = { 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024) } },
          .p2 = (vertex) { 
            .world = { 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024), 0.7+0.001*(rand()%2048) } },
          .color = RGB(0xff,0xef,0)
        };
        message("e[%02d].p1.world=(%f, %f, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2]);
        message("     .p2.world=(%f, %f, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2]);
    }
    for (int i=4; i<nume; ++i)
    {
        e[i] = (edge) { 
          .p1 = (vertex) { 
            .world = { 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024) } },
          .p2 = (vertex) { 
            .world = { 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024), 0.001*(rand()%2048-1024) } },
          .color = RGB(0xff,0xff,0xff)
        };
        message("e[%02d].p1.world=(%f, %f, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2]);
        message("     .p2.world=(%f, %f, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2]);
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
    init_drawing_edges();

    // the graphing algorithm (for displaying on screen) benefits from the edges
    // being sorted top to bottom, here we do it with heap sort, O(nume lg nume):
    heap_sort_edges();
    #ifdef DEBUG
    for (int i=0; i<nume; ++i)
    {
        message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
        message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
    }
    #endif
}

void game_frame()
{
    kbd_emulate_gamepad();

    // move camera to arrow keys (or d-pad):
    float delta = 0.1;
    if (GAMEPAD_PRESSED(0, start))
        delta = 0.01;
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
    }
    #ifdef EMULATOR
    if (GAMEPAD_PRESSED(0, start))
    {
        if (!debug_draw)
        {
            debug_draw = 2;
            for (int i=0; i<nume; ++i)
            {
                message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
                message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
            }
            message("\n");
        }
    }
    #endif
}
