
#include <stdlib.h>
#include <stdio.h>


#include "term.h"

#include "y.tab.h"

int
main(int argc, char **argv) {
  extern CorTerm *parsed_term;
  int yyparse(void);
  
  yyparse();
  while (parsed_term) {
    print_term(parsed_term);
    puts("");
    yyparse();
  }
  
  return 0;
}

