/**
 * @file  executeAction.h
 * @brief Header for executeAction implementation
 *
 * Execute actions refer to additional actions which have to be executed. An authorization action may require
 * an successful execution of an action for an allowing of the intercepted event. Furthermore execute actions
 * can be specified in the policy independent of the decision, e.g. for notifications, which should be executed
 * whenever the condition is satisfied.
 *
 * @author cornelius moucha
 **/

#ifndef EXECUTEACTION_H_
#define EXECUTEACTION_H_

#include "xmlUtils.h"
#include "actionDescStore.h"
#include "pdpInternal.h"

/******************************************************************************
 *  ExecuteAction definitions
******************************************************************************/

/**
 * Creates a new executeAction based on the given argument, i.e. the action name
 * @param   name  the name of the execute action
 * @return  reference to created execute action on success or NULL otherwise
**/
executeAction_ptr executeActionNew(const char *name);

/**
 * Deallocates an execute action\n
 * Only intended for internal usage! This method is called when the superior mechanism structure is deallocated.
 * @remark Although a reference to the execute action is also contained in the notify response structure this should
 *         NOT be deallocated, because it is just a reference, not a copy!
 *
 * @param   execAction  execute action for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int      executeActionFree(executeAction_ptr execAction);

/**
 * Parse a given XML document to instantiate a new execute action
 * Internally called when a policy is parsed using mechanismParseXML
 *
 * @param   mech  reference to the mechanism of this execute action
 * @param   node  XML node for parsing
 * @return  reference to created execute action on success or NULL otherwise
**/
unsigned int      executeActionParseXML(mechanism_ptr mech, const xmlNodePtr node);

/**
 * Profiling method calculating the allocated memory size of the given execute action structure
 *
 * @param   action   execute action structure for calculation
 * @return  the size of the allocated memory for this execute action
**/
unsigned long     executeActionMemSize(const executeAction_ptr action);

/**
 * Adding a parameter instance, i.e. a key-value pair specifying the parameters for this action,
 * to the execute action\n
 * In contrast to the parameter descriptions used in the action description store, these parameter
 * instances are actual, concrete parameters with a defined name and value, instead of a list of allowed values, which
 * is the case for the parameter descriptions.
 *
 * @param   lAction  reference to the execute action for the new parameter instance
 * @param   name     name of the parameter
 * @param   value    value of the parameter
 * @param   type     type of parameter (STRING or e.g. xpath)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int      executeActionAddParamInstance(executeAction_ptr lAction, const char *name,
                                                const char *value, const char *type);

/**
 * Creates a new parameter instance based on the given argument, i.e. the parameter description as global reference
 * to the name, allowed values etc. and the concrete value of this instance\n
 *
 * @param   paramDesc  reference to the parameter description used in the action description store
 * @param   value      the value of the parameter instance
 * @param   type       type of parameter instance (STRING or e.g. xpath)
 * @return  reference to created parameter instance on success or NULL otherwise
**/
parameterInstance_ptr parameterInstanceNew(paramDescription_ptr paramDesc, const char *value, const char *type);

/**
 * Deallocates an parameter instance\n
 *
 * @param   param  parameter instance for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int          parameterInstanceFree(parameterInstance_ptr param);

#endif /* EXECUTEACTION_H_ */




