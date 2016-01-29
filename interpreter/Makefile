# simple makefile

# See lib/simple.c for modes
VGA_SIMPLE_MODE=10

NAME = nterpret

GAMECHUNKS = screen bsod

GAME_C_FILES = common.c main.c $(GAMECHUNKS:%=%.c)
GAME_H_FILES = common.h $(GAMECHUNKS:%=%.h)

GAMECHUNKS_H = $(GAMECHUNKS:%=%.h)

everything: $(GAMECHUNKS_H) all

# bitbox_simple.mk, will include files by default
include $(BITBOX)/lib/bitbox.mk

$(GAMECHUNKS_H):
	for chunk in $(GAMECHUNKS); do \
	echo "#ifndef $${chunk}_H\n#define $${chunk}_H\n\n" > $${chunk}.h; \
	echo "void $${chunk}_frame(void);\nvoid $${chunk}_init(void);\n\n" >> $${chunk}.h; \
	echo "#endif" >> $${chunk}.h; \
	done


clean::
	rm $(GAMECHUNKS_H)


