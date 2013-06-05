CC = gcc

CFLAGS = -std=c99 \
  -Wall -W \
  -I. \
  -I../logger \
  -I../utilities \
  -I../event \
  -I../syscall_handler \
  -I../control_monitor \
  -I/usr/local/include \
  -I/usr/local/include/glib-2.0 \
  -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2

OBJ_UTILS = ../logger/logger.o \
			../event/event.o \
			../utilities/network_utils.o \
			../utilities/filesystem_utils.o \
			../utilities/str_utils.o


LIBS = -L/usr/local/lib -lxml2 

all: dataCont.o list_of_events.o data_flow_monitor_service.o data_flow_monitor.o list.o

list.o: list.h list.c
	$(CC) $(CFLAGS) -c list.c -o list.o
	
dataCont.o: dataCont.c dataCont.h
	$(CC) $(CFLAGS) -c dataCont.c -o dataCont.o

list_of_events.o: list_of_events.h list_of_events.c
	$(CC) $(CFLAGS) -c list_of_events.c -o list_of_events.o 

data_flow_monitor_service.o: data_flow_monitor_service.c data_flow_monitor_service.h dataCont.o
	$(CC) $(CFLAGS) -c data_flow_monitor_service.c -o data_flow_monitor_service.o  	

data_flow_monitor.o: data_flow_monitor.c data_flow_monitor.h dataCont.o data_flow_monitor_service.o
	$(CC) $(CFLAGS) -c data_flow_monitor.c -o data_flow_monitor.o  

		
clean:
	rm -f *.o *.core

install:

uninstall:

run:
