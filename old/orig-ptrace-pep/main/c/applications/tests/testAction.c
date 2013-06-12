/**
 * @file  testAction.c
 * @brief Exemplary tests for action implementation: action.c, actionDescStore.c, paramDesc.c
 *
 * @author cornelius moucha, Ricardo Neisse
**/

#include "actionDescStore.h"
#include "pdpCommInterfaces.h"
#include "log_testAction_pef.h"

pdpInterface_ptr pdpInterfaceStub;

int main(int argc, char **argv)
{
  actionDescStore_ptr actionDescStore=actionDescriptionStoreNew();
  if(actionDescStore==NULL) log_warn("Error creating action description store.");

  actionDescription_ptr openDesc, unlinkDesc, testDesc, actionDesc;
  paramDescription_ptr paramDesc;
  char *paramValue;

  // Action description open
  openDesc=actionDescriptionNew("open", ACTION_USAGE);
  actionDescAddParam(openDesc, "filename", "string");
  addParamValue(openDesc, "filename", "/etc/passwd");
  addParamValue(openDesc, "filename", "/etc/passwd");
  addParamValue(openDesc, "filename", "/etc/tests");

  testDesc=actionDescriptionNew(NULL, 0);
  if(testDesc==NULL) log_warn("error creating actionDescription");
  testDesc=actionDescriptionNew("test", ACTION_SIGNALLING);
  actionDescAddParam(NULL, NULL, NULL);
  actionDescAddParam(testDesc, NULL, NULL);
  actionDescAddParam(testDesc, "abc", NULL);
  actionDescAddParam(testDesc, NULL, "def");

  addParamValue(NULL, NULL, NULL);
  addParamValue(testDesc, NULL, NULL);
  addParamValue(testDesc, "abc", NULL);
  addParamValue(testDesc, NULL, "def");

  // Action description unlink
  unlinkDesc=actionDescriptionNew("unlink", ACTION_OTHER);

  // Action description store
  actionDescStore->add(NULL, NULL);
  actionDescStore->add(actionDescStore, NULL);
  actionDescStore->add(actionDescStore, openDesc);
  actionDescStore->add(actionDescStore, unlinkDesc);
  actionDescStore->add(actionDescStore, testDesc);

  actionDescriptionStoreLog(NULL);
  actionDescriptionStoreLog(actionDescStore);
  // calls actionDescriptionLog(...)
  // calls paramDescriptionLog(...);
  // calls paramValueLog(...);

  testDesc=actionDescriptionFind(NULL, NULL, 0);
  testDesc=actionDescriptionFind(actionDescStore, NULL, 0);
  actionDesc=actionDescriptionFind(actionDescStore, "read", TRUE);

  actionDesc=actionDescriptionFind(actionDescStore, "open", FALSE);
  if(actionDesc != NULL)
  {
    paramDesc=actionDescFindParam(NULL, NULL, 0);
    paramDesc=actionDescFindParam(actionDesc, NULL, 0);
    paramDesc=actionDescFindParam(actionDesc, "abcdef", TRUE);

    paramDesc=actionDescFindParam(actionDesc, "does_not_exist", FALSE);
    paramDesc=actionDescFindParam(actionDesc, "does_not_exist", FALSE);
    paramDesc=actionDescFindParam(actionDesc, "filename", FALSE);
    if(paramDesc != NULL)
    {
      paramValue=paramValueFind(NULL, NULL);
      paramValue=paramValueFind(paramDesc, NULL);

      paramValue=paramValueFind(paramDesc, "/etc/does_not_exist");
      paramValue=paramValueFind(paramDesc, "/etc/passwd");
    }
  }

  actionDescriptionStoreFree(actionDescStore);
  // calls actionDescriptionFree(gpointer data, gpointer userData);
  // calls paramDescriptionFree(gpointer data, gpointer userData);

  return 0;
}
