# DO NOT FORGET to define BITBOX environment variable 

# see lib/simple.h for modes:
#
# 800x600 at 1 bit per pixel (monochrome color palette):
# VGA_SIMPLE_MODE=2 
# 640x400 (with black top/bottom bands) with 4 color palette:
# VGA_SIMPLE_MODE=3 
# 400x300 with 16 colors:
# VGA_SIMPLE_MODE=4
# 320x200 (with black top/bottom bands) at 256 colors:
VGA_SIMPLE_MODE=5
#
# do a `make clean` if you change the simple view mode.

NAME = vbuff3d
GAME_C_FILES = main.c bb3d.c
GAME_H_FILES = bb3d.h

# see this file for options
include $(BITBOX)/lib/bitbox.mk


