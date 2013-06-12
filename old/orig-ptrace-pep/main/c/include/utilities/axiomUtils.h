/**
 * @file  axiomUtils.h
 * @brief Utilities for axis2; used in Master project
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

// needed for xml scheme validation
#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>

#include "xml_utils.h"

#include "const.h"

typedef axiom_node_t      *axiom_node_ptr;
typedef axiom_element_t   *axiom_element_ptr;
typedef axiom_attribute_t *axiom_attr_ptr;
typedef axis2_char_t      *axiom_char_ptr;
typedef axis2_msg_ctx_t   *axiom_msg_ctx_ptr;

void dumpAxiomNode(const axutil_env_t *env, axiom_node_t *root_node);
void printNodeTypes(const axutil_env_t *env, axiom_node_t *root);

void* axiomFindNode(const axiom_element_ptr, const axiom_node_ptr, const unsigned char *, bool);

axiom_char_ptr axiom_getName(axiom_element_ptr);
axiom_char_ptr axiom_node_getName(axiom_node_ptr);

axiom_element_ptr axiom_getElement(axiom_node_ptr);

// needed for xmlSchemaValidation
void axiomLogHelper(FILE *, unsigned char *msg, ...);

