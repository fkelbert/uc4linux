/*
 * crypto_utils.h
 *
 *      Author: Ricardo Neisse
 */

#ifndef _crypto_utils_h
#define _crypto_utils_h

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

#include "logger.h"

// Struct to store a message digest
#ifndef MESSAGE_DIGEST_STRUCT
#define MESSAGE_DIGEST_STRUCT

struct md {
  unsigned char value[EVP_MAX_MD_SIZE];
  UINT32 valueLen;
};

#endif

#define BUFSIZE 2048

void calc_digest(u_int8_t *digest, int len, void *data);

int computeFileDigest(char *digestName, char *fileName, struct md *digest);

int computeDataDigest(char *digestName, BYTE *data, int dataLength, struct md *digest);

#endif
