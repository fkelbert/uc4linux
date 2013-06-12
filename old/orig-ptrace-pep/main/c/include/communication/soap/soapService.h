/**
 * @file  soapService.h
 * @brief Definitions for providing a soap service handler using axis2; used in master project
 *
 * @author cornelius moucha
 **/

#ifndef SOAP_SERVICE_H

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
#include "logger.h"
#include "axiomUtils.h"
#include "base.h"


typedef axis2_svc_skeleton_t *axis2_svc_skeleton_ptr;
typedef axiom_node_t         *axiom_node_ptr;

axis2_svc_skeleton_t *axis2_generic_create(const axutil_env_t *env, const axis2_svc_skeleton_ops_t *SoapSkeleton);
int AXIS2_CALL soap_free(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env);
axiom_node_t* AXIS2_CALL soap_on_fault(axis2_svc_skeleton_t *svc_skeli, const axutil_env_t *env, axiom_node_t *node);

extern axiom_node_t* AXIS2_CALL soap_invoke(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env, axiom_node_t *node, axis2_msg_ctx_t *msg_ctx);
extern int AXIS2_CALL soap_init(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env);

static const axis2_svc_skeleton_ops_t svc_skeleton_ops_var =
{
  soap_init,
  soap_invoke,
  soap_on_fault,
  soap_free
};

#endif
