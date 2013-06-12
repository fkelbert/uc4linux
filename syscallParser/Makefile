CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
LIBGLIB=`pkg-config --cflags --libs glib-2.0`

INCLUDE=-I../include $(LIBGLIB)


all: libsyscallParser

libsyscallParser: syscallParser.o event.o param.o syscallIntercepts.o syscallNames.o
	ar rs $(OBJDIR)/libsyscallParser.a \
						$(OBJDIR)/syscallParser.o \
						$(OBJDIR)/event.o \
						$(OBJDIR)/param.o \
						$(OBJDIR)/syscallIntercepts.o \
						$(OBJDIR)/syscallNames.o
	ln -sf ../$(OBJDIR)/libsyscallParser.a $(RELEASEDIR)/libsyscallParser-bleeding.a


syscallParser.o:  
	$(CC) -c $(SRC)/syscallParser.c -o $(OBJDIR)/syscallParser.o $(INCLUDE)
	
event.o: 
	$(CC) -c $(SRC)/event.c -o $(OBJDIR)/event.o $(INCLUDE)

param.o: 
	$(CC) -c $(SRC)/param.c -o $(OBJDIR)/param.o $(INCLUDE)
	
syscallNames.o:
	$(CC) -c $(SRC)/syscallNames.c -o $(OBJDIR)/syscallNames.o $(INCLUDE)

syscallIntercepts.o:	
	$(CC) -c $(SRC)/syscallIntercepts.c -o $(OBJDIR)/syscallIntercepts.o $(INCLUDE)
	
clean:
	rm -rf $(OBJDIR)/*
