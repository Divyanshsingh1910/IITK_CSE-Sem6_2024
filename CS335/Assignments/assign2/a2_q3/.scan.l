/* lex file to process the input */

%{
	#include "scan.tab.h"
	void yyerror (char *s);
	 int yylex();
	int getMarks(char *s);
%}

%option nounput yylineno

%%

(?i:quiz)								{printf("lexer found %s\n",yytext); /*return Quiz;*/ }
(?i:singleselect)						{printf("lexer found %s\n",yytext); /*return SinglesQs;*/ }
(?i:multiselect)							{printf("lexer found %s\n",yytext); /*return MultisQs;*/ }
(?i:marks)								{printf("lexer found %s\n",yytext); /*return Marks;*/ }
(?i:choice)								{printf("lexer found %s\n",yytext); /*return Choice;*/ }
(?i:correct)								{printf("lexer found %s\n",yytext); /*return Correct;*/ }

["][ ]*[1-8][]*["]						{yylval.num = getMarks(yytext); printf("lexer found %s\n",yytext); /*return Mark_val;*/ }
[^<>]+/[\\n<]							{printf("lexer found %s\n",yytext); /*return String_literal;*/ }
"<"|">"|"/"									{printf("lexer found %s\n",yytext); /*return yytext[0];*/ }
.										;

%%

int getMarks(char* s){
	return 5;
}

int yywrap (void){ return 1;}
