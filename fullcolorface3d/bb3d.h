#ifndef BB3D_H
#define BB3D_H
#include "bitbox.h"
#include "math.h" // sqrt
#include "stdint.h" // uint8_t

struct vertex {
    union {
        struct {
            float world[3]; // coordinates in the world reference frame
        };
        struct {
            float x, y, z;
        };
    };
    union {
        struct {
            int image[2]; // coordinates for the image
            float image_z; // remaining z-coordinate
        };
        struct {
            int ix, iy; // coordinates for the image
            float iz; // remaining z-coordinate
        };
    };
};

struct face; // define after defining the edge...

struct edge {
    struct vertex *p1, *p2; // ordered by p1->image.y <= p2->image.y
    struct face *f1, *f2; // the two faces which join at this edge
    int scan_xL, scan_xR; // the non-white points just down and to the left or right of p1 
    // if p1->iy == p2->iy, then scan_xL = min(p1->ix, p2->ix) 
    //                           scan_xR = max(p1->ix, p2->ix)
};

struct face {
    struct vertex *v1, *v2, *v3; // do not move these around
    struct edge *left, *right;
    union {
        struct edge *bottom; // normal_case
        struct edge *top; // strange case
    };
    uint8_t normal_case; // 1 or 2 if case I or II.
    uint8_t visible; // = is_ccw(v1->image, v2->image, v3->image)
    uint16_t color;
};

typedef struct _Camera {
    float viewer[3]; // position of the camera
    float viewee[3]; // object which the camera is looking at
    float right[3]; // vector which points right in the camera reference frame
    float down[3]; // vector which points down in the camera reference frame
    float forward[3]; // vector which points forward in the camera reference frame
    float magnification; // multiplier to project points onto screen
    float view_matrix[12]; // camera matrix
} Camera;

inline float sqr(float x)
{
    return x*x;
}

// normalize vector_in and store it in vector_out:
inline void normalize(float *vector_out, float *vector_in)
{
    float mag = sqrt(sqr(vector_in[0]) + sqr(vector_in[1]) + sqr(vector_in[2]));
    #ifdef DEBUG
    if (mag == 0.0f)
    {
        message("divide by zero in normalization!!!\n");
        return;
    }
    #endif
    vector_out[0] = vector_in[0]/mag;
    vector_out[1] = vector_in[1]/mag;
    vector_out[2] = vector_in[2]/mag;
    //vector_out[3] = 1.0f; 
}
// TODO:consider dropping vector_in, vector_out unless there is a use-case;
//      i.e. just use put in one vector to normalize.

// dot product
inline float dot(float *vector1, float *vector2)
{
    return vector1[0]*vector2[0] + vector1[1]*vector2[1] + vector1[2]*vector2[2];
}

inline int is_ccw(int *p1, int *p2, int *p3)
{
    // calculates whether p1-p2-p3 (as a face) is ccw oriented, assuming IMAGE coordinates.
    // (p2-p1) x (p3-p2) should be positive.
    
    if ((p2[0]-p1[0])*(p3[1]-p2[1]) - (p2[1]-p1[1])*(p3[0]-p2[0]) > 0)
        return 1;
    else
        return 0;
}

// safe cross product:  vector_out can be one of vector1 or vector2, 
// at the expense of extra memory used:
void cross(float *vector_out, float *vector1, float *vector2);

// unsafe cross:  vector_out should be unique, different than vector1 or vector2:
void cross0(float *vector_out, float *vector1, float *vector2);

// safe matrix multiply:  vector_out = matrix * vector,
// vector_out could be the same as vector:
void matrix_multiply_vector(float *vector_out, float *matrix, float *vector);

// unsafe matrix multiply:  vector_out should be unique, different than vector:
void matrix_multiply_vector0(float *vector_out, float *matrix, float *vector);

// safe matrix matrix multiply; matrix_out can be one of matrix1 or matrix2:
void matrix_multiply_matrix(float *matrix_out, float *matrix1, float *matrix2);

// unsafe matrix matrix multiply; matrix_out SHOULD NOT be one of matrix1 or matrix2:
void matrix_multiply_matrix0(float *matrix_out, float *matrix1, float *matrix2);

// get a 3x4 matrix which describes a translation to the vector origin:
void get_translation_matrix(float *matrix, float *origin);

// setup the camera view:
void get_view(Camera *camera); 

#endif
