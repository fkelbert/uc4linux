CC=gcc -Wall -std=gnu99 -Werror

EXECFILE=ptrace-pep
EXECDIR=~/bin
OBJDIR=../../obj
BINDIR=../../bin

LIBGLIB=glib-2.0

all: ptrace-pep

ptrace-pep: pep.c pep.h helpers.o ptrace_interface_pdp.o traceeManager.o constants.h
	$(CC) pep.c $(OBJDIR)/*.o -o $(BINDIR)/$(EXECFILE) `pkg-config --cflags --libs $(LIBGLIB)`

helpers.o: helpers.c helpers.h
	$(CC) -c helpers.c -o $(OBJDIR)/helpers.o `pkg-config --cflags --libs $(LIBGLIB)`

tracee.o: tracee.c tracee.h
	$(CC) -c tracee.c -o $(OBJDIR)/tracee.o

ptrace_interface_pdp.o: ptrace_interface_pdp.c ptrace_interface_pdp.h
	$(CC) -c ptrace_interface_pdp.c -o $(OBJDIR)/ptrace_interface_pdp.o

traceeManager.o: traceeManager.c traceeManager.h helpers.o tracee.o
	$(CC) -c traceeManager.c -o $(OBJDIR)/traceeManager.o `pkg-config --cflags --libs $(LIBGLIB)`

clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*

uninstall:
	rm -f $(EXECDIR)/$(EXECFILE)

install: uninstall
	cp $(BINDIR)/$(EXECFILE) $(EXECDIR)/
	chown root:root $(EXECDIR)/$(EXECFILE)
	chmod u+s $(EXECDIR)/$(EXECFILE)

run:
	PATH="$(EXECDIR):$(PATH)" $(EXECFILE)
