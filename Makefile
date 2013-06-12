all: rule_pepPtrace
	
rule_pepPtrace: rule_syscallParser rule_pdpConnector rule_utils
	make -C pepPtrace all

rule_syscallParser:
	make -C syscallParser all

rule_pdpConnector: 
	make -C pdpConnector all

rule_utils:
	make -C utils all

clean:
	make -C pepPtrace clean
	make -C syscallParser clean
	make -C pdpConnector clean
	make -C utils clean
	
run:
	make -C pepPtrace run
	
install:
	make -C pepPtrace install
