CC = gcc

CFLAGS = -std=gnu99 \
  -Wall \
  -fno-builtin \
  -I../logger \
  -I../utilities \
  `pkg-config --cflags --libs glib-2.0 libsoup-2.4`

OBJ_UTILS = ../logger/logger.o \
	../utilities/soup_utils.o
  

all: shell_wrapper

shell_wrapper: shell_wrapper.o $(OBJ_UTILS) 
	$(CC) shell_wrapper.o -o shell_wrapper $(OBJ_UTILS) $(CFLAGS)

install:
	cp shell_wrapper /usr/bin/shell_wrapper
	chgrp bin /usr/bin/shell_wrapper
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f shell_wrapper
	
uninstall:
	rm /usr/bin/shell_wrapper
