/*
 * dataCont.c
 *
 *  Created on: 28/giu/2010
 *      Author: lovat
 */

#include "dataCont.h"



/*
 * This function returns a pointer to a data node with:
 * 		field dataID = data_ID
 * 		
 *
 */

data_ptr data_new(char* data_ID){
	int i;
	data_ptr dp = (data_ptr) malloc(sizeof(data_t));
	/*dp->data_ID = (char *) malloc (sizeof(char) * MAX_DATA_ID); //MAX_DATA_ID = size of the field (in char)
/*  for (i=0; i< MAX_DATA_ID; i++){
    dp->data_ID[i]=data_ID[i];
  }*/
	dp->data_ID = strdup(data_ID);


	return dp;
}


unsigned int data_free(data_ptr data){

//	printf ("data_free\n");
//	fflush(NULL);

	if (data!=NULL){
		if (data->data_ID!=NULL){
			free (data->data_ID);
		}
		free (data);
		return 0;
	}
	return 1;



}

unsigned int data_equals(data_ptr data1, data_ptr data2){  //by equals we mean "with the same ID"
	int i=0;
	if ((data1==NULL)&&(data2==NULL)) return 0;  //0 = equals
	if ((data1==NULL)||(data2==NULL)) return 2;  //2 = one of the two data pointer is NULL
	if ((data1->data_ID ==NULL)&&(data2->data_ID ==NULL)) return 0;  //0 = equals
	if ((data1->data_ID ==NULL)||(data2->data_ID ==NULL)) return 2;  //2 = one of the two data pointer is NULL

	//assumption: if the pointer is not NULL then data has been initialized (ID field not NULL)

/*	printf ("DATA1=%s, DATA2=%s\n",data1->data_ID,data2->data_ID);*/


/*
	while ((data1->data_ID[i]==data2->data_ID[i])&&(data1->data_ID[i]!='\0')&&(data2->data_ID[i]!='\0')) {
		i++;
	}

	if (data1->data_ID[i]==data2->data_ID[i]) {
		return 0;	  //0 = equals
	} else{
		return 1;  //1 = data IDs are different
	}
*/

	return (strcmp(data1->data_ID,data2->data_ID));

}


data_ptr data_copy (data_ptr data){
	int i;

	data_ptr dp = data_new(data->data_ID); //new data node

	return dp;
}


/*
 *
 */

cont_ptr cont_new(char* cont_ID){
	int i;
	cont_ptr cp = (cont_ptr) malloc(sizeof(cont_t));
	/*cp->cont_ID = (char* ) malloc (sizeof(char) * MAX_CONT_ID); //MAX_CONT_ID = size of the field (in char)
  /*for (i=0; i<MAX_CONT_ID; i++){
    cp->cont_ID[i]=cont_ID[i];
  };
	 */

	cp->cont_ID = strdup(cont_ID);
	cp->contained=NULL;

	return cp;
}



unsigned int cont_free(cont_ptr cont){
//	printf ("cont_free\n");
//	fflush(NULL);


	if (cont!=NULL){
		if (cont->cont_ID!=NULL){
			free (cont->cont_ID);
		}
		if (cont->contained!=NULL){
			list_of_data_free(cont->contained);
		}
		free (cont);
		return 0;
	}
	return 1;



}


cont_ptr cont_copy (cont_ptr cont){
	int i;
	list_of_data_ptr ptr_src, ptr_dst;

	cont_ptr cp = cont_new(cont->cont_ID); //new cont node

	if (list_of_data_length(cont->contained) > 0) {
		ptr_src = cont->contained;
		ptr_dst = list_of_data_new();
		cp->contained= ptr_dst;
		while (ptr_src->next != NULL) {
			ptr_dst->data=ptr_src->data;
			ptr_dst->next=list_of_data_new();
			ptr_dst=ptr_dst->next;
			ptr_src=ptr_src->next;
		}
		ptr_dst->data=ptr_src->data;
		ptr_dst->next=NULL; //redundant
	}

	return cp;
}


unsigned int cont_equals(cont_ptr cont1, cont_ptr cont2){  //by equals we mean "with the same ID". we don't look into the list of contained. 
	int i=0;
	if ((cont1==NULL)&&(cont2==NULL)) return 0;  //0 = equals
	if ((cont1==NULL)||(cont2==NULL)) return 2;  //2 = one of the two cont pointer is NULL
	//assumption: if the pointer is not NULL then cont has been initialized (ID field not NULL)
	while ((cont1->cont_ID[i]==cont2->cont_ID[i])&&(cont1->cont_ID[i]!='\0')&&(cont2->cont_ID[i]!='\0')) {
		i++;
	}

	if (cont1->cont_ID[i]==cont2->cont_ID[i]) {
		return 0;	  //0 = equals
	} else {
		return 1;  //1 = cont IDs are different
	}
}
/*
 *
 */

list_of_data_ptr list_of_data_new(){
	list_of_data_ptr lodp = (list_of_data_ptr) malloc (sizeof(list_of_data_t));
	lodp->data = NULL;
	lodp->next = NULL;
	return lodp;
}









unsigned int list_of_data_free(list_of_data_ptr list_of_data){
	list_of_data_ptr lod= NULL;
	list_of_data_ptr tmplod = NULL;

//	printf ("list_of_data_free\n");
//	fflush(NULL);

	if (list_of_data!=NULL){
		lod=list_of_data;
		while (lod!=NULL){
			if (lod->data!=NULL) data_free(lod->data);
			tmplod=lod;
			lod=lod->next;
			free (tmplod);
		}
//		free (list_of_data);
		return 0;
	}
	return 1;


}

unsigned int list_of_data_length(list_of_data_ptr list_of_data){
	unsigned int n;
	list_of_data_ptr ptr;

	n=0;
	ptr=list_of_data;

	if (ptr!=NULL) {
		if (ptr->data==NULL) return 0;
	}

	while (ptr!=NULL){
		ptr=ptr->next;
		n++;
	}

	return n;
}


unsigned int list_of_data_add(list_of_data_ptr* list_of_data, data_ptr data){
	list_of_data_ptr ptr = *list_of_data;

/*
	if (data!=NULL) {
		printf ("listofdataAdd DATA %s",data->data_ID);
	} else {
		printf ("listofdataAdd DATA NULL");
	}
*/

	if (ptr != NULL) { 					//if list is not empty, checks for duplicates
		if (data_equals(ptr->data,data)==0) { 		//check if data is the first element
			return 1;   		//1 = element is already present in the list
		}
		//else go through the list
		while (ptr->next!=NULL) {
			if (data_equals(ptr->data, data)==0) {
				return 1;   			//1 = element is already present in the list
			}
			ptr=ptr->next;
		}
		//		if (ptr->data!=NULL) printf("\n++++++++++++++++++ptr->data=%s, data=%s\n",ptr->data->data_ID,data->data_ID);
		if (data_equals(ptr->data, data)==0) {
			return 1;   			//1 = element is already present in the list
		}

	}
	//if this point is reached, then the element is not present
	//into the list or the list is empty
	if (*list_of_data==NULL){
		*list_of_data=list_of_data_new();
		(*list_of_data)->data=data;
	} else {
		if ((*list_of_data)->data==NULL){
			(*list_of_data)->data=data;
		}else{
			ptr->next=list_of_data_new();

			ptr=ptr->next;
			ptr->data=data;
		}
	}

	return 0; // OK, element added

}

unsigned int list_of_data_remove(list_of_data_ptr* list_of_data, data_ptr data){
	list_of_data_ptr ptr = *list_of_data;
	list_of_data_ptr tmp_ptr, prev;


	if (ptr==NULL) return 1;			//list is empty: element not found

	//if list is not empty


	if (data_equals(ptr->data,data)==0) {							//check if data is the first element

		*list_of_data=(*list_of_data)->next;	//skip the first element   ************* to check
		ptr->next=NULL;

		list_of_data_free(ptr);				//free the memory (1st element)

		return 0;					   		//0 = element is deleted, correct execution

	}


	//else go through the list
	while (ptr->next!=NULL) {
		if (data_equals((ptr->next)->data, data)==0) {
			tmp_ptr=(ptr->next)->next;

			ptr->next->next = NULL; //just in case, we remove the reference from the node to be removed to the next one, before freeing the memory

			list_of_data_free (ptr->next);

			ptr->next=tmp_ptr;

			return 0;   			//0 = element is deleted, correct execution
		}
		prev=ptr;
		ptr=ptr->next;
	}

	if (data_equals(ptr->data, data)==0) {
		prev->next=NULL;
		list_of_data_free (ptr);
		return 0;   			//0 = element is deleted, correct execution
	}

	return 1;   			//1 = element not found

}


unsigned int list_of_data_find(list_of_data_ptr list_of_data, char* data){
	list_of_data_ptr ptr = list_of_data;
	data_ptr tmp_ptr;

	tmp_ptr=data_new(data);

	/*
	 * NULLness checks for parameters
	 */

	if (ptr==NULL) {
		return 1;			//list is empty: element not found
	}

	if (data_equals(ptr->data,tmp_ptr)==0)	{
		data_free(tmp_ptr);
		return 0; // data is the first element
	}

	//else go through the list
	while (ptr!=NULL) {
		if (data_equals(ptr->data,tmp_ptr)==0) {
			data_free(tmp_ptr);
			return 0;
		}
		ptr=ptr->next;
	}

	return 1;   			//element not found
}



/*
 *
 */

list_of_cont_ptr list_of_cont_new(){
	list_of_cont_ptr locp= (list_of_cont_ptr) malloc (sizeof(list_of_cont_t));
	locp->cont = NULL;
	locp->next = NULL;
	return locp;
}

unsigned int list_of_cont_free(list_of_cont_ptr list_of_cont){
	list_of_cont_ptr loc= NULL;
	list_of_cont_ptr tmploc = NULL;

//	printf ("list_of_cont_free\n");
//	fflush(NULL);


	if (list_of_cont!=NULL){
		loc=list_of_cont;
		while (loc!=NULL){
			if (loc->cont!=NULL) cont_free(loc->cont);
			tmploc=loc;
			loc=loc->next;
			free (tmploc);
		}
//		free (list_of_cont);
		return 0;
	}

	return 1;
}

unsigned int list_of_cont_length(list_of_cont_ptr list_of_cont){
	unsigned int n;
	list_of_cont_ptr ptr;

	n=0;
	ptr=list_of_cont;

	if (ptr!=NULL){
		if (ptr->cont==NULL) return 0;
	}

	while (ptr!=NULL){
		ptr=ptr->next;
		n++;
	}
	return n;
}

unsigned int list_of_cont_add (list_of_cont_ptr *list_of_cont, cont_ptr cont){
	list_of_cont_ptr ptr = *list_of_cont;
	list_of_cont_ptr prevptr = ptr;



	if (ptr != NULL) { 	//if list is not empty
		prevptr=ptr;
		if (cont_equals(ptr->cont,cont)==0) { 		//check if cont is the first element

			return 1;   		//1 = element is already present in the list
		}
		//else go through the list
		while (ptr!=NULL) {
			if (cont_equals(ptr->cont,cont)==0) {
				return 1;   			//1 = element is already present in the list
			}
			prevptr=ptr;
			ptr=ptr->next;
		}
	}
	//if this point is reached, then the element is not present
	//into the list or the list is empty

	ptr=prevptr; //ptr points to the last element of the list

	if (*list_of_cont==NULL){
		//		printf("list of cont=NULL\n");
		(*list_of_cont)=list_of_cont_new();
		(*list_of_cont)->cont=cont;
	} else {
		if ((*list_of_cont)->cont==NULL){

			(*list_of_cont)->cont=cont;
		}else{


			ptr->next=list_of_cont_new();
			ptr=ptr->next;
			ptr->cont=cont;
			ptr->next=NULL; //redundant
		}
	}

	return 0; // OK, element added

}

unsigned int list_of_cont_remove(list_of_cont_ptr* list_of_cont, cont_ptr cont){
	list_of_cont_ptr ptr = *list_of_cont;
	list_of_cont_ptr tmp_ptr;

	if (ptr==NULL) return 1;			//list is empty: element not found
	//if list is not empty
	if (cont_equals(ptr->cont,cont)==0) {							//check if cont is the first element

		(*list_of_cont)=(*list_of_cont)->next;	//skip the first element
		ptr->next=NULL;

		list_of_cont_free(ptr);				//free the memory

		return 0;					   		//0 = element is deleted, correct execution

	}

	//else go through the list

	while (ptr->next!=NULL) {
		if ((ptr->next)->cont == cont) {
			tmp_ptr=(ptr->next)->next;
			ptr->next->next = NULL;

			list_of_cont_free (ptr->next);
			ptr->next=tmp_ptr;
			return 0;   			//0 = element is deleted, correct execution
		}
		ptr=ptr->next;
	}

	if (ptr->cont == cont) {
		list_of_cont_free (ptr);
		return 0;         //0 = element is deleted, correct execution
	}
	return 1;   			//1 = element not found

}



cont_ptr list_of_cont_find(list_of_cont_ptr list_of_cont, char* cont){
	int i=0;
	list_of_cont_ptr ptr = list_of_cont;
	cont_ptr tmp_ptr;

	tmp_ptr=cont_new(cont);


	/*
	 * NULLness checks for parameters
	 */


	if (ptr==NULL) return NULL;			//list is empty: element not found

	if (cont_equals(ptr->cont,tmp_ptr)==0)	{
		cont_free(tmp_ptr);
		return ptr->cont; //check if cont is the first element
	}

	//else go through the list



	while (ptr->next!=NULL) {
		if (cont_equals((ptr->next)->cont,tmp_ptr)==0) {
			cont_free(tmp_ptr);
			return ptr->next->cont; //check if cont is the first element
		}

		ptr=ptr->next;
	}



	return NULL;   			//1 = element not found
}










dataCont_ptr dataCont_new(){
	int i;
	dataCont_ptr dcp= (dataCont_ptr) malloc (sizeof(dataCont_t));

/*
	dcp->data_ID_counter = (char*) malloc (sizeof(char) * (MAX_DATA_ID));

	//srand(time(NULL));

	//initialization of ID counters
	for (i=1; i<(MAX_DATA_ID - 1); i++){
		//dcp->data_ID_counter[i]=(char)('0'+(int)(rand()%10));
		dcp->data_ID_counter[i]='0';

	}
	dcp->data_ID_counter[0]='D';
	dcp->data_ID_counter[(MAX_DATA_ID - 1)]='\0';
*/
/*
	dcp->cont_ID_counter = (char*) malloc (sizeof(char) * (MAX_CONT_ID));
	for (i=1; i< (MAX_CONT_ID-1); i++){
		//dcp->cont_ID_counter[i]=(char)('0'+(int)(rand()%10));
		dcp->cont_ID_counter[i]='0';
	}
	dcp->cont_ID_counter[0]='C';
	dcp->cont_ID_counter[(MAX_CONT_ID - 1)]='\0';
*/
	dcp->cont_ID_counter = 0;
	dcp->data_ID_counter = 0;


	//creation of the two empty lists
	dcp->cont = list_of_cont_new();
	dcp->data = list_of_data_new();
	dcp->alias = NULL;


	char ip_addr_list[2048];

	dcp->ip = calloc(9, sizeof(char));
	if (xget_host_ip(dcp->ip,xget_all_ip_addresses(ip_addr_list,2048,0)) == -1) {
		memset(dcp->ip,'0',9);
	}

	return dcp;
}

unsigned int dataCont_free(dataCont_ptr dataCont){

	if (dataCont==NULL) return 1;
	if (dataCont->alias!=NULL) alias_free(dataCont->alias);
	if (dataCont->cont!=NULL) list_of_cont_free(dataCont->cont);
	if (dataCont->data!=NULL) list_of_data_free(dataCont->data);
	if (dataCont->naming!=NULL) naming_free(dataCont->naming);

	//if (dataCont->cont_ID_counter!=NULL) free(dataCont->cont_ID_counter);
	//if (dataCont->data_ID_counter!=NULL) free(dataCont->data_ID_counter);

	free(dataCont);

	return 0;
}


/*
 *
 */

alias_ptr alias_new(){
	alias_ptr res = (alias_ptr) malloc (sizeof(alias_t));
	res->size=0;
	res->cont_ID=NULL;
	res->aliases=NULL;
	res->next=NULL;
	return res;
}

unsigned int alias_free(alias_ptr alias){
	int s=0;
	alias_ptr al=NULL;
	alias_ptr tmpal=NULL;

	if (alias==NULL) return 1;

	al=alias;
	while (al!=NULL){
		if (al->aliases!=NULL){
			for (s=0; s<al->size; s++){
				free (al->aliases[s]);
			}
			free(al->aliases);
		}
		tmpal=al;
		al=al->next;
		free(tmpal);
	}

//	free (alias);

	return 0;
}

alias_ptr alias_copy (alias_ptr alias){
	alias_ptr tmp_source = alias;
	alias_ptr res = alias_new();
	alias_ptr tmp_dest = res;

	int x=0;

	while (tmp_source!=NULL){
		tmp_dest->size=tmp_source->size;
		if (tmp_source->cont_ID !=NULL){
			tmp_dest->cont_ID=(char*) malloc (strlen(tmp_source->cont_ID));
			strcpy (tmp_dest->cont_ID,tmp_source->cont_ID);
		}
		tmp_dest->aliases= (char**) malloc (sizeof(char*)*(tmp_source->size));
		for (x=0; x<tmp_source->size; x++){
			tmp_dest->aliases[x]=(char*)malloc (sizeof(char)*(strlen(tmp_source->aliases[x])));
			strcpy(tmp_dest->aliases[x],tmp_source->aliases[x]);
		}
		if (tmp_source->next!=NULL) tmp_dest->next=alias_new();
		tmp_source=tmp_source->next;
		tmp_dest=tmp_dest->next;
	}
	return res;
}


unsigned int alias_add(alias_ptr* alias, char* from, char* to){
	alias_ptr tmp = *alias;
	char** tmpptr = NULL;
	int x=0;



	if ((strcmp(from,to)==0)||(from==NULL)||(to==NULL)) return 1; //self-pointing alias is useless and NULL values are wrong


	if ((*alias)==NULL) { //emty list -> insert the new value at the beginning

		(*alias)=alias_new();
		(*alias)->size=1;
		(*alias)->cont_ID=(char*) malloc (sizeof(char)*strlen(from));
		strcpy((*alias)->cont_ID,from);
		(*alias)->aliases=(char**) malloc (sizeof(char*));
		(*alias)->aliases[0]= (char*) malloc (sizeof(char)*strlen(to));
		strcpy((*alias)->aliases[0],to);
		return 0;
	} else {

		while ((tmp->next!=NULL)&&(strcmp(tmp->cont_ID,from)!=0)) {
			tmp=tmp->next;
		}


		if ((tmp->next==NULL) && (strcmp(tmp->cont_ID,from)!=0)) { //no matchings for "FROM" -> add a new node at the end


			tmp->next=alias_new();
			(tmp->next)->size=1;
			(tmp->next)->cont_ID=(char*) malloc (sizeof(char)*strlen(from));
			strcpy((tmp->next)->cont_ID,from);
			(tmp->next)->aliases=(char**) malloc (sizeof(char*));
			(tmp->next)->aliases[0]= (char*) malloc (sizeof(char)*strlen(to));
			strcpy((tmp->next)->aliases[0],to);
			return 0;
		} else { //there's a mathing for "FROM" and it's pointed by tmp
			//let's look for "TO"


			x=0;
			while (x<(tmp->size)){
				if (strcmp(tmp->aliases[x],to)==0) return 1; //alias already exists
				x++;
			}

			//else we add "TO" at the end of the aliases
			(tmp->size)++;
			tmpptr =(char**) realloc (tmp->aliases, tmp->size * sizeof(char*));

			if (tmpptr==NULL) {
				printf("ERROR during memory allocation!\n");
				fflush(NULL);
				exit(0);
			}
			tmp->aliases=tmpptr;
			tmp->aliases[(tmp->size)-1]=(char*)malloc(sizeof(char)*strlen(to));
			strcpy(tmp->aliases[(tmp->size)-1],to);
			return 0;
		}


	}

}

unsigned int alias_remove(alias_ptr* alias, char* from, char* to){
	alias_ptr tmp = *alias;
	alias_ptr tmpprev;
	char** tmpptr = NULL;
	int x=0;
	int y=0;



	if ((from==NULL)||(to==NULL)) return 1; //self-pointing alias is not accepted and NULL values are wrong
	if (strcmp(from,to)==0) return 1;

	if (*alias==NULL) return 1; //aliaslist is empty

	if (strcmp(from,(*alias)->cont_ID)==0) { //The "FROM" is the first element
		if ((*alias)->size==0) return 1; //no aliases for "FROM"
		x=0;
		while (x<((*alias)->size)){
			if (strcmp((*alias)->aliases[x],to)==0){//alias already exists
				//then I have to copy all the other aliases
				free ((*alias)->aliases[x]);
				for (y=x; y<(*alias)->size; y++){
					if (y==((*alias)->size)-1){
						tmp->aliases[y]=NULL;
					} else {
						tmp->aliases[y]=tmp->aliases[y+1];
					}
				}
				tmp->size--;
				if (tmp->size==0){
					(*alias)=(*alias)->next;
				}
				return 0;
			}
			x++;
		}
		return 1; //there exist no aliases from "FROM" to "TO


	} else { //the "FROM" is not the first of the list
		tmpprev=NULL;
		while ((tmp->next!=NULL)&&(strcmp(tmp->cont_ID,from)!=0)) {
			tmpprev=tmp;
			tmp=tmp->next;
		}


		if (strcmp(tmp->cont_ID,from)==0) {

			if (tmp->size==0) return 1; //no aliases for "FROM"
			x=0;
			while (x<(tmp->size)){
				if (strcmp(tmp->aliases[x],to)==0){//alias already exists
					//then I have to copy all the other aliases
					free (tmp->aliases[x]);
					for (y=x; y<tmp->size; y++){
						if (y==(tmp->size)-1){
							tmp->aliases[y]=NULL;
						} else {
							tmp->aliases[y]=tmp->aliases[y+1];
						}
					}
					tmp->size--;
					if (tmp->size==0){
						tmpprev->next=tmp->next;
					}

					return 0;
				}
				x++;
			}
			return 1; //there exist no aliases from "FROM" to "TO



		} else { //there's no matching for "FROM"
			return 1;
		}
	}

}

char** alias_find(alias_ptr alias, char* cont, int* size){
	alias_ptr tmp=alias;
	char **res=NULL;
	int x=0;
	*size=0;
	if ((alias==NULL)||(cont==NULL)) return NULL;
	while (tmp!=NULL) {
		if (tmp->cont_ID!=NULL){
			if (strcmp(tmp->cont_ID,cont)==0){
				*size=tmp->size;

				if (tmp->size >0) res=(char**)malloc(sizeof(char*)*tmp->size);
				for (x=0; x<tmp->size; x++)res[x]=strdup(tmp->aliases[x]);
				return res;
			}
		}
		tmp=tmp->next;
	}

	return NULL;
}



char** alias_closure(alias_ptr alias, char* cont, int* size){
	char ** tmp;
	alias_ptr* result;
	alias_ptr tmpdest;
	alias_ptr tmpsource = alias;
	int x=0;
	int y=0;
	int tmpsize=0;
	int count=0;
	int changed=1;

	result=&tmpdest;
	if (alias==NULL) return NULL;
	tmpdest=NULL;
	while (tmpsource!=NULL){
		for (x=0; x< tmpsource->size; x++){
			alias_add(&(tmpdest),tmpsource->cont_ID, tmpsource->aliases[x]);
		}
		tmpsource=tmpsource->next;
	}



	while ((count<(pow(10,CONT_ID_LEN)))&&(changed!=0)){
		changed=0;
		tmpsource=alias;
		while (tmpsource!=NULL){
			for (x=0; x< tmpsource->size; x++){
				tmp=alias_find(tmpdest, tmpsource->aliases[x], &tmpsize);
				if (tmp!=NULL){
					for (y=0; y< tmpsize; y++){
						changed+=(1- alias_add(&(tmpdest), tmpsource->cont_ID, tmp[y]));
					}
				}
			}
			tmpsource=tmpsource->next;
			count++;
		}
	}

	return alias_find(tmpdest,cont,size);

}


char* alias_list(alias_ptr alias, char* cont){
	char ** tmp;
	char * result, tmpresult;
	int x,size=0;

	if (alias==NULL) return NULL;

	tmp=alias_find(alias,cont,&size);

	if (tmp==NULL) return NULL;

	for (x=0; x<size; x++){
		tmpresult=(char*) realloc (result, (strlen(result)+strlen(tmp[x]))*sizeof(char));
		if (tmpresult==NULL) {
			printf("Error: Memory Allocation\n");
			fflush(NULL);
			return NULL;
		} else {
			result=tmpresult;
		}
	}

	return result;

}


/***
 *
 * NAMING
 *
 */

naming_ptr naming_new(){
	naming_ptr res = (naming_ptr) malloc (sizeof(naming_t));
	res->id=NULL;
	res->pid=NULL;
	res->cont_ID=NULL;
	res->next=NULL;
	return res;
}



unsigned int naming_free(naming_ptr naming){
	naming_ptr nam=NULL;
	naming_ptr tmpnam=NULL;

	if (naming==NULL) return 1;

	nam=naming;
	while (nam!=NULL){
		if (nam->cont_ID!=NULL) free (nam->cont_ID);
		if (nam->id!=NULL) free (nam->id);
		if (nam->pid!=NULL) free (nam->pid);

		tmpnam=nam;
		nam=nam->next;
		free(tmpnam);
	}

//	free (naming);

	return 0;

}

unsigned int naming_add(naming_ptr* naming, char* pid, char* id, char* cont){
	naming_ptr tmp = *naming;
	if ((id==NULL)||(pid==NULL)||(cont==NULL)) return 2; //self-pointing alias is useless and NULL values are wrong

	if ((*naming)==NULL) { //emty list -> insert the new value at the beginning

		(*naming)=naming_new();
		tmp=*naming;
		tmp->cont_ID=(char*) malloc (sizeof(char)*strlen(cont));
		strcpy(tmp->cont_ID,cont);
		tmp->id=(char*) malloc (sizeof(char)*strlen(id));
		strcpy(tmp->id,id);
		tmp->pid=(char*) malloc (sizeof(char)*strlen(pid));
		strcpy(tmp->pid,pid);
		return 0;

	} else {

		while ((tmp->next!=NULL)&&((strcmp(tmp->pid,pid)!=0)||(strcmp(tmp->id,id)!=0))) {
			tmp=tmp->next;
		}

		if ((tmp->next==NULL) && ((strcmp(tmp->pid,pid)!=0)||(strcmp(tmp->id,id)!=0))) { //no matchings for "PID && ID" -> add a new node at the end
			tmp->next=naming_new();
			tmp=tmp->next;
			tmp->cont_ID=(char*) malloc (sizeof(char)*strlen(cont));
			strcpy(tmp->cont_ID,cont);
			tmp->id=(char*) malloc (sizeof(char)*strlen(id));
			strcpy(tmp->id,id);
			tmp->pid=(char*) malloc (sizeof(char)*strlen(pid));
			strcpy(tmp->pid,pid);
			return 0;

		} else { //there's a matching for "pid && id" -> overwrite the container ID

			tmp->cont_ID=(char*) malloc (sizeof(char)*strlen(cont));
			strcpy(tmp->cont_ID,cont);

			return 1;
		}


	}


}

unsigned int naming_remove(naming_ptr* naming, char* pid, char* id){
	naming_ptr tmp = *naming;
	naming_ptr tmpptr;
	char* asterisk="*";
	int star=((strcmp(id,asterisk)==0)?1:0);

	if ((id==NULL)||(pid==NULL)||(*naming==NULL)) return 2; //self-pointing alias is useless and NULL values are wrong


	// printf("*naming=%x  tmp->pid=%s tmp->id=%s   pid=%s  id=%s star=%d start=",naming, tmp->pid, tmp->id, pid,id,star);


	if ((strcmp(tmp->pid,pid)==0)&&((strcmp(tmp->id,id)==0)||star)) { //first element
		//if ((strcmp(tmp->pid,pid)==0)&&(strcmp(tmp->id,id)==0)) { //first element



		(*naming)=(*naming)->next;
		free(tmp->cont_ID);
		free(tmp->id);
		free(tmp->pid);
		return 0;

	} else {

		while ((tmp->next!=NULL)&&((strcmp(tmp->pid,pid)!=0)||((strcmp(tmp->id,id)!=0)&&(1-star)))) {
			//while ((tmp->next!=NULL)&&((strcmp(tmp->pid,pid)!=0)||(strcmp(tmp->id,id)!=0))) {
			tmpptr=tmp; //keep track of the previous node
			tmp=tmp->next;
		}
		if ((strcmp(tmp->pid,pid)==0)&&((strcmp(tmp->id,id)==0)||star)) { //current element (eventually the last one) matches pid && id
			//    if ((strcmp(tmp->pid,pid)==0)&&(strcmp(tmp->id,id)==0)) { //current element (eventually the last one) matches pid && id

			tmpptr->next=tmp->next; //skip the current node
			free(tmp->cont_ID);
			free(tmp->id);
			free(tmp->pid);
			return 0;

		} else { //there's no matching for "pid && id"

			return 1;
		}

	}


}

char* naming_find(naming_ptr naming, char* pid, char* id){
	naming_ptr tmp = naming;

	if ((id==NULL)||(pid==NULL)||(tmp==NULL)) return NULL; //self-pointing alias is useless and NULL values are wrong

	while ((tmp->next!=NULL)&&((strcmp(tmp->pid,pid)!=0)||(strcmp(tmp->id,id)!=0))) {
		tmp=tmp->next;
	}

	if ((strcmp(tmp->pid,pid)==0)&&(strcmp(tmp->id,id)==0)) { //current element (eventually the last one) matches pid && id
		return tmp->cont_ID;
	} else { //there's no matching for "pid && id"
		return NULL;
	}

}




char* dataCont_getNewContIDType(dataCont_ptr dataCont, char *type) {
	char *id = dataCont_getNewContID(dataCont);
	strncpy(id+11,type,CONT_TYPE_LENGTH);
	return id;
}


char* dataCont_getNewContIDSocket(dataCont_ptr dataCont) {
	return dataCont_getNewContIDType(dataCont, CONT_TYPE_SOCK);
}

char* dataCont_getNewContIDPipe(dataCont_ptr dataCont) {
	return dataCont_getNewContIDType(dataCont, CONT_TYPE_PIPE);
}


char* dataCont_getNewContIDFile(dataCont_ptr dataCont) {
	return dataCont_getNewContIDType(dataCont, CONT_TYPE_FILE);
}



char* dataCont_getNewContID(dataCont_ptr dataCont) {
	char* id = (char *) calloc (CONT_ID_LEN, sizeof(char));

	snprintf(id,CONT_ID_LEN,"%.*s%.*s-%.*s-%08d",2,CONT_PREFIX,8,dataCont->ip,4,CONT_TYPE_UNKNOWN,dataCont->cont_ID_counter);
	dataCont->cont_ID_counter++;

	return id;
}

char* dataCont_getNewDataID(dataCont_ptr dataCont){
	char* id = (char *) calloc (DATA_ID_LEN, sizeof(char));

	snprintf(id,DATA_ID_LEN,"D%013d",dataCont->data_ID_counter);
	dataCont->data_ID_counter++;

	return id;
}



unsigned int dataCont_updateContIDCounter(dataCont_ptr dataCont, char* cont_ID){

	/**TODO**/
	/*
	 * TO BE IMPLEMENTED
	 */

	/*
	if (dataCont->cont_ID_counter[MAX_CONT_ID - 2]<cont_ID[MAX_CONT_ID - 2]){
		dataCont->cont_ID_counter[MAX_CONT_ID - 2]=cont_ID[MAX_CONT_ID - 2];
	}

	 */
	return 0;

}

unsigned int dataCont_updateDataIDCounter(dataCont_ptr dataCont, char* data_ID){
	/**TODO**/
	/*
	 * TO BE IMPLEMENTED


	if (dataCont->data_ID_counter[MAX_DATA_ID - 2]<data_ID[MAX_DATA_ID - 2]){
		dataCont->data_ID_counter[MAX_DATA_ID - 2]=data_ID[MAX_DATA_ID - 2];
	}

	 */
	return 0;

}


unsigned int dataCont_newCont(dataCont_ptr dataCont, char* cont_ID){
	cont_ptr cp;

	if (cont_ID!=NULL){
		cp = cont_new(cont_ID);
		dataCont_updateContIDCounter(dataCont, cont_ID);

		list_of_cont_add (&(dataCont->cont), cp);

		return 0; 					//OK
	} else return 1;				//cont_ID == NULL

}


unsigned int dataCont_newData(dataCont_ptr dataCont, char* data_ID){
	data_ptr dp;
	if (data_ID!=NULL){
		dp = data_new(data_ID);
		dataCont_updateDataIDCounter(dataCont, data_ID);

		list_of_data_add (&(dataCont->data), dp);
		return 0; 					//OK
	} else return 1;				//data_ID == NULL
}


unsigned int how_many_copies(list_of_cont_ptr list_of_cont, char* data){
	list_of_cont_ptr tmp_ptr=list_of_cont;
	list_of_data_ptr lod;
	data_ptr d=data_new(data);

	unsigned int result=0;



	while (tmp_ptr!=NULL){
		lod=tmp_ptr->cont->contained;
		while (lod!=NULL){
			if (data_equals(lod->data,d)==0) result++;
			lod=lod->next;
		}
		tmp_ptr=tmp_ptr->next;
	}

	return result;

}



unsigned int dataCont_addDataCont(dataCont_ptr dataCont, char* data, char* cont){
	dataCont_addDataCont_noUpdate(dataCont, data, cont);
	dataCont_updateAlias(dataCont);
}

unsigned int dataCont_addDataCont_noUpdate(dataCont_ptr dataCont, char* data, char* cont){
	/*
	 * checks for NULLness of parameters
	 *
	 */




	unsigned int result=0;
	data_ptr d = NULL;
	cont_ptr c = NULL;

	//	printf("///%d",list_of_data_find(dataCont->data, data));
	if (list_of_data_find(dataCont->data, data)!=0){
/*		printf("listofdatafind!=0 - ");*/

		dataCont_newData(dataCont, data);
	} else {
/*		printf("listofdatafind ==0;\n");*/

	}
	d = data_new(data);



	c = list_of_cont_find(dataCont->cont, cont);	// retrieve the pointer to the element (if  present)


	if (c==NULL){
		result =1;
		dataCont_newCont(dataCont, cont);
		c = list_of_cont_find(dataCont->cont, cont);	// retrieve the pointer to the element (if  present)
	}


	if (c->contained==NULL)	{
		c->contained = list_of_data_new();
	}


	result = result + list_of_data_add(&(c->contained), d);


	//	printf("add %s to %s result=%d\n",data, cont, result);

	if (result != 0) result =1; //1 error
	return result;

}

unsigned int dataCont_removeCont(dataCont_ptr dataCont, char* cont) {
	cont_ptr tmpcont;

	//only if empty

	if ((dataCont==NULL)||(cont==NULL)) return 1;
	if (dataCont->cont==NULL) return 1;

	tmpcont=list_of_cont_find(dataCont->cont,cont);

	if (tmpcont==NULL) return 1; //element is not in the list

	return list_of_cont_remove(&(dataCont->cont),tmpcont);

}

unsigned int dataCont_delDataCont(dataCont_ptr dataCont, char* data, char* cont) {
	char **list;
	int size,x;
	int res=0;
	int tmpres=0;

	if ((dataCont==NULL)||(data==NULL)||(cont==NULL)) return 1;
	list=dataCont_getAliasesOf(dataCont, cont, &size);
	for (x=0;x<size;x++){
		tmpres=dataCont_delDataCont_noUpdate(dataCont, data, list[x]);
		res=res||tmpres;
	}
	tmpres=dataCont_delDataCont_noUpdate(dataCont, data, cont);
	res=res||tmpres;

	return res;

}


unsigned int dataCont_delDataCont_noUpdate(dataCont_ptr dataCont, char* data, char* cont) {
	/*
	 * checks for NULLness of parameters
	 *
	 */

	unsigned int result=0;

	data_ptr d=NULL;
	if (list_of_data_find(dataCont->data, data)==0){
		d=data_new(data);
	} else {
		return 1;
	}


	cont_ptr c=list_of_cont_find(dataCont->cont, cont);
	if (c==NULL) return 1;   //1 = one of the elements does not exists


	result = result + list_of_data_remove(&(c->contained), d);

	/*
	 *
	 * if data has no other references, than data has to be deleted
	 *
	 */
	if (how_many_copies(dataCont->cont,data)==0) {
		list_of_data_remove (&(dataCont->data), d);
		data_free(d);
	}

	if (result != 0) result =1; //1 error
	return result;

}

unsigned int dataCont_emptyCont(dataCont_ptr dataCont, char* cont){
	list_of_data_ptr lod,tmp;
	unsigned int res;
	unsigned int tmpres;
	if ((dataCont==NULL)||(cont==NULL)) return 1;

	lod=dataCont_getDataIn(dataCont, cont);
	tmp=lod;
	res=(tmp==NULL);

	while (tmp!=NULL){
		tmpres=dataCont_delDataCont_noUpdate(dataCont, tmp->data->data_ID, cont);
		res=res||tmpres;
		tmp=tmp->next;
	}

	return res;

}



unsigned int dataCont_addAlias(dataCont_ptr dataCont, char* from, char* to){
	if ((dataCont==NULL)||(from==NULL)||(to==NULL)) return 1; //error
	dataCont_newCont(dataCont, from);
	dataCont_newCont(dataCont, to);
	return alias_add(&(dataCont->alias), from, to);
}

unsigned int dataCont_delAlias(dataCont_ptr dataCont, char* from, char* to){
	if (dataCont==NULL) return 1; //error
	if (dataCont->alias==NULL) return 1; //error
	return alias_remove(&(dataCont->alias), from, to);
}


unsigned int dataCont_delAllAliasesTo(dataCont_ptr dataCont, char* to){
	alias_ptr tmp;
	char* tmpfrom=NULL;
	char* from=NULL;
	unsigned int res=0;
	unsigned int tmpres=0;
	if (dataCont==NULL) return 1; //error
	if (dataCont->alias==NULL) return 1; //error
	tmp=dataCont->alias;
	while (tmp!=NULL){
		if (tmp->cont_ID!=NULL){
			tmpfrom=(char*)realloc(from,sizeof(char)*strlen(tmp->cont_ID));
			from=tmpfrom;
			strcpy(from,tmp->cont_ID);
			tmpres=(alias_remove(&(dataCont->alias), from, to));
			res=res||tmpres;
		}
		tmp=tmp->next;
	}

	return res;
}


unsigned int dataCont_delAllAliasesFrom(dataCont_ptr dataCont, char* from){
	alias_ptr tmp;
	int size,x;
	char **list;
	char* to=NULL;
	unsigned int res=0;


	if (dataCont==NULL) return 1; //error
	if (dataCont->alias==NULL) return 1; //error



	list=alias_find(dataCont->alias,from,&size);

	if (list!=NULL){
		for (x=0; x<size; x++){
			res=(res)||(alias_remove(&(dataCont->alias), from, list[x]));

		}
	}

	return res;
}


list_of_data_ptr dataCont_getDataIn(dataCont_ptr dataCont, char* cont_ID){
	if (dataCont==NULL) return NULL;
	if (dataCont->cont==NULL) return NULL;
	if (cont_ID==NULL) return NULL;

	cont_ptr c=list_of_cont_find(dataCont->cont, cont_ID);
	if (c==NULL) return NULL;
	return c->contained;   ///NOT A COPY!!!!
}


list_of_cont_ptr dataCont_getContOf(dataCont_ptr dataCont, char* data_ID){
	list_of_cont_ptr tmp_ptr;
	list_of_cont_ptr result= list_of_cont_new();
	list_of_data_ptr lod = list_of_data_new();

	if (dataCont==NULL) return NULL;
	if (dataCont->cont==NULL) return NULL;
	if (data_ID==NULL) return NULL;

	tmp_ptr= dataCont->cont;
	data_ptr d= data_new(data_ID);

	while (tmp_ptr!=NULL){
		lod=tmp_ptr->cont->contained;
		while(lod!=NULL){
			if (data_equals(lod->data,d)==0) {
				list_of_cont_add(&(result),tmp_ptr->cont);
				lod=NULL;
			} else {
				lod=lod->next;
			}
		}
		tmp_ptr=tmp_ptr->next;
	}

	list_of_data_free(lod);  /////////MODIFIED 1

	if (list_of_cont_length(result)==0) return NULL;
	return result; //COPY
}



//return l*(cont)
char** dataCont_getAliasesOf(dataCont_ptr dataCont, char* cont, int *size){
	if (dataCont==NULL) return NULL;
	return alias_closure(dataCont->alias, cont, size);
}


//return l(cont)
char** dataCont_getAliasesOf_noUpdate(dataCont_ptr dataCont, char* cont, int *size){
	if (dataCont==NULL) return NULL;
	return alias_find(dataCont->alias, cont, size);
}


//return list of c |  cont in l(c)
char** dataCont_getAliasesTo(dataCont_ptr dataCont, char* cont, int *size){
	char** res, **tmpres;
	char *tmpptr;
	alias_ptr tmp;
	int ressize, tmpsize, x;

	if (dataCont==NULL) return NULL;
	if (cont==NULL) return NULL;

	res=NULL;
	tmp=dataCont->alias;
	ressize=0;

	while (tmp!=NULL) {

		tmpsize=tmp->size;
		for (x=0; x<tmpsize; x++){
			if (strcmp(tmp->aliases[x], cont)==0){
				ressize++;
				tmpres=(char**)realloc(res,sizeof(char*)*(ressize));
				res=tmpres;
				tmpptr=strdup(tmp->aliases[x]);
				res[ressize-1]=tmpptr;
			}
		}
		tmp=tmp->next;
	}

	*size=ressize;
	return res;

}



unsigned int dataCont_updateAlias(dataCont_ptr dataCont){
	alias_ptr tmp;
	list_of_data_ptr lod, tmplod;
	list_of_cont_ptr loc;
	int x=0;
	unsigned int res=0, tmpres=0, loop=0;

	if (dataCont==NULL) return 1;

	while ((res==0)&&(loop <= pow(10,CONT_ID_LEN-2))){  //MAX NUM OF ITERATIONS = NUM OF CONTAINERS
		res=1;
		tmp=dataCont->alias;
		while (tmp!=NULL){
			lod=dataCont_getDataIn(dataCont, tmp->cont_ID);
			for (x=0; x<tmp->size; x++){
				tmplod=lod;
				while (tmplod!=NULL){
					//				printf ("ADDING DATA %s TO CONTAINER %s\n",tmplod->data->data_ID, tmp->aliases[x]);
					tmpres=(dataCont_addDataCont_noUpdate(dataCont, tmplod->data->data_ID, tmp->aliases[x]));
					res=(res)&&(tmpres);
					tmplod=tmplod->next;
				}
				//			printf ("\n");
			}
			//		printf("next\n");
			tmp=tmp->next;
		}
	}

	return res;

}


unsigned int dataCont_addNaming(dataCont_ptr dataCont, char* pid, char* id, char* cont){
	if ((dataCont==NULL)||(pid==NULL)||(id==NULL)||(cont==NULL)) return 1; //error
	if (strcmp(pid,"FNAME")!=0){
		dataCont_newCont(dataCont, pid);
	}
	return naming_add(&(dataCont->naming), pid, id, cont);
}

unsigned int dataCont_delNaming(dataCont_ptr dataCont, char* pid, char* id){
	if (dataCont==NULL) return 1; //error
	return naming_remove(&(dataCont->naming), pid, id);
}


unsigned int dataCont_delAllNamingFor(dataCont_ptr dataCont, char* cont){
	naming_ptr tmp,tmpprev;
	char* tmpcont;
	int res=0;
	int count=0;
	if ((dataCont==NULL)||(cont==NULL)) return 1; //error


	while ((res==0)&&(count++<MAX_LOOPS)){
		res=naming_remove(&(dataCont->naming), cont, "*");
	}

	return (count <  MAX_LOOPS);


	/*  tmp=(dataCont->naming);
  tmpprev=tmp;

  if (tmp!=NULL){
    if (tmp->cont_ID!=NULL){
      if (strcmp(tmp->cont_ID,cont)==0){
        free(tmp);
        tmp=tmp->next;
      }
    }
  }

  while (tmp!=NULL){
    if (tmp->cont_ID!=NULL){
      if (strcmp(tmp->cont_ID,cont)==0){
        if (tmpprev!=NULL) {
           tmpprev->next=tmp->next;
           free(tmpprev->next);
        }
      }
    }
    tmpprev=tmp;
    tmp=tmp->next;
  }
	 */

	return 0;


}



char* dataCont_getNaming(dataCont_ptr dataCont, char* pid, char* id){
	if (dataCont==NULL) return NULL; //error
	return naming_find(dataCont->naming, pid, id);
}

char** dataCont_getAllNamingsFrom(dataCont_ptr dataCont, char* pid, int* size){
	naming_ptr tmp;
	char **tmpres, **res;

	*size=-1;
	if ((pid==NULL)||(dataCont==NULL)) return NULL; // NULL values are wrong
	if (dataCont->naming==NULL) return NULL;

	tmp=dataCont->naming;


	*size=0;
	tmpres=NULL;
	res=NULL;

	while (tmp!=NULL){
		if ((tmp->pid!=NULL)&&(tmp->pid!=NULL)){
			if (strcmp(tmp->pid,pid)==0){
				(*size)++;
				tmpres=(char**)realloc(res,sizeof(char*)*(*size));
				res=tmpres;
				res[(*size)-1]=strdup(tmp->id);
			}
		}
		tmp=tmp->next;
	}

	return res;

}



/**
 * Given: Container ID cont_ID
 * Returns: an array of an array of strings.
 * The size of the "outer" array is given be the returned size parameter, indicating the number of names found.
 * The size of the "inner" array is 2, where the first index gives the PID and the second index gives the ID.
 */
char*** dataCont_getAllNamingsOfContainer(dataCont_ptr dataCont, char* cont_ID, int* size){
	naming_ptr tmp;
	char ***tmpres, ***res;
	char *pair[2];

	*size=-1;
	if ((cont_ID==NULL)||(dataCont==NULL)) return NULL; // NULL values are wrong
	if (dataCont->naming==NULL) return NULL;

	tmp=dataCont->naming;


	*size=0;
	tmpres=NULL;
	res=NULL;

	while (tmp!=NULL){
		if ((tmp->cont_ID!=NULL) && (tmp->pid!=NULL) && (tmp->id!=NULL)){
			if (strcmp(tmp->cont_ID,cont_ID)==0){
				(*size)++;
				//tmpres=(char***)realloc(res,(sizeof(char**) * (*size)) * (2 * sizeof(char*)));
				tmpres = (char***)realloc(res,sizeof(char**) * (*size));
				res=tmpres;
/*				*(*(res+((*size)-1)*sizeof(char**))+0*sizeof(char*)) = strdup(tmp->pid);
				*(*(res+((*size)-1)*sizeof(char**))+1*sizeof(char*)) = strdup(tmp->id);
	*/

				res[*size-1] = malloc(2 * sizeof(char*));

				res[*size-1][0] = strdup(tmp->pid);
				res[*size-1][1] = strdup(tmp->id);
			}
		}
		tmp=tmp->next;
	}

	return res;

}




unsigned int dataCont_addLodCont (dataCont_ptr dataCont, list_of_data_ptr lod, char*cont){
	list_of_data_ptr tmplod=lod;
	int res=0;
	int tmpres=0;
	if ((dataCont==NULL)||(lod==NULL)||(cont==NULL)) return 1;
	while (tmplod!=NULL){
//		printf ("ADDING DATA -> %s (list length=%d)\n",tmplod->data->data_ID, list_of_data_length(tmplod));
		tmpres=dataCont_addDataCont_noUpdate(dataCont,tmplod->data->data_ID,cont);
		res=res||tmpres;
		tmplod=tmplod->next;
	}
	return res;
}

unsigned int dataCont_addLodCont_noUpdate (dataCont_ptr dataCont, list_of_data_ptr lod, char*cont){
	list_of_data_ptr tmplod=lod;
	int res=0;
	int tmpres=0;
	if ((dataCont==NULL)||(lod==NULL)||(cont==NULL)) return 1;
	while (tmplod!=NULL){
		tmpres=dataCont_addDataCont_noUpdate(dataCont,tmplod->data->data_ID,cont);
		res=res||tmpres;
		tmplod=tmplod->next;
	}
	return res;
}





/**
 * PRINT
 */



void list_of_data_print(list_of_data_ptr lod, int mode){
	list_of_data_ptr tmp=lod;

	if (mode==1){ //verbose
		printf("length=%d : {",list_of_data_length(lod));
		fflush(NULL);
	}
	if ((mode==0)&&(tmp==NULL)) {
		printf(" ;"); //purely for aesthetical reasons
		fflush(NULL);
	}

	while (tmp!=NULL){
		if (tmp->data!=NULL) {
			printf(" %s%c",tmp->data->data_ID,((tmp->next==NULL)?';':','));
			fflush(NULL);
		}
		tmp=tmp->next;
	}
	if (mode==1){ //verbose
		printf(" }");
		fflush(NULL);
	}
	printf("\n");
	fflush(NULL);
}

void list_of_data_printf(FILE * file, list_of_data_ptr lod, int mode){
	list_of_data_ptr tmp=lod;

	if (mode==1){ //verbose
		fprintf(file, "length=%d : {",list_of_data_length(lod));
		fflush(NULL);
	}
	if ((mode==0)&&(tmp==NULL)) {
		fprintf(file," ;"); //purely for aesthetical reasons
		fflush(NULL);
	}

	while (tmp!=NULL){
		if (tmp->data!=NULL) {
			fprintf(file," %s%c",tmp->data->data_ID,((tmp->next==NULL)?';':','));
			fflush(NULL);
		}
		tmp=tmp->next;
	}
	if (mode==1){ //verbose
		fprintf(file," }");
		fflush(NULL);
	}
	fprintf(file,"\n");
	fflush(NULL);
}



void list_of_cont_print(list_of_cont_ptr loc, int mode){
	list_of_cont_ptr tmp=loc;
	if (mode>0){ //verbose
		printf("\nlength=%d : \n",list_of_cont_length(loc));
		fflush(NULL);
	}
	if ((mode==0)&&(tmp==NULL)){
		printf("<empty>"); //purely for aesthetical reasons
		fflush(NULL);
	}
	while (tmp!=NULL){
		if (tmp->cont!=NULL){
			if (!((mode==2)&&(list_of_data_length(tmp->cont->contained)==0))) {
				if (tmp->cont->cont_ID!=NULL) {
					printf("%s",tmp->cont->cont_ID);
					fflush(NULL);
				} else {
					printf("ERROR!!! list_of_cont_print: cont_ID==NULL\n");
					fflush(NULL);
					exit(1);
				}
				if (mode>0) {
					printf (" -> ");
					fflush(NULL);
					list_of_data_print(tmp->cont->contained,0);
				} else {
					printf ("%c ",((tmp->next==NULL)?';':','));
					fflush(NULL);
				}
			}
		}
		tmp=tmp->next;
	}
	printf("\n");
	fflush(NULL);
}


void list_of_cont_printf(FILE * file, list_of_cont_ptr loc, int mode){
	list_of_cont_ptr tmp=loc;
	if (mode>0){ //verbose
		fprintf(file,"\nlength=%d : \n",list_of_cont_length(loc));
		fflush(NULL);
	}
	if ((mode==0)&&(tmp==NULL)){
		fprintf(file,"<empty>"); //purely for aesthetical reasons
		fflush(NULL);
	}
	while (tmp!=NULL){
		if (tmp->cont!=NULL){
			if (!((mode==2)&&(list_of_data_length(tmp->cont->contained)==0))) {
				if (tmp->cont->cont_ID!=NULL) {
					fprintf(file,"%s",tmp->cont->cont_ID);
					fflush(NULL);
				} else {
					fprintf(file,"ERROR!!! list_of_cont_print: cont_ID==NULL\n");
					fflush(NULL);
					exit(1);
				}
				if (mode>0) {
					fprintf(file," -> ");
					fflush(NULL);
					list_of_data_printf(file, tmp->cont->contained,0);
				} else {
					fprintf(file,"%c ",((tmp->next==NULL)?';':','));
					fflush(NULL);
				}
			}
		}
		tmp=tmp->next;
	}
	fprintf(file,"\n");
	fflush(NULL);
}




void alias_print (alias_ptr alias, int mode){  //only mode=1 so far /** TODO **/
	alias_ptr tmp=alias;
	int x;

	if (tmp==NULL){
		if (mode==1) {
			printf ("Empty Alias Function!\n");
			fflush(NULL);
		}
	}
	while (tmp!=NULL){
		if (tmp->cont_ID!=NULL){
			printf("%s",tmp->cont_ID);
			fflush(NULL);

			if (mode==1) {
				printf (" -> ");
				fflush(NULL);
				for (x=0; x<tmp->size; x++){
					printf ("%s%c ",tmp->aliases[x],((x==tmp->size-1)?';':','));
					fflush(NULL);
				}
				printf("\n");
				fflush(NULL);
			}

		}
		tmp=tmp->next;
	}
	printf("\n");
	fflush(NULL);

}

void alias_printf (FILE *file, alias_ptr alias, int mode){  //only mode=1 so far /** TODO **/
	alias_ptr tmp=alias;
	int x;

	if (tmp==NULL){
		if (mode==1) {
			fprintf (file, "Empty Alias Function!\n");
			fflush(NULL);
		}
	}
	while (tmp!=NULL){
		if (tmp->cont_ID!=NULL){
			fprintf (file,"%s",tmp->cont_ID);
			fflush(NULL);

			if (mode==1) {
				fprintf (file," -> ");
				fflush(NULL);
				for (x=0; x<tmp->size; x++){
					fprintf (file,"%s%c ",tmp->aliases[x],((x==tmp->size-1)?';':','));
					fflush(NULL);
				}
				fprintf (file,"\n");
				fflush(NULL);
			}

		}
		tmp=tmp->next;
	}
	fprintf (file,"\n");
	fflush(NULL);

}


void naming_print (naming_ptr naming, int mode){
	naming_ptr tmp=naming;
	int x;

	if (tmp==NULL){
		if (mode==1) {
			printf ("Empty Naming Function!\n");
			fflush(NULL);
		}
	}
	while (tmp!=NULL){
		if ((tmp->id!=NULL)&&(tmp->pid!=NULL)){
			printf("%19s x %-40s %s %s\n",tmp->pid, tmp->id, "->", tmp->cont_ID);
			fflush(NULL);
		}
		tmp=tmp->next;
	}
	printf("\n");fflush(NULL);

}

void naming_printf(FILE * file, naming_ptr naming, int mode){
	naming_ptr tmp=naming;
	int x;

	if (tmp==NULL){
		if (mode==1) {
			fprintf (file, "Empty Naming Function!\n");
			fflush(NULL);
		}
	}
	while (tmp!=NULL){
		if ((tmp->id!=NULL)&&(tmp->pid!=NULL)){
			fprintf (file,"%s x %s -> %s\n",tmp->pid, tmp->id, tmp->cont_ID);
			fflush(NULL);
		}
		tmp=tmp->next;
	}
	fprintf (file,"\n");fflush(NULL);

}



void dataCont_print(dataCont_ptr dataCont, int mode){
	FILE *file;
	int cont_mode = -mode;

	list_of_cont_ptr loc= dataCont->cont;
	list_of_data_ptr lod;


	if (mode<0){
		file = fopen("output.txt","w+");

		fprintf(file,"\n-------------------------\n");
		fprintf(file,"contID = %d, dataID = %d\n", dataCont->cont_ID_counter,dataCont->data_ID_counter);
		fflush(NULL);
		fprintf(file,"cont size = %d, data size = %d\n", list_of_cont_length(dataCont->cont), list_of_data_length(dataCont->data));
		fflush(NULL);


		list_of_cont_printf(file, dataCont->cont,cont_mode); //1 std print 2 no-empty printed
		alias_printf (file, dataCont->alias,1);
		naming_printf (file, dataCont->naming,1);
		fprintf(file,"-------------------------\n\n\n\n");
		fclose(file); /*done!*/

	} else {
		printf("\n-------------------------\n");
		printf("contID = %d, dataID = %d\n", dataCont->cont_ID_counter,dataCont->data_ID_counter);
		fflush(NULL);
		printf("cont size = %d, data size = %d\n", list_of_cont_length(dataCont->cont), list_of_data_length(dataCont->data));
		fflush(NULL);


		list_of_cont_print(dataCont->cont,mode);//1 std print 2 no-empty printed
		alias_print(dataCont->alias,1);
		naming_print (dataCont->naming,1);
		printf("-------------------------\n\n\n\n");

	}





	/*	while (list_of_cont_length(loc)>0){
		printf ("%s -> ",(loc->cont)->cont_ID);
		lod=(loc->cont)->contained;
		printf ("(%d) ", list_of_data_length(lod));
		while (list_of_data_length(lod)> 0){
			printf("%s ", (lod->data)->data_ID);
			lod=lod->next;
		}
		printf (";\n");
		loc=loc->next;
	}

	printf ("\n");
	 */

}




/*

int main(){
	dataCont_ptr dc = dataCont_new();
	char* c0, *c1, *c2, *c3, *c4, *c5, *c6, *c7, *c8, *c9; 
	char* d0, *d1, *d2, *d3, *d4, *d5, *d6;

	char *p0="App0";
	char *p1="App1";
	char *p2="App2";
	char *p3="App3";


	char *fn0="FileName0";
	char *fn1="FileName1";
	char *fn2="FileName2";
	char *fn3="FileName3";

	char *fd0="FileDesc0";
	char *fd1="FileDesc1";
	char *fd2="FileDesc2";
	char *fd3="FileDesc3";

	char **list;
	int size;


	int x=-1;

	d0 = dataCont_getNewDataID(dc);
	d1 = dataCont_getNewDataID(dc);
	d2 = dataCont_getNewDataID(dc);
	d3 = dataCont_getNewDataID(dc);
	d4 = dataCont_getNewDataID(dc);
	d5 = dataCont_getNewDataID(dc);
	d6 = dataCont_getNewDataID(dc);

	c0 = dataCont_getNewContID(dc);
	c1 = dataCont_getNewContID(dc);
	c2 = dataCont_getNewContID(dc);
	c3 = dataCont_getNewContID(dc);
	c4 = dataCont_getNewContID(dc);
	c5 = dataCont_getNewContID(dc);
	c6 = dataCont_getNewContID(dc);
	c7 = dataCont_getNewContID(dc);
	c8 = dataCont_getNewContID(dc);
	c9 = dataCont_getNewContID(dc);





	printf ("c0 (%x) = %s\n",c0,c0);
	printf ("c1 (%x) = %s\n",c1,c1);
	printf ("c2 (%x) = %s\n",c2,c2);
	printf ("c3 (%x) = %s\n\n",c3,c3);
	printf ("c4 (%x) = %s\n\n",c4,c4);
	printf ("c5 (%x) = %s\n",c5,c5);
	printf ("c6 (%x) = %s\n",c6,c6);
	printf ("c7 (%x) = %s\n",c7,c7);
	printf ("c8 (%x) = %s\n\n",c8,c8);
	printf ("c9 (%x) = %s\n\n",c9,c9);


	printf ("d0 (%x) = %s\n",d0,d0);
	printf ("d1 (%x) = %s\n",d1,d1);
	printf ("d2 (%x) = %s\n",d2,d2);
	printf ("d3 (%x) = %s\n",d3,d3);
	printf ("d4 (%x) = %s\n",d4,d4);
	printf ("d5 (%x) = %s\n",d5,d5);
	printf ("d6 (%x) = %s\n",d6,d6);

	printf ("\n////////\n\n");



	dataCont_print(dc,1);

	printf ("----\n\n");

	dataCont_newData(dc, d0);
	dataCont_newData(dc, d1);
	dataCont_newData(dc, d2);
	dataCont_newData(dc, d3);
	dataCont_newData(dc, d4);
	dataCont_newData(dc, d5);
	dataCont_newData(dc, d6);

	dataCont_newCont(dc, c0);
	dataCont_newCont(dc, c1);
	dataCont_newCont(dc, c2);
	dataCont_newCont(dc, c3);
	dataCont_newCont(dc, c4);
	dataCont_newCont(dc, c5);
	dataCont_newCont(dc, c6);
	dataCont_newCont(dc, c7);
	dataCont_newCont(dc, c8);
	dataCont_newCont(dc, c9);

	dataCont_newCont(dc, p0);
	dataCont_newCont(dc, p1);
	dataCont_newCont(dc, p2);
	dataCont_newCont(dc, p3);


	dataCont_print(dc,1);

	printf ("----\n\n");

	dataCont_addDataCont (dc, d0, c0);
	dataCont_addDataCont (dc, d1, c1);
	dataCont_addDataCont (dc, d2, c2);
	dataCont_addDataCont (dc, d3, c3);
	dataCont_addDataCont (dc, d4, c4);
	dataCont_addDataCont (dc, d5, c5);


	dataCont_addAlias (dc, c4, c5);
	dataCont_addAlias (dc, c2, c4);
	dataCont_addAlias (dc, c2, c3);
	dataCont_addAlias (dc, c4, c3);	
	dataCont_addAlias (dc, c5, c2);
	dataCont_addAlias (dc, c1, c2);
	dataCont_addAlias (dc, c6, c1);


	dataCont_addAlias (dc, c2, c6);
	dataCont_addAlias (dc, c2, c4);
	dataCont_addAlias (dc, c4, c3);
	dataCont_addAlias (dc, c4, c2);
	dataCont_addAlias (dc, c5, c1);
	dataCont_addAlias (dc, c1, c5);
	dataCont_addAlias (dc, c6, c4);



	dataCont_addNaming (dc, p0, fn0, c3);
	dataCont_addNaming (dc, p1, fn0, c3);
	dataCont_addNaming (dc, p2, fn1, c2);
	dataCont_addNaming (dc, p3, fd0, c0);
	dataCont_addNaming (dc, p3, fd1, c1);
	dataCont_addNaming (dc, p3, fd2, c2);

	dataCont_print(dc,1);
	printf ("----\n\n");
	getchar();

	x=dataCont_addNaming (dc, p3, fd0, c0);
	printf ("----\n\n");
	printf ("ADDNAMING %s %s %s -> %d\n",p3,fd0,c0,x);
	dataCont_print(dc,1);
	getchar();


	x=dataCont_delNaming (dc, p3, fd0);
	printf ("----\n\n");
	printf ("DELNAMING %s %s -> %d\n",p3,fd0,x);
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delAllAliasesTo (dc, c3);
	printf ("----\n\n");
	printf ("delAllAliasesTo %s -> %d\n",c3,x);
	dataCont_print(dc,1);
	getchar();



	list=dataCont_getAliasesOf(dc, c2, &size);
	printf ("size=%d\ncont %s -> ",size,c2);
	for (x=0; x<size; x++){
		printf ("%s ",list[x]);
	}
	getchar();




	x=dataCont_delNaming (dc, p2, fd0);
	printf ("----\n\n");
	printf ("DELNAMING %s %s -> %d\n",p2,fd0,x);
	dataCont_print(dc,1);
	getchar();


	x=dataCont_delNaming (dc, p0, fn0);
	printf ("----\n\n");
	printf ("DELNAMING %s %s -> %d\n",p0,fn0,x);
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delNaming (dc, p3, fd0);
	printf ("----\n\n");
	printf ("DELNAMING %s %s -> %d\n",p3,fd0,x);
	dataCont_print(dc,1);
	getchar();


	x=dataCont_delNaming (dc, p3, fd2);
	printf ("----\n\n");
	printf ("DELNAMING %s %s -> %d\n",p3,fd2,x);
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delNaming (dc, p3, fd2);
	printf ("----\n\n");
	printf ("DELNAMING %s %s -> %d\n",p3,fd2,x);
	dataCont_print(dc,1);
	getchar();








	dataCont_addNaming (dc, p3, fd0, c1);
	printf ("----\n\n");
	printf ("ADDNAMING %s %s %s\n",p3,fd0,c1);
	dataCont_print(dc,1);
	getchar();

	dataCont_addNaming (dc, p3, fd1, c0);
	printf ("----\n\n");
	printf ("ADDNAMING %s %s %s\n",p3,fd1,c0);
	dataCont_print(dc,1);
	getchar();

	dataCont_addNaming (dc, p0, fn0, c3);
	printf ("----\n\n");
	printf ("ADDNAMING %s %s %s\n",p0,fn0,c3);
	dataCont_print(dc,1);
	getchar();

	dataCont_addNaming (dc, p0, fn0, c1);
	printf ("----\n\n");
	printf ("ADDNAMING %s %s %s\n",p0,fn0,c1);
	dataCont_print(dc,1);
	getchar();




	x=dataCont_updateAlias(dc);
	dataCont_print(dc,1);
	printf ("----  X = %d \n\n\n\n\n",x);
	getchar();

	x=dataCont_updateAlias(dc);
	dataCont_print(dc,1);
	printf ("----  X = %d \n\n\n\n\n",x);
	getchar();









	printf("add alias c0 c1\n");
	dataCont_addAlias (dc, c0, c1);
	printf("add alias c1 c0\n");
	dataCont_addAlias (dc, c1, c1);	
	printf("add alias c2 c1\n");
	dataCont_addAlias (dc, c2, c1);
	printf("add alias c2 c3\n");
	dataCont_addAlias (dc, c2, c3);

	printf("add alias c4 c1\n");
	dataCont_addAlias (dc, c4, c1);
	printf("add alias c4 c2\n");
	dataCont_addAlias (dc, c4, c2);	
	printf("add alias c4 c3\n");
	dataCont_addAlias (dc, c4, c3);
	printf("add alias c4 c5\n");
	dataCont_addAlias (dc, c4, c5);

	printf("add alias c7 c8\n");
	dataCont_addAlias (dc, c7, c8);
	printf("add alias c7 c9\n");
	dataCont_addAlias (dc, c7, c9);	
	printf("add alias c7 c3\n");
	dataCont_addAlias (dc, c7, c3);
	printf("add alias c7 c5\n");
	dataCont_addAlias (dc, c7, c5);
	printf("add alias c7 c1\n");
	dataCont_addAlias (dc, c7, c1);
	printf("add alias c7 c2\n");
	dataCont_addAlias (dc, c7, c2);	
	printf("add alias c7 c6\n");
	dataCont_addAlias (dc, c7, c6);



	printf("add alias c0 c8\n");
	dataCont_addAlias (dc, c0, c8);
	printf("add alias c0 c9\n");
	dataCont_addAlias (dc, c0, c9);	
	printf("add alias c0 c3\n");
	dataCont_addAlias (dc, c0, c3);
	printf("add alias c0 c5\n");
	dataCont_addAlias (dc, c0, c5);
	printf("add alias c0 c1\n");
	dataCont_addAlias (dc, c0, c1);
	printf("add alias c0 c2\n");
	dataCont_addAlias (dc, c0, c2);	
	printf("add alias c0 c6\n");
	dataCont_addAlias (dc, c0, c6);






















	printf("del alias c0 c1\n");
	dataCont_delAlias (dc, c0, c1);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();


	printf("del alias c0 c1\n");
	dataCont_delAlias (dc, c0, c1);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();


	printf("del alias c1 c1\n");
	dataCont_delAlias (dc, c1, c1);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();


	printf("del alias c2 c3\n");
	dataCont_delAlias (dc, c2, c3);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();


	printf("del alias c2 c3\n");
	dataCont_delAlias (dc, c2, c3);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("add alias c1 c4\n");
	dataCont_addAlias (dc, c1, c4);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("del alias c1 c2\n");
	dataCont_delAlias (dc, c1, c2);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("add alias c1 c2\n");
	dataCont_addAlias (dc, c1, c2);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();







	printf("del alias c0 c9\n");
	dataCont_delAlias (dc, c0, c9);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("del alias c0 c1\n");
	dataCont_delAlias (dc, c0, c1);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("del alias c0 c6\n");
	dataCont_delAlias (dc, c0, c6);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();




	printf("del alias c1 c0\n");
	dataCont_delAlias (dc, c1, c0);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("del alias c1 c2\n");
	dataCont_delAlias (dc, c1, c2);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("del alias c1 c3\n");
	dataCont_delAlias (dc, c1, c3);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();







	printf("del alias c7 c5\n");
	dataCont_delAlias (dc, c7, c5);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("del alias c7 c6\n");
	dataCont_delAlias (dc, c7, c6);

	dataCont_print(dc,1);

	 ("----\n\n\n\n\n");
	getchar();



	printf("del alias c7 c8\n");
	dataCont_delAlias (dc, c7, c8);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();


	printf("add alias c7 c8\n");
	dataCont_addAlias (dc, c7, c8);
	printf("add alias c7 c9\n");
	dataCont_addAlias (dc, c7, c9);	
	printf("add alias c7 c3\n");
	dataCont_addAlias (dc, c7, c3);
	printf("add alias c7 c5\n");
	dataCont_addAlias (dc, c7, c5);
	printf("add alias c7 c1\n");
	dataCont_addAlias (dc, c7, c1);
	printf("add alias c7 c2\n");
	dataCont_addAlias (dc, c7, c2);	
	printf("add alias c7 c6\n");
	dataCont_addAlias (dc, c7, c6);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();



	printf("add alias c0 c8\n");
	dataCont_addAlias (dc, c0, c8);
	printf("add alias c0 c9\n");
	dataCont_addAlias (dc, c0, c9);	
	printf("add alias c0 c3\n");
	dataCont_addAlias (dc, c0, c3);
	printf("add alias c0 c5\n");
	dataCont_addAlias (dc, c0, c5);
	printf("add alias c0 c1\n");
	dataCont_addAlias (dc, c0, c1);
	printf("add alias c0 c2\n");
	dataCont_addAlias (dc, c0, c2);	
	printf("add alias c0 c6\n");
	dataCont_addAlias (dc, c0, c6);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();	




	printf("add alias c1 c8\n");
	dataCont_addAlias (dc, c1, c8);
	printf("add alias c1 c9\n");
	dataCont_addAlias (dc, c1, c9);	
	printf("add alias c1 c3\n");
	dataCont_addAlias (dc, c1, c3);
	printf("add alias c1 c5\n");
	dataCont_addAlias (dc, c1, c5);
	printf("add alias c1 c1\n");
	dataCont_addAlias (dc, c1, c1);
	printf("add alias c1 c2\n");
	dataCont_addAlias (dc, c1, c2);	
	printf("add alias c1 c6\n");
	dataCont_addAlias (dc, c1, c6);

	dataCont_print(dc,1);

	printf ("----\n\n\n\n\n");
	getchar();


	x=how_many_copies(dc->cont, d3);
	printf("how many copies of D3? %d\n",x);

	x=how_many_copies(dc->cont, d2);
	printf("how many copies of D2? %d\n",x);

	x=how_many_copies(dc->cont, d1);
	printf("how many copies of D1? %d\n",x);

	x=how_many_copies(dc->cont, d0);
	printf("how many copies of D0? %d\n",x);

	x=how_many_copies(dc->cont, "pippo");
	printf("how many copies of pippo? %d\n",x);


	x=dataCont_delDataCont (dc, d3, c3);
	printf("#############%d\n",x);
	printf("## D3 - C3 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d3, c3);
	printf("#############%d\n",x);
	printf("## D0 - C0 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();


	x=dataCont_delDataCont (dc, d3, c2);
	printf("#############%d\n",x);
	printf("## D3 - C2 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d3, c2);
	printf("#############%d\n",x);
	printf("## D3 - C2 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d3, c1);
	printf("#############%d\n",x);
	printf("## D3 - C1 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();


	x=dataCont_delDataCont (dc, d0, c0);
	printf("#############%d\n",x);
	printf("## D0 - C0 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d0, c0);
	printf("#############%d\n",x);
	printf("## D0 - C0 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();



	printf("getDataIn C0\n");
	list_of_data_print(dataCont_getDataIn(dc, c0),1);

	printf("getDataIn C1\n");
	list_of_data_print(dataCont_getDataIn(dc, c1),1);

	printf("getDataIn C2\n");
	list_of_data_print(dataCont_getDataIn(dc, c2),1);

	printf("getDataIn C3\n");
	list_of_data_print(dataCont_getDataIn(dc, c3),1);


	printf("\n\ngetContOf D0\n");
	list_of_cont_print(dataCont_getContOf(dc, d0),0);

	printf("getContOf D1\n");
	list_of_cont_print(dataCont_getContOf(dc, d1),0);

	printf("getContOf D2\n");
	list_of_cont_print(dataCont_getContOf(dc, d2),0);

	printf("getContOf D3\n");
	list_of_cont_print(dataCont_getContOf(dc, d3),0);

	printf("\n\n");


	/*
	x=dataCont_addDataCont (dc, d0, c0);
	printf("+++++++++++++%d\n",x);
	printf("++ D0 - C0 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_addDataCont (dc, d1, c1);
	printf("+++++++++++++%d\n",x);
	printf("++ D1 - C1 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_addDataCont (dc, d2, c1);
	printf("+++++++++++++%d\n",x);
	printf("++ D2 - C1 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_addDataCont (dc, d3, c1);
	printf("+++++++++++++%d\n",x);
	printf("++ D3 - C1 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_addDataCont (dc, d2, c1);
	printf("+++++++++++++%d\n",x);
	printf("++ D2 - C1 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d2, c1);
	printf("#############%d\n",x);
	printf("## D2 - C1 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_addDataCont (dc, d2, c1);
	printf("+++++++++++++%d\n",x);
	printf("++ D2 - C1 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d1, c1);
	printf("#############%d\n",x);
	printf("## D1 - C1 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();	

	x=dataCont_addDataCont (dc, d1, c1);
	printf("+++++++++++++%d\n",x);
	printf("++ D1 - C1 ++\n");
	printf("+++++++++++++\n");
	dataCont_print(dc,1);
	getchar();

	x=dataCont_delDataCont (dc, d3, c1);
	printf("#############%d\n",x);
	printf("## D3 - C1 ##\n");
	printf("#############\n");
	dataCont_print(dc,1);
	getchar();





	return 0;
}
 */




/**
 * Returns the ip address that identifies this host in dest,
 * which is must provide memory for at least 9 chars.
 * ip_addr_list is a list of IP addresses separated by IP_ADDR_SEP.
 * This method chooses the lowest ip addresses in the list, excluding localhost 127.0.0.1.
 * This method returns -1 on failure, which means that no valid address != 127.0.0.1 has been found,
 * 0 on success.
 */
uint32_t xget_host_ip(char *dest, char *ip_addr_list) {
	uint32_t res_ip = 0xFFFFFFFF;
	uint32_t tmp_ip = 0;
	char *pos;

	while ((pos = strchr(ip_addr_list,IP_ADDR_SEP)) != NULL) {
		sscanf(ip_addr_list,"%08X,",&tmp_ip);

		if ((tmp_ip < res_ip) && (tmp_ip >> 24 != 0x7F)) {
			res_ip = tmp_ip;
		}

		ip_addr_list = pos+1;
	}

	sscanf(ip_addr_list,"%08X,",&tmp_ip);

	if ((tmp_ip < res_ip) && (tmp_ip >> 24 != 0x7F)) {
		res_ip = tmp_ip;
	}


	if (res_ip == 0xFFFFFFFF || res_ip == 0x7F000001) {
		// failure
		return -1;
	}

	memset(dest,0,9);
	sprintf(dest,"%08X",res_ip);
	return 0;
}



/**
 * Returns a string of all ip addresses in buf of size size. The
 * addresses are separated by IP_ADDR_SEP. The return value is the address of buf.
 * If parameter lo == 1, then the localhost ip address (127.0.0.1 = 7F000001) is included,
 * otherwise it is not.
 */
char *xget_all_ip_addresses(char *buf, int size, int lo) {
	struct ifaddrs *ifa_struct=NULL;
	struct ifaddrs *ifa=NULL;

	// get all ip addresses
	getifaddrs(&ifa_struct);

	memset(buf,0,size);
	int i = 0;
	// loop over all ip addresses and assign a name to the container for each address
	for (ifa = ifa_struct; ifa != NULL; ifa = ifa->ifa_next) {

		// only for IPv4 addresses: add address to buffer
		if (ifa->ifa_addr->sa_family == AF_INET) {
			uint32_t addr = ntohl(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr);
			// only include addresses that are note local loopback 127.0.0.1
			if (addr >> 24 != 0x7f) {
				snprintf(buf+i*9,size-i*9,"%08X%c",ntohl(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr),IP_ADDR_SEP);
				i++;
			}
		}
	}

	// include local loopback 127.0.0.1, if requested
	if (lo == 1) {
		snprintf(buf+i*9,size-i*9,"%08X%c",0x7F000001,IP_ADDR_SEP);
		i++;
	}

	buf[i*9-1] = '\0';
	return buf;
}

