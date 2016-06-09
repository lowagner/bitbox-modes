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

FileError io_init()
{
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

FileError io_record_recent_filename()
{
    int filename_len = strlen(base_filename);
    if (filename_len == 0)
        return ConstraintError;

    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

    fat_result = f_open(&fat_file, "RECENT16.TXT", FA_READ | FA_WRITE | FA_OPEN_ALWAYS); 
    if (fat_result != FR_OK) 
        return OpenError;
    UINT bytes_get;
    fat_result = f_write(&fat_file, base_filename, filename_len, &bytes_get);
    f_close(&fat_file);
    if (fat_result != FR_OK)
        return WriteError;
    if (bytes_get != filename_len)
        return MissingDataError;
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
    fat_result = f_read(&fat_file, &base_filename, 8, &bytes_get); 
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
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        --k;
        filename[++k] = '.';
        filename[++k] = 'P';
        filename[++k] = '1';
        filename[++k] = '6';
        filename[++k] = 0;
    }
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
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        --k;
        filename[++k] = '.';
        filename[++k] = 'P';
        filename[++k] = '1';
        filename[++k] = '6';
        filename[++k] = 0;
    }
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
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        --k;
        filename[++k] = '.';
        filename[++k] = 'T';
        filename[++k] = '1';
        filename[++k] = '6';
        filename[++k] = 0;
    }
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return OpenError;
    
    if (i >= 16)
    {
        // write them all
        for (i=0; i<16; ++i)
        {
            UINT bytes_get; 
            fat_result = f_write(&fat_file, &tile_info[i], 4, &bytes_get);
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
        UINT bytes_get; 
        f_lseek(&fat_file, i*(sizeof(tile_draw[0])+4)); 
        fat_result = f_write(&fat_file, &tile_info[i], 4, &bytes_get);
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
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        --k;
        filename[++k] = '.';
        filename[++k] = 'T';
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
    if (io_mounted == 0)
    {
        if (io_init())
            return MountError;
    }

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
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return OpenError;
    
    if (i >= 16)
    {
        // write them all
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
    {
        return OpenError;
    }
    
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
    return WriteError; 
}

FileError io_load_map()
{
    return ConstraintError; 
}
