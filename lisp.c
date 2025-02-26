/* An implementation of lisp. */
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

/* Implementation notes:
   nil is a symbol and also the only false value.
   NULL isn't used as an Sexp, and therefore always an error.
*/

typedef enum SexpType SexpType;
enum SexpType {
  ST_UNKNOWN = 0,
  ST_SYMBOL,
  ST_PAIR,
  ST_BUILTIN,
  ST_NUMBER,
};

typedef union Sexp Sexp ;
union Sexp {
  SexpType type;
  struct {
    SexpType type;
    char *name;
  } symbol;
  struct {
    SexpType type;
    uint64_t value;
  } number;
  struct {
    SexpType type;
    Sexp *head;
    Sexp *tail;
  } pair;
  struct {
    SexpType type;
    uint64_t nargs;
    Sexp *(*fun)();
  } builtin;
};


Sexp *NIL;
Sexp *TRUE;
Sexp *_env;

static void
cerror(char *msg)
{
  fprintf(stderr, "error: %s\n", msg);
  exit(1);
}

static Sexp *
mk_sexp(SexpType type)
{
  Sexp *exp = malloc(sizeof(*exp));
  if (!exp) cerror("mk_sexp: could not allocate sexp");
  exp->type = type;
  return exp;
}

static Sexp *
symbol(char *name)
{
  Sexp *sym = mk_sexp(ST_SYMBOL);
  sym->symbol.name = strdup(name);
  return sym;
}

static Sexp *
number(int64_t value)
{
  Sexp *res = mk_sexp(ST_NUMBER);
  res->number.value = value;
  return res;
}

static Sexp *
builtin(Sexp *(*fun)(), uint64_t nargs)
{
  Sexp *res = mk_sexp(ST_BUILTIN);
  res->builtin.fun = fun;
  res->builtin.nargs = nargs;
  return res;
}

Sexp *
cons(Sexp *head, Sexp *tail)
{
  Sexp *e = mk_sexp(ST_PAIR);
  e->pair.head = head;
  e->pair.tail = tail;
  return e;
}

Sexp *
car(Sexp *pair)
{
  if (!pair || pair->type != ST_PAIR)
    cerror("car: not a pair");
  return pair->pair.head;
}

Sexp *
cdr(Sexp *s)
{
  if (!s || s->type != ST_PAIR)
    cerror("cdr: not a pair");
  return s->pair.tail;
}

static Sexp *
asBool(int b) { return b ? TRUE : NIL ; }

Sexp *
eq(Sexp *s1, Sexp *s2)
{
  if (!s1 || !s2)
    cerror("eq: got NULL");
  if ((s1->type != ST_SYMBOL && s1->type != ST_NUMBER)
      || (s2->type != ST_SYMBOL && s2->type != ST_NUMBER))
    cerror("eq: only symbols and numbers can be compared");
  if (s1->type != s2->type) return NIL;
  if (s1->type == ST_SYMBOL)
    return asBool(0 == strcmp(s1->symbol.name, s2->symbol.name));
  if (s1->type == ST_NUMBER)
    return asBool(s1->number.value == s2->number.value);
  cerror("eq: unreachable");
}

void write_sexp(Sexp *);

void
write_pair(Sexp *s)
{
  write_sexp(s->pair.head);
  if (s->pair.tail->type == ST_PAIR) {
    printf(" ");
    write_pair(s->pair.tail);
  } else if (s->pair.tail->type == ST_SYMBOL && TRUE == eq(s->pair.tail, NIL))
    ;
  else {
    printf(" . ");
    write_sexp(s->pair.tail);
  }
}

void
write_sexp(Sexp *s)
{
  if (!s) cerror("write: got NULL");
  switch (s->type) {
  case ST_NUMBER:
    printf("%" PRId64, s->number.value);
    break;
  case ST_SYMBOL:
    printf("%s", s->symbol.name);
    break;
  case ST_PAIR:
    printf("(");
    write_pair(s);
    printf(")");
    break;
  default:
    cerror("write_sexp: type not implemented");
  }
}

Sexp *
pairp(Sexp *e) { return (e && e->type == ST_PAIR) ? TRUE : NIL ; }
Sexp *
symbolp(Sexp *e) { return (e && e->type == ST_SYMBOL) ? TRUE : NIL ; }
Sexp *
numberp(Sexp *e) { return (e && e->type == ST_NUMBER) ? TRUE : NIL ; }
Sexp *
nilp(Sexp *e) { return (e && e->type == ST_SYMBOL) ? eq(e, NIL) : NIL ; }
Sexp *
builtinp(Sexp *e) { return (e && e->type == ST_BUILTIN) ? TRUE : NIL; }

Sexp *
bind(Sexp *key, Sexp *val, Sexp *env)
{
  return cons(cons(key, val), env);
}

static int
isNil(Sexp *e)
{
  return e && e->type == ST_SYMBOL && (0 == strcmp(e->symbol.name, "nil"));
}
static int
isTrue(Sexp *e) { return !isNil(e); }
static Sexp *
list(Sexp **exps)
{
  Sexp **end = exps;
  Sexp *res = NIL;
  while (*end) end += 1;
  end--;
  while (end >= exps) {
    res = cons(*end, res);
    end -= 1;
  }
  return res;
}

// Returns tail of env starting at entry for key, e.g. ((key . val) ...)
// Or nil if key is not in env.
Sexp *
lookup(Sexp *key, Sexp *env)
{
  Sexp *head;
  while (!isNil(env)) {
    head = car(env);
    if (isNil(pairp(head))) cerror("lookup: car of environment has to be a pair.");
    if (isTrue(eq(key, car(head))))
      return env;
    env = cdr(env);
  }
  return NIL;
}

Sexp *
listp(Sexp *exp)
{
  while (isTrue(pairp(exp)))
    exp = cdr(exp);
  return eq(NIL, exp);
}

Sexp *
length(Sexp *exp)
{
  int64_t len = 0;
  while (isTrue(pairp(exp))) {
    len += 1;
    exp = cdr(exp);
  }
  if (!isNil(exp)) cerror("length: argument has to be list");
  return number(len);
}

Sexp *
apply_builtin(Sexp *builtin, Sexp *args)
{
  Sexp *res;
  Sexp *arg1, *arg2, *arg3, *arg4;
  int64_t nargs = length(args)->number.value;
  if (builtin->builtin.nargs != nargs) cerror("apply_builtin: wrong number of arguments");
  switch (nargs) {
  default:
    cerror("apply_builtin: invalid number of arguments");
  case 0:
    Sexp *(*builtin0)() = builtin->builtin.fun;
    return builtin0();
  case 1:
    arg1 = car(args);
    Sexp *(*builtin1)(Sexp *) = (Sexp *(*)(Sexp *)) builtin->builtin.fun;
    return builtin1(arg1);
  case 2:
    arg1 = car(args);
    arg2 = car(cdr(args));
    Sexp *(*builtin2)(Sexp *, Sexp *) = (Sexp *(*)(Sexp *, Sexp *)) builtin->builtin.fun;
    return builtin2(arg1, arg2);
  case 3:
    arg1 = car(args);
    arg2 = car(cdr(args));
    arg3 = car(cdr(cdr(args)));
    Sexp *(*builtin3)(Sexp *, Sexp *, Sexp *) = (Sexp *(*)(Sexp *, Sexp *, Sexp *)) builtin->builtin.fun;
    return builtin3(arg1, arg2, arg3);
  case 4:
    arg1 = car(args);
    arg2 = car(cdr(args));
    arg3 = car(cdr(cdr(args)));
    arg4 = car(cdr(cdr(cdr(args))));
    Sexp *(*builtin4)(Sexp *, Sexp *, Sexp *, Sexp *) = (Sexp *(*)(Sexp *, Sexp *, Sexp *, Sexp *)) builtin->builtin.fun;
    return builtin4(arg1, arg2, arg3, arg4);
  }
  cerror("apply_builtin: unreachable code");
}

Sexp *lambdap(Sexp *fun)
{
  if (isNil(listp(fun)) || isNil(eq(number(3), length(fun)))) return NIL;
  if (isNil(eq(symbol("lambda"), car(fun)))) return NIL;
  if (isNil(listp(car(cdr(fun))))) return NIL;
  return TRUE;
}

Sexp *eval(Sexp *);

Sexp *
apply(Sexp *fun, Sexp *args /*, Sexp *env*/)
{
  Sexp *fargs, *body, *oldenv, *result;
  if (isNil(lambdap(fun))) cerror("apply: expected a lambda");
  fargs = car(cdr(fun));
  if (isNil(eq(length(fargs), length(args)))) cerror("apply: number of arguments does not match");
  body = car(cdr(cdr(fun)));
  oldenv = _env;
  while (!isNil(fargs)) {
    _env = bind(car(fargs), car(args), _env);
    fargs = cdr(fargs);
    args = cdr(args);
  }
  result = eval(body);
  _env = oldenv;
  return result;
}

Sexp *eval_list(Sexp * /*, Sexp *env*/);

Sexp *
eval(Sexp *exp) /*, Sexp *env, Sexp **newenv) */
{
  Sexp *tmp, *head, *tail;
  if (!exp) cerror("eval: got NULL\n");
  if (isTrue(numberp(exp))) {
    return exp;
  } else if (isTrue(symbolp(exp))) {
    tmp = lookup(exp, _env /*env*/);
    if (isNil(tmp)) cerror("eval: got unbound symbol");
    return cdr(car(tmp));
  } else if (isTrue(pairp(exp))) {
    if (isTrue(symbolp(car(exp))) && isTrue(eq(symbol("if"), car(exp)))) {
      // handle if
      if (isNil(listp(cdr(exp))) || isNil(eq(number(3), length(cdr(exp)))))
	cerror("eval: if takes the form (if condition true-case false-case)");
      tmp = eval(car(cdr(exp)) /*, env*/);
      return  isTrue(tmp) ? eval(car(cdr(cdr(exp))) /*, env*/) : eval(car(cdr(cdr(cdr(exp)))) /*, env*/) ;
    } else if (isTrue(symbolp(car(exp))) && isTrue(eq(symbol("quote"), car(exp)))) {
      // handle quote
      if (isNil(listp(cdr(exp))) || isNil(eq(number(1), length(cdr(exp)))))
	cerror("eval: quote takes the form (quote expression)");
      return car(cdr(exp));
    } else if (isTrue(symbolp(car(exp))) && isTrue(eq(symbol("define"), car(exp)))) {
      // handle define
      if (isNil(listp(exp))) cerror("eval: define has to be a list");
      if (isNil(eq(number(3), length(exp)))) cerror("eval: define takes the form (define symbol expression)");
      if (isNil(symbolp(car(cdr(exp))))) cerror("eval: define: first argument has to be a symbol");
      head = car(cdr(exp));
      tail = eval(car(cdr(cdr(exp))) /*, env*/);
      _env = bind(head, tail, _env);
      return NIL;
    } else if (isTrue(symbolp(car(exp))) && isTrue(eq(symbol("fun"), car(exp)))) {
      // handle fun / lambda
      cerror("eval: fun not yet implemented.");
      if (isNil(listp(exp))) cerror("eval: fun has to be a list");
      if (isNil(eq(number(4), length(exp)))) cerror("eval: fun takes the form (fun name args body)");
    } else {
      head = eval(car(exp) /*, env*/);
      tail = eval_list(cdr(exp) /*, env*/);
      if (head->type == ST_BUILTIN) return apply_builtin(head, tail);
      else return apply(head, tail);
    }
  }
}

Sexp *
eval_list(Sexp *list /*, Sexp *env*/)
{
  if (isNil(list)) return list;
  if (isNil(pairp(list))) cerror("eval_list: argument has to be a list");
  return cons(eval(car(list) /*, env*/), eval_list(cdr(list) /*, env*/));
}

static Sexp *
quote(Sexp *exp)
{
  return cons(symbol("quote"), cons(exp, NIL));
}
static Sexp *
lambda(Sexp *args, Sexp *body)
{
  return cons(symbol("lambda"), cons(args, cons(body, NIL)));
}
static Sexp *
sexp_if(Sexp *cond, Sexp *iftrue, Sexp *iffalse)
{
  return cons(symbol("if"), cons(cond, cons(iftrue, cons(iffalse, NIL))));
}

static Sexp *
sexp_add(Sexp *n1, Sexp *n2)
{
  if (isNil(numberp(n1)) || isNil(numberp(n2))) cerror("sexp_add: both arguments have to be numbers");
  return number(n1->number.value + n2->number.value);
}

// NEXT: TODO: read

typedef enum {
  TT_LP, TT_RP, TT_QUOTE, TT_DOT, TT_SYMBOL, TT_NUMBER, TT_EOI, TT_NONE
} TokenType;

typedef struct Token {
  TokenType type;
  Sexp *value;
} Token ;

static char *current_input;
static Token current_token = { TT_NONE, NULL };

void
set_input(char *string)
{
  current_input = string;
  current_token.type = TT_NONE;
  current_token.value = NULL;
}
int
read_peekchar()
{
  if (!*current_input) return -1;
  return *current_input;
}
int
read_getchar()
{
  if (!*current_input) return -1;
  return *current_input++;
}

static int
is_special_char(int c)
{
  return c == ' ' || c == '\n' || c == '\t' ||
    c == '\'' || c == '(' || c == ')' || c == '.';
}

// NEXT: TODO: Test token reading

#define MAX_TOKEN_LENGTH 512
void
read_token()
{
  char token_text[MAX_TOKEN_LENGTH];
  int i, isnum;
  int64_t numval;
  int c = read_getchar();
  while (c == ' ' || c == '\n' || c == '\t') c = read_getchar();
  current_token.value = NULL;
  switch (c) {
  case -1: current_token.type = TT_EOI; break;
  case '\'': current_token.type = TT_QUOTE; break;
  case '(': current_token.type = TT_LP; break;
  case ')': current_token.type = TT_RP; break;
  case '.': current_token.type = TT_DOT; break;
  default: current_token.type = TT_SYMBOL; break;
  }
  if (current_token.type != TT_SYMBOL) return;
  i = 0;
  token_text[i++] = c;
  while (-1 != (c = read_peekchar()) && !is_special_char(c) && i < MAX_TOKEN_LENGTH - 1) {
    token_text[i++] = c;
    read_getchar();
  }
  token_text[i] = '\0';
  isnum = 1;
  for (i=0; token_text[i]; i++)
    if (!isdigit(token_text[i])) { isnum = 0; break; }
  if (isnum) {
    current_token.type = TT_NUMBER;
    current_token.value = number(atol(token_text));
  } else {
    current_token.value = symbol(token_text);
  }
}
int
match_token(TokenType type)
{
  if (current_token.type == TT_NONE) read_token();
  return current_token.type == type;
}
Token
consume_token(void)
{
  Token res;
  if (current_token.type == TT_NONE) read_token();
  res = current_token;
  read_token();
  return res;
}

Sexp *
read(void)
{
  if (match_token(TT_EOI)) {
    consume_token();
    return symbol("end-of-file");
  } else if (match_token(TT_NUMBER) || match_token(TT_SYMBOL)) {
    Token token = consume_token();
    return token.value;
  } else if (match_token(TT_QUOTE)) {
    consume_token();
    Sexp *val = read();
    if (isTrue(symbolp(val)) && isTrue(eq(symbol("end-of-file"), val))) cerror("read: unexpected eof after quote");
    return quote(val);
  } else if (match_token(TT_LP)) {
    Sexp *res, *val, *nextval;
    consume_token();
    nextval = read();
    val = res = cons(nextval, NIL);
    while (!match_token(TT_RP) && !match_token(TT_DOT) && !match_token(TT_EOI)
	   && !(isTrue(symbolp(nextval = read())) && isTrue(eq(symbol("end-of-file"), nextval)))) {
      // nextval = read();
      val->pair.tail = cons(nextval, NIL);
      val = val->pair.tail;
    }
    if (isTrue(symbolp(nextval)) && isTrue(eq(symbol("end-of-file"), nextval))) cerror("read: unexpected eof in list");
    if (match_token(TT_EOI)) {
      cerror("read: unclosed list, got eof");
    } else if (match_token(TT_RP)) {
      consume_token();
    } else if (match_token(TT_DOT)) {
      consume_token();
      nextval = read();
      if (isTrue(symbolp(nextval)) && isTrue(eq(symbol("end-of-file"), nextval)) ||
	  !match_token(TT_RP))
	cerror("read: unterminated dotted list");
      consume_token(); // consume RP
      val->pair.tail = nextval;
    } else {
      cerror("read: unexpected token in list");
    }
    return res;
  } else {
    Token token = consume_token();
    switch (token.type) {
    case TT_EOI: printf("*** EOI\n"); break;
    case TT_DOT: printf("*** DOT\n"); break;
    case TT_RP: printf("*** RP\n"); break;
    default: printf("*** UNKNOWN\n"); break;
    }
    cerror("read: sexp starts with unexpected token");
  }
}

// TODO: Proper error handling, both lisp and C.
void
init(void)
{
  NIL = symbol("nil");
  TRUE = symbol("t");
  _env = NIL;
  _env = bind(NIL, NIL, _env);
  _env = bind(TRUE, TRUE, _env);
  _env = bind(symbol("sixteen"), number(16), _env);
  _env = bind(symbol("car"), builtin(car, 1), _env);
  _env = bind(symbol("cdr"), builtin(cdr, 1), _env);
  _env = bind(symbol("cons"), builtin(cons, 2), _env);
  _env = bind(symbol("number?"), builtin(numberp, 1), _env);
  _env = bind(symbol("nil?"), builtin(nilp, 1), _env);
  _env = bind(symbol("symbol?"), builtin(symbolp, 1), _env);
  _env = bind(symbol("pair?"), builtin(pairp, 1), _env);
  _env = bind(symbol("builtin?"), builtin(builtinp, 1), _env);
  _env = bind(symbol("+"), builtin(sexp_add, 2), _env);
}

int
main(void)
{
  init();

  ///* Test eval and read
  int i;
  char *test_strings[] = {
    "1234",
    "'123",
    "'hellow-orld",
    "'''5",
    " 'many 'reads 'for 'many 'symbols",
    " '( a list with spaces ) ",
    "'(dotted . pair)",
    "'(a dotted . list)",
    "'(quoted . '(nesting))",
    "(car '(a . b))",
      "(cdr '(a . b))",
      "(cdr '(1 2 3 4))",
      "(car (cdr (cdr '(1 2 3 4))))",
      "(car (cons 'a 'b))",
      "(if nil 'yes 'no)",
      "(if 'yes 'yes! 'no!)",
    "(define length\n"
    "  '(lambda (a-list)\n"
    "    (if a-list\n"
    "        (+ 1 (length (cdr a-list)))\n"
    "        0)))\n",
    "(length (quote (1 2 3 4 5)))",
    "(length '(1 2 3 4 5))",
  };

  for (i = 0; i < sizeof(test_strings)/sizeof(*test_strings); i++) {
    Sexp *prog, *result;
    set_input(test_strings[i]);
    while (!(isTrue(symbolp(prog = read())) && isTrue(eq(symbol("end-of-file"), prog)))) {
      printf("eval of "); write_sexp(prog); printf(": ");
      write_sexp(eval(prog)); printf("\n");
    }
  }
  //*/

  /* Read test
  int i;
  Token token;
  char *test_strings[] = {
    "1234",
    "'hellow-orld",
    "'''5",
    "  quote ",
    " many reads for many symbols",
    " ( a list with spaces ) ",
    "(dotted . pair)",
    "(a dotted . list)",
    "'(quoted . '(nesting))",
    "' more-quote-tests",
    "(length (quote (1 2 3 4 5)))",
    "(length '(1 2 3 4 5))",
    "(define length\n"
    "  (lambda (a-list)\n"
    "    (if a-list\n"
    "        (+ 1 (length (cdr a-list)))\n"
    "        0)))\n",
  };
  for (i = 0; i < sizeof(test_strings)/sizeof(*test_strings); i++) {
    Sexp *val;
    printf(">>%s<<\n", test_strings[i]);
    set_input(test_strings[i]);
    while (!(isTrue(symbolp(val = read())) && isTrue(eq(symbol("end-of-file"), val)))) {
      write_sexp(val); printf("\n");
    }
  }
  //*/

  /* Read token test
  int i;
  Token token;
  char *test_strings[] = {
    "1234",
    " 1 2 3 4 5 '6 ",
    "'''5",
    "(1+ 15)",
    "'hellow-orld",
    "  quote ",
    " many reads for many symbols",
    " ( a list with spaces ) ",
    "(dotted . pair)",
    "(a dotted . list)",
    "'(quoted . '(nesting))",
    "' more-quote-tests",
    "(length (quote (1 2 3 4 5)))",
    "(length '(1 2 3 4 5))",
    "(define length\n"
    "  (lambda (a-list)\n"
    "    (if a-list\n"
    "        (+ 1 (length (cdr a-list)))\n"
    "        0)))\n",
  };
  
  for (i = 0; i < sizeof(test_strings)/sizeof(*test_strings); i++) {
    Sexp *val;
    printf(">>%s<<\n", test_strings[i]);
    set_input(test_strings[i]);
    while ((token = consume_token()).type != TT_EOI) {
      switch (token.type) {
      case TT_LP: printf("(\n"); break;
      case TT_RP: printf(")\n"); break;
      case TT_QUOTE: printf("'\n"); break;
      case TT_DOT: printf(".\n"); break;
      case TT_SYMBOL: printf("symbol: >>%s<<\n", token.value->symbol.name); break;
      case TT_NUMBER: printf("number: %"PRId64"\n", token.value->number.value); break;
      default:
	printf("*** Unknown token type\n"); break;
      }
    }
  }
  //*/

  /* Read Test
  char *defun_string =
    "(define length\n"
    "  (lambda (a-list)\n"
    "    (if a-list\n"
    "        (+ 1 (length (cdr a-list)))\n"
    "        0)))\n";
  char *invoke_string =
    "(length (quote (1 2 3 4 5)))";
  //*/
  
  /* Test program
  Sexp *fun = lambda(cons(symbol("a-list"), NIL),
		     sexp_if(symbol("a-list"),
			     cons(symbol("+"),
				  cons(number(1),
				       cons(cons(symbol("length"),
						 cons(cons(symbol("cdr"), cons(symbol("a-list"), NIL)),
						      NIL)), NIL))),
			     number(0)));
  Sexp *defun = cons(symbol("define"), cons(symbol("length"), cons(quote(fun), NIL)));
  Sexp *invocation = cons(symbol("length"), cons(quote(cons(number(1), cons(number(2), cons(number(3), NIL)))), NIL));

  printf("defining length\n");
  eval(defun);
  printf("eval (length '(1 2 3)): ");
  write_sexp(eval(invocation)); printf("\n");
  printf("eval (length nil): ");
  write_sexp(eval(cons(symbol("length"), cons(NIL, NIL)))); printf("\n");
  
  //*/
  
  /* Test define
  Sexp *def1 = cons(symbol("define"), cons(symbol("a-number"), cons(number(53535), NIL)));
  Sexp *def2 = cons(symbol("define"), cons(symbol("a-list"), cons(cons(symbol("quote"),
								       cons(cons(number(1), cons(number(2), NIL)) , NIL)), NIL)));
  eval(def1);
  eval(def2);
  printf("eval of a-number: ");
  write_sexp(eval(symbol("a-number"))); printf("\n");
  printf("eval of a-list: ");
  write_sexp(eval(symbol("a-list"))); printf("\n");
  */
  
  /* Test function application
  Sexp *fun1 = lambda(cons(symbol("x"), NIL),
		      number(5));
  Sexp *fun2 = lambda(cons(symbol("x"), NIL),
		      symbol("x"));
  Sexp *fun3 = lambda(cons(symbol("yes?"), NIL),
		      sexp_if(symbol("yes?"), quote(symbol("yes!")), quote(symbol("no!"))));
  Sexp *defun1 = cons(symbol("define"), cons(symbol("yes-or-no"), cons(quote(fun3), NIL)));
  printf("eval of ((quote (lambda (x) 5)) 17): ");
  write_sexp(eval(cons(quote(fun1), cons(number(17), NIL)))); printf("\n");
  printf("eval of ((quote (lambda (x) x)) 17): ");
  write_sexp(eval(cons(quote(fun2), cons(number(17), NIL)))); printf("\n");
  printf("eval of ((quote (lambda (yes?) (if yes? 'yes! 'no!)) 'yes): ");
  write_sexp(eval(cons(quote(fun3), cons(quote(symbol("yes")), NIL)))); printf("\n");
  printf("eval of ((quote (lambda (yes?) (if yes? 'yes! 'no!)) nil): ");
  write_sexp(eval(cons(quote(fun3), cons(NIL, NIL)))); printf("\n");
  printf("eval of ((quote (lambda (yes?) (if yes? 'yes! 'no!)) 'nil): ");
  write_sexp(eval(cons(quote(fun3), cons(quote(NIL), NIL)))); printf("\n");
  printf("eval of ((quote (lambda (yes?) (if yes? 'yes! 'no!)) ''nil): ");
  write_sexp(eval(cons(quote(fun3), cons(quote(quote(NIL)), NIL)))); printf("\n");
  printf("defining yes-or-no\n");
  eval(defun1);
  printf("eval of (yes-or-no 'yes): ");
  write_sexp(eval(cons(symbol("yes-or-no"), cons(quote(symbol("yes")), NIL)))); printf("\n");
  printf("eval of (yes-or-no nil): ");
  write_sexp(eval(cons(symbol("yes-or-no"), cons(NIL, NIL)))); printf("\n");
  //*/

  /* eval test
  printf("eval: t: ");
  write_sexp(eval(symbol("t"), _env)); printf("\n");
  printf("eval: sixteen: ");
  write_sexp(eval(symbol("sixteen"), _env)); printf("\n");
  printf("eval: 16: ");
  write_sexp(eval(number(16), _env)); printf("\n");
  printf("eval: nil: ");
  write_sexp(eval(symbol("nil"), _env)); printf("\n");
  //*/
  
  /* quote test
  printf("eval: (quote 15): ");
  write_sexp(eval(cons(symbol("quote"), cons(number(15), NIL)), _env)); printf("\n");
  printf("eval: (quote sym): ");
  write_sexp(eval(cons(symbol("quote"), cons(symbol("sym"), NIL)), _env)); printf("\n");
  printf("eval: (quote (quote sym)): ");
  write_sexp(eval(cons(symbol("quote"), cons(cons(symbol("quote"), cons(symbol("sym"), NIL)), NIL)), _env)); printf("\n");
  printf("eval: (quote (1 2 3)): ");
  write_sexp(eval(cons(symbol("quote"), cons(cons(number(1), cons(number(2), cons(number(3), NIL))), NIL)),
		  _env)); printf("\n");
  //*/
  /* if test
  Sexp *IF = symbol("if");
  Sexp *yes = symbol("yes");
  Sexp *no = symbol("no");
  _env = bind(yes, yes, _env);
  _env = bind(no, no, _env);
  printf("eval: (if nil yes no): ");
  write_sexp(eval(cons(IF, cons(NIL, cons(yes, cons(no, NIL)))), _env)); printf("\n");
  printf("eval: (if t yes no): ");
  write_sexp(eval(cons(IF, cons(TRUE, cons(yes, cons(no, NIL)))), _env)); printf("\n");
  printf("eval: (if (quote t) yes no): ");
  write_sexp(eval(cons(IF, cons(cons(symbol("quote"), cons(TRUE, NIL)), cons(yes, cons(no, NIL)))), _env)); printf("\n");
  // Should throw error
  //printf("eval: unknown: ");
  //write_sexp(eval(symbol("unknown"), _env)); printf("\n");
  //*/

  /* builtin test
  Sexp *app;
  app = cons(symbol("number?"), cons(number(15), NIL));
  printf("eval: (number? 15): ");
  write_sexp(eval(app, _env)); printf("\n");
  app = cons(symbol("number?"), cons(cons(symbol("quote"), cons(symbol("hello"), NIL)), NIL));
  printf("eval: (number? (quote hello)): ");
  write_sexp(eval(app, _env)); printf("\n");
  app = cons(symbol("car"), cons(cons(symbol("quote"), cons(cons(number(1), number(2)), NIL)), NIL));
  printf("eval: (car (quote (cons 1 2))): ");
  write_sexp(eval(app, _env)); printf("\n");
  app = cons(symbol("cdr"), cons(cons(symbol("quote"), cons(cons(number(1), number(2)), NIL)), NIL));
  printf("eval: (cdr (quote (cons 1 2))): ");
  write_sexp(eval(app, _env)); printf("\n");
  app = cons(symbol("cons"), cons(number(1), cons(number(2), NIL)));
  printf("eval: (cons 1 2): ");
  write_sexp(eval(app, _env)); printf("\n");
  app = cons(symbol("cons"), cons(NIL, cons(NIL, NIL)));
  printf("eval: (cons nil nil): ");
  write_sexp(eval(app, _env)); printf("\n");
  //*/

  /* list test
  printf("write: (nil nil nil nil): ");
  write_sexp(list((Sexp *[]){NIL, NIL, NIL, NIL, NULL})); printf("\n");
  printf("write: (1 2 3 4): ");
  write_sexp(list((Sexp *[]){number(1), number(2), number(3), number(4), NULL})); printf("\n");
  printf("write: (quote (1 2)): ");
  write_sexp(list((Sexp *[]){symbol("quote"), list((Sexp *[]){number(1), number(2), NULL}), NULL})); printf("\n");
  printf("write: (if t yes no): ");
  write_sexp(list((Sexp *[]){symbol("if"), TRUE, symbol("yes"), symbol("no"), NULL})); printf("\n");
  //*/

  // printing pairs
  //Sexp *pairs = cons(cons(symbol("x"), symbol("y")), cons(cons(NIL, TRUE), cons(number(0), number(1))));
  
  /* Lookup test
  printf("lookup '+: ");
  write_sexp(lookup(symbol("+"), _env)); printf("\n");
  printf("lookup 'nil: ");
  write_sexp(lookup(symbol("nil"), _env)); printf("\n");
  printf("lookup 't: ");
  write_sexp(lookup(symbol("t"), _env)); printf("\n");
  printf("lookup 'sixteen: ");
  write_sexp(lookup(symbol("sixteen"), _env)); printf("\n");
  //*/

  return 0;
}
