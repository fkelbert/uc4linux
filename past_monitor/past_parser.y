// Declarations Section 
%{

#include "past_monitor.h"
#include <stdio.h>
#include <string.h>
  
#define INITIAL_TABLE_SIZE 10
#define INITIAL_NUM_PARAMS 5
	
int yylex(void);
int new_node(int operator, int op1, int op2, int op3, int op4, event_t* event);
int yyerror ();

%}

%union {
  int num;
  char *text;
  event_t event_p;
}

%token T
%token F
%token EFST
%token EALL    
%token XPATH    
%token NOT    
%token AND    
%token OR    
%token IMPLIES    
%token SINCE        
%token ALWAYS        
%token BEFORE        
%token WITHIN        
%token DURING        
%token REPMAX        
%token REPSINCE    
%token REPLIM    
%token POEVNAME        
%token POPARAM
%token DENYC
%token DENYD
%token LIMIT
%token DAY
%token HOUR
%token MINUTE
%token SECOND
%token TIMESTEP
%token<num> NUMBER        
%token<text> IDENT        
%token ERROR        

%type <num> expression
%type <num> pexpression
%type <num> time_unit
%type <event_p> event

%left IMPLIES
%left OR 
%left AND
%right NOT

%start policy
%{
   int n;
   int table_size;
   formula_t *p_actions;
   event_t p_event;
   int n_params;
   int size_params;
   param_t *param_list;
%}

%%

// Rules Section
policy: {
  n = 0;
  table_size = INITIAL_TABLE_SIZE;
  p_actions = (formula_t *)malloc(sizeof(formula_t)*table_size);
  memset(p_actions,NULL,sizeof(formula_t)*table_size);} expression;
   
time_unit: 
            {$$ = 1;} // empty
 | DAY      {$$ = 86400;}
 | HOUR     {$$ = 3600;}
 | MINUTE   {$$ = 60;}
 | SECOND   {$$ = 1;}
 | TIMESTEP {$$ = 1;}
;

expression: 
   T {$$ = new_node(T,NULL,NULL,NULL,NULL,NULL);}
 | F {$$ = new_node(F,NULL,NULL,NULL,NULL,NULL);}
 | EFST '(' event ')' {$$ = new_node(EFST,NULL,NULL,NULL,NULL,&$3);}
 | EALL '(' event ')' {$$ = new_node(EALL,NULL,NULL,NULL,NULL,&$3);}
 | XPATH '(' IDENT ')' {$$ = new_node(XPATH,$3,NULL,NULL,NULL,NULL);}
 | '(' expression ')' {$$ = $2;}
 | NOT expression {$$ = new_node(NOT,$2,NULL,NULL,NULL,NULL);}
 | expression AND expression {$$ = new_node(AND,$1,$3,NULL,NULL,NULL);}
 | expression OR expression {$$ = new_node(OR,$1,$3,NULL,NULL,NULL);}
 | expression IMPLIES expression {$$ = new_node(IMPLIES,$1,$3,NULL,NULL,NULL);}
 | SINCE '('expression ',' expression')' {$$ = new_node(SINCE,$3,$5,NULL,NULL,NULL);}
 | ALWAYS '('expression')' {$$ = new_node(ALWAYS,$3,NULL,NULL,NULL,NULL);}
 | BEFORE '('NUMBER time_unit ',' expression')' {$$ = new_node(BEFORE,$3*$4,$6,NULL,NULL,NULL);}
 | WITHIN '('NUMBER time_unit ',' expression')' {$$ = new_node(WITHIN,$3*$4,$6,NULL,NULL,NULL);}
 | DURING '('NUMBER time_unit ',' expression')' {$$ = new_node(DURING,$3*$4,$6,NULL,NULL,NULL);}
 | REPMAX '('NUMBER ',' pexpression')' {$$ = new_node(REPMAX,$3,$5,NULL,NULL,NULL);}
 | REPLIM '('NUMBER time_unit ',' NUMBER ',' NUMBER ',' pexpression')' {$$ = new_node(REPLIM,$3*$4,$6,$8,$10,NULL);}
 | REPSINCE '('NUMBER ',' pexpression ',' expression')' {$$ = new_node(REPSINCE,$3,$5,$7,NULL,NULL);}
 | POEVNAME '(''{'levent'}' ',''{' lparams'}'')' {$$ = n;}
 | POPARAM '(' '{'levent'}' ',' IDENT ',' IDENT ',''{' lparams'}' ')' {$$ = n;}
 | DENYC '(' ')' {$$ = new_node(DENYC,NULL,NULL,NULL,NULL,NULL);}
 | DENYD '(' ')' {$$ = new_node(DENYD,NULL,NULL,NULL,NULL,NULL);}
 | LIMIT '(' ')' {$$ = new_node(LIMIT,NULL,NULL,NULL,NULL,NULL);}
;

pexpression: 
   T {$$ = new_node(T,NULL,NULL,NULL,NULL,NULL);}
 | F {$$ = new_node(F,NULL,NULL,NULL,NULL,NULL);}
 | EFST '(' event ')' {$$ = new_node(EFST,NULL,NULL,NULL,NULL,&$3);}
 | EALL '(' event ')' {$$ = new_node(EALL,NULL,NULL,NULL,NULL,&$3);}
 | XPATH '(' IDENT ')' {$$ = new_node(XPATH,$3,NULL,NULL,NULL,NULL);}
 | '(' pexpression ')' {$$ = $2;}
 | NOT pexpression {$$ = new_node(NOT,$2,NULL,NULL,NULL,NULL);}
 | pexpression AND pexpression {$$ = new_node(AND,$1,$3,NULL,NULL,NULL);}
 | pexpression OR pexpression {$$ = new_node(OR,$1,$3,NULL,NULL,NULL);}
 | pexpression IMPLIES pexpression {$$ = new_node(IMPLIES,$1,$3,NULL,NULL,NULL);}
;

event: IDENT ',' '{' { 
  n_params = 0; 
  size_params = INITIAL_NUM_PARAMS;
  param_list = (param_t*)malloc(sizeof(param_t)*size_params)
} lparams '}' { $$.event_name = $1; $$.n_params = n_params; $$.params = param_list; }
 | IDENT    {$$.event_name = $1; $$.n_params = 0;}
;

lparams: param
     | param ',' lparams
     ;    

param: '(' IDENT ',' IDENT ')' {
  param_t *p_aux;
  if(n_params >= size_params){
    p_aux = (param_t*)malloc(sizeof(param_t) * size_params * 2);
    memcpy(p_aux,param_list,sizeof(param_t)*size_params);
    free(param_list);
    param_list = p_aux;
    size_params = size_params * 2;  
  }
  param_list[n_params].param_name = $2;
  param_list[n_params].param_value = $4;
  n_params++;
};

levent:
   IDENT
 | IDENT ',' levent;

%%
/* Code Section */

int parse_policy (char *policy, int *num, formula_t **p_formula) {
  
  int result;  
  
  init_scanner(policy);
  
  /*Analyse string*/
  result = yyparse();    
  
  close_scanner();
  
  if(!result){
    *num = n;
    *p_formula = p_actions;
  }

  /* return 0 if ok, else error */
  return (result);
}

int new_node(int operator, int op1, int op2, int op3, int op4, event_t* event) {

  formula_t *p_aux;

  if(n >= table_size){
    //resize table
    p_aux = (formula_t*)malloc(sizeof(formula_t)*table_size*2);
    memset(p_aux,NULL,sizeof(formula_t)*table_size*2);
    memcpy(p_aux,p_actions,sizeof(formula_t)*table_size);
    free(p_actions);
    p_actions = p_aux;
    table_size = table_size * 2;
  }
   
  //insert node
  p_actions[n].op = operator;
  p_actions[n].op1 = op1;
  p_actions[n].op2 = op2;
  p_actions[n].op3 = op3;
  p_actions[n].op4 = op4;
  if(event){
    p_actions[n].ev.event_name = (char *)malloc(strlen(event->event_name));
    strcpy(p_actions[n].ev.event_name, event->event_name);
    p_actions[n].ev.n_params = event->n_params;
    p_actions[n].ev.params = event->params;
  }
    
  //update number of sub-formulas
  n++;
  
  //return table position
  return(n - 1);
}
