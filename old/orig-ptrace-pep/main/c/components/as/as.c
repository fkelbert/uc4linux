/**
 * @file as.c
 * @brief Attestation service; used in Master project
 */

#include "as.h"
#include "log_as_c.h"

void bytes_to_string(unsigned char *string, unsigned char *bytes, uint32_t len)
{
  int i;
  int pointer=0;
  for(i=0; i < len; i++)
    pointer+=sprintf((char*)string + pointer, "%02x", bytes[i]);
}

JNIEXPORT void JNICALL Java_de_fhg_iese_esf_as_AttestationServiceNative_start(JNIEnv *env, jobject jobj)
{
  tpm_connect(SRK_PASS);
  tpm_loadKey();
  // registerAS(TM_URL);
  // void register_as() {
  // axiom_node_t * tmCertNode = getPrivacyCertificate(lenv);
  log_info("Native attestation service started");
}

JNIEXPORT void JNICALL Java_de_fhg_iese_esf_as_AttestationServiceNative_stop(JNIEnv *env, jobject jobj)
{ // @todo not implemented
  log_info("Native attestation service stopped");
}

//(JNIEnv *env, jobject jobj, jobjectArray jobjarray)
JNIEXPORT jstring JNICALL Java_de_fhg_iese_esf_as_AttestationServiceNative_attestFile(JNIEnv *env, jobject jobj,
                                                        jstring path, jstring type, jstring args, jint pcrToExtened)
{
//  struct md *digest;
//  unsigned char *hValue;
//  UINT32 vLen;
//  BYTE* vValue;
//  char *path="";
//
//  log_trace("%s - received attestFile request",__func__);
//  //*(env)->GetObjectArrayElement()
//
//  // why this is necessary?? cert is not used in this method!
//  //char* cert=getPrivacyCertificate();
//  digest=(struct md*)malloc(sizeof(struct md));
//  computeFileDigest("SHA1", path, digest);
//  hValue=(unsigned char *)malloc((2 * digest->valueLen + 1) * sizeof(unsigned char));
//  bytes_to_string(hValue, digest->value, digest->valueLen);
//  char *signature=getSignature(digest);
//  tpm_extendPCR(PCR, digest->valueLen, digest->value, &vLen, &vValue);
//  //log_trace("freeing vValue=[%p]",vValue);
//  // commented -> freeing produce segfault!
//  //free(vValue);
//  free(digest);
//  char* quote=getPCRQuote();
//  return (*env)->NewStringUTF(env, quote);
  return (*env)->NewStringUTF(env, "attestFileResponse");
}

JNIEXPORT jstring JNICALL Java_de_fhg_iese_esf_as_AttestationServiceNative_getAikCertificate(JNIEnv *env, jobject jobj)
{
//  FILE *fp;
//  long len;
//  char *buf;
//  fp=fopen(CERT_FILE, "rb");
//  fseek(fp, 0, SEEK_END);
//  len=ftell(fp);
//  fseek(fp, 0, SEEK_SET);
//  buf=(char *)calloc(len + 1, sizeof(char));
//  fread(buf, len, 1, fp);
//  fclose(fp);
//  return (*env)->NewStringUTF(env, buf);
  return (*env)->NewStringUTF(env, "getAikCertificateResponse");
}

//char* getPCRQuote()
JNIEXPORT jstring JNICALL Java_de_fhg_iese_esf_as_AttestationServiceNative_getQuotedPCRs(JNIEnv *env, jobject jobj, jstring nonce)
{
//  unsigned char * Data;
//  uint32_t dLen;
//
//  unsigned char * ValidationData;
//  uint32_t vLen;
//
//  unsigned char * Message;
//  uint32_t mLen;
//
//  unsigned char prefix[9]={0x00, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0xe0}; //00 03 ff ff ff 00 00 01 e0
//  uint32_t pLen=9;
//
//  unsigned char *elemText;
//
//  int quote_failed=tpm_getQuote(&dLen, &Data, &vLen, &ValidationData);
//
//  if(quote_failed == EXIT_SUCCESS)
//  {
//    mLen=(pLen + TPM_SHA1_160_HASH_LEN * 24);
//    Message=(unsigned char *)malloc(mLen * sizeof(BYTE));
//    memcpy(Message, prefix, pLen);
//    int i;
//    for(i=0; i < 24; i++)
//    {
//      BYTE* value;
//      UINT32 vLen;
//      tpm_getPCR(i, &vLen, &value);
//      memcpy(Message + pLen + i * TPM_SHA1_160_HASH_LEN, value, TPM_SHA1_160_HASH_LEN);
//      free(value);
//    }
//    elemText=malloc((mLen * 2 + dLen * 2 + vLen * 2 + 1 + 1 + 1) * sizeof(unsigned char));
//    bytes_to_string(elemText, Message, mLen);
//    memcpy(elemText + 2 * mLen, "\n", 1);
//
//    bytes_to_string(elemText + +2 * mLen + 1, Data, dLen);
//    memcpy(elemText + 2 * mLen + 1 + 2 * dLen, "\n", 1);
//
//    bytes_to_string(elemText + 2 * mLen + 1 + 2 * dLen + 1, ValidationData, vLen);
//
//    strcpy((char *)(elemText + 2 * mLen + 1 + 2 * dLen + 1 + vLen * 2), "");
//
//    free(Data);
//    free(ValidationData);
//    free(Message);
//    return elemText;
//  }
//  return NULL;
  return (*env)->NewStringUTF(env, "getQuotedPCRresponse");
}

char *getSignature(struct md* digest)
{
  unsigned char *sign;
  uint32_t sLen;
  unsigned char *sValue;

  int hash_failed=tpm_getSignedHash(digest->valueLen, digest->value, &sLen, &sign);
  if(hash_failed == EXIT_SUCCESS)
  {
    sValue=(unsigned char *)malloc((2 * sLen + 1) * sizeof(unsigned char));
    bytes_to_string(sValue, sign, sLen);
    free(sign);
    log_trace("%s - result: %s", sValue);
    return (char *)sValue;
  }
  else log_trace("%s - result: %d",__func__, hash_failed);
  return NULL;
}



