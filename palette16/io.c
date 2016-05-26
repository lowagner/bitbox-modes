#include "fatfs/ff.h"
#include "nonsimple.h"
#include "string.h" // memcpy

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

int save_picture()
{
    if (mount_failed)
    {
        io_init();
        if (mount_failed)
            return 1;
    }
    char count[3] = { '0', '0', '0' };
    fat_result = f_open(&fat_file, "info16.txt", FA_READ | FA_WRITE | FA_OPEN_ALWAYS); 
    if (fat_result==FR_OK) 
    {
        UINT bytes_read;
        if (f_read(&fat_file, &count, 3, &bytes_read) == FR_OK && bytes_read == 3) 
        {
            ++count[2];
            if (count[2] > '9')
            {
                count[2] = '0';
                ++count[1];
                if (count[1] > '9')
                {
                    count[1] = '0';
                    ++count[0];
                    if (count[0] > '9')
                    {
                        count[0] = '0';
                    }
                }
            }
        } 
        
        f_lseek(&fat_file, 0);
        f_write(&fat_file,  &count, 3, &bytes_read); // dont check result
        f_close(&fat_file);
    }

    char filename[13] = { '1', '6', 'p', 'a', 'l', count[0], count[1], count[2], '.', 'p', 'b', 'm', 0 };
    // save in pbm format
    #ifdef PBM_PICTURES
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return 1;
    UINT bytes_get; 
    f_write(&fat_file, "P6\n320 240 31\n", 14, &bytes_get);
    if (bytes_get != 14)
    {
        message("couldn't write header to %s\n", filename);
    }
    for (int j=0; j<SCREEN_H; j++)
    {
        char msg[SCREEN_W*3]; // number of rgb chars to write per line
        char *src = msg;
        for (int i=0; i<SCREEN_W/2; i++)
        {
            uint32_t C2 = palette[superpixel[j][i]];
            *src++ = (C2 >> 10)&31; // red
            *src++ = (C2 >> 5)&31; // green
            *src++ = (C2)&31; // blue
            *src++ = (C2 >> (16+10))&31; // red
            *src++ = (C2 >> (16+5))&31; // green
            *src++ = (C2 >> (16))&31; // blue
        }
        f_write(&fat_file, msg, sizeof(msg), &bytes_get);
        if (bytes_get != SCREEN_W*3)
        {
            message("couldn't write to file %s at line %d\n", filename, j);
            goto end_save;
        }
    }
    #else
    filename[10] = 'x'; filename[11] = 'l';
    fat_result = f_open(&fat_file, filename, FA_WRITE | FA_OPEN_ALWAYS);
    if (fat_result != FR_OK)
        return 1;
    UINT bytes_get; 
    f_write(&fat_file, "pxl16 i4 c16 320 240\n", 21, &bytes_get);
    if (bytes_get != 21)
    {
        message("couldn't write header to file %s\n", filename);
        goto end_save;
    }

    uint8_t p[16*2];
    for (int i=0; i<16; ++i)
    {
        p[2*i] = palette[i]&255;
        p[2*i+1] = (palette[i]>>8)&255;
    }
    f_write(&fat_file, p, sizeof(p), &bytes_get);
    if (bytes_get != sizeof(p))
    {
        message("couldn't write palette to %s\n", filename);
        goto end_save;
    }
    
    f_write(&fat_file, superpixel, sizeof(superpixel), &bytes_get);
    if (bytes_get != sizeof(superpixel))
    {
        message("couldn't write pixels to %s\n", filename);
        goto end_save;
    }
    #endif

    end_save:
    f_close(&fat_file);
    message("finished save_picture()\n");
    return 0;
}


int load_picture(int previous)
{
    UINT bytes_read;
    if (mount_failed)
    {
        io_init();
        if (mount_failed)
            return 1;
    }
    char count[3] = { '0', '0', '0' };
    fat_result = f_open(&fat_file, "info16.txt", FA_READ | FA_OPEN_EXISTING); 
    if (fat_result==FR_OK) 
    {
        if (f_read(&fat_file, &count, 3, &bytes_read) == FR_OK && bytes_read == 3 && previous) 
        {
            int value = (count[0]-'0')*100 + (count[1]-'0')*10 + (count[2]-'0');
            value -= previous;
            if (value < 0)
                value = 999;
            else if (value > 999)
                value = 0;
            count[0] = '0' + value/100;
            value %= 100;
            count[1] = '0' + value/10;
            count[2] = '0' + value%10;
        } 
        f_close(&fat_file);
    }

    char filename[13] = { '1', '6', 'p', 'a', 'l', count[0], count[1], count[2], '.', 'p', 'b', 'm', 0 };
    // save in pbm format
    #ifdef PBM_PICTURES
    return 1;
    #else
    filename[10] = 'x'; filename[11] = 'l';
    fat_result = f_open(&fat_file, filename, FA_READ | FA_OPEN_EXISTING);
    if (fat_result != FR_OK)
    {
        message("could not open file %s\n", filename);
        goto file_error;
    }
    uint8_t buffer[256];
    f_read(&fat_file, &buffer, sizeof(buffer), &bytes_read);
    if (bytes_read < 256 || buffer[0] != 'p' || buffer[1] != 'x' || buffer[2] != 'l')
    {
        message("not a valid file format, or too small\n");
        goto file_error;
    }

    int i=3, palette_count = 0; 
    if (buffer[i] < '0' || buffer[i] > '9')
    {
        message("expected a palette for this program.\n");
        goto file_error; // expected a palette count, file format error
    }
    else
        palette_count = buffer[i] - '0';
    
    while (i < 256 - 1)
    {
        ++i;
        if (buffer[i] == ' ') // end palette
            break;
        else if (buffer[i] < '0' || buffer[i] > '9')
        {
            message("expected a space or number for palette_count\n");
            goto file_error; // expected a number or a space here
        }
        else
            palette_count = (palette_count*10) + buffer[i] - '0';
    }
    if (palette_count == 0)
    {
        message("got palette_count == 0\n");
        goto file_error; // can't have a zero paletted picture
    }
    if (palette_count > 16)
    {
        message("got palette_count > 16\n");
        goto file_error; // in this program, you can't have more than 16 colors
    }
   
    // now read the specific meta-data
    int index_bits = 4;
    int color_depth = 16;
    int width = 0, height = 0;
    while (i < 256 - 1)
    {
        ++i;
        // look for a key/value pair in the form i16 or c24
        uint8_t key = 0;
        while (i < 256)
        {
            key = buffer[i];
            if (key == ' ' || key == '\n' || key == '\t')
                ++i; 
            else
                break;
        }
        if (i == 256)
        {
            message("got to end of header without width and height info\n");
            goto file_error;
        }

        if (key >= '0' && key <= '9')
        {
            width = key - '0';
            break; // switch over into width x height mode
        }
        ++i;
        
        if (i >= 256 || buffer[i] < '0' || buffer[i] > '9')
        {
            message("got key but expected value immediately after it\n");
            goto file_error; // expected a number here
        }
        int value = buffer[i] - '0';
        
        while (i < 256 - 1)
        {
            ++i;
            if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t')
                break;
            else if (buffer[i] < '0' || buffer[i] > '9')
            {
                message("expected number or space to get value for key %c\n", key);
                goto file_error; // expected a number here
            }
            else
                value = (value*10) + buffer[i] - '0';
        }
        switch (key)
        {
        case 'i':
            index_bits = value;
            break;
        case 'c':
            color_depth = value;
            break;
        default:
            message("unknown key %c -> value %d\n", key, value);
            // unknown key, do nothing
        }
    }
    // get rest of width until space
    while (i < 256 - 1)
    {
        ++i;
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t')
            break;
        else if (buffer[i] < '0' || buffer[i] > '9')
        {
            message("expected width then space\n");
            goto file_error; // expected a number here
        }
        else
            width = (width*10) + buffer[i] - '0';
    }
    // get space until height starts
    while (i < 256 - 1)
    {
        ++i;
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') {}
        else if (buffer[i] < '0' || buffer[i] > '9')
        {
            message("expected space between width and height\n");
            goto file_error; // expected number or spaces to get to height
        }
        else
        {
            height = buffer[i] - '0';
            break;
        }
    }
    // get rest of height
    while (i < 256 - 1)
    {
        ++i;
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t')
            break;
        else if (buffer[i] < '0' || buffer[i] > '9')
        {
            message("expected space to break out of height\n");
            goto file_error; // expected a number here
        }
        else
            height = (height*10) + buffer[i] - '0';
    }
    if (width != 320)
    {
        message("expected width 320, got %d\n", width);
        goto file_error;
    }
    if (height != 240)
    {
        message("expected height 240, got %d\n", height);
        goto file_error;
    }
    if (index_bits != 4)
    {
        message("expected index bits 4, got %d\n", index_bits);
        goto file_error; // for this program
    }
    if (color_depth != 16)
    {
        message("expected color depth 16, got %d\n", color_depth);
        goto file_error; // for this program.  could modify it to read in 24 color depth
    }

    ++i; // now arrived at start of palette section
    if (i + palette_count*2 >= 256)
    {
        message("expected palette within the first 256 bytes of the file, or file truncated.\n");
        goto file_error; // still need data and palette, expecting it in the first 256 bytes
    }

    for (int k=0; k<palette_count; ++k)
    {
        palette[k] = (buffer[i])|(buffer[i+1]<<8);
        message("got palette %d -> %d\n", k, palette[k]);
        i += 2;
    }
    // initialize the 256 palette:
    for (int first=0; first<palette_count; ++first)
    for (int second=0; second<palette_count; ++second)
        palette[first + second*16] = (palette[first] & 65535)|( (palette[second]&65535) << 16 );

    // now arrived at the start of the data section
    uint8_t *dst = superpixel[0];
    memcpy(dst, &buffer[i], 256-i);
    i = 256 - i; // now let i manager where we are in superpixel, byte-wise:
    dst += i;
    do
    {
        int size = 256;
        if (i + size > sizeof(superpixel))
            size = sizeof(superpixel) - i;
        f_read(&fat_file, dst, size, &bytes_read);
        //f_read(&fat_file, buffer, size, &bytes_read);
        i += bytes_read;
        dst += bytes_read;
        if (bytes_read != size)
        {
            message("probably done with file, but unexpectedly. (filled %d / %d)\n", i, sizeof(superpixel));
            goto file_error; // probably done with file, but something seems wrong
        }
    } while (bytes_read == 256 && i < sizeof(superpixel));

    #endif

    message("finished load_picture() - %d / %d\n", i, sizeof(superpixel));
    f_close(&fat_file);
    return 0;
    
    file_error:
    clear_screen();
    f_close(&fat_file);
    return 1;
}
