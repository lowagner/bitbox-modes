# DO NOT FORGET to define BITBOX environment variable 

# allow use of SD card for io:
USE_SDCARD = 1

NAME = paddle16
GAME_C_FILES = fill.c nonsimple.c io.c main.c
GAME_H_FILES = fill.h nonsimple.h io.h

GAME_C_OPTS += -DVGAMODE_320

# see this file for options
include $(BITBOX)/lib/bitbox.mk


clean::
	rm -rf 16pal*.pxl
