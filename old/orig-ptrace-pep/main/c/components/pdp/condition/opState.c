/**
 * @file  opState.c
 * @brief Implementation of operator states, i.e. the state of a operator node in the condition
 *
 * @see opState.h, state_s
 *
 * @author cornelius moucha
 **/

#include "opState.h"
#include "log_opState_pef.h"


/******************************************************************************
 *  Circular array implementation (bit-array)
******************************************************************************/
circArray_ptr circArrayNew(unsigned long size)
{
  circArray_ptr ncarray=memAlloc(sizeof(circArray_t));
  checkNullPtr(ncarray, "Could not allocate memory for circular array");

  ncarray->values=memCalloc((size+7)/8,sizeof(unsigned char));
  ncarray->p_first=0;
  //ncarray->p_next=0;
  // modification for before-operator -> to check for others...
  ncarray->p_next=size-1;
  ncarray->size=size;
  ncarray->type=cARRAY_BITARRAY;
  return ncarray;
}

unsigned int  circArrayFree(circArray_ptr array)
{ /// @todo check
  free(array->values);
  free(array);
  return R_SUCCESS;
}

bool circArrayGet(circArray_ptr array, unsigned long pos)
{
  return (*(array->values+pos/8) & (1<<(pos%8)))!=0;
}

void circArraySet(circArray_ptr array, unsigned long pos, bool val)
{
  if(val) *(array->values+pos/8) |=1<<(pos%8);    // setting to 1
  else    *(array->values+pos/8) &=~(1<<(pos%8)); // setting to 0
}

void circArrayLog(circArray_ptr array)
{
  char *buffer=(char*)memCalloc(array->size*2+1, sizeof(char));
  unsigned int a;
  for(a=0; a<array->size; a++)
  {
    buffer[2*a]=circArrayGet(array, a)==0 ? '0' : '1';
    buffer[2*a+1]=' ';
  }
  log_trace("circArray=[%s]", buffer);
  memFree(buffer);
}

char *circArrayString(circArray_ptr array)
{
  char *buffer=(char*)memCalloc(array->size*2+1, sizeof(char));
  unsigned int a;
  for(a=0; a<array->size; a++)
  {
    buffer[2*a]=circArrayGet(array, a)==0 ? '0' : '1';
    buffer[2*a+1]=' ';
  }
  return buffer;
}

/******************************************************************************
 *  Circular array implementation (int-array)
******************************************************************************/
circIntArray_ptr circIntArrayNew(unsigned long size)
{
  circIntArray_ptr ncarray=memAlloc(sizeof(circIntArray_t));
  checkNullPtr(ncarray, "Could not allocate memory for circular array");

  ncarray->values=memCalloc(size,sizeof(unsigned int));
  ncarray->p_first=0;
  ncarray->p_next=0;
  ncarray->size=size;
  ncarray->type=cARRAY_INTARRAY;
  return ncarray;
}

unsigned int  circIntArrayFree(circIntArray_ptr array)
{ /// @todo check
  free(array->values);
  free(array);
  return R_SUCCESS;
}

unsigned int circIntArrayGet(circIntArray_ptr array, unsigned long pos)
{
  return *(array->values+pos);
}

void circIntArraySet(circIntArray_ptr array, unsigned long pos, unsigned int val)
{
  *(array->values+pos)=val;
}

void circIntArrayLog(circIntArray_ptr array)
{
  unsigned int a;
  for(a=0; a<array->size; a++)
  {
    printf(" %d", circIntArrayGet(array, a));
  }
  printf("\n");
}

char *circIntArrayString(circIntArray_ptr array)
{
  char *buffer=(char*)memCalloc(array->size*2+1, sizeof(char));
  unsigned int a;
  for(a=0; a<array->size; a++)
  {
    buffer[2*a]=circIntArrayGet(array, a)==0 ? '0' : '1';
    buffer[2*a+1]=' ';
  }
  return buffer;
}


/******************************************************************************
 *  State implementation
******************************************************************************/
state_ptr stateNew(bool value)
{
  state_ptr nstate=memAlloc(sizeof(state_t));
  checkNullPtr(nstate, "Could not allocate memory for state");

  nstate->value=value;
  nstate->savedValues.bitArray=NULL;
  nstate->immutable=FALSE;
  nstate->subEverTrue=FALSE;
  nstate->counter=0;
  return nstate;
}

unsigned int stateFree(state_ptr state)
{ /// @todo check
  if(state->savedValues.bitArray!=NULL)
  {
    if(state->savedValues.intArray->type==cARRAY_INTARRAY)
      circIntArrayFree(state->savedValues.intArray);
    else circArrayFree(state->savedValues.bitArray);
  }
  free(state);
  return R_SUCCESS;
}

unsigned long stateMemSize(state_ptr state)
{
  unsigned long size=sizeof(state_ptr);
  size+=2*sizeof(bool);
  size+=sizeof(unsigned int);
  size+=sizeof(unsigned char);
  size+=sizeof(circArray_ptr);
  // circular array cannot be counted here, because size is not known; has to be done at operator calculation
  return size;
}

void stateLog(state_ptr state)
{ /// @todo stateLog not implemented
}
