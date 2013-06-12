/**
 * @file as.h
 * @brief Attestation service; used in Master project
 */

#ifndef AS_H
#define	AS_H

#include <string.h>

#include "tss_utils.h"
#include "crypto_utils.h"

// JNI connection header
#include "AttestationServiceNative.h"

#define SRK_PASS  "root"
#define CERT_FILE "cert.pem"
#define BLOB_FILE "key.blob"
#define PCR 12

void  bytes_to_string(unsigned char *string, unsigned char *bytes, uint32_t len);
char* getPCRQuote();
char *getSignature(struct md* digest);


#endif
