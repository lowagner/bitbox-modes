#ifndef IO_H
#define IO_H

int io_init();
int io_save_palette(char count[]);
int io_load_palette(char count[]);
int io_save_tile(char count[], unsigned int i);
int io_load_tile(char count[], unsigned int i);

#endif
