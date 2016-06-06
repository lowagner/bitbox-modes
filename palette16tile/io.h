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
    MissingDataError
} FileError;


FileError io_init();
FileError io_get_recent_filename();
FileError io_save_palette();
FileError io_load_palette();
FileError io_save_tile(unsigned int i);
FileError io_load_tile(unsigned int i);

#endif
