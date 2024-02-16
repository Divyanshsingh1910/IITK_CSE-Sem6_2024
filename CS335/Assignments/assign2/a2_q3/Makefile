# makefiel for parserner

#Default target
all: parser


# Rule to generate lex.yy.c
lex.yy.c: parser.l parser.tab.h
	flex parser.l


# Rule to generate y.tab.c & y.tab.h
parser.tab.c parser.tab.h: parser.y
	bison -d parser.y

#Rule to build the executable
parser: lex.yy.c parser.tab.c
	gcc lex.yy.c parser.tab.h parser.tab.c -o parser

#Clean rule
clean:
	rm -f parser parser.tab.h parser.tab.c lex.yy.c
