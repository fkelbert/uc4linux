#include "pdp/mechanism.h"
#include "log_mechanism_c.h"

extern action_desc_store_ptr pdp_action_desc_store;

/******************************************************************************
 *  Action methods
******************************************************************************/
action_ptr action_new(const unsigned char *name)
{
  action_desc_ptr laction = find_action_desc_by_name(pdp_action_desc_store, (char*)name);
  if(laction == NULL)
  {
    log_trace("Mentioned execute action not present in action_description_store -> adding it");
    // Default Value is ACTION_USAGE -> as defined in action schema (action-FhG.xsd)
    laction = action_desc_new((char*)name, ACTION_USAGE);
    if(add_action_desc(pdp_action_desc_store, laction)!=R_SUCCESS)
    {
      log_error("Failure parsing event: action_description could not be added to store!");
      return NULL;
    }
  }
  action_ptr nAction=mem_alloc(sizeof(action_t));
  check_null_ptr(nAction,"Could not allocate memory for action");
  nAction->action=laction;
  nAction->n_params_to_action=0;
  nAction->params=NULL;
  log_trace("%s - successfully created new action=[%s]", __func__, name);
  return nAction;
}

unsigned int action_free(action_ptr action)
{
//  free(action->name);
//  unsigned int a;
//  for(a=0; a<action->n_params; a++)
//    if(param_free(action->params[a])) return R_ERROR;
//  free(action);
  return R_SUCCESS;
}

unsigned int action_addParamInstance(mechanism_ptr curMech, action_ptr lAction, const unsigned char *name,
                             const unsigned char *value, const unsigned char *isXPath)
{
  check_null_int(curMech, "%s - curMech is NULL",__func__);
  check_null_int(lAction, "%s - lAction is NULL",__func__);
  check_null_int(name, "%s - name is NULL",__func__);
  check_null_int(value, "%s - value is NULL",__func__);
  check_null_int(lAction->action, "%s - referenced action is NULL?!", __func__);

  lAction->params=realloc(lAction->params,(lAction->n_params_to_action+1)*sizeof(action_match_op_ptr));
  check_null_int(lAction->params,"Could not allocate memory for new parameter");

  // @todo casting to (char*) should be not necessary! modify method find_param_desc_by_name to use const unsigned char *
  action_param_desc_ptr lparam = find_param_desc_by_name(lAction->action, (char*)name);
  if(lparam == NULL)
  {
    if(add_param_desc(lAction->action, (char*)name, "string")!=R_SUCCESS)
    {
      log_error("Parameter Description could not be added");
      return R_ERROR;
    }
    lparam = find_param_desc_by_name(lAction->action, (char*)name);
  }

  lAction->params[lAction->n_params_to_action]=action_match_op_new(lparam,value,isXPath);
  check_null_int(lAction->params[lAction->n_params_to_action],"Could not add parameter instance to mechanism_actions");

  log_debug("Successfully added parameter instance \"%s\" with value \"%s\", isXPath=[%d] to action_ptr",
              lAction->params[lAction->n_params_to_action]->param_desc->name,
              lAction->params[lAction->n_params_to_action]->value,
              lAction->params[lAction->n_params_to_action]->is_xpath);
  lAction->n_params_to_action++;
  return R_SUCCESS;
}

unsigned long actions_getMemSize(action_ptr action)
{
  unsigned long size=sizeof(action_ptr);
  //size+=(strlen(action->name)+1)*sizeof(unsigned char);
  size+=sizeof(unsigned char*);
  size+=sizeof(unsigned int);
  //unsigned int a;
  //for(a=0; a<action->n_params; a++)
  //  size+=param_getMemSize(action->params[a]);
  return size;
}

action_match_op_ptr action_match_op_new(action_param_desc_ptr lparam, const unsigned char *value, const unsigned char *isXPath)
{
  check_null_ptr(lparam, "%s - lparam is NULL", __func__);
  check_null_ptr(lparam, "%s - value is NULL", __func__);
  action_match_op_ptr nActionMatchOp = mem_alloc(sizeof(action_match_op_t));
  nActionMatchOp->param_desc=lparam;
  nActionMatchOp->value=strdup(value);
  nActionMatchOp->is_xpath=(isXPath==NULL ? FALSE : strncmp(isXPath, "true", 4)==0 );
  return nActionMatchOp;
}

/******************************************************************************
 *  Mechanism_actions implementation
******************************************************************************/
mechanism_actions_ptr mechanism_actions_new(unsigned int response, unsigned int delay)
{
  mechanism_actions_ptr nmechactions=mem_alloc(sizeof(mechanism_actions_t));
  check_null_ptr(nmechactions, "Could not allocate memory for mechanism_actions");

  nmechactions->response=response;
  nmechactions->delay=delay;
  nmechactions->n_params_to_modify=0;
  nmechactions->params_to_modify=NULL;
  log_debug("Successfully allocated new mechanism_action (response: %d, delay: %d)", nmechactions->response, nmechactions->delay);
  return nmechactions;
}

unsigned int mechanism_addAction(mechanism_ptr curMech, xmlNodePtr node, const unsigned char *name)
{ // node = XMLnode action -> increase list of action_ptr's and parse parameters in action
  check_null_int(curMech, "%s - curMech is NULL",__func__);
  check_null_int(curMech->trigger->match_action, "%s - trigger action is NULL?!", __func__);
  curMech->actions_to_execute=realloc(curMech->actions_to_execute,(curMech->n_actions_to_execute+1)*sizeof(action_ptr));
  check_null_int(curMech->actions_to_execute, "Could not allocate memory for new action");

  action_ptr nAction = action_new(name);

  // parse parameters associated to action
  xmlNodePtr cur=node->children;
  while(cur!=NULL)
  {
    if(xmlIsElement(cur) && !xmlStrncasecmp(cur->name, "parameter",9))
      action_addParamInstance(curMech, nAction, xmlGetProp(cur,"name"), xmlGetProp(cur,"value"),xmlGetProp(cur,"isXPath"));
    cur=cur->next;
  }

  curMech->actions_to_execute[curMech->n_actions_to_execute]=nAction;
  log_trace("Successfully added action \"%s\" mechanism", name);
  curMech->n_actions_to_execute++;
  return R_SUCCESS;
}

unsigned int mechanism_actions_addModifyParam(mechanism_ptr curMech, mechanism_actions_ptr curMechAction, const unsigned char *name,
                                              const unsigned char *value, const unsigned char *isXPath)
{
  check_null_int(curMech, "%s - curMech is NULL",__func__);
  check_null_int(curMechAction, "%s - curMechAction is NULL",__func__);
  check_null_int(name, "%s - name is NULL",__func__);
  check_null_int(value, "%s - value is NULL",__func__);

  // reference to actions of trigger!
  curMechAction->params_to_modify=realloc(curMechAction->params_to_modify,(curMechAction->n_params_to_modify+1)*sizeof(action_match_op_ptr));
  check_null_int(curMechAction->params_to_modify,"Could not allocate memory for new parameter");

  check_null_int(curMech->trigger->match_action, "%s - trigger action is NULL?!", __func__);
  action_desc_ptr laction = curMech->trigger->match_action;
  // @todo casting (char*) should be not necessary! modify method find_param_desc_by_name to use const unsigned char *
  action_param_desc_ptr lparam = find_param_desc_by_name(laction, (char*)name);
  if(lparam == NULL)
  {
    log_trace("Mentioned parameter in modify not present in action_description -> adding it");
    if(add_param_desc(laction, (char*)name, "string")!=R_SUCCESS)
    {
      log_error("Parameter Description could not be added");
      return R_ERROR;
    }
    lparam = find_param_desc_by_name(laction, (char*)name);
  }

  curMechAction->params_to_modify[curMechAction->n_params_to_modify]=action_match_op_new(lparam,value,isXPath);
  check_null_int(curMechAction->params_to_modify[curMechAction->n_params_to_modify],"Could not add parameter instance to mechanism_actions");

  log_debug("Successfully added parameter_description \"%s\" with value \"%s\" to mechanism_actions",name,value);
  curMechAction->n_params_to_modify++;
  return R_SUCCESS;
}

unsigned int mechanism_actions_free(mechanism_actions_ptr mechaction)
{
  //unsigned int a;
  //for(a=0; a<mechaction->n_params_to_modify; a++)
  //  if(param_free(mechaction->params_to_modify[a])) return R_ERROR;
  //for(a=0; a<mechaction->n_actions_to_execute; a++)
  //  if(action_free(mechaction->actions_to_execute[a])) return R_ERROR;
  return R_SUCCESS;
}

unsigned long mechanism_actions_getMemSize(mechanism_actions_ptr mechaction)
{
  unsigned long size=sizeof(mechanism_actions_ptr);
  size+=4*sizeof(unsigned int);
  //unsigned int a;
  //for(a=0; a<mechaction->n_params_to_modify; a++)
  //  size+=param_getMemSize(mechaction->params_to_modify[a]);
  //size+=sizeof(param_ptr*);
  //for(a=0; a<mechaction->n_actions_to_execute; a++)
  //  size+=actions_getMemSize(mechaction->actions_to_execute[a]);
  size+=sizeof(action_ptr*);
  return size;
}

/// parse authorizationAction in preventiveMechanisms
mechanism_actions_ptr mechanism_actions_parse_xml(mechanism_ptr curMech, xmlNodePtr node)
{ // node = XMLnode <authorizationAction>
  check_null_ptr(node, "Failure parsing authorizationAction: NULL node!");
  xmlNodePtr cur=node->children;
  mechanism_actions_ptr curmechaction=NULL;

  while(cur!=NULL)
  {
    if(xmlIsElement(cur))
    {
      if(!xmlStrncasecmp(cur->name,"allow",5) || !xmlStrncasecmp(cur->name, "inhibit",7))
      { //PreventiveMechanismType
        unsigned int actiontype=(!xmlStrcmp(cur->name,"allow") ? ACTION_ALLOW : ACTION_INHIBIT);
        uint64_t delay=0;
        xmlNodePtr tmpNode=xmlFindNode(cur, "delay");
        if(tmpNode!=NULL)
        {
          delay=mechanism_parse_timestepSize(tmpNode);
          log_trace("Delay of AuthorizationAction: %llu",delay);
        }
        curmechaction=mechanism_actions_new(actiontype, delay);

        if(!xmlStrncasecmp(cur->name, "allow",5) && (tmpNode=xmlFindNode(cur, "modify"))!=NULL)
        {
          xmlNodePtr lcur=tmpNode->children;
          while(lcur!=NULL)
          {
            if(xmlIsElement(lcur) && !xmlStrncasecmp(lcur->name, "parameter",9))
            {
              mechanism_actions_addModifyParam(curMech, curmechaction, xmlGetProp(lcur,"name"), xmlGetProp(lcur,"value"),xmlGetProp(lcur,"isXPath"));
              log_trace("Successfully parsed modify parameter: param_desc=[%s], value=[%s], isXPath=[%d]",
                          curmechaction->params_to_modify[curmechaction->n_params_to_modify-1]->param_desc->name,
                          curmechaction->params_to_modify[curmechaction->n_params_to_modify-1]->value,
                          curmechaction->params_to_modify[curmechaction->n_params_to_modify-1]->is_xpath);
            }

            lcur=lcur->next;
          }
        }
        break;
      }
    }
    cur=cur->next;
  }
  log_trace("finished mechanism_action_parse_xml");
  return curmechaction;
}

unsigned int mechanism_execute_parse_xml(mechanism_ptr curMech, xmlNodePtr rootNode)
{ // node = XMLnode <action>
  check_null_int(curMech, "%s - curMech is NULL", __func__);
  check_null_int(rootNode, "%s - node is NULL", __func__);

  xmlNodePtr node=rootNode->children;
  while(node!=NULL)
  {
    if(xmlIsElement(node) && !xmlStrcmp(node->name, "action"))
    {
      mechanism_addAction(curMech, node, xmlGetProp(node, "name"));
      log_trace("Successfully parsed action[%s]", curMech->actions_to_execute[curMech->n_actions_to_execute-1]->action->action_name);
    }
    node=node->next;
  }

  return R_SUCCESS;
}

/******************************************************************************
 *  Mechanism implementation
******************************************************************************/
mechanism_ptr mechanism_new(unsigned char *name)
{
  mechanism_ptr nmech=mem_alloc(sizeof(mechanism_t));
  check_null_ptr(nmech, "Could not allocate memory for mechanism");
  nmech->mech_name=mem_calloc(strlen(name)+1, sizeof(unsigned char));
  check_null(nmech->mech_name, NULL, {free(nmech);}, "Could not allocate memory for mechanism name");
  strncpy(nmech->mech_name, name, strlen(name)+1);

  nmech->timestamp_start=timestamp_new();
  nmech->timestepSize=0;
  nmech->timestamp_lastUpdate=timestamp_new();
  nmech->timestep=0;
  nmech->trigger=NULL;
  nmech->condition=NULL;
  nmech->actions=NULL;
  nmech->n_actions_to_execute=0;
  nmech->actions_to_execute=NULL;
  log_trace("Successfully allocated mechanism \"%s\" (@ %llu)",nmech->mech_name,nmech->timestamp_start->usec);
  return nmech;
}

unsigned int mechanism_free(mechanism_ptr mech)
{
  mem_free(mech->mech_name);
  //if(mech->trigger!=NULL)
  //  if(event_free(mech->trigger)) return R_ERROR;
  if(formula_free(mech->condition)) return R_ERROR;
  if(mechanism_actions_free(mech->actions)) return R_ERROR;
  return R_SUCCESS;
}

/// @todo what does that mean: timestepSize = "1 timestep" (for evaluatation for example)???
uint64_t mechanism_parse_timestepSize(xmlNodePtr rootNode)
{
  if(rootNode==NULL) return 0;
  uint64_t interval=atoll(xmlGetProp(rootNode,"amount"));
  unsigned char *unit=xmlGetProp(rootNode, "unit");
  if(unit) interval*=usec_getTimeUnitMultiplier(unit);
  log_trace("%s - timestepSize: %llu us",__func__, interval);
  //xmlFree(unit);
  free(unit);
  return interval;
}

/// @todo improvement: statical set e.g. sizeof(uint64_t) or often used sizes in base.h
///       (or look if compiler statically sets these values in asm)
unsigned long mechanism_getMemSize(mechanism_ptr mech)
{
  unsigned long size=sizeof(mechanism_ptr);
  size+=(strlen(mech->mech_name)+1)*sizeof(unsigned char);
  size+=4*sizeof(uint64_t);   // usec_start, usec_last, usec_timestep, timestep
  //if(mech->trigger!=NULL)   size+=event_getMemSize(mech->trigger);
  if(mech->condition!=NULL) size+=formula_getMemSize(mech->condition);
  if(mech->actions!=NULL)   size+=mechanism_actions_getMemSize(mech->actions);
  return size;
}

// TODO: checking for existing mechanism in pdp_table should be done right after parsing its name or here after parsing the whole XML-file
mechanism_ptr mechanism_parse_xml(xmlNodePtr rootNode)
{
  log_trace("Mechanism Type: %s", rootNode->name);
  unsigned char *mechanismName=xmlGetProp(rootNode, "name");
  check_null_ptr(mechanismName,"Empty name for mechanism not allowed! Need a name (ID) for inserting into hash table.");
  mechanism_ptr nmech=mechanism_new(mechanismName);
  //xmlFree(mechanismName);
  free(mechanismName);
  nmech->timestepSize=mechanism_parse_timestepSize(xmlFindNode(rootNode, "timestep"));

  xmlNodePtr tmpNode=xmlFindNode(rootNode, "trigger");
  if(tmpNode!=NULL)
  {
    nmech->trigger=event_match_parse_xml(pdp_action_desc_store,tmpNode);
    // add mechanism to action entry action_description store (for later lookup for triggered mechanisms)
    nmech->trigger->match_action->mechanisms=g_slist_append(nmech->trigger->match_action->mechanisms, nmech);
  }
  else
  {
    if(!xmlStrncasecmp(rootNode->name,"preventiveMechanism",19))
    {
      log_error("NULL as trigger in preventiveMechanism NOT allowed!");
      // @todo free created stuff
      return NULL;
    }
    else nmech->trigger=NULL;
  }

  nmech->condition=formula_parse_xml(xmlFindNode(rootNode, "condition"));
  tmpNode=xmlFindNode(rootNode, "authorizationAction");
  if(tmpNode!=NULL) nmech->actions=mechanism_actions_parse_xml(nmech,tmpNode);

  mechanism_execute_parse_xml(nmech,rootNode);
  log_trace("Successfully parsed mechanism %s (size: %ld bytes)", nmech->mech_name, mechanism_getMemSize(nmech));
  mechanism_log(nmech);
  return nmech;
}

void mechanism_log(mechanism_ptr mech)
{
  log_trace("Mechanism: ");
  log_trace("         name: %s",   mech->mech_name);
  log_trace(" timestepSize: %llu", mech->timestepSize);
  log_trace("        start: %llu", mech->timestamp_start->usec);
  log_trace("  last update: %llu", mech->timestamp_lastUpdate->usec);
  log_trace(" cur timestep: %llu", mech->timestep);
}

xmlDocPtr mechanism_serialize_actions(mechanism_ptr mech)
{
  xmlDocPtr xmlDoc = NULL;
  xmlNodePtr responseNode = NULL, authorizationNode = NULL, authorizationResponseNode = NULL, actionNode = NULL, paramNode = NULL;
  xmlDoc = xmlNewDoc("1.0");
  responseNode = xmlNewNode(NULL, "notifyEventResponse");
  xmlDocSetRootElement(xmlDoc, responseNode);

  if(mech->actions!=NULL)
  {
    authorizationNode=xmlNewChild(responseNode, NULL, "authorizationAction", NULL);
    authorizationResponseNode=xmlNewChild(authorizationNode, NULL, (mech->actions->response==0 ? "inhibit" : "allow"), NULL);
    if(mech->actions->delay!=0)
    {
      char *amountStr = mem_alloc(20 * sizeof(char));
      snprintf(amountStr, 20, "%llu", mech->actions->delay);
      xmlNodePtr delayNode = xmlNewChild(authorizationResponseNode, NULL, "delay", NULL);
      xmlNewProp(delayNode, "amount", amountStr);
      xmlNewProp(delayNode, "unit", "MICROSECONDS");
      // @todo free amountStr?!
    }
    if(mech->actions->n_params_to_modify>0)
    {
      xmlNodePtr modifyNode = NULL;
      modifyNode=xmlNewChild(authorizationResponseNode, NULL, "modify", NULL);
      unsigned int b;
      for(b=0; b<mech->actions->n_params_to_modify; b++)
      {
        paramNode=xmlNewChild(modifyNode, NULL, "parameter", NULL);
        xmlNewProp(paramNode, "name", mech->actions->params_to_modify[b]->param_desc->name);
        xmlNewProp(paramNode, "value", mech->actions->params_to_modify[b]->value);
        xmlNewProp(paramNode, "isXPath", (mech->actions->params_to_modify[b]->is_xpath ? "true" : "false"));
      }
    }
  }

  unsigned int a;
  for(a=0; a<mech->n_actions_to_execute; a++)
  {
    actionNode=xmlNewChild(responseNode, NULL, "action", NULL);
    xmlNewProp(actionNode, "name", mech->actions_to_execute[a]->action->action_name);
    unsigned int b;
    for(b=0; b<mech->actions_to_execute[a]->n_params_to_action; b++)
    {
      paramNode=xmlNewChild(actionNode, NULL, "parameter", NULL);
      xmlNewProp(paramNode, "name", mech->actions_to_execute[a]->params[b]->param_desc->name);
      xmlNewProp(paramNode, "value", mech->actions_to_execute[a]->params[b]->value);
      xmlNewProp(paramNode, "isXPath", (mech->actions_to_execute[a]->params[b]->is_xpath ? "true" : "false"));
    }
  }
  return xmlDoc;
}
