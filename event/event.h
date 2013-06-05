/*
 * event.h
 *
 *      Author: Ricardo Neisse
 */

#ifndef event_h
#define event_h

#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>

#include "const.h"
#include "logger.h"

// Structure used to describe an event parameter
typedef struct param_s {
  char *param_name;
  char *param_value;
  char *param_type;
  int is_xpath;
} param_t;


// this are the "modes" of the event;
#define M_EFST 0
#define M_EALL 1


// Structure used to describe an event
// To allocate params array use:
// params = (param_t *)malloc(sizeof(param_t) * num_params)
typedef struct event_s {
  char *event_name;
  int desired; // TRUE/FALSE
  int mode; // EFST / EALL
  int n_params;
  param_t *params;
  xmlDocPtr xml_doc; // depends if _new or parse_xml
} event_t;

typedef event_t* event_ptr;

void free_event(event_t* event);
/*int A_is_refinement_of_B (event_t* A, event_t* B);*/
void create_xml_doc(event_t* event);


// assuming type N
/* 

-- 
N_s => structure of N. It should always have an xml_doc structure;
structure N_s {
	// ...
	xmlDocPtr xml_doc; // from libxml2
}
N_t => struct type of N
N_ptr => pointer for N (make code more readable)
N_ptr <- N_new(args) => create new N structure/object and copies all the args, so the caller should free the args after invoking this function;
N_free(N_ptr) => free all the memory allocated in N_new, because we copied everything we are sure there will be no double free happening;
N_ptr <- N_parse_xml(xmlDocPtr xml_doc); => doc is the pointer for the <N> XML element and the function should parse and copy everything from xml_doc. Because xml_doc can be freeed

- Return of _new and _free:
-- _new returns NULL or the pointer for the new object
-- _free returns R_SUCCESS or R_ERROR defined in "const.h"

// Parameters
param_s
param_t
param_ptr
xml_doc is pointer for <param> element, library should know how to parse;
name = malloc(char * 3);
// always copy to guarantee that free function will not crash
param_ptr p = param_new(name, value);
free(name);
param_ptr param_new(char *name, char *value, bool is_xpath);
param


// Events
event_s 
event_t 
event_ptr 
event_new
event_add_par(
typedef event_ptr event_t *;

xmlDocPtr xml_doc = parseXML("event.xml");
event_ptr event = event_parse_xml(xml_doc); // create new event and copy xml doc
// event_new(..); // create new event and instantiate xml doc (see function already done for this)
event_new("login", TRUE, EALL); // name, is_try, mode

SUCCESS / FAILURE <-event_add_param(name, value); // this instantiates and create a new par
// how to manage a dynamic growing list of parameters efficietnly? we might need to query values of parameters in an event, ?Hashtable from GLib is efficient?
// allocate memory every time a new param is added? allocate chunks of pointers for params?
// event_is_param("userId", "ricardo"); // TRUE/FALSE 
//  

event_free(event); // this will free all parameters and all internal structures;

bool ret = event_xpath_eval("//..");
event_a_is_refinement_of_b(event a, event b);
if (ret) {...}
event_free(event);
//

main program that loads events from an XML file

// trace.xml
<?xml version="1.0"?>
<trace>
  <event try="true" mode="EFST" xmlns="http://www.master-fp7.eu/event" >
    <name>event1</name>
    <parameter name="" value=""/>
  </event>
</trace>

main() {
	
	loadXML("trace.xml");
	event_ptr event;
	for (...) {
		if (doc->name == "event") {
			event = event_parse_xml(doc);
			.. print event information ...
			event_free(event);
		}
	}
}
*/


#endif
