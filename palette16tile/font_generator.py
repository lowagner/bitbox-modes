#!/usr/bin/env python2

starting_index = 32

characters = [
   ["    ", # ' '
    "    ",
    "    ",
    "    "],
   [" ** ", # !
    " ** ",
    "    ",
    " ** "],
   ["* **", # "
    "* **",
    "    ",
    "    "],
   [" * *", # #
    "****",
    " * *",
    "****"],
   [" ***", # $
    "* * ",
    " * *",
    "*** "],
   ["** *", # %
    "* * ",
    " * *",
    "* **"],
   [" ** ", # &
    "****",
    "* * ",
    " ** "],
   [" ** ", # '
    " ** ",
    "    ",
    "    "],
   [" ** ", # (
    "**  ",
    "**  ",
    " ** "],
   [" ** ", # )
    "  **",
    "  **",
    " ** "],
   [" * *", # *
    "  * ",
    " * *",
    "    "],
   ["  * ", # +
    " ***",
    "  * ",
    "    "],
   ["    ", # ,
    "    ",
    " ** ",
    "*** "],
   ["    ", # -
    " ***",
    "    ",
    "    "],
   ["    ", # .
    "    ",
    " ** ",
    " ** "],
   ["   *", # /
    "  * ",
    " *  ",
    "*   "],
   ["****", # 0
    "** *",
    "** *",
    "****"],
   ["*** ", # 1
    " ** ",
    " ** ",
    "****"],
   ["*** ", # 2
    "  **",
    "**  ",
    "****"],
   ["****", # 3
    " ***",
    "   *",
    "****"],
   ["* **", # 4
    "****",
    "  **",
    "  **"],
   ["****", # 5
    "*** ",
    "  **",
    "*** "],
   ["****", # 6
    "**  ",
    "* **",
    "****"],
   ["****", # 7
    "  **",
    " ** ",
    " ** "],
   ["*** ", # 8
    "* **",
    "** *",
    " ***"],
   ["****", # 9
    "** *",
    "  **",
    "****"],
   [" *  ", # :
    "    ",
    " *  ",
    "    "],
   [" *  ", # ;
    "    ",
    " *  ",
    "**  "],
   ["  * ", # <
    " *  ",
    "  * ",
    "    "],
   [" ***", # =
    "    ",
    " ***",
    "    "],
   [" *  ", # >
    "  * ",
    " *  ",
    "    "],
   ["*** ", # ?
    "  * ",
    " ** ",
    "*   "],
   ["****", # @
    "   *",
    "** *",
    "****"],
   [" ** ", # A
    "* **",
    "****",
    "* **"],
   ["*** ", # B
    "* **",
    "** *",
    "****"],
   [" ***", # C
    "*** ",
    "*** ",
    " ***"],
   ["*** ", # D
    "** *",
    "** *",
    "*** "],
   ["****", # E
    "*** ",
    "**  ",
    "****"],
   ["****", # F
    "**  ",
    "*** ",
    "**  "],
   [" ***", # G
    "**  ",
    "** *",
    "****"],
   ["* **", # H
    "****",
    "* **",
    "* **"],
   ["****", # I
    " ** ",
    " ** ",
    "****"],
   ["****", # J
    "  **",
    "  **",
    "*** "],
   ["** *", # K
    "*** ",
    "** *",
    "** *"],
   ["**  ", # L
    "**  ",
    "**  ",
    "****"],
   ["****", # M
    "****",
    "* **",
    "*  *"],
   ["** *", # N
    "** *",
    "* **",
    "* **"],
   [" ** ", # O
    "** *",
    "** *",
    " ** "],
   ["****", # P
    "** *",
    "****",
    "**  "],
   ["****", # Q
    "*  *",
    "* **",
    "****"],
   ["*** ", # R
    "** *",
    "*** ",
    "* **"],
   ["****", # S
    "**  ",
    "  **",
    "****"],
   ["****", # T
    " ** ",
    " ** ",
    " ** "],
   ["* **", # U
    "* **",
    "* **",
    " ** "],
   ["** *", # V
    "** *",
    "*** ",
    " *  "],
   ["*  *", # W
    "** *",
    "****",
    "****"],
   ["** *", # X
    "  * ",
    "** *",
    "** *"],
   ["** *", # Y
    "** *",
    " ** ",
    " **"],
   ["****", # Z
    "  **",
    "**  ",
    "****"],
   ["*** ", # [
    "**  ",
    "**  ",
    "*** "],
   ["*   ", # \
    " *  ",
    "  * ",
    "   *"],
   [" ***", # ]
    "  **",
    "  **",
    " ***"],
   ["  * ", # ^
    " * *",
    "    ",
    "    "],
   ["    ", # _
    "    ",
    "    ",
    "****"],
   [" *  ", # `
    "  * ",
    "    ",
    "    "],
   ["    ", # a
    " ***",
    "* **",
    "****"],
   ["**  ", # b
    "*** ",
    "** *",
    "*** "],
   ["    ",
    " ***", # c
    "**  ",
    " ***"],
   ["  **", # d
    " ***",
    "* **",
    " ***"],
   [" ** ", # e
    "** *",
    "*** ",
    " ***"],
   [" ***", # f
    "**  ",
    "*** ",
    "**  "],
   [" ** ", # g
    "* **",
    " ***",
    "*** "],
   ["**  ", # h
    "**  ",
    "****",
    "** *"],
   [" ** ", # i
    "    ",
    " ** ",
    " ***"],
   ["  **", # j
    "    ",
    "  **",
    "****"],
   ["**  ", # k
    "** *",
    "*** ",
    "** *"],
   [" ** ", # l
    " ** ",
    " ** ",
    " ** "],
   ["    ", # m
    "****",
    "* **",
    "*  *"],
   ["    ", # n
    "*** ",
    "** *",
    "** *"],
   ["    ", # o
    " ** ",
    "** *",
    " ** "],
   ["*** ", # p
    "** *",
    "*** ",
    "**  "],
   [" ***", # q
    "* **",
    " ***",
    "  **"],
   ["    ", # r
    "*** ",
    "** *",
    "**  "],
   ["    ",
    " ***", # s
    " ** ",
    "*** "],
   [" ** ", # t
    "****",
    " ** ",
    " ***"],
   ["    ", # u
    "* **",
    "* **",
    " ** "],
   ["    ", # v
    "** *",
    "*** ",
    " *  "],
   ["    ",
    "*  *", # w
    "** *",
    "****"],
   ["    ", # x
    "** *",
    "  * ",
    "** *"],
   ["    ", # y
    " * *",
    " ***",
    "***"],
   ["    ", # z
    "*** ",
    " ** ",
    " ***"],
   [" ** ", # {
    "**  ",
    " *  ",
    " ** "],
   ["  * ", # |
    "  * ",
    "  * ",
    "    "],
   [" ** ", # }
    "  **",
    "  * ",
    " ** "],
   ["   *", # ~
    " ***",
    " *  ",
    "    "],
   ["    ", # 127
    " ** ",
    " ** ",
    "    "]
]

with open("font.c", 'w') as f:
    f.write("#include \"bitbox.h\"\n#include \"common.h\"\n#include \"font.h\"\n#include <string.h> // strlen\n")
    f.write("uint16_t font[256] CCM_MEMORY;\n")
    f.write("uint16_t font_cache[256] = {\n")
    if starting_index:
        f.write("[%d]="%starting_index)
    else:
        f.write("  ")
    for i in range(len(characters)):
        char = characters[i]
        x = 0
        for j in range(len(char)):
            power = 4*j
            for c in char[j]:
                if c != ' ':
                    x |= 1<<power
                power += 1
        if i + 1 == len(characters):
            f.write("%d\n"%x)
        else:
            f.write("%d,\n  "%x)
    f.write("};\n")
    if len(characters) + starting_index > 256:
        print "WARNING, overflow!"
    f.write("""
void font_render_line_doubled(const uint8_t *text, int x, int y, uint16_t color_fg, uint16_t color_bg)
{
    #ifdef EMULATOR
    if (y < 0 || y >= 8)
    {
        message("got too big a line count for text (%s):  %d\\n", text, y);
        return;
    }
    if (x < 0 || x + 8*strlen((char *)text) >= SCREEN_W)
    {
        message("text (%s) goes off screen!\\n", text);
        return;
    }
    #endif
    y = ((y/2))*4; // make y now how much to shift
    uint16_t *dst = draw_buffer + x;
    uint16_t color_choice[2] = { color_bg, color_fg };
    *dst = color_choice[0];
    --text;
    int c;
    while ((c = *(++text)))
    {
        uint8_t row = (font[c] >> y) & 15;
        for (int j=0; j<4; ++j)
        {
            *(++dst) = color_choice[row&1];
            *(++dst) = color_choice[row&1];
            row >>= 1;
        }
        *(++dst) = color_choice[0];
    }
}""")


with open("font.h", 'w') as f:
    f.write("#ifndef FONT_H\n#define FONT_H\n#include <stdint.h>\n")
    f.write("extern uint16_t font_cache[256];\nextern uint16_t font[256];\nvoid font_render_line_doubled(const uint8_t *text, int x, int y, uint16_t color_fg, uint16_t color_bg);\n#endif\n")

"""
some characters left to implement
    ***   **
      ** ****
     *** ** *
    * ** ** *

"""
