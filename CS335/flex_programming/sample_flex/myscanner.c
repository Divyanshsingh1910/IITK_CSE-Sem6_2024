
#include <stdio.h>
#include "myscanner.h"

extern int yylex();
extern int yylineno;
extern char* yytext;
/*These three things are defined in another module*/

char *names[] = {NULL, "NAME", "AGE", "COLLEGE","CPI"};
	/*		  1	 2	  3	  4	*/
int main() {

	int name_token,value_token;

	name_token = yylex();

	while(name_token){
		printf("%d\n",name_token);
		
		if(yylex() != COLON ){
			printf("Syntax error in line %d, Expected a ':' but found %s\n",yylineno,yytext);
			return 1;	
		}

		value_token = yylex();
		/*switch cases for diff type of name_token*/
		switch (name_token) {
		case NAME:
			if(value_token!= VALUE){
				printf("Syntax error in line %d, Expected a name but found %s\n",yylineno,yytext);
				return 1;
			}
			
			printf("%s is set to %s\n",names[name_token],yytext);
			break;

		case AGE:
		case CPI:
			if(value_token!= INTEGER){
				printf("Syntax error in line %d, Expected an integer but found %s\n",yylineno,yytext);
				return 1;
			}
			printf("%s is set to %s\n",names[name_token],yytext);
			break;
		case COLLEGE:
			if(value_token!= STRING){
				printf("Syntax error in line %d, Expected a college_name but found %s\n",yylineno,yytext);
				return 1;
			}
			printf("%s is set to %s\n",names[name_token],yytext);
			break;
		default:
			printf("Syntax error in line no: %d, unexpected entry found\n",yylineno);
			return 1;
		}
		
		//Getting next line name_token
		name_token = yylex();

	}
	printf("This line is triggered\n");

	return 0;
}
