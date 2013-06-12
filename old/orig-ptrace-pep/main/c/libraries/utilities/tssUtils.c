/**
 * @file  tssUtils.c
 * @brief Implementation of auxiliary utilties for working with TPM
 *
 * @author Dominik Holling
**/

#include "tssUtils.h"


/// All functions can only be used mutually exclusive
pthread_mutex_t tss_mutex;
/// The context created on startup
TSS_HCONTEXT hContext;
/// The representation of the TPM in memory
TSS_HOBJECT hTPM;
/// The signing key to use for signature
TSS_HKEY hSigningKey = 0;

static TSS_UUID SRK_UUID = TSS_UUID_SRK; // SRK

/// The password of the storage root key
unsigned char *pSRK_password;

int tpm_connect(char *srk_password)
{
  TSS_RESULT ret;

  //initialize mutex for tss functions
  pthread_mutex_init(&tss_mutex, NULL);
  pthread_mutex_lock(&tss_mutex);

  ret=Tspi_Context_Create(&hContext);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Create context");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  ret=Tspi_Context_Connect(hContext, NULL);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Connect context");
    (void)Tspi_Context_Close(hContext);
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  ret=Tspi_Context_GetTpmObject(hContext, &hTPM);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Get TPM object");
    (void)Tspi_Context_Close(hContext);
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  pSRK_password=(unsigned char *)srk_password;

  pthread_mutex_unlock(&tss_mutex);
  return (tpm_selfTest());
}

int tpm_disconnect()
{
  TSS_RESULT ret;

  pthread_mutex_lock(&tss_mutex);

  ret=Tspi_Context_Close(hContext);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Close context");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  pthread_mutex_unlock(&tss_mutex);
  pthread_mutex_destroy(&tss_mutex);

  return (EXIT_SUCCESS);
}

int tpm_selfTest()
{
  TSS_RESULT ret;

  pthread_mutex_lock(&tss_mutex);

  ret=Tspi_TPM_SelfTestFull(hTPM);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Performing self test");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  pthread_mutex_unlock(&tss_mutex);

  return (EXIT_SUCCESS);
}

int tpm_loadKey()
{
  TSS_RESULT ret;
  TSS_HKEY hSRK;
  TSS_HPOLICY hSRKp;
  FILE * f;

  BYTE * blob;
  UINT32 bLen;

  pthread_mutex_lock(&tss_mutex);

  // Load Storage Root Key by UUID
  ret=Tspi_Context_LoadKeyByUUID(hContext, TSS_PS_TYPE_SYSTEM, SRK_UUID, &hSRK);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_Context_LoadKeyByUUID failed");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  // Set the correct SRK policy
  ret=Tspi_GetPolicyObject(hSRK, TSS_POLICY_USAGE, &hSRKp);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_GetPolicyObject failed");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  ret
      =Tspi_Policy_SetSecret(hSRKp, TSS_SECRET_MODE_PLAIN, strlen((char*)pSRK_password), pSRK_password);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_Policy_SetSecret failed");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  if((f=fopen(BLOB_FILE, "rb")) == NULL)
  {
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }
  fseek(f, 0, SEEK_END);
  bLen=ftell(f);
  fseek(f, 0, SEEK_SET);
  blob=malloc(bLen);
  if(fread(blob, 1, bLen, f) != bLen)
  {
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }
  fclose(f);

  ret=Tspi_Context_LoadKeyByBlob(hContext, hSRK, bLen, blob, &hSigningKey);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_Context_LoadKeyByUUID failed");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  free(blob);

  pthread_mutex_unlock(&tss_mutex);

  return (EXIT_SUCCESS);
}

int tpm_getSignedHash(UINT32 ulHashValueLength, BYTE* rgbHashValue, UINT32* sLen, BYTE** signature)
{
  TSS_RESULT ret;
  TSS_HHASH hHash;

  pthread_mutex_lock(&tss_mutex);

  if(hSigningKey == 0)
  {
    //nlog(LOG_FATAL, "Signing key not set");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  /* open valid hash object */
  ret=Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_HASH, TSS_HASH_SHA1, &hHash);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Create hash object");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  /* set hash value and get valid signature */
  ret=Tspi_Hash_SetHashValue(hHash, ulHashValueLength, rgbHashValue);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Set hash value");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  ret=Tspi_Hash_Sign(hHash, hSigningKey, sLen, signature);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Sign hash");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  /* verify signature */
  ret=Tspi_Hash_VerifySignature(hHash, hSigningKey, *sLen, *signature);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Verify hash signature");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  pthread_mutex_unlock(&tss_mutex);

  return (EXIT_SUCCESS);
}

int tpm_getPCR(UINT32 ulPcrIndex, UINT32* vLen, BYTE** value)
{
  TSS_RESULT ret;
  BYTE * hValue;
  ret=Tspi_TPM_PcrRead(hTPM, ulPcrIndex, vLen, &hValue);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_TPM_PcrRead failed");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }
  *value=(unsigned char *)malloc(*vLen * sizeof(unsigned char));
  memcpy(*value, hValue, *vLen);
  Tspi_Context_FreeMemory(hContext, hValue);
  return (EXIT_SUCCESS);

}

int tpm_extendPCR(UINT32 ulPcrIndex, UINT32 hLen, BYTE * hash, UINT32* vLen, BYTE** vValue)
{
  TSS_RESULT ret;
  BYTE * tValue;

  log_trace("%s", __func__);
  pthread_mutex_lock(&tss_mutex);

  ret=Tspi_TPM_PcrExtend(hTPM, ulPcrIndex, hLen, hash, NULL, vLen, &tValue);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_TPM_PcrExtend failed");
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  *vValue=(unsigned char *)malloc(*vLen * sizeof(unsigned char));
  memcpy(*vValue, tValue, *vLen);

  Tspi_Context_FreeMemory(hContext, tValue);

  pthread_mutex_unlock(&tss_mutex);

  return (EXIT_SUCCESS);

}

int tpm_getQuote(UINT32 *dLen, BYTE ** Data, UINT32* vLen, BYTE** ValidationData)
{
  TSS_RESULT ret;
  TSS_HPCRS hPCRs;
  TSS_VALIDATION val;
  UINT32 i;
  BYTE *nonce;

  pthread_mutex_lock(&tss_mutex);

  ret=Tspi_Context_CreateObject(hContext, TSS_OBJECT_TYPE_PCRS, 0, &hPCRs);
  if(ret)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Sign hash");
    printf("ERROR: %s", Trspi_Error_String(ret));
    fflush(NULL);
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  for(i=0; i < 24; i++)
  {
    ret=Tspi_PcrComposite_SelectPcrIndex(hPCRs, i);
    if(ret)
    {
      printf("ERROR: %s", Trspi_Error_String(ret));
      fflush(NULL);
      pthread_mutex_unlock(&tss_mutex);
      return (EXIT_FAILURE);
    }
  }

  //generate a nonce
  ret=Tspi_TPM_GetRandom(hTPM, TPM_SHA1_160_HASH_LEN, &nonce);
  if(ret != TSS_SUCCESS)
  {
    //log_tpm(LOG_FATAL, ret, "%s", "Tspi_TPM_GetRandom failed");
    return (EXIT_FAILURE);
  }

  val.rgbExternalData=nonce;
  val.ulExternalDataLength=TPM_SHA1_160_HASH_LEN;

  ret=Tspi_TPM_Quote(hTPM, hSigningKey, hPCRs, &val);
  Tspi_Context_FreeMemory(hContext, val.rgbExternalData);
  if(ret==TSS_SUCCESS)
  {

    *Data=malloc((val.ulDataLength) * sizeof(BYTE));
    memcpy(*Data, val.rgbData, val.ulDataLength);
    *dLen=val.ulDataLength;
    Tspi_Context_FreeMemory(hContext, val.rgbData);

    *ValidationData=malloc(val.ulValidationDataLength * sizeof(BYTE));
    memcpy(*ValidationData, val.rgbValidationData, val.ulValidationDataLength);
    *vLen=val.ulValidationDataLength;
    Tspi_Context_FreeMemory(hContext, val.rgbData);

  }
  else
  {
    *dLen=0;
    *vLen=0;
    printf("ERROR: %s", Trspi_Error_String(ret));
    fflush(NULL);
    pthread_mutex_unlock(&tss_mutex);
    return (EXIT_FAILURE);
  }

  pthread_mutex_unlock(&tss_mutex);
  return (EXIT_SUCCESS);
}
