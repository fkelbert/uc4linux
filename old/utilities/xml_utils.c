/*
 * xml_utils.c
 *
 *      Author: Ricardo Neisse
 */

#include "xml_utils.h"

// This function returns NULL if schema contains errors
xmlSchemaPtr loadXmlSchema(char *schemaFilename) {
  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;
  ctxt = xmlSchemaNewParserCtxt(schemaFilename);
  xmlSchemaSetParserErrors(ctxt,
      (xmlSchemaValidityErrorFunc) fprintf,
      (xmlSchemaValidityWarningFunc) fprintf,
      stderr);
  schema = xmlSchemaParse(ctxt);
  // xmlSchemaFreeParserCtxt(ctxt);
  if (LOG_LEVEL == LOG_TRACE) {
    // plog(LOG_TRACE, "Schema dump:");
    // xmlSchemaDump(stdout, schema);
  }
  if (schema==NULL) {
    plog(LOG_ERROR, "Error loading schema [%s]", schemaFilename);
    exit(1);
  }
  plog(LOG_DEBUG, "Schema [%s] loaded", schemaFilename);
  return schema;
}

xmlDocPtr loadXmlDoc(char *docFilename, char *schemaFilename) {
  xmlDocPtr doc;
  doc = xmlParseFile(docFilename);

  if (doc == NULL) {
    plog(LOG_ERROR, "Could not parse XML file [%s]", docFilename);
    return doc;
  } else {
    plog(LOG_INFO, "XML file [%s] parsed", docFilename);
    xmlSchemaPtr schema = loadXmlSchema(schemaFilename);
    if (schema != NULL) {
      xmlSchemaValidCtxtPtr ctxt;
      ctxt = xmlSchemaNewValidCtxt(schema);
      xmlSchemaSetValidErrors(ctxt,
          (xmlSchemaValidityErrorFunc) fprintf,
          (xmlSchemaValidityWarningFunc) fprintf,
          stderr);
      int ret = xmlSchemaValidateDoc(ctxt, doc);
      if (ret == 0) {
        plog(LOG_INFO, "XML file [%s] is valid according to schema [%s]", docFilename, schemaFilename);
      } else {
        plog(LOG_ERROR, "XML file [%s] is not valid according to schema [%s]", docFilename, schemaFilename);
        return NULL;
      }
      //xmlSchemaFreeValidCtxt(ctxt);
      //xmlSchemaCleanupTypes();
      //xmlSchemaFree(schema);
      //xmlFreeDoc(doc);
      //xmlMemoryDump();
      //xmlCleanupParser();
    }
  }
  return doc;
}

char *getText(xmlNode * e_node) {
  xmlNode * node;
  char *val;
  for (node = e_node->children; node; node = node->next) {
    if (count_non_empty(node->content)!=0) {
      val = (char *)calloc(strlen(node->content)+1, sizeof(char));
      snprintf(val, strlen(node->content)+1, "%s", node->content);
      return val;
    }
  }
}

xmlNode *next_valid(xmlNode *node) {
  while(node = node->next) {
    if (!(node->type == XML_TEXT_NODE && count_non_empty(node->content)==0)) {
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
    if (count_non_empty(cur_node->content)==0) {
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
      if (strcmp(key,"schemaLocation")!=0) {
        fprintf(tmp_file, "[%s]=[%s]\n", key, value);
      }
      xmlFree(value);
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

int exec_xpath(xmlDocPtr doc, const xmlChar* xpathExpr) {
   xmlChar *xmlbuff;
    int buffersize;

	xmlDocDumpFormatMemory (doc, &xmlbuff, &buffersize, 1);

//	printf ("XPATH EXPRESSION - - - - - - %s\n", ((char *) xmlbuff));

	xmlFree (xmlbuff);

  xmlXPathContextPtr xpathCtx;
  xmlXPathObjectPtr xpathObj;
  int ret_val = FALSE;

  xmlInitParser();

  assert(xpathExpr);


  // Create XPath evaluation context
  xpathCtx = xmlXPathNewContext(doc);
  if(xpathCtx == NULL) {
    plog(LOG_ERROR, "Unable to create new XPath context");
    xmlFreeDoc(doc);
    return ret_val;
  }




  // Evaluate XPath expression
  xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(xpathObj == NULL) {
    plog(LOG_ERROR,  "Unable to evaluate XPath expression %s", xpathExpr);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    return ret_val;
  }

  ret_val = xpathObj->boolval;
  // printf("boolval %d \n",xpathObj->boolval);
  // printf("stringval %s \n",xpathObj->stringval);

  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);

  xmlCleanupParser();

  return ret_val;
}
