/**
 * @file  condition.c
 * @brief Implementation of mechanism condition, i.e. the OSL condition, which will be evaluated
 *
 * @see condition.h, condition_s
 *
 * @author cornelius moucha
 **/

#include "condition.h"
#include "log_condition_pef.h"

extern pdp_ptr pdp;

/******************************************************************************
 *  Condition formula implementation
******************************************************************************/

condition_ptr conditionNew()
{
  condition_ptr ncondition=memAlloc(sizeof(condition_t));
  checkNullPtr(ncondition, "Could not allocate memory for condition formula");

  ncondition->cntNodes=0;
  ncondition->nodes=NULL;
  return ncondition;
}

unsigned int conditionFree(condition_ptr condition, mechanism_ptr curMech)
{
  log_debug("deallocating condition");
  if(condition->nodes!=NULL)
  {
    log_debug("deallocating operator-nodes (%d)", condition->cntNodes);
    if(operatorFree(condition->nodes[condition->cntNodes-1], curMech)==R_ERROR) {log_error("Error deallocating operator in condition formula"); return R_ERROR;}
    free(condition->nodes);
  }
  free(condition);
  return R_SUCCESS;
}

unsigned long conditionMemSize(condition_ptr condition)
{ /// @todo check
  unsigned long size=sizeof(condition_ptr);
  size+=sizeof(unsigned int);
  unsigned int a;
  for(a=0; a<condition->cntNodes; a++)
    size+=operatorMemSize(condition->nodes[a]);
  size+=sizeof(operator_ptr*);
  return size;
}

operator_ptr conditionParseSubCondition(condition_ptr curCondition, xmlNodePtr node, unsigned int *curnode, mechanism_ptr curMechanism)
{
  operator_ptr op=NULL;
  char curType=getOperatorType((char*)node->name);
  switch(curType)
  {
    //   tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:
    case OSL_FALSE: op=operatorNew(curType); break;
    //   tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    case ALWAYS:
    {
      op=operatorNew(curType);
      op->operands=conditionParseSubCondition(curCondition, xmlGetFirstChildElement(node), curnode, curMechanism);
      break;
    }
    //   event:EventMatchingOperatorType, XPATH  -> ???
    case EVENTMATCH:
    {
      op=operatorNew(curType);
      op->operands=eventMatchParseXML(pdp->actionDescStore, node);

      // subscribe to PEP for EVENTMATCH:
      #if PDP_PEPSUBSCRIPTION == 1
        //unsigned int ret=pepSubscribe(((eventMatch_ptr)op->operands)->matchAction->actionName, 0);
        pdpInterface_ptr curInterface=((eventMatch_ptr)op->operands)->matchAction->pepInterface;
        unsigned int ret=curInterface->pepSubscribe(curInterface,
                            ((eventMatch_ptr)op->operands)->matchAction->actionName, 0);
        log_debug("PEP subscription for event name=[%s]=>[%d]", ((eventMatch_ptr)op->operands)->matchAction->actionName, ret);
      #endif

      // cannot subscribe the mechanism to the list; the mechanism is NOT interested in this eventMatch, only this very operator!
      log_trace("Subscribing this eventMatch condition operand to the list of condition nodes");
      ((eventMatch_ptr)op->operands)->matchAction->conditions=g_slist_append(((eventMatch_ptr)op->operands)->matchAction->conditions, op);
      break;
    }
    case XPATH:
    {
      op=operatorNew(XPATH);
      xmlNodePtr tmp = NULL;
      for(tmp=node->children; tmp; tmp=tmp->next)
        if(tmp->type==XML_TEXT_NODE)
        {
          char *content=(char*)xmlNodeGetContent(tmp);
          op->operands=memCalloc(strlen(content)+1, sizeof(char));
          strncpy(op->operands, content, strlen(content));
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
      op=operatorNew(curType);
      oslBinary_ptr curOperands=op->operands;

      unsigned int iops=0;
      xmlNodePtr nextNode=NULL;
      for(nextNode=node->children; nextNode; nextNode=nextNode->next)
      {
        if(xmlIsElement(nextNode)) curOperands->operand[iops++]=conditionParseSubCondition(curCondition, nextNode, curnode, curMechanism);
        if(iops==2) break;
      }
      break;
    }
    //   tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:
    case DURING:
    case WITHIN:
    { // 1st operand: (unsigned long)amount, 2nd operand: (char*)timeunit,
      // 3rd operand: operator_ptr to nontemporal operator (propositional or event)
      op=operatorNew(curType);
      char *amountStr=xmlGetProp(node, "amount");
      unsigned long amount=atol(amountStr);
      xmlFree(amountStr);

      char *tmp=(char*)xmlGetProp(node, "unit");
      char *timeUnit;
      if(tmp==NULL)
      {
        timeUnit=memAlloc(9*sizeof(char));
        sprintf(timeUnit,"timestep");
      }
      else timeUnit=strdup(tmp);
      free(tmp);

      uint64_t beforeInterval=amount*getTimeUnitMultiplier(timeUnit);
      uint64_t intervalRelative=div(beforeInterval, curMechanism->timestepSize).quot;

      // for evaluation without history set counter to interval for DURING
      if(curType==DURING) op->state->counter=intervalRelative+1;

      oslTimeUnary_ptr curOperands=op->operands;
      curOperands->amount=amount;
      curOperands->unit=timeUnit;
      curOperands->operand=conditionParseSubCondition(curCondition, xmlGetFirstChildElement(node), curnode, curMechanism);
      curOperands->interval=intervalRelative;

      // allocate bit array for (interval+1) timestep-windows (circular bit-array is only used for BEFORE-operator)
      if(curType==BEFORE) op->state->savedValues.bitArray=circArrayNew(curOperands->interval+1);
      break;
    }
    //   tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2*operator_ptr)
    case REPSINCE:
    { // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr, 3rd operand: operator_ptr
      xmlNodePtr nextNode=NULL;
      op=operatorNew(REPSINCE);
      char *limitStr=xmlGetProp(node, "limit");
      unsigned long limit=atol(limitStr);
      xmlFree(limitStr);

      oslRepSince_ptr curOperands=op->operands;
      curOperands->limit=limit;

      unsigned int iops=0;
      for(nextNode=node->children; nextNode, iops<2; nextNode=nextNode->next)
      {
        if(xmlIsElement(nextNode)) curOperands->operand[iops++]=conditionParseSubCondition(curCondition, nextNode, curnode, curMechanism);
      }
      break;
    }
    //   tns:RepLimOperatorType  -> operands=special (1operator_ptr as pointer to 3 different operators(low_limit, high_limit, operator_ptr
    case REPLIM:
    { // 1st operand: (unsigned long)amount, 2nd operand: (char*)timeunit,
      // 3rd operand: (unsigned long)LowLimit, 4th operand: (unsigned long)Highlimit, 5th operand: operator_ptr
      op=operatorNew(REPLIM);
      char *lowLimitStr=xmlGetProp(node, "lowerLimit");
      char *highLimitStr=xmlGetProp(node, "upperLimit");
      unsigned long lowLimit=atol(lowLimitStr);
      unsigned long highLimit=atol(highLimitStr);
      xmlFree(lowLimitStr);
      xmlFree(highLimitStr);

      char *amountStr=xmlGetProp(node, "amount");
      unsigned long amount=atol(amountStr);
      xmlFree(amountStr);
      char *tmp=xmlGetProp(node, "unit");
      char *timeUnit;
      if(tmp==NULL)
      {
        timeUnit=memAlloc(9*sizeof(char));
        sprintf(timeUnit,"timestep");
      }
      else timeUnit=strdup(tmp);
      free(tmp);

      uint64_t beforeInterval=amount*getTimeUnitMultiplier(timeUnit);
      uint64_t intervalRelative=div(beforeInterval, curMechanism->timestepSize).quot;

      oslRepLim_ptr curOperands=op->operands;
      curOperands->amount=amount;
      curOperands->unit=timeUnit;
      curOperands->operand=conditionParseSubCondition(curCondition, xmlGetFirstChildElement(node), curnode, curMechanism);
      curOperands->interval=intervalRelative;
      curOperands->lowerLimit=lowLimit;
      curOperands->upperLimit=highLimit;

      op->state->savedValues.bitArray=circArrayNew(curOperands->interval+1);
      break;
    }
    //   tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:
    { // 1st operand: (unsigned long)limit, 2nd operand: operator_ptr
      op=operatorNew(REPMAX);
      char *limitStr=xmlGetProp(node, "limit");
      unsigned long limit=atol(limitStr);
      xmlFree(limitStr);

      oslRepMax_ptr curOperands=op->operands;
      curOperands->upperLimit=limit;
      curOperands->operand=conditionParseSubCondition(curCondition, xmlGetFirstChildElement(node), curnode, curMechanism);
      break;
    }
    default: log_error("Not supported operator node: %s\n",node->name); break;
  }
  operatorLog(op);
  curCondition->nodes[*curnode]=op;
  (*curnode)++;
  return op;
}

unsigned int conditionParseXMLstage1(condition_ptr curCondition, xmlNodePtr subCondition)
{
  xmlNodePtr cur_node=NULL;
  for(cur_node=subCondition->children; cur_node; cur_node = cur_node->next)
  {
    if(xmlIsElement(cur_node) && getOperatorType(cur_node->name)!=NO_OSL_OP)
    {
      curCondition->cntNodes++;
      conditionParseXMLstage1(curCondition, cur_node);
    }
  }
  return R_SUCCESS;
}

condition_ptr conditionParseXML(xmlNodePtr rootNode, mechanism_ptr mech)
{
  checkNullPtr(rootNode, "Empty condition formula?");
  condition_ptr curCondition=conditionNew();
  conditionParseXMLstage1(curCondition, rootNode);
  curCondition->nodes=memCalloc(curCondition->cntNodes, sizeof(operator_ptr));

  unsigned int lcurnode=0;
  conditionParseSubCondition(curCondition, xmlGetFirstChildElement(rootNode), &lcurnode, mech);
  conditionLog(curCondition);
  return curCondition;
}

void conditionLogRecursive(condition_ptr condition, unsigned int *a, unsigned int level)
{
  unsigned int A=*a;
  (*a)--;
  unsigned int b=level;
  char *intention=memCalloc(level+2,sizeof(char));
  sprintf(intention, "");
  do
  {
    strcat(intention," ");
  } while(b-->0);

  switch(condition->nodes[A]->type)
  {
    // tns:EmptyOperatorType            -> operands=NULL
    case OSL_TRUE:  log_trace("%s%s",intention,operatorNames[condition->nodes[A]->type]); break;
    case OSL_FALSE: log_trace("%s%s",intention,operatorNames[condition->nodes[A]->type]); break;
    // tns:UnaryOperatorType            -> operands=1 operator_ptr
    case NOT:
    case ALWAYS:
    {
      log_trace("%s%s",intention,operatorNames[condition->nodes[A]->type]);
      conditionLogRecursive(condition, a, ++level);
      break;
    }
    // event:EventMatchingOperatorType  -> ???
    case EVENTMATCH:
    {
      log_trace("%s%s [actionName=%s]",intention,operatorNames[condition->nodes[A]->type],
          ((eventMatch_ptr)condition->nodes[A]->operands)->matchAction->actionName);
      break;
    }
    case XPATH:
    {
      log_trace("%sXPath expr=[%s]", intention, condition->nodes[A]->operands);
      break;
    }
    // tns:BinaryOperatorType           -> operands=2 operator_ptr
    case OR:
    case AND:
    case IMPLIES:
    case SINCE:
    {
      log_trace("%s%s",intention,operatorNames[condition->nodes[A]->type]);
      conditionLogRecursive(condition, a, ++level);
      conditionLogRecursive(condition, a, level);
      break;
    }
    // tns:TimeBoundedUnaryOperatorType -> operands=special (1operator_ptr as pointer to 3 different operators(amount, unit, operator)
    case BEFORE:
    case DURING:
    case WITHIN:
    {
      oslTimeUnary_ptr curOperand=condition->nodes[A]->operands;
      log_trace("%s%s [amount=%lu, timeunit=%s => interval=%llu timesteps]",intention,operatorNames[condition->nodes[A]->type],curOperand->amount,curOperand->unit,
                                                                            curOperand->interval);
      conditionLogRecursive(condition, a, ++level);
      break;
    }
    // tns:RepSinceOperatorType         -> operands=special (1operator_ptr as pointer to 3 different operators(limit, 2 * operator_ptr)
    case REPSINCE:
    {
      oslRepSince_ptr curOperand=condition->nodes[A]->operands;
      log_trace("%s%s [limit=%lu]",intention,operatorNames[condition->nodes[A]->type],curOperand->limit);
      conditionLogRecursive(condition, a, ++level);
      conditionLogRecursive(condition, a, level);
      break;
    }
    // tns:RepLimOperatorType           -> operands=special (1operator_ptr as pointer to 3 different operators(low_limit, high_limit, operator_ptr)
    case REPLIM:
    {
      oslRepLim_ptr curOperand=condition->nodes[A]->operands;
      log_trace("%s%s [amount=%lu; timeUnit=%s => interval=%llu timesteps; lowerLimit=%lu; upperLimit=%lu]",intention,operatorNames[condition->nodes[A]->type],
                                                                                 curOperand->amount,curOperand->unit,curOperand->interval,
                                                                                 curOperand->lowerLimit,curOperand->upperLimit);
      conditionLogRecursive(condition, a, ++level);
      break;
    }
    // tns:RepMaxOperatorType           -> operands=special (1operator_ptr as pointer to 2 different operators(limit, operator_ptr)
    case REPMAX:
    {
      oslRepMax_ptr curOperand=condition->nodes[A]->operands;
      log_trace("%s%s [limit=%lu]",intention,operatorNames[condition->nodes[A]->type],curOperand->upperLimit);
      conditionLogRecursive(condition, a, ++level);
      break;
    }
    default: log_trace("%s - unsupported operator type", __func__); break;
  }
  memFree(intention);
}

void conditionLog(condition_ptr condition)
{
  log_debug("Condition formula with %d nodes", condition->cntNodes);
  unsigned int logNode=condition->cntNodes-1;
  if(condition->cntNodes>0) conditionLogRecursive(condition,&logNode,0);
}


/******************************************************************************
 *  Condition updating implementation
******************************************************************************/
bool conditionUpdate(mechanism_ptr mechanism, condition_ptr condition, event_ptr event)
{
  //log_trace("\n\n%s - --------------------------------", __func__);
  log_trace("%s - updating with event=[%p]", __func__, event);
  unsigned int a;
  for(a=0; a<condition->cntNodes; a++)
  {
    operator_ptr op=condition->nodes[a];
    condition->nodes[a]->eval(op, event, mechanism);
  }
  // return overall value of mechanism
  log_debug(" [%s] - condition value=[%s]", mechanism->mechName, boolStr[condition->nodes[condition->cntNodes-1]->state->value]);
  return condition->nodes[condition->cntNodes-1]->state->value;
}



