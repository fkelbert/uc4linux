SUBS = procfsUtils strUtils

all:
	for i in $(SUBS); do make -C $$i all; done

release:
	for i in $(SUBS); do make -C $$i release; done

clean:
	for i in $(SUBS); do make -C $$i clean; done
