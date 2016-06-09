#include "bitbox.h"
#include "common.h"
#include "tiles.h"
#include "save.h"
#include "io.h"

#include "fatfs/ff.h"
#include <string.h> // strlen

int io_mounted = 0;
FATFS fat_fs;
FIL fat_file;
FRESULT fat_result;
char base_filename[9] CCM_MEMORY; // up to 8 characters, plus a zero
char old_base_filename[9] CCM_MEMORY;

FileError io_init()
{
    old_base_filename[0] = 0;
    fat_result = f_mount(&fat_fs, "", 1); // mount now...
    if (fat_result != FR_OK)
    {
        io_mounted = 0;
        return MountError;
    }
    else
    {
        io_mounted = 1;
        return NoError;
    }
}

FileError io_set_extension(char *filename, const char *ext)
{
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }
    io_set_recent_filename();

    int k = 0;
    for (; k<8 && base_filename[k]; ++k)
    { 
        filename[k] = base_filename[k];
    }
    --k;
    filename[++k] = '.';
    filename[++k] = *ext++;
    filename[++k] = *ext++;
    filename[++k] = *ext;
    filename[++k] = 0;
    return NoError;
}

FileError io_open_or_zero_file(const char *fname, unsigned int size)
{
    if (size == 0)
        return NoDataError;

    fat_result = f_open(&fat_file, fname, FA_WRITE | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
    {
        fat_result = f_open(&fat_file, fname, FA_WRITE | FA_OPEN_ALWAYS);
        if (fat_result != FR_OK)
            return OpenError;
        uint8_t zero[128] = {0};
        while (size) 
        {
            int write_size;
            if (size <= 128)
            {
                write_size = size;
                size = 0;
            }
            else
            {
                write_size = 128;
                size -= 128;
            }
            UINT bytes_get;
            f_write(&fat_file, zero, write_size, &bytes_get);
            if (bytes_get != write_size)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
    }
    return NoError;
}

FileError io_set_recent_filename()
{
    int filename_len = strlen(base_filename);
    if (filename_len == 0)
        return ConstraintError;

    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }
    
    if (strcmp(old_base_filename, base_filename) == 0)
        return NoError; // don't rewrite  

    fat_result = f_open(&fat_file, "RECENT16.TXT", FA_WRITE | FA_CREATE_ALWAYS); 
    if (fat_result != FR_OK) 
        return OpenError;
    UINT bytes_get;
    fat_result = f_write(&fat_file, base_filename, filename_len, &bytes_get);
    f_close(&fat_file);
    if (fat_result != FR_OK)
        return WriteError;
    if (bytes_get != filename_len)
        return MissingDataError;
    strcpy(old_base_filename, base_filename);
    return NoError;
}

FileError io_get_recent_filename()
{
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

    fat_result = f_open(&fat_file, "RECENT16.TXT", FA_READ | FA_OPEN_EXISTING); 
    if (fat_result != FR_OK) 
        return OpenError;

    UINT bytes_get;
    fat_result = f_read(&fat_file, base_filename, 8, &bytes_get); 
    f_close(&fat_file);
    if (fat_result != FR_OK)
        return ReadError;

    base_filename[bytes_get] = 0;
    if (bytes_get == 0)
        return NoDataError;
    return NoError;
}

FileError io_save_palette()
{
    char filename[13];
    if (io_set_extension(filename, "P16"))
        return MountError;

    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return OpenError;

    UINT bytes_get; 
    fat_result = f_write(&fat_file, palette, sizeof(palette), &bytes_get);
    f_close(&fat_file);
    if (fat_result != FR_OK)
        return WriteError;

    if (bytes_get != sizeof(palette))
        return MissingDataError;
    return NoError;
}

FileError io_load_palette()
{
    char filename[13];
    if (io_set_extension(filename, "P16"))
        return MountError;
    
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
        return OpenError;

    UINT bytes_get; 
    fat_result = f_read(&fat_file, palette, sizeof(palette), &bytes_get);
    f_close(&fat_file);
    if (fat_result != FR_OK)
        return ReadError;

    if (bytes_get != sizeof(palette))
        return MissingDataError;
    return NoError;
}

FileError io_save_tile(unsigned int i)
{
    char filename[13];
    if (io_set_extension(filename, "T16"))
        return MountError;
   
    if (i >= 16)
    {
        // write them all
        fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
        if (fat_result != FR_OK)
            return OpenError;

        for (i=0; i<16; ++i)
        {
            UINT bytes_get; 
            fat_result = f_write(&fat_file, &tile_info[i], 4, &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return WriteError;
            }
            if (bytes_get != 4)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
            fat_result = f_write(&fat_file, tile_draw[i], sizeof(tile_draw[0]), &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return WriteError;
            }
            if (bytes_get != sizeof(tile_draw[0]))
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
        f_close(&fat_file);
        return NoError;
    }
    else
    {
        FileError ferr = io_open_or_zero_file(filename, 16*(sizeof(tile_draw[0])+4));
        if (ferr)
            return ferr;

        UINT bytes_get; 
        f_lseek(&fat_file, i*(sizeof(tile_draw[0])+4)); 
        fat_result = f_write(&fat_file, &tile_info[i], 4, &bytes_get);
        if (fat_result != FR_OK)
        {
            f_close(&fat_file);
            return WriteError;
        }
        if (bytes_get != 4)
        {
            f_close(&fat_file);
            return MissingDataError;
        }
        fat_result = f_write(&fat_file, tile_draw[i], sizeof(tile_draw[0]), &bytes_get);
        f_close(&fat_file);
        if (fat_result != FR_OK)
            return WriteError;
        if (bytes_get != sizeof(tile_draw[0]))
            return MissingDataError;
        return NoError;
    }
}

FileError io_load_tile(unsigned int i) 
{
    char filename[13];
    if (io_set_extension(filename, "T16"))
        return MountError;
    
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
        return OpenError;
    
    if (i >= 16)
    {
        // read them all
        for (i=0; i<16; ++i)
        {
            UINT bytes_get;
            fat_result = f_read(&fat_file, &tile_info[i], 4, &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return ReadError; 
            }
            else if (bytes_get != 4)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
            fat_result = f_read(&fat_file, tile_draw[i], sizeof(tile_draw[0]), &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return ReadError;
            }
            if (bytes_get != sizeof(tile_draw[0]))
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
        f_close(&fat_file);
        return NoError;
    }
    else
    {
        f_lseek(&fat_file, i*(sizeof(tile_draw[0])+4)); 
        UINT bytes_get;
        fat_result = f_read(&fat_file, &tile_info[i], 4, &bytes_get);
        if (fat_result != FR_OK)
        {
            f_close(&fat_file);
            return ReadError; 
        }
        else if (bytes_get != 4)
        {
            f_close(&fat_file);
            return MissingDataError;
        }
        fat_result = f_read(&fat_file, tile_draw[i], sizeof(tile_draw[0]), &bytes_get);
        f_close(&fat_file);
        if (fat_result != FR_OK)
            return ReadError;
        if (bytes_get != sizeof(tile_draw[0]))
            return MissingDataError;
        return NoError;
    }
}

FileError io_save_sprite(unsigned int i)
{
    char filename[13];
    if (io_set_extension(filename, "S16"))
        return MountError;
    
    if (i >= 16)
    {
        // write them all
        fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
        if (fat_result != FR_OK)
            return OpenError; 

        for (i=0; i<16; ++i)
        for (int f=0; f<8; ++f)
        {
            UINT bytes_get;
            fat_result = f_write(&fat_file, &sprite_info[i][f], 4, &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return WriteError; 
            }
            else if (bytes_get != 4)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
            fat_result = f_write(&fat_file, sprite_draw[i][f], sizeof(sprite_draw[0][0]), &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return WriteError;
            }
            if (bytes_get != sizeof(sprite_draw[0][0]))
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
        f_close(&fat_file);
        return NoError;
    }
    else
    {
        FileError ferr = io_open_or_zero_file(filename, 16*8*(sizeof(sprite_draw[0][0])+4));
        if (ferr)
            return ferr;

        f_lseek(&fat_file, i*8*(sizeof(sprite_draw[0][0])+4)); 
        for (int f=0; f<8; ++f)
        {
            UINT bytes_get;
            fat_result = f_write(&fat_file, &sprite_info[i][f], 4, &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return WriteError; 
            }
            else if (bytes_get != 4)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
            fat_result = f_write(&fat_file, sprite_draw[i][f], sizeof(sprite_draw[0][0]), &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return WriteError;
            }
            if (bytes_get != sizeof(sprite_draw[0][0]))
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
        f_close(&fat_file);
        return NoError;
    }
}

FileError io_load_sprite(unsigned int i) 
{
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }
    io_set_recent_filename();

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        --k;
        filename[++k] = '.';
        filename[++k] = 'S';
        filename[++k] = '1';
        filename[++k] = '6';
        filename[++k] = 0;
    }
        
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
        return OpenError;
    
    if (i >= 16)
    {
        // read them all
        for (i=0; i<16; ++i)
        for (int f=0; f<8; ++f)
        {
            UINT bytes_get;
            fat_result = f_read(&fat_file, &sprite_info[i][f], 4, &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return ReadError; 
            }
            else if (bytes_get != 4)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
            fat_result = f_read(&fat_file, sprite_draw[i][f], sizeof(sprite_draw[0][0]), &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return ReadError;
            }
            if (bytes_get != sizeof(sprite_draw[0][0]))
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
        f_close(&fat_file);
        return NoError;
    }
    else
    {
        f_lseek(&fat_file, i*8*(sizeof(sprite_draw[0][0])+4));
        for (int f=0; f<8; ++f) // frame
        {
            UINT bytes_get;
            fat_result = f_read(&fat_file, &sprite_info[i][f], 4, &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return ReadError; 
            }
            else if (bytes_get != 4)
            {
                f_close(&fat_file);
                return MissingDataError;
            }
            fat_result = f_read(&fat_file, sprite_draw[i][f], sizeof(sprite_draw[0][0]), &bytes_get);
            if (fat_result != FR_OK)
            {
                f_close(&fat_file);
                return ReadError;
            }
            if (bytes_get != sizeof(sprite_draw[0][0]))
            {
                f_close(&fat_file);
                return MissingDataError;
            }
        }
        f_close(&fat_file);
        return NoError;
    }
}

FileError io_save_map()
{
    if (tile_map_height <= 0 || tile_map_width <= 0 || 
        (tile_map_width*tile_map_height > TILE_MAP_MEMORY))
        return ConstraintError;

    char filename[13];
    if (io_set_extension(filename, "M16"))
        return MountError;
    
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return OpenError;

    UINT bytes_get; 
    // write width and height
    fat_result = f_write(&fat_file, &tile_map_width, 2, &bytes_get);
    if (fat_result != FR_OK)
    {
        f_close(&fat_file);
        return WriteError;
    }
    if (bytes_get != 2)
    {
        f_close(&fat_file);
        return MissingDataError;
    }
    fat_result = f_write(&fat_file, &tile_map_height, 2, &bytes_get);
    if (fat_result != FR_OK)
    {
        f_close(&fat_file);
        return WriteError;
    }
    if (bytes_get != 2)
    {
        f_close(&fat_file);
        return MissingDataError;
    }

    // write the rest of stuff 
    int size = tile_map_width * tile_map_height;
    uint8_t *src = tile_map;
    while (size) 
    {
        int write_size;
        if (size <= 128)
        {
            write_size = size;
            size = 0;
        }
        else
        {
            write_size = 128;
            size -= 128;
        }
        UINT bytes_get;
        f_write(&fat_file, src, write_size, &bytes_get);
        if (bytes_get != write_size)
        {
            f_close(&fat_file);
            return MissingDataError;
        }
        src += write_size;
    } 
    
    f_close(&fat_file);
    return NoError;
}

FileError io_load_map()
{
    char filename[13];
    if (io_set_extension(filename, "M16"))
        return MountError;
    
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
        return OpenError;

    UINT bytes_get; 
    fat_result = f_read(&fat_file, &tile_map_width, 2, &bytes_get);
    if (fat_result != FR_OK)
    {
        f_close(&fat_file);
        return ReadError;
    }
    if (bytes_get != 2)
    {
        f_close(&fat_file);
        return MissingDataError;
    }
    if (tile_map_width <= 0)
        return ConstraintError;
    fat_result = f_read(&fat_file, &tile_map_height, 2, &bytes_get);
    if (fat_result != FR_OK)
    {
        f_close(&fat_file);
        return ReadError;
    }
    if (bytes_get != 2)
    {
        f_close(&fat_file);
        return MissingDataError;
    }
    if (tile_map_height <= 0)
        return ConstraintError;
    
    int size = tile_map_width * tile_map_height;
    if (size > TILE_MAP_MEMORY)
        return ConstraintError;
    uint8_t *src = tile_map;
    while (size) 
    {
        int read_size;
        if (size <= 128)
        {
            read_size = size;
            size = 0;
        }
        else
        {
            read_size = 128;
            size -= 128;
        }
        UINT bytes_get;
        f_read(&fat_file, src, read_size, &bytes_get);
        if (bytes_get != read_size)
        {
            f_close(&fat_file);
            return MissingDataError;
        }
        src += read_size;
    } 
    
    f_close(&fat_file);
    return NoError;
}
