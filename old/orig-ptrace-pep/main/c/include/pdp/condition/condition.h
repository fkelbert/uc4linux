/**
 * @file  condition.h
 * @brief Header for condition implementation
 *
 * Conditions refer to the OSL formula specified in the mechanisms. These conditions are evaluated upon receiving of a matching
 * event and internally at the end of every timestep (depending on the mechanism's timestep size). The latter evaluation is
 * necessary to model the passing of time in the state machine for the temporal operators. The condition is stored as tree of the
 * single operator nodes, similar to the XML representation in the policy. This enables a traversal walk in post-order
 * (left-right-depth-first) for condition evaluation.
 *
 * @author cornelius moucha
 **/

#ifndef condition_h
#define condition_h

#include "pdpTypes.h"
#include "pefEvent.h"
#include "pefEventMatch.h"
#include "oslOperators.h"

/******************************************************************************
 *  Condition formula definitions
******************************************************************************/
/**
 * Creates a new condition, i.e. allocated the memory for the hierarchy of operator nodes
 * @return  reference to created condition on success or NULL otherwise
**/
condition_ptr conditionNew();

/**
 * Deallocates an condition\n
 * Only intended for internal usage! This method is called when the superior mechanism structure is deallocated.
 * Internally invokes deallocation of the operator nodes.
 *
 * @param   condition   condition structure for deallocation
 * @param   curMech     reference to the mechanism of this condition; required for deallocation of condition operators
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  conditionFree(condition_ptr condition, mechanism_ptr curMech);

/**
 * Parse a given XML document to instantiate a new condition
 * Internally called when a policy is parsed using mechanismParseXML
 *
 * @param   node  XML node for parsing
 * @param   mech  reference to the mechanism of this authorization action
 * @return  reference to created condition on success or NULL otherwise
**/
condition_ptr conditionParseXML(xmlNodePtr node, mechanism_ptr mech);

/**
 * Internal auxiliary method for parsing a given XML document to instantiate a new condition\n
 * Recursively counts condition nodes and searching for parametrized matchingEvents.
 *
 * @param   condition    condition to be evaluated
 * @param   subCondition current sub condition, which is evaluated
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  conditionParseXMLstage1(condition_ptr condition, xmlNodePtr subCondition);

/**
 * Recursively parse a given XML node to instantiate a new operator node for the condition
 * Internally called when a condition is parsed using conditionParseXML
 *
 * @param   condition   reference to the condition which is set up
 * @param   node        XML node for parsing
 * @param   curnode     numerical iterator; indicating the current operator in the list of operators of the condition
 * @param   mechanism   reference to the mechanism of this condition
 * @return  reference to currently created operator node on success or NULL otherwise
**/
operator_ptr  conditionParseSubCondition(condition_ptr condition, xmlNodePtr node, unsigned int *curnode, mechanism_ptr mechanism);


/**
 * Profiling method calculating the allocated memory size of the given condition structure
 *
 * @param   condition   condition structure for calculation
 * @return  the size of the allocated memory for this event
**/
unsigned long conditionMemSize(condition_ptr condition);

/**
 * Logging of an condition\n
 * Internally calls recursively conditionLogRecursive for every operator node
 *
 * @param   condition reference to the condition for logging
**/
void          conditionLog(condition_ptr condition);

/**
 * Auxiliary logging method; recursively called for every operator node in the condition
 *
 * @param   condition reference to the condition for logging
 * @param   a         iterator; indicating the current operator node for logging
 * @param   level     intentation level for pretty printing
**/
void          conditionLogRecursive(condition_ptr condition, unsigned int *a, unsigned int level);

/**
 * Update the condition\n
 * This method triggers an evaluation cycle of the condition, which internally walks over the condition tree in post-order
 * to evaluate the overall condition state. Post order traversal => parent can directly access child's state values.
 *
 * @todo necessary to iterate over all nodes to get final condition state?! should be automatically called due to traversal strategy?!
 *
 * @param   mechanism reference to the mechanism of this condition
 * @param   condition reference to the condition for the evaluation
 * @param   event     reference to the event against which the evaluation should happen
 *                    (reference to the intercepted event or NULL reference in case of the automatically triggered evaluation at the
 *                     end of the timestep)
 * @return  the overall condition evaluation state
**/
bool          conditionUpdate(mechanism_ptr mechanism, condition_ptr condition, event_ptr event);

#endif






