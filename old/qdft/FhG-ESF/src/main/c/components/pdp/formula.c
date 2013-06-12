#include "pdp/formula.h"
#include "log_formula_c.h"
#include "pdp/mechanism.h"

extern action_desc_store_ptr pdp_action_desc_store;

/******************************************************************************
 *  Circular array implementation (bit-array)
******************************************************************************/
c_array_ptr c_array_new(unsigned long size)
{
  c_array_ptr ncarray=mem_alloc(sizeof(c_array_t));
  if(ncarray == NULL)
  {
    log_error("Could not allocate memory for circular array");
    return ncarray;
  }

  ncarray->values=mem_calloc((size+7)/8,sizeof(unsigned char));
  ncarray->p_first=0;
  ncarray->p_next=0;
  return ncarray;
}

unsigned int  c_array_free(c_array_ptr array)
{ // @todo not implemented
  return R_SUCCESS;
}

bool c_array_get(c_array_ptr array, unsigned long pos)
{
  return (*(array->values+pos/8) & (1<<(pos%8)))!=0;
}

void c_array_set(c_array_ptr array, unsigned long pos, bool val)
{
  if(val) *(array->values+pos/8) |=1<<(pos%8);    // setting to 1
  else    *(array->values+pos/8) &=~(1<<(pos%8)); // setting to 0
}

void c_array_log(c_array_ptr array)
{ // @todo not implemented

}

/******************************************************************************
 *  Circular array implementation (int-array)
******************************************************************************/
cint_array_ptr cint_array_new(unsigned long size)
{
  cint_array_ptr ncarray=mem_alloc(sizeof(cint_array_t));
  if(ncarray == NULL)
  {
    log_error("Could not allocate memory for circular array");
    return ncarray;
  }

  ncarray->values=mem_calloc(size,sizeof(unsigned int));
  ncarray->p_first=0;
  ncarray->p_next=0;
  return ncarray;
}

unsigned int  cint_array_free(cint_array_ptr array)
{ // @todo not implemented
  return R_SUCCESS;
}

unsigned int cint_array_get(cint_array_ptr array, unsigned long pos)
{
  return *(array->values+pos);
}

void cint_array_set(cint_array_ptr array, unsigned long pos, unsigned int val)
{
  *(array->values+pos)=val;
}

void cint_array_log(cint_array_ptr array)
{ // @todo not implemented

}

/******************************************************************************
 *  State implementation
******************************************************************************/
state_ptr state_new(bool value)
{
  state_ptr nstate=mem_alloc(sizeof(state_t));
  if(nstate == NULL)
  {
    log_error("Could not allocate memory for state");
    return NULL;
  }
  nstate->value=value;
  nstate->saved_values.bitArray=NULL;
  nstate->immutable=FALSE;
  nstate->count_true=0;
  // @todo was_ever_true=false correct or is it still used at all?
  nstate->was_ever_true=FALSE;
  return nstate;
}

unsigned int state_free()
{ // @todo not implemented
  return R_SUCCESS;
}

unsigned long state_getMemSize(state_ptr state)
{ // @todo check!
  unsigned long size=sizeof(state_ptr);
  size+=2*sizeof(bool);
  size+=sizeof(unsigned int);
  size+=sizeof(unsigned char);
  size+=sizeof(c_array_ptr);
  // circular array cannot be counted here, because size is not known; has to be done at operator calculation
  return size;
}

void state_log(state_ptr state)
{ // @todo not implemented

}

/******************************************************************************
 *  Operator implementation
******************************************************************************/
operator_ptr operator_new(unsigned char type)
{
  operator_ptr nop=mem_alloc(sizeof(operator_t));
  if(nop == NULL)
  {
    log_error("Could not allocate memory for operator");
    return NULL;
  }

  // statically set state-value to true for OSL_TRUE node -> will never change; otherwise first state value is false
  nop->state=state_new( (type==OSL_TRUE ? TRUE : FALSE));
  nop->type=type;

  // depending on type -> preallocating memory for appropriate number of operands
  switch(type)
  {
    // tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:  nop->operands=NULL; nop->eval=eval_true; break;
    case OSL_FALSE: nop->operands=NULL; nop->eval=eval_false; break;
    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:       nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_not; break;
    case ALWAYS:    nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_always; break;
    // event:EventMatchingOperatorType  -> ???
    case EVENTMATCH:nop->operands=mem_alloc(sizeof(event_match_op_ptr)); nop->eval=eval_event; break;
    case XPATH:     nop->operands=mem_alloc(sizeof(unsigned char *)); nop->eval=eval_xpath; break;
    // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:        nop->operands=mem_alloc(sizeof(operator_ptr*));   nop->eval=eval_or; break;
    case AND:       nop->operands=mem_alloc(sizeof(operator_ptr*));   nop->eval=eval_and; break;
    case IMPLIES:   nop->operands=mem_alloc(sizeof(operator_ptr*));   nop->eval=eval_impl; break;
    case SINCE:     nop->operands=mem_alloc(sizeof(operator_ptr*));   nop->eval=eval_since; break;
    // tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:    nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_before; break;
    case DURING:    nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_during; break;
    case WITHIN:    nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_within; break;
    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:  nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_repsince; break;
    // tns:RepLimOperatorType           -> operands=special (1operator_ptr as pointer to 5 different operators
    //                                                      (amount, unit, low_limit, high_limit, operator_ptr)
    case REPLIM:    nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_replim; break;
    // tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:    nop->operands=mem_alloc(sizeof(operator_ptr));    nop->eval=eval_repmax; break;
    default:        log_error("%s - unsupported operator type (%s) in formula", __func__, operatorNames[type]);
  }
  return nop;
}

unsigned int operator_free(operator_ptr op)
{ // @todo not implemented
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

unsigned long operator_getMemSize(operator_ptr op)
{ // @todo check!
  unsigned long size=sizeof(operator_ptr);
  size+=sizeof(char);
  size+=sizeof(void*);
  size+=state_getMemSize(op->state);
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
    case XPATH: size+=sizeof(char*)+(strlen( (char*)op->operands)+1)*sizeof(char); break;
    case BEFORE: size+=0; break;  // also counting size of circular array here!
    case WITHIN: size+=0; break;
    case DURING: size+=0; break;
    case ALWAYS: size+=0; break;
    case SINCE: size+=0; break;
    case REPLIM: size+=0; break;
    case REPMAX: size+=0; break;
    case REPSINCE: size+=0; break;
  }
  return size;
}

void operator_log(operator_ptr op)
{
  log_trace("Operator type: %s",operatorNames[op->type]);
  switch(op->type)
  {
    // tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE: break;
    case OSL_FALSE: break;
    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    case ALWAYS:
      log_trace("          op1: %s",operatorNames[((operator_ptr)op->operands)->type]); break;
    // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:
    case AND:
    case IMPLIES:
    case SINCE:
    {
      log_trace("          op1: %s",operatorNames[((operator_ptr*)op->operands)[0]->type]);
      log_trace("          op2: %s",operatorNames[((operator_ptr*)op->operands)[1]->type]);
      break;
    }
    // event:EventMatchingOperatorType, XPath  -> ???
    case EVENTMATCH:
    {
      log_trace("          op1: %s", ((event_match_op_ptr)op->operands)->match_action->action_name );
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
    { // 1st: unsigned long interval, 2nd unsigned char* timeunit, 3rd operator_ptr
      unsigned long interval=*((unsigned long*)*((unsigned long*)op->operands));
      log_trace("          op1: %lu",interval);
      unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(op->operands+sizeof(void*))) ));
      log_trace("          op2: %s",timeUnit);
      operator_ptr op2=*((operator_ptr*)(op->operands+2*sizeof(void*)));
      log_trace("          op3: %s",operatorNames[op2->type]);
      break;
    }
    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:
    { // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr, 3rd operand: operator_ptr
      unsigned long limit=*((unsigned long*)*((unsigned long*)op->operands));
      log_trace("          op1: %lu",limit);
      operator_ptr op2=*((operator_ptr*)(op->operands+1*sizeof(void*)));
      log_trace("          op2: %s",operatorNames[op2->type]);
      operator_ptr op3=*((operator_ptr*)(op->operands+2*sizeof(void*)));
      log_trace("          op3: %s",operatorNames[op3->type]);
      break;
    }
    // tns:RepLimOperatorType           -> operands=special (1operator_ptr as pointer to 3 different operators(low_limit, high_limit, operator_ptr)
    case REPLIM:
    {
      unsigned long interval =*((unsigned long*)*((unsigned long*)op->operands));
      log_trace("          op1: %lu",interval);
      unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(op->operands+1*sizeof(void*))) ));
      log_trace("          op2: %s",timeUnit);
      unsigned long lowLimit=*((unsigned long*)*((unsigned long*)(op->operands+2*sizeof(void*))));
      log_trace("          op3: %lu",lowLimit);
      unsigned long highLimit=*((unsigned long*)*((unsigned long*)(op->operands+3*sizeof(void*))));
      log_trace("          op4: %lu",highLimit);
      operator_ptr op3=*((operator_ptr*)(op->operands+4*sizeof(void*)));
      log_trace("          op5: %s",operatorNames[op3->type]);

      break;
    }
    // tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:
    {

      break;
    }
    default: log_trace("    logging for this operand not implemented"); break;
  }
}

/******************************************************************************
 *  Condition formula implementation
******************************************************************************/

formula_ptr formula_new()
{
  formula_ptr nformula=mem_alloc(sizeof(formula_t));
  if(nformula == NULL)
  {
    log_error("Could not allocate memory for formula");
    return nformula;
  }
  nformula->n_nodes=0;
  nformula->nodes=NULL;
  return nformula;
}

unsigned int formula_free(formula_ptr formula)
{ // @todo not implemented
  return R_SUCCESS;
}

unsigned long formula_getMemSize(formula_ptr formula)
{ // @todo check
  unsigned long size=sizeof(formula_ptr);
  size+=sizeof(unsigned int);
  unsigned int a;
  for(a=0; a<formula->n_nodes; a++)
    size+=operator_getMemSize(formula->nodes[a]);
  size+=sizeof(operator_ptr*);
  return size;
}

operator_ptr parse_subformula(formula_ptr curFormula, xmlNodePtr node, unsigned int *curnode)
{
  operator_ptr op=NULL;
  char curType=getOperatorType((char*)node->name);
  switch(curType)
  {
    //   tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:
    case OSL_FALSE: op=operator_new(curType); break;
    //   tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    case ALWAYS:
    {
      op=operator_new(curType);
      op->operands=parse_subformula(curFormula, xmlGetFirstChildElement(node), curnode);
      break;
    }
    //   event:EventMatchingOperatorType, XPATH  -> ???
    case EVENTMATCH:
    {
      op=operator_new(curType);
      op->operands=event_match_parse_xml(pdp_action_desc_store, node);
      break;
    }
    case XPATH:
    {
      op=operator_new(XPATH);
      xmlNodePtr tmp = NULL;
      for(tmp=node->children; tmp; tmp=tmp->next)
        if(tmp->type==XML_TEXT_NODE)
        {
          char *content=(char*)xmlNodeGetContent(tmp);
          op->operands=mem_calloc(strlen(content)+1, sizeof(unsigned char));
          strncpy(op->operands, content, strlen(content));
          //xmlFree(content);
		  free(content);
          break;
        }
      break;
    }
    //   tns:BinaryOperatorType           -> operands=2 operator_ptr
    case AND:
    case OR:
    case IMPLIES:
    case SINCE:
    {
      op=operator_new(curType);
      operator_ptr *ops=mem_calloc(2,sizeof(operator_ptr));
      unsigned int iops=0;
      xmlNodePtr nextNode=NULL;
      for(nextNode=node->children; nextNode; nextNode=nextNode->next)
      {
        if(xmlIsElement(nextNode)) ops[iops++]=parse_subformula(curFormula, nextNode, curnode);
        if(iops==2) break;
      }
      op->operands=ops;
      break;
    }
    //   tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:
    case DURING:
    case WITHIN:
    { // 1st operand: (unsigned long)amount, 2nd operand: (unsigned char*)timeunit,
      // 3rd operand: operator_ptr to nontemporal operator (propositional or event)
      op=operator_new(curType);
      unsigned long interval=atol((char*)xmlGetProp(node, "amount"));

      void *ops=mem_calloc(3,sizeof(void*));
      unsigned long *ops0=mem_alloc(sizeof(unsigned long));
      *(ops0)=interval;
      *(  (unsigned long*)ops ) = (unsigned long)ops0;

      // @todo default value for UNIT really timestep??
      char *tmp=(char*)xmlGetProp(node, "unit");
      char *timeUnit;
      if(tmp==NULL)
      {
        timeUnit=mem_alloc(9*sizeof(char));
        sprintf(timeUnit,"timestep");
      }
      else timeUnit=strdup(tmp);
      //xmlFree(tmp);
	  free(tmp);

      char **ops1=mem_alloc(sizeof(char*));
      *(ops1)=timeUnit;
      *( (char**) (ops+sizeof(void*)) )=(char*)ops1;

      operator_ptr ops2;
      ops2=parse_subformula(curFormula, xmlGetFirstChildElement(node), curnode);

      *( (operator_ptr*) (ops+2*sizeof(void*)) ) = (operator_ptr)ops2;
      op->operands=ops;
      //op->state->saved_values=c_array_new(interval);
      op->state->saved_values.bitArray=c_array_new(interval);
      break;
    }
    //   tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2*operator_ptr)
    case REPSINCE:
    { // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr, 3rd operand: operator_ptr
      xmlNodePtr nextNode=NULL;
      op=operator_new(REPSINCE);
      unsigned long limit=atol(xmlGetProp(node, "limit"));

      void *ops=mem_calloc(3,sizeof(void*));
      unsigned long *ops0=mem_alloc(sizeof(unsigned long));
      *(ops0)=limit;
      *(  (unsigned long*)ops ) = (unsigned long)ops0;

      operator_ptr *ops2=mem_alloc(2*sizeof(operator_ptr));
      unsigned int iops=0;
      for(nextNode=node->children; nextNode; nextNode=nextNode->next)
      {
        if(nextNode->type == XML_ELEMENT_NODE)
        {
          ops2[iops++]=parse_subformula(curFormula, nextNode, curnode);
          if(iops==2) break;
        }
      }
      *( (operator_ptr*) (ops+1*sizeof(void*)) ) = (operator_ptr)ops2[0];
      *( (operator_ptr*) (ops+2*sizeof(void*)) ) = (operator_ptr)ops2[1];
      op->operands=ops;
      break;
    }
    //   tns:RepLimOperatorType  -> operands=special (1operator_ptr as pointer to 3 different operators(low_limit, high_limit, operator_ptr
    case REPLIM:
    { // 1st operand: (unsigned long)amount, 2nd operand: (unsigned char*)timeunit,
      // 3rd operand: (unsigned long)LowLimit, 4th operand: (unsigned long)Highlimit, 5th operand: operator_ptr
      op=operator_new(REPLIM);
      unsigned long lowLimit=atol(xmlGetProp(node, "lowerLimit"));
      unsigned long highLimit=atol(xmlGetProp(node, "upperLimit"));

      unsigned long interval=atol(xmlGetProp(node, "amount"));
      // @todo default value for UNIT really timestep??
      unsigned char *tmp=xmlGetProp(node, "unit");
      unsigned char *timeUnit;
      if(tmp==NULL)
      {
        timeUnit=mem_alloc(9*sizeof(unsigned char));
        sprintf(timeUnit,"timestep");
      }
      else timeUnit=strdup(tmp);
      //xmlFree(tmp);
	  free(tmp);

      void *ops=mem_calloc(5,sizeof(void*));
      unsigned long *ops0=mem_alloc(sizeof(unsigned long));
      *(ops0)=interval;
      *(  (unsigned long*)ops ) = (unsigned long)ops0;

      unsigned char **ops1=mem_alloc(sizeof(unsigned char*));
      *(ops1)=timeUnit;
      *( (unsigned char**) (ops+1*sizeof(void*)) )=(unsigned char*)ops1;

      unsigned long *ops2=mem_alloc(sizeof(unsigned long));
      *(ops2)=lowLimit;
      *(  (unsigned long*) (ops+2*sizeof(void*)) ) = (unsigned long)ops2;

      unsigned long *ops3=mem_alloc(sizeof(unsigned long));
      *(ops3)=highLimit;
      *( (unsigned long*) (ops+3*sizeof(void*)) ) = (unsigned long)ops3;

      operator_ptr ops4;
      ops4=parse_subformula(curFormula, xmlGetFirstChildElement(node), curnode);
      *( (operator_ptr*) (ops+4*sizeof(void*)) ) = (operator_ptr)ops4;
      op->operands=ops;
      //op->state->saved_values=cint_array_new(interval);
      op->state->saved_values.intArray=cint_array_new(interval);
      break;
    }
    //   tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:
    { // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr
      op=operator_new(REPMAX);
      unsigned long limit=atol(xmlGetProp(node, "limit"));

      void *ops=mem_calloc(2,sizeof(void*));
      unsigned long *ops0=mem_alloc(sizeof(unsigned long));
      *(ops0)=limit;
      *(  (unsigned long*)ops ) = (unsigned long)ops0;

      operator_ptr ops2;
      ops2=parse_subformula(curFormula, xmlGetFirstChildElement(node), curnode);

      *( (operator_ptr*) (ops+1*sizeof(void*)) ) = (operator_ptr)ops2;
      op->operands=ops;
      break;
    }
    default: log_error("not yet implemented operator node: %s\n",node->name);
  }
  operator_log(op);
  curFormula->nodes[*curnode]=op;
  (*curnode)++;
  return op;
}

/// counting formula nodes and searching for parametrized matchingEvents, ...
unsigned int formula_parse_stage1(formula_ptr curFormula, xmlNodePtr subformula)
{
  xmlNodePtr cur_node=NULL;
  for(cur_node=subformula->children; cur_node; cur_node = cur_node->next)
  {
    if(xmlIsElement(cur_node) && getOperatorType(cur_node->name)!=NO_OSL_OP)
    {
      curFormula->n_nodes++;
      formula_parse_stage1(curFormula, cur_node);
    }
  }
  return R_SUCCESS;
}

formula_ptr formula_parse_xml(xmlNodePtr rootNode)
{
  check_null_ptr(rootNode, "Empty condition formula?");
  formula_ptr curFormula=formula_new();
  formula_parse_stage1(curFormula, rootNode);
  curFormula->nodes=mem_calloc(curFormula->n_nodes, sizeof(operator_ptr));

  unsigned int lcurnode=0;
  parse_subformula(curFormula, xmlGetFirstChildElement(rootNode), &lcurnode);
  formula_log(curFormula);
  return curFormula;
}

void formula_logRecursive(formula_ptr formula, unsigned int *a, unsigned int level)
{
  unsigned int A=*a;
  (*a)--;
  unsigned int b=level;
  char *intention=mem_calloc(level + 1,sizeof(char));
  do
  {
    strcat(intention," ");
  } while(b-->0);

  switch(formula->nodes[A]->type)
  {
    // tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:  log_trace("%s%s",intention,operatorNames[formula->nodes[A]->type]); break;
    case OSL_FALSE: log_trace("%s%s",intention,operatorNames[formula->nodes[A]->type]); break;
    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    case ALWAYS:
    {
      log_trace("%s%s",intention,operatorNames[formula->nodes[A]->type]);
      formula_logRecursive(formula, a, ++level);
      break;
    }
    // event:EventMatchingOperatorType  -> ???
    case EVENTMATCH:
    {
      log_trace("%s%s [action_name=%s]",intention,operatorNames[formula->nodes[A]->type],
          ((event_match_op_ptr)formula->nodes[A]->operands)->match_action->action_name);
      break;
    }
    case XPATH:
    {
      log_trace("%sXPath expr=[%s]", intention, formula->nodes[A]->operands);
      break;
    }
    // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:
    case AND:
    case IMPLIES:
    case SINCE:
    {
      log_trace("%s%s",intention,operatorNames[formula->nodes[A]->type]);
      formula_logRecursive(formula, a, ++level);
      formula_logRecursive(formula, a, level);
      break;
    }
    // tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:
    case DURING:
    case WITHIN:
    {
      unsigned long interval =*((unsigned long*)*((unsigned long*)formula->nodes[A]->operands));
      unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(formula->nodes[A]->operands+sizeof(void*))) ));
      log_trace("%s%s [interval=%lu, timeunit=%s]",intention,operatorNames[formula->nodes[A]->type],interval,timeUnit);
      formula_logRecursive(formula, a, ++level);
      break;
    }
    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:
    {
      unsigned long limit=*((unsigned long*)*((unsigned long*)formula->nodes[A]->operands));
      log_trace("%s%s [limit=%lu]",intention,operatorNames[formula->nodes[A]->type],limit);
      formula_logRecursive(formula, a, ++level);
      formula_logRecursive(formula, a, level);
      break;
    }
    // tns:RepLimOperatorType           -> operands=special (1operator_ptr as pointer to 3 different operators(low_limit, high_limit, operator_ptr)
    case REPLIM:
    {
      unsigned long interval =*((unsigned long*)*((unsigned long*)formula->nodes[A]->operands));
      unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(formula->nodes[A]->operands+1*sizeof(void*))) ));
      unsigned long highLimit=*((unsigned long*)*( ((unsigned long*)(formula->nodes[A]->operands+3*sizeof(void*))) ));
      unsigned long lowLimit =*((unsigned long*)*( ((unsigned long*)(formula->nodes[A]->operands+2*sizeof(void*))) ));
      log_trace("%s%s [amount=%lu; timeUnit=%s; lowerLimit=%lu; upperLimit=%lu]",intention,operatorNames[formula->nodes[A]->type],
                                                                                 interval,timeUnit,lowLimit,highLimit);
      formula_logRecursive(formula, a, ++level);
      break;
    }
    // tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:
    {
      unsigned long limit=*((unsigned long*)*((unsigned long*)formula->nodes[A]->operands));
      log_trace("%s%s [limit=%lu]",intention,operatorNames[formula->nodes[A]->type],limit);
      formula_logRecursive(formula, a, ++level);
      break;
    }
    default: log_trace("%s - unsupported operator type", __func__);
  }
  mem_free(intention);
}

void formula_log(formula_ptr formula)
{
  log_debug("Condition formula with %d nodes", formula->n_nodes);
  unsigned int logNode=formula->n_nodes-1;
  if(formula->n_nodes>0) formula_logRecursive(formula,&logNode,0);
}


/******************************************************************************
 *  Formula updating implementation
******************************************************************************/

bool update_formula(void* mech, formula_ptr formula, event_ptr curEvent)
{
  log_trace("%s - updating with event=[%p]", __func__, curEvent);
  unsigned int a;
  for(a=0; a<formula->n_nodes; a++)
  {
    operator_ptr op=formula->nodes[a];
    formula->nodes[a]->eval(op, curEvent, mech);
  }
  // return overall value of mechanism
  log_debug(" [%s] - condition value=[%s]", ((mechanism_ptr)mech)->mech_name, bool_str[formula->nodes[formula->n_nodes-1]->state->value]);

  return formula->nodes[formula->n_nodes-1]->state->value;
}

bool eval_true(operator_ptr curop, event_ptr curEvent, void* mech)
{ // will never change, so it is already set to true with creating new state for this node
  log_trace(" [%s] - evaluating TRUE     node => true", ((mechanism_ptr)mech)->mech_name);
  return TRUE;
}

bool eval_false(operator_ptr curop, event_ptr curEvent, void* mech)
{ // will never change, so it is already set to false with creating new state for this node
  log_trace(" [%s] - evaluating FALSE    node => false", ((mechanism_ptr)mech)->mech_name);
  return FALSE;
}

bool eval_or(operator_ptr curop, event_ptr curEvent, void* mech)
{
  curop->state->value=((operator_ptr*)curop->operands)[0]->state->value ||
                      ((operator_ptr*)curop->operands)[1]->state->value;
  log_trace(" [%s] - evaluating OR       node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_and(operator_ptr curop, event_ptr curEvent, void* mech)
{
  curop->state->value=((operator_ptr*)curop->operands)[0]->state->value && ((operator_ptr*)curop->operands)[1]->state->value;
  log_trace(" [%s] - evaluating AND      node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_impl(operator_ptr curop, event_ptr curEvent, void* mech)
{
  curop->state->value=(!((operator_ptr*)curop->operands)[0]->state->value) ||
                        ((operator_ptr*)curop->operands)[1]->state->value;
  log_trace(" [%s] - evaluating IMPL      node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

// @todo why do we have xml_doc as parameter for event_matches?????
bool eval_event(operator_ptr curop, event_ptr curEvent, void* mech)
{
  if(curEvent!=NULL)
  {
    if(event_matches(curop->operands, curEvent, NULL))
    {
      curop->state->value=TRUE;
      curop->state->count_true++;
    }
  }
  log_trace(" [%s] - evaluating EVENT    node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_xpath(operator_ptr curop, event_ptr curEvent, void* mech)
{
  if(curEvent!=NULL)
  {
    if(curEvent->xmlDoc==NULL) event_createXMLdoc(curEvent);
    //dumpXMLdoc(curEvent->xmlDoc);
    log_debug("xpath: [%s]", curop->operands);
    curop->state->value=exec_xpath(curEvent->xmlDoc, curop->operands);
  }
  log_trace(" [%s] - evaluating XPATH    node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_not(operator_ptr curop, event_ptr curEvent, void* mech)
{
  curop->state->value=!((operator_ptr)curop->operands)->state->value;
  log_trace(" [%s] - evaluating NOT      node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_always(operator_ptr curop, event_ptr curEvent, void* mech)
{
  // If formula op1 was ever false in the past ALWAYS will evaluate to false independently of value of appropriate child node
  if(!curop->state->immutable)
  {
    curop->state->value=((operator_ptr)curop->operands)->state->value;
    if(!curop->state->value && curEvent==NULL)
    {
      log_trace(" [%s] - evaluating ALWAYS:  activating IMMUTABILITY", ((mechanism_ptr)mech)->mech_name);
      curop->state->immutable = TRUE;
    }
  }
  log_trace(" [%s] - evaluating ALWAYS   node => %u (immutable=[%s])", ((mechanism_ptr)mech)->mech_name,  curop->state->value,
                                                                     bool_str[curop->state->immutable]);
  return curop->state->value;
}

bool eval_before(operator_ptr curop, event_ptr curEvent, void* mech)
{
  bool IS_HISTORY_AVAILABLE = FALSE;
  mechanism_ptr curMech=mech;

  unsigned long amount=*((unsigned long*)*((unsigned long*)curop->operands));
  //log_trace("          op1: %ld",amount);
  unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(curop->operands+sizeof(void*))) ));
  //log_trace("          op2: %s",timeUnit, timeUnit);
  operator_ptr op3=*((operator_ptr*)(curop->operands+2*sizeof(void*)));
  //log_trace("          op3: %s",operatorNames[op3->type]);

  if(!strncasecmp(timeUnit,"timestep",8))
  {
    int passed=curMech->timestep - amount;
    if( passed>=0 ) IS_HISTORY_AVAILABLE=TRUE;
    log_trace(" [%s] - evaluating BEFORE   history available=[%s]", curMech->mech_name, bool_str[IS_HISTORY_AVAILABLE]);
  }
  else
  {
    log_error(" [%s] - evaluating BEFORE:  not yet implemented timeUnit (%s)", curMech->mech_name, timeUnit);
    return FALSE;
  }

  // before = at (currentTime - interval) op3 was true
  //c_array_ptr sv = curop->state->saved_values;
  c_array_ptr sv = curop->state->saved_values.bitArray;
  if(IS_HISTORY_AVAILABLE)
  { // If history is available recover the first (resp. oldest) value
    if(c_array_get(sv, sv->p_first)) curop->state->value = TRUE;
    else curop->state->value = FALSE;
  }
  else curop->state->value = TRUE;
  // If there isn't history formula is TRUE // @todo do we still want this behaviour??

  // Updating saved values at end of timestep
  if(curEvent==NULL)
  { // Update the circular buffer pointers; When there is enough history first==next
    if (IS_HISTORY_AVAILABLE)
    {
      sv->p_first++;
      if(c_array_get(sv,sv->p_first)==amount)  sv->p_first=0;
    }
    // Save the current value of op3 in the array
    c_array_set(sv,sv->p_next,op3->state->value);
    // Increment next pointer
    sv->p_next++;
    // Reset next if bigger then array size
    if (sv->p_next==amount) sv->p_next=0;
  }
  log_trace(" [%s] - evaluating BEFORE   node => %u", curMech->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_during(operator_ptr curop, event_ptr curEvent, void* mech)
{
  bool IS_HISTORY_AVAILABLE = FALSE;
  mechanism_ptr curMech=mech;

  unsigned long amount=*((unsigned long*)*((unsigned long*)curop->operands));
  unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(curop->operands+sizeof(void*))) ));
  operator_ptr op3=*((operator_ptr*)(curop->operands+2*sizeof(void*)));
  //log_trace("          op1: %ld",amount);
  //log_trace("          op2: %s",timeUnit, timeUnit);
  //log_trace("          op3: %s",operatorNames[op3->type]);

  if(!strncasecmp(timeUnit,"timestep",8))
  {
    int passed=curMech->timestep - amount;
    if( passed>=0 ) IS_HISTORY_AVAILABLE=TRUE;
    log_trace(" [%s] - evaluating DURING   history available=[%s]", curMech->mech_name, bool_str[IS_HISTORY_AVAILABLE]);
  }
  else
  {
    log_error(" [%s] - evaluating DURING:  not yet implemented timeUnit (%s)", curMech->mech_name, timeUnit);
    return FALSE;
  }

  // Temporary variable pointing to circular buffer
  //c_array_ptr sv = curop->state->saved_values;
  c_array_ptr sv = curop->state->saved_values.bitArray;
  if (IS_HISTORY_AVAILABLE)
  { // During all previously time steps
    int cmp_val=op3->state->count_true;
    if(op3->state->value) cmp_val++;

    if (cmp_val == amount) curop->state->value=TRUE;
    else curop->state->value=FALSE;
  }
  else  curop->state->value = TRUE;    // If there isn't history formula is TRUE // @todo still intended behaviour?

  if(curEvent==NULL)   // End of a time step
  { // Update the circular buffer pointers; When there is enough history first==next
    if(IS_HISTORY_AVAILABLE)
    { // If the formula was true discount the true from the counter of true values
      if(c_array_get(sv,sv->p_first)) op3->state->count_true--;
      sv->p_first++;
      if(c_array_get(sv,sv->p_first)==amount)  sv->p_first=0;
    }

    // Add counter if true in the current time step
    if(op3->state->value) op3->state->count_true++;
    // Save the current value of op3 in the array
    c_array_set(sv,sv->p_next,op3->state->value);
    // Increment next pointer
    sv->p_next++;
    // Reset next if bigger then array size
    if(sv->p_next==amount) sv->p_next=0;
  }
  log_trace(" [%s] - evaluating DURING   node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_within(operator_ptr curop, event_ptr curEvent, void* mech)
{
  bool IS_HISTORY_AVAILABLE = FALSE;
  mechanism_ptr curMech=mech;

  unsigned long amount=*((unsigned long*)*((unsigned long*)curop->operands));
  unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(curop->operands+sizeof(void*))) ));
  operator_ptr op3=*((operator_ptr*)(curop->operands+2*sizeof(void*)));
  //log_trace("          op1: %ld",amount);
  //log_trace("          op2: %s",timeUnit);
  //log_trace("          op3: %s",operatorNames[op3->type]);

  if(!strncasecmp(timeUnit,"timestep",8))
  {
    int passed=curMech->timestep - amount;
    if( passed>=0 ) IS_HISTORY_AVAILABLE=TRUE;
    log_trace(" [%s] - evaluating WITHIN   history available=[%s]", curMech->mech_name, bool_str[IS_HISTORY_AVAILABLE]);
  }
  else
  {
    log_error(" [%s] - evaluating WITHIN:  not yet implemented timeUnit (%s)", curMech->mech_name, timeUnit);
    return FALSE;
  }

  // Temporary variable pointing to circular buffer
  //c_array_ptr sv = curop->state->saved_values;
  c_array_ptr sv = curop->state->saved_values.bitArray;
  if (IS_HISTORY_AVAILABLE)
  { // At least once within the previous time steps
    int cmp_val = op3->state->count_true;
    if(op3->state->value) cmp_val++;
    if(cmp_val > 0) curop->state->value=TRUE;
    else curop->state->value=FALSE;
  }
  else curop->state->value=TRUE;   // If there isn't history formula is TRUE

  if(curEvent==NULL)   // End of a time step
  { // Update the circular buffer pointers; When there is enough history first==next
    if(IS_HISTORY_AVAILABLE)
    { // If the formula was true discount the true from the counter of true values
      if(c_array_get(sv,sv->p_first)) op3->state->count_true--;
      sv->p_first++;
      if(c_array_get(sv,sv->p_first)==amount)  sv->p_first=0;
    }
    // Add counter if true in the current time step
    if(op3->state->value) op3->state->count_true++;
    // Save the current value of op2 in the array
    c_array_set(sv,sv->p_next,op3->state->value);
    // Increment next pointer
    sv->p_next++;
    // Reset next if bigger then array size
    if(sv->p_next==amount) sv->p_next=0;
  }
  log_trace(" [%s] - evaluating WITHIN   node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_since(operator_ptr curop, event_ptr curEvent, void* mech)
{ // A since B = since(A, B);  ==> A = op1 / B = op2
  operator_ptr op1=*((operator_ptr*)(curop->operands));
  operator_ptr op2=*((operator_ptr*)(curop->operands+1*sizeof(void*)));
  //log_trace("          op1: %s",operatorNames[op1->type]);
  //log_trace("          op2: %s",operatorNames[op2->type]);

  if (op2->state->value && !op2->state->was_ever_true)
  { // First time B happens
    curop->state->value = TRUE;
  }
  else
  {
    if(!curop->state->immutable)
    {
      if(op2->state->value || op2->state->was_ever_true)
      {
        if(op1->state->value) curop->state->value = TRUE;
        else curop->state->value = FALSE;
      }
      else  curop->state->value = TRUE;
    }
  }

  // We update the states
  if(curEvent==NULL)
  {
    if(!curop->state->value)
    {
      log_trace(" [%s] - evaluating SINCE    activating IMMUTABILITY", ((mechanism_ptr)mech)->mech_name);
      curop->state->immutable=TRUE;
    }
    if(op2->state->value)
    {
      op2->state->was_ever_true=TRUE;
      curop->state->immutable=FALSE;
    }
  }
  log_trace(" [%s] - evaluating SINCE    node => %u (immutable=[%s])", ((mechanism_ptr)mech)->mech_name,  curop->state->value,
                                                                      bool_str[curop->state->immutable]);
  return curop->state->value;
}

bool eval_repsince(operator_ptr curop, event_ptr curEvent, void* mech)
{ // A(n) since B = repsince(n, A, B); // n = op1 / A = op2 / B = op3
  // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr, 3rd operand: operator_ptr
  unsigned long limit=*((unsigned long*)*((unsigned long*)curop->operands));
  operator_ptr op2=*((operator_ptr*)(curop->operands+1*sizeof(void*)));
  operator_ptr op3=*((operator_ptr*)(curop->operands+1*sizeof(void*)));
  //log_trace("          op1: %ld",limit);
  //log_trace("          op2: %s",operatorNames[op2->type]);
  //log_trace("          op3: %s",operatorNames[op3->type]);

  // B was never TRUE and is not TRUE now // first time B appears, it is TRUE
  if(op3->state->value && !op3->state->was_ever_true) curop->state->value = TRUE;
  else
  {
    int cmp_val=op2->state->count_true;
    if(op2->state->value) cmp_val++;
    if(op3->state->value || op3->state->was_ever_true)
    { // B is TRUE now or already happen before
      if(cmp_val<=limit || op3->state->value)
      { // A count true is not violated
        curop->state->value=TRUE;
      }
      else
      { // A count true violated
        curop->state->value=FALSE;
      }
    }
    else curop->state->value = TRUE;
  }

  // We update the states only with NULL event
  if(curEvent)
  {
    if(op2->state->value) op2->state->count_true++;
    if(op3->state->value)
    {
      op3->state->was_ever_true=TRUE;
      op2->state->count_true=0;
    }
  }
//  plog_formula("REPSINCE", i,cur_state);
//  plog(LOG_TRACE, "    immutable         (%d)", cur_state->immutable);
//  plog(LOG_TRACE, "    op1               (%d)", formula->op1);
//  plog(LOG_TRACE, "    op2 count_true    (%d)", cur_states[formula->op2].count_true);
//  plog(LOG_TRACE, "    op3 was_ever_true (%d)", cur_states[formula->op3].was_ever_true);
  log_trace(" [%s] - evaluating REPSINCE node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}

bool eval_repmax(operator_ptr curop, event_ptr curEvent, void* mech)
{ // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr
  unsigned long limit=*((unsigned long*)*((unsigned long*)curop->operands));
  operator_ptr op2=*((operator_ptr*)(curop->operands+1*sizeof(void*)));
  //log_trace("I'm evaluating repmax...");

  // We stop checking after the formula was violated because of the cardinality checking, once violated it will be never true again
  if(!curop->state->immutable)
  { // check the counter and current state considering that the counter is not updated yet
    //log_trace("NOT yet immutable.");
    if( (op2->state->value  && ( (op2->state->count_true + 1) <= limit)) ||
        (!op2->state->value && ( (op2->state->count_true)     <= limit)) )
    {
      //log_trace("ok, set current state value to TRUE "
      //          "(op2->state->value=[%d], op2->state->count_true=[%d]",
      //              op2->state->value, op2->state->count_true);
      curop->state->value=TRUE;
    }
    else
    {
      //log_trace("set current state value to FALSE"
      //    "(op2->state->value=[%d], op2->state->count_true=[%d]",
      //        op2->state->value, op2->state->count_true);
      curop->state->value=FALSE;
    }

    // We only update the counter at the end of the time step if the formula was not violated
    if(curEvent==NULL)
    {
      //log_trace("updating counters (received NULL event...");
      if(op2->state->value)
      {
        //log_trace("increment counters...");
        op2->state->count_true++;
      }
      if(!curop->state->value)
      {
        //log_trace("set to immutable!");
        curop->state->immutable=TRUE;
      }
    } //else log_trace("not updating counters, not NULL event");
  }
  //        plog_formula("REPMAX", i,cur_state);
  //        plog(LOG_TRACE, "    immutable      (%d)", cur_state->immutable);
  //        plog(LOG_TRACE, "    op1            (%d)", formula->op1);
  //        plog(LOG_TRACE, "    op2 count_true (%d)", cur_states[formula->op2].count_true);
  log_trace(" [%s] - evaluating REPMAX   node => %u (immutable=[%s])", ((mechanism_ptr)mech)->mech_name,  curop->state->value,
                                                                       bool_str[curop->state->immutable]);
  return curop->state->value;
}

bool eval_replim(operator_ptr curop, event_ptr curEvent, void* mech)
{ // 1st operand: (unsigned long)amount, 2nd operand: (unsigned char*)timeunit,
  // 3rd operand: (unsigned long)LowLimit, 4th operand: (unsigned long)Highlimit, 5th operand: operator_ptr
  bool IS_HISTORY_AVAILABLE = FALSE;
  mechanism_ptr curMech=mech;

  unsigned long amount=*((unsigned long*)*((unsigned long*)curop->operands));
  unsigned char *timeUnit=*( (unsigned char**)*( ((unsigned char**)(curop->operands+sizeof(void*))) ));

  unsigned long  lowLimit=*((unsigned long*)*((unsigned long*)(curop->operands + 2*sizeof(void*))));
  unsigned long highLimit=*((unsigned long*)*((unsigned long*)(curop->operands + 3*sizeof(void*))));
  operator_ptr op5       =*((operator_ptr*)  (curop->operands+4*sizeof(void*)));
  //log_trace("          op1: %ld",amount);
  //log_trace("          op2: %s",timeUnit);
  //log_trace("          op3: %ld",lowLimit);
  //log_trace("          op4: %ld",highLimit);
  //log_trace("          op5: %s",operatorNames[op5->type]);

  if(!strncasecmp(timeUnit,"timestep",8))
  {
    int passed=curMech->timestep - amount;
    if( passed>=0 ) IS_HISTORY_AVAILABLE=TRUE;
    log_trace(" [%s] - evaluating REPLIM   history available=[%s]", curMech->mech_name, bool_str[IS_HISTORY_AVAILABLE]);
  }
  else
  {
    log_error(" [%s] - evaluating REPLIM:  not yet implemented timeUnit (%s)", curMech->mech_name, timeUnit);
    return FALSE;
  }

  // Temporary variable pointing to circular buffer
  //sv = cur_state->saved_values;
  cint_array_ptr sv = curop->state->saved_values.intArray;
  if(IS_HISTORY_AVAILABLE)
  { // During all previously time steps
    if( op5->state->count_true >= lowLimit && op5->state->count_true <= highLimit)
         curop->state->value=TRUE;
    else curop->state->value=FALSE; // changed
  }
  else  curop->state->value = TRUE;  // If there isn't history formula is TRUE

  // End of a time step
  if(curEvent==NULL)
  { // Update the circular buffer pointers; When there is enough history first==next
    if(IS_HISTORY_AVAILABLE)
    { // If the formula was true discount the true from the counter of true values
      if(cint_array_get(sv,sv->p_first)) op5->state->count_true--;
      sv->p_first++;
      if(cint_array_get(sv,sv->p_first)==amount) sv->p_first=0;
    }
    // Add counter if true in the current time step
    if(op5->state->value) op5->state->count_true++;
    // Save the current value of op2 in the array
    cint_array_set(sv,sv->p_next,op5->state->value);
    // Increment next pointer
    sv->p_next++;
    // Reset next if bigger then array size
    if(sv->p_next==amount) sv->p_next=0;
  }
  //        plog_formula("REPLIM", i, cur_state);
  //        plog(LOG_TRACE, "    Elapsed time since start (%d usec)", usec_elpsd_start);
  //        plog(LOG_TRACE, "    Time window          - op1 (%d)", formula->op1);
  //        plog(LOG_TRACE, "    Lower limit          - op2 (%d)", formula->op2);
  //        plog(LOG_TRACE, "    Upper limit          - op3 (%d)", formula->op3);
  //        plog(LOG_TRACE, "    Saved history        - op4 (%d)", cur_states[formula->op4].value);
  //        plog(LOG_TRACE, "    Count true next step - op4 (%d)", cur_states[formula->op4].count_true);
  log_trace(" [%s] - evaluating REPLIM   node => %u", ((mechanism_ptr)mech)->mech_name,  curop->state->value);
  return curop->state->value;
}


