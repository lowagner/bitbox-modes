#ifndef SAVE_H
#define SAVE_H
extern char base_filename[9];
extern uint8_t save_not_load; // whether to be in save (1) or load (0)
void save_line();
void save_controls();
#endif
