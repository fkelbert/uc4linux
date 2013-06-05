CC = gcc

CFLAGS = \
  -fno-builtin \
  -I../logger \
  -I../utilities \
  -I/usr/local/include \
  -I/usr/local/include/glib-2.0 \
  -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2

OBJ_UTILS = ../logger/logger.o \
	../utilities/soup_utils.o
  
LIBS = \
  -L/usr/local/lib \
  -L/usr/X11R6/lib \
  -lm \
  -lsoup-2.4 \
  -lxml2 \
  -lpthread 

all: shell_wrapper

shell_wrapper: shell_wrapper.o $(OBJ_UTILS) 
	$(CC) $(CFLAGS) $(LIBS) $(OBJ_UTILS) shell_wrapper.o -o shell_wrapper

install:
	cp shell_wrapper /usr/local/bin/shell_wrapper
	chgrp bin /usr/local/bin/shell_wrapper
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f shell_wrapper
	
uninstall:
	rm /usr/local/bin/shell_wrapper