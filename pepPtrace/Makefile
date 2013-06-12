CC=gcc -Wall -std=gnu99 -Werror

LIBGLIB=`pkg-config --cflags --libs glib-2.0`
LIBPDP=../pdpConnector/release/pdpConnector-bleeding.o
LIBPROCFS=../utils/procfsUtils/release/procfsUtils-bleeding.o
LIBSYSCALLPARSER=../syscallParser/release/libsyscallParser-bleeding.a


EXECFILE=pepPtrace
EXECDIR=~/bin
BINDIR=bin
SRC=src
RELEASEDIR=release
INCLUDE=-I../include $(LIBGLIB) $(LIBPROCFS) $(LIBPDP) $(LIBSYSCALLPARSER)


all: pepPtrace

pepPtrace: 
	$(CC) $(SRC)/pep.c $(SRC)/tracee.c $(SRC)/traceeManager.c -o $(BINDIR)/$(EXECFILE)  $(INCLUDE) 
	ln -sf ../$(BINDIR)/$(EXECFILE) $(RELEASEDIR)/$(EXECFILE)-bleeding
	
clean:
	rm -rf $(BINDIR)/*

uninstall:
	rm -f $(EXECDIR)/$(EXECFILE)
	
install: uninstall
	cp $(BINDIR)/$(EXECFILE) $(EXECDIR)/
	chown root:root $(EXECDIR)/$(EXECFILE)
	chmod u+s $(EXECDIR)/$(EXECFILE)

run:
	PATH="$(EXECDIR):$(PATH)" $(EXECFILE)