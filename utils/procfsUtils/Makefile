CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
VER_FILE=version
INCLUDE=-I../../include


all: procfsUtils.o

procfsUtils.o: mkdirs
	$(CC) -c $(SRC)/procfsUtils.c -o $(OBJDIR)/procfsUtils.o $(LIBGLIB) $(INCLUDE)
	ln -sf ../$(OBJDIR)/procfsUtils.o $(RELEASEDIR)/procfsUtils-bleeding.o
	
release: all
	cp $(OBJDIR)/procfsUtils.o $(RELEASEDIR)/procfsUtils-v`cat $(VER_FILE)`.o
	ln -sf procfsUtils-v`cat $(VER_FILE)`.o $(RELEASEDIR)/procfsUtils-latest.o
	oldV=`cat $(VER_FILE)`; newV=`expr $$oldV + 1`; echo "$$newV" > $(VER_FILE)

clean:
	rm -rf $(OBJDIR)/

mkdirs:
	mkdir -p $(OBJDIR) $(RELEASEDIR)
