#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY 100 // 92 bytes each, so HISTORY=128 would be ~11.8 kB

struct history 
{
    struct history *next_free;  // 4
    struct history *next_active;  // 8
    uint8_t multiline; // 9
    uint16_t info;  // 11
    char text[81];    // 92
} history[HISTORY];

struct history *free_history, *oldest_history;

// do a circularly linked list.  oldest_history points to the tail of the history,
// and if you go to the next one after oldest_history, you get to the front of the list.
// (front of list is most recent, back of list is most ancient.)

void init_history()
{
    for (int i=0; i<HISTORY-1; ++i)
    {
        history[i].next_free = &history[i+1];
        history[i].text[0] = 0;
    }
    history[HISTORY-1].next_free = NULL;
    history[HISTORY-1].text[0] = 0;

    free_history = &history[0];
    oldest_history = NULL;
}

void free_oldest_history()
{
    if (oldest_history)
    {
        if (!oldest_history->multiline)
        {
            // just a single line in the oldest history
            // whether there's nothing free or something else free, put it after old history:
            oldest_history->next_free = free_history;
            // update head of free history:
            free_history = oldest_history;
            // push oldest history forward:
            oldest_history = oldest_history->next_active;
        }
        else // there are multiple lines in the oldest history.
        {   // follow the next_active lines to free all of them.
            int i = (int)oldest_history->multiline;
            oldest_history->next_free = oldest_history->next_active;
            struct history *linked = oldest_history->next_active;
            while (i > 1)
            {
                linked->next_free = linked->next_active;
                linked = linked->next_active;
                --i;
            }
            linked->next_free = free_history;
        }
    }
    else
        message("error, there was no history to free the oldest!\n");
}

void insert_history(const char *text, struct history *previous, struct history *next)
{
    struct history *h;
    if (!oldest_history)
    {   // nothing in the history so far
        oldest_history = free_history;
    }
    else if (!free_history)
    {   // history is too full!
        free_oldest_history();
    }
    h = free_history;
    free_history = h->next_free;
}

#endif

