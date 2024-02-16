
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

quiz_open				"<"{space}(?i:quiz){space}">"
quiz_close              "<"{space}"/"{space}(?i:quiz){space}">"

question				[^\<\>]*
Digit					[0-9]
space					[ \n\t]*
marks_part				{space}(?i:marks){space}"="{space}["]{space}{Digit}+{space}["]{space}

single_open				"<"{space}(?i:singleselect){marks_part}">"{question}
single_close			"<"{space}"/"{space}(?i:singleselect){space}">"

multi_open				"<"{space}(?i:multiselect){marks_part}">"{question}
multi_close				"<"{space}"/"{space}(?i:multiselect){space}">"

choice_open				"<"{space}(?i:choice){space}">"
choice_close			"<"{space}"/"{space}(?i:choice){space}">"

correct_open			"<"{space}(?i:correct){space}">"
correct_close			"<"{space}"/"{space}(?i:correct){space}">"

choice					{choice_open}{question}{choice_close}
correct					{correct_open}{question}{correct_close}

/*	Error handling patterns goes here	*/

/*	Quotes around Marks value	*/
marks_err1				{space}(?i:marks){space}"="{space}{Digit}+{space}
marks_err2				{space}(?i:marks){space}"="{space}["]{space}{Digit}+{space}
marks_err3				{space}(?i:marks){space}"="{space}{Digit}+{space}["]{space}
marks_err4				{space}(?i:marks){space}"="{space}
marks_err5				{space}(?i:marks){space}"="{space}["]{space}["]{space}
marks_err				{marks_err1}|{marks_err2}|{marks_err3}|{marks_err4}|{marks_err5}

question_err			"<"{space}(?i:("singleselect"|"multiselect")){marks_err}">"{question}

/*	Multiline tags	*/
/*multi_line_tag			"<"[^\<\>\n]*[\n][^\<\>]*">"*/

/*	Close tag is missing	*/
choice_close_missing	{choice_open}[^\<\>]*
correct_close_missing	{correct_open}[^\<\>]*

single_close_missing	{single_open}({choice}|{correct}|{space}|[\n])*({multi_open}|{multi_close}|{single_open}|{quiz_close})

last_scq				[ \t]*{single_open}[ \t\n]*({choice}|{correct}|[ \t\n])*[ \t\n]*{quiz_close}

last_mcq				[ \t]*{multi_open}[ \t\n]*({choice}|{correct}|[ \t\n])*[ \t\n]*{quiz_close}

%%

{last_scq}				{	printf("Missing close tag for <singleselect> at lineno: %d\n",yylineno-get_line(yytext)); return 0;}

{last_mcq}				{	printf("Missing close tag for <multiselect> at lineno: %d\n",yylineno-get_line(yytext)); return 0;}

{quiz_open}				{	/*printf("L: quiz opened\n");*/
							return Quiz_Open; }

{quiz_close}			{   /*printf("L:quiz closed\n");*/
							return Quiz_Close; }

{question_err}				{	printf("Incorrect quotes around marks in line no: %d",yylineno-get_line(yytext));
							return -1; }

{single_open}			{	//printf("L: SCQ opened\n");
							int marks = getMarks(yytext);
							(yylval.get).mark_val = marks;
							(yylval.get).line_num = yylineno - get_line(yytext);
							if(marks<1 || marks>2){
								printf("Marks bound error in singleselect in line no: %d\n",yylineno-get_line(yytext));
								return -1;
							}
							return Single_Open; }				

{multi_open}			{	/*printf("L: MCQ opened\n");*/
							int marks = getMarks(yytext);
							(yylval.get).mark_val = marks;
							(yylval.get).line_num = yylineno - get_line(yytext);
							if(marks<2 || marks>8){
								printf("Marks bound error in Multiselect in line no: %d\n",yylineno-get_line(yytext));
								return -1;
							}
							return Multi_Open; }		

{choice}				{	/*printf("L: Choice found\n");*/
							return Choice; }

{correct}				{	/*printf("L: Correct found\n");*/
							return Correct; }								

{single_close}			{	/*printf("L: SCQ closed\n");*/
							(yylval.get).line_num = yylineno - get_line(yytext);
							return Single_Close; }							

{multi_close}			{	/*printf("L: MCQ closed\n");*/
							(yylval.get).line_num = yylineno - get_line(yytext);
							return Multi_Close; }								


{choice_close_missing}	{	printf("Close tag of <choice> is missing, error line no: %d\n",yylineno-get_line(yytext)); 
								return -1; }

{correct_close_missing} {	printf("Close tag of <correct> is missing, error line no: %d\n",yylineno-get_line(yytext));
								return -1; }

{choice_close}			{	printf("Stray close tag </choice> found at line no: %d\n",yylineno);
								return -1; }
{correct_close}			{	printf("Stray close tag </correct> found at line no: %d\n",yylineno);
								return -1;}
["\=0-9a-zA-Z\#\:\<\>\?\! .\t\n]					{	/*printf("special char: %s at line: %d\n",yytext,yylineno)*/ ;}

%%
/*


{multi_line_tag}		{	printf("Invalid tag at line no: %d\n",yylineno - get_line(yytext));
							printf("Hint: Multiline tag encountered!\n");
							return -1; }
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

