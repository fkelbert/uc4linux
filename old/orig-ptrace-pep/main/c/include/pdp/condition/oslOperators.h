/**
 * @file  oslOperators.h
 * @brief Header for OSL operators implementation
 *
 * The OSL operators specify the condition which has to be evaluated by the PDP upon incoming events and
 * time based, depending on the mechanism's notion of time.
 *
 * @author cornelius moucha
 **/

#ifndef OSLOPERATORS_H_
#define OSLOPERATORS_H_

#include "pefEvent.h"
#include "pefEventMatch.h" // for eventMatchFree
#include "opState.h"
#include "oslEval.h"

/******************************************************************************
 *  Operator definitions
******************************************************************************/
/**
 * Creates a new operator based on the given argument, i.e. the operator type (e.g. OSL_TRUE, BEFORE, ...)
 * @param   opType  the operator type (e.g. OSL_TRUE, BEFORE, ...)
 * @return  reference to created operator on success or NULL otherwise
**/
operator_ptr  operatorNew(const char opType);

/**
 * Deallocates an operator\n
 * Only intended for internal usage! This method is called when the superior condition structure is deallocated.
 * @param   op      operator for deallocation
 * @param   curMech reference to the mechanism using the operator's associated condition;
 *                  required for unsubscribing the mechanism in case of a contained eventMatch operator
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  operatorFree(operator_ptr op, mechanism_ptr curMech);

/**
 * Determine the operator type based on the given string
 * @param   opName   string representation of the operator type
 * @return  operator type using the predefined integer values (e.g. OSL_TRUE, BEFORE, ...)
**/
uint8_t       getOperatorType(const char *opName);

/**
 * Profiling method calculating the allocated memory size of the given operator structure
 * @param   op   operator structure for calculation
 * @return  the size of the allocated memory for this operator
**/
unsigned long operatorMemSize(operator_ptr op);

/**
 * Logging of an operator\n
 * Only intended for internal usage! This method is called when the superior condition is prepared for logging.
 * @param   op  operator, which should be logged
**/
void          operatorLog(operator_ptr op);

#endif /* OSLOPERATORS_H_ */










