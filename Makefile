SUBDIRS = logger utilities event dataCont control_monitor past_monitor shell_wrapper

all: subdirs
subdirs:
	for i in $(SUBDIRS) ; do ( cd $$i ; make ) ; done
install:
	for i in $(SUBDIRS) ; do ( cd $$i ; make install); done
uninstall:
	for i in $(SUBDIRS) ; do ( cd $$i ; make uninstall); done
clean:
	for i in $(SUBDIRS) ; do ( cd $$i ; make clean) ; done
	
