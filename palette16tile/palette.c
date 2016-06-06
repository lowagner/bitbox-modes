#include "palette.h"
#include "bitbox.h"

uint16_t palette[16] CCM_MEMORY; 

void palette_reset()
{
    static const uint16_t colors[16] = {
        [BLACK]=RGB(0, 0, 0),
        [GRAY]=RGB(157, 157, 157),
        [WHITE]=(1<<15) - 1,
        [PINK]=RGB(224, 111, 139),
        [RED]=RGB(190, 38, 51),
        [BLAZE]=RGB(235, 137, 49),
        [BROWN]=RGB(164, 100, 34),
        [DULLBROWN]=RGB(73, 60, 43),
        [GINGER]=RGB(247, 226, 107),
        [SLIME]=RGB(163, 206, 39),
        [GREEN]=RGB(68, 137, 26),
        [BLUEGREEN]=RGB(47, 72, 78),
        [CLOUDBLUE]=RGB(178, 220, 239),
        [SKYBLUE]=RGB(49, 162, 242),
        [SEABLUE]=RGB(0, 87, 132),
        [INDIGO]=RGB(28, 20, 40),
    };
    memcpy(palette, colors, sizeof(colors));
}

