/**
 * @file  soapUtils.h
 * @brief Auxiliary utilities for soap-communication
 *
 * @author cornelius moucha
 **/

#pragma once
#include <axis2_svc_skeleton.h>
#include <axutil_log_default.h>
#include <axutil_error_default.h>
#include <axutil_array_list.h>
#include <axiom_text.h>
#include <axiom_node.h>
#include <axiom_element.h>
#include <axiom.h>
#include <axis2_util.h>
#include <axutil_env.h>
#include "utilities/axiom_utils.h"
#include "base/const.h"

typedef axis2_svc_skeleton_t *axis2_svc_skeleton_ptr;

typedef struct soapService_s
{
  axis2_svc_skeleton_ops_t soapServiceSkeleton;
  unsigned int numOp;
  unsigned char **opNames;
  unsigned char **inputNames;
  unsigned char **outputNames;
} soapService_t;

typedef soapService_t *soapService_ptr;
soapService_ptr initSoapService(soapService_ptr nSoapService, unsigned char *wsdl, const axutil_env_t *env);
unsigned int    readWSDL(soapService_ptr, unsigned char *wsdl, const axutil_env_t *env);

axis2_svc_skeleton_ptr    axis2_generic_create(const axutil_env_t *env, const axis2_svc_skeleton_ops_t *SoapSkeleton);
int            AXIS2_CALL generic_free(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env);
axiom_node_ptr AXIS2_CALL generic_on_fault(axis2_svc_skeleton_t *svc_skeli, const axutil_env_t *env, axiom_node_t *node);
axiom_node_ptr AXIS2_CALL generic_invoke(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env, axiom_node_t *node, axis2_msg_ctx_t *msg_ctx);
int            AXIS2_CALL generic_init(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env);

#define SOAP_OPERATIONS(X) axiom_node_ptr (*X[])(unsigned int, axiom_node_ptr, axiom_msg_ctx_ptr)

#define GENERATE_SOAP_SERVICE(NAME, INIT, WSDL, OPERATION_PTRS)                                 \
  static soapService_ptr NAME;                                                                  \
  const axutil_env_t *soapenv = NULL;                                                           \
  AXIS2_EXPORT int axis2_get_instance(axis2_svc_skeleton_t **inst, const axutil_env_t *env)     \
  {                                                                                             \
    soapenv = env;                                                                              \
    NAME = initSoapService(NAME, WSDL, env);                                                    \
    if(NAME==NULL) return AXIS2_FAILURE;                                                        \
    *inst = axis2_generic_create(env, &NAME->soapServiceSkeleton);                              \
    if (!(*inst)) {fprintf(stderr, "error getting soap instance!\n"); return AXIS2_FAILURE;}    \
    return AXIS2_SUCCESS;                                                                       \
  }                                                                                             \
                                                                                                \
  AXIS2_EXPORT int axis2_remove_instance(axis2_svc_skeleton_t *inst, const axutil_env_t *env)   \
  {                                                                                             \
    axis2_status_t status = AXIS2_FAILURE;                                                      \
    if (inst) status = AXIS2_SVC_SKELETON_FREE(inst, env);                                      \
    return status;                                                                              \
  }                                                                                             \
                                                                                                \
  int AXIS2_CALL generic_init(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env)      \
  {                                                                                             \
    svc_skeleton->func_array = axutil_array_list_create(env, 0);                                \
    axutil_array_list_add(svc_skeleton->func_array, env, "helloString");                        \
    if(INIT!=NULL) INIT();                                                                      \
    return AXIS2_SUCCESS;                                                                       \
  }                                                                                             \
                                                                                                \
  axiom_node_t* AXIS2_CALL generic_invoke(axis2_svc_skeleton_t *svc_skeleton,                   \
                        const axutil_env_t *env, axiom_node_t *node, axis2_msg_ctx_t *msg_ctx)  \
  {                                                                                             \
    axis2_char_t *nodename=axiom_element_get_localname(axiom_node_get_data_element(node, env),  \
                                                       env);                                    \
    unsigned int a;                                                                             \
    fprintf(stderr, "msgctx=[%p]\n", msg_ctx);                                                  \
    fflush(stderr);                                                                             \
    for(a=0; a<NAME->numOp; a++) {                                                              \
      if(strcmp(nodename, NAME->opNames[a])==0 )                                                \
        return OPERATION_PTRS[a](a, node, msg_ctx);                                             \
    }                                                                                           \
    fprintf(stderr, "no operation found for given name in soap message: %s\n", nodename);       \
    fflush(stderr);                                                                             \
  }





