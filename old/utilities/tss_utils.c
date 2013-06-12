/*
 * tss_utils.c
 *
 *      Author: Ricardo Neisse
 */

#include "tss_utils.h"

void printTspiResult(TSS_RESULT tResult) {
  if (tResult != TSS_SUCCESS) {
    plog(LOG_ERROR," TSS Error[%s]", Trspi_Error_String(tResult));
  } else {
    // log(LOG_INFO, " SUCCESS!");
  }
}

TSS_RESULT tss_connect(TSS_HCONTEXT *hContext, TSS_HTPM *hTpm) {

  plog(LOG_TRACE,"Creating TSPI Context");
  TSS_RESULT tResult = Tspi_Context_Create(hContext);
  if (tResult != TSS_SUCCESS) {    
    printTspiResult(tResult);
    return tResult;
  }  
  
  plog(LOG_TRACE,"Connecting to TSS in localhost");
  tResult = Tspi_Context_Connect(*hContext, NULL);
  if (tResult != TSS_SUCCESS) {    
    printTspiResult(tResult);
    return tResult;
  }
  
  plog(LOG_TRACE,"Getting software representation of the TPM");
  tResult = Tspi_Context_GetTpmObject(*hContext, hTpm);
  if (tResult != TSS_SUCCESS) {    
    printTspiResult(tResult);
    return tResult;
  }
  
  return TSS_SUCCESS; 
}


TSS_RESULT TPM_PcrExtend(TSS_HTPM hTpm, char* desc, int index, struct md *digest, struct pcr *pcrOut) {
  TSS_RESULT tResult = Tspi_TPM_PcrExtend(
      hTpm,
      index, digest->valueLen, &digest->value[0],
      NULL,
      &pcrOut->valueLen, &pcrOut->value
  );
  return tResult;
}

TSS_RESULT TPM_PcrRead(TSS_HTPM hTpm, struct pcr *pcrOut) {
  return Tspi_TPM_PcrRead(
      hTpm, 
      pcrOut->index, 
      &pcrOut->valueLen, 
      &pcrOut->value
  );
} 

void printPcrHex(struct pcr *pcrIn, int indent, int showBrackets) {
  printHex(pcrIn->value, pcrIn->valueLen, indent, showBrackets);
}

void printDigestHex(struct md *digest, int indent, int showBrackets) {
  printHex(digest->value, digest->valueLen, indent, showBrackets);
}

void printHex(BYTE *blob, UINT32 blobLen, int indent, int showBrackets) {
  if (!elog(LOG_DEBUG)) return;
  int i;
  if (indent) printf("  ");
  if (showBrackets) printf("[");
  for(i=0; i < blobLen; i++) {
    printf("%02x", blob[i]);
    // if((i+1) % 32 == 0) printf("\n");
  }
  if (showBrackets) printf("]");
  printf("\n");
}
