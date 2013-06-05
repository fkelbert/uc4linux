CC = gcc

CFLAGS = -fno-builtin

all: logger.o

logger.o: logger.c logger.h
	$(CC) $(CFLAGS) -c logger.c -o logger.o
	
install:

clean:
	rm -f *.core
	rm -f *.o

uninstall: