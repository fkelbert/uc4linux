/*
 * xml_utils.c
 *
 *      Author: Ricardo Neisse, Cornelius Moucha
 */

#include "xml_utils.h"
#include "log_xml_utils_c.h"

// needed for xmlSchemaValidation
void xml_log_helper(FILE *fp, char *msg, ...)
{
  va_list fmtargs;
  va_start(fmtargs,msg);
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
  log_error("Schemas validity error: %s", buffer);
  va_end(fmtargs);
}

// This function returns NULL if schema contains errors
xmlSchemaPtr loadXmlSchema(char *schemaFilename)
{
  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;
  ctxt = xmlSchemaNewParserCtxt(schemaFilename);
  //xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf,
  //                               (xmlSchemaValidityWarningFunc) fprintf,
  //                               stderr);

  xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) xml_log_helper,
                                  (xmlSchemaValidityWarningFunc) xml_log_helper,
                                  LOG_XML_UTILS_C_OUTPUT);
  schema = xmlSchemaParse(ctxt);
  if (schema==NULL)
  {
    log_error("Error loading schema [%s]", schemaFilename);
    //exit(1);
    return NULL;
  }
  log_debug("Schema [%s] loaded", schemaFilename);
  return schema;
}

xmlDocPtr loadXmlDoc(const char *docFilename, char *schemaFilename)
{
  xmlDocPtr doc;
  doc = xmlParseFile(docFilename);
  if(doc==NULL) log_error("Could not parse XML file [%s]", docFilename);
  else
  {
    log_debug("XML file [%s] parsed", docFilename);
    /*xmlSchemaPtr schema = loadXmlSchema(schemaFilename);
    if(schema!=NULL)
    {
      xmlSchemaValidCtxtPtr ctxt;
      ctxt = xmlSchemaNewValidCtxt(schema);
      //xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf,
      //                              (xmlSchemaValidityWarningFunc) fprintf, stderr);
      xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) xml_log_helper,
                                    (xmlSchemaValidityWarningFunc) xml_log_helper, LOG_XML_UTILS_C_OUTPUT);
      int ret = xmlSchemaValidateDoc(ctxt, doc);
      if(ret==0) log_info("XML file [%s] is valid according to schema [%s]", docFilename, schemaFilename);
      else
      {
        log_error("XML file [%s] is not valid according to schema [%s]", docFilename, schemaFilename);
        xmlFreeDoc(doc);
      }
      xmlSchemaFreeValidCtxt(ctxt);
      xmlSchemaCleanupTypes();
      xmlSchemaFree(schema);
      //xmlMemoryDump();
      //xmlCleanupParser();
    }
    else return NULL;
    */
  }
  return doc;
}

char *getText(xmlNode * e_node)
{
  xmlNode * node;
  char *val;
  for (node = e_node->children; node; node = node->next) {
    if (count_non_empty((char*)node->content)!=0) {
      val = (char *)malloc(sizeof(char) * strlen( (char*)node->content)+1);
      sprintf(val, "%s", node->content);
      return val;
    }
  }
  return NULL;
}

xmlNode *next_valid(xmlNode *node) {
  while( (node = node->next) ) {
    if (!(node->type == XML_TEXT_NODE && count_non_empty( (char*)node->content)==0)) {
      break;
    }
  }
  return node;
}

int count=0;

void xml_dump (FILE *tmp_file, xmlNode * node) {
  xmlNode *cur_node = NULL;
  xmlChar *key, *value;
  for (cur_node = node; cur_node; cur_node = cur_node->next) {
    if (count_non_empty( (char*)cur_node->content)==0) {
      if (cur_node->type != XML_TEXT_NODE) {
        fprintf(tmp_file,"[%s]\n", cur_node->name, cur_node->type);
      }
    } else {
      fprintf(tmp_file,"[%s]=[%s]\n", cur_node->name, cur_node->content);
    }
    xmlAttrPtr propPtr;
    propPtr = cur_node->properties;
    while( propPtr ) {
      key = (xmlChar*) propPtr->name;
      value = xmlGetProp( cur_node, key);
      if (strcmp( (char*)key,"schemaLocation")!=0) {
        fprintf(tmp_file, "[%s]=[%s]\n", key, value);
      }
      //xmlFree(value);
	  free(value);
	  //xmlFreeProp(value);
      propPtr = propPtr->next;
    }

    // plog(LOG_TRACE, "--> xml_dump(children)");
    xml_dump(tmp_file, cur_node->children);
    // plog(LOG_TRACE, "<--");
  }
}

void print_element_names(xmlNode * a_node)
{
  xmlNode *cur_node = NULL;

  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      printf("node type: Element, name: %s\n", cur_node->name);
    }

    print_element_names(cur_node->children);
  }
}

/*
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
  xmlNodePtr cur;
  int size;
  int i;

  assert(output);
  size = (nodes) ? nodes->nodeNr : 0;

  fprintf(output, "Result (%d nodes):\n", size);
  for(i = 0; i < size; ++i) {
    assert(nodes->nodeTab[i]);

    if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
      xmlNsPtr ns;

      ns = (xmlNsPtr)nodes->nodeTab[i];
      cur = (xmlNodePtr)ns->next;
      if(cur->ns) {
        fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n",
            ns->prefix, ns->href, cur->ns->href, cur->name);
      } else {
        fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n",
            ns->prefix, ns->href, cur->name);
      }
    } else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
      cur = nodes->nodeTab[i];
      if(cur->ns) {
        fprintf(output, "= element node \"%s:%s\"\n",
            cur->ns->href, cur->name);
      } else {
        fprintf(output, "= element node \"%s\"\n",
            cur->name);
      }
    } else {
      cur = nodes->nodeTab[i];
      fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
    }
  }
}
*/

int exec_xpath(xmlDocPtr doc, const xmlChar* xpathExpr)
{
  xmlXPathContextPtr xpathCtx;
  xmlXPathObjectPtr xpathObj;
  int ret_val = FALSE;

  xmlInitParser();
  // Create XPath evaluation context
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL)
  {
    log_error("Unable to create new XPath context");
    xmlFreeDoc(doc);
    return ret_val;
  }

  // Evaluate XPath expression
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(xpathObj == NULL)
  {
    log_error("Unable to evaluate XPath expression %s", xpathExpr);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    return ret_val;
  }
  ret_val = xpathObj->boolval;
  //log_debug("boolval %d \n",xpathObj->boolval);
  //log_debug("stringval %s \n",xpathObj->stringval);

  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  //xmlCleanupParser(); //-> sometimes freezes the process or result in JAVA ACCESS EXCEPTION?!
  return ret_val;
}


void dumpXMLnode(const xmlNodePtr node)
{
  xmlNodePtr mynode=xmlCopyNode(node, 1);
  //print_element_names(mynode);
  //log_trace("printing element names finished");

  xmlBufferPtr myxbp=xmlBufferCreate();
  xmlDocPtr tmpdoc = NULL;
  tmpdoc = xmlNewDoc((xmlChar*)"1.0");
  xmlDocSetRootElement(tmpdoc, mynode);
  xmlNodeDump(myxbp,tmpdoc,mynode,1,1);
  log_trace("\n%s",xmlBufferContent(myxbp));
}

xmlBufferPtr dumpXMLdoc(xmlDocPtr xmldoc)
{
  xmlBufferPtr myxbp=xmlBufferCreate();
  xmlNodeDump(myxbp,xmldoc,xmlDocGetRootElement(xmldoc),1,1);
  log_debug("\n[%s]",xmlBufferContent(myxbp));
  return myxbp;
}

xmlNodePtr xmlFindNode(const xmlNodePtr root, const unsigned char *name)
{
  xmlNodePtr cur=root->children;
  while(cur!=NULL)
  {
    if(!xmlStrncasecmp(cur->name,(const xmlChar *)name,xmlStrlen(cur->name))) return cur;
    cur=cur->next;
  }
  log_trace("Specified node (\"%s\") not found in root node %s",name,(root->type==XML_ELEMENT_NODE ? root->name : (xmlChar*)""));
  return NULL;
}

bool xmlIsElement(xmlNodePtr node)
{
  if(node->type==XML_ELEMENT_NODE) return TRUE;
  else return FALSE;
}

xmlNodePtr xmlGetFirstChildElement(xmlNodePtr node)
{
  xmlNodePtr tmp = NULL;
  for(tmp=node->children; tmp; tmp=tmp->next)
    if(tmp->type==XML_ELEMENT_NODE) return tmp;
  return NULL;
}

xmlDocPtr load_xml_memory(const char *xmlmem, unsigned long size)
{
  xmlDocPtr doc;

  doc = xmlReadMemory(xmlmem, size, "tmpxml.xml", NULL, 0);
  check_null_ptr(doc, "Failure reading XML document in memory!");

  return doc;
}
