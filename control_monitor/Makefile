CC = gcc

CFLAGS = -Wall \
  -I. \
  -I../event \
  -I../logger \
  -I../utilities \
  -I../utilities/libpbl \
  -I../past_monitor \
  -I../control_monitor \
  -I../dataCont \
  -I/usr/local/include \
  -I/usr/local/include/glib-2.0 \
  -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2

OBJ_UTILS = \
	../logger/logger.o \
	../event/event.o \
	../utilities/threads.o \
	../utilities/str_utils.o \
	../utilities/time_utils.o \
	../utilities/hash_table.o \
	../utilities/xml_utils.o \
	../utilities/soup_utils.o \
	../utilities/crypto_utils.o \
	../utilities/tss_utils.o \
	../utilities/network_utils.o \
	../utilities/filesystem_utils.o \
	../past_monitor/lex.yy.o \
	../dataCont/dataCont.o \
	../dataCont/data_flow_monitor.o \
	../dataCont/data_flow_monitor_service.o \
	../dataCont/list_of_events.o \
	../dataCont/list.o \
	../past_monitor/past_parser.tab.o \
	../past_monitor/past_monitor.o


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
  -lpthread 

all: control_monitor.o #control_monitor_service 


control_monitor_service: control_monitor.o control_monitor_service.c
	$(CC) $(CFLAGS) \
	control_monitor_service.c -o control_monitor_service \
	control_monitor.o $(OBJ_UTILS) $(LIBS) -ly -lfl -lm

clean:
	rm -f *.o *.core
	rm -f control_monitor_service

install:

uninstall:

run:
