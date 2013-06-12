#include "pep.h"
#include "log_pep_c.h"

#define SHMSZ     27

unsigned int init_pep()
{
  log_trace("PEP initialization:\n");

  return R_SUCCESS;
}

void pep_startSharedMemoryThread()
{
  pthread_t shmthread;
  pthread_create(&shmthread,NULL,(void*)pep_shthread, NULL);
}

void pep_shthread()
{
  log_debug("Starting waiting thread for shared memory changes");
  while(1)
  {
    //log_trace("PEP sh: %c %c %c %c", *(pdppepCommunication->shmSegment), *(pdppepCommunication->shmSegment+1),
    //                                 *(pdppepCommunication->shmSegment+2), *(pdppepCommunication->shmSegment+3));
    rlognl(LOG_PEP_C_OUTPUT, LOG_TRACE, "-");
    if( *(pdppepCommunication->shmSegment)!='b')
    {
      log_trace("%s - shared memory changed: %c", __func__, pdppepCommunication->shmSegment[0]);
      *(pdppepCommunication->shmSegment)='b';
    }
    sleep(2);
  }
}

key_t prepareSharedMemory()
{
  pdppepCommunication = malloc(sizeof(shm_t));
  pdppepCommunication->key=0x9983;
  pdppepCommunication->sharedMemoryID=0;
  pdppepCommunication->shmSegment=NULL;

  if((pdppepCommunication->sharedMemoryID = shmget(pdppepCommunication->key, 4, IPC_CREAT | 0666)) < 0)
    {log_error("%s - Error allocating shared memory", __func__); return 0;}

  if ((pdppepCommunication->shmSegment = shmat(pdppepCommunication->sharedMemoryID, NULL, 0)) == (char *) -1)
    {log_error("%s - Error attaching shared memory", __func__); return 0;}

  unsigned int a;
  char *s = pdppepCommunication->shmSegment;
  for(a=0; a<4; a++) *s++='b';

  pep_startSharedMemoryThread();

  return pdppepCommunication->key;
}


axiom_node_ptr pep_soap_handler(unsigned int curOpNum, axiom_node_ptr node, axiom_msg_ctx_ptr msg_ctx)
{
  axiom_node_t *messageNode = NULL;
  axiom_node_ptr responseNode = NULL;
  axiom_element_ptr responseElem = NULL;

  AXIS2_ENV_CHECK(soapenv, NULL);
  if(node)
  {
    log_trace("%s - received message: %s = [%s]", __func__, axiom_node_getName(node),
              axiom_text_get_value( (axiom_text_t*)axiom_node_get_data_element(axiom_node_get_first_child(node, soapenv), soapenv), soapenv));

    axis2_endpoint_ref_t *endp=axis2_msg_ctx_get_from(msg_ctx, soapenv);
    key_t ret=1;
    if(endp!=NULL)
    {
      // according to documentation returns FROM address, but actually looks more like recipient as specified in the calling client
      log_trace("%s - endpoint address: %s", __func__, axis2_endpoint_ref_get_address(endp, soapenv));
      if(!strncmp(axis2_endpoint_ref_get_address(endp, soapenv), "http://localhost/axis2/services/pep", 35))
      {
        log_trace("%s - okay, I'm on the same host as PDP -> preparing shared memory segment", __func__);
        ret=prepareSharedMemory();
        if(ret==0) log_error("%s - Error preparing shared memory segment");
        else log_trace("Shared Memory successfully prepared using key=[%x]", ret);
      }
    }

    unsigned char *pepResponse=calloc(5, sizeof(unsigned char));
    snprintf(pepResponse, 5, "%d", ret);

    responseElem = axiom_element_create(soapenv, NULL, pepSoapService->outputNames[curOpNum], NULL, &responseNode);
    axiom_element_set_text(responseElem, soapenv, pepResponse, responseNode);
  }
  else
  {
    AXIS2_ERROR_SET(soapenv->error, AXIS2_ERROR_SVC_SKEL_INVALID_XML_FORMAT_IN_REQUEST, AXIS2_FAILURE);
    log_trace("ERROR: invalid XML in request\n");

    responseElem = axiom_element_create(soapenv, NULL, pepSoapService->outputNames[curOpNum], NULL, &responseNode);
    axiom_element_set_text(responseElem, soapenv, "Client! Who are you?", responseNode);
  }
  return responseNode;
}



