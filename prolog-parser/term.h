


typedef enum  {
  COR_VAR, COR_NUM, COR_ATOM, COR_TUPLE
} CorTermType; 

typedef union CorTerm CorTerm;
union CorTerm {
  CorTermType type;
  // name is the display name, id is the actual identifier
  // we use to look it up in environments
  struct { CorTermType type; char *name; long timestep; int id; } var;
  struct { CorTermType type; long val; } num;
  struct { CorTermType type; char *name; } atom;
  struct { CorTermType type; long nterms; CorTerm **terms; } tuple;
} ;

CorTerm *mk_tuple(long nterms, CorTerm **terms);
CorTerm *mk_num(long number);
CorTerm *mk_atom(char *name);
CorTerm *mk_var(char *name, long timestep);

void print_term(CorTerm *term);

