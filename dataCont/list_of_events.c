/*
 * list_of_events.c
 *
 *  Created on: 19/lug/2010
 *      Author: lovat
 */

#include "list_of_events.h"

int syscall_to_int (char* syscall){
  if (syscall==NULL) return -1;
  int i=0;
  while (DFM_eventnames[i]!=NULL){
    if (strcmp(DFM_eventnames[i], syscall)==0) {
      return i;
    }
    i++;
  }
  return -1;  //not found
}
/*
int main (){
  return 0;
}
*/
