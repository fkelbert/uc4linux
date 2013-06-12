/*
 * dataCont.h
 *
 *  Created on: 28/giu/2010
 *      Author: lovat
 */

#ifndef DATACONT_H_
#define DATACONT_H_

/*
 * Max length of ID's (in chars, including '\0')
 */
#define MAX_DATA_ID 39
#define MAX_CONT_ID 39
#define MAX_LOOPS 10000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "base.h"

/*
 *	DATA type declaration
 *
 *
 */
struct data_s;
struct cont_s;
struct list_of_data_s;
struct list_of_cont_s;



typedef struct data_s data_t;
typedef data_t* data_ptr;

typedef struct cont_s cont_t;
typedef cont_t* cont_ptr;

typedef struct list_of_data_s list_of_data_t;
typedef list_of_data_t* list_of_data_ptr;


typedef struct list_of_cont_s list_of_cont_t;
typedef list_of_cont_t* list_of_cont_ptr;


struct data_s{
	char* data_ID;
};

data_ptr data_new(char* data_ID);
unsigned int data_free(data_ptr data);

data_ptr data_copy (data_ptr data);
unsigned int data_equals(data_ptr data1, data_ptr data2);


unsigned int how_many_copies(list_of_cont_ptr list_of_cont, char* data);


/*
 *	LIST OF DATA type declaration
 *
 *
 */

struct list_of_data_s{
	data_ptr data;
	list_of_data_ptr next;
};

list_of_data_ptr list_of_data_new();

unsigned int list_of_data_free(list_of_data_ptr list_of_data);
unsigned int list_of_data_length(list_of_data_ptr list_of_data);
unsigned int list_of_data_add(list_of_data_ptr* list_of_data, data_ptr data);
unsigned int list_of_data_remove(list_of_data_ptr* list_of_data, data_ptr data);
unsigned int list_of_data_find(list_of_data_ptr list_of_data, char* data); //boolean : is there data in list_of_data?

/*
 *	CONTAINER type declaration
 *
 *
 */

struct cont_s{
	char* cont_ID;
	list_of_data_ptr contained;
};

cont_ptr cont_new(char* cont_ID);
unsigned int cont_free(cont_ptr cont);

cont_ptr cont_copy (cont_ptr cont);
unsigned int cont_equals(cont_ptr cont1, cont_ptr cont2);

/*
 *	LIST OF CONTAINER type declaration
 *
 *
 */

struct list_of_cont_s{
	cont_ptr cont;
	list_of_cont_ptr next;
};

list_of_cont_ptr list_of_cont_new();

unsigned int list_of_cont_free(list_of_cont_ptr list_of_cont);
unsigned int list_of_cont_length(list_of_cont_ptr list_of_cont);
unsigned int list_of_cont_add(list_of_cont_ptr* list_of_cont, cont_ptr cont);
unsigned int list_of_cont_remove(list_of_cont_ptr* list_of_cont, cont_ptr cont);
cont_ptr list_of_cont_find(list_of_cont_ptr list_of_cont, char* cont);

/*
 *	NAMING and ALIAS type declaration
 *
 *
 */

typedef struct alias_s alias_t;
typedef alias_t *alias_ptr;

struct alias_s{
	char*	cont_ID;
	char** aliases;
	int size;
	alias_ptr next;
};

alias_ptr alias_new();
unsigned int alias_free(alias_ptr alias);

alias_ptr alias_copy (alias_ptr alias);
unsigned int alias_add(alias_ptr* alias, char* from, char* to);
unsigned int alias_remove(alias_ptr* alias, char* from, char* to);
char** alias_find(alias_ptr alias, char* cont, int* size);
char** alias_closure(alias_ptr alias, char* cont, int* size);
char* alias_list(alias_ptr alias, char* cont);



typedef struct naming_s naming_t;
typedef naming_t *naming_ptr;

struct naming_s
{
	char*	id;
	char*	pid;
	char*	cont_ID;
	naming_ptr next;
};


naming_ptr naming_new();
unsigned int naming_free(naming_ptr naming);
unsigned int naming_add(naming_ptr* naming, char* pid, char* id, char* cont);
unsigned int naming_remove(naming_ptr* naming, char* pid, char* id);
char* naming_find(naming_ptr naming, char* pid, char* id);




/*
 *	DATA-CONTAINER MAPPING type declaration
 *
 *
 */

typedef struct dataCont_s{
	char*	cont_ID_counter;
	char*	data_ID_counter;

	naming_ptr			naming;
	alias_ptr			alias;
	list_of_data_ptr	data;
	list_of_cont_ptr	cont;
	
}dataCont_t;

typedef dataCont_t *dataCont_ptr;

dataCont_ptr dataCont_new();

unsigned int dataCont_free(dataCont_ptr dataCont);


char* dataCont_getNewContID(dataCont_ptr dataCont);
char* dataCont_getNewDataID(dataCont_ptr dataCont);

unsigned int dataCont_newCont(dataCont_ptr dataCont, char* cont_ID);
unsigned int dataCont_newData(dataCont_ptr dataCont, char* data_ID);

unsigned int dataCont_updateContIDCounter(dataCont_ptr dataCont, char* cont_ID);
unsigned int dataCont_updateDataIDCounter(dataCont_ptr dataCont, char* data_ID);

unsigned int dataCont_addDataCont(dataCont_ptr dataCont, char* data, char* cont);
unsigned int dataCont_delDataCont(dataCont_ptr dataCont, char* data, char* cont);
unsigned int dataCont_addDataCont_noUpdate(dataCont_ptr dataCont, char* data, char* cont);
unsigned int dataCont_delDataCont_noUpdate(dataCont_ptr dataCont, char* data, char* cont);

unsigned int dataCont_removeCont(dataCont_ptr dataCont, char* cont);


unsigned int dataCont_emptyCont(dataCont_ptr dataCont, char* cont);

list_of_data_ptr dataCont_getDataIn(dataCont_ptr dataCont, char* cont_ID);
list_of_cont_ptr dataCont_getContOf(dataCont_ptr dataCont, char* data_ID);

unsigned int dataCont_addAlias(dataCont_ptr dataCont, char* from, char* to);
unsigned int dataCont_delAlias(dataCont_ptr dataCont, char* from, char* to);

unsigned int dataCont_delAllAliasesTo(dataCont_ptr dataCont, char* to);
unsigned int dataCont_delAllAliasesFrom(dataCont_ptr dataCont, char* from);

char** dataCont_getAliasesOf(dataCont_ptr dataCont, char* cont, int* size);
char** dataCont_getAliasesOf_noUpdate(dataCont_ptr dataCont, char* cont, int *size);
char** dataCont_getAliasesTo(dataCont_ptr dataCont, char* cont, int *size);


unsigned int dataCont_updateAlias(dataCont_ptr dataCont);

unsigned int dataCont_addNaming(dataCont_ptr dataCont, char* pid, char* id, char* cont);
unsigned int dataCont_delNaming(dataCont_ptr dataCont, char* pid, char* id);

unsigned int dataCont_delAllNamingFor(dataCont_ptr dataCont, char* cont);

char* dataCont_getNaming(dataCont_ptr dataCont, char* pid, char* id);
char** dataCont_getAllNamingsFrom(dataCont_ptr dataCont, char* pid, int* size);

unsigned int dataCont_addLodCont (dataCont_ptr dataCont, list_of_data_ptr lod, char*cont);
unsigned int dataCont_addLodCont_noUpdate (dataCont_ptr dataCont, list_of_data_ptr lod, char*cont);


/**
 *
 * PRINT
 *
 */


void dataCont_print(dataCont_ptr dataCont, int mode);
void list_of_data_print (list_of_data_ptr lod, int mode);
void list_of_cont_print (list_of_cont_ptr loc, int mode);
void alias_print (alias_ptr lod, int mode);
void naming_print (naming_ptr nam, int mode);
void list_of_data_printf (FILE *file, list_of_data_ptr lod, int mode);
void list_of_cont_printf (FILE *file, list_of_cont_ptr loc, int mode);
void alias_printf (FILE *file, alias_ptr lod, int mode);
void naming_printf (FILE *file, naming_ptr nam, int mode);







#endif /* DATACONT_H_ */
