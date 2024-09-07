%{
#include <bits/stdc++.h>
using namespace std;
#define TAB_FACTOR	2
#include "python.tab.h"
bool verbose_lexer = false;
bool ind_de = true;
queue<int> tokenq;
stack<int> stk;
int current_indentation=0;
map<char,int > mp1;
map<int,char > mp2;

extern struct terminal_node* new_terminal_node(int lineno, string type, string lexeme);

void init_stack(){
	stk.push(0);
	mp1['%']=-21;mp1['&']=-22;mp1['(']=-23;mp1[')']=-24;mp1['*']=-25;mp1['/']=-6;mp1['+']=-7;mp1['-']=-8;mp1['~']=-9;mp1[',']=-10;
	mp1[':']=-11;mp1[';']=-12;mp1['<']=-13;mp1['=']=-14;mp1['>']=-15;mp1['[']=-16;mp1[']']=-17;mp1['^']=-18;mp1['|']=-19;mp1['.']=-20;

	mp2[-21]='%';mp2[-22]='&';mp2[-23]='(';mp2[-24]=')';mp2[-25]='*';mp2[-6]='/';mp2[-7]='+';mp2[-8]='-';mp2[-9]='~';mp2[-10]=',';
	mp2[-11]=':';mp2[-12]=';';mp2[-13]='<';mp2[-14]='=';mp2[-15]='>';mp2[-16]='[';mp2[-17]=']';mp2[-18]='^';mp2[-19]='|';mp2[-20]='.';
}
int find_indentation (char *str){
	int c=0,i;
	for(i=0;i<strlen(str);i++){
		if(str[i]==' '){
			c++;
		}
		else if(str[i] == '\t'){
			c=c-c%TAB_FACTOR;
			c=c+TAB_FACTOR;
		}
		else{
			break;
		}
	}
	return c;
}

int check(){
	int c = find_indentation(yytext);
	if(c%TAB_FACTOR){
		cout << "INDENTATION NOT RIGHT at line no. " << yylineno << endl;
		return -1;
	}
	if(c>stk.top()){
		stk.push(c);
		tokenq.push(INDENT);
	}
	while(stk.top()>c){
		stk.pop();
		tokenq.push(DEDENT);
	}
	if(c!=stk.top()){
		cout << "INDENTATION NOT CORRECT while DEDENTING at line no. " << yylineno << endl;
		return -1;
	}
	current_indentation = c;
	return 0;
}

int check2(int token){
	int c = current_indentation;
	while(stk.top()>c){
		stk.pop();
		tokenq.push(DEDENT);
	}
	if(c!=stk.top()){
		cout << "INDENTATION NOT CORRECT while DEDENTING at line no. " << yylineno << endl;
		return -1;
	}
	tokenq.push(token);
	token = tokenq.front();
	tokenq.pop();
	if(mp2.find(token)!=mp2.end()){
		return mp2[token];
	}
	return token;
}

int check3(char character){
	int c = current_indentation;
	while(stk.top()>c){
		stk.pop();
		tokenq.push(DEDENT);
	}
	if(c!=stk.top()){
		cout << "INDENTATION NOT CORRECT while DEDENTING at line no. " << yylineno << endl;
		return -1;
	}
	int token = mp1[character];
	tokenq.push(token);
	token = tokenq.front();
	tokenq.pop();
	//cout << "checked2" << endl;
	return token;
}
%}

%option noyywrap
%option yylineno


dunder					"__main__"|"__name__"|"__init__"

underscore				 "_"
letter 					[a-zA-Z]
digit					[0-9]
nonzerodigit			[1-9]
name					({letter}|{underscore})({letter}|{underscore}|{digit})*

/*Integer coming*/
hexdigit				{digit}|[a-f]|[A-F]
octdigit				[0-7]
bindigit				"0"|"1"
hexinteger				"0"("x"|"X")({underscore}?{hexdigit})+
octinteger				"0"("o"|"O")({underscore}?{octdigit})+
bininteger				"0"("b"|"B")({underscore}?{bindigit})+
decinteger				({nonzerodigit}({underscore}?{digit})*)|"0"+({underscore}?"0")*
integer					{decinteger}|{bininteger}|{hexinteger}|{octinteger}

/*Floating point coming*/
digitpart				{digit}({underscore}?{digit})*
exponent				("e"|"E")(("+"|"-")?){digitpart}
fraction 				"."{digitpart}
pointfloat				({digitpart}?{fraction})|({digitpart}".")
exponentfloat				({digitpart}|{pointfloat}){exponent}
floatnumber				{pointfloat}|{exponentfloat}

/*String Literal coming*/
stringescapeseq				\\(.|\n)
longstringchar				[^\\]
shortstringchar				[^\\\n"']
longstringitem				{longstringchar}|{stringescapeseq}
shortstringitem				{shortstringchar}|{stringescapeseq}
longstring				(\'\'\'{longstringitem}*\'\'\')|(\"\"\"{longstringitem}*\"\"\")
shortstring				(\'{shortstringitem}*\')|(\"{shortstringitem}*\")
stringprefix				"r"|"u"|"R"|"U"|"f"|"F"|"fr"|"Fr"|"fR"|"FR"|"rf"|"rF"|"Rf"|"RF"
string					{stringprefix}?({shortstring}|{longstring})

/*Comment specification is here*/
whitespace				^([ ]|[\t])*
comment_line_eof		^[ \t]*"#"(.)*/[<<EOF>>]
comment_line 			^[ \t]*"#"(.)*\n
comment					"#"(.)*
black_line_eof			^[ \t]*/[<<EOF>>]
black_line              ^[ \t]*\n


%%	
	if(!tokenq.empty()){
		int token = tokenq.front();
		tokenq.pop();
		if(mp2.find(token)!=mp2.end()){
			return mp2[token];
		}
		return token;
	}


{comment_line_eof}					{
									if(verbose_lexer)
										cout << "COMMENT FOUND: " << yytext << "\n";
								}
{black_line_eof}				{
									if(verbose_lexer)
										cout << "BLANK LINE found\n";	
								}
{whitespace}				{
									if(verbose_lexer)
										cout << "STARTING WHITESPACE found\n";
									int token = check();
									if(token==-1){
										return -1;
									}
									else if(tokenq.size()>0){
										token = tokenq.front();
										tokenq.pop();
										if(mp2.find(token)!=mp2.end()){
											return mp2[token];
										}
										/*if(ind_de)
											if(token==INDENT) cout << "--------INDENT FOUND-----------" << endl;
											if(token==DEDENT) cout << "--------DEDENT FOUND-----------" << endl;*/
										return token;
									}
								}
{comment_line}					{
									current_indentation = 0;
									if(verbose_lexer)
										cout << "COMMENT FOUND: " << yytext << "\n";
								}
{comment}						{
									if(verbose_lexer)
										cout << "COMMENT FOUND: " << yytext << "\n";
								}
{black_line}					{
									current_indentation = 0;
									if(verbose_lexer)
										cout << "BLANK LINE found\n";	
								}
\n								{
									current_indentation = 0;
									if(verbose_lexer)
										cout << "NEWLINE LINE found\n";	
									return NEWLINE;
								}

"%"|"&"|"("|")"|"*"|"/"|"+"|"-"|"~"|","|":"|";"|"<"|"="|">"|"["|"]"|"^"|"|"|"."	{
																				if(verbose_lexer)
																					printf("SINGLE CHARACTER TOKEN found: %c\n", yytext[0]);
																				int token = check3(yytext[0]);
																				if(mp2.find(token)!=mp2.end()){
																					return mp2[token];
																				}
																				return token;
																			}	

"&="							{
									if(verbose_lexer)
										cout << "AMPEREQUAL found\n";
									return check2(AMPEREQUAL);
								}
"^="							{
									if(verbose_lexer)
										cout << "CIRCUMFLEXEQUAL found\n";
									return check2(CIRCUMFLEXEQUAL);
								}	
"//"							{
									if(verbose_lexer)
										cout << "DOUBLESLASH found\n";
									return check2(DOUBLESLASH);
								}
"//="							{	
									if(verbose_lexer)
										cout << "DOUBLESLASHEQUAL found\n";
									return check2(DOUBLESLASHEQUAL);
								}	
"**"							{
									if(verbose_lexer)
										cout << "DOUBLESTAR found\n";
									return check2(DOUBLESTAR);
								}
"**="							{
									if(verbose_lexer)
										cout << "DOUBLESTAREQUAL found\n";
									return check2(DOUBLESTAREQUAL);
								}
"=="							{
									if(verbose_lexer)
										cout << "EQEQUAL found\n";
									return check2(EQEQUAL);
								}
">="							{
									if(verbose_lexer)
										cout << "GREATEREQUAL found\n";
									return check2(GREATEREQUAL);
								}
"<<"							{
									if(verbose_lexer)
										cout << "LEFTSHIFT found\n";
									return check2(LEFTSHIFT);
								}
"<<="							{	
									if(verbose_lexer)
										cout << "LEFTSHIFTEQUAL found\n";
									return check2(LEFTSHIFTEQUAL);
								}	
"<="							{
									if(verbose_lexer)
										cout << "LESSEQUAL found\n";
									return check2(LESSEQUAL);
								}
"-="							{
									if(verbose_lexer)
										cout << "MINEQUAL found\n";
									return check2(MINEQUAL);
								}
"!="							{	
									if(verbose_lexer)
										cout << "NOTEQUAL found\n";
									return check2(NOTEQUAL);
								}	
"%="							{
									if(verbose_lexer)
										cout << "PERCENTEQUAL found\n";
									return check2(PERCENTEQUAL);
								}
"+="							{
									if(verbose_lexer)
										cout << "PLUSEQUAL found\n";
									return check2(PLUSEQUAL);
								}
"->"							{
									if(verbose_lexer)
										cout << "RARROW found\n";
									return check2(RARROW);
								}
">>="							{
									if(verbose_lexer)
										cout << "RIGHTSHIFTEQUAL found\n";
									return check2(RIGHTSHIFTEQUAL);
								}
">>"							{	
									if(verbose_lexer)
										cout << "RIGHTSHIFT found\n";
									return check2(RIGHTSHIFT);
								}	
"/="							{
									if(verbose_lexer)
										cout << "SLASHEQUAL found\n";
									return check2(SLASHEQUAL);
								}
"*="							{
									if(verbose_lexer)
										cout << "STAREQUAL found\n";
									return check2(STAREQUAL);
								}
"|="							{
									if(verbose_lexer)
										cout << "VBAREQUAL found\n";
									return check2(VBAREQUAL);
								}
"break"							{
									if(verbose_lexer)
										cout << "BREAK found\n";
									return check2(BREAK);
								}
"continue"						{
									if(verbose_lexer)
										cout << "CONTINUE found\n";
									return check2(CONTINUE);
								}
"class"							{
									if(verbose_lexer)
										cout << "CLASS found\n";
									return check2(CLASS);
								}
"def"							{	
									if(verbose_lexer)
										cout << "DEF found\n";
									return check2(DEF);
								}
"elif"							{
									if(verbose_lexer)
										cout << "ELIF found\n";
									return check2(ELIF);
								}
<<EOF>>							{	
									if(verbose_lexer)
										cout << "ENDMARKER found\n";
									current_indentation=0;
									return check2(ENDMARKER);
								}
"False"							{
									if(verbose_lexer)
										cout << "FALSE found\n";
									return check2(FALSE);
								}
"for"							{
									if(verbose_lexer)
										cout << "FOR found\n";
									return check2(FOR);
								}
"global"						{
									if(verbose_lexer)
										cout << "GLOBAL found\n";
									return check2(GLOBAL);
								}
"if"							{
									if(verbose_lexer)
										cout << "IF found\n";
									return check2(IF);
								}
"in"							{
									if(verbose_lexer)
										cout << "IN found\n";
									return check2(IN);
								}
"is"							{
									if(verbose_lexer)
										cout << "IS found\n";
									return check2(IS);
								}
"None"							{
									if(verbose_lexer)
										cout << "NONE found\n";
									return check2(NONE);
								}

"int"							{
									if(verbose_lexer)
										cout << "INT found\n";
									return check2(INT);
								}

"float"							{
									if(verbose_lexer)
										cout << "FLOAT found\n";
									return check2(FLOAT);
								}

"str"							{
									if(verbose_lexer)
										cout << "STR found\n";
									return check2(STR);
								}

"bool"							{
									if(verbose_lexer)
										cout << "BOOL found\n";
									return check2(BOOL);
								}

"list"							{
									if(verbose_lexer)
										cout << "LIST found\n";
									return check2(LIST);
								}

"and"							{
									if(verbose_lexer)
										cout << "AND found\n";
									return check2(AND);
								}

"or"							{
									if(verbose_lexer)
										cout << "OR found\n";
									return check2(OR);
								}
	
"not"							{
									if(verbose_lexer)
										cout << "NOT found\n";
									return check2(NOT);
								}
"return"						{
									if(verbose_lexer)
										cout << "RETURN found\n";
									return check2(RETURN);
								}
"True"							{
									if(verbose_lexer)
										cout << "TRUE found\n";
									return check2(TRUE);
								}
"while"							{
									if(verbose_lexer)
										cout << "WHILE found\n";
									return check2(WHILE);
								}
"else"							{
									if(verbose_lexer)
										cout << "ELSE found\n";
									yylval.terminal_node_info = new_terminal_node(yylineno, "ELSE", string(yytext));
									return check2(ELSE);
								}
{integer}						{
									if(verbose_lexer)
										printf("INTEGER found\n");
									yylval.terminal_node_info = new_terminal_node(yylineno, "INTEGER", string(yytext));
									return check2(INTEGER);
								}
{floatnumber}						{
									if(verbose_lexer)
										printf("FLOAT found\n");
									yylval.terminal_node_info = new_terminal_node(yylineno, "FLOAT", string(yytext));
									return check2(FLOAT_NUMBER);
								}
{dunder}						{
									if(verbose_lexer)
										cout << "DUNDER FOUND: " << yytext << "\n";
									yylval.terminal_node_info = new_terminal_node(yylineno, "STR", string(yytext));
									return check2(DUNDER);
								}
{name}							{
									if(verbose_lexer)
										cout << "NAME FOUND: " << yytext << "\n";
									yylval.terminal_node_info = new_terminal_node(yylineno, "NAME", string(yytext));
									return check2(NAME);
								}
{string}						{
									if(verbose_lexer)
										cout << "STRING FOUND: " << yytext << "\n";
									yylval.terminal_node_info = new_terminal_node(yylineno, "STR", string(yytext));
									return check2(STRING);
								}
[ \t]							{	
									if(verbose_lexer)
										cout << "SPACE/TAB FOUND: " << yytext << "\n";
								}
.								{
									if(verbose_lexer)
										cout << "Unidentified char found: " << yytext << "\n";
									return -1;
								}
%%
