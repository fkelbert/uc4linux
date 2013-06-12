/**
 * @file  oslOperators.c
 * @brief Implementation of the OSL operators, contained in the mechanism's condition
 *
 * @see oslOperators.h, operator_s
 *
 * @author cornelius moucha
 **/

#include "oslOperators.h"
#include "log_oslOperators_pef.h"


/******************************************************************************
 *  Operator implementation
******************************************************************************/
operator_ptr operatorNew(const char opType)
{
  operator_ptr nop=memAlloc(sizeof(operator_t));
  if(nop == NULL) {log_error("Could not allocate memory for operator"); return NULL;}

  nop->state=stateNew( (opType==OSL_TRUE ? TRUE : FALSE));
  nop->type=opType;

  // depending on type -> preallocating memory for appropriate number of operands
  switch(opType)
  {
    // tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:  nop->operands=NULL; nop->eval=eval_true; break;
    case OSL_FALSE: nop->operands=NULL; nop->eval=eval_false; break;

    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:       nop->eval=eval_not;
                    break;
    case ALWAYS:    nop->eval=eval_always;
                    break;
    // event:EventMatchingOperatorType  -> ???
    case EVENTMATCH:nop->eval=eval_event;
                    break;
    case XPATH:     nop->eval=eval_xpath;
                    break;
    // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:        nop->operands=memAlloc(sizeof(oslBinary_t));
                    nop->eval=eval_or;
                    break;
    case AND:       nop->operands=memAlloc(sizeof(oslBinary_t));
                    nop->eval=eval_and;
                    break;
    case IMPLIES:   nop->operands=memAlloc(sizeof(oslBinary_t));
                    nop->eval=eval_impl;
                    break;
    case SINCE:     nop->operands=memAlloc(sizeof(oslBinary_t));
                    nop->eval=eval_since;
                    break;

    // tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:    nop->operands=memAlloc(sizeof(oslTimeUnary_t));
                    nop->eval=eval_before;
                    break;
    case DURING:    nop->operands=memAlloc(sizeof(oslTimeUnary_t));
                    nop->eval=eval_during;
                    break;
    case WITHIN:    nop->operands=memAlloc(sizeof(oslTimeUnary_t));
                    nop->eval=eval_within;
                    break;

    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:  nop->operands=memAlloc(sizeof(oslRepSince_t));
                    nop->eval=eval_repsince;
                    break;
    // tns:RepLimOperatorType           -> operands=special (1operator_ptr as pointer to 5 different operators
    //                                                      (amount, unit, low_limit, high_limit, operator_ptr)
    case REPLIM:    nop->operands=memAlloc(sizeof(oslRepLim_t));
                    nop->eval=eval_replim;
                    break;
    // tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:    nop->operands=memAlloc(sizeof(oslRepMax_t));
                    nop->eval=eval_repmax;
                    break;
    default:        log_error("%s - unsupported operator type (%s) in formula", __func__, operatorNames[opType]); break;
  }
  return nop;
}

unsigned int operatorFree(operator_ptr op, mechanism_ptr curMech)
{
  checkNullInt(op,      "Error: Could not deallocate NULL-operator!");
  checkNullInt(curMech, "Error: Required parameter for operator deallocation is NULL!");

  if(op->state!=NULL)
  {
    if(stateFree(op->state)==R_ERROR) {log_error("Error deallocating operator state"); return R_ERROR;}
    log_trace("Operator state deallocated successfully");
  }

  switch(op->type)
  {
    case OSL_TRUE:  log_trace("Deallocating TRUE-operator"); break;
    case OSL_FALSE: log_trace("Deallocating FALSE-operator"); break;
    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    {
      log_trace("Deallocating NOT-operator", op->operands);
      if(operatorFree(op->operands, curMech)==R_ERROR) {log_error("Error deallocating operator operand (NOT,ALWAYS)!"); return R_ERROR;}
      break;
    }
    case ALWAYS:    if(operatorFree(op->operands, curMech)==R_ERROR) {log_error("Error deallocating operator operand (NOT,ALWAYS)!"); return R_ERROR;}
                    break;
    // event:EventMatchingOperatorType  -> ???
    case EVENTMATCH:((eventMatch_ptr)op->operands)->matchAction->conditions=g_slist_remove(((eventMatch_ptr)op->operands)->matchAction->conditions, op);
                    if(eventMatchFree(op->operands, curMech)==R_ERROR)
                      {log_error("Error deallocating operator operand (EVENTMATCH)!"); return R_ERROR;}
                    break;
    case XPATH:     log_warn("freeing xpath operand"); free(((char *)op->operands));
                    break;
    // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:
    case AND:
    case IMPLIES:
    case SINCE:
    { // op->operands=2*operator_ptr
      log_trace("Deallocating OR|AND|IMPLIES|SINCE-operator");
      if(operatorFree(((oslBinary_ptr)op->operands)->operand[0], curMech)==R_ERROR) {log_error("Error deallocating operator operand (BINARY0)!"); return R_ERROR;}
      if(operatorFree(((oslBinary_ptr)op->operands)->operand[1], curMech)==R_ERROR) {log_error("Error deallocating operator operand (BINARY1)!"); return R_ERROR;}
      free(op->operands);
      break;
    }
    // tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:
    case DURING:
    case WITHIN:
    {
      log_trace("Deallocating BEFORE|DURING|WITHIN-operator");
      oslTimeUnary_ptr curOperand=op->operands;
      log_trace("          op1: %lu",curOperand->amount);
      log_trace("          op2: %s",curOperand->unit);
      log_trace("          op3: %s",operatorNames[curOperand->operand->type]);
      free(curOperand->unit);

      if(operatorFree(curOperand->operand, curMech)==R_ERROR) {log_error("Error deallocating operator operand (BEFORE2)!"); return R_ERROR;}
      free(curOperand);
      break;
    }
    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:
    {
      log_trace("Deallocating REPSINCE-operator");
      oslRepSince_ptr curOperand=op->operands;
      log_trace("          op1: %lu",curOperand->limit);
      log_trace("          op2: %s",operatorNames[curOperand->operand[0]->type]);
      log_trace("          op3: %s",operatorNames[curOperand->operand[1]->type]);

      if(operatorFree(curOperand->operand[0], curMech)==R_ERROR) {log_error("Error deallocating operator operand (REPSINCE1)!"); return R_ERROR;}
      if(operatorFree(curOperand->operand[1], curMech)==R_ERROR) {log_error("Error deallocating operator operand (REPSINCE2)!"); return R_ERROR;}
      free(curOperand);
      break;
    }
    case REPLIM:
    {
      log_trace("Deallocating REPLIM-operator");
      oslRepLim_ptr curOperand=op->operands;
      log_trace("          op1: %lu",curOperand->amount);
      log_trace("          op2: %s",curOperand->unit);
      log_trace("          op3: %lu",curOperand->lowerLimit);
      log_trace("          op4: %lu",curOperand->upperLimit);
      log_trace("          op5: %s",operatorNames[curOperand->operand->type]);

      free(curOperand->unit);
      if(operatorFree(curOperand->operand, curMech)==R_ERROR) {log_error("Error deallocating operator operand (REPLIM5)!"); return R_ERROR;}
      free(curOperand);
      break;
    }
    case REPMAX:
    {
      log_trace("Deallocating REPMAX-operator");
      oslRepMax_ptr curOperand=op->operands;
      log_trace("          ptr: %p", curOperand);
      log_trace("          op1: %lu",curOperand->upperLimit);
      log_trace("          op2: %s",operatorNames[curOperand->operand->type]);

      if(operatorFree(curOperand->operand, curMech)==R_ERROR) {log_error("Error deallocating operator operand (REPLIM5)!"); return R_ERROR;}
      free(curOperand);
      break;
    }
  }

  free(op);
  return R_SUCCESS;
}

uint8_t getOperatorType(const char *str)
{
  if(!strncasecmp(str,"or",2))                return OR;
  else if(!strncasecmp(str,"not",3))          return NOT;
  else if(!strncasecmp(str,"and",3))          return AND;
  else if(!strncasecmp(str,"true",4))         return OSL_TRUE;
  else if(!strncasecmp(str,"false",4))        return OSL_FALSE;
  else if(!strncasecmp(str,"since",5))        return SINCE;
  else if(!strncasecmp(str,"xpath",5))        return XPATH;
  else if(!strncasecmp(str,"always",6))       return ALWAYS;
  else if(!strncasecmp(str,"before",6))       return BEFORE;
  else if(!strncasecmp(str,"during",6))       return DURING;
  else if(!strncasecmp(str,"within",6))       return WITHIN;
  else if(!strncasecmp(str,"replim",6))       return REPLIM;
  else if(!strncasecmp(str,"repmax",6))       return REPMAX;
  else if(!strncasecmp(str,"implies",7))      return IMPLIES;
  else if(!strncasecmp(str,"repsince",8))     return REPSINCE;
  else if(!strncasecmp(str, "eventmatch",10)) return EVENTMATCH;
  else return NO_OSL_OP;
}

unsigned long operatorMemSize(operator_ptr op)
{ /// @todo check!
  unsigned long size=sizeof(operator_ptr);
  size+=sizeof(char);
  size+=sizeof(void*);
  size+=stateMemSize(op->state);
  switch(op->type)
  {
    case OSL_TRUE:
    case OSL_FALSE: size+=sizeof(void*); break;
    case EVENTMATCH: //size+=event_getMemSize( (event_ptr)op->operands );
      break;
    case AND:
    case NOT:
    case OR:
    case IMPLIES: break;
    case XPATH:  size+=sizeof(char*)+(strlen( (char*)op->operands)+1)*sizeof(char); break;
    case BEFORE: size+=0; break;  // also counting size of circular array here!
    case WITHIN: size+=0; break;
    case DURING: size+=0; break;
    case ALWAYS: size+=0; break;
    case SINCE:  size+=0; break;
    case REPLIM: size+=0; break;
    case REPMAX: size+=0; break;
    case REPSINCE: size+=0; break;
  }
  return size;
}

void operatorLog(operator_ptr op)
{
  log_trace("Operator type: %s",operatorNames[op->type]);
  switch(op->type)
  {
    // tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:  break;
    case OSL_FALSE: break;

    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    case ALWAYS: log_trace("          op1: %s",operatorNames[((operator_ptr)op->operands)->type]); break;

      // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:
    case AND:
    case IMPLIES:
    case SINCE:
    {
      oslBinary_ptr curOperands=op->operands;
      log_trace("          op1: %s",operatorNames[curOperands->operand[0]->type]);
      log_trace("          op2: %s",operatorNames[curOperands->operand[1]->type]);
      break;
    }
    // event:EventMatchingOperatorType, XPath  -> ???
    case EVENTMATCH:
    {
      log_trace("          op1: %s", ((eventMatch_ptr)op->operands)->matchAction->actionName);
      break;
    }
    case XPATH:
    {
      log_trace("          op1: %s",op->operands);
      break;
    }
    // tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:
    case DURING:
    case WITHIN:
    { // 1st: unsigned long interval, 2nd char* timeunit, 3rd operator_ptr
      oslTimeUnary_ptr curOperands=op->operands;
      log_trace("          op1: %lu",curOperands->amount);
      log_trace("          op2: %s", curOperands->unit);
      log_trace(" ==> interval: %llu timesteps ", curOperands->interval);
      log_trace("          op3: %s", operatorNames[curOperands->operand->type]);
      break;
    }
    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:
    { // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr, 3rd operand: operator_ptr
      oslRepSince_ptr curOperands=op->operands;
      log_trace("          op1: %lu",curOperands->limit);
      log_trace("          op2: %s",operatorNames[curOperands->operand[0]->type]);
      log_trace("          op3: %s",operatorNames[curOperands->operand[1]->type]);
      break;
    }
    // tns:RepLimOperatorType           -> operands=special (1operator_ptr as pointer to 3 different operators(low_limit, high_limit, operator_ptr)
    case REPLIM:
    {
      oslRepLim_ptr curOperands=op->operands;
      log_trace("          op1: %lu",curOperands->amount);
      log_trace("          op2: %s",curOperands->unit);
      log_trace(" ==> interval: %llu timesteps ",curOperands->interval);
      log_trace("          op3: %lu",curOperands->lowerLimit);
      log_trace("          op4: %lu",curOperands->upperLimit);
      log_trace("          op5: %s",operatorNames[curOperands->operand->type]);
      break;
    }
    // tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:
    {
      oslRepMax_ptr curOperands=op->operands;
      log_trace("          op1: %lu",curOperands->upperLimit);
      log_trace("          op2: %s",operatorNames[curOperands->operand->type]);
      break;
    }
    default: log_trace("    logging for this operand not implemented"); break;
  }
}





