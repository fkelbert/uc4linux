/**
 * @file  opState.h
 * @brief Header for the implementation of operator states
 *
 * The operator state refers to the concrete state in the state automaton of an operator in
 * the mechanism's condition. It holds information about the history of states for temporal operators,
 * cardinal information and the current state value.
 *
 * @author cornelius moucha
 **/

#ifndef OPSTATE_H_
#define OPSTATE_H_

#include "memUtils.h"
#include "pdpTypes.h"

/*******************************************************************************************************
 *  Circular array definitions (bit-array)
*******************************************************************************************************/
/// flag whether circular array is a bit or integer array (bit => 0; integer => 1)
#define cARRAY_BITARRAY 0
/// flag whether circular array is a bit or integer array (bit => 0; integer => 1)
#define cARRAY_INTARRAY 1

/**
 * Creates a new circular bit array
 * @param   size   size of the circular array
 * @return  reference to created circular array on success or NULL otherwise
**/
circArray_ptr circArrayNew(unsigned long size);

/**
 * Deallocates a circular bit array structure \n
 * Only intended for internal usage! This method is called when the superior state structure is deallocated.
 * @param   array   circular array for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  circArrayFree(circArray_ptr array);

/**
 * Retrieve the value at the given position in the circular array
 * @param   array   reference to the circular array
 * @param   pos     the position whose value should be retrieved
 * @return  the value at the given position in the circular array
**/
bool          circArrayGet(circArray_ptr array,unsigned long pos);

/**
 * Set the value at the given position in the circular array
 * @param   array   reference to the circular array
 * @param   pos     the position whose value should be retrieved
 * @param   val     new value for the given position
**/
void          circArraySet(circArray_ptr array,unsigned long pos, bool val);

/**
 * Logging of a circular array\n
 * Only intended for internal usage! This method is called when the superior state is prepared for logging.
 * @param   array       reference to the circular array for logging
**/
void          circArrayLog(circArray_ptr array);

/**
 * Logging of a circular array; preparing a string representing the circular array's content
 * Only intended for internal usage! This method is called when the superior state is prepared for logging.
 * @param   array       reference to the circular array for logging
 * @return  the prepared string representation of the circular array
**/
char*         circArrayString(circArray_ptr array);

/*******************************************************************************************************
 *  Circular array definitions (int-array)
*******************************************************************************************************/

/**
 * Creates a new circular integer array
 * @param   size   size of the circular array
 * @return  reference to created circular array on success or NULL otherwise
**/
circIntArray_ptr circIntArrayNew(unsigned long size);

/**
 * Deallocates a circular integer array structure \n
 * Only intended for internal usage! This method is called when the superior state structure is deallocated.
 *
 * @param   array   circular array for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int     circIntArrayFree(circIntArray_ptr array);

/**
 * Retrieve the value at the given position in the circular array
 *
 * @param   array   reference to the circular array
 * @param   pos     the position whose value should be retrieved
 * @return  the value at the given position in the circular array
**/
unsigned int     circIntArrayGet(circIntArray_ptr array, unsigned long pos);

/**
 * Set the value at the given position in the circular array
 *
 * @param   array   reference to the circular array
 * @param   pos     the position whose value should be retrieved
 * @param   val     new value for the given position
**/
void             circIntArraySet(circIntArray_ptr array, unsigned long pos, unsigned int val);

/**
 * Logging of a circular array\n
 * Only intended for internal usage! This method is called when the superior state is prepared for logging.
 * @param   array       reference to the circular array for logging
**/
void             circIntArrayLog(circIntArray_ptr array);

/**
 * Logging of a circular array; preparing a string representing the circular array's content
 * Only intended for internal usage! This method is called when the superior state is prepared for logging.
 * @param   array       reference to the circular array for logging
 * @return  the prepared string representation of the circular array
**/
char*           circIntArrayString(circIntArray_ptr array);

/*******************************************************************************************************
 *  State definitions
*******************************************************************************************************/

/**
 * Creates a new state
 * @param   value   the initial value of the operator's state
 * @return  reference to created state on success or NULL otherwise
**/
state_ptr     stateNew(bool value);

/**
 * Deallocates a state structure \n
 * Only intended for internal usage! This method is called when the superior mechanism, resp. the condition, structure is deallocated.
 * @param   state  state for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  stateFree(state_ptr state);

/**
 * Profiling method calculating the allocated memory size of the given state structure
 * @param   state   state structure for calculation
 * @return  the size of the allocated memory for this state
**/
unsigned long stateMemSize(state_ptr state);

/**
 * Logging of a state\n
 * Only intended for internal usage! This method is called when the superior condition is prepared for logging.
 * @param   state       reference to the state for logging
**/
void          stateLog(state_ptr state);

#endif /* OPSTATE_H_ */




