all: python

lex.yy.c: python.l python.tab.h
	flex python.l

python.tab.c python.tab.h: python.y ast.h symtable.h x86.h
	bison -d -t python.y

python: lex.yy.c python.tab.c ast.h symtable.h symtable.cpp ast.cpp x86.h x86.cpp
	g++ symtable.cpp x86.cpp ast.cpp lex.yy.c python.tab.h python.tab.c -ll -o python

clean:
	rm -f python python.tab.h python.tab.c lex.yy.c *.dot *.png *.csv *.txt *.s *.o final asm
	
