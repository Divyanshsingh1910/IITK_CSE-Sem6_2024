/* Don't count the price you are paying */

%{

void yyerror(char *s);
int yylex();
extern char* yytext;
extern int yylineno;
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Global variables to maintain the counts */

int num_qs = 0;
int singles_qs = 0;
int multis_qs = 0;
int ans_choices = 0;
int corr_choice = 0;
int total_marks = 0;

int markwiseQuestions[8];
void Increment_mw_Qs(int marks);
void print_info();

/* Types of error to handle */
#define close_missing
#define open_missing
#define marks_not_quoted
#define marks_bound_error
#define less_choices
#define more_choices

%}

%union { int num; char* str;}

%token Quiz_Open
%token Quiz_Close

%token <num> Single_Open
%token Single_Close

%token <num> Multi_Open
%token Multi_Close

%token Question
%token Choice
%token Correct

%start line

%%

line : Quiz_Open QS Quiz_Close						{;}
	;

QS : SCQ QS											{;}
	| MCQ QS										{;}
	;

QS : %empty											{ /*printf(" Empty Production here\n")*/;}
	;

SCQ : Single_Open Question E Single_Close			{num_qs++;
														singles_qs++;
														Increment_mw_Qs($1);
														total_marks+= $1;}
	;

MCQ : Multi_Open Question E Multi_Close				{num_qs++;
														multis_qs++;
														Increment_mw_Qs($1);
														total_marks+= $1; }
	;

E : Choice E										{ans_choices++;}
	| Correct E										{corr_choice++;}
	;

E : %empty											{ /*printf(" Empty production here\n")*/;}

%%


void Increment_mw_Qs(int marks){
	markwiseQuestions[marks-1]++;
	return ;
}

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

#define close_missing
#define open_missing
#define marks_not_quoted
#define marks_bound_error
#define less_choices
#define more_choices

void yyerror(char* s){
	fprintf(stderr, "%s\n",s);
	return ;
}

int main(){

	/*initializing markwise questions*/
	for(int i=0; i<8; i++){
		markwiseQuestions[i]=0;
	}

	printf("Parsing started...\n");
	yyparse();
	print_info();
	printf("Found %d at line %d\n",yytext[0],yylineno);
	printf("Returned from yyparse here\n"); return 0;
}


