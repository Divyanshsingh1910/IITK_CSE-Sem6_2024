
/*Including the header with the defined macros*/
%{
#include "myscanner.h"
%}

/*To fetch the lineno. correctly*/
%option nounput yylineno

%%

: 		return COLON; //separate the colon 
"NAME"		return NAME;  
"AGE"		return AGE;
"COLLEGE"	return COLLEGE;
"CPI" 		return CPI;

[A-Z][a-z]*	return VALUE; //typical name starting with a capital letter the small letters
[A-Z]*		return STRING; //college name with all the capital letters
[a-z]?[1-9][0-9]*	return INTEGER;
[ \t\n] 	;
.		printf("unexpected character\n");

%%

int yywrap(void)
{
	return 1;
}


