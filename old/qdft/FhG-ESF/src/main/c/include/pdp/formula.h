#ifndef formula_h
#define formula_h

#include "base.h"
#include "xml_utils.h"
#include "esfevent.h"

#define OSL_FALSE    0
#define OSL_TRUE     1
#define NOT          4
#define AND          5
#define OR           6
#define IMPLIES      7

/// one event_ptr operand, if check_refinement(new_event, operand[0])
#define EVENTMATCH   2
#define SINCE        8
#define ALWAYS       9
#define BEFORE      10
#define DURING      11
#define WITHIN      12
#define REPLIM      13
#define REPSINCE    14
#define REPMAX      15

/// one char* operand, state->value = xpath_eval(event->xmlDoc, operand[0]);
#define XPATH       16
#define NO_OSL_OP   99

static char* const operatorNames[]={"FALSE","TRUE","EVENTMATCH","","NOT","AND","OR","IMPLIES",
                                    "SINCE","ALWAYS","BEFORE","DURING","WITHIN","REPLIM","REPSINCE","REPMAX","XPATH"};

/******************************************************************************
 *  Circular array definitions (bit-array)
******************************************************************************/
typedef struct c_array_s  c_array_t;
typedef c_array_t        *c_array_ptr;

struct c_array_s
{
  unsigned char *values;
  unsigned int   p_first;  // first position
  unsigned int   p_next;   // last position
};

c_array_ptr   c_array_new(unsigned long);
unsigned int  c_array_free(c_array_ptr);
bool          c_array_get(c_array_ptr,unsigned long);
void          c_array_set(c_array_ptr,unsigned long,bool);
void          c_array_log(c_array_ptr);

/******************************************************************************
 *  Circular array definitions (int-array)
******************************************************************************/
typedef struct cint_array_s  cint_array_t;
typedef cint_array_t        *cint_array_ptr;

struct cint_array_s
{
  unsigned int *values;
  unsigned int  p_first;  // first position
  unsigned int  p_next;   // last position
};

cint_array_ptr cint_array_new(unsigned long);
unsigned int   cint_array_free(cint_array_ptr);
unsigned int   cint_array_get(cint_array_ptr,unsigned long);
void           cint_array_set(cint_array_ptr,unsigned long,unsigned int);
void           cint_array_log(cint_array_ptr);

/******************************************************************************
 *  State definitions
******************************************************************************/
typedef struct state_s  state_t;
typedef state_t        *state_ptr;


typedef union cArray_s cArray_t;
typedef cArray_t  *cArray_ptr;

union cArray_s
{
  c_array_ptr    bitArray;
  cint_array_ptr intArray;
};

struct state_s
{
  bool          value;            // indicates the (sub)formula's value (TRUE/FALSE)
  unsigned char immutable;
  unsigned int  count_true;
  bool          was_ever_true;    // used by always, if the formula was ever ?FALSE?
  union
  {
    c_array_ptr    bitArray;
    cint_array_ptr intArray;
  } saved_values;
};

state_ptr     state_new(bool);
unsigned int  state_free();
unsigned long state_getMemSize(state_ptr);
void          state_log(state_ptr);

/******************************************************************************
 *  Operator definitions
******************************************************************************/
typedef struct operator_s  operator_t;
typedef operator_t        *operator_ptr;

struct operator_s
{
  unsigned char type;
  void*         operands;   // type tells us how to process the operands; they can be e.g. operator_ptr, char *, event_ptr
  state_ptr     state;
  bool          (*eval)(operator_ptr, event_ptr, void*);
};

operator_ptr  operator_new(unsigned char);
unsigned int  operator_free(operator_ptr);
uint8_t       getOperatorType(const char *);
unsigned long operator_getMemSize(operator_ptr);
void          operator_log(operator_ptr);

/******************************************************************************
 *  Condition formula definitions
******************************************************************************/
typedef struct formula_s  formula_t;
typedef formula_t        *formula_ptr;

// conf parameters: delay until end of timestep; pass through and return as soon as possible; return UNDECIDABLE
struct formula_s
{
  unsigned int  n_nodes;
  operator_ptr *nodes;    // ordered by a left right depth first traversal - post-order
};

formula_ptr   formula_new();
unsigned int  formula_free(formula_ptr);
formula_ptr   formula_parse_xml(xmlNodePtr);
unsigned int  formula_parse_stage1(formula_ptr, xmlNodePtr);
operator_ptr  parse_subformula(formula_ptr, xmlNodePtr, unsigned int *);
unsigned long formula_getMemSize(formula_ptr);
void          formula_log(formula_ptr);
void          formula_logRecursive(formula_ptr, unsigned int *, unsigned int);

bool          update_formula( void*, formula_ptr, event_ptr);
bool          eval_true(operator_ptr, event_ptr, void*);
bool          eval_false(operator_ptr, event_ptr, void*);
bool          eval_or(operator_ptr, event_ptr, void*);
bool          eval_and(operator_ptr, event_ptr, void*);
bool          eval_impl(operator_ptr, event_ptr, void*);
bool          eval_not(operator_ptr, event_ptr, void*);

// more or less copied from old past_monitor! has to be checked!
bool          eval_before(operator_ptr, event_ptr, void*);
bool          eval_event(operator_ptr, event_ptr, void*);
bool          eval_xpath(operator_ptr, event_ptr, void*);
bool          eval_always(operator_ptr, event_ptr, void*);
bool          eval_within(operator_ptr, event_ptr, void*);
bool          eval_during(operator_ptr, event_ptr, void*);
bool          eval_replim(operator_ptr, event_ptr, void*);
bool          eval_since(operator_ptr, event_ptr, void*);
bool          eval_repsince(operator_ptr, event_ptr, void*);
bool          eval_repmax(operator_ptr, event_ptr, void*);

#endif
