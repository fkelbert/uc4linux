/*
 * Author: Ricardo Neisse
 */
 
#include <trousers/tss.h>
#include <trousers/trousers.h>

#include "logger.h"
#include "str_utils.h"
#include "time_utils.h"
#include "hash_table.h"
#include "crypto_utils.h"
#include "tss_utils.h"
#include "xml_utils.h"
#include "soup_utils.h"

void verifyShells(char *shellPath, int *shellsAreOk);

void verifyShells(char *shellPath, int *shellsAreOk) {
  struct passwd *pwd;  
  setpassent(0); // go to beginning of passwd file
  *shellsAreOk = 0;
  int count=0;
  while(pwd = getpwent()) {
    plog(LOG_TRACE,"Checking user entry [%s][%s]",pwd->pw_name,pwd->pw_shell);
    if ( strcmp(pwd->pw_name,"root")==0 || 
         strcmp(pwd->pw_shell,"/sbin/nologin")==0 ||
         strcmp(pwd->pw_shell,"/usr/libexec/uucp/uucico")==0
    ) continue;
    if (strcmp(pwd->pw_shell,shellPath)!=0) {
      count++;
      plog(LOG_ERROR,"Non-compliant shell configuration: [%s][%s]",pwd->pw_name,pwd->pw_shell);
    }
  }
  if (count==0) {
    *shellsAreOk = 1;
  } else {
    *shellsAreOk = 0;
  }
}

int main (int argc, char* argv) {

  TSS_RESULT tResult;
  TSS_HCONTEXT hContext;
  TSS_HTPM hTpm;
  int i;
 
  plog(LOG_INFO,"Connecting to TPM/TSS");
  tResult = tss_connect(&hContext, &hTpm);  
  if (tResult!=TSS_SUCCESS) {
    plog(LOG_FATAL,"TPM/TSS API is not available, aborting...");
    return;
  }

  int shellsAreOk=0;
  char *shellPath = "/usr/local/bin/shell_wrapper";
  plog(LOG_INFO,"Checking shell configuration [%s]",shellPath);
  verifyShells(shellPath, &shellsAreOk);
  if (!shellsAreOk) {
    plog(LOG_ERROR,"Shell configuration error");
  }  

    
  char *fileName = "/bsd";
  char *digestName = "sha1";
  int index = 2;
  
  struct md digest;
  struct pcr pcrOut;
  if (computeFileDigest(digestName,fileName,&digest)) {
    // printDigestHex(&digest, 1, 1);
    plog(LOG_INFO, "Extending PCR[%d] with [%s][%s]",index, digestName, fileName);
    tResult = TPM_PcrExtend(hTpm, "", index, &digest, &pcrOut);
    printTspiResult(tResult);
  }


  fileName = "/home/workspace/OSLFramework/control_monitor/mechanisms.xml";
  index = 3;
  if (computeFileDigest(digestName,fileName,&digest)) {
    // printDigestHex(&digest, 1, 1);
    plog(LOG_INFO, "Extending PCR[%d] with [%s][%s]",index, digestName, fileName);
    tResult = TPM_PcrExtend(hTpm, "", index, &digest, &pcrOut);
    printTspiResult(tResult);
  }

  fileName = "/home/workspace/OSLFramework/control_monitor/control_monitor_service";
  index = 4;
  if (computeFileDigest(digestName,fileName,&digest)) {
    // printDigestHex(&digest, 1, 1);
    plog(LOG_INFO, "Extending PCR[%d] with [%s][%s]",index, digestName, fileName);
    tResult = TPM_PcrExtend(hTpm, "", index, &digest, &pcrOut);
    printTspiResult(tResult);
  }

  fileName = "/etc/rc.local";
  index = 5;
  if (computeFileDigest(digestName,fileName,&digest)) {
    // printDigestHex(&digest, 1, 1);
    plog(LOG_INFO, "Extending PCR[%d] with [%s][%s]",index, digestName, fileName);
    tResult = TPM_PcrExtend(hTpm, "", index, &digest, &pcrOut);
    printTspiResult(tResult);
  }

  plog(LOG_DEBUG, "Reading TPM.PCR[] values");
  // There are 16 PCR registers in the TPM
  if (elog(LOG_INFO)) {
    for (i=0; i<16; i++) {
      printf("  [%02d]",i);
      pcrOut.index = i;
      tResult = TPM_PcrRead(hTpm, &pcrOut); 
      if (tResult == TSS_SUCCESS) {
        printPcrHex(&pcrOut, 0, 1);
      } else {
        printTspiResult(tResult);
      } 
    }
  }
  
}


