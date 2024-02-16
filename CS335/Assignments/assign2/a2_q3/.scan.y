/* 3rd part of the assgn2 */

%{
void yyerror (char *s);
int yylex();

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*Any global variable declaration if needed*/
int num_qs = 0;
int singles_qs = 0;
int multis_qs = 0;
int ans_choices = 0;
int corr_choice = 0;
int total_marks = 0;

int markwiseQuestions[8];
void Increment_mw_Qs(int marks);

extern int yylex();
extern int yylineno;
extern char* yytext;

void print_info()
{

	printf("#Number: %d\n",num_qs);
	printf("#Single_qs: %d\n",singles_qs);
	printf("#Multi_qs: %d\n",multis_qs);
	printf("#Ans_choice: %d\n",ans_choices);
	printf("#Correct_choice: %d\n",corr_choice);
	printf("#Total_marks: %d\n",total_marks);

	for(int i=0; i<8; i++){
		printf("#Question of %d marks: %d\n",i+1,markwiseQuestions[i]);
	}
	return;
}

%}

%union {int num; char id;}

%token Quiz
%token SinglesQs
%token MultisQs
%token Marks
%token <num> Mark_val
%token Choice
%token Correct
%token String_literal
/*
%token Open_br
%token Close_br
%token Fwd_slash
*/
%start QUIZ

%%

/* grammar specifications with corresponding semantic actions */


QUIZ : '<' Quiz '>' QUESTIONS '<' '/' Quiz '>'			{printf("First production\n");}
		;

QUESTIONS : %empty											{printf("Empty production\n");}									
			| QUESTION QUESTIONS								{;}
			;

QUESTION : SINGLE_CHOICE							{singles_qs++; printf("SQS:%d\n",singles_qs);}
			| MULTI_CHOICE							{multis_qs++; printf("MQS:%d\n",multis_qs);}
			;

/* Single choice qs specifications */
SINGLE_CHOICE : SC_OPEN String_literal ENTRIES SC_CLOSE			{;}
				;
SC_OPEN : '<' SinglesQs Marks '=' Mark_val '>'		{Increment_mw_Qs($5);}
			;
SC_CLOSE : '<' '/' SinglesQs '>'						{;}	


/* Muliple choice questons specifications */
MULTI_CHOICE : MC_OPEN String_literal ENTRIES MC_CLOSE	{;}
				;
MC_OPEN : '<' MultisQs Marks '=' Mark_val '>'		{Increment_mw_Qs($5);}
			;
MC_CLOSE : '<' '/' MultisQs '>'						{;}


/* Question content's specifications */
ENTRIES :	
			 /* empty */									
			| CHOICES ENTRIES									{;}
			| CORRECTS ENTRIES							{;}
			;

CHOICES : 
			 /* empty */							
			|CHOICE CHOICES								{;}
			;

CORRECTS : 
			/* empty */										
			| CORRECT CORRECTS									{;}

CHOICE : '<' Choice '>' String_literal '<' '/' Choice '>'	{ans_choices++;}
CORRECT : '<' Correct '>' String_literal '<' '/' Correct '>' {corr_choice++;}

%%

		/* C code */
void Increment_mw_Qs(int marks){
	markwiseQuestions[marks]++;
	return ;
}


int main(){
	/*initializing markwise questions*/
	for(int i=0; i<8; i++){
		markwiseQuestions[i]=0;
	}

	printf("Parsing started...\n");
	yyparse();
	int temp = 1;

	printf("Found %s at line %d\n",yytext,yylineno);
	printf("Returned from yyparse here\n"); return 0;
	while(temp==0){
		temp = yyparse();
	}

	if(temp!=0){
		printf("Found %s at line %d\n",yytext,yylineno);
	}
	
	print_info();
	return 0;
}

void yyerror (char *s) {fprintf (stderr, "%s\n",s);}
