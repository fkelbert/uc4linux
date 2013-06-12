#pragma once
#include <string.h>
#include "shm_utils.h"

// SOAP v2 interface
#include "soap_utils.h"

shm_ptr pdppepCommunication;

unsigned int init_pep();
key_t prepareSharedMemory();

void pep_startSharedMemoryThread();
void pep_shthread();

axiom_node_ptr pep_soap_handler(unsigned int, axiom_node_ptr, axiom_msg_ctx_ptr);

SOAP_OPERATIONS(pep_soapHandler) = {&pep_soap_handler};
GENERATE_SOAP_SERVICE(pepSoapService, init_pep, "/home/raindrop/hiwi/prog/FhG-ESF/src/main/c/build/applications/tests/pep.wsdl", pep_soapHandler)
