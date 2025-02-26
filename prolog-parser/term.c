
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "term.h"



CorTerm *
mk_tuple(long nterms, CorTerm **terms) {
  CorTerm *t = calloc(1, sizeof(CorTerm));
  t->type = COR_TUPLE;
  t->tuple.nterms = nterms;
  t->tuple.terms = terms;
  return t;
}

CorTerm *
mk_num(long number) {
  CorTerm *term = calloc(1, sizeof(CorTerm));
  term->type = COR_NUM;
  term->num.val = number;
  return term;
}

CorTerm *
mk_atom(char *name) {
  CorTerm *term = calloc(1, sizeof(CorTerm));
  term->type = COR_ATOM;
  term->atom.name = strdup(name);
  return term;
}

CorTerm *
mk_var(char *name, long timestep) {
  CorTerm *term = calloc(1, sizeof(CorTerm));
  term->type = COR_VAR;
  term->var.name = strdup(name);
  term->var.timestep = timestep;
  return term;
}


// TODO: Handle atoms requiring quoting
void
print_term(CorTerm *t) {
    char *sep = "";
    long i;
    if (!t) { fprintf(stdout, "null"); return; }
    switch (t->type) {
    case COR_TUPLE:
	fprintf(stdout, "{");
	for (i=0; i < t->tuple.nterms; i++) {
	    fprintf(stdout, "%s", sep);
	    print_term(t->tuple.terms[i]);
	    sep = ", ";
	}
	fprintf(stdout, "}");
	break;
    case COR_NUM:
	fprintf(stdout, "%ld", t->num.val);
	break;
    case COR_VAR:
	fprintf(stdout, "%s_%ld", t->var.name, t->var.timestep);
	break;
    case COR_ATOM:
	fprintf(stdout, "%s", t->atom.name);
	break;
    default:
	fprintf(stdout, "?");
	break;
    }
}
	    
