
/*The is the lex file for specifying the patterns*/

%{ 
	#include <stdio.h>
	#include "scan.h"
%}

%option nounput yylineno
keywords						 ?i:"ARRAY"|"BEGIN"|"BOOLEAN"|"COMMENT"|"CONTINUE"|"DO"|"DOUBLE"|"ELSE"|"END"|"FALSE"|"FOR"|"IF"|"INTEGER"|"LABEL"|"LIST"|"LONG"|"OWN"|"PROCEDURE"|"STEP"|"SWITCH"|"THEN"|"TRUE"|"UNTIL"|"VALUE"|"WHILE"
Operators						 ?i:"AND"|"OR"|"LEQ"|"LT"|"GEQ"|"GT"|"NOT"|"EQL"|"NEQ"|":="|"+"|"-"|"*"|"/"|"%"|"^"|"|"|"&"|"<<"|">>"|"<="|">="
integer                          ([1-9][0-9]*)|"0"
%%

{keywords}						return KEYWORD;
{Operators}						return OPERATOR;

[a-zA-Z][a-zA-Z0-9]*			return IDENTIFIER;


"'"[^'"]*"'"					return STRING;
["][^"']*["]					return STRING;
(["][^'"]*)|(['][^'"]*)  		return STR_ERROR;
["][^"]*['][^"]*["]				return STR_ERROR;
['][^']*["][^']*[']             return STR_ERROR;


";"								return DELIMITER;
","								return DELIMITER;
"("								return DELIMITER;
")"								return DELIMITER;
":"								return DELIMITER;
"{"								return DELIMITER;
"}"								return DELIMITER;
"["								return DELIMITER;
"]"								return DELIMITER;

{integer}		 				return INTEGER;

([0][xX]"0")|("0"(?i:X)[1-9a-fA-F][A-Fa-f0-9]*)			return HEXADECIMAL;

{integer}"."[0-9]{1,6}         return FLOATING_POINT;

[{][^}]*[}]						return COMMENT;
[ \t\n\f\r] 					;
.								return ERROR;
						
	 		
%%

int yywrap(){
	return 1;
}
