/**
 * @file  soapUtils.c
 * @brief Implementation of auxiliary utilities for soap-communication
 *
 * @author cornelius moucha
 **/
#include "soapUtils.h"

soapService_ptr initSoapService(soapService_ptr nSoapService, unsigned char *wsdl, const axutil_env_t *env)
{
  nSoapService=malloc(sizeof(soapService_t));
  nSoapService->soapServiceSkeleton = (axis2_svc_skeleton_ops_t)
          {
            generic_init,
            generic_invoke,
            generic_on_fault,
            generic_free
          };
  nSoapService->numOp=0;

  unsigned int ret = readWSDL(nSoapService, wsdl, env);
  if(ret==R_ERROR) {log_error("%s - Error reading wsdl file for soap service", __func__); return NULL;}
  log_trace("WSDL - found %d operations", nSoapService->numOp);
  unsigned int a;
  for(a=0; a<nSoapService->numOp; a++)
    log_trace("   operation: %s (input node: %s, output node: %s)", nSoapService->opNames[a], nSoapService->inputNames[a], nSoapService->outputNames[a]);

  return nSoapService;
}

/// if wsdl is not present/accessible -> child pid 5625 exit signal Segmentation fault (11)
unsigned int readWSDL(soapService_ptr curSoapService, unsigned char *wsdl, const axutil_env_t *env)
{
  axiom_xml_reader_t *xml_reader = NULL;
  axiom_stax_builder_t *om_builder = NULL;
  axiom_document_t *document = NULL;

  xml_reader = axiom_xml_reader_create_for_file(env, wsdl, NULL);
  if(xml_reader == NULL) {fprintf(stderr, "%s - Error reading wsdl-file [%s]", wsdl); return R_ERROR;}
    //{log_error("%s - Error reading wsdl-file [%s]", wsdl); return R_ERROR;}
  om_builder = axiom_stax_builder_create(env, xml_reader);
  if(om_builder == NULL)
  {
    //log_error("%s - Error creating stax builder for reading", __func__);
    fprintf(stderr, "%s - Error creating stax builder for reading", __func__);
    axiom_xml_reader_free(xml_reader, env);
    return R_ERROR;
  }
  document = axiom_stax_builder_get_document(om_builder, env);
  if(document == NULL)
  {
    //log_error("%s - Error getting document", __func__);
    fprintf(stderr, "%s - Error getting document", __func__);
    axiom_stax_builder_free(om_builder, env);
    axiom_xml_reader_free(xml_reader, env);
    return R_ERROR;
  }

  axiom_node_ptr rootNode = axiom_document_build_all(document, env);
  if(rootNode == NULL)
  {
    //log_trace("%s - Error getting root node", __func__);
    fprintf(stderr, "%s - Error getting root node", __func__);
    axiom_stax_builder_free(om_builder, env);
    return R_ERROR;
  }
  axiom_element_ptr rootElem = axiom_getElement(rootNode);

  axiom_node_ptr portType = axiomFindNode(axiom_getElement(rootNode), rootNode, "portType", TRUE);
  if(portType==NULL)
  {
    //log_trace("%s - Error reading portType in WSDL", __func__);
    fprintf(stderr, "%s - Error reading portType in WSDL", __func__);
    return R_ERROR;
  }

  axiom_namespace_t *cns=axiom_element_get_namespace(rootElem, env, rootNode);
  if(cns==NULL) {log_warn("%s - Failure in retrieving namespace", __func__); return R_ERROR;}

  axiom_children_qname_iterator_t* curChildren = NULL;
  curChildren = axiom_element_get_children_with_qname(axiom_getElement(portType), env,
      axutil_qname_create(env, "operation", axiom_namespace_get_uri(cns, env), axiom_namespace_get_prefix(cns, env)), portType);

  if(curChildren==NULL) log_trace("%s - Failure in getting children for portType=[%s]", __func__, axiom_node_getName(portType));
  while(axiom_children_qname_iterator_has_next(curChildren, env))
  {
    axiom_node_ptr ncur=axiom_children_qname_iterator_next(curChildren, env);
    axiom_char_ptr curOperationName = axiom_element_get_attribute_value_by_name(axiom_getElement(ncur), env, "name");
    log_trace("found operation: %s", curOperationName);
    axiom_element_ptr inputMessage=axiomFindNode(axiom_getElement(ncur), ncur, "input", FALSE);
    axiom_element_ptr outputMessage=axiomFindNode(axiom_getElement(ncur), ncur, "output", FALSE);
    if(inputMessage==NULL || outputMessage==NULL) {log_error("%s - Error reading input/output message in WSDL", __func__); return R_ERROR;}
    axiom_char_ptr inputTypeName  = axiom_element_get_attribute_value_by_name(inputMessage, env, "message");
    axiom_char_ptr outputTypeName = axiom_element_get_attribute_value_by_name(outputMessage, env, "message");
    if(inputTypeName==NULL || outputTypeName==NULL) {log_error("%s - Error reading input/output message type in WSDL", __func__); return R_ERROR;}
    //log_trace("inputTypeName=[%s], outputTypeName=[%s]", inputTypeName, outputTypeName);

    axiom_children_qname_iterator_t* messageIterator = NULL;
    messageIterator = axiom_element_get_children_with_qname(rootElem, env,
        axutil_qname_create(env, "message", axiom_namespace_get_uri(cns, env), axiom_namespace_get_prefix(cns, env)), rootNode);
    if(messageIterator==NULL) {log_error("%s - Error generating Iterator for wsdl:messages", __func__); return R_ERROR;}

    axiom_char_ptr inputPartName = NULL;
    axiom_char_ptr outputPartName = NULL;
    while(axiom_children_qname_iterator_has_next(messageIterator, env))
    {
      axiom_node_ptr messageNode = axiom_children_qname_iterator_next(messageIterator, env);
      axiom_char_ptr messageName = axiom_element_get_attribute_value_by_name(axiom_getElement(messageNode), env, "name");

      if(!strncmp(inputTypeName, messageName, strlen(messageName)))
      {
        //log_trace("found specification for inputType");
        inputPartName = axiom_element_get_attribute_value_by_name(axiomFindNode(axiom_getElement(messageNode), messageNode, "part", FALSE),
                                                                                 env, "name");
        if(inputPartName==NULL) {log_error("%s - Error reading input node name in WSDL", __func__); return R_ERROR;}
      }

      if(!strncmp(outputTypeName, messageName, strlen(messageName)))
      {
        //log_trace("found specification for outputType");
        outputPartName = axiom_element_get_attribute_value_by_name(axiomFindNode(axiom_getElement(messageNode), messageNode, "part", FALSE),
                                                                                 env, "name");
        if(outputPartName==NULL) {log_error("%s - Error reading output node name in WSDL", __func__); return R_ERROR;}
      }
    }
    log_trace("   input node name=[%s], output node name=[%s]", inputPartName, outputPartName);

    curSoapService->opNames=realloc(curSoapService->opNames, (curSoapService->numOp+1) * sizeof(unsigned char*));
    curSoapService->opNames[curSoapService->numOp]=calloc(strlen(curOperationName)+1, sizeof(unsigned char*));
    curSoapService->opNames[curSoapService->numOp]=strdup(curOperationName);

    curSoapService->inputNames=realloc(curSoapService->inputNames, (curSoapService->numOp+1) * sizeof(unsigned char*));
    curSoapService->inputNames[curSoapService->numOp]=calloc(strlen(inputPartName)+1, sizeof(unsigned char*));
    curSoapService->inputNames[curSoapService->numOp]=strdup(inputPartName);

    curSoapService->outputNames=realloc(curSoapService->outputNames, (curSoapService->numOp+1) * sizeof(unsigned char*));
    curSoapService->outputNames[curSoapService->numOp]=calloc(strlen(outputPartName)+1, sizeof(unsigned char*));
    curSoapService->outputNames[curSoapService->numOp]=strdup(outputPartName);

    curSoapService->numOp++;

  }

  return R_SUCCESS;
}

// Generic Soap Interface
axis2_svc_skeleton_t *axis2_generic_create(const axutil_env_t *env, const axis2_svc_skeleton_ops_t *soapSkeleton)
{
  axis2_svc_skeleton_t *svc_skeleton = NULL;
  svc_skeleton = AXIS2_MALLOC(env->allocator, sizeof(axis2_svc_skeleton_t));
  svc_skeleton->ops = soapSkeleton;
  svc_skeleton->func_array = NULL;
  return svc_skeleton;
}

axiom_node_t* AXIS2_CALL generic_on_fault(axis2_svc_skeleton_t *svc_skeli, const axutil_env_t *env, axiom_node_t *node)
{
  axiom_node_t *error_node = NULL;
  axiom_node_t* text_node = NULL;
  axiom_element_t *error_ele = NULL;
  error_ele = axiom_element_create(env, node, "EchoServiceError", NULL, &error_node);
  axiom_element_set_text(error_ele, env, "Echo service failed ", text_node);
  return error_node;
}

int AXIS2_CALL generic_free(axis2_svc_skeleton_t *svc_skeleton, const axutil_env_t *env)
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
