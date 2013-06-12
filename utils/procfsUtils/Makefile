CC=gcc -Wall -std=gnu99 -Werror
OBJDIR=obj
SRC=src
RELEASEDIR=release
INCLUDE=-I../../include


all: procfsUtils.o

procfsUtils.o: 
	$(CC) -c $(SRC)/procfsUtils.c -o $(OBJDIR)/procfsUtils.o $(LIBGLIB) $(INCLUDE)
	ln -sf ../$(OBJDIR)/procfsUtils.o $(RELEASEDIR)/procfsUtils-bleeding.o
	
clean:
	rm -rf $(OBJDIR)/*
