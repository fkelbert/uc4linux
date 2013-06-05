CC = gcc

CFLAGS = \
  -I. \
  -I/usr/local/include \
  -I../event \
  -I../logger \
  -I../utilities \
  -I../utilities/libpbl \
  -I../dataCont \
  -I/usr/local/include/libxml2 \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2/libxml \
  -I../control_monitor \
  -I/usr/local/include/glib-2.0 \
  -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libxml2

OBJ_UTILS = \
	../event/event.o \
	../logger/logger.o \
	../utilities/str_utils.o \
	../utilities/xml_utils.o \
	../utilities/time_utils.o \
	../utilities/soup_utils.o \
	../utilities/network_utils.o \
	../utilities/threads.o \
	../utilities/filesystem_utils.o \
	../utilities/libpbl/libpbl.a \
	../dataCont/dataCont.o \
	../dataCont/data_flow_monitor.o \
	../dataCont/data_flow_monitor_service.o \
	../dataCont/list_of_events.o \
	../dataCont/list.o \
	../control_monitor/control_monitor.o \
	../utilities/hash_table.o


LIBS = \
  -L/usr/local/lib \
  -L/usr/X11R6/lib \
  -lm \
  -lsoup-2.4 \
  -lgnutls \
  -lgcrypt \
  -lgpg-error \
  -lxml2 \
  -lconfig \
  -lpthread \
#  -ltspi
  
all: past_parser.tab.c past_parser.tab.o lex.yy.c lex.yy.o past_monitor.o past_monitor_demo

past_parser.tab.c: past_parser.y
	bison -d past_parser.y

past_parser.tab.o: past_parser.tab.c
	$(CC) $(CFLAGS) -c past_parser.tab.c -o past_parser.tab.o

lex.yy.c: past_scanner.l
	flex past_scanner.l
	
lex.yy.o: lex.yy.c 
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o

past_monitor.o: past_monitor.c past_monitor.h
	$(CC) $(CFLAGS)  -c past_monitor.c -o past_monitor.o

past_monitor_demo: past_monitor_demo.o $(OBJ_UTILS) lex.yy.o past_monitor.o
	$(CC) $(CFLAGS)  $(LIBS) \
	past_monitor_demo.o -o past_monitor_demo $(OBJ_UTILS) lex.yy.o past_parser.tab.o past_monitor.o -ly -lfl -lm ../utilities/libpbl/libpbl.a

	
clean:
	rm -f *.o
	rm -f *.core
	rm -f lex.yy.c
	rm -f past_parser.tab.c
	rm -f past_parser.tab.h
	rm -f past_monitor.o
	rm -f past_monitor_demo
	
install:

uninstall:

run:
