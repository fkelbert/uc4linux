/**
 * @file  xmlUtils.c
 * @brief Auxiliary methods for XML processing
 *
 * @see xmlUtils.h
 *
 * @todo separate schema validation in separate method! for loadXMLdoc, loadXMLmemory, ...
 *
 * @author cornelius moucha
 **/

#include "xmlUtils.h"
#include "log_xmlUtils_pef.h"

/**
 * Auxiliary method for print error messages during XML schema validation or XML processing
 *
 * @param  fp   the filepointer (usually stdout) for printing the message; not used here, instead log_error is used!
 * @param  msg  the message to print
**/
void xmlLogger(FILE *fp, char *msg, ...)
{
  va_list fmtargs;
  va_start(fmtargs,msg);
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
  log_error("Schemas validity error: %s", buffer);
  va_end(fmtargs);
}

#if pdpXMLvalidation == 1
  /// @todo: optimize to load schema only once!
  xmlSchemaPtr loadXMLschema()
  {
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    /*ctxt = xmlSchemaNewParserCtxt(schemaFilename);
    xmlDocPtr schemaDoc = NULL;
    if(!(schemaDoc = xmlReadFile(schemaFilename, NULL, XML_PARSE_NONET)))
    {
      log_error("Failed to read schema File");
      return NULL;
    }*/

    xmlDocPtr schemaDoc=NULL;
    schemaDoc=xmlReadMemory(pefschema, strlen(pefschema), "tmpxmlschema.xml", NULL, 0);
    checkNullPtr(schemaDoc, "Failure reading XML schema in memory!");

    ctxt = xmlSchemaNewDocParserCtxt(schemaDoc);
    //xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) xmlLogger,
    //                               (xmlSchemaValidityWarningFunc) xmlLogger, LOG_XMLUTILS_C_OUTPUT);
    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf,
                                   (xmlSchemaValidityWarningFunc) fprintf, stderr);
    schema=xmlSchemaParse(ctxt);
    if(schema==NULL)  {log_error("Error loading PEF XML schema..."); return NULL;}
    log_debug("Schema successfully loaded");

    xmlFreeDoc(schemaDoc);
    xmlSchemaFreeParserCtxt(ctxt);
    return schema;
  }
#endif

xmlDocPtr loadXMLdoc(const char *docFilename)
{
  checkNullPtr(docFilename, "Error loading XML-document for empty filename");
  xmlDocPtr doc=xmlParseFile(docFilename);
  if(doc==NULL) {log_error("Could not parse XML file [%s]", docFilename); xmlPrintError();}
  else
  {
    log_debug("XML file [%s] parsed", docFilename);
    #if pdpXMLvalidation == 1
      xmlSchemaPtr schema = loadXMLschema();
      if(schema!=NULL)
      {
        xmlSchemaValidCtxtPtr ctxt=xmlSchemaNewValidCtxt(schema);
        //xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) xmlLogger,
        //                              (xmlSchemaValidityWarningFunc) xmlLogger, LOG_XML_UTILS_C_OUTPUT);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf,
                                      (xmlSchemaValidityWarningFunc) fprintf, stderr);
        int ret = xmlSchemaValidateDoc(ctxt, doc);
        if(ret==0) log_info("XML file [%s] is valid according to pef-schema", docFilename);
        else
        {
          log_warn("XML file [%s] is not valid according to pef-schema", docFilename);
          xmlFreeDoc(doc);
          doc=NULL;
        }
        xmlSchemaFreeValidCtxt(ctxt);
        xmlSchemaCleanupTypes();
        xmlSchemaFree(schema);
      }
      else return NULL;
    #endif // pdpXMLvalidation == 1
  }
  return doc;
}

/// @todo validate encoding
xmlDocPtr loadXMLmemory(const char *xmlmem, unsigned long size, bool validate)
{
  checkNullPtr(xmlmem, "Could not load XML from NULL memory location");
  //printf("xml(%d)=[[%s]]\n",size,xmlmem);
  xmlDocPtr doc=xmlReadMemory(xmlmem, size, "tmpxml.xml", "ISO-8859-15", 0);
  if(doc==NULL) log_warn("ERROR reading xml in memory...");
  xmlPrintError();
  xmlResetLastError();
  checkNullPtr(doc, "Failure reading XML document in memory!");

  if(validate)
  {
    #if pdpXMLvalidation == 1
      xmlSchemaPtr schema = loadXMLschema();
      if(schema!=NULL)
      {
        xmlSchemaValidCtxtPtr ctxt=xmlSchemaNewValidCtxt(schema);
        //xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) xmlLogger,
        //                              (xmlSchemaValidityWarningFunc) xmlLogger, LOG_XML_UTILS_C_OUTPUT);
        xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf,
                                      (xmlSchemaValidityWarningFunc) fprintf, stderr);
        int ret=xmlSchemaValidateDoc(ctxt, doc);
        if(ret==0) log_info("XML in-memory is valid according to pef-schema");
        else
        {
          log_error("XML in-memory is not valid according to pef-schema");
          xmlFreeDoc(doc);
          doc=NULL;
        }
        xmlSchemaFreeValidCtxt(ctxt);
        xmlSchemaCleanupTypes();
        xmlSchemaFree(schema);
      }
      else return NULL;
  }
  #endif // pdpXMLvalidation == 1
  return doc;
}

void xmlPrintElementNames(const xmlNodePtr node)
{
  if(node==NULL) {log_warn("Cannot print NULL XML node!"); return;}
  xmlNode *curNode = NULL;
  for(curNode=node; curNode; curNode=curNode->next)
  {
    if(curNode->type==XML_ELEMENT_NODE) log_trace("node type: Element, name: %s\n", curNode->name);
    xmlPrintElementNames(curNode->children);
  }
}

void dumpXMLnode(const xmlNodePtr node)
{
  if(node==NULL) {log_warn("Cannot dump NULL XML node!"); return;}
  xmlNodePtr mynode=xmlCopyNode(node, 1);

  xmlBufferPtr myxbp=xmlBufferCreate();
  xmlDocPtr tmpdoc = NULL;
  tmpdoc=xmlNewDoc((xmlChar*)"1.0");
  xmlDocSetRootElement(tmpdoc, mynode);
  xmlNodeDump(myxbp,tmpdoc,mynode,1,1);
  log_trace("\n%s",xmlBufferContent(myxbp));
}

xmlBufferPtr dumpXMLdoc(const xmlDocPtr xmldoc)
{
  checkNullPtr(xmldoc, "Cannot dump NULL XML document!");
  xmlBufferPtr myxbp=xmlBufferCreate();
  xmlNodeDump(myxbp,xmldoc,xmlDocGetRootElement(xmldoc),1,1);
  log_trace("\n[%s]",xmlBufferContent(myxbp));
  return myxbp;
}


unsigned int xpathEval(xmlDocPtr doc, const xmlChar* xpathExpr)
{
  if(xpathExpr==NULL) {log_warn("Could not evaluate NULL xpath expression!"); return FALSE;}
  if(doc==NULL)       {log_warn("Could not evaluate xpath expression on NULL XML pointer!"); return FALSE;}
  xmlXPathContextPtr xpathCtx;
  xmlXPathObjectPtr  xpathObj;
  int ret=FALSE;

  xpathCtx=xmlXPathNewContext(doc);
  if(xpathCtx==NULL)
  {
    log_error("Unable to create new xpath context");
    xmlFreeDoc(doc);
    return ret;
  }

  // Evaluate XPath expression
  xpathObj=xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(xpathObj==NULL)
  {
    log_error("Unable to evaluate XPath expression %s", xpathExpr);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    return ret;
  }
  ret=xpathObj->boolval;

  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  return ret;
}

char* xpathExec(xmlDocPtr doc, const xmlChar* xpathExpr)
{
  if(xpathExpr==NULL) {log_warn("Could not execute NULL xpath expression!"); return NULL;}
  if(doc==NULL)       {log_warn("Could not execute xpath expression on NULL XML pointer!"); return NULL;}
  xmlXPathContextPtr xpathCtx;
  xmlXPathObjectPtr  xpathObj;

  //log_warn("Executing xpath expression: [%s]", xpathExpr);
  xpathCtx=xmlXPathNewContext(doc);
  if(xpathCtx==NULL)
  {
    log_error("Unable to create new xpath context");
    xmlFreeDoc(doc);
    return NULL;
  }

  // Evaluate XPath expression
  xpathObj=xmlXPathEvalExpression(xpathExpr, xpathCtx);
  if(xpathObj==NULL)
  {
    log_error("Unable to evaluate XPath expression %s", xpathExpr);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc);
    return NULL;
  }
  //ret=xpathObj->boolval;
  //log_warn("Boolean value of xpathObj: [%d]", ret);
  //log_warn("type: [%d]", xpathObj->type);
  //log_warn("string: [%s]", xpathObj->stringval);
  if(xpathObj->type!=XPATH_STRING) return NULL;
  char *ret=strdup(xpathObj->stringval);

  xmlXPathFreeObject(xpathObj);
  xmlXPathFreeContext(xpathCtx);
  return ret;
}

xmlNodePtr xmlFindNode(const xmlNodePtr node, const char *name)
{
  checkNullPtr(node, "%s - Could not search in NULL node!", __func__);
  checkNullPtr(name, "%s - Could not search for NULL name!", __func__);
  xmlNodePtr cur=node->children;
  while(cur!=NULL)
  {
    if(!xmlStrncasecmp(cur->name,(const xmlChar *)name,xmlStrlen(cur->name))) return cur;
    cur=cur->next;
  }
  log_trace("Specified node (\"%s\") not found in root node [%s]",name,(node->type==XML_ELEMENT_NODE ? node->name : (xmlChar*)""));
  return NULL;
}

bool xmlIsElement(xmlNodePtr node)
{
  if(node==NULL) return FALSE;
  if(node->type==XML_ELEMENT_NODE) return TRUE;
  else return FALSE;
}

xmlNodePtr xmlGetFirstChildElement(xmlNodePtr node)
{
  checkNullPtr(node, "Could not search for child elements in NULL node!");
  xmlNodePtr tmp=NULL;
  for(tmp=node->children; tmp; tmp=tmp->next)
    if(tmp->type==XML_ELEMENT_NODE) return tmp;
  return NULL;
}

/* error handling for libxml2
 * http://xmlsoft.org/html/libxml-xmlerror.html#xmlErrorDomain
 * domain=1 -> parser
 * code=9   -> invalid char
 * xmlErrorPtr perr = xmlGetLastError();
 * if(perr!=NULL) log_trace("domain=[%d], code=[%d]",perr->domain, perr->code);
 */
void xmlPrintError()
{
  xmlErrorPtr perr=xmlGetLastError();
  if(perr!=NULL)
  {
    log_error("level=[%d], domain=[%d], code=[%d]", perr->level, perr->domain, perr->code);
    log_error("msg=[%s]", perr->message);
    if(perr->str1!=NULL) log_error("str1=[%s]", perr->str1);
    if(perr->str2!=NULL) log_error("str2=[%s]", perr->str2);
    if(perr->str3!=NULL) log_error("str3=[%s]", perr->str3);
    // dumping error node not working (SEGFAULT)
    //#0  0xb7db19e8 in xmlCopyNamespace () from /usr/lib/libxml2.so.2
    //#1  0xb7db1a48 in xmlCopyNamespaceList () from /usr/lib/libxml2.so.2
    //#2  0xb7db8351 in ?? () from /usr/lib/libxml2.so.2
    //#3  0xb7f8e7b1 in dumpXMLnode (node=0x805cfc8) at libraries/utilities/xmlUtils.c:121
    //if(perr->node!=NULL)
    //{log_error("error node: ["); dumpXMLnode(perr->node); log_error("]");}
    xmlResetError(perr);
    xmlResetLastError();
  }
}












