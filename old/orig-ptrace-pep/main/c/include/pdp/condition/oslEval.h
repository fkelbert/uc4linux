/**
 * @file  oslEval.h
 * @brief Header for OSL operators evaluation implementation
 *
 * Define internaly used structures for the different operator types and method declarations for the
 * evaluation methods.
 *
 * @author cornelius moucha
 **/

#ifndef OSLEVAL_H_
#define OSLEVAL_H_

#include "base.h"
#include "opState.h"
#include "pefEventMatch.h"
#include "pdpTypes.h"

#define OSL_FALSE    0
#define OSL_TRUE     1
#define EVENTMATCH   2
#define NOT          4
#define AND          5
#define OR           6
#define IMPLIES      7
#define SINCE        8
#define ALWAYS       9
#define BEFORE      10
#define DURING      11
#define WITHIN      12
#define REPLIM      13
#define REPSINCE    14
#define REPMAX      15
#define XPATH       16
#define NO_OSL_OP   99

/// static textual representation of osl operators; used for logging
static char* const operatorNames[]={"FALSE","TRUE","EVENTMATCH","","NOT","AND","OR","IMPLIES",
                                    "SINCE","ALWAYS","BEFORE","DURING","WITHIN","REPLIM","REPSINCE","REPMAX","XPATH"};


// Forward definitions
typedef struct oslTimeUnary_s  oslTimeUnary_t;
typedef        oslTimeUnary_t *oslTimeUnary_ptr;

typedef struct oslRepMax_s  oslRepMax_t;
typedef        oslRepMax_t *oslRepMax_ptr;

typedef struct oslRepLim_s  oslRepLim_t;
typedef        oslRepLim_t *oslRepLim_ptr;

typedef struct oslRepSince_s  oslRepSince_t;
typedef        oslRepSince_t *oslRepSince_ptr;

typedef struct oslBinary_s  oslBinary_t;
typedef        oslBinary_t *oslBinary_ptr;

/**
 * @struct oslTimeUnary_s
 * @brief Structure for representation of a time-bounded unary operator
 *
 * This structure represents a time-bounded unary OSL-operator as defined in the XML schema. Internally this structure is used
 * for the operators: before, during and within. The structure fields amount and unit are only stored with parsing the policy
 * for logging purposes and are not involved in the evaluation.
 *
**/
struct oslTimeUnary_s
{
  ///  Operator time interval relative to mechanism's timestepSize
  uint64_t       interval;
  /// Amount attribute specified in policy; only kept for logging
  unsigned long  amount;
  /// Time unit attribute specified in policy; only kept for logging
  char          *unit;
  /// Child operand
  operator_ptr   operand;
};

/**
 * @struct oslBinary_s
 * @brief Structure for representation of a binary operator
 *
 * This structure represents a binary OSL-operator as defined in the XML schema. Internally this structure is used
 * for the operators: and, or, implies and since.
 *
**/
struct oslBinary_s
{ /// Child operands
  operator_ptr operand[2];
};

/**
 * @struct oslRepmax_s
 * @brief Structure for representation of a repmax-operator
 *
 * This structure represents an OSL-repmax operator as defined in the XML schema.
**/
struct oslRepMax_s
{ ///
  unsigned long  upperLimit;
  /// Child operand
  operator_ptr   operand;
};

/**
 * @struct oslReplim_s
 * @brief Structure for representation of a replim-operator
 *
 * This structure represents an OSL replim-operator as defined in the XML schema. The structure fields amount and unit
 * are only stored with parsing the policy for logging purposes and are not involved in the evaluation.
 *
**/
struct oslRepLim_s
{
  ///  Operator time interval relative to mechanism's timestepSize
  uint64_t       interval;
  /// Amount attribute specified in policy; only kept for logging
  unsigned long  amount;
  /// Time unit attribute specified in policy; only kept for logging
  char          *unit;

  unsigned long lowerLimit;
  unsigned long upperLimit;

  /// Child operand
  operator_ptr  operand;
};


/**
 * @struct oslRepSince_s
 * @brief Structure for representation of a repSince-operator
 *
 * This structure represents an OSL repsince-operator as defined in the XML schema.
**/
struct oslRepSince_s
{
  unsigned long limit;
  operator_ptr operand[2];
};


/****************************************************************************************************************
 *  Operator evaluation methods: Propositional operators
****************************************************************************************************************/
/**
 * Evaluation of the OSL operator: TRUE\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_true(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: FALSE\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_false(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: OR\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_or(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: AND\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_and(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: IMPLIES\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_impl(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: NOT\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_not(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);


/****************************************************************************************************************
 *  Operator evaluation methods: event, xpath operators
****************************************************************************************************************/
/**
 * Evaluation of the OSL operator: EVENTMATCH\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_event(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: XPATH\n
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_xpath(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/****************************************************************************************************************
 *  Operator evaluation methods: Temporal operators
****************************************************************************************************************/
// more or less copied from old past_monitor! has to be checked!

/**
 * Evaluation of the OSL operator: BEFORE\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_before(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: ALWAYS\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_always(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: WITHIN\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_within(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: DURING\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_during(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: SINCE\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_since(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: REPSINCE\n
 * @todo check semantics of evaluation for REPSINCE operator
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_repsince(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/****************************************************************************************************************
 *  Operator evaluation methods: Cardinal operators
****************************************************************************************************************/
/**
 * Evaluation of the OSL operator: REPLIM\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_replim(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);

/**
 * Evaluation of the OSL operator: REPMAX\n
 *
 * @param   curop     the current operator for evaluation
 * @param   curEvent  the event against which the operator should be evaluated or NULL in case of time-triggered evaluation
 * @param   mech      reference to the mechanism using the operator's associated condition
 * @return  evaluation result for this condition operator
**/
bool          eval_repmax(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech);


#endif /* OSLEVAL_H_ */




