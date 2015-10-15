#include <stdlib.h> // rand
#include "nonsimple.h"
#include "bb3d.h"

struct vertex vertex[64]; // array of vertices
struct edge edge[32];
struct face face[16];

uint16_t edge_color = RGB(255,255,255);

int numv; // number of vertices
int nume; // number of edges
int numf; // number of faces

Camera camera;

inline uint16_t get_face_color(uint16_t color, float multiplier)
{
    // use multipleir as -dot(camera.forward, face-normal) 
    return ((int)((color>>10)*multiplier)<<10) |
           ((int)(((color>>5)&31)*multiplier)<<5) |
           ((int)((color&31)*multiplier));
}



/*
algorithm idea:

    draw edges as white.
    fill in faces via scanline algorithm.
      start at the highest (smallest y) vertex of the triangle, increment y once.
      go left until you hit white, go right until you hit white,
      calculate and save the deltax_left and deltax_right that you had to go.
      fill in with face's normalized color.
      do:
        increment y (go down)
        find left edge and right edge via adding deltax_left/right, and then small search.
        


assuming we've already determined the orientation of the face as ccw (so that we should draw):

order points 1,2,3 by increasing y, break ties by ordering by x:

cases:
    I. 

            1

                2
        3

    II.
            1
        2
                3

    III.
        1       2

            3

    IV.  a variation of case 1 which does not need to be checked for...
            1
        2       3


I.  how to draw case I

        1

            2
    3




II.  how to draw case II.
        1
    2
            3

III.
    1       2

        3

dxL = (p3.x - p1.x)/(p3.y - p1.y)
dxR = (p3.x - p2.x)/(p3.y - p2.y)
y = p1.y + 1
xL = p1.x + dxL
xR = p2.x + dxR

if (superpixel[y][xL] is white)
    move right until superpixel[y][xL] is black
else if superpixel[y][xL-1] is white
    do nothing, this is the right spot.
else if superpixel[y][xL+1] is white
    xL = xL+1
    move right until superpixel[y][xL] is black





*/


inline void draw_face(struct face *ff)
{
    //float *p1, *p2, *p3; // image coordinates of the triangle
    superpixel[ff->v1->iy][ff->v1->ix] = RGB(255,0,0); 
    superpixel[ff->v2->iy][ff->v2->ix] = RGB(255,0,0); 
    superpixel[ff->v3->iy][ff->v3->ix] = RGB(255,0,0); 
}

inline void get_coordinates(struct vertex *vv)
{
    float view[3]; // coordinates of the vertex within the camera's reference frame
    // you get this by matrix multiplication:
    matrix_multiply_vector0(view, camera.view_matrix, vv->world);
    // the following assumes that iz > 0
    // apply perspective to the coordinates:
    view[0] *= camera.magnification / view[2]; 
    view[1] *= camera.magnification / view[2]; 
    vv->ix = SCREEN_W/2 + round(view[0]);
    vv->iy = SCREEN_H/2 + round(view[1]);
    vv->iz = view[2]; // allow for testing behindness
}

inline void draw_edge(struct edge *ei)
{
    if (ei->p1->iy == ei->p2->iy)
    {   // horizontal line
        // enforce p2->ix >= p1->ix
        if (ei->p2->ix < ei->p1->ix) // sort left-to-right
        {
            struct vertex *old_p1 = ei->p1;
            ei->p1 = ei->p2;
            ei->p2 = old_p1;
        }
        ei->scan_xL = ei->p1->ix;
        ei->scan_xR = ei->p2->ix;
        for (uint16_t *draw = &superpixel[ei->p1->iy][ei->scan_xL];
             draw <= &superpixel[ei->p1->iy][ei->scan_xR]; draw++)
            *draw = edge_color;
        return; // finish drawing horizontal line.
    }
    // otherwise the line has some vertical displacement
    // enforce p1.y < p2.y:
    if (ei->p2->iy < ei->p1->iy)
    {
        struct vertex *old_p1 = ei->p1;
        ei->p1 = ei->p2;
        ei->p2 = old_p1;
    }

    if (ei->p2->ix == ei->p1->ix)
    {   // straight vertical line
        ei->scan_xL = ei->p1->ix - 1;
        ei->scan_xR = ei->p1->ix + 1;

        // draw it:
        for (int j=ei->p1->iy; j<=ei->p2->iy; ++j)
            superpixel[j][ei->p1->ix] = edge_color;

        return; // finish drawing vertical line.
    }

    // otherwise we have a diagonal line
    int dy = ei->p2->iy - ei->p1->iy; // guaranteed not to be zero
    int dx; // abs(ei->p2->ix - ei->p1->ix);
    if (ei->p2->ix > ei->p1->ix)
    {   // x2 > x1
        dx = ei->p2->ix - ei->p1->ix;
        //int error = (dx>dy ? dx : -dy)/2;
        int error = (dx>dy ? dx : -dy/2);
        int x = ei->p1->ix;
        int y = ei->p1->iy;
        for (; y < ei->p2->iy; ++y)
        {
            if (x == ei->p2->ix) // we have achieved the final x coordinate
                superpixel[y][x] = edge_color;
            else // x2 > x1
            {
                if (error < dy) // draw just one point
                {
                    if (error > -dx) // move x
                    {
                        superpixel[y][x++] = edge_color;
                        error += dx - dy;
                    }
                    else // don't move x
                    {
                        superpixel[y][x] = edge_color;
                        error += dx;
                    }
                }
                else 
                //error > -dx and error >= dy
                {
                    int xleft, xright;
                    int moveover = (error - dy)/dy;
                    error -= dy * moveover;
                    xleft = x;
                    if (error > -dx) 
                    {
                        xright = xleft + moveover;
                        x = xright + 1;
                        error -= dy;
                    }
                    else
                    {
                        xright = xleft + moveover + 1;
                        x = xright;
                    }
                    if (xright > ei->p2->ix)
                        xright = ei->p2->ix;
                    // for going down in y
                    error += dx;

                    for (int x2=xleft; x2<=xright; ++x2)
                        superpixel[y][x2] = edge_color;
                }
            }
        }
        for (; x <= ei->p2->ix; ++x)
            superpixel[y][x] = edge_color;
    }
    else // x2 < x1
    {
        dx = -ei->p2->ix + ei->p1->ix;
        //int error = (dx>dy ? dx : -dy)/2;
        int error = (dx>dy ? dx : -dy/2);
        int x = ei->p1->ix;
        int y = ei->p1->iy;
        for (; y < ei->p2->iy; ++y)
        {
            if (x == ei->p2->ix) // we have achieved the final x coordinate
                superpixel[y][x] = edge_color;
            else // x2 > x1 or x1 < x2
            {
                if (error < dy) // draw just one point
                {
                    if (error > -dx) // move x
                    {
                        superpixel[y][x--] = edge_color;
                        error += dx - dy;
                    }
                    else // don't move x
                    {
                        superpixel[y][x] = edge_color;
                        error += dx;
                    }
                }
                else 
                //error > -dx and error >= dy
                {
                    int xleft, xright;
                    int moveover = (error - dy)/dy;
                    error -= dy * moveover;
                    // moving left
                    xright = x;
                    if (error > -dx) 
                    {
                        xleft = xright - moveover;
                        x = xleft - 1;
                        error -= dy;
                    }
                    else
                    {
                        xleft = xright - moveover - 1;
                        x = xleft;
                    }
                    if (xleft < ei->p2->ix)
                        xleft = ei->p2->ix;
                    // for going down in y
                    error += dx;

                    for (int x2=xleft; x2<=xright; ++x2)
                        superpixel[y][x2] = edge_color;
                }
            }
        }
        for (; x >= ei->p2->ix; --x)
            superpixel[y][x] = edge_color;
    }
}

void get_faces()
{
    // first get vertices
    for (int i=0; i<numv; ++i)
        get_coordinates(&vertex[i]);

    // then compute which faces are visible:
    for (int k=0; k<numf; ++k)
        face[k].visible = 1; //is_ccw(face[k].v1->image, face[k].v2->image, face[k].v3->image);

    // draw all the visible edges:
    for (int j=0; j<nume; ++j)
        // if one of the faces between the edges is visible...
        if ((edge[j].f1 && edge[j].f1->visible) || (edge[j].f2 && edge[j].f2->visible))
            // draw the edge:
            draw_edge(&edge[j]);

    // fill in the faces with the given color:
    for (int k=0; k<numf; ++k)
        if (face[k].visible)
            draw_face(&face[k]);
}

void game_init()
{
    // setup the game with some random vertices
    numv = 0;
    nume = 0;
    numf = 0;

    vertex[numv++] = (struct vertex) { .world = { 0, 0, 0 } };
    vertex[numv++] = (struct vertex) { .world = { 0, 1, 0 } };
    vertex[numv++] = (struct vertex) { .world = { 1, 1, 0 } };
    face[numf++] = (struct face) { .v1 = &vertex[0], .v2 = &vertex[1], .v3 = &vertex[2], .visible = 1, .color = RGB(255,0,0) };

    vertex[numv++] = (struct vertex) { .world = { -1, 1, 0 } };
    face[numf++] = (struct face) { .v1 = &vertex[0], .v2 = &vertex[3], .v3 = &vertex[1], .visible = 1, .color = RGB(255,0,0) };
    edge[nume++] = (struct edge) { .p1 = &vertex[0], .p2 = &vertex[1], .f1 = &face[0], .f2 = &face[1] };


    // setup the camera
    camera = (Camera) {
        .viewer = {0,0,4},
        .viewee = {0,0,0},
        .down = {0,1,0},
        .magnification = 130
    };
    // get the view of the camera:
    get_view(&camera);

    // get the vertices' screen positions:
    get_faces();

    #ifdef DEBUG
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
        // fix the camera at four units away from the origin
        normalize(camera.viewer, camera.viewer);
        for (int i=0; i<3; ++i)
            camera.viewer[i] *= 4;
        // need to still update the view matrix of the camera,
        get_view(&camera);
        // and then apply the matrix to all vertex positions:
        get_faces();
    }
}
