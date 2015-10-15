#include <stdlib.h> // rand
#include "nonsimple.h"
#include "bb3d.h"
#include "world.h"


void game_init()
{
    world_init();
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
        // fix the camera at four units away from the origin
        normalize(camera.viewer, camera.viewer);
        for (int i=0; i<3; ++i)
            camera.viewer[i] *= 4;
        // need to still update the view matrix of the camera,
        get_view(&camera);
        // and then apply the matrix to all vertex positions:
        world_draw();
    }
}
