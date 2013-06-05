/*
 * hash_table.c
 *
 *      Authors: Carolina Lorini, Ricardo Neisse
 */

#include "hash_table.h"

// Ullman pg 188 - Dragon Book
unsigned int hash_func(char *key, int key_len) {
  char* p;
  p = key;
  unsigned int h = 0;
  unsigned int g;
  int i;
  for(i = 0; i < key_len; i++) {
    h = ((h << 4) + (*p));
    if ((g = (h & 0xf0000000))) {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
    p++;
  }  
  return h;
}

entry_t **allocate_new_table(int size) {
  int n;
  entry_t** new_entry;
  
  // Declares and allocates a new hash table
  new_entry = (entry_t **)malloc(size*sizeof(entry_t*));
  if(new_entry == NULL)
    return new_entry;

  // Initializes all table entries
  for(n=0; n < size; n++)	
    new_entry[n] = NULL;

  return new_entry;
}

int init_table(table_t** a_table, int size) {
  table_t* table = *a_table;

  table = (table_t *)malloc(sizeof(struct table_s));
  if(table == NULL) {
    return 1;
  }

  // allocates new table
  table->hash = allocate_new_table(size);
  if(table->hash == NULL)
    return 1;

  // initializes table parameters
  table->size_table = size;
  table->number_index = 0;

  *a_table = table;
  return 0;
}

void free_table(table_t** a_table) {
  table_t* table = *a_table;
  int size_table;     
  if(table) {
    size_table = table->size_table;
    free(table->hash);
    free(table);
    table = NULL;
  }
  *a_table = table;
}

entry_t* lookup(table_t **a_table, char* name) {
  unsigned int hash_key, hash_key_2;
  table_t* table = *a_table;

  if(table->size_table == 0)
    return NULL;

  // Calculates the hash value
  hash_key = hash_func(name,strlen(name))%(table->size_table);
  hash_key_2 = hash_key;

  while(1){
    if( ((table->hash)[hash_key_2]) && (!strcmp(table->hash[hash_key_2]->name,name)) ) {
      // Entry found
      return table->hash[hash_key_2];			
    }
    
    // Searches for name in the rest of the table
    if(hash_key) {          				
      hash_key_2 = (hash_key_2+1)%table->size_table;
      if(hash_key_2 == hash_key - 1) {
        // Search has been through the whole table, name is not in table
        return NULL;					
      }
      
    } else {
      hash_key_2 = hash_key_2 + 1;
      if(hash_key_2 == table->size_table) {
        // Search has been through the whole table, name is not in table
        return NULL;					
      }
    }
    
  }
  *a_table = table;
  return NULL;
}

int insert_hash(table_t** a_table, entry_t *entry) {
  table_t* table = *a_table;
  unsigned int hash_key, hash_key_2;
  
  hash_key = hash_func(entry->name,strlen(entry->name))%(table->size_table);
  hash_key_2 = hash_key;

  // Searches for a free position on the table
  while(table->hash[hash_key_2] != NULL){
    
    if(!strcmp(table->hash[hash_key_2]->name,entry->name)) {
      // Entry already exists in the table
      return -1;				                
    }

    // Collision on position, searches for new position
    if(hash_key) {
      hash_key_2 = (hash_key_2+1)%table->size_table;
      if(hash_key_2 == hash_key-1) {
        // End of table reached
        return -1;				                
      }

    } else {
      hash_key_2 = hash_key_2 + 1;
      if(hash_key_2 == table->size_table) {
        // End of table reached
        return -1;				
      }                
    }
    
  }
  // Position found, inserts entry in table
  table->hash[hash_key_2] = entry;
  table->number_index++;

  *a_table = table;
  return 0;
}

int resize_table(table_t** a_table) {
  table_t *table = *a_table;
  table_t *new_table;
  int n;
  
  if(init_table(&new_table,table->size_table * 2)) {
    // Table creation failed
    return 1;							            
  }

  for(n=0; n < table->size_table; n++) {
    // Inserts elements from the old table into the new table
    if( table->hash[n] != NULL ){
      insert_hash(&new_table, table->hash[n]);
      table->hash[n] = NULL;
      if(--table->number_index == 0) {
        // Table is empty
        break;							 
      }           
    }
  }

  free_table(&table);
  *a_table = new_table;
  
  return 0;
}

int insert(table_t** a_table, entry_t* entry) {
  table_t *table = *a_table;
  // Resizes table if it is over 3/4 full 
  if(table->number_index >= ((table->size_table*3))/4){
    if(resize_table(&table)) {
      return 1;
    }
  }
  return insert_hash(&table,entry);
}

void delete_entry(table_t **a_table, char* name) {
  table_t *table = *a_table;
  unsigned int hash_key, hash_key_2;

  if(table->size_table == 0)
    return;

  // Calculates the hash value
  hash_key = hash_func(name,strlen(name))%(table->size_table);
  hash_key_2 = hash_key;

  while(1){
    if( ((table->hash)[hash_key_2]) && (!strcmp(table->hash[hash_key_2]->name,name)) ){
      free(table->hash[hash_key_2]->name);
      free(table->hash[hash_key_2]->value);
      free(table->hash[hash_key_2]);
      (table->hash)[hash_key_2] = NULL;
      table->size_table--;
      break;    
    }
    
    // Searches for name in the rest of the table
    if(hash_key){          				             
      hash_key_2 = (hash_key_2+1)%table->size_table;
      if(hash_key_2 == hash_key - 1) {
        // Search has been through the whole table, name is not in table
        break;					      
      }               
    } else {
      hash_key_2 = hash_key_2 + 1;
      if(hash_key_2 == table->size_table) {		
        // Search has been through the whole table, name is not in table
        break;					                     
      }
    }
  }
  *a_table = table;
}
