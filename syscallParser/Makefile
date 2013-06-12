CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
LIBGLIB=`pkg-config --cflags --libs glib-2.0`



all: syscallParser.o

syscallParser.o: 
	$(CC) -c $(SRC)/syscallParser.c -o $(OBJDIR)/sycallParser.o $(LIBGLIB)
	ln -s ../$(OBJDIR)/sycallParser.o $(RELEASEDIR)/syscallParser-bleeding.o
	
clean:
	rm -rf $(OBJDIR)/*