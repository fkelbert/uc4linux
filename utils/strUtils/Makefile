CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
INCLUDE=-I../../include


all: strUtils.o 

strUtils.o: 
	$(CC) -c $(SRC)/strUtils.c -o $(OBJDIR)/strUtils.o $(LIBGLIB) $(INCLUDE)
	ln -sf ../$(OBJDIR)/strUtils.o $(RELEASEDIR)/strUtils-bleeding.o
	
clean:
	rm -rf $(OBJDIR)/*
