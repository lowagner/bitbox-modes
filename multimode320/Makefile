# DO NOT FORGET to define BITBOX environment variable 

# allow use of SD card for io:
USE_SDCARD = 1

NAME = mult320
GAME_C_FILES = main.c nonsimple.c fonts.c
GAME_H_FILES = nonsimple.h

GAME_C_OPTS += -DVGA_MODE=320

# see this file for options
include $(BITBOX)/kernel/bitbox.mk


clean::
	rm -rf hello.ppm
