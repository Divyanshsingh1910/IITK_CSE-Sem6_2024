
%{
	#include "parser.tab.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	void yyerror (char *s);
	int yylex();
	int getMarks(char *s);
%}

%option nounput yylineno noyywrap

quiz_open				"<"(?i:quiz)">"
quiz_close              "<""/"(?i:quiz)">"

Digit					[1-8]
space					[ ]*
marks_part				{space}(?i:marks){space}"="{space}["]{space}{Digit}{space}["]{space}

single_open				"<"{space}(?:singleselect){marks_part}">"
single_close			"<""/"{space}(?i:singleselect){space}">"

multi_open				"<"{space}(?i:multiselect){marks_part}">"
multi_close				"<""/"{space}(?i:multiselect){space}">"

string				[^<>"\n\t]*

choice_open				"<"{space}(?i:choice){space}">"
choice_close			"<""/"{space}(?i:choice){space}">"

correct_open			"<"{space}(?i:correct){space}">"
correct_close			"<""/"{space}(?i:correct){space}">"

choice					{choice_open}{string}{choice_close}
correct					{correct_open}{string}{correct_close}

%%

{quiz_open}				{	printf("L: quiz opened\n");
							return Quiz_Open; }

{quiz_close}			{   printf("L:quiz closed\n");
							return Quiz_Close; }

{single_open}			{	printf("L: SCQ opened\n");
							yylval.num = getMarks(yytext);
							return Single_Open; }				

{multi_open}			{	printf("L: MCQ opened\n");
							yylval.num = getMarks(yytext);
							return Multi_Open; }		

{choice}				{	printf("L: Choice found\n");
							return Choice; }

{correct}				{	printf("L: Correct found\n");
							return Correct; }								

{single_close}			{	printf("L: SCQ closed\n");
							return Single_Close; }							

{multi_close}			{	printf("L: MCQ closed\n");
							return Multi_Close; }								

{string}				{	/*printf("L: Question found: %s\n",yytext);*/
							return Question; }

[. \t\n]					{	/*printf("special char: %d at line: %d\n",yytext[0],yylineno)*/ ;}

%%

int getMarks(char* s){
	//printf(" String input to getMarks: %s\n",s);
	int i=0;
	int left = 0,right = 0;
	while(1){
		if(s[i]==34){
			if(left){
				right = i-1;
				break;
			}
			else left = i+1;
		}
		i++;
	}
	int N = right-left+2;
	char str[N];
	for(int i=0; i< N; i++){
		str[i] = s[left + i];
	}
	str[N-1]='\0';
	//printf("Returning yylval: %d\n",atoi(str));
	return atoi(str);
}

