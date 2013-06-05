CC = gcc

CFLAGS = \
  -fno-builtin  `pkg-config --cflags libconfig` \
  -I../logger \
  -I../utilities \
  -I/usr/local/include \
  -I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2

OBJ_UTILS = \
	../logger/logger.o \
	../utilities/soup_utils.o \
	../utilities/crypto_utils.o \
	../utilities/tss_utils.o \
	../utilities/str_utils.o \
	../utilities/xml_utils.o 

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

all: event_signaler

event_signaler: $(OBJ_UTILS) event_signaler.c
	$(CC) $(CFLAGS) $(OBJ_UTILS) $(LIBS) event_signaler.c -o event_signaler 

install:
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f event_signaler
	
uninstall:
	
