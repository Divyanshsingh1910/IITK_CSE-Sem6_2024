all: python

lex.yy.c: python.l python.tab.h
	flex python.l

python.tab.c python.tab.h: python.y ast.h
	bison -d -t python.y

python: lex.yy.c python.tab.c ast.h
	g++ lex.yy.c python.tab.h python.tab.c -ll -o python

clean:
	rm -f python python.tab.h python.tab.c lex.yy.c
