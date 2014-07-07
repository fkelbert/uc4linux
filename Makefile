MAIN = pepPtrace
SUBS = syscallParser pdpConnector utils

all:
	for c in $(SUBS); do make -C $$c all; done
	make -C $(MAIN) all

release:
	for c in $(SUBS); do make -C $$c release; done
	make -C $(MAIN) release
	
	
clean:
	for c in $(SUBS); do make -C $$c clean; done
	make -C $(MAIN) clean
	
run:
	make -C $(MAIN) run
	
uninstall:
	make -C $(MAIN) uninstall

install: 
	make -C $(MAIN) install

	
