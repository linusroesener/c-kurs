

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "term.h"

int yylex(void);
int yyerror(char *msg);
extern int yylineno;
extern char *yytext;

// This is where the parsed term is put after yyparse() accepts.
CorTerm *parsed_term;

// xTODO: How do we allocate terms when reading tuples?
//   Given that they can be nested, we would need a stack, and keep track
//   of the beginning index and size, so that we can reduce N terms starting at I
//   to a single tuple of size N at I.
#define MAXNUM_TERMS  4096
CorTerm *terms[MAXNUM_TERMS];
CorTerm **top_of_terms = terms;
long timestep = 0;

void pushterm(CorTerm *term);
CorTerm *popterm(void);
CorTerm **popterms(long nterms);

%}


%union {
    long nterms;
    CorTerm *term;
}

%token	<term>		CP_ATOM CP_NUM CP_FUNCTOR CP_VAR
%type	<term>		term structure tuple list list_tail
%type	<nterms>	terms
			
			
%%

toplevel_term:	term '.'
		{ timestep++ ;
		  parsed_term = $1;
		  YYACCEPT; }
	|	
		{ parsed_term = NULL; YYACCEPT; }
	;
term:		CP_VAR
		{ $$ = $1; }
	|	CP_ATOM
		{ $$ = $1; }
	|	tuple
		{ $$ = $1; }
	|	CP_NUM
		{ $$ = $1; }
	|	list
		{ $$ = $1; }
	;
tuple:		'{' term      { pushterm($2); }
		terms '}'
		{ $$ = mk_tuple($4 + 1, popterms($4 + 1)); }
	|	structure
		{ $$ = $1; }
	;
structure:	CP_FUNCTOR     { pushterm($1); }
		'(' term       { pushterm($4); }
		terms ')'
		{ $$ = mk_tuple($6 + 2, popterms($6 + 2)); }
	;
list:		'[' ']'
		    { $$ = mk_atom("[]"); }
	|	'[' term     { pushterm($2); }
		terms list_tail ']'
		{ /* TODO */  ;
		    CorTerm *list = $5;
		    CorTerm *tail, *head;
		    long nelem = $4 + 1;
		    while (nelem--) {
			tail = list;
			head = popterm();
			pushterm(mk_atom("."));
			pushterm(head);
			pushterm(tail);
			list = mk_tuple(3, popterms(3));
		    }
		    $$ = list;
		}
		;

list_tail:      /* empty */
		{ $$ = mk_atom("[]"); }
	|	'|' term
		{ $$ = $2; }
		;
terms:
		{ $$ = 0; }
	|	terms ',' term
	        { pushterm($3); $$ = $1 + 1; }
	;

%%

int
yyerror(char *msg) {
    fprintf(stderr, "%d: %s at >>%s<<\n", yylineno, msg, yytext);
    return 0;
}


void
pushterm(CorTerm *term) {
    *top_of_terms = term;
    top_of_terms += 1;
}

CorTerm *
popterm()
{
    return *(--top_of_terms);
}

CorTerm **
popterms(long nterms) {
    CorTerm **res = malloc(nterms * sizeof(CorTerm *));
    memcpy(res, top_of_terms-nterms, nterms * sizeof(CorTerm*));
    top_of_terms -= nterms;
    return res;
}

