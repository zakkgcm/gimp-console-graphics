# Makefile for gimp-console-graphics plugin
# cheeseum 2011

PLUG_IN_NAME = file-console-graphics

CC = cc
GIMPTOOL = gimptool-2.0

CFLAGS 	= $(shell $(GIMPTOOL) --cflags)
LDFLAGS = $(shell $(GIMPTOOL) --libs)

SRC = file-console-graphics.c\
	  file-console-graphics-load.c\
	  file-console-graphics-save.c\
	  file-console-graphics-formatdefs.c

HEAD = $(SRC:.h=.o)
OBJ = $(SRC:.c=.o)

.PHONY: install

all: options gimp-console-graphics

options:
	@echo gimp-console-graphics-options:
	@echo "CFLAGS ${CFLAGS}"
	@echo "LDFLAGS: ${LDFLAGS}"
	@echo

%.o: %.c ${HEAD}
	@echo ${CC} $<
	@${CC} -c ${CFLAGS} $< -o $@

gimp-console-graphics: options ${OBJ}
	@echo ${CC} ${OBJ} -o ${PLUG_IN_NAME}
	@${CC} ${LDFLAGS} ${OBJ} -o ${PLUG_IN_NAME}
	@echo

install: gimp-console-graphics
	@echo "installing gimp-console-graphics (as ${PLUG_IN_NAME}) using ${GIMPTOOL}"
	${GIMPTOOL} --quiet --install-bin ${PLUG_IN_NAME}

clean:
	@echo cleaning up object files and binaries...
	@rm -vf ${OBJ}
	@rm -vf ${PLUG_IN_NAME}
	@echo all clean~
