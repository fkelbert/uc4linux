/*
 * hash_table.h
 *
 *      Authors: Carolina Lorini, Ricardo Neisse
 */

#ifndef _hash_table_h
#define _hash_table_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct entry_s {
  char* name;  			            /* unique monitor identifier */
  void *value;	            /* running monitor */
} entry_t ;

typedef struct table_s {
   entry_t** hash;   	            /* hash table */
   int size_table;   	            /* size of the hash table */
   int number_index; 	            /* number of elements currently on the table */
} table_t;

int init_table(table_t **table, int size);

int insert(table_t **table, entry_t* entry);

void delete_entry(table_t **table, char* name);

entry_t* lookup(table_t **table, char* name);

void free_table(table_t **table);

#endif
