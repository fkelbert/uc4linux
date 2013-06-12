/*
 * tss_utils.h
 *
 *      Author: Ricardo Neisse
 */

#ifndef tss_utils_h
#define tss_utils_h

#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <trousers/tss.h>
#include <trousers/trousers.h>

#include "crypto_utils.h"
#include "logger.h"

/*
 * Struct to store a pcr value
 */
struct pcr {
  int index;
  BYTE *value;
  UINT32 valueLen;
};

TSS_RESULT TPM_PcrRead(TSS_HTPM hTpm, struct pcr *pcrOut);
TSS_RESULT TPM_PcrExtend(TSS_HTPM hTpm, char* desc, int index, struct md *digest, struct pcr *pcrOut);

void printTspiResult(TSS_RESULT tResult);
void printHex(BYTE *blob, UINT32 blobLen, int indent, int showBrackets);
void printPcrHex(struct pcr *pcrIn, int indent, int showBrackets);
void printDigestHex(struct md *digest, int indent, int showBrackets);

#endif

