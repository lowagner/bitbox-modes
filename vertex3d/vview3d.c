// vview3d.c: vertex view 3d
#include "vview3d.h"
#include <string.h> // memset
#include <stdint.h>

int iv; // index of the vertex which is currently being drawn (or is next).

void insertion_sort_vertices()
{
    // sort by y (lower y is closer to top of screen).
    for (int i=1; i<numv; ++i)
    for (int k=i; k>0; --k)
    {
        if (v[k].image[1] < v[k-1].image[1])  // y[k] < y[k-1]
        {
            swap(&v[k], &v[k-1]); // swap v[k] and v[k-1]
        }
        else
            break;
    } // list is sorted from v[0] to v[i] after the k loop finishes.
}

void heap_sort_demote(int i0, int n)
{
    // in max heap, we want H[i] > H[2*i] and H[2*i+1] for all indices <= n.
    // for us, we use H[i] = v[i-1].image[1].
    // here we demote H[i] as far as H[n] in case H[i] is smaller than any of its children,
    // i.e., smaller than either H[2*i] or H[2*i+1].
    int i = i0;
    int lc = 2*i; // left child
    int rc, mc; // right child, max child
    while (lc <= n)
    {
        rc = lc+1; 
        if (rc > n) // if the right child does not exist (we are at the end of the array)
            mc = lc; // then the max child is the left one, since there is no right child.
        else
        {
            if (v[lc-1].image[1] > v[rc-1].image[1])
                mc = lc; // left child is larger than right child (y values)
            else
                mc = rc;
        }
        
        if (v[i-1].image[1] >= v[mc-1].image[1]) // H[i] is greater than its max child (H[2*i] or H[2*i+1])
            return; // we are finished, we do not need to demote any further.
        // otherwise, v[i-1] is smaller than v[mc-1], so promote v[mc-1] and demote v[i-1]:
        swap(&v[i-1], &v[mc-1]); 
        // and we need to see if the guy we demoted needs to go even further down:
        i = mc;
        lc = 2*mc;
    }
}

void heap_sort_vertices()
{
    // max heapify:
    for (int i=numv/2; i>=1; --i)
        heap_sort_demote(i, numv);

    for (int i=1; i<=numv; ++i)
    {
        // the guy at v[0] (H[1]) is the largest in the array from 0 to n-i, inclusive,
        // so put it at the end of the array:
        swap(&v[0], &v[numv-i]); 
        // maintain the heap property but not all the way to n:
        heap_sort_demote(1, numv-i);
    }
}

void graph_frame() 
{
    // reset the current index of the vertex we will draw
    iv = 0;
}

void graph_line() 
{
    // race the beam:  draw the vertices if they are at the current vga_line
    memset(draw_buffer, 0, SCREEN_W*2);
    while (iv < numv && v[iv].image[1] <= (int) vga_line) // necessary since vga_line is uint
    {
        // we either need to draw v[iv] or skip it, since its vga_line is now or previous...
        if (v[iv].image[1] == vga_line && v[iv].image[0] >= 0 && v[iv].image[0] < SCREEN_W)
        {
            draw_buffer[v[iv].image[0]] = v[iv].color;
        }
        ++iv;
    } // O(1+numv/SCREEN_H) on average!
}

