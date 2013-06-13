CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
VER_FILE=version
INCLUDE=-I../../include


all: strUtils.o 

strUtils.o: mkdirs
	$(CC) -c $(SRC)/strUtils.c -o $(OBJDIR)/strUtils.o $(LIBGLIB) $(INCLUDE)
	ln -sf ../$(OBJDIR)/strUtils.o $(RELEASEDIR)/strUtils-bleeding.o
	
release: all
	cp $(OBJDIR)/strUtils.o $(RELEASEDIR)/strUtils-v`cat $(VER_FILE)`.o
	ln -sf strUtils-v`cat $(VER_FILE)`.o $(RELEASEDIR)/strUtils-latest.o
	oldV=`cat $(VER_FILE)`; newV=`expr $$oldV + 1`; echo "$$newV" > $(VER_FILE)

clean:
	rm -rf $(OBJDIR)/

mkdirs:
	mkdir -p $(OBJDIR) $(RELEASEDIR)
