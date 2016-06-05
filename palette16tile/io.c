#include "bitbox.h"
#include "nonsimple.h"
#include "tiles.h"
#include "save.h"
#include "io.h"
#include "fatfs/ff.h"

int mount_failed = 0;
FATFS fat_fs;
FIL fat_file;
FRESULT fat_result;

int io_init()
{
    fat_result = f_mount(&fat_fs, "", 1); // mount now...
    if (fat_result != FR_OK)
        mount_failed = 1;
    else
        mount_failed = 0;
    return mount_failed;
}

//int io_get_count(char count[], int increment)
//{
//    //char count[3] = { '0', '0', '0' };
//    count[0] = count[1] = count[2] = 0;
//    fat_result = f_open(&fat_file, "count16.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS); 
//    if (fat_result != FR_OK) 
//        return 1;
//    UINT bytes_read;
//    if (f_read(&fat_file, &count, 3, &bytes_read) == FR_OK && bytes_read == 3)
//    {
//        if (increment == 0)
//            return 0;
//        int value = (count[0]-'0')*100 + (count[1]-'0')*10 + (count[2]-'0');
//        value += increment;
//        if (value < 0)
//            value = 999;
//        else if (value > 999)
//            value = 0;
//        count[0] = '0' + value/100;
//        value %= 100;
//        count[1] = '0' + value/10;
//        count[2] = '0' + value%10;
//    } 
//    //else if (increment == 0) // could check for error here
//    //    return 1;
//    // but instead, just write out a zero count into the file: 
//    if (increment >= 0)
//    {
//        f_lseek(&fat_file, 0);
//        f_write(&fat_file, &count, 3, &bytes_read); // dont check result
//        if (bytes_read != 3)
//            message("couldn't write into count file\n");
//    }
//    f_close(&fat_file);
//    return 0;
//}

int io_save_palette()
{
    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        filename[k++] = '.';
        filename[k++] = 'p';
        filename[k++] = 'l';
        filename[k++] = 't';
        filename[k++] = 0;
    }
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return 1;
    UINT bytes_get; 
    f_write(&fat_file, palette, sizeof(palette), &bytes_get);
    if (bytes_get != sizeof(palette))
        return 1;
    return 0;
}

int io_load_palette()
{
    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        filename[k++] = '.';
        filename[k++] = 'p';
        filename[k++] = 'l';
        filename[k++] = 't';
        filename[k++] = 0;
    }
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
        return 1;
    UINT bytes_get; 
    f_read(&fat_file, palette, sizeof(palette), &bytes_get);
    if (bytes_get != sizeof(palette))
        return 1;
    return 0;
}

int io_save_tile(unsigned int i)
{
    if (mount_failed)
    {
        io_init();
        if (mount_failed)
            return 1;
    }

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        filename[k++] = '.';
        filename[k++] = 'p';
        filename[k++] = '1';
        filename[k++] = '6';
        filename[k++] = 0;
    }
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return 1;
    
    UINT bytes_get; 
    if (i >= 16)
    {
        // write them all
        f_write(&fat_file, tile_draw, sizeof(tile_draw), &bytes_get);
        if (bytes_get != sizeof(tile_draw))
        {
            message("couldn't write all tiles to %s\n", filename);
            goto io_tile_save_error;
        }
    }
    else
    {
        f_lseek(&fat_file, i*sizeof(tile_draw[0])); 
        f_write(&fat_file, tile_draw[i], sizeof(tile_draw[0]), &bytes_get);
        if (bytes_get != sizeof(tile_draw[0]))
        {
            message("couldn't write tile %u to %s\n", i, filename);
            goto io_tile_save_error;
        }
    }

    f_close(&fat_file);
    return 0;
    
    io_tile_save_error:
    f_close(&fat_file);
    return 1;
}


int io_load_tile(unsigned int i) 
{
    if (mount_failed)
    {
        io_init();
        if (mount_failed)
            return 1;
    }

    char filename[13] = {0};
    {
        int k = 0;
        for (; k<8 && base_filename[k]; ++k)
        { 
            filename[k] = base_filename[k];
        }
        filename[k++] = '.';
        filename[k++] = 'p';
        filename[k++] = '1';
        filename[k++] = '6';
        filename[k++] = 0;
    }
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
    {
        message("could not open file %s\n", filename);
        return 1;
    }
    
    UINT bytes_get;
    if (i >= 16)
    {
        // read them all
        f_read(&fat_file, tile_draw, sizeof(tile_draw), &bytes_get);
        if (bytes_get != sizeof(tile_draw))
        {
            message("couldn't read all tiles from %s\n", filename);
            goto io_load_tile_error;
        }
    }
    else
    {
        f_lseek(&fat_file, i*sizeof(tile_draw[0])); 
        f_read(&fat_file, tile_draw[i], sizeof(tile_draw[0]), &bytes_get);
        if (bytes_get != sizeof(tile_draw[0]))
        {
            message("couldn't read tile %u from %s\n", i, filename);
            goto io_load_tile_error;
        }
    }

    f_close(&fat_file);
    return 0;
    
    io_load_tile_error:
    f_close(&fat_file);
    return 1;
}
