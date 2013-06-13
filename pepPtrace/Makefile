CC=gcc -Wall -std=gnu99 -Werror

VER_FILE=version

LIBGLIB=`pkg-config --cflags --libs glib-2.0`

LIBPDP=../pdpConnector/release/pdpConnector-bleeding.o
LIBPROCFS=../utils/procfsUtils/release/procfsUtils-bleeding.o
LIBSYSCALLPARSER=../syscallParser/release/libsyscallParser-bleeding.a


EXECFILE=pepPtrace
EXECDIR=~/bin
BINDIR=bin
SRC=src
RELEASEDIR=release
INCLUDE=-I../include $(LIBGLIB)


all: pepPtrace

pepPtrace: mkdirs
	$(CC) $(SRC)/pep.c $(SRC)/tracee.c $(SRC)/traceeManager.c -o $(BINDIR)/$(EXECFILE) $(INCLUDE) $(LIBPDP) $(LIBPROCFS) $(LIBSYSCALLPARSER)
	ln -sf ../$(BINDIR)/$(EXECFILE) $(RELEASEDIR)/$(EXECFILE)-bleeding


release: all
	cp $(BINDIR)/$(EXECFILE) $(RELEASEDIR)/$(EXECFILE)-v`cat $(VER_FILE)`
	ln -sf $(EXECFILE)-v`cat $(VER_FILE)` $(RELEASEDIR)/$(EXECFILE)-latest
	oldV=`cat $(VER_FILE)`; newV=`expr $$oldV + 1`; echo "$$newV" > $(VER_FILE)
clean:
	rm -rf $(BINDIR)/

uninstall:
	rm -f $(EXECDIR)/$(EXECFILE)
	
install: uninstall
	cp $(BINDIR)/$(EXECFILE) $(EXECDIR)/
	chown root:root $(EXECDIR)/$(EXECFILE)
	chmod u+s $(EXECDIR)/$(EXECFILE)

run:
	PATH="$(EXECDIR):$(PATH)" $(EXECFILE)

mkdirs:
	mkdir -p $(BINDIR) $(RELEASEDIR)
