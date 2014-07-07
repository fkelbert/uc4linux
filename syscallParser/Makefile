CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
VER_FILE=version
GLIB=`pkg-config --cflags --libs glib-2.0`

INCL=-I../include


all: libsyscallParser

libsyscallParser: syscallParser.o event.o param.o syscallIntercepts.o syscallNames.o mkdirs
	ar rs $(OBJDIR)/libsyscallParser.a \
						$(OBJDIR)/syscallParser.o \
						$(OBJDIR)/event.o \
						$(OBJDIR)/param.o \
						$(OBJDIR)/syscallIntercepts.o \
						$(OBJDIR)/syscallNames.o
	ln -sf ../$(OBJDIR)/libsyscallParser.a $(RELEASEDIR)/libsyscallParser-bleeding.a

release: all
	cp $(OBJDIR)/libsyscallParser.a $(RELEASEDIR)/libsyscallParser-v`cat $(VER_FILE)`.a
	ln -sf libsyscallParser-v`cat $(VER_FILE)`.a $(RELEASEDIR)/libsyscallParser-latest.a
	oldV=`cat $(VER_FILE)`; newV=`expr $$oldV + 1`; echo "$$newV" > $(VER_FILE)

syscallParser.o: mkdirs
	$(CC) -c $(SRC)/syscallParser.c -o $(OBJDIR)/syscallParser.o $(INCL) $(GLIB)
	
event.o: mkdirs
	$(CC) -c $(SRC)/event.c -o $(OBJDIR)/event.o $(INCL) $(GLIB)

param.o: mkdirs
	$(CC) -c $(SRC)/param.c -o $(OBJDIR)/param.o $(INCL)
	
syscallNames.o: mkdirs
	$(CC) -c $(SRC)/syscallNames.c -o $(OBJDIR)/syscallNames.o $(INCL)

syscallIntercepts.o: mkdirs
	$(CC) -c $(SRC)/syscallIntercepts.c -o $(OBJDIR)/syscallIntercepts.o $(INCL)

mkdirs:
	mkdir -p $(RELEASEDIR) $(OBJDIR)

clean:
	rm -rf $(OBJDIR)/
