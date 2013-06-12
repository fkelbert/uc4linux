/**
 * @file  soapService.c
 * @brief Implementation for providing a soap service handler using axis2; used in master project
 *
 * @author cornelius moucha
 **/

#include "soap_service.h"

// Generic Soap Interface

AXIS2_EXPORT int axis2_get_instance(axis2_svc_skeleton_t **inst, const axutil_env_t *env)
{
  //soapService_ptr pdpSoapService = initSoapService(&pdpSoap_init, &hello_invoke, NULL, NULL);
  *inst = axis2_generic_create(env, &svc_skeleton_ops_var);
  if (!(*inst)) return AXIS2_FAILURE;
  return AXIS2_SUCCESS;
}

// generic
AXIS2_EXPORT int axis2_remove_instance(axis2_svc_skeleton_t *inst, const axutil_env_t *env)
{
  axis2_status_t status = AXIS2_FAILURE;
  if (inst) status = AXIS2_SVC_SKELETON_FREE(inst, env);
  return status;
}

axis2_svc_skeleton_t *axis2_generic_create(const axutil_env_t *env, const axis2_svc_skeleton_ops_t *soapSkeleton)
{
  axis2_svc_skeleton_t *svc_skeleton = NULL;
  svc_skeleton = AXIS2_MALLOC(env->allocator, sizeof(axis2_svc_skeleton_t));
  svc_skeleton->ops = soapSkeleton;
  svc_skeleton->func_array = NULL;
  return svc_skeleton;
}

axiom_node_t* AXIS2_CALL soap_on_fault(axis2_svc_skeleton_t *svc_skeli, const axutil_env_t *env, axiom_node_t *node)
{
  axiom_node_t *error_node = NULL;
  axiom_node_t* text_node = NULL;
  axiom_element_t *error_ele = NULL;
  error_ele = axiom_element_create(env, node, "EchoServiceError", NULL, &error_node);
  axiom_element_set_text(error_ele, env, "Echo service failed ", text_node);
  return error_node;
}

int AXIS2_CALL soap_free(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env)
{
  if (svc_skeleton->func_array)
  {
    axutil_array_list_free(svc_skeleton->func_array, env);
    svc_skeleton->func_array = NULL;
  }
  if (svc_skeleton)
  {
    AXIS2_FREE(env->allocator, svc_skeleton);
    svc_skeleton = NULL;
  }
  return AXIS2_SUCCESS;
}

