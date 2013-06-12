#include <unistd.h>
#include <string.h>
#include "event_trace.h"

typedef struct param_s {
  char *name;
  char *value;
  unsigned char is_xpath:1; // bit type
} param_t;

typedef struct event_s {
  unsigned char is_try:1;
  unsigned char mode:1;
  char *name;
  unsigned char n_params;
  param_t *params;
} event_t;

typedef struct param_decl_s {
  char *name;
  int n_values;
  char *values;
} param_decl_t;

typedef struct event_decl_s {
  char *name;
  unsigned char n_params;
  param_decl_t *param_decl[];
} event_decl_t;

// For each parameterized event in the
// trace we keep all events that possibly match
// the declaration
//
typedef struct event_store_s {
  int timestep;
  event_decl_t *event_declaration;
  unsigned char *param_idx;
  unsigned char *value_idx;
  unsigned char is_try:1;
  unsigned char mode:1;
} event_store_t;

typedef struct event_trace_s {
  int block_size;
  event_decl_t *event_declarations;
  event_store_t *event_store;
} event_trace_t;

/*
 * function to check memory size of event including size of strings
 */

int main(int argc, char **argv) {
  event_t event;
  event.mode = 0;
  printf("Mode %d \n", event.mode);
  event.mode = 1;
  printf("Mode %d \n", event.mode);

  printf("Event size is %d \n", sizeof(event_t));
  printf("Event store size is %d \n", sizeof(event_store_t));
  printf("Page size: %d bytes \n", sysconf(_SC_PAGESIZE));
  printf("Page event store capacity is: %d events \n", sysconf(_SC_PAGESIZE) / sizeof(event_store_t));

  printf("Mode %x \n",  event);
  int i=0;
   printf("Struct size is %d \n", sizeof(param_t));
   printf("Struct size is %d \n", sizeof(event_t));
}
