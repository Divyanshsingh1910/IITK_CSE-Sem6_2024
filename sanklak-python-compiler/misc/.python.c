#include <bits/stdc++.h>
#include "python.h"
using namespace std;

int yylex();
extern FILE *yyin;
extern char *yytext;
extern int yylineno;
extern void init_stack();

vector<string> names = {"AMPEREQUAL", "AND", "ASSERT", "BREAK", "CIRCUMFLEXEQUAL", "CLASS", "CONTINUE", "DEDENT", "DEF", "DOUBLESLASH", "DOUBLESLASHEQUAL", "DOUBLESTAR", "DOUBLESTAREQUAL", "ELIF", "ELSE", "ENDMARKER", "EQEQUAL", "FALSE", "FOR", "GREATEREQUAL", "IF", "IN", "INDENT", "IS", "LEFTSHIFT", "LEFTSHIFTEQUAL", "LESSEQUAL", "MINEQUAL", "NAME", "NEWLINE", "NONE", "NOT", "NOTEQUAL", "NUMBER", "OR", "PERCENTEQUAL", "PLUSEQUAL", "RARROW", "RETURN", "RIGHTSHIFTEQUAL", "RIGHTSHIFT", "SLASHEQUAL", "STAREQUAL", "STRING", "THREEDOTS", "TRUE", "VBAREQUAL", "WHILE", "WHITESPACE"};

int main(int argc, char *argv[]){
    if(argc != 2){
        printf("READING FROM STANDARD INPUT\n");
        yyin = stdin;
    }
	else{
		yyin = fopen(argv[1], "r");
		if(!yyin){
			printf("ERROR: NO SUCH FILE FOUND\n");
			return -1;
		}
	}
	init_stack();
    int ntoken;
	ntoken = yylex();
	while(ntoken != ENDMARKER){
		cout << names[ntoken] << " found\n";
		ntoken = yylex();
	}
    if(yyin != stdin){
		fclose(yyin);
	}
    return 0;
}
