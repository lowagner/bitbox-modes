#include "common.h"
#include "bitbox.h"

uint8_t tile_draw[16][16][8] CCM_MEMORY;
uint8_t tile_map[TILE_MAP_MEMORY] CCM_MEMORY;
uint8_t tile_translator[16] CCM_MEMORY;
// info about a tile:
//   5 bits for "what is hiding inside this tile?", 
//      a value < 16 is "nothing hides here", and also indicates if the tile is a water-based tile.
//          [0 = not water, 1 = quick-sand, ..., 8 = water, ..., 15 = liquid mercury] 
//      a value >= 16 means sprite (value-16) hides here.
//   3 bits:
//      if not water:  
//          for tile strength (can only destroy a tile using an attack which &'s the strength).
//      if water:
//          current direction and strength
//   4 bits for translation of tile (use tile number itself for no translation)
//   4 bits for translation timing (0000 -> 16 units of time), where one unit of time is 32 frames
//   4 bits for damage/passable property, x 4 sides 
//      first bit indicates damaging (or not if it's zero)
//      second three bits:  passable, breakable, hard, slippery, sticky, bouncey, 
uint32_t tile_info[16] CCM_MEMORY;
int16_t tile_map_x CCM_MEMORY, tile_map_y CCM_MEMORY;
uint16_t tile_map_width CCM_MEMORY, tile_map_height CCM_MEMORY;
// tile_map_width * tile_map_height <= TILE_MAP_MEMORY
// tile_map_x < tile_map_width - 320
// tile_map_y < tile_map_height - 240

void tiles_init()
{
    // init tile mapping
    for (int j=0; j<16; ++j)
        tile_translator[j] = j; 
}

uint32_t pack_tile_info(uint8_t hiding, uint8_t strength, 
    uint8_t translation, uint8_t timing, const SideType *sides)
{
    return (hiding&31)|((strength&7)<<5)|((translation&15)<<8)|((timing&15)<<12)|
        (sides[0]<<16)|(sides[1]<<20)|(sides[2]<<24)|(sides[3]<<28);
}

void unpack_tile_info(uint32_t value, uint8_t *hiding, uint8_t *strength, 
    uint8_t *translation, uint8_t *timing, SideType *sides)
{
    *hiding = value&31;
    value >>= 5;
    *strength = value & 7;
    value >>= 3;
    *translation = value & 15;
    value >>= 4;
    *timing = value & 15;
    value >>= 4;
    sides[0] = value & 15;
    value >>= 4;
    sides[1] = value & 15;
    value >>= 4;
    sides[2] = value & 15;
    value >>= 4;
    sides[3] = value & 15;
}

void tiles_line()
{
    if (tile_map_x % 16)
    {
        uint16_t *dst = draw_buffer;
        int tile_j = tile_map_y + vga_line;
        int index = (tile_j/16)*(tile_map_width) + tile_map_x / 16;
        tile_j %= 16;
        uint8_t *tile = &tile_map[index/2];
        if (index % 2)
        {
            // draw the first tile (it's somewhat off-screen)
            uint8_t trans = tile_translator[((*tile)>>4)]; // first one is odd
                
            uint8_t *tile_color = &tile_draw[trans][tile_j][(tile_map_x%16)/2] - 1;
            if (tile_map_x % 2)
            {
                ++tile_color;
                for (int l=(tile_map_x%16)/2; l<7; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
                *dst++ = palette[(*tile_color)>>4];
            }
            else
            {
                for (int l=(tile_map_x%16)/2; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }

            // draw 19 un-broken tiles (tile 2 to tile 21)
            for (int k=0; k<19/2; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                // first one is even (not odd)
                trans = tile_translator[(*(++tile))&15];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }

                // second one is odd
                trans = tile_translator[((*tile)>>4)];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
            // 21st tile is even, but still unbroken
            trans = tile_translator[(*(++tile))&15];
            tile_color = &tile_draw[trans][tile_j][0] - 1;
            for (int l=0; l<8; ++l)
            {
                *dst++ = palette[(*(++tile_color))&15];
                *dst++ = palette[(*tile_color)>>4];
            }

            // draw 22'nd broken tile is odd
            trans = tile_translator[((*tile)>>4)&15];
            tile_color = &tile_draw[trans][tile_j][0]-1; 
            if (tile_map_x % 2)
            {
                *dst++ = palette[(*(++tile_color))&15];
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
            }
            else
            {
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
        }
        else 
        {
            // draw the first tile (it's somewhat off-screen)
            uint8_t trans = tile_translator[(*tile)&15]; // first one is even
            uint8_t *tile_color = &tile_draw[trans][tile_j][(tile_map_x%16)/2] - 1;
            if (tile_map_x % 2)
            {
                ++tile_color;
                for (int l=(tile_map_x%16)/2; l<7; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
                *dst++ = palette[(*tile_color)>>4];
            }
            else
            {
                for (int l=(tile_map_x%16)/2; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }

            // draw 19 un-broken tiles (tile 2 to tile 21)
            for (int k=0; k<19/2; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                // first one is odd
                trans = tile_translator[((*tile)>>4)];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
                
                // second one is even
                trans = tile_translator[(*(++tile))&15];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
            // 21st tile is odd, but still unbroken
            trans = tile_translator[((*tile)>>4)];
            tile_color = &tile_draw[trans][tile_j][0] - 1;
            for (int l=0; l<8; ++l)
            {
                *dst++ = palette[(*(++tile_color))&15];
                *dst++ = palette[(*tile_color)>>4];
            }

            // draw 22'nd broken tile is even
            trans = tile_translator[(*(++tile))&15];
            tile_color = &tile_draw[trans][tile_j][0]-1; 
            if (tile_map_x % 2)
            {
                *dst++ = palette[(*(++tile_color))&15];
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*tile_color)>>4];
                    *dst++ = palette[(*(++tile_color))&15];
                }
            }
            else
            {
                for (int l=0; l<(tile_map_x%16)/2; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
        }
    }
    else // tile_map_x puts a tile exactly on the edge of the screen
    {
        uint16_t *dst = draw_buffer;
        int tile_j = tile_map_y + vga_line;
        int index = (tile_j/16)*(tile_map_width) + tile_map_x / 16;
        tile_j %= 16;
        uint8_t *tile = &tile_map[index/2]-1;
        if (index % 2)
        {
            ++tile;
            for (int k=0; k<10; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                uint8_t trans = tile_translator[((*tile)>>4)];

                uint8_t *tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
                    
                trans = tile_translator[(*(++tile))&15];
                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }
        }
        else // not odd
        {
            for (int k=0; k<10; ++k)
            {
                // translate the tile into what tile it should be drawn as:
                uint8_t trans = tile_translator[(*(++tile))&15];

                uint8_t *tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
                
                // translate the tile into what tile it should be drawn as:
                trans = tile_translator[((*tile)>>4)];

                tile_color = &tile_draw[trans][tile_j][0] - 1;
                for (int l=0; l<8; ++l)
                {
                    *dst++ = palette[(*(++tile_color))&15];
                    *dst++ = palette[(*tile_color)>>4];
                }
            }

        }
    }
}

void tiles_reset()
{
    // tile 0
    uint8_t *tc = tile_draw[0][0];
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        *tc++ = SKYBLUE|(SKYBLUE<<4);
    }
    // next tiles are mostly solid colors
    for (int k=0; k<9; ++k)
    for (int l=0; l<128; ++l)
    {
        *tc++ = (2+k)|((3+k)<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/8)%2 + i/4) % 2)
            *tc++ = BLAZE|(BLAZE<<4);
        else
            *tc++ = DULLBROWN|(DULLBROWN<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j)%2 + i) % 2)
            *tc++ = CLOUDBLUE|(SEABLUE<<4);
        else
            *tc++ = BLUEGREEN|(BLUEGREEN<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/2)%2 + i) % 2)
            *tc++ = BROWN|(BROWN<<4);
        else
            *tc++ = DULLBROWN|(DULLBROWN<<4);
    }
    // etc.
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/4)%2 + i/2) % 2)
            *tc++ = RED|(RED<<4);
        else
            *tc++ = GINGER|(GINGER<<4);
    }
    // next tile
    for (int j=0; j<16; ++j)
    for (int i=0; i<8; ++i)
    {
        if (((j/8)%2 + i/4) % 2)
            *tc++ = GREEN|(GREEN<<4);
        else
            *tc++ = INDIGO|(INDIGO<<4);
    }
    // splitting a tile up
    for (int i=0; i<8; ++i)
    {
        if (((0/8)%2 + i/4) % 2)
            *tc++ = CLOUDBLUE|(CLOUDBLUE<<4);
        else
            *tc++ = SLIME|(SLIME<<4);
    }
    for (int j=1; j<15; ++j)
    {
        if (((j/8)%2 + 0/8) % 2)
            *tc++ = CLOUDBLUE|(SKYBLUE<<4);
        else
            *tc++ = SLIME|(GREEN<<4);
        for (int i=1; i<7; ++i)
        {
            if (((j/8)%2 + i/4) % 2)
                *tc++ = SKYBLUE|(SKYBLUE<<4);
            else
                *tc++ = GREEN|(GREEN<<4);
        }
        if (((j/8)%2 + 15/8) % 2)
            *tc++ = SKYBLUE|(SEABLUE<<4);
        else
            *tc++ = GREEN|(BLUEGREEN<<4);
    }
    for (int i=0; i<8; ++i)
    {
        if (((15/8)%2 + i/4) % 2)
            *tc++ = SEABLUE|(SEABLUE<<4);
        else
            *tc++ = BLUEGREEN|(BLUEGREEN<<4);
    }
    {
    SideType pass[4] = { Passable, Passable, Passable, Passable };
    tile_info[0] = pack_tile_info(16, 1, 0, 0, pass);
    }
    SideType sides[4] = { Hard, Hard, Hard, Hard };
    for (int i=1; i<16; ++i)
        tile_info[i] = pack_tile_info(16, 1, i, 0, sides);
}
