CC = gcc

CFLAGS = \
  -I. \
  -I../logger \
  -I../utilities \
  -I/usr/include \
  -I/usr/include/glib-2.0 \
  -I/usr/lib/glib-2.0/include \
  -I/usr/include/libsoup-2.4 \
  -I/usr/include/libxml2

OBJ_UTILS = ../logger/logger.o

LIBS = -L/usr/local/lib -lxml2

all: event.o  event_trace

event.o: event.c event.h
	$(CC) $(CFLAGS) -c event.c -o event.o

event_trace: event_trace.c event_trace.h
	$(CC) $(CFLAGS) event_trace.c -o event_trace $(LIBS)
	
clean:
	rm -f *.o *.core

install:

uninstall:

run:
