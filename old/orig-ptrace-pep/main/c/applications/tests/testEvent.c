/**
 * @file  testEvent.c
 * @brief Exemplary tests for event implementation: pefEvent.c, pefEventMatch.c, pefParam.c, pefParamMatch.c
 *
 * @author cornelius moucha
 **/

#include <glib.h>
#include "pdp.h"
#include "action.h"
#include "pefEvent.h"
#include "pefEventMatch.h"
#include "log_testEvent_pef.h"

GHashTable *pdpContextsTable=NULL;

int main(int argc, char **argv)
{

  // Store and action descriptions
  actionDescStore_ptr   actionDescriptionStore;
  actionDescription_ptr openDesc,  unlinkDesc, actionDesc;
  paramDescription_ptr  paramDesc, paramDesc2;
  char *paramValue;

  // event
  event_ptr event, event1;

  // Action description open
  openDesc=actionDescriptionNew("open", ACTION_USAGE);
  actionDescAddParam(openDesc, "filename", "string");
  actionDescAddParam(openDesc, "username", "string");
  addParamValue(openDesc, "filename", "/etc/passwd");

  // Action description store
  actionDescriptionStore=actionDescriptionStoreNew();
  actionDescriptionStore->add(actionDescriptionStore, openDesc);

  actionDesc=actionDescriptionFind(actionDescriptionStore, "open", FALSE);
  paramDesc=actionDescFindParam(actionDesc, "filename", FALSE);
  paramDesc2=actionDescFindParam(actionDesc, "username", FALSE);

  // testing event methods
  log_info("Testing event methods...");
  event=eventNew(actionDesc, TRUE);
  eventAddParamString(event, "filename", "\\etc\\passwd");
  eventAddParamString(event, "username", "neisse");
  eventFindParam(event, "filename");

  eventLog(NULL, NULL);
  eventLog("msg: ", NULL);
  eventLog(NULL, event);
  eventLog("msg: ", event);

  event1=eventNew(NULL, TRUE);
  log_info("event1a=[%p]", event1);
  eventLog("msg: ", event1);
  event1=eventNew(actionDesc, 99);
  log_info("event1b=[%p]", event1);
  eventLog("msg: ", event1);

  eventCreateXMLdoc(NULL);
  eventCreateXMLdoc(event1);
  eventCreateXMLdoc(event);

  event1=eventNew(actionDesc, TRUE);
  log_info("event1=[%p]", event1);

  eventAddParamString(NULL, NULL, NULL);
  eventAddParamString(event1, NULL, NULL);
  eventAddParamString(event1, "abc", NULL);
  eventAddParamString(event1, "abc", "def");
  log_info("addParamString finished");

  eventAddParamInt(NULL, NULL, 99);
  eventAddParamInt(event1, NULL, 99);
  eventAddParamInt(event1, "abcInt", 99);
  log_info("addParamInt finished");

  eventAddParam(NULL, NULL, PARAM_STRING, NULL);
  eventAddParam(event1, NULL, PARAM_STRING, NULL);
  eventAddParam(event1, paramDesc, PARAM_STRING, NULL);
  log_info("addParam finished");

  eventParam_ptr eventParam;
  eventParam=eventFindParam(NULL, NULL);
  eventParam=eventFindParam(event1, NULL);
  eventParam=eventFindParam(NULL, "qwerty");
  eventParam=eventFindParam(event1, "qwerty");
  eventParam=eventFindParam(event1, "abc");
  log_info("findParam finished");


  eventParamValue_ptr paramValueType;
  paramValueType=eventGetParamValue(NULL, NULL);
  paramValueType=eventGetParamValue(event1, NULL);
  paramValueType=eventGetParamValue(NULL, "qwerty");
  paramValueType=eventGetParamValue(event1, "qwerty");
  paramValueType=eventGetParamValue(event1, "abc");
  log_info("getParamValue finished");

  char* paramValueString;
  paramValueString=eventGetParamValueString(NULL, NULL);
  paramValueString=eventGetParamValueString(event1, NULL);
  paramValueString=eventGetParamValueString(NULL, "qwerty");
  paramValueString=eventGetParamValueString(event1, "qwerty");
  paramValueString=eventGetParamValueString(event1, "abc");
  log_info("getParamValueString finished");

  int paramValueInt;
  paramValueInt=eventGetParamValueInt(NULL, NULL);
  paramValueInt=eventGetParamValueInt(event1, NULL);
  paramValueInt=eventGetParamValueInt(NULL, "qwerty");
  paramValueInt=eventGetParamValueInt(event1, "qwerty");
  paramValueInt=eventGetParamValueInt(event1, "abcInt");
  log_info("getParamValueInt finished");

  eventFree(NULL);
  eventFree(event1);

  // testing eventMatch methods
  log_info("eventMatch tests");
  eventMatch_ptr trigger;
  trigger=eventMatchNew(NULL, 99);
  trigger=eventMatchNew(actionDesc, 99);
  trigger=eventMatchNew(actionDesc, 99);
  trigger=eventMatchNew(actionDesc, 99);
  trigger=eventMatchNew(actionDesc, TRUE);
  trigger=eventMatchNew(actionDesc, TRUE);

  trigger=eventMatchNew(actionDesc, TRUE);
  eventMatchAddParam(NULL, NULL, NULL, 99, FALSE);
  eventMatchAddParam(trigger, NULL, NULL, 99, FALSE);
  eventMatchAddParam(trigger, paramDesc, NULL, 99, FALSE);
  eventMatchAddParam(trigger, paramDesc, "value", 99, FALSE);
  eventMatchAddParam(trigger, paramDesc, "value", 99, FALSE);
  eventMatchAddParam(trigger, paramDesc, "value", PARAM_STRING, FALSE);

  eventMatchAddParam(trigger, paramDesc, "\\etc\\passwd", PARAM_STRING, FALSE);
  eventMatchAddParam(trigger, paramDesc2, "neisse", PARAM_STRING, FALSE);

  log_info("trying to match eventMatch against event 1");
  eventMatches(NULL, NULL);
  log_info("trying to match eventMatch against event 2");
  eventMatches(trigger, NULL);
  log_info("trying to match eventMatch against event 3");
  eventMatches(NULL, event);
  log_info("trying to match eventMatch against event 4");
  eventMatches(trigger,event);
  log_info("eventMatch finished");

  eventMatchFree(NULL, NULL);
  eventMatchFree(trigger, NULL);

  actionDescriptionStoreFree(actionDescriptionStore);
}
