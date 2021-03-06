#include "world.h"
#include <stdlib.h> // rand
#include <math.h> // sin and cos
#include <string.h> // memset 

#define SCREEN_W 640
#define SCREEN_H 480

struct vertex vertex[256] CCM_MEMORY; // array of vertices
struct face face[256] CCM_MEMORY;

int numv CCM_MEMORY; // number of vertices
int numf CCM_MEMORY; // number of faces

uint8_t y_draw_order[256] CCM_MEMORY; 
#define DRAW_COUNT y_draw_order[0]
int y_draw_index CCM_MEMORY;

int32_t matrix_changed; // remove this later

#define FACE_TOP_X(k) vertex[face[k].o1].ix
#define FACE_TOP(k) vertex[face[k].o1].iy

#define FACE_MIDDLE_X(k) vertex[face[k].o2].ix
#define FACE_MIDDLE_Y(k) vertex[face[k].o2].iy

#define FACE_BOTTOM_X(k) vertex[face[k].o3].ix
#define FACE_BOTTOM(k) vertex[face[k].o3].iy

Camera camera CCM_MEMORY;
float camera_distance CCM_MEMORY;

#define PI 3.14159265358979323f

uint16_t face_color[256];

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


static inline uint8_t connect_vertex_to_vertex(uint8_t vi, uint8_t vj)
{
    int retval = 0;
    struct vertex *vv = &vertex[vi];
    int i=0;
    while (i < CONNECTED && vv->cxn[i])
        ++i;
    if (i < CONNECTED)
        vv->cxn[i] = vj;
    else
    {
        message("too many vertices nearby, please increase CONNECTED (bb3d.h)\n");
        retval |= 1;
    }
    vv = &vertex[vj];
    i=0;
    while (i < CONNECTED && vv->cxn[i])
        ++i;
    if (i < CONNECTED)
        vv->cxn[i] = vi;
    else
    {
        message("too many vertices nearby, please increase CONNECTED (bb3d.h)\n");
        retval |= 2;
    }
    return retval;
}

static inline void swap_y_draw_order_j_jplus1(int j)
{
    int face_jplus1 = y_draw_order[j+1];
    int face_j = y_draw_order[j];
    
    y_draw_order[j+1] = face_j;
    y_draw_order[j] = face_jplus1;

    face[face_jplus1].draw_order = j;
    face[face_j].draw_order = j+1;
}

static inline void sort_faces_y()
{
    for (int i=2; i<=DRAW_COUNT; ++i)
    {
        uint8_t tos = y_draw_order[i];
        if (!tos)
        {
            message("unexpected null y_draw_order at %d\n", tos);
            return;
        }
        int32_t tos_value = FACE_TOP(tos);
        for (int j=i-1; (j>0) && (tos_value < FACE_TOP(y_draw_order[j])); --j)
            swap_y_draw_order_j_jplus1(j);
    }
}

void world_init()
{
    camera_distance = 3;

    DRAW_COUNT = 0;
    y_draw_index = 1;
    y_draw_order[1] = 0;

    numv = 42;

    // bottom of sphere:
    // level 0 has just one vertex:
    vertex[1] = (struct vertex) { .world = { 0, 0.7693199+0.52573111*2, 0 } };
    for (int i=0; i<5; ++i)
    {
        float angle = 2 * PI * (i  + 0.5)/5;
        float rcosa = 1.538842*cos(angle), rsina = 1.538842*sin(-angle);
        // level 1 has 5 vertices:
        vertex[2+i] = (struct vertex) { 
            .world = { rcosa/2, 0.7693199+0.52573111, rsina/2 } 
        };
        // level 2 has 10 vertices
        vertex[1+6+2*i] = (struct vertex) { 
            .world = { rcosa, 0.7693199, rsina } 
        };
    }
    // the other 5 vertices in level 2 can be averaged:
    for (int i=0; i<5; ++i)
      for (int k=0; k<3; ++k)
        vertex[1+6+2*i+1].world[k] = 0.5*(vertex[1+6+2*i].world[k] + vertex[1+6+2*((i+1)%5)].world[k]);
    
    // do levels 6, 5, and 4, then come back for level 3:
    for (int i=0; i<16; ++i)
      for (int k=0; k<3; ++k)
        vertex[42 - i].world[k] = -vertex[i+1].world[k]; // mirror image
    
    // there are 10 vertices in level 3, which you can average things from level 2 and 4 to get:
    // vertices 6 through 15 are on the bottom middle, with RHR spin in the down (+y) direction
    // vertices 26 to 35 are on the top middle, with RHR spin in the up (-y) direction.
    for (int k=0; k<3; ++k)
    {
        vertex[1+16].world[k] = 0.5*(vertex[1+ 6].world[k] + vertex[1+31].world[k]);
        vertex[1+17].world[k] = 0.5*(vertex[1+ 6].world[k] + vertex[1+29].world[k]);
        vertex[1+18].world[k] = 0.5*(vertex[1+ 8].world[k] + vertex[1+29].world[k]);
        vertex[1+19].world[k] = 0.5*(vertex[1+ 8].world[k] + vertex[1+27].world[k]);
        vertex[1+20].world[k] = 0.5*(vertex[1+10].world[k] + vertex[1+27].world[k]);
        vertex[1+21].world[k] = 0.5*(vertex[1+10].world[k] + vertex[1+35].world[k]);
        vertex[1+22].world[k] = 0.5*(vertex[1+12].world[k] + vertex[1+35].world[k]);
        vertex[1+23].world[k] = 0.5*(vertex[1+12].world[k] + vertex[1+33].world[k]);
        vertex[1+24].world[k] = 0.5*(vertex[1+14].world[k] + vertex[1+33].world[k]);
        vertex[1+25].world[k] = 0.5*(vertex[1+14].world[k] + vertex[1+31].world[k]);
    }
    for (int i=1; i<=numv; ++i)
        message("vertex %d:  (x,y,z) = (%f, %f, %f)\n", i, vertex[i].x, vertex[i].y, vertex[i].z);

    numf = 0;

    // lazily determine the edges.
    for (int i=1; i<numv; ++i)
    for (int j=i+1; j<=numv; ++j)
    // vertices closer than this are connected:
    if (distance2(vertex[i].world, vertex[j].world) < 1.04f) 
    {
        if (connect_vertex_to_vertex(i, j))
            message("couldn't connect vertex %d to vertex %d\n", j, i);
    }

    // lazily determine faces:
    for (int i=1; i<numv-1; ++i)
    for (int j=i+1; j<numv; ++j)
    for (int k=j+1; k<=numv; ++k)
    if ((distance2(vertex[i].world, vertex[j].world) < 1.04f) &&
        (distance2(vertex[j].world, vertex[k].world) < 1.04f) &&
        (distance2(vertex[k].world, vertex[i].world) < 1.04f))
    {
        float vij[3], vjk[3], normal[3];
        for (int l=0; l<3; ++l)
        {
            vij[l] = vertex[j].world[l] - vertex[i].world[l];
            vjk[l] = vertex[k].world[l] - vertex[j].world[l];
        }
        cross(normal, vij, vjk);
        if (dot(normal, vertex[i].world) < 0)  // make proper ccw vertex arrangement
        {
            face[++numf] = (struct face) { 
                .v1 = i, .v2 = j, .v3 = k, 
                .visible = 0, 
                .o1 = i, .o2 = j, .o3 = k
            };
        }
        else
        {
            face[++numf] = (struct face) { 
                .v1 = i, .v2 = k, .v3 = j,  // note swap here.
                .visible = 0, 
                .o1 = i, .o2 = j, .o3 = k
            };
        }
    }

    message("got faces %d and vertices %d\n", numf, numv);
    // setup the camera
    camera = (Camera) {
        .viewer = {0,0,camera_distance},
        .viewee = {0,0,0},
        .down = {0,1,0},
        .magnification = 210
    };
    // get the view of the camera:
    get_view(&camera);
    
    srand(vga_frame);
    for (int i=1; i<=numf; ++i)
        face_color[i] = random_color();

    // get the vertices' screen positions:
    world_update();
}

inline uint16_t get_face_color(uint16_t color, float multiplier)
{
    // use multipleir as -dot(camera.forward, face-normal) 
    return ((int)((color>>10)*multiplier)<<10) |
           ((int)(((color>>5)&31)*multiplier)<<5) |
           ((int)((color&31)*multiplier));
}

inline void get_coordinates(uint8_t vi)
{
    float view[3]; // coordinates of the vertex within the camera's reference frame
    // you get this by matrix multiplication:
    matrix_multiply_vector0(view, camera.view_matrix, vertex[vi].world);
    // the following assumes that iz > 0
    // apply perspective to the coordinates:
    view[0] *= camera.magnification / view[2]; 
    view[1] *= camera.magnification / view[2]; 
    vertex[vi].ix = SCREEN_W/2 + round(view[0]);
    vertex[vi].iy = SCREEN_H/2 + round(view[1]);
    vertex[vi].iz = view[2]; // allow for testing behindness
}

inline void compute_face(uint8_t k)
{
    struct face *fk = &face[k];
    int new_visible = is_ccw(vertex[fk->v1].image, vertex[fk->v2].image, vertex[fk->v3].image);
    if (fk->visible) // was originally visible
    {
        fk->visible = new_visible;
        if (!new_visible) // now not visible
        {
            // remove face from y_draw_order
            for (int j = face[k].draw_order; j<DRAW_COUNT; ++j)
            {
                y_draw_order[j] = y_draw_order[j+1];
                face[y_draw_order[j]].draw_order = j;
            }
            --DRAW_COUNT;
            return;
        }
    }
    else // wasn't visible
    {
        fk->visible = new_visible;
        if (!new_visible) // still not visible
            return;
        // now visible, need to add face to y_draw_order
        if (DRAW_COUNT < 255)
        {
            y_draw_order[++DRAW_COUNT] = k;
            face[k].draw_order = DRAW_COUNT;
        }
        else
        {
            message("too many faces on screen... shouldn't be possible\n");
            return;
        }
    }
    
    int32_t o1 = vertex[face[k].o1].iy;
    int32_t o2 = vertex[face[k].o2].iy;
    int32_t o3 = vertex[face[k].o3].iy;

    if (o1 <= o2)
    {
        // o1 <= o2
        if (o2 <= o3)
        {
            // o1 <= o2 <= o3
            // vertex_order is good. 
        } 
        else if (o1 <= o3)
        {
            // o1 <= o3 < o2
            // need to swap order 2 and 3:
            uint8_t old_o2 = face[k].o2;
            face[k].o2 = face[k].o3;
            face[k].o3 = old_o2;
        }
        else
        {
            // o3 < o1 <= o2
            uint8_t old_o2 = face[k].o2;
            face[k].o2 = face[k].o1;
            face[k].o1 = face[k].o3;
            face[k].o3 = old_o2;
        }
    }
    else
    {
        // o2 < o1
        if (o1 <= o3)
        {
            // o2 < o1 <= o3
            uint8_t old_o2 = face[k].o2;
            face[k].o2 = face[k].o1;
            face[k].o1 = old_o2;
        }
        else if (o2 <= o3)
        {
            // o2 <= o3 < o1
            uint8_t old_o2 = face[k].o2;
            face[k].o2 = face[k].o3;
            face[k].o3 = face[k].o1;
            face[k].o1 = old_o2;
        }
        else
        {
            // o3 < o2 < o1
            uint8_t old_o1 = face[k].o1;
            face[k].o1 = face[k].o3;
            face[k].o3 = old_o1;
        }
    }
    float normal[3];
    get_normal(normal, vertex[face[k].v1].world, vertex[face[k].v2].world, vertex[face[k].v3].world);
    float multiplier = dot(normal, camera.forward);
    if (multiplier < 0)
        message("weird!");
    else
        face[k].color = get_face_color(face_color[k], multiplier);
}


void world_update()
{
    // clear screen
    // first get vertices
    for (int i=1; i<=numv; ++i)
        get_coordinates(i);

    // then compute which faces are visible:
    for (int k=1; k<=numf; ++k)
        compute_face(k);
    
    // sort the draw orders by y appearing first!
    sort_faces_y();
    matrix_changed = 1;
}

void graph_frame() 
{
    //message("ordering:\n");
    //for (int j=1; j<=MAX_BOXES; ++j)
    //    message(" box %d\n", y_draw_order[j]);
    face[0].next = 0; // set head of list
    y_draw_index = 1;
    while (y_draw_index <= DRAW_COUNT && FACE_BOTTOM(y_draw_order[y_draw_index]) <= 0)
        ++y_draw_index;
}

static inline void insert_face(uint8_t current)
{
    /*
    add face[current] to singly linked list

    assume the whole list is sorted correctly already, just fill in current,
    TODO based on Z.  
    */
    face[current].next = face[0].next;
    face[0].next = current;
}

void graph_line() 
{
    memset(draw_buffer, 0, 2*SCREEN_W);
    // add any new faces to the board, sort left to right
    while (y_draw_index <= DRAW_COUNT && FACE_TOP(y_draw_order[y_draw_index]) <= vga_line)
        insert_face(y_draw_order[y_draw_index++]);

    // remove any dead faces, draw the 
    uint8_t previous = 0;
    uint8_t current = 0;
    while ((current = face[current].next))
    {
        if (FACE_BOTTOM(current) <= vga_line)
        {
            // remove this face from being actively drawn
            face[previous].next = face[current].next;
            // don't update previous.
            continue;
        }
        // draw current face
        int32_t y1 = FACE_TOP(current);
        int32_t x1 = FACE_TOP_X(current);
        int32_t y2 = FACE_MIDDLE_Y(current);
        int32_t x2 = FACE_MIDDLE_X(current);
        int32_t y3 = FACE_BOTTOM(current);
        int32_t x3 = FACE_BOTTOM_X(current);
        int32_t x13 = x3 + (int)( (float)(x1-x3)*(y3-vga_line)/(y3-y1) );
        int32_t xother;
        if (vga_line < y2)
            xother = x2 + (int)( (float)(x1-x2)*(y2-vga_line)/(y2-y1) ); // x12
        else
            xother = x3 + (int)( (float)(x2-x3)*(y3-vga_line)/(y3-y2) ); // x23
        previous = current;
        int32_t xmin, xmax;
        if (xother < x13)
        {
            xmin = xother;
            xmax = x13;
        }
        else
        {
            xmin = x13;
            xmax = xother;
        }
        if (xmax < 0 || xmin >= SCREEN_W)
            continue;
        if (xmin < 0)
            xmin = 0;
        if (xmax > SCREEN_W)
            xmax = SCREEN_W;
        uint16_t color = face[current].color;
        if (current == 54)
        for (int x=xmin; x<xmax; ++x)
            draw_buffer[x] = (x%2) ? color : 65535;
        else
        for (int x=xmin; x<xmax; ++x)
            draw_buffer[x] = color;
    }

}
