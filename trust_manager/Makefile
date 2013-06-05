CC = gcc

CFLAGS = \
  -I. \
  -I../logger \
  -I../utilities \
  -I/usr/local/include \
  -I/usr/local/include/glib-2.0 \
  -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2

OBJ_UTILS = \
	../logger/logger.o \
	../utilities/str_utils.o \
	../utilities/xml_utils.o \
	../utilities/time_utils.o \
	../utilities/crypto_utils.o \
	../utilities/soup_utils.o \
	../utilities/tss_utils.o \

LIBS = \
  -L/usr/local/lib \
  -L/usr/X11R6/lib \
  -lm \
  -lsoup-2.4 \
  -lgnutls \
  -lgcrypt \
  -lgpg-error \
  -lxml2 \
  -ltspi \
  -lconfig \
  -lpthread 

all: trust_manager

trust_manager: $(OBJ_UTILS) trust_manager.o
	$(CC) $(CFLAGS) trust_manager.o -o trust_manager $(OBJ_UTILS) $(LIBS)
	
install:
	cp trust_manager /usr/local/bin/trust_manager
	chgrp bin /usr/local/bin/trust_manager
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f trust_manager
	
uninstall:
	rm -f /usr/local/bin/trust_manager
	
