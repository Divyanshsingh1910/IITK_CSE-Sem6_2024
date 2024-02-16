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

int temp_choice_count = 0;
int temp_correct_count = 0;

%}

%code requires {
	struct meta_data{
		int mark_val;
		int line_num;
	};
}
%union { struct meta_data get;}

%token Quiz_Open
%token Quiz_Close

%token <get> Single_Open
%token <get> Single_Close

%token <get> Multi_Open
%token <get> Multi_Close

/*%token Question*/
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

SCQ : Single_Open E Single_Close			{	
													/*	printf("Current at line:%d\n",yylineno);
														printf("#Choice: %d\n",temp_choice_count);
														printf("#Correct: %d\n", temp_correct_count);
														*/
												num_qs++;
												singles_qs++;
												Increment_mw_Qs(($1).mark_val);
												total_marks+= ($1).mark_val;

												if(temp_choice_count<3 || temp_choice_count>4){
													printf("Choice count error for Singleselect at line no: %d\n",($1).line_num);
													return 0;
												}
												if(temp_correct_count!=1){

												printf("Correct count more than 1 for Singleselect ending at line no: %d\n",($1).line_num);
												return 0;
												}
												temp_choice_count = 0;
												temp_correct_count = 0;
												}
	;

SCQ : Single_Open E Multi_Open E Single_Close		{	printf("Stray Open <multiselect> tag at lineno: %d\n",($3).line_num); return 0; }
	| Single_Open E Multi_Close E Single_Close		{	printf("Stray </multiselect> tag at lineno: %d\n",($3).line_num); return 0; }
	| Multi_Open E Single_Open E Single_Close		{ 	printf(" Missing Closing tag for <multiselect> at lineno: %d\n",($1).line_num); return 0; }
	| Single_Open E Single_Open E Single_Close		{ 	printf(" Missing Closing tag for <singleselect> at lineno: %d\n",($1).line_num); return 0; }
	| E Single_Close								{	printf(" Missing opening tag of <singleselec> at lineno: %d\n",($2).line_num); return 0;}
	;


MCQ : Multi_Open E Multi_Close				{			/*
														printf("Current at line:%d\n",yylineno);
														printf("#Choice: %d\n",temp_choice_count);
														printf("#Correct: %d\n", temp_correct_count);
														*/
														num_qs++;
														multis_qs++;
														Increment_mw_Qs(($1).mark_val);
														total_marks+= ($1).mark_val;

												if(temp_choice_count<3 || temp_choice_count>4){
													printf("Choice count error for Multiselect ending at line no: %d\n",($1).line_num);
													return 0;
												}

												if(temp_correct_count > temp_choice_count){
													printf("Too many correct choices for Multiselect ending at line no: %d\n",($1).line_num);
												}
														temp_choice_count = 0;
														temp_correct_count = 0;
														}
	;


MCQ : Multi_Open E Single_Open E Multi_Close		{	printf("Stray Open <singleselect> tag at lineno: %d\n",($3).line_num); return 0; }
	| Multi_Open E Single_Close E Multi_Close		{	printf("Stray </singleselect> tag at lineno: %d\n",($3).line_num); return 0; }
	| Single_Open E Multi_Open E Multi_Close		{ 	printf(" Missing Closing tag for <singleselect> at lineno: %d\n",($1).line_num); return 0; }
	| Multi_Open E Multi_Open E Multi_Close			{ 	printf(" Missing Closing tag for <multiselect> at lineno: %d\n",($1).line_num); return 0; }
	| E	Multi_Close									{	printf(" Missing opening tag for <multiselect> at lineno: %d\n",($2).line_num); return 0;}
	;

E : Choice E										{temp_choice_count++; ans_choices++;}
	| Correct E										{temp_correct_count++; corr_choice++;}
	;

E : %empty											{ /*printf(" Empty production here\n")*/;}

%%


void Increment_mw_Qs(int marks){
	markwiseQuestions[marks-1]++;
	return ;
}

void print_info()
{

	printf("Number of questions: %d\n",num_qs);
	printf("Number of singleselect questions: %d\n",singles_qs);
	printf("Number of multiselect questions: %d\n",multis_qs);
	printf("Number of answer choices: %d\n",ans_choices);
	printf("Number of correct answers: %d\n",corr_choice);
	printf("Total marks: %d\n",total_marks);

	for(int i=0; i<8; i++){
		switch(i){
			case 0:
				printf("Number of %d mark questions: %d\n",i+1,markwiseQuestions[i]);

			default:
				printf("Number of %d marks questions: %d\n",i+1,markwiseQuestions[i]);
		}
	}
	return;
}

void yyerror(char* s){
//	fprintf(stderr, "In line no: %d found %s\n",yylineno,s);
//	return ;
}

int main(){

	/*initializing markwise questions*/
	for(int i=0; i<8; i++){
		markwiseQuestions[i]=0;
	}

	/*printf("Parsing started...\n");*/
	yyparse();
	print_info();
	/*printf("Found %s at line %d\n",yytext,yylineno);
	printf("Returned from yyparse here\n");*/
	return 0;
}


