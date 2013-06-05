CC = gcc

CFLAGS = \
  -fno-builtin \
  -I../logger \
  -I/usr/include \
  -I/usr/include/glib-2.0 \
  -I/usr/include/libsoup-2.4 \
  -I/usr/include/libxml2 \
  -I/usr/lib/i386-linux-gnu/glib-2.0/include/

LIBS = \
  -L/usr/local/lib \
  -lsoup-2.4 \
  -lgnutls \
  -lgcrypt \
  -lgpg-error \
  -lxml2 \
  -lconfig \
  -lpthread 

LOGGER = ../logger/logger.h

all: filesystem_utils.o network_utils.o str_utils.o time_utils.o hash_table.o xml_utils.o soup_utils.o

filesystem_utils.o: filesystem_utils.c filesystem_utils.h
	$(CC) $(CFLAGS) -Wall -c filesystem_utils.c -o filesystem_utils.o

network_utils.o: network_utils.c network_utils.h
	$(CC) $(CFLAGS) -Wall -c network_utils.c -o network_utils.o

str_utils.o: str_utils.c str_utils.h $(LOGGER)
	$(CC) $(CFLAGS) -c str_utils.c -o str_utils.o

time_utils.o: $(LOGGER) time_utils.c time_utils.h
	$(CC) $(CFLAGS) -c time_utils.c -o time_utils.o

hash_table.o: hash_table.c hash_table.h $(LOGGER)
	$(CC) $(CFLAGS) -c hash_table.c -o hash_table.o

xml_utils.o: xml_utils.c xml_utils.h $(LOGGER)
	$(CC) $(CFLAGS) -c xml_utils.c -o xml_utils.o

soup_utils.o: soup_utils.c soup_utils.h $(LOGGER)
	$(CC) $(CFLAGS) -c soup_utils.c -o soup_utils.o

#crypto_utils.o: crypto_utils.c crypto_utils.h $(LOGGER)
#	$(CC) $(CFLAGS) -c crypto_utils.c -o crypto_utils.o

#tss_utils.o: tss_utils.c tss_utils.h $(LOGGER)
#	$(CC) $(CFLAGS) -c tss_utils.c -o tss_utils.o

#systrace_utils.o: systrace_utils.c systrace_utils.h $(LOGGER)
#	$(CC) $(CFLAGS) -c systrace_utils.c -o systrace_utils.o

#openbsd_utils.o: openbsd_utils.c openbsd_utils.h $(LOGGER)
#	$(CC) $(CFLAGS) -c openbsd_utils.c -o openbsd_utils.o

install:
	
clean:
	rm -f *.core
	rm -f *.o

uninstall:
	
