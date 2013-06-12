/**
 * @file cryptoUtils.c
 * @author Ricardo Neisse
 *  
 */
#include "cryptoUtils.h"

/**libcrypto is not thread safe therefore only one thread
 * should use libcrypto at one point in time.
 * This is used by the xmlrpc server and the watch files thread
 */
pthread_mutex_t crypto_mutex = PTHREAD_MUTEX_INITIALIZER;

void calc_digest(unsigned char *digest, int len, void *data)
{
  pthread_mutex_lock(&crypto_mutex);
  SHA_CTX c;
  memset(digest, 0, sizeof *digest);
  SHA1_Init(&c);
  SHA1_Update(&c, data, len);
  SHA1_Final(digest, &c);
  pthread_mutex_unlock(&crypto_mutex);
}

int computeFileDigest(char *digestName, char *fileName, struct md *digest)
{
  pthread_mutex_lock(&crypto_mutex);
  FILE* fp;
  EVP_MD_CTX mdctx;
  const EVP_MD *evp_md;
  unsigned char *buf;
  int n;
  //nlog(LOG_TRACE, "Computing digest[%s][%s]", digestName, fileName);
  //nlog(LOG_TRACE, "Openning file %s", fileName);
  fp = fopen(fileName, "r");
  if(!fp)
  {
    //nlog(LOG_ERROR, "Can't open file %s", fileName);
    digest->valueLen = 20;
    for (n = 0; n < digest->valueLen; n++)
      digest->value[n] = 0;
    pthread_mutex_unlock(&crypto_mutex);
    return (EXIT_SUCCESS); //not quite a success, but we better get an empty hash
  }
  //nlog(LOG_TRACE, "Allocating buffer[%d]", BUFSIZE);
  if((buf = (unsigned char *) OPENSSL_malloc(BUFSIZE)) == NULL)
  {
    //nlog(LOG_ERROR, "can't allocate buffer[%d]", BUFSIZE);
    return (EXIT_FAILURE);
  }
  OpenSSL_add_all_digests();
  evp_md = EVP_get_digestbyname(digestName);
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, evp_md, NULL);
  //nlog(LOG_TRACE, "Reading file and updating digest");
  while ((n = fread(buf, 1, BUFSIZE, fp)))
  {
    EVP_DigestUpdate(&mdctx, buf, n);
  }
  EVP_DigestFinal_ex(&mdctx, &digest->value[0], &digest->valueLen);
  EVP_MD_CTX_cleanup(&mdctx);
  fclose(fp);
  pthread_mutex_unlock(&crypto_mutex);
  return (EXIT_SUCCESS);
}

int computeDataDigest(char *digestName, BYTE *data, int dataLength, struct md *digest)
{
  pthread_mutex_lock(&crypto_mutex);
  EVP_MD_CTX mdctx;
  const EVP_MD *evp_md;
  OpenSSL_add_all_digests();
  evp_md = EVP_get_digestbyname(digestName);
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, evp_md, NULL);
  EVP_DigestUpdate(&mdctx, data, dataLength);
  EVP_DigestFinal_ex(&mdctx, &digest->value[0], &digest->valueLen);
  EVP_MD_CTX_cleanup(&mdctx);
  pthread_mutex_unlock(&crypto_mutex);
  return (EXIT_SUCCESS);
}

