CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
LIBGLIB=`pkg-config --cflags --libs glib-2.0`

INCLUDE=-I../include


all: pdpConnector.o

pdpConnector.o: 
	$(CC) -c $(SRC)/pdpConnector.c -o $(OBJDIR)/pdpConnector.o $(LIBGLIB) $(INCLUDE)
	ln -sf ../$(OBJDIR)/pdpConnector.o $(RELEASEDIR)/pdpConnector-bleeding.o
	
clean:
	rm -rf $(OBJDIR)/*
