/*
 * xml_utils.h
 *
 *      Author: Ricardo Neisse
 */

#ifndef xml_utils_h
#define xml_utils_h

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>

#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>

#include "const.h"
#include "logger.h"
#include "str_utils.h"

xmlSchemaPtr loadXmlSchema(char *schemaFilename);

xmlDocPtr loadXmlDoc(char *docFilename, char *schemaFilename);

char *getText(xmlNode * e_node);

xmlNode *next_valid(xmlNode *node);

void xml_dump (FILE *tmp_file, xmlNode * node);

void print_element_names(xmlNode * a_node);

int exec_xpath(xmlDocPtr doc, const xmlChar* xpathExpr);

#endif
