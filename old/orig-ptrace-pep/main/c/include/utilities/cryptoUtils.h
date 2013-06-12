/**
 * @file cryptoUtils.h
 *
 * Contained is everything that is concerned with cryptography and hashing.
 * @author Ricardo Neisse
 */
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <trousers/tss.h>
#include <trousers/trousers.h>
#include <pthread.h>
#include <string.h>
#include "base.h"


#ifndef MESSAGE_DIGEST_STRUCT
#define MESSAGE_DIGEST_STRUCT

/// Struct to store a message digest
struct md
{
  /// The message digest
  unsigned char value[EVP_MAX_MD_SIZE];
  /// The length of the message digest
  uint32_t valueLen;
};

#endif

/**
 * Buffer size to use when calculating hash of a file
 * @see computeFileDigest
 */
#define BUFSIZE 2048


/**
 * Calculate the SHA-1 digest of a string (char *)
 * @param digest Resulting digest of data (not allocated)
 * @param len Length of the given data
 * @param data Data to be hashed
 */
void calc_digest(unsigned char *digest, int len, void *data);

/**
 * Calculate the digest of a file
 * @param digestName Name of the hashing algorithm to be used (e.g. SHA1)
 * @param fileName Name of the file to be hashed
 * @param digest Resulting digest of data (not allocated)
 */
int computeFileDigest(char *digestName, char *fileName, struct md *digest);

/**
 * Calculate the digest of a string (char *)
 * @param digestName Name of the hashing algorithm to be used (e.g. SHA1)
 * @param data Data to be hashed
 * @param dataLength Length of the given data
 * @param digest Resulting digest of data (not allocated)
 */
int computeDataDigest(char *digestName, BYTE *data, int dataLength, struct md *digest);
