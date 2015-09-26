#include "wview3d.h"
#include <string.h> // memset
#include <stdlib.h> // abs
#include <stdint.h>


#define MAX_FIRST_SWAP 10 // number of swaps to allow in start_insertion_sort
#define MAX_EFFORT 2 // effort per line for calculating edges, until the final line.
/* 
    put all the edges in a fixed-size array, and don't move them.

    order a list of starting points

       1        2    
        \       |    3
         \      |   /  4 ___ 
          \     |  /
           \
            \

    e[i] = i'th edge, ordered in no particular way
    edge_start[i] = index of edge which is the i'th to start
                    (break ties arbitrarily)

    create a singly linked list which indicates the active edges,
    iterate through those during the drawing.  
    if any edge becomes unactive, remove it from the linked list.
    (keep track of previous edge in the linked list, so we know how to stitch it back up)
    do not add a horizontal line into the active edge list, just draw it that iteration.
   
    e[i].{p1, p2}:  {top,bottom} vertex in e[i], i.e. p1.y < p2.y

*/

void free_recent_edges()
{
    LINT ill = first_active_index;
    while (ill < 255)
    {
        ledger[ill].next_free_index = first_free_index;
        first_free_index = ill;
        
        ill = ledger[ill].next_active_index;
    }
    first_active_index = -1;
}

void init_drawing_edges()
{
    // free everything, and setup the links in the free indices
    for (int i=0; i<MAX_EDGES; ++i)
    {
        ledger[i] = (Ledger) {
            .active_edge = NULL,
            .next_active_index = -1,
            .next_free_index = i+1
        };
    }
    ledger[MAX_EDGES-1].next_free_index = -1;
    first_active_index = -1;
    first_free_index = 0;
}

#ifdef DEBUG
void print_list()
{
    #ifdef EMULATOR
    LINT ill = first_active_index;
    LINT counter = 0;
    while (ill < 255 && counter < 255)
    {
        edge *ei = ledger[ill].active_edge;
        message("ledger[%d].active_edge = (%d, %d, %f) to (%d, %d, %f), ", (int)ill, 
            ei->p1.image[0], ei->p1.image[1], ei->p1.image_z,
            ei->p2.image[0], ei->p2.image[1], ei->p2.image_z
        );
        ill = ledger[ill].next_active_index;
        ++counter;
    }
    ill = first_free_index;
    message("\nfree indices: ");
    counter = 0;
    while (ill < 255 && counter < 255)
    {
        message("%d, ", (int)ill);
        ill = ledger[ill].next_free_index;
        ++counter;
    }
    #endif
}
#endif

LINT sie; // sorted index

void start_insertion_sort_edges()
{
    // sort by y (lower y is closer to top of screen).
    LINT swaps = 0;
    sie = 1;
    while (sie<nume/2)
    {
        for (int k=sie; k>0; --k)
        {
            if (se[k]->p1.image[1] < se[k-1]->p1.image[1])  // y[k] < y[k-1]
            {
                edge *sek = se[k];
                se[k] = se[k-1];
                se[k-1] = sek;
                ++swaps;
            }
            else
                break;
        }
        ++sie;
        if (swaps > MAX_FIRST_SWAP)
            break;
    }
    #ifdef DEBUG
    if (swaps)
        message("first sorted up to %d using %d swaps\n", sie, swaps);
    #endif
}

void finish_insertion_sort_edges()
{
    // sort by y (lower y is closer to top of screen).
    for (; sie<nume; ++sie)
    for (int k=sie; k>0; --k)
    {
        if (se[k]->p1.image[1] < se[k-1]->p1.image[1])  // y[k] < y[k-1]
        {
            edge *sek = se[k];
            se[k] = se[k-1];
            se[k-1] = sek;
        }
        else
            break;
    } 
    #ifdef DEBUG
    message("finished sorting\n");
    #endif
}

void heap_sort_demote_edges(int i0, int n)
{
    // in max heap, we want H[i] > H[2*i] and H[2*i+1] for all indices <= n.
    // for us, we use H[i] = e[i-1].p1.image[1].
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
            if (se[lc-1]->p1.image[1] > se[rc-1]->p1.image[1])
                mc = lc; // left child is larger than right child (y values)
            else
                mc = rc;
        }
        
        if (se[i-1]->p1.image[1] >= se[mc-1]->p1.image[1]) // H[i] is greater than its max child (H[2*i] or H[2*i+1])
            return; // we are finished, we do not need to demote any further.
        // swap:
        edge *seim1 = se[i-1];
        se[i-1] = se[mc-1];
        se[mc-1] = seim1;
        // and we need to see if the guy we demoted needs to go even further down:
        i = mc;
        lc = 2*mc;
    }
}

void heap_sort_edges()
{
    // first get the pointer indices working:
    for (int i=0; i<nume; ++i)
        se[i] = &e[i];

    // max heapify:
    for (int i=nume/2; i>=1; --i)
        heap_sort_demote_edges(i, nume);

    for (int i=1; i<=nume; ++i)
    {
        // the guy at v[0] (H[1]) is the largest in the array from 0 to n-i, inclusive,
        // so put it at the end of the array:
        edge *se0 = se[0];
        se[0] = se[nume-i];
        se[nume-i] = se0;
        // maintain the heap property but not all the way to n:
        heap_sort_demote_edges(1, nume-i);
    }
}

void graph_frame() 
{
    #ifdef EMULATOR
    if (debug_draw>0)
        --debug_draw;
    #endif

    #ifdef DEBUG
    message("\n\nbegin/end frame\n");
    message("all edges:\n");
    for (int ill=0; ill<nume; ++ill)
    {
        edge *ei = se[ill];
        message("  edge p1.(x,y,z) = (%f, %f, %f), .(i, j) = (%d, %d)\n", ei->p1.world[0], ei->p1.world[1],ei->p1.world[2], ei->p1.image[0], ei->p1.image[1]);
        message("       p2.(x,y,z) = (%f, %f, %f), .(i, j) = (%d, %d)\n", ei->p2.world[0], ei->p2.world[1],ei->p2.world[2], ei->p2.image[0], ei->p2.image[1]);
    }
    message("\n");
    print_list();
    message("\n\n");
    #endif
}

void get_more_edges()
{
    for (int effort=0; sie<nume && effort < MAX_EFFORT; ++sie, ++effort)
        get_coordinates(sie);

    #ifdef DEBUG
    message("got up to sie=%d\n", sie);
    #endif
}

void get_last_edges()
{
    for (; sie<nume; ++sie)
        get_coordinates(sie);

    #ifdef DEBUG
    message("finally finished\n");
    #endif
}

void get_all_coordinates()
{
    for (int i=0; i<nume; ++i)
    {
        get_coordinates(i);
        #ifdef DEBUG
        message("e[%02d].p1.world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, e[i].p1.world[0], e[i].p1.world[1], e[i].p1.world[2], e[i].p1.image[0], e[i].p1.image[1], e[i].p1.image_z);
        message("     .p2.world=(%f, %f, %f), .image=(%d, %d, %f)\n", e[i].p2.world[0], e[i].p2.world[1], e[i].p2.world[2], e[i].p2.image[0], e[i].p2.image[1], e[i].p2.image_z);
        #endif
    }
}


LINT ise; // index of the edge in se (the sorted edges) which is next to be drawn 

void graph_line() 
{
uint16_t *draw_location = draw_buffer;
// in the following, we distribute the workload over a few lines,
// in order to avoid doing too many computations on one line.  if you
// modify the following, and the bitbox doesn't boot the game, try to
// move your computations around.
switch (vga_line)
{
  case 0:
    ise = 0;
    sie = 0;
    free_recent_edges();
    get_view(&camera);
    memset(draw_buffer, 136, SCREEN_W*2); // happens to be a pretty blue
    return;
  case 1:
    memset(draw_buffer, 136, SCREEN_W*2);
    // no return/break, continue
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12: 
    get_more_edges();
    return;
  case 13:
    get_last_edges();
    return;
  case 14:
    start_insertion_sort_edges();
    return;
  case 15:
    finish_insertion_sort_edges();
    return;
  case 464:
  case 465:
    memset(draw_buffer, 136, SCREEN_W*2);
    return;
  case 466:
  case 467:
  case 468:
  case 469:
  case 470:
  case 471:
  case 472:
  case 473:
  case 474:
  case 475:
  case 476:
  case 477:
  case 478:
  case 479:
    // could do some calculations here at the end
    break;
  default:
    memset(draw_buffer, 0, SCREEN_W*2);
    
    int this_line = (int) vga_line; // necessary since vga_line is uint but not huge.
    edge *ei;
    while ( ise < nume && (ei=se[ise], ei->p1.image[1] <= this_line) )
    {
        // ei is the next edge which should be drawn to screen
        // check if it is a horizontal line
        if (ei->p1.image[1] == this_line && ei->p2.image[1] == this_line)
        {
            // horizontal line, just draw it here    
            int xL, xR;
            if (ei->p1.image[0] < ei->p2.image[0]) // x1 < x2
            {
                xL = ei->p1.image[0];
                xR = ei->p2.image[0];
            }
            else
            {
                xL = ei->p2.image[0];
                xR = ei->p1.image[0];
            }
            if (xR < 0 || xL >= SCREEN_W)
            {}
            else
            {
                if (xL < 0)
                    xL = 0;
                if (xR >= SCREEN_W)
                    xR = SCREEN_W-1;

                draw_location = draw_buffer + xL;
                for (;xL<=xR; xL++)
                {
                    *draw_location++ = ei->color;
                }
            }
        } // otherwise (not a horizontal line) 
        else if (ei->p2.image[1] >= this_line)
        {
            // initialize the drawing pixel:
            if (ei->p1.image[1] == this_line)
            {
                ei->draw_x = ei->p1.image[0];
            }
            else // line started before this line (off screen, probably)
            {
                float fraction1 = 1.0*(ei->p2.image[1] - this_line)/
                                        (ei->p2.image[1] - ei->p1.image[1]);
                ei->draw_x = round(fraction1*ei->p1.image[0] + (1-fraction1)*ei->p2.image[0]);
            }
            // also check if it will appear offscreen left or right, and ignore if so.
            // only lines which might appear on screen should be added to the active edges list

            int dx; // abs(e[ie].draw_x - e[ie].p2.image[0]);
            int dy = abs(this_line - ei->p2.image[1]);
           
            int ok = 1;
            if (ei->p2.image[0] > ei->draw_x)
            {   // x2 > x1
                if (ei->draw_x >= SCREEN_W || // x1 > screen width
                    ei->p2.image[0] < 0) // x2 < 0
                    ok = 0; // completely off screen
                else
                {
                    dx = ei->p2.image[0] - ei->draw_x;
                    ei->draw_sx = 1;
                }
            }
            else // x2 <= x1
            {
                if (ei->draw_x < 0 || // x1 < 0
                    ei->p2.image[0] >= SCREEN_W) // x2 > screen width
                    ok = 0; // completely off screen
                else
                {
                    dx = ei->draw_x - ei->p2.image[0];
                    ei->draw_sx = -1;
                }
            }
            
            if (ok)
            {
                // insert this edge into the linked list
                // insert into the first free spot, with everything else after it:
                ledger[first_free_index].active_edge = ei;
                ledger[first_free_index].next_active_index = first_active_index;
                // update the first active index:
                first_active_index = first_free_index;
                // move the free index up here:
                first_free_index = ledger[first_free_index].next_free_index;
                #ifdef DEBUG
                print_list();
                #endif
                // setup the rest of the drawing variables
                ei->draw_dx = dx;
                ei->draw_dy = dy;
                ei->draw_error = (dx>dy ? dx : -dy)/2;
            }
        }
        ++ise;
    }

    // draw edges in the list
    LINT il_previous = -1; // previous index in the linked list
    LINT il = first_active_index; // current index in the linked list
    while (il < 255) // 255 is the NULL index
    {
        ei = ledger[il].active_edge; // recall the singly linked list records e's indices
        // check if we are done with the edge, and move the current/previous_edge pointers:
        if (ei->p2.image[1] <= this_line)
        {
            // if it has ended, then remove current_edge from the list
            // this is why we need il_previous, to stitch things up in the linked list.
            // remove current_index from the active_edges, but add it to the free_edges
            LINT next_il = ledger[il].next_active_index;
            if (il == first_active_index)
            {   // we're at the head of the list
                first_active_index = next_il; // reset the head
            }
            else // current_index is not the root
            {
                // stitch up previous next to the current next:
                ledger[il_previous].next_active_index = next_il;
            }
            // and insert il at the head of the free edges:
            ledger[il].next_free_index = first_free_index;
            first_free_index = il;
            // update the current index
            il = next_il;
        }
        else
        {
            // move things up
            il_previous = il;
            il = ledger[il].next_active_index;
        }

        // proceed with drawing the edge e[ie]:
        if (ei->p2.image[0] == ei->draw_x) // we have achieved the final x coordinate
        {
            if (ei->draw_x >= 0 && ei->draw_x < SCREEN_W)
            {
                draw_buffer[ei->draw_x] = ei->color;
            } 
        }
        else // x2 > x1 or x1 < x2
        {
            // see code in bitbox/lib/simple.c for draw_line for the algorithm:
            /*
            while (1) 
            {
                if (ei->draw_x >= 0 && ei->draw_x < SCREEN_W)
                {
                    draw_buffer[ei->draw_x] = RGB(0xff,0xff,0xff);
                }
                if (ei->draw_x*ei->draw_sx >= ei->p2.image[0]*ei->draw_sx) //   x * sx >= ix * sx  -- ok for sx = 1
                    // finished drawing this edge, break // -x >= -ix   (sx=-1) -> x <= ix
                    break;
                int pre_error = ei->draw_error;
                if (pre_error > -ei->draw_dx) 
                { 
                    ei->draw_error -= ei->draw_dy; 
                    ei->draw_x += ei->draw_sx; 
                }
                if (pre_error < ei->draw_dy) 
                { 
                    ei->draw_error += ei->draw_dx; 
                    // wait til next line to continue drawing
                    break;
                }
            }
            */
            // new algorithm does all blitting in one line at one time, to avoid
            // constantly checking each point.
            
            if (ei->draw_error > -ei->draw_dx)
            {
                int xleft, xright;
                int moveover = (ei->draw_error - ei->draw_dy)/ei->draw_dy + 1;
                ei->draw_error -= ei->draw_dy * moveover;
                if (ei->draw_sx == 1)
                {    // moving right
                    xleft = ei->draw_x;
                    if (ei->draw_error > -ei->draw_dx) 
                    {
                        xright = xleft + moveover;
                        ei->draw_x = xright + 1;
                        ei->draw_error -= ei->draw_dy;
                    }
                    else
                    {
                        xright = xleft + moveover + 1;
                        ei->draw_x = xright;
                    }
                    if (xright > ei->p2.image[0])
                        xright = ei->p2.image[0];
                }
                else // moving left
                {
                    xright = ei->draw_x;
                    if (ei->draw_error > -ei->draw_dx) 
                    {
                        xleft = xright - moveover;
                        ei->draw_x = xleft - 1;
                        ei->draw_error -= ei->draw_dy;
                    }
                    else
                    {
                        xleft = xright - moveover - 1;
                        ei->draw_x = xleft;
                    }
                    if (xleft < ei->p2.image[0])
                        xleft = ei->p2.image[0];
                }
                // for going down in y
                ei->draw_error += ei->draw_dx;

                if (xright < 0 || xleft >= SCREEN_W)
                {}
                else
                {
                    if (xleft < 0)
                        xleft = 0;
                    if (xright >= SCREEN_W)
                        xright = SCREEN_W-1;

                    for(uint16_t *src = &draw_buffer[xleft]; src<=&draw_buffer[xright]; src++)
                        *src = ei->color;
                }

            }
            else // draw just one point
            {
                if (ei->draw_x >= 0 && ei->draw_x < SCREEN_W)
                    draw_buffer[ei->draw_x] = ei->color;

                if (ei->draw_error < ei->draw_dy) 
                    ei->draw_error += ei->draw_dx; 
            }
        }
    }
}
}
