# DO NOT FORGET to define BITBOX environment variable 

USE_SDCARD = 1      # allow use of SD card for io

NAME = 16tiles
GAME_C_FILES = font.c io.c map.c palette.c edit.c save.c fill.c tiles.c sprites.c main.c
GAME_H_FILES = font.h io.h map.h palette.h edit.h save.h fill.h tiles.h sprites.h common.h 
GAME_C_OPTS += -DVGAMODE_320

# see this file for options
include $(BITBOX)/lib/bitbox.mk

font.c: font_generator.py
	python2 font_generator.py

clean::
	rm -f font.c font.h
