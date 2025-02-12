

all:	main

main:	main.o term.o lex.yy.o y.tab.o
	cc -o main main.o term.o lex.yy.o y.tab.o -ll

main.o: main.c y.tab.h

y.tab.c y.tab.h:	cord-parser.y
	yacc -d cord-parser.y

lex.yy.c:	cord-lexer.l y.tab.h
	lex cord-lexer.l

clean:
	rm *.o y.tab.* lex.yy.c main


