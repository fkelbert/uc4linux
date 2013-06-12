/**
 * @file  xmlUtils.h
 * @brief Auxiliary methods for XML processing
 *
 * @author cornelius moucha
 **/

#ifndef xmlUtils_h
#define xmlUtils_h

#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/xpointer.h>  // required for Android (otherwise: 'xmlXPathContextPtr' undeclared)

#include "base.h"
#include "stringUtils.h"

#if pdpXMLvalidation == 1
  #include <libxml/xmlschemas.h>
  #include <libxml/xmlschemastypes.h>

  #include "pefschema.h"

  /**
   * Preload a XML schema referencing the PEF policy schema
   *
   * @return  XML schema reference on success or NULL otherwise
   */
  xmlSchemaPtr loadXMLschema();
#endif

/**
 * Load a XML document from a given file
 *
 * @param   docFilename   the filename of the XML document
 * @return  XML document on success or NULL otherwise
**/
xmlDocPtr    loadXMLdoc(const char *docFilename);

/**
 * Load a XML document from memory
 *
 * @param   xmlmem   the memory pointer of the xml document, i.e. the string
 * @param   size     size of the xml memory string
 * @param   validate flag for XML schema validation
 * @return  XML document on success or NULL otherwise
**/
xmlDocPtr    loadXMLmemory(const char *xmlmem, unsigned long size, bool validate);

/**
 * Evaluate an xpath expression in the context of the given XML document
 *
 * @param   doc       the XML document for evaluating the xpath expression
 * @param   xpathExpr the xpath expression for evaluation
 * @return  TRUE on successfull xpath evaluation, FALSE otherwise
**/
unsigned int xpathEval(xmlDocPtr doc, const xmlChar* xpathExpr);

/**
 * Execute an xpath expression in the context of the given XML document; in contrast to
 * xpathEval not the boolean value is returned
 *
 * @param   doc       the XML document for evaluating the xpath expression
 * @param   xpathExpr the xpath expression for evaluation
 * @return  the return value of the xpath execution or NULL in case of error (or if result is not a string)
**/
char*        xpathExec(xmlDocPtr doc, const xmlChar* xpathExpr);

/**
 * Search a node with the given name in the given XML node (including its children)
 *
 * @param   node  the XML node for searching (starting node)
 * @param   name  the name for searching
 * @return  the XML node with the given name if found, otherwise NULL
**/
xmlNodePtr   xmlFindNode(const xmlNodePtr node, const char *name);

/**
 * Checks whether the given XML node is an element node
 *
 * @param   node  the XML node to check
 * @return  TRUE if node is an element node, FALSE otherwise
**/
bool         xmlIsElement(xmlNodePtr node);

/**
 * Retrieves the first child node, which is an element node
 *
 * @param   node  the XML node to search for its first child-element
 * @return  the first child element node if present, NULL otherwise
**/
xmlNodePtr   xmlGetFirstChildElement(xmlNodePtr node);

/**
 * Internal method to print the last error, which occurred during XML processing
**/
void         xmlPrintError();

/**
 * Print all node names of element nodes starting from the given root node
 *
 * @param   node  the starting XML node
**/
void         xmlPrintElementNames(const xmlNodePtr node);

/**
 * Dump the XML node and its children; the dump internally uses log_trace to print
 *
 * @param   node  the XML node to dump
**/
void         dumpXMLnode(const xmlNodePtr node);

/**
 * Dump the entire XML document; internally uses log_trace
 *
 * @param   doc  the XML document to dump
**/
xmlBufferPtr dumpXMLdoc(const xmlDocPtr doc);

#endif





