/* TODO: Handle quoted atom */
%option noyywrap

%{
#include "term.h"

#include "y.tab.h"

extern long timestep;
%}

ATOM      [a-z][a-zA-Z0-9_]*|"'"([^\']|"''")*"'"
VARIABLE  [A-Z_][a-zA-Z0-9_]*
NUMBER    [0-9]+
			
%%

"%"[^\n]*       ;
[][{}()|,.]        { return *yytext; }
{ATOM}/"("      { yylval.term = mk_atom(yytext);
                  return CP_FUNCTOR; }
{ATOM}    	{ yylval.term = mk_atom(yytext); return CP_ATOM; }
{VARIABLE} 	{ yylval.term = mk_var(yytext, timestep); return CP_VAR; }
{NUMBER}  	{ yylval.term = mk_num(atol(yytext)); return CP_NUM; }
"\n"            { yylineno++; }
[ \t]+          ;
.         	{ printf("Error: Unrecognized character >>%c<< in line %d\n",
                         *yytext, yylineno);
                  return -1; }


%%

