#SUBDIRS = logger utilities trust_manager attestation_service event dataCont control_monitor past_monitor shell_wrapper syscall_handler 
SUBDIRS = logger utilities event dataCont control_monitor past_monitor shell_wrapper syscall_handler 

all: subdirs
subdirs:
	for i in $(SUBDIRS) ; do ( cd $$i ; make ) ; done
install:
	for i in $(SUBDIRS) ; do ( cd $$i ; make install); done
uninstall:
	for i in $(SUBDIRS) ; do ( cd $$i ; make uninstall); done
clean:
	for i in $(SUBDIRS) ; do ( cd $$i ; make clean) ; done
	
