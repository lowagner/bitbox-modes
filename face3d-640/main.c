#include "bb3d.h"
#include "world.h"


void game_init()
{
    world_init();
}

void game_frame()
{
    // move camera to arrow keys (or d-pad):
    const float delta = 0.1;
    const float delta_rot = 0.01;
    int need_new_view = 0;
    if (GAMEPAD_PRESSED(0, L)) 
    {
        float old_right[3] = { camera.right[0], camera.right[1], camera.right[2] };
        for (int i=0; i<3; ++i)
            camera.right[i] += delta_rot*camera.down[i];
        for (int i=0; i<3; ++i)
            camera.down[i] -= delta_rot*old_right[i];
        need_new_view = 2;
    }
    if (GAMEPAD_PRESSED(0, R)) 
    {
        float old_right[3] = { camera.right[0], camera.right[1], camera.right[2] };
        for (int i=0; i<3; ++i)
            camera.right[i] -= delta_rot*camera.down[i];
        for (int i=0; i<3; ++i)
            camera.down[i] += delta_rot*old_right[i];
        need_new_view = 2;
    }
    if (GAMEPAD_PRESSED(0, left)) 
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] -= delta*camera.right[i];
        need_new_view |= 1;
    }
    else if (GAMEPAD_PRESSED(0, right)) 
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] += delta*camera.right[i];
        need_new_view |= 1;
    }
    if (GAMEPAD_PRESSED(0, down))
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] += delta*camera.down[i];
        need_new_view |= 1;
    }
    else if (GAMEPAD_PRESSED(0, up))
    {
        for (int i=0; i<3; ++i)
            camera.viewer[i] -= delta*camera.down[i];
        need_new_view |= 1;
    }
    if (need_new_view)
    {
        if (need_new_view & 2)
        {
            normalize(camera.right, camera.right);
            normalize(camera.down, camera.down);
        }
        if (need_new_view & 1)
        {
            // fix the camera at four units away from the origin
            normalize(camera.viewer, camera.viewer);
            for (int i=0; i<3; ++i)
                camera.viewer[i] *= camera_distance;
        }
        // need to still update the view matrix of the camera,
        get_view(&camera);
        // and then apply the matrix to all vertex positions:
        world_update();
    }
    graph_frame();
}
