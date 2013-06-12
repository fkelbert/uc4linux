/**
 * @file  axiomUtils.c
 * @brief Implementation of utilities for axis2; used in Master project
 *
 * @author cornelius moucha
**/

#include "axiomUtils.h"

// needed for axiom_... methods; is initialized in GENERATE_SOAP_SERVICE macro
extern const axutil_env_t *soapenv;

void dumpAxiomNode(const axutil_env_t *env, axiom_node_t *root_node)
{
  axiom_stax_builder_t *om_builder = NULL;
  axiom_xml_writer_t   *xml_writer = NULL;
  axiom_output_t       *om_output = NULL;
  axis2_char_t         *buffer = NULL;

  if(!root_node) log_warn("root_node=NULL?!\n");
  xml_writer = axiom_xml_writer_create_for_memory(env, NULL, AXIS2_TRUE, 0, AXIS2_XML_PARSER_TYPE_BUFFER);
  om_output = axiom_output_create(env, xml_writer);
  axiom_node_serialize(root_node, env, om_output);
  buffer = (axis2_char_t*)axiom_xml_writer_get_xml(xml_writer, env);
  log_trace("%s ", buffer);
  log_trace("\n\n");
}

void printNodeTypes(const axutil_env_t *env, axiom_node_t *root)
{
  axiom_child_element_iterator_t *children_iter = NULL;
  children_iter = axiom_child_element_iterator_create(env, root);
  if(children_iter==NULL) log_trace("error children iterator\n");

  while( axiom_child_element_iterator_has_next(children_iter, env) )
  {
    axiom_node_t *node = NULL;
    node=axiom_child_element_iterator_next(children_iter, env);
    if(NULL != node)
    {
      log_trace("Node type=[%d] found\n", axiom_node_get_node_type(node, env));
    } else log_trace("error getting next child!\n");
  }
}

void* axiomFindNode(const axiom_element_ptr elem, const axiom_node_ptr node, const unsigned char *name, bool retNode)
{
  axiom_attribute_t *attr = NULL;
  axutil_qname_t *qname = NULL;
  axiom_namespace_t *cns = NULL;
  axiom_node_ptr sNode = NULL;
  axiom_element_ptr sElem = NULL;

  cns=axiom_element_get_namespace(elem, soapenv, node);
  if(cns==NULL) {log_warn("%s - Failure in retrieving namespace", __func__); return NULL;}
  qname = axutil_qname_create(soapenv, name, axiom_namespace_get_uri(cns, soapenv), axiom_namespace_get_prefix(cns, soapenv));
  if(qname == NULL) {log_error("%s - Failure in creating qualified name for [%s]", __func__, name); return NULL;}

  sElem = axiom_element_get_first_child_with_qname(elem, soapenv, qname, node, &sNode);
  if(sElem == NULL)
  {
    log_trace("%s - Specified node (\"%s\") not found in root node (\"%s\")",__func__, name,
                  axiom_element_get_localname(axiom_node_get_data_element(node, soapenv), soapenv));
    return NULL;
  }
  if(retNode) return sNode;
  else return sElem;
}

axiom_char_ptr axiom_getName(axiom_element_ptr elem)
{
  return axiom_element_get_localname(elem, soapenv);
}

axiom_char_ptr axiom_node_getName(axiom_node_ptr node)
{
  return axiom_element_get_localname((axiom_element_ptr)axiom_node_get_data_element(node, soapenv), soapenv);
}

axiom_element_ptr axiom_getElement(axiom_node_ptr node)
{
  if(axiom_node_get_node_type(node, soapenv) == AXIOM_ELEMENT)
    return (axiom_element_ptr)axiom_node_get_data_element(node, soapenv);
  else return NULL;
}

unsigned int validateXML(axiom_node_ptr rootNode)
{
  xmlDocPtr doc = NULL;
  axiom_xml_writer_t *xml_writer = NULL;
  axiom_output_t *output = NULL;
  axis2_char_t *buffer = NULL;

  xml_writer=axiom_xml_writer_create_for_memory(soapenv, NULL, AXIS2_TRUE, 0, AXIS2_XML_PARSER_TYPE_BUFFER);
  output=axiom_output_create(soapenv, xml_writer);
  axiom_node_serialize(rootNode, soapenv, output);
  buffer=(axis2_char_t*)axiom_xml_writer_get_xml(xml_writer, soapenv);

  doc=xmlReadMemory(buffer, axiom_xml_writer_get_xml_size(xml_writer, soapenv), "tmpfile.xml", NULL, 0);
  if(doc==NULL)
  {
    log_error("Could not validate XML - xmlReadMemory");
    return R_ERROR;
  }
  else
  {
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr pctxt;
    const char *schemaFile="/home/raindrop/hiwi/prog/FhG-ESF/src/main/xml/enforcementLanguage-FhG.xsd";
    pctxt = xmlSchemaNewParserCtxt(schemaFile);
    xmlSchemaSetParserErrors(pctxt, (xmlSchemaValidityErrorFunc) axiomLogHelper, (xmlSchemaValidityWarningFunc) axiomLogHelper, LOG_AXIOM_UTILS_C_OUTPUT);
    schema = xmlSchemaParse(pctxt);
    if(schema==NULL)
    {
      log_error("%s - Error loading schema [%s]", __func__, schemaFile);
      return R_ERROR;
    }
    xmlSchemaValidCtxtPtr ctxt;
    ctxt = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) axiomLogHelper, (xmlSchemaValidityWarningFunc) axiomLogHelper, LOG_AXIOM_UTILS_C_OUTPUT);
    int ret = xmlSchemaValidateDoc(ctxt, doc);
    if (ret == 0) log_info("XML is valid according to schema [%s]",schemaFile);
    else log_error("XML is NOT valid according to schema [%s]", schemaFile);
    xmlSchemaFreeValidCtxt(ctxt);
    xmlSchemaCleanupTypes();
    xmlSchemaFree(schema);
    xmlFreeDoc(doc);
    return ret;
  }
}

// needed for xmlSchemaValidation
void axiomLogHelper(FILE *fp, unsigned char *msg, ...)
{
  va_list fmtargs;
  va_start(fmtargs,msg);
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
  log_error("Schemas validity error: %s", buffer);
  va_end(fmtargs);
}













