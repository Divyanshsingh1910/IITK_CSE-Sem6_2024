
%{
	#include "parser.tab.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <ctype.h>
	void yyerror (char *s);
	int yylex();
	int getMarks(char *s);
	int get_line(char *s);
%}

%option nounput yylineno noyywrap

quiz_open				"<"(?i:quiz)">"
quiz_close              "<""/"(?i:quiz)">"

question				[^\<\>]*
Digit					[0-9]
space					[ \t]*
marks_part				{space}(?i:marks){space}"="{space}["]{space}{Digit}+{space}["]{space}

single_open				"<"{space}(?i:singleselect){marks_part}">"{question}
single_close			"<""/"{space}(?i:singleselect){space}">"

multi_open				"<"{space}(?i:multiselect){marks_part}">"{question}
multi_close				"<""/"{space}(?i:multiselect){space}">"

choice_open				"<"{space}(?i:choice){space}">"
choice_close			"<""/"{space}(?i:choice){space}">"

correct_open			"<"{space}(?i:correct){space}">"
correct_close			"<""/"{space}(?i:correct){space}">"

choice					{choice_open}{question}{choice_close}
correct					{correct_open}{question}{correct_close}

/*	Error handling patterns goes here	*/

/*	Quotes around Marks value	*/
marks_err1				{space}(?i:marks){space}"="{space}{Digit}+{space}
marks_err2				{space}(?i:marks){space}"="{space}["]{Digit}+{space}
marks_err3				{space}(?i:marks){space}"="{space}{Digit}+["]{space}
marks_err4				{space}(?i:marks){space}"="{space}
marks_err5				{space}(?i:marks){space}"="{space}["]["]{space}
marks_err				{marks_err1}|{marks_err2}|{marks_err3}|{marks_err4}|{marks_err5}

question_err			"<"{space}(?i:("singleselect"|"multiselect")){marks_err}">"{question}

/*	Multiline tags	*/
multi_line_tag			"<"[^\<\>\n]*[\n][^\<\>]*">"

/*	Close tag is missing	*/
choice_close_missing	{choice_open}[^\<\>]*
correct_close_missing	{correct_open}[^\<\>]*

single_close_missing	{single_open}({choice}|{correct}|{space}|[\n])*({multi_open}|{multi_close}|{single_open}|{quiz_close})

%%

{quiz_open}				{	printf("L: quiz opened\n");
							return Quiz_Open; }

{quiz_close}			{   printf("L:quiz closed\n");
							return Quiz_Close; }

{question_err}				{	printf("Incorrect quotes around marks in line no: %d",yylineno);
							return -1; }

{single_open}			{	printf("L: SCQ opened\n");
							yylval.num = getMarks(yytext);
							if(yylval.num<1 || yylval.num>2){
								printf("Marks bound error in singleselect in line no: %d\n",yylineno-get_line(yytext));
								return -1;
							}
							return Single_Open; }				

{multi_open}			{	printf("L: MCQ opened\n");
							yylval.num = getMarks(yytext);
							if(yylval.num<2 || yylval.num>8){
								printf("Marks bound error in Multiselect in line no: %d\n",yylineno-get_line(yytext));
								return -1;
							}
							return Multi_Open; }		

{choice}				{	printf("L: Choice found\n");
							return Choice; }

{correct}				{	printf("L: Correct found\n");
							return Correct; }								

{single_close}			{	printf("L: SCQ closed\n");
							return Single_Close; }							

{multi_close}			{	printf("L: MCQ closed\n");
							return Multi_Close; }								

{multi_line_tag}		{	printf("Invalid tag at line no: %d\n",yylineno - get_line(yytext));
							printf("Hint: Multiline tag encountered!\n");
							return -1; }

{single}

{choice_close_missing}	{	printf("Close tag of <choice> is missing, error line no: %d\n",yylineno-get_line(yytext)); 
								return -1; }

{correct_close_missing} {	printf("Close tag of <correct> is missing, error line no: %d\n",yylineno-get_line(yytext));
								return -1; }


[. \t\n]					{	/*printf("special char: %d at line: %d\n",yytext[0],yylineno)*/ ;}

%%
/*

{question}				{	printf("L: Question found: %s\n",yytext);
							return Question; }
*/
int get_line(char *s){
	int count = 0;
	for(int i=0; i<strlen(s); i++){
		if(s[i]=='\n'){
			count++;
		}
	}

	return count;
}

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

