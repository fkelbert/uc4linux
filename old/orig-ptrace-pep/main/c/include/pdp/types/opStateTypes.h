/**
 * @file  opStateTypes.h
 * @brief Type definition header for state related implementation
 *
 * Provides type and structure definitions for operator internal state
 *
 * @author cornelius moucha
 **/

#ifndef OPSTATETYPES_H_
#define OPSTATETYPES_H_

// Forward definitions
typedef struct state_s        state_t;
typedef        state_t        *state_ptr;
typedef struct circArray_s     circArray_t;
typedef        circArray_t    *circArray_ptr;
typedef struct circIntArray_s  circIntArray_t;
typedef        circIntArray_t *circIntArray_ptr;

/**
 * @struct circArray_s
 * @brief Circular bit array
**/
struct circArray_s
{
  /// Type of this circular array: bit or integer array
  unsigned int   type;
  /// Values of this circular array
  unsigned char *values;
  /// Size of this circular array
  unsigned int   size;
  /// First position
  unsigned int   p_first;
  /// Last position
  unsigned int   p_next;
};

/**
 * @struct circIntArray_s
 * @brief Circular integer array
**/
struct circIntArray_s
{
  /// Type of this circular array: bit or integer array
  unsigned int  type;
  /// Values of this circular array
  unsigned int *values;
  /// Size of this circular array
  unsigned int  size;
  /// First position
  unsigned int  p_first;
  /// Last position
  unsigned int  p_next;
};

/**
 * @struct state_s
 * @brief State of an OSL operator
 *
 * The operator state refers to the concrete state in the state automaton of an operator in
 * the mechanism's condition. It holds information about the history of states for temporal operators,
 * cardinal information and the current state value.
**/
struct state_s
{
  /// indicates the state value of the condition operator (TRUE/FALSE)
  bool          value;
  /// flag whether the state is immutable; e.g. a satisfied repmax will never change again (repmax, since, always)
  unsigned char immutable;

  /// Internal state flag; used as reference whether subformula was ever true/false in the past (since)
  bool          subEverTrue;
  /// Internal state counter; used for counting purposes in repmax, replim
  unsigned long counter;

  /// circular array for this operator; used for temporal operators
  union
  {
    circArray_ptr    bitArray;
    circIntArray_ptr intArray;
  } savedValues;
};



#endif /* OPSTATETYPES_H_ */











