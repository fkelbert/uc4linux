CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
VER_FILE=version

INCLUDE=-I../include


all: pdpConnector.o

pdpConnector.o: mkdirs
	$(CC) -c $(SRC)/pdpConnector.c -o $(OBJDIR)/pdpConnector.o $(LIBGLIB) $(INCLUDE)
	ln -sf ../$(OBJDIR)/pdpConnector.o $(RELEASEDIR)/pdpConnector-bleeding.o
	
release: all
	cp $(OBJDIR)/pdpConnector.o $(RELEASEDIR)/pdpConnector-v`cat $(VER_FILE)`.o
	ln -sf pdpConnector-v`cat $(VER_FILE)`.o $(RELEASEDIR)/pdpConnector-latest.o
	oldV=`cat $(VER_FILE)`; newV=`expr $$oldV + 1`; echo "$$newV" > $(VER_FILE)
	

clean:
	rm -rf $(OBJDIR)/

mkdirs:
	mkdir -p $(OBJDIR) $(RELEASEDIR)
