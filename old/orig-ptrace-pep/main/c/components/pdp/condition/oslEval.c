/**
 * @file  oslEval.c
 * @brief Implementation of the OSL operators evaluation methods
 *
 * @author cornelius moucha
 **/

#include "oslEval.h"
#include "log_oslEval_pef.h"

bool eval_true(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{ // will never change, so it is already set to true with creating new state for this node
  log_debug(" [%s] - evaluating TRUE     node => true", mech->mechName);
  return TRUE;
}

bool eval_false(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{ // will never change, so it is already set to false with creating new state for this node
  log_debug(" [%s] - evaluating FALSE    node => false", mech->mechName);
  return FALSE;
}

bool eval_or(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  curop->state->value=((oslBinary_ptr)curop->operands)->operand[0]->state->value ||
                      ((oslBinary_ptr)curop->operands)->operand[1]->state->value;
  log_debug(" [%s] - evaluating OR       node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_and(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  curop->state->value=((oslBinary_ptr)curop->operands)->operand[0]->state->value && ((oslBinary_ptr)curop->operands)->operand[1]->state->value;
  log_debug(" [%s] - evaluating AND      node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_impl(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  curop->state->value=(!((oslBinary_ptr)curop->operands)->operand[0]->state->value) ||
                        ((oslBinary_ptr)curop->operands)->operand[1]->state->value;
  log_debug(" [%s] - evaluating IMPL      node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_event(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  if(curEvent!=NULL)
  {
    if(eventMatches(curop->operands, curEvent)==TRUE)
    {
      curop->state->value=TRUE;
    }
  }
  log_debug(" [%s] - evaluating EVENT    node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_xpath(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  if(curEvent!=NULL)
  {
    //dumpXMLdoc(curEvent->xmlDoc);
    if(curEvent->xmlDoc==NULL) eventCreateXMLdoc(curEvent);
    //dumpXMLdoc(curEvent->xmlDoc);
    curop->state->value=xpathEval(curEvent->xmlDoc, curop->operands);
  }
  log_debug(" [%s] - evaluating XPATH    node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_not(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  curop->state->value=!((operator_ptr)curop->operands)->state->value;
  log_debug(" [%s] - evaluating NOT      node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_always(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  /// @todo optimization: immutability => reset function pointer to eval_true
  // If subformula was ever false in the past, ALWAYS will evaluate to false independently of current value of appropriate child node
  if(!curop->state->immutable)
  {
    curop->state->value=((operator_ptr)curop->operands)->state->value;
    if(!curop->state->value && curEvent==NULL)
    {
      log_trace(" [%s] - evaluating ALWAYS:  activating IMMUTABILITY", mech->mechName);
      curop->state->immutable=TRUE;
    }
  }
  log_debug(" [%s] - evaluating ALWAYS   node => %u (immutable=[%s])", mech->mechName,curop->state->value,boolStr[curop->state->immutable]);
  return curop->state->value;
}


bool eval_before(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  oslTimeUnary_ptr curOperands=curop->operands;
  //log_trace("          op1: %ld",curOperands->amount);
  //log_trace("          op2: %s",curOperands->unit);
  //log_trace(" ==> interval: %llu timesteps ",curOperands->interval);
  //log_trace("          op3: %s",operatorNames[curOperands->operand->type]);

  // before = at (currentTime - interval) op3 was true
  circArray_ptr sv = curop->state->savedValues.bitArray;
  log_trace("[BEFORE] Reading position [%u]", sv->p_first);
  char *svbuf=circArrayString(sv);
  log_trace("[BEFORE] circArray=[%s]", svbuf);
  free(svbuf);
  if(circArrayGet(sv, sv->p_first)) curop->state->value = TRUE;
  else curop->state->value = FALSE;

  if(curEvent==NULL)
  {
    if(sv->p_first++==curOperands->interval) sv->p_first=0;
    log_trace("[BEFORE] Setting position [%u] to [%u]", sv->p_next, curOperands->operand->state->value);
    circArraySet(sv,sv->p_next,curOperands->operand->state->value);

    if(sv->p_next++==curOperands->interval) sv->p_next=0;
    svbuf=circArrayString(sv);
    log_trace("[BEFORE] circArray=[%s]", svbuf);
    free(svbuf);
  }
  log_debug(" [%s] - evaluating BEFORE   node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_during(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  oslTimeUnary_ptr curOperands=curop->operands;
  //log_trace("          op1: %ld",curOperands->amount);
  //log_trace("          op2: %s",curOperands->unit);
  //log_trace(" ==> interval: %llu timesteps ",curOperands->interval);
  //log_trace("          op3: %s",operatorNames[curOperands->operand->type]);

  log_trace("[DURING] Current state counter=[%lu]", curop->state->counter);
  if(curop->state->counter==0) curop->state->value=TRUE;
  else curop->state->value=FALSE;

  if(curEvent==NULL)
  {
    if(!curOperands->operand->state->value)
    {
      curop->state->counter=curOperands->interval+1;
      log_trace("[DURING] Set negative counter to interval=[%llu] due to subformulas state value=[%u]",
                curOperands->interval+1, curOperands->operand->state->value);
    }
    else
    {
      if(curop->state->counter>0) curop->state->counter--;
      log_trace("[DURING} New state counter: [%lu]", curop->state->counter);
    }

    // question:
    // in timestep 5: event -> false
    // in timestep 6: event -> false
    // in timestep 7: event -> FALSE? actually DURING is satisfied if this action is NOT a try event, i.e. if it will really happen/has happened
    //                then in the last 3 timesteps (during 3) subformula is satisfied, so condition should be TRUE?!
    //   -> usual problem: when occurring events are counted? actual happening ones or only at end of timestep?
    //      if only at end of timestep -> contradiction to counting operators, where it is clearly stated in paper, that ALL events have to be counted

    // update state->value for logging output
    if(curop->state->counter==0) curop->state->value=TRUE;
    else curop->state->value=FALSE;
  }
  log_debug(" [%s] - evaluating DURING   node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

bool eval_within(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  oslTimeUnary_ptr curOperands=curop->operands;
  //log_trace("          op1: %ld",curOperands->amount);
  //log_trace("          op2: %s",curOperands->unit);
  //log_trace(" ==> interval: %llu timesteps ",curOperands->interval);
  //log_trace("          op3: %s",operatorNames[curOperands->operand->type]);

  log_trace("[WITHIN] Current state counter=[%lu]", curop->state->counter);
  if(curop->state->counter>0) curop->state->value=TRUE;
  else curop->state->value=FALSE;

  if(curEvent==NULL)
  {
    if(curOperands->operand->state->value)
    {
      curop->state->counter=curOperands->interval+1;
      log_trace("[WITHIN] Set negative counter to interval=[%llu] due to subformulas state value=[%u]",
          curOperands->interval+1, curOperands->operand->state->value);
    }
    else
    {
      if(curop->state->counter>0) curop->state->counter--;
      log_trace("[WITHIN} New state counter: [%lu]", curop->state->counter);
    }

    // update state->value for logging output
    if(curop->state->counter>0) curop->state->value=TRUE;
    else curop->state->value=FALSE;
  }
  log_debug(" [%s] - evaluating WITHIN   node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

/// @todo optimization: remove duplicate code
bool eval_since(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{ // since(A, B): (B since last occurence of A) OR (always B) // op1 = A, op2 = B
  oslBinary_ptr curOperands=curop->operands;

  operator_ptr op1=curOperands->operand[0]; // subformula A
  operator_ptr op2=curOperands->operand[1]; // subformula B
  log_trace("          op1: %s (%s)",operatorNames[op1->type], ((eventMatch_ptr)op1->operands)->matchAction->actionName);
  log_trace("          op2: %s (%s)",operatorNames[op2->type], ((eventMatch_ptr)op2->operands)->matchAction->actionName);

  // A occurs, SINCE is satisfied (LTL doesn't state anything about B in the timestep where A happens
  if(op1->state->value)
  {
    log_trace("[SINCE] Subformula A satisfied this timestep => TRUE");
    curop->state->value=TRUE;
  }
  else
  {
    if(!curop->state->immutable)
    { // until now B occurred every following timestep
      if(curop->state->counter==1)
      {
        log_trace("[SINCE] Subformula A was satisfied any previous timestep");

        if(op2->state->value)
        {
          log_trace("[SINCE] Subformula B is satisfied this timestep => TRUE");
          curop->state->value=TRUE;
        }
        else
        {
          log_trace("[SINCE] Subformula B NOT satisfied this timestep => FALSE");
          curop->state->value=FALSE;
        }
      }
      else
      {
        log_trace("[SINCE] Subformula A NOT satisfied this timestep or any previous timestep");
        log_trace("[SINCE] Not yet immutable; check (ALWAYS B) part of since");

        if(op2->state->value)
        {
          log_trace("[SINCE] Subformula B is satisfied this timestep => TRUE");
          curop->state->value=TRUE;
        }
        else
        {
          log_trace("[SINCE] Subformula B NOT satisfied this timestep => FALSE");
          curop->state->value=FALSE;
        }
      }
    }
  }

  if(curEvent==NULL)
  {
    if(!curop->state->value)
    {
      if(!curop->state->immutable)
      {
        log_trace("[SINCE] Evaluating current state value was FALSE =>  activating IMMUTABILITY");
        curop->state->immutable=TRUE;
      }
    }

    if(op1->state->value)
    {
      log_trace("[SINCE] Subformula A satisfied this timestep => setting counter flag");
      curop->state->counter=1;
      if(curop->state->immutable)
      {
        log_trace("[SINCE] Deactivating immutability");
        curop->state->immutable=FALSE;
      }
    }

    if(!curop->state->subEverTrue && !op2->state->value)
    {
      log_trace("[SINCE] Subformula B was previously always satisfied, but NOT this timestep => 2nd part of since can never be satisfied any more");
      log_trace("[SINCE] Setting subEverFalse flag and activating immutability");
      curop->state->subEverTrue=TRUE; // intention here subformula was ever FALSE (in contrast to name...)
      curop->state->immutable=TRUE;
    }
  }

  log_debug(" [%s] - evaluating SINCE    node => %u (immutable=[%s])",mech->mechName,curop->state->value,boolStr[curop->state->immutable]);
  return curop->state->value;
}

/// @todo optimization: remove duplicate code
bool eval_repsince(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{ // repsince(n, A, B); // n = op1 / A = op2 / B = op3
  // B(n) >= limit n times subformula B since the last occurrence of subformula A

  oslRepSince_ptr curOperands=curop->operands;
  unsigned long limit=curOperands->limit;
  operator_ptr op2=curOperands->operand[0];
  operator_ptr op3=curOperands->operand[1];
  log_trace("          op1: %lu",limit);
  log_trace("          op2: %s",operatorNames[op2->type]);
  log_trace("          op3: %s",operatorNames[op3->type]);

  if(op2->state->value)
  {
    log_trace("[REPSINCE] Subformula A satisfied this timestep => TRUE");
    curop->state->value=TRUE;
  }
  else
  {
    unsigned int limitComparison=curop->state->counter + (op3->state->value ? 1 : 0);
    log_trace("[REPSINCE] Counter for subformula B [%u]", limitComparison);

    if(curop->state->subEverTrue==1)
    {
      log_trace("[REPSINCE] Subformula A was satisfied any previous timestep");
      if(limitComparison<=limit)
      {
        log_trace("[REPSINCE] Amount of occurrences of subformula B <= limit ==> TRUE");
        curop->state->value=TRUE;
      }
      else
      {
        log_trace("[REPSINCE] Occurrence limitation exceeded! ==> FALSE");
        curop->state->value=FALSE;
      }
    }
    else
    {
      log_trace("[REPSINCE] Subformula A NOT satisfied this timestep or any previous timestep");
      if(limitComparison<=limit)
      {
        log_trace("[REPSINCE] Global amount of occurrences of subformula B <= limit ==> TRUE");
        curop->state->value=TRUE;
      }
      else
      {
        log_trace("[REPSINCE] Global occurrence limitation exceeded! ==> FALSE");
        curop->state->value=FALSE;
      }

    }
  }

  if(curEvent==NULL)
  {
    if(op2->state->value)
    {
      log_trace("[REPSINCE] Subformula A satisfied this timestep => setting flag and resetting counter");
      curop->state->subEverTrue=1;

      curop->state->counter=0;
      log_trace("[REPSINCE] Counter for subformula B [%u]", curop->state->counter);
    }

    if(op3->state->value)
    {
      curop->state->counter++;
      log_trace("[REPSINCE] Counter for subformula B [%u]", curop->state->counter);
    }
  }

  log_debug(" [%s] - evaluating REPSINCE node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}

/// @todo optimization: instead of activating immutability reset evaluation function to eval_true
bool eval_repmax(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  oslRepMax_ptr curOperands=curop->operands;
  unsigned long limit=curOperands->upperLimit;
  operator_ptr op2   =curOperands->operand;

  if(!curop->state->immutable)
  { /// @remark changed to count every event, not only at the end of timesteps, as described in paper
    ///         "A policy language for Distributed Usage Control":
    ///         "...we allow multiple similar events to occur within one timestep. For example the movie m may be played on two devices
    ///         simultaneously, which counts twice in the repuntil example above."...
    if(curEvent!=NULL && op2->state->value)
    {
      curop->state->counter++;
      log_trace("[REPMAX] Subformula was satisfied; counter incremented to [%u]", curop->state->counter);
    }

    if(curop->state->counter<=limit) curop->state->value=TRUE;
    else curop->state->value=FALSE;

    if(curEvent==NULL && !curop->state->value)
    {
      log_trace("[REPMAX] Activating immutability");
      curop->state->immutable=TRUE;
    }
  }
  log_debug(" [%s] - evaluating REPMAX   node => %u (immutable=[%s])",mech->mechName,curop->state->value,boolStr[curop->state->immutable]);
  return curop->state->value;
}

bool eval_replim(operator_ptr curop, event_ptr curEvent, mechanism_ptr mech)
{
  oslRepLim_ptr curOperands=curop->operands;
  operator_ptr op5=curOperands->operand;
  //log_trace("          op1: %ld",curOperands->amount);
  //log_trace("          op2: %s", curOperands->unit);
  //log_trace(" ==> interval: %llu timesteps ",curOperands->interval);
  //log_trace("          op3: %ld",curOperands->lowerLimit);
  //log_trace("          op4: %ld",curOperands->upperLimit);
  //log_trace("          op5: %s",operatorNames[op5->type]);

  circArray_ptr sv=curop->state->savedValues.bitArray;
  if(curop->state->counter >= curOperands->lowerLimit && curop->state->counter <= curOperands->upperLimit)
    curop->state->value=TRUE;
  else curop->state->value=FALSE;

  if(curEvent==NULL)
  {
    char *svbuf=circArrayString(sv);
    log_trace("[REPLIM] circArray=[%s]", svbuf);
    free(svbuf);
    if(circArrayGet(sv,sv->p_first))
    {
      curop->state->counter--;
      log_trace("[REPLIM] Decrementing counter to [%u]", curop->state->counter);
    }
    if(sv->p_first++==curOperands->interval) sv->p_first=0;

    if(op5->state->value)
    {
      curop->state->counter++;
      log_trace("[REPLIM] Incrementing counter to [%u] due to intercepted event", curop->state->counter);
    }

    log_trace("[REPLIM] Setting position [%u] to [%u]", sv->p_next, op5->state->value);
    circArraySet(sv,sv->p_next,op5->state->value);

    if(sv->p_next++==curOperands->interval) sv->p_next=0;
    svbuf=circArrayString(sv);
    log_trace("[REPLIM] circArray=[%s]", svbuf);
    free(svbuf);
  }
  log_debug(" [%s] - evaluating REPLIM   node => %u", mech->mechName,  curop->state->value);
  return curop->state->value;
}



