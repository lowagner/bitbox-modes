#ifndef IO_H
#define IO_H

typedef enum {
    NoError = 0,
    MountError,
    ConstraintError,
    OpenError,
    ReadError,
    WriteError,
    NoDataError,
    MissingDataError,
    BotchedIt
} FileError;


FileError io_init();
FileError io_set_recent_filename();
FileError io_get_recent_filename();
FileError io_save_palette();
FileError io_load_palette();
FileError io_save_tile(unsigned int i);
FileError io_load_tile(unsigned int i);
FileError io_save_sprite(unsigned int i);
FileError io_load_sprite(unsigned int i);
FileError io_save_map();
FileError io_load_map();

extern char base_filename[9];

#endif
