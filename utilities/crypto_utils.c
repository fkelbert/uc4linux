/*
 * crypto_utils.c
 *
 *      Author: Ricardo Neisse
 */

#include "crypto_utils.h"

void calc_digest(u_int8_t *digest, int len, void *data) {
  SHA_CTX c;
  memset(digest, 0, sizeof *digest);
  SHA1_Init(&c);
  SHA1_Update(&c, data, len);
  SHA1_Final(digest, &c);
}

int computeFileDigest(char *digestName, char *fileName, struct md *digest) {
  FILE* fp;
  EVP_MD_CTX mdctx;
  const EVP_MD *evp_md;
  unsigned char *buf;
  int n;
  plog(LOG_TRACE, "Computing digest[%s][%s]",digestName, fileName);
  plog(LOG_TRACE, "Openning file %s", fileName);
  fp = fopen(fileName, "r");
  if (!fp) {
    plog(LOG_ERROR, "Can't open file %s", fileName);
    return 0;
  }
  plog(LOG_TRACE, "Allocating buffer[%d]", BUFSIZE);
  if ((buf=(unsigned char *)OPENSSL_malloc(BUFSIZE)) == NULL) {
    plog(LOG_ERROR, "can't allocate buffer[%d]", BUFSIZE);
    return 0;
  } 
  OpenSSL_add_all_digests();
  evp_md = EVP_get_digestbyname(digestName);
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, evp_md, NULL);
  plog(LOG_TRACE,"Reading file and updating digest");
  while (n = fread(buf, 1, BUFSIZE, fp)) {
    EVP_DigestUpdate(&mdctx, buf,n);
  }
  EVP_DigestFinal_ex(&mdctx, &digest->value[0], &digest->valueLen);
  EVP_MD_CTX_cleanup(&mdctx);
  fclose(fp);
}

int computeDataDigest(char *digestName, BYTE *data, int dataLength, struct md *digest) {
  EVP_MD_CTX mdctx;
  const EVP_MD *evp_md;
  int n;
  OpenSSL_add_all_digests();
  evp_md = EVP_get_digestbyname(digestName);
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, evp_md, NULL);
  EVP_DigestUpdate(&mdctx, data, dataLength);
  EVP_DigestFinal_ex(&mdctx, &digest->value[0], &digest->valueLen);
  EVP_MD_CTX_cleanup(&mdctx);
}

