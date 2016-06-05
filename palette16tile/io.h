#ifndef IO_H
#define IO_H

int io_init();
int io_save_palette();
int io_load_palette();
int io_save_tile(unsigned int i);
int io_load_tile(unsigned int i);

#endif
