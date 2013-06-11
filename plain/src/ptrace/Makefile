CC=gcc -Wall -std=gnu99 -Werror

EXECFILE=ptrace-pep
EXECDIR=~/bin
OBJDIR=../../obj
BINDIR=../../bin

LIBGLIB=`pkg-config --cflags --libs glib-2.0`

all: ptrace-pep

ptrace-pep: pep.c pep.h helpers.o ptrace_interface_pdp.o traceeManager.o event.o param.o constants.h syscallParser.o syscalls.o
	$(CC) pep.c $(OBJDIR)/*.o -o $(BINDIR)/$(EXECFILE) $(LIBGLIB)

helpers.o: helpers.c helpers.h
	$(CC) -c helpers.c -o $(OBJDIR)/helpers.o $(LIBGLIB)

tracee.o: tracee.c tracee.h
	$(CC) -c tracee.c -o $(OBJDIR)/tracee.o

ptrace_interface_pdp.o: ptrace_interface_pdp.c ptrace_interface_pdp.h
	$(CC) -c ptrace_interface_pdp.c -o $(OBJDIR)/ptrace_interface_pdp.o

traceeManager.o: traceeManager.c traceeManager.h helpers.o tracee.o
	$(CC) -c traceeManager.c -o $(OBJDIR)/traceeManager.o $(LIBGLIB)

param.o: param.c param.h
	$(CC) -c param.c -o $(OBJDIR)/param.o

event.o: event.c event.h 
	$(CC) -c event.c -o $(OBJDIR)/event.o $(LIBGLIB)

syscallParser.o: syscallParser.c syscallParser.h
	$(CC) -c syscallParser.c -o $(OBJDIR)/syscallParser.o $(LIBGLIB)

syscalls.o: syscalls.c syscalls.h
	$(CC) -c syscalls.c -o $(OBJDIR)/syscalls.o $(LIBGLIB)

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
