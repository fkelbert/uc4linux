/**
 * @file  pdpOutPIP.h
 * @brief Header for PDP communication for PXP execution
 *
 * Provide information about involved communication of the PDP for communicating with an
 * associated PIP. Depending on the configuration the PIP is involved via native method call,
 * JNI interface or via socket.
 *
 * @remark the methods representationRefinesData and initialRepresentation are only used for OpenBSD-PIP in combination
 *         with the systrace-PEP and should not be used. They will be replaced with pipEval and pipInit!
 *
 * @author cornelius moucha
 **/

#ifndef PDPOUTPIP_H_
#define PDPOUTPIP_H_

#include "pdpCommInterfaces.h"
#include "pdpJNItypes.h"

unsigned int representationRefinesData(char *rep, char *dataID);
char*        initialRepresentation(char *cont, char *qod);

int          pipEval(char *method, char *params);
char*        pipInit(char *method, char *params);

#endif /* PDPOUTPIP_H_ */
