#include "world.h"
#include <stdlib.h> // rand
#include <math.h> // sin and cos

#define SCREEN_W 320
#define SCREEN_H 240

struct vertex vertex[256] CCM_MEMORY; // array of vertices
struct edge edge[256] CCM_MEMORY;
struct face face[256] CCM_MEMORY;

int numv CCM_MEMORY; // number of vertices
int nume CCM_MEMORY; // number of edges
int numf CCM_MEMORY; // number of faces

uint8_t y_draw_order[256] CCM_MEMORY; 
#define DRAW_COUNT y_draw_order[0]
int y_draw_index CCM_MEMORY;

#define FACE_TOP(k) vertex[face[k].v[face[k].vertex_order&3]].iy
#define FACE_BOTTOM(k) vertex[face[k].v[(face[k].vertex_order>>4)&3]].iy

#define FACE_TOP_X(k) vertex[face[k].v[face[k].vertex_order&3]].ix
#define FACE_BOTTOM_X(k) vertex[face[k].v[(face[k].vertex_order>>4)&3]].ix

#define FACE_MIDDLE_X(k) vertex[face[k].v[(face[k].vertex_order>>2)&3]].ix
#define FACE_MIDDLE_Y(k) vertex[face[k].v[(face[k].vertex_order>>2)&3]].iy

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


static inline uint8_t connect_vertex_to_edge(uint8_t vi, uint8_t ei)
{
    struct vertex *vv = &vertex[vi];
    int i=0;
    while (i < CONNECTED && vv->edge[i])
        ++i;
    if (i < CONNECTED)
        vv->edge[i] = ei;
    else
    {
        message("too many vertices nearby, please increase CONNECTED (bb3d.h)\n");
        return 1;
    }
    return 0;
}

static inline uint8_t find_common_edge(uint8_t vi, uint8_t vj)
{
    struct vertex *v1 = &vertex[vi];
    struct vertex *v2 = &vertex[vj];
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
    return 0;
}

static inline void swap_y_draw_order_j_jplus1(int j)
{
    int face_jplus1 = y_draw_order[j+1];
    int face_j = y_draw_order[j];

    face[face_jplus1].draw_order = j;
    face[face_j].draw_order = j+1;

    y_draw_order[j+1] = face_j;
    y_draw_order[j] = face_jplus1;
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
        int32_t tos_value = vertex[face[tos].v1].iy;
        for (int j=i-1; (j>0) && (tos_value < vertex[face[y_draw_order[j]].v1].iy); --j)
            swap_y_draw_order_j_jplus1(j);
    }
}

void world_init()
{
    edge_color = RGB(200,200,200);

    srand(vga_frame);

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

    nume = 0;
    numf = 0;

    // lazily determine the edges.
    for (int i=1; i<numv; ++i)
    for (int j=i+1; j<=numv; ++j)
    // vertices closer than this are connected:
    if (distance2(vertex[i].world, vertex[j].world) < 1.04f) 
    {
        //message("edge %d to %d, distance %f\n", i, j, distance2(vertex[i].world, vertex[j].world));
        edge[++nume] = (struct edge) { 
            .p1 = i, .p2 = j,
            .f1 = 0, .f2 = 0
        };
        if (connect_vertex_to_edge(i, nume))
            message("couldn't connect edge %d to vertex %d\n", nume, i);
        if (connect_vertex_to_edge(j, nume))
            message("couldn't connect edge %d to vertex %d\n", nume, j);
    }

    // lazily determine faces:
    for (int i=1; i<numv-1; ++i)
    for (int j=i+1; j<numv; ++j)
    for (int k=j+1; k<=numv; ++k)
    if ((distance2(vertex[i].world, vertex[j].world) < 1.04f) &&
        (distance2(vertex[j].world, vertex[k].world) < 1.04f) &&
        (distance2(vertex[k].world, vertex[i].world) < 1.04f))
    {
        // find the common edge in i-j, j-k, and k-i:
        uint8_t e1 = find_common_edge(i, j);
        uint8_t e2 = find_common_edge(j, k);
        uint8_t e3 = find_common_edge(k, i);
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
                .e1 = e1, .e2 = e2, .e3 = e3,
                .visible = 0, 
                .vertex_order = 0 | (1<<2) | (2<<4),
                .color = random_color() 
            };
        }
        else
        {
            face[++numf] = (struct face) { 
                .v1 = i, .v2 = k, .v3 = j,  // note swap here.
                .e1 = e1, .e2 = e2, .e3 = e3,
                .visible = 0, 
                .vertex_order = 0 | (1<<2) | (2<<4),
                .color = random_color() 
            };
        }
        // notify each edge that it has a new face:
        if (!edge[e1].f1)
            edge[e1].f1 = numf;
        else if (!edge[e1].f2)
            edge[e1].f2 = numf;
        else
            message("your edge got connected to too many faces somehow!\n");
        if (!edge[e2].f1)
            edge[e2].f1 = numf;
        else if (!edge[e2].f2)
            edge[e2].f2 = numf;
        else
            message("your edge got connected to too many faces somehow!\n");
        if (!edge[e3].f1)
            edge[e3].f1 = numf;
        else if (!edge[e3].f2)
            edge[e3].f2 = numf;
        else
            message("your edge got connected to too many faces somehow!\n");
    }

    message("got faces %d and edges %d\n", numf, nume);
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

inline void order_edge(uint8_t ej)
{
    if (vertex[edge[ej].p2].iy <= vertex[edge[ej].p1].iy)
    {
        if (vertex[edge[ej].p2].iy == vertex[edge[ej].p1].iy)
            return;
        uint8_t p2 = edge[ej].p2;
        edge[ej].p2 = edge[ej].p1;
        edge[ej].p1 = p2;
    }

    // find which vertex is not p1 or p2 in each face, to order f1 and f2
    // take cross product of edge with other vertex.
    // f1 should be "left of" f2, so cross((p2-p1), p3) for f1 should be negative
    // note that in case of y1 == y2,
    // we could have x2 > x1 for the top most face to be first, but
    // that shouldn't happen in practice
    int p3;
    if (face[edge[ej].f1].v1 != edge[ej].p1 && face[edge[ej].f1].v1 != edge[ej].p2)
        p3 = 0;
    else if (face[edge[ej].f1].v2 != edge[ej].p1 && face[edge[ej].f1].v2 != edge[ej].p2)
        p3 = 1;
    else //if (face[edge[ej].f1].v3 != edge[ej].p1 && face[edge[ej].f1].v3 != edge[ej].p2)
        p3 = 2;
    if (is_ccw(vertex[edge[ej].p1].image, vertex[edge[ej].p2].image, vertex[face[edge[ej].f1].v[p3]].image))
    {
        // switch f2 and f1
        uint8_t f2 = edge[ej].f2;
        edge[ej].f2 = edge[ej].f1;
        edge[ej].f1 = f2; 
    }
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
                y_draw_order[j] = y_draw_order[j+1];
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
            message("too many faces on screen... shouldn't be possible\n");
    }
    
    uint8_t order = face[k].vertex_order;
    int32_t v1 = vertex[face[k].v[order&3]].iy;
    int32_t v2 = vertex[face[k].v[(order>>2)&3]].iy;
    int32_t v3 = vertex[face[k].v[(order>>4)&3]].iy;

    if (v1 <= v2)
    {
        // v1 <= v2
        if (v2 <= v3)
        {
            // v1 <= v2 <= v3
            // vertex_order is good. 
        } 
        else if (v1 <= v3)
        {
            // v1 <= v3 < v2
            // need to swap order 2 and 3:
            face[k].vertex_order = (order&3) | ((order&(3<<4))>>2) | ((order&(3<<2))<<2);
        }
        else
        {
            // v3 < v1 <= v2
            face[k].vertex_order = ((order>>4)&3) | ((order&3)<<2) | ((order&(3<<2))<<2);
        }
    }
    else
    {
        // v2 < v1
        if (v1 <= v3)
        {
            // v2 < v1 <= v3
            face[k].vertex_order = ((order>>2)&3) | ((order&3)<<2) | (order&(3<<4));
        }
        else if (v2 <= v3)
        {
            // v2 <= v3 < v1
            face[k].vertex_order = ((order>>2)&3) | ((order&(3<<4))>>2) | ((order&3)<<4);
        }
        else
        {
            // v3 < v2 < v1
            face[k].vertex_order = ((order>>4)&3) | (order&(3<<2)) | ((order&3)<<4);
        }
    }
}


void world_update()
{
    // clear screen
    // first get vertices
    for (int i=1; i<=numv; ++i)
        get_coordinates(i);

    // order the edges
    for (int j=1; j<=nume; ++j)
        order_edge(j);

    // then compute which faces are visible:
    for (int k=1; k<=numf; ++k)
        compute_face(k);
    
    // sort the draw orders by y appearing first!
    sort_faces_y();
}

void graph_frame() 
{
    //message("ordering:\n");
    //for (int j=1; j<=MAX_BOXES; ++j)
    //    message(" box %d\n", y_draw_order[j]);
    face[0].next = 0; // set head of list
    y_draw_index = 1;
    while (y_draw_index <= DRAW_COUNT && FACE_BOTTOM(y_draw_order[y_draw_index]) <= 0)
    {
        ++y_draw_index;
    }
}

static inline void insert_face(uint8_t current)
{
    /*
    add face[current] to singly linked list

    assume the whole list is sorted correctly already, just fill in current.
    */
    uint8_t previous = 0;
    uint8_t next = 0;
    while ((next = face[next].next))
    {
        // check for any edge equality
        uint8_t ej = 0;
        if (face[current].e1 == face[next].e1)
            ej = face[current].e1;
        else if (face[current].e1 == face[next].e2)
            ej = face[current].e1;
        else if (face[current].e1 == face[next].e3)
            ej = face[current].e1;
        else if (face[current].e2 == face[next].e1)
            ej = face[current].e2;
        else if (face[current].e2 == face[next].e2)
            ej = face[current].e2;
        else if (face[current].e2 == face[next].e3)
            ej = face[current].e2;
        else if (face[current].e3 == face[next].e1)
            ej = face[current].e3;
        else if (face[current].e3 == face[next].e2)
            ej = face[current].e3;
        else if (face[current].e3 == face[next].e3)
            ej = face[current].e3;
        if (ej)
        {
            // common edge
            if (current != edge[ej].f1) // current is not the first face
            {
                previous = next;
                next = face[next].next;
            }
            break;
        }
        else // no common edge
        {
            // see if current is left of next
            int32_t current_minx = (vertex[face[current].v1].ix < vertex[face[current].v2].ix) ? vertex[face[current].v1].ix : vertex[face[current].v2].ix;
            current_minx = (current_minx < vertex[face[current].v3].ix) ? current_minx : vertex[face[current].v3].ix;
            
            int32_t next_minx = (vertex[face[next].v1].ix < vertex[face[next].v2].ix) ? vertex[face[next].v1].ix : vertex[face[next].v2].ix;
            next_minx = (next_minx < vertex[face[next].v3].ix) ? next_minx : vertex[face[next].v3].ix;

            if (current_minx <= next_minx)
                break;
        }
            
        previous = next;
    }
    face[previous].next = current;
    face[current].next = next;
}

void graph_line() 
{
    // add any new faces to the board, sort left to right
    while (y_draw_index <= DRAW_COUNT && FACE_TOP(y_draw_order[y_draw_index]) <= vga_line)
    {
        insert_face(y_draw_order[y_draw_index++]);
    }

    // remove any dead faces, draw the 
    uint8_t previous = 0;
    uint8_t current = 0;
    while ((current = face[current].next))
    {
        if (FACE_BOTTOM(current) <= vga_line)
        {
            // remove this face from being actively drawn
            face[previous].next = face[current].next;
            continue;
        }
        previous = current;
        // draw current face

    }

}
