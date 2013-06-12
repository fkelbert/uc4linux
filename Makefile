all: rule_pepPtrace
	
rule_pepPtrace: rule_syscallParser rule_pdpConnector
	make -C pepPtrace all

rule_syscallParser:
	make -C syscallParser all

rule_pdpConnector: 
	make -C pdpConnector all

clean:
	make -C pepPtrace clean
	make -C syscallParser clean
	make -C pdpConnector clean
	
run:
	make -C pepPtrace run
	
install:
	make -C pepPtrace install
