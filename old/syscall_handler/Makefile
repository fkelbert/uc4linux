CC = gcc

CFLAGS = -Wall \
	-I. \
	-I../event \
	-I../logger \
	-I../utilities \
	-I../dataCont \
	-I../control_monitor \
	-I../past_monitor \
	-I/usr/local/include \
	-I/usr/local/include/glib-2.0 \
	-I/usr/local/lib/glib-2.0/include \
	-I/usr/local/include/libsoup-2.4 \
	-I/usr/local/include/libxml2

OBJ_UTILS = \
	../logger/logger.o \
	../event/event.o \
	../utilities/str_utils.o \
	../utilities/time_utils.o \
	../utilities/hash_table.o \
	../utilities/xml_utils.o \
	../utilities/soup_utils.o \
	../utilities/systrace_utils.o \
	../utilities/openbsd_utils.o \
	../utilities/threads.o \
	../utilities/network_utils.o \
	../utilities/filesystem_utils.o \
	../past_monitor/lex.yy.o \
	../past_monitor/past_parser.tab.o \
	../past_monitor/past_monitor.o \
	../dataCont/dataCont.o \
	../dataCont/data_flow_monitor.o \
	../dataCont/data_flow_monitor_service.o \
	../dataCont/list_of_events.o \
	../dataCont/list.o \
	../control_monitor/control_monitor.o \
	../utilities/libpbl/libpbl.a

LIBS = \
	-L/usr/local/lib \
	-L/usr/X11R6/lib \
	-lm \
	-lsoup-2.4 \
	-lxml2 \
	-lgnutls \
	-lgcrypt \
	-lgpg-error \
	-lpthread \
	-lkvm \
#	-ltspi

all: syscall_handler

syscall_handler.o: syscall_handler.c syscall_handler.h
	$(CC) $(CFLAGS) -c syscall_handler.c -o syscall_handler.o

syscall_handler: $(OBJ_UTILS) syscall_handler.o syscall_handler.h
	$(CC) $(CFLAGS) $(LIBS)	$(OBJ_UTILS) -o syscall_handler syscall_handler.o -ly -lfl

clean:
	rm -f *.o	
	rm -f *.core
	rm -f syscall_handler
