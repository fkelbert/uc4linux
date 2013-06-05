CC = cc
CFLAGS = -fno-builtin `pkg-config --cflags libconfig` \
  -I../logger \
  -I../utilities \
  -I/usr/local/include \
  -I/usr/local/include/glib-2.0 -I/usr/local/lib/glib-2.0/include \
  -I/usr/local/include/libsoup-2.4 \
  -I/usr/local/include/libxml2

OBJ_UTILS = ../logger/logger.o \
	../utilities/soup_utils.o \
	../utilities/crypto_utils.o \
	../utilities/tss_utils.o
  
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

all: attestation_service attestation_client

attestation_service: $(OBJ_UTILS) attestation_service.c
	$(CC) $(CFLAGS) $(LIBS)	attestation_service.c -o attestation_service $(OBJ_UTILS)

attestation_client: $(OBJ_UTILS) attestation_client.c
	$(CC) $(CFLAGS) $(LIBS)	attestation_client.c -o attestation_client $(OBJ_UTILS)
	
install:
	
clean:
	rm -f *.core
	rm -f *.o
	rm -f attestation_client attestation_service
	
uninstall:
	
