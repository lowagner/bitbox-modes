#include "world.h"
#include <stdlib.h> // rand
#include <math.h> // sin and cos
#include "nonsimple.h"

struct vertex vertex[64] FAST_MEM; // array of vertices
struct edge edge[128] FAST_MEM;
struct face face[96] FAST_MEM;

int numv FAST_MEM; // number of vertices
int nume FAST_MEM; // number of edges
int numf FAST_MEM; // number of faces

uint16_t edge_color; 

Camera camera;

#define PI 3.14159265358979323f

uint16_t random_color()
{
    switch (rand()%8)
    {
        case 0:  // gray
        {
            uint8_t gray = 8 + rand()%24;
            return (gray<<10) | (gray<<5) | gray;
        }
        case 1:  // featuring red
            return ((8 + rand()%24)<<10) | ((rand()%8)<<5) | (rand()%8);
        case 2:  // featuring green
            return ((rand()%8)<<10) | ((8 + rand()%24)<<5) | (rand()%8);
        case 3:  // featuring blue
            return ((rand()%8)<<10) | ((rand()%8)<<5) | (8 + rand()%24);
        case 4:  // featuring red+green
            return ((8 + rand()%24)<<10) | ((8 + rand()%24)<<5) | (rand()%8);
        case 5:  // featuring green+blue
            return ((rand()%8)<<10) | ((8 + rand()%24)<<5) | (8 + rand()%24);
        case 6:  // featuring blue+red
            return ((8 + rand()%24)<<10) | ((rand()%8)<<5) | (8 + rand()%24);
        case 7:  // quite random
            return ((4 + rand()%28)<<10) | ((4 + rand()%28)<<5) | (4 + rand()%28);
    }
    return 0;
}

inline float distance2(float *p1, float *p2)
{
    return sqr(p1[0]-p2[0]) + sqr(p1[1]-p2[1]) + sqr(p1[2]-p2[2]);
}


inline int connect_vertex_to_edge(struct vertex *vv, struct edge *ee)
{
    int i=0;
    while (i < CONNECTED && vv->edge[i])
        ++i;
    if (i < CONNECTED)
        vv->edge[i] = ee;
    else
    {
        message("too many vertices nearby, please increase CONNECTED (bb3d.h)\n");
        return 1;
    }
    return 0;
}

inline struct edge *find_common_edge(struct vertex *v1, struct vertex *v2)
{
    for (int i=0; i<CONNECTED; ++i)
    {
        if (!v1->edge[i])
            break;
        for (int j=0; j<CONNECTED; ++j)
        {
            if (!v2->edge[j])
                break;
            if (v1->edge[i] == v2->edge[j])
                return v1->edge[i];
        }
    }
    return NULL;
}


void world_init()
{
    edge_color = RGB(200,200,200);

    srand(vga_frame);

    numv = 42;

    // bottom of sphere:
    // level 0 has just one vertex:
    vertex[0] = (struct vertex) { .world = { 0, 0.7693199+0.52573111*2, 0 } };
    for (int i=0; i<5; ++i)
    {
        float angle = 2 * PI * (i  + 0.5)/5;
        float rcosa = 1.538842*cos(angle), rsina = 1.538842*sin(-angle);
        // level 1 has 5 vertices:
        vertex[1+i] = (struct vertex) { 
            .world = { rcosa/2, 0.7693199+0.52573111, rsina/2 } 
        };
        // level 2 has 10 vertices
        vertex[6+2*i] = (struct vertex) { 
            .world = { rcosa, 0.7693199, rsina } 
        };
    }
    // the other 5 vertices in level 2 can be averaged:
    for (int i=0; i<5; ++i)
      for (int k=0; k<3; ++k)
        vertex[6+2*i+1].world[k] = 0.5*(vertex[6+2*i].world[k] + vertex[6+2*((i+1)%5)].world[k]);
    
    // do levels 6, 5, and 4, then come back for level 3:
    for (int i=0; i<16; ++i)
      for (int k=0; k<3; ++k)
        vertex[41 - i].world[k] = -vertex[i].world[k]; // mirror image
    
    // there are 10 vertices in level 3, which you can average things from level 2 and 4 to get:
    // vertices 6 through 15 are on the bottom middle, with RHR spin in the down (+y) direction
    // vertices 26 to 35 are on the top middle, with RHR spin in the up (-y) direction.
    for (int k=0; k<3; ++k)
    {
        vertex[16].world[k] = 0.5*(vertex[ 6].world[k] + vertex[31].world[k]);
        vertex[17].world[k] = 0.5*(vertex[ 6].world[k] + vertex[29].world[k]);
        vertex[18].world[k] = 0.5*(vertex[ 8].world[k] + vertex[29].world[k]);
        vertex[19].world[k] = 0.5*(vertex[ 8].world[k] + vertex[27].world[k]);
        vertex[20].world[k] = 0.5*(vertex[10].world[k] + vertex[27].world[k]);
        vertex[21].world[k] = 0.5*(vertex[10].world[k] + vertex[35].world[k]);
        vertex[22].world[k] = 0.5*(vertex[12].world[k] + vertex[35].world[k]);
        vertex[23].world[k] = 0.5*(vertex[12].world[k] + vertex[33].world[k]);
        vertex[24].world[k] = 0.5*(vertex[14].world[k] + vertex[33].world[k]);
        vertex[25].world[k] = 0.5*(vertex[14].world[k] + vertex[31].world[k]);
    }
    for (int i=0; i<numv; ++i)
        message("vertex %d:  (x,y,z) = (%f, %f, %f)\n", i, vertex[i].x, vertex[i].y, vertex[i].z);

    nume = 0;
    numf = 0;

    // lazily determine the edges.
    for (int i=0; i<numv-1; ++i)
    for (int j=i+1; j<numv; ++j)
    // vertices closer than this are connected:
    if (distance2(vertex[i].world, vertex[j].world) < 1.04f) 
    {
        //message("edge %d to %d, distance %f\n", i, j, distance2(vertex[i].world, vertex[j].world));
        edge[nume] = (struct edge) { 
            .p1 = &vertex[i], .p2 = &vertex[j],
            .f1 = NULL, .f2 = NULL
        };
        if (connect_vertex_to_edge(&vertex[i], &edge[nume]))
            message("couldn't connect edge %d to vertex %d\n", nume, i);
        if (connect_vertex_to_edge(&vertex[j], &edge[nume]))
            message("couldn't connect edge %d to vertex %d\n", nume, j);
        ++nume;
    }

    // lazily determine faces:
    for (int i=0; i<numv-2; ++i)
    for (int j=i+1; j<numv-1; ++j)
    for (int k=j+1; k<numv; ++k)
    if ((distance2(vertex[i].world, vertex[j].world) < 1.04f) &&
        (distance2(vertex[j].world, vertex[k].world) < 1.04f) &&
        (distance2(vertex[k].world, vertex[i].world) < 1.04f))
    {
        // find the common edge in i-j, j-k, and k-i:
        struct edge *e1 = find_common_edge(&vertex[i], &vertex[j]), 
                    *e2 = find_common_edge(&vertex[j], &vertex[k]), 
                    *e3 = find_common_edge(&vertex[k], &vertex[i]);
        float vij[3], vjk[3], normal[3];
        for (int l=0; l<3; ++l)
        {
            vij[l] = vertex[j].world[l] - vertex[i].world[l];
            vjk[l] = vertex[k].world[l] - vertex[j].world[l];
        }
        cross(normal, vij, vjk);
        if (dot(normal, vertex[i].world) < 0)  // make proper ccw vertex arrangement
        {
            face[numf] = (struct face) { 
                .v1 = &vertex[i], .v2 = &vertex[j], .v3 = &vertex[k], 
                .e1 = e1, .e2 = e2, .e3 = e3,
                .visible = 0, .color = random_color() 
            };
        }
        else
        {
            face[numf] = (struct face) { 
                .v1 = &vertex[i], .v2 = &vertex[k], .v3 = &vertex[j],  // note swap here.
                .e1 = e1, .e2 = e2, .e3 = e3,
                .visible = 0, .color = random_color() 
            };
        }
        // notify each edge that it has a new face:
        if (!e1->f1)
            e1->f1 = &face[numf];
        else if (!e1->f2)
            e1->f2 = &face[numf];
        else
            message("your edge got connected to too many faces somehow!\n");
        if (!e2->f1)
            e2->f1 = &face[numf];
        else if (!e2->f2)
            e2->f2 = &face[numf];
        else
            message("your edge got connected to too many faces somehow!\n");
        if (!e3->f1)
            e3->f1 = &face[numf];
        else if (!e3->f2)
            e3->f2 = &face[numf];
        else
            message("your edge got connected to too many faces somehow!\n");
        ++numf;
    }

    // setup the camera
    camera = (Camera) {
        .viewer = {0,0,4},
        .viewee = {0,0,0},
        .down = {0,1,0},
        .magnification = 110
    };
    // get the view of the camera:
    get_view(&camera);

    // get the vertices' screen positions:
    world_draw();
}

inline uint16_t get_face_color(uint16_t color, float multiplier)
{
    // use multipleir as -dot(camera.forward, face-normal) 
    return ((int)((color>>10)*multiplier)<<10) |
           ((int)(((color>>5)&31)*multiplier)<<5) |
           ((int)((color&31)*multiplier));
}

inline void draw_face(struct face *ff)
{
    //float *p1, *p2, *p3; // image coordinates of the triangle
    superpixel[ff->v1->iy][ff->v1->ix] = ff->color; 
    superpixel[ff->v2->iy][ff->v2->ix] = ff->color; 
    superpixel[ff->v3->iy][ff->v3->ix] = ff->color;
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

inline void order_edge(struct edge *ei)
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
    }
    else
    // enforce p1.y < p2.y:
    if (ei->p2->iy < ei->p1->iy)
    {
        struct vertex *old_p1 = ei->p1;
        ei->p1 = ei->p2;
        ei->p2 = old_p1;
    }
}

inline void draw_edge(struct edge *ei)
{
    if (ei->p1->iy == ei->p2->iy)
    {   // horizontal line.  p1.ix is guaranteed to be <= p2.ix
        for (uint16_t *draw = &superpixel[ei->p1->iy][ei->p1->ix];
             draw <= &superpixel[ei->p1->iy][ei->p2->ix]; draw++)
            *draw = edge_color;
        return; // finished drawing horizontal line.
    }

    // otherwise the line has some vertical displacement
    if (ei->p2->ix == ei->p1->ix)
    {   // straight vertical line
        // draw it.  p1.iy is guaranteed to be <= p2.iy
        for (int j=ei->p1->iy; j<=ei->p2->iy; ++j)
            superpixel[j][ei->p1->ix] = edge_color;
        return; // finished drawing vertical line.
    }

    // otherwise we have a diagonal line
    int dy = ei->p2->iy - ei->p1->iy; // guaranteed not to be zero
    int dx; // abs(ei->p2->ix - ei->p1->ix);
    if (ei->p2->ix > ei->p1->ix)
    {   // x2 > x1
        dx = ei->p2->ix - ei->p1->ix;
        int error = (dx>dy ? dx : -dy)/2;
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
                    int moveover = (error)/dy;
                    error -= dy * (moveover+1);
                    xleft = x;
                    xright = xleft + moveover;
                    x = xright + 1;
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
        int error = (dx>dy ? dx : -dy)/2;
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
                    int moveover = (error)/dy;
                    error -= dy * (moveover+1);
                    // moving left
                    xright = x;
                    xleft = xright - moveover;
                    x = xleft - 1;
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

void world_draw()
{
    // clear screen
    clear();
    // first get vertices
    for (int i=0; i<numv; ++i)
        get_coordinates(&vertex[i]);

    // order edges internally:
    for (int j=0; j<nume; ++j)
        order_edge(&edge[j]);

    // then compute which faces are visible:
    for (int k=0; k<numf; ++k)
    {
        struct face *fk = &face[k];
        fk->visible = is_ccw(fk->v1->image, fk->v2->image, fk->v3->image);
        if (fk->visible)
        {   
            // order the face's edges
            if (fk->left->p1 == fk->right->p1)
            {
                if (fk->bottom->p1 == fk->right->p2)
                {
                    
                }
                else // fk->bottom->p1 == fk->left->p2
                {

                }
            }
            else if (fk->left->p2 == fk->right->p2) // there's a "top" in there
            {

            }
            else // fk->left->p1 == fk->right->p2 or left->p2 == right->p1
            {

            }

            // (top->p1->iy == top->p2->iy) && (top->p1->iy < left->p2->iy) && (top->p1->iy < right->p2->iy)


            // check if it's top or bottom is straight horizontal
            if (fk->top->p1->iy == fk->top->p2->iy)
            {
            }
            else
            {

            }
        }
    }

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

