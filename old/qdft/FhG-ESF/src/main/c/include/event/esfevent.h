/**
 * @file event.h
 * @brief Header for event processing
 *
 * creating, deleting event objects;\n
 * handling event parameters;\n
 * handling XML information for appropriate events
 *
 * @author Cornelius Moucha, Ricardo Neisse
 **/

#ifndef event_h
#define event_h

#include <glib.h>
#include <string.h>
#include <sys/types.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "base.h"
#include "logger.h"
#include "mem_utils.h"
#include "xml_utils.h"
#include "eventTypes.h"
#include "data_flow_monitor.h"

#define IDX_START   0
#define IDX_ONGOING 1

#define PARAM_CONTUSAGE 0
#define PARAM_DATAUSAGE 1

static char* const event_index_str[]={"first", "ongoing"};

#define MATCH_FIRST 0
#define MATCH_ALL   1

static char* const event_match_str[]={"fst", "all"};

static char* const paramUsageTypeStr[]={"ContUsage", "DataUsage"};
static char* const paramDataTypeStr[]={"string", "int", "address"};

event_ptr       event_new(action_desc_ptr action_desc, uint index, bool is_try);

//unsigned int    event_add_param(event_ptr event, action_param_desc_ptr param_desc, unsigned int usageType, char *value);
unsigned int    event_add_param(event_ptr event, action_param_desc_ptr param_desc, unsigned int usageType, unsigned int paramType, event_paramType_ptr paramValue);
unsigned int    event_add_paramInt(event_ptr event, const unsigned char *paramName, unsigned int usageType, unsigned int value);
unsigned int    event_add_paramString(event_ptr event, const unsigned char *paramName, unsigned int usageType, unsigned char *value);
void            event_param_free(gpointer data, gpointer user_data); // only for internal use
void            event_param_log(gpointer key, gpointer value, gpointer user_data);
void            event_log(char *msg, event_ptr event);
event_param_ptr event_find_param_by_name(event_ptr event, char *param_desc_name);
//unsigned char *event_get_param_value_by_name(event_ptr event, char *param_desc_name);
event_paramType_ptr event_get_param_value_by_name(event_ptr event, char *param_desc_name);
unsigned char *event_get_param_value_by_name_string(event_ptr event, char *param_desc_name);
int            event_get_param_value_by_name_int(event_ptr event, char *param_desc_name);
event_match_op_ptr  event_match_parse_xml(action_desc_store_ptr action_desc_store, xmlNodePtr rootNode);
event_ptr           event_parse_xml(action_desc_store_ptr action_desc_store, xmlDocPtr xmlDoc);
uint                event_free(event_ptr event);
unsigned long       event_getMemSize(event_ptr);

/*event_trace_ptr event_trace_new();
void event_trace_event_free(gpointer data, gpointer user_data); // only for internal use
uint event_trace_add(event_trace_ptr event_trace, event_ptr event);
uint event_trace_remove(event_trace_ptr event_trace, event_ptr event);
uint event_trace_free(event_trace_ptr event_trace);

timestep_window_ptr timestep_window_new();
uint timestep_window_empty(timestep_window_ptr window);
uint timestep_window_add_event(timestep_window_ptr window, event_ptr event);

discrete_trace_ptr discrete_trace_new();
timestep_window_ptr discrete_trace_next_window(discrete_trace_ptr discrete_trace);
timestep_window_ptr dicrete_trace_get_window(discrete_trace_ptr discrete_trace, uint time_shift);
uint discrete_trace_free(discrete_trace_ptr discrete_trace);*/

event_match_op_ptr event_match_op_new(action_desc_ptr match_action, uint match_index, bool match_try);
uint event_match_add_param(event_match_op_ptr event_match, action_param_desc_ptr param_desc, char *value, unsigned int type, bool is_xpath);
bool event_matches(event_match_op_ptr event_match, event_ptr event, char *xml_doc);

unsigned int event_createXMLdoc(event_ptr);

#endif
