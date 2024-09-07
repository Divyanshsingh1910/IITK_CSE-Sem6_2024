%{

#include <bits/stdc++.h>
using namespace std;
#include "ast.h"
#include "symtable.h"
#include "x86.h"

int yylex();
void yyerror(char *s);
extern FILE *yyin;
extern char *yytext;
extern int yylineno;

extern void init_stack();

extern struct symbol_table_node* symbol_table_head;

bool verbose_parser = false;
bool verbose_parser_empty = false;
struct ast_node *root = NULL;
vector<struct ast_node *> children;

void generate_dot_file(struct ast_node* root, string output_file);

%}

%token  AMPEREQUAL AND BREAK CIRCUMFLEXEQUAL CLASS CONTINUE DEDENT DEF DOUBLESLASH DOUBLESLASHEQUAL DOUBLESTAR DOUBLESTAREQUAL ELIF ENDMARKER
		EQEQUAL FALSE FOR GREATEREQUAL GLOBAL IF IN INDENT IS LEFTSHIFT LEFTSHIFTEQUAL LESSEQUAL MINEQUAL NEWLINE NONE NOT NOTEQUAL OR PERCENTEQUAL 
		PLUSEQUAL RARROW RETURN RIGHTSHIFTEQUAL RIGHTSHIFT SLASHEQUAL STAREQUAL TRUE VBAREQUAL WHILE INT FLOAT STR BOOL LIST

%start	file_input

%union
{
	struct ast_node *ast_node_info;
	struct terminal_node *terminal_node_info;
}

%token<terminal_node_info>	  STRING
%token<terminal_node_info>    DUNDER
%token<terminal_node_info>    NAME
%token<terminal_node_info>    INTEGER
%token<terminal_node_info>    FLOAT_NUMBER
%token<terminal_node_info>    ELSE

%type<ast_node_info>		 file_input
%type<ast_node_info>		 stmt_OR_NEWLINE_seq	
%type<ast_node_info>		 COMMA_expr_seq	
%type<ast_node_info>		 stmt
%type<ast_node_info>		 funcdef
%type<ast_node_info>		 NAME_DUNDER
%type<ast_node_info>		 parameters
%type<ast_node_info>		 test
%type<ast_node_info>		 suite
%type<ast_node_info>		 arglist
%type<ast_node_info>		 argument
%type<ast_node_info>		 COMMA_argument_seq
%type<ast_node_info>		 simple_stmt
%type<ast_node_info>		 compound_stmt
%type<ast_node_info>		 small_stmt
%type<ast_node_info>		 SEMICOLON_small_stmt_seq
%type<ast_node_info>		 expr_stmt
%type<ast_node_info>		 flow_stmt
%type<ast_node_info>		 augassign_expr
%type<ast_node_info>		 testlist
%type<ast_node_info>		 assign_expr
%type<ast_node_info>		 COMMA_test_seq
%type<ast_node_info>		 return_stmt
%type<ast_node_info>		 if_stmt 
%type<ast_node_info>		 while_stmt
%type<ast_node_info>		 for_stmt
%type<ast_node_info>		 classdef
%type<ast_node_info>		 elif_seq
%type<ast_node_info>		 exprlist
%type<ast_node_info>		 stmt_seq
%type<ast_node_info>		 and_test
%type<ast_node_info>		 not_test
%type<ast_node_info>		 comparison
%type<ast_node_info>		 expr
%type<ast_node_info>		 xor_expr
%type<ast_node_info>		 and_expr
%type<ast_node_info>		 shift_expr
%type<ast_node_info>		 arith_expr
%type<ast_node_info>		 term
%type<ast_node_info>		 factor
%type<ast_node_info>		 power
%type<ast_node_info>		 atom_expr
%type<ast_node_info>		 atom
%type<ast_node_info>		 trailer

%%

/* file_input -> (stmt | NEWLINE)* ENDMARKER */
file_input 					: stmt_OR_NEWLINE_seq ENDMARKER						{		
																					// printf("Parsed successfully\n");
																					$$ = $1;
																					root = $$;
																					return -1;
																				}
							;

stmt_OR_NEWLINE_seq			: stmt_OR_NEWLINE_seq NEWLINE						{
																					if(verbose_parser)
																						printf("stmt_OR_NEWLINE_seq -> stmt_OR_NEWLINE_seq NEWLINE\n");  
																					$$ = $1;
																				}
							
							| stmt_OR_NEWLINE_seq stmt							{
																					if(verbose_parser)
																						printf("stmt_OR_NEWLINE_seq -> stmt_OR_NEWLINE_seq stmt\n");  
																					children = {$1,$2};
																					$$ = new_ast_node(LINE_IRRELEVANT,"","","stmt_OR_NEWLINE_seq",children);
																				}
							
							| %empty											{
																					if(verbose_parser_empty)
																						printf("stmt_OR_NEWLINE_seq -> empty\n"); 
																					$$ = NULL;
																				}
							;

stmt 						: simple_stmt 										{
																					if(verbose_parser)
																						printf("stmt -> simple_stmt\n");
																					$$ = $1;
																				}
							
							| compound_stmt										{
																					if(verbose_parser)
																						printf("stmt -> compound_stmt\n");
																					$$ = $1;
																				}
							;

/* simple_stmt -> small_stmt (';' small_stmt)* [';'] NEWLINE */
simple_stmt 				: small_stmt SEMICOLON_small_stmt_seq NEWLINE		{
																					if(verbose_parser) 
																						printf("simple_stmt -> small_stmt SEMICOLON_small_stmt_seq NEWLINE\n");
																					children = {$1,$2};
																					$$ = new_ast_node($1->line_num,"","","simple_stmt",children);
																				}
							
							| small_stmt SEMICOLON_small_stmt_seq ';' NEWLINE	{
																					if(verbose_parser)
																						printf("simple_stmt -> small_stmt SEMICOLON_small_stmt_seq ';' NEWLINE\n");
																					children = {$1,$2};
																					$$ = new_ast_node($1->line_num,"","","simple_stmt",children);
																				}
							;

SEMICOLON_small_stmt_seq	: SEMICOLON_small_stmt_seq ';' small_stmt			{
																					if(verbose_parser)
																						printf("SEMICOLON_small_stmt_seq -> SEMICOLON_small_stmt_seq ';' small_stmt\n");
																					children = {$1,$3};
																					$$ = new_ast_node($3->line_num,"","","SEMICOLON_small_stmt_seq",children);
																				}
							
							| %empty											{
																					if(verbose_parser_empty)
																						printf("SEMICOLON_small_stmt_seq -> empty\n");
																					$$ = NULL;
																				}
							;

small_stmt					: expr_stmt 										{
																					if(verbose_parser)
																						printf("small_stmt -> expr_stmt\n");
																					$$ = $1;
																				}
							
							| flow_stmt 										{
																					if(verbose_parser) 
																						printf("small_stmt -> flow_stmt\n");
																					$$ = $1;
																				}
							;

expr_stmt					: testlist ':' test									{	
																					if(verbose_parser)
																						printf("expr_stmt 1 found\n");
																					children = {$1,$3};
																					$$ = new_ast_node($3->line_num,"",":","expr_stmt",children);
																				}

							| testlist ':' test '=' test						{	
																					if(verbose_parser)
																						printf("expr_stmt 1 found\n");
																						/*
																						expr_stmt(=)
																							/		\
																						   :		test   <---- children
																						  /	\
																			testlist  test   <---- children_1       

																						*/
																					vector<struct ast_node *> children_1 = {$1,$3};
																					children = {new_ast_node($3->line_num,"",":","expr_stmt",children_1),$5};
																					$$ = new_ast_node($3->line_num,"","ASSIGNMENT OPERATOR: =","expr_stmt",children);
																				}

							| augassign_expr									{
																					if(verbose_parser)
																						printf("expr_stmt -> augassign_expr\n");
																					$$ = $1;
																				}
							
							| assign_expr										{
																					if(verbose_parser)
																						printf("expr_stmt -> assign_expr\n");
																					$$ = $1;
																				}
							;

augassign_expr				: testlist PLUSEQUAL testlist 						{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '+=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: +=", "", children);
																				}
							
							| testlist MINEQUAL testlist						{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '-=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: -=", "", children);
																				}
							
							| testlist STAREQUAL testlist 						{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '*=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: *=", "", children);
																				}
							
							| testlist SLASHEQUAL testlist						{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '/=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: /=", "", children);
																				}
							
							| testlist PERCENTEQUAL testlist					{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '%%=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: %=", "", children);
																				}
							
							| testlist AMPEREQUAL testlist						{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '&=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: &=", "", children);
																				}
							
							| testlist VBAREQUAL testlist						{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '|=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: |=", "", children);
																				}
							
							| testlist CIRCUMFLEXEQUAL testlist					{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '^=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: ^=", "", children);
																				}
							
							| testlist LEFTSHIFTEQUAL testlist					{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '<<=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: <<=", "", children);
																				}
							
							| testlist RIGHTSHIFTEQUAL testlist					{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '>>=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: >>=", "", children);
																				}
							
							| testlist DOUBLESTAREQUAL testlist 				{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '**=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: **=", "", children);
																				}
							
							| testlist DOUBLESLASHEQUAL testlist				{
																					if(verbose_parser)
																						printf("augassign_expr -> testlist '//=' testlist\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: //=", "", children);
																				}
							;

/* 
 *	assign_expr -> testlist ('=' testlist)* 
 *	Note: right recursive production for right associative operator '=' 
 */
assign_expr					: testlist											{
																					if(verbose_parser)
																						printf("assign_expr -> testlist\n");
																					$$ = $1;
																				}
							
							| testlist '=' assign_expr							{
																					if(verbose_parser_empty)
																						printf("assign_expr -> testlist '=' assign_expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ASSIGNMENT OPERATOR: =", "", children);
																				}
							;

flow_stmt					: BREAK		 										{
																					if(verbose_parser)
																						printf("flow_stmt -> BREAK\n");
																					$$ = new_ast_node(yylineno,"BREAK","break","",no_child);
																				}
							
							| CONTINUE		 									{
																					if(verbose_parser) 
																						printf("flow_stmt -> CONTINUE\n");
																					$$ = new_ast_node(yylineno,"CONTINUE","continue","",no_child);
																				}

							| return_stmt										{
																					if(verbose_parser) 
																						printf("flow_stmt -> return_stmt\n");
																					$$ = $1;
																				}
							;

return_stmt 				: RETURN											{
																					if(verbose_parser) 
																						printf("return_stmt -> RETURN\n");
																					$$ = new_ast_node(yylineno,"RETURN","return","",no_child);
																				}
							
							| RETURN testlist									{
																					if(verbose_parser) 
																						printf("return_stmt -> RETURN testlist\n");
																					children = {$2};
																				 	$$ = new_ast_node($2->line_num,"","return","return_stmt",children);
																				}
							;

compound_stmt				: if_stmt 											{
																					if(verbose_parser)
																						printf("compound_stmt -> if_stmt\n");
																					$$ = $1;
																				}
							
							| while_stmt 										{
																					if(verbose_parser) 
																						printf("compound_stmt -> while_stmt\n");
																					$$ = $1;
																				}
							
							| for_stmt 											{
																					if(verbose_parser)
																						printf("compound_stmt -> for_stmt\n");
																					$$ = $1;
																				}
							
							| funcdef 											{
																					if(verbose_parser)
																						printf("compound_stmt -> funcdef\n");
																					$$ = $1;
																				}
							
							| classdef											{
																					if(verbose_parser)
																						printf("compound_stmt -> classdef\n");
																					$$ = $1;
																				}
							;

if_stmt						: IF test ':' suite elif_seq						{
																					if(verbose_parser)
																						printf("if_stmt -> IF test ':' suite elif_seq\n");
																					children = {$2, $4};
																					struct ast_node *temp = new_ast_node($2->line_num,"", "if", "if_stmt", children);
																					children = {temp, $5};
																					$$ = new_ast_node($2->line_num,"","if_stmt","if_stmt",children);
																				}
							
							| IF test ':' suite elif_seq ELSE ':' suite			{
																					if(verbose_parser) 
																						printf("if_stmt -> IF test ':' suite elif_seq ELSE ':' suite\n");
																					children = {$2, $4};
																					struct ast_node *temp1 = new_ast_node($2->line_num,"", "if", "if_stmt", children);
																					children = {$8};
																					struct ast_node *temp2 = new_ast_node($6->line_num,"", "else", "if_stmt", children);
																					children = {temp1, $5, temp2};
																					$$ = new_ast_node($2->line_num,"","if_stmt","if_stmt",children);
																				}
							;

while_stmt					: WHILE test ':' suite								{
																					if(verbose_parser) 
																						printf("while_stmt -> WHILE test ':' suite\n");
																					children = {$2,$4};
																					$$ = new_ast_node($2->line_num,"","while","while_stmt",children);
																				}

for_stmt					: FOR exprlist IN testlist ':' suite				{
																					if(verbose_parser) 
																						printf("for_stmt -> FOR exprlist IN testlist ':' suite\n");
																					children = {new_ast_node($2->line_num,"FOR","for","",no_child),$2,
																								new_ast_node($2->line_num,"IN","in","",no_child),$4,$6};
																					$$ = new_ast_node($2->line_num,"","for_stmt","for_stmt",children);
																				}
							;

funcdef						: DEF NAME_DUNDER parameters RARROW test ':' suite	{
																					if(verbose_parser) 
																						printf("funcdef -> DEF NAME_DUNDER parameters RARROW test ':' suite\n");
																					($5->text2print) = "return type: " + ($5->text2print);
																					children = {$3,$7,$5};
																					string str = "function\n" + ($2->text2print);
																					$$ = new_ast_node($2->line_num,$2->nonterminal,str,"funcdef",children);
																				}
							
							| DEF NAME_DUNDER parameters ':' suite				{
																					if(verbose_parser) 
																						printf("funcdef -> DEF NAME_DUNDER parameters ':' suite\n");
																					children = {$3,$5};
																					string str = "function\n" + ($2->text2print);
																					$$ = new_ast_node($2->line_num,$2->nonterminal,str,"funcdef",children);
																				}
							;

parameters	 				: '(' ')'											{
																					if(verbose_parser)
																						printf("parameters -> '(' ')'\n");
																					children = {new_ast_node(yylineno,"", "No Parameter","",no_child)};
																					$$ = new_ast_node(yylineno,"","parameters","parameters",children);
																				}
							
							| '(' arglist ')'									{
																					if(verbose_parser)
																						printf("parameters -> '(' arglist ')'\n");
																					children = {$2};
																					$$ = new_ast_node($2->line_num,"","parameters","parameters",children);
																				}
							;

arglist 					: argument COMMA_argument_seq						{
																					if(verbose_parser)
																						printf("arglist -> argument COMMA_argument_seq\n");
																					children = {$1,$2};
																					$$ = new_ast_node($1->line_num,"","","arglist",children);
																				}
							
							| argument COMMA_argument_seq  ','					{
																					if(verbose_parser) 
																						printf("arglist -> argument COMMA_argument_seq ','\n");
																					children = {$1,$2};
																					$$ = new_ast_node($1->line_num,"","","arglist",children);
																				}
							;

COMMA_argument_seq 			: COMMA_argument_seq ',' argument 					{
																					if(verbose_parser)
																						printf("COMMA_argument_seq -> COMMA_argument_seq ',' argument\n");
																					children = {$1,$3};
																					$$ = new_ast_node($3->line_num,"","","COMMA_argument_seq",children);
																				}
							
							| %empty											{
																					if(verbose_parser_empty)
																						printf("COMMA_argument_seq -> empty\n");
																					$$ = NULL;
																				}
							;

argument 					: NAME_DUNDER ':' test								{
																					if(verbose_parser)
																						printf("argument -> NAME_DUNDER ':' test\n");
																					children = {$1,$3};
																					$$ = new_ast_node($1->line_num,"",":","argument",children);
																				}
							
							| NAME_DUNDER										{
																					if(verbose_parser)
																						printf("argument -> NAME_DUNDER\n");
																					$$ = $1;
																				}
							;

classdef 					: CLASS NAME ':' suite  							{	
																					if(verbose_parser)
																						printf("classdef -> CLASS NAME ':' suite\n");
																					children = {$4};
																					string str = "class\n";
																					str += "NAME: ";
																					str += ($2->lexeme);
																					$$ = new_ast_node($2->line_num,$2->lexeme,str,"classdef",children);
																				}

							| CLASS NAME '(' ')' ':' suite  					{	
																					if(verbose_parser)
																						printf("classdef -> CLASS NAME '(' ')' ':' suite\n");
																					children = {$6};
																					string str = "class\n";
																					str += "NAME: ";
																					str += ($2->lexeme);
																					$$ = new_ast_node($2->line_num,$2->lexeme,str,"classdef",children);	
																				}

							| CLASS NAME '(' NAME ')' ':' suite  				{
																					if(verbose_parser)
																						printf("classdef -> CLASS NAME '(' NAME ')' ':' suite\n");
																					string str = "class\n";
																					str += "NAME: ";
																					str += ($2->lexeme);
																					str += "\nInherited from class\n";
																					str += "NAME: ";
																					str += ($4->lexeme);
																					children = {$7};
																					$$ = new_ast_node($2->line_num,$2->lexeme,str,"classdef",children);
																				}
							;

elif_seq					: elif_seq ELIF test ':' suite						{
																					if(verbose_parser)
																						printf("elif_seq -> elif_seq ELIF test ':' suite\n");
																					children = {$3, $5};
																					struct ast_node *temp = new_ast_node($3->line_num,"", "elif", "elif_seq", children);
																					if(!($1)){
																						children = {temp};
																						$$ = new_ast_node($3->line_num,"","","elif_seq",children);
																					}
																					else{
																						$$ = add_child_back($1, temp);
																					}
																				}
							
							| %empty											{
																					if(verbose_parser_empty)
																						printf("elif_seq -> empty\n");
																					$$ = NULL;
																				}
							;

suite						: simple_stmt										{	
																					if(verbose_parser) 
																						printf("suite -> simple_stmt\n");
																					$$ = $1;
																				}

							| NEWLINE INDENT stmt stmt_seq DEDENT 				{	
																					if(verbose_parser) 
																						printf("suite -> NEWLINE INDENT stmt stmt_seq DEDENT\n");
																					children = {$3,$4};
																					$$ = new_ast_node($3->line_num,"","body","suite",children);
																				}
							;

stmt_seq					: stmt_seq stmt			 							{	
																					if(verbose_parser)
																						printf("stmt_seq -> stmt_seq stmt\n");
																					children = {$1,$2};
																					$$ = new_ast_node($2->line_num,"","","stmt_seq",children);
																				}

							| %empty  											{	
																					if(verbose_parser_empty)
																						printf("stmt_seq -> empty\n");
																					$$ = NULL;
																				}
							;

/* test -> and_test ('or' and_test)* */
test						: and_test											{	
																					if(verbose_parser) 
																						printf("test -> and_test\n");
																					$$ = $1;	
																				}

							| test OR and_test									{
																					if(verbose_parser)
																						printf("test -> test OR and_test\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "LOGICAL OPERATOR: or", "test", children);
																				}
							;

/* and_test -> not_test ('and' not_test)* */
and_test					: not_test											{	
																					if(verbose_parser)
																						printf("and_test -> not_test\n");
																					$$ = $1;
																				}
							
							| and_test AND not_test								{
																					if(verbose_parser)
																						printf("and_test -> and_test AND not_test\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "LOGICAL OPERATOR: and", "and_test", children);
																				}
							;

not_test					: NOT not_test 										{	
																					if(verbose_parser) 
																						printf("not_test -> NOT not_test\n");
																					children = {$2};
																					$$ = new_ast_node($2->line_num,"","LOGICAL OPERATOR: not","not_test",children);
																				}

							| comparison										{	
																					if(verbose_parser) 
																						printf("not_test -> comparison\n");
																					$$ = $1;
																				}
							;							

comparison					: expr 												{	
																					if(verbose_parser) 
																						printf("comparison -> expr\n");
																					$$ = $1;
																				}
							
							| comparison '<' expr								{
																					if(verbose_parser)
																						printf("comparison -> comparison '<' expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "RELATIONAL OPERATOR: <", "comparison", children);
																				}

							| comparison '>' expr								{
																					if(verbose_parser)
																						printf("comparison -> comparison '>' expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "RELATIONAL OPERATOR: >", "comparison", children);
																				}

							| comparison EQEQUAL expr							{
																					if(verbose_parser)
																						printf("comparison -> comparison '==' expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "RELATIONAL OPERATOR: ==", "comparison", children);
																				}

							| comparison GREATEREQUAL expr						{
																					if(verbose_parser)
																						printf("comparison -> comparison '>=' expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "RELATIONAL OPERATOR: >=", "comparison", children);
																				}
							
							| comparison LESSEQUAL expr							{
																					if(verbose_parser)
																						printf("comparison -> comparison '<=' expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "RELATIONAL OPERATOR: <=", "comparison", children);
																				}
							
							| comparison NOTEQUAL expr							{
																					if(verbose_parser)
																						printf("comparison -> comparison '!=' expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "RELATIONAL OPERATOR: !=", "comparison", children);
																				}
							;

/* expr -> xor_expr ('|' xor_expr)* */
expr						: xor_expr 											{	
																					if(verbose_parser) 
																						printf("expr -> xor_expr\n");
																					$$ = $1;	
																				}	
							
							| expr '|' xor_expr									{
																					if(verbose_parser)
																						printf("expr -> expr '|' xor_expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "BITWISE OPERATOR: |", "expr", children);
																				}
							;

/* xor_expr -> and_expr ('^' and_expr)* */
xor_expr					: and_expr 											{	
																					if(verbose_parser) 
																						printf("xor_expr -> and_expr\n");
																					$$ = $1;
																				}
							
							| xor_expr '^' and_expr								{
																					if(verbose_parser)
																						printf("xor_expr -> xor_expr '^' and_expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "BITWISE OPERATOR: ^", "xor_expr", children);
																				}
							;	

/* and_expr -> shift_expr ('&' shift_expr)* */
and_expr					: shift_expr 										{	
																					if(verbose_parser) 
																						printf("and_expr -> shift_expr\n");
																					$$ = $1;
																				}
							
							| and_expr '&' shift_expr							{
																					if(verbose_parser)
																						printf("and_expr -> and_expr '&' shift_expr\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "BITWISE OPERATOR: &", "and_expr", children);
																				}
							;

/* shift_expr -> arith_expr (('<<' | '>>') arith_expr)* */
shift_expr					: arith_expr 										{	
																					if(verbose_parser) 
							   															printf("shift_expr -> arith_expr\n");
							   														$$ = $1;
							   													}
							
							| shift_expr LEFTSHIFT arith_expr					{
							   														if(verbose_parser)
							   															printf("shift_expr -> shift_expr '<<' arith_expr\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "BITWISE OPERATOR: <<", "shift_expr", children);
							   													}
							
							| shift_expr RIGHTSHIFT arith_expr					{
							   														if(verbose_parser)
							   															printf("shift_expr -> shift_expr '>>' arith_expr\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "BITWISE OPERATOR: >>", "shift_expr", children);
							   													}
							;

/* arith_expr -> term (('+'| '-') term)* */
arith_expr					: term												{
							   														if(verbose_parser)
							   															printf("arith_expr -> term\n");
							   														$$ = $1;
							   													}

							| arith_expr '+' term								{
							   														if(verbose_parser)
							   															printf("arith_expr -> arith_expr + term\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: +", "arith_expr", children);
							   													}

							| arith_expr '-' term								{
							   														if(verbose_parser)
							   															printf("arith_expr -> arith_expr - term\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: -", "arith_expr", children);
							   													}
							;

term						: factor 					 						{	
																					if(verbose_parser) 
							   															printf("term -> factor\n");
                                                                                   $$ = $1;
							   													}
							
							| term '*' factor									{
							   														if(verbose_parser)
							   															printf("term -> term '*' factor\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: *", "term", children);
							   													}
							
							| term '/' factor									{
							   														if(verbose_parser)
							   															printf("term -> term '/' factor\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: /", "term", children);
							   													}
							
							| term '%' factor									{
							   														if(verbose_parser)
							   															printf("term -> term '%%' factor\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: %", "term", children);
							   													}
	
							| term DOUBLESLASH factor							{
																					if(verbose_parser)
																						printf("term -> term '//' factor\n");
																					children = {$1, $3};
																					$$ = new_ast_node($1->line_num, "", "ARITHMETIC OPERATOR: //", "term", children);
																				}
							;

factor						: '+' factor										{	
																					if(verbose_parser)
							   															printf("factor -> '+' factor\n");
							   														children = {$2};
							   														$$ = new_ast_node($2->line_num,"","UNARY OPERATOR: +","factor",children);
																				}

							| '-' factor										{	
																					if(verbose_parser)
							   															printf("factor -> '-' factor\n");
							   														children = {$2};
							   														$$ = new_ast_node($2->line_num,"","UNARY OPERATOR: -","factor",children);	
																				}

							| '~' factor										{	
																					if(verbose_parser)	
							   															printf("factor -> '~' factor\n");
							   														children = {$2};
							   														$$ = new_ast_node($2->line_num,"","UNARY OPERATOR: ~","factor",children);
																				}

							| power												{	
																					if(verbose_parser)
							   															printf("factor -> power\n");
							   														$$ = $1;	
							   													}
							;

power						: atom_expr											{	
																					if(verbose_parser)
							   															printf("power -> atom_expr\n");
							   														$$ = $1;
							   													}

							| atom_expr DOUBLESTAR factor						{	
																					if(verbose_parser)
							   															printf("power -> atom_expr '**' factor\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($3->line_num,"","ARITHMETIC OPERATOR: **","power",children);
																				}
							;

atom_expr					: atom trailer										{	
																					if(verbose_parser)
							   															printf("atom_expr -> atom trailer\n");
							   														$$ = add_child_back($1,$2);
							   													}
																				
							| atom '.' NAME_DUNDER								{	
																					if(verbose_parser)
                                                                                        printf(" atom_expr -> atom '.' NAME_DUNDER\n");
							   														children = {$1,$3};
							   														$$ = new_ast_node($3->line_num,"",".","atom_expr",children);
							   													}

							| atom '.' NAME_DUNDER trailer						{	
																					if(verbose_parser)
                                                                                        printf("atom_expr -> atom '.' NAME_DUNDER trailer\n");
							   														$3 = add_child_back($3,$4);
							   														children = {$1,$3};
							   														$$ = new_ast_node($3->line_num,"",".","atom_expr",children);
							   													}

							| atom 												{	
																					if(verbose_parser)
                                                                                         printf("atom_expr -> atom\n");
							   														$$ = $1;
							   													}
							;							

trailer						: '(' ')'											{	
																					if(verbose_parser)
																						printf("trailer -> '(' ')'\n");
																					$$ = new_ast_node(yylineno,"","( )","trailer",no_child);
																				}

							| '(' testlist ')'									{
																					if(verbose_parser)
																						printf("trailer -> '(' testlist ')'\n");
																					children = {$2};
																					$$ = new_ast_node($2->line_num,"","","trailer",children);
																				}
							
							| '[' testlist ']'									{
																					if(verbose_parser)
																						printf("trailer -> '[' testlist ']'\n");
																					children = {$2};
																					$$ = new_ast_node($2->line_num, "", "", "trailer", children);
																				}
							;

atom						: '(' ')'											{	
																					if(verbose_parser)
							   															printf("atom -> '(' ')'\n");
							   														$$ = new_ast_node(yylineno,"","( )","atom",no_child);
							   													}

							| '(' testlist ')'									{	
																					if(verbose_parser)
							   															printf("atom -> '(' testlist ')'\n");
							   														$$ = $2;
							   													}

							| '[' ']'											{	
																					if(verbose_parser)
							   															printf("atom -> '[' ']'\n");
							   														$$ = new_ast_node(yylineno,"","[ ]","atom",no_child);
							   													}

							| '[' testlist ']'									{		
																					if(verbose_parser)
							   															printf("atom -> '[' testlist ']'\n");
							   														children = {$2};
							   														$$ = new_ast_node($2->line_num,"","list [ ]","atom",children);
																				}

							| NAME_DUNDER										{	
																					if(verbose_parser)
							   															printf("atom -> NAME_DUNDER\n");
							   														$$ = $1;
							   													}

							| INTEGER											{	
																					if(verbose_parser)
							   															printf("atom -> INTEGER\n");
							   														string str = $1->lexeme;
							   														str = "INTEGER: " + str;
							   														$$ = new_ast_node($1->line_num,"INTEGER",str,"",no_child);
																					$$->type = "INT";
																					$$->what = "CONSTANT";

							   													}
							
							| FLOAT_NUMBER										{	
																					if(verbose_parser)
																						printf("atom -> FLOAT_NUMBER\n");
																					string str = $1->lexeme;
																					str = "FLOAT_NUMBER: " + str;
																					$$ = new_ast_node($1->line_num,"FLOAT_NUMBER",str,"",no_child);
																					$$->type = "FLOAT";
																					$$->what = "CONSTANT";
																				}

							| STRING 											{	
																					if(verbose_parser)
																						printf("atom ->STRING\n");
																					string str = $1->lexeme;
																					str = "STRING: " + str;
							   														$$ = new_ast_node($1->line_num,"",str,"atom",no_child);
																					$$->type = "STR";
																					$$->what = "CONSTANT";
																				}

							| NONE												{	
																					if(verbose_parser)
																						printf("atom -> NONE\n");
																					$$ = new_ast_node(yylineno,"","None","atom",no_child);
																					$$->type = "NONE";
																				}

							| TRUE												{	
																					if(verbose_parser)
																						printf("atom -> TRUE\n");
																					$$ = new_ast_node(yylineno,"","True","atom",no_child);
																					$$->type = "BOOL";
																					$$->what = "CONSTANT";
																				}

							| FALSE												{	
																					if(verbose_parser)
																						printf("atom -> FALSE\n");
																					$$ = new_ast_node(yylineno,"","False","atom",no_child);
																					$$->type = "BOOL";
																					$$->what = "CONSTANT";
																				}

							| INT												{	
																					if(verbose_parser)
																						printf("atom -> INT\n");
																					$$ = new_ast_node(yylineno,"","int","atom",no_child);
																					//$$->type = "INT";
																				}

							| FLOAT												{	
																					if(verbose_parser)
																						printf("atom -> FLOAT\n");
																					$$ = new_ast_node(yylineno,"","float","atom",no_child);
																					//$$->type = "FLOAT";
																				}

							| STR												{	
																					if(verbose_parser)
																						printf("atom -> STR\n");
																					$$ = new_ast_node(yylineno,"","str","atom",no_child);
																					//$$->type = "STR";
																				}

							| BOOL												{	
																					if(verbose_parser)
																						printf("atom -> BOOL\n");
																					$$ = new_ast_node(yylineno,"","bool","atom",no_child);
																					//$$->type = "BOOL";
																				}
							
							| LIST												{	
																					if(verbose_parser)
																						printf("atom -> LIST\n");
																					$$ = new_ast_node(yylineno,"","list [ ]","atom",no_child);
																				}
							;

exprlist					: expr COMMA_expr_seq								{	
																					if(verbose_parser)
							   															printf("exprlist -> expr COMMA_expr_seq\n");
							   														children = {$1, $2};
							   														$$ = new_ast_node($1->line_num,"","","exprlist",children);
							   													}	

							| expr COMMA_expr_seq ','							{	
																					if(verbose_parser)
							   															printf("exprlist -> expr COMMA_expr_seq ','\n");
							   														children = {$1, $2};
							   														$$ = new_ast_node($1->line_num,"","","exprlist",children);
							   													}
							;

COMMA_expr_seq				: COMMA_expr_seq ',' expr							{
																					if(verbose_parser)
							   															printf("COMMA_expr_seq -> COMMA_expr_seq ',' expr\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($3->line_num,"","","COMMA_expr_seq",children);
							   													}

							| %empty											{	
																					if(verbose_parser_empty)
							   															printf("COMMA_expr_seq -> empty\n");
							   														$$ = NULL;
							   													}
							;

testlist					: test COMMA_test_seq								{	
																					if(verbose_parser)
							   															printf("testlist -> test COMMA_test_seq\n");
							   														children = {$1, $2};
							   														$$ = new_ast_node($1->line_num,"","","testlist",children);
							   													}

							| test COMMA_test_seq ','							{	
																					if(verbose_parser)
							   															printf("testlist -> test COMMA_test_seq ','\n");
							   														children = {$1, $2};
							   														$$ = new_ast_node($1->line_num,"","","testlist",children);
							   													}
							;

COMMA_test_seq				: COMMA_test_seq ',' test							{
							   														if(verbose_parser) 
							   															printf("COMMA_test_seq -> COMMA_test_seq ',' test\n");
							   														children = {$1, $3};
							   														$$ = new_ast_node($3->line_num,"","","COMMA_test_seq",children);
							   													}
							
							| %empty											{
							   														if(verbose_parser_empty)
							   															printf("COMMA_test_seq -> empty\n");
							   														$$ = NULL;
							   													}
							;

NAME_DUNDER					: NAME												{	
																					if(verbose_parser)
																						printf("NAME_DUNDER -> NAME\n");
																					string str = $1->lexeme;
																					str = "NAME: " + str;
																					$$ = new_ast_node($1->line_num,"NAME", str, $1->lexeme, no_child);
							   													}

							| DUNDER											{	
																					if(verbose_parser)
							   															printf("NAME_DUNDER -> DUNDER\n");
							   														string str = $1->lexeme;
							   														str = "DUNDER: " + str;
							   														$$ = new_ast_node($1->line_num,"DUNDER", str, $1->lexeme, no_child);
																				}
							;

%%

void yyerror(char *s){
	fprintf(stderr, "Lineno: %d %s\nOffending token: %s\n", yylineno, s, yytext);
	exit(0);
}

void print_help_manual();
void print_3ac(struct ast_node *root_ast);

void live_feeder(vector<struct quad_node *> &code){
    map<string,int> mp;
    for(int i=code.size()-1;i>=0;i--){
		if(((code[i])->arg2 != NULL) && (((code[i])->arg2)->type == "temporary")){
            if(mp.count(((code[i])->arg2)->value )){
                ((code[i])->arg2)->live = true;
            }
            else{
                mp[((code[i])->arg2)->value] = 1;
                ((code[i])->arg2)->live = false;
            }
        }
        if(((code[i])->arg1 != NULL) && (((code[i])->arg1)->type == "temporary")){
            if(mp.count(((code[i])->arg1)->value)){
                ((code[i])->arg1)->live = true;
            }
            else{
                mp[((code[i])->arg1)->value] = 1;
                ((code[i])->arg1)->live = false;
            }
        }
		if(((code[i])->result != NULL) && (((code[i])->result)->type == "temporary")){
            if(mp.count(((code[i])->result)->value )){
                ((code[i])->result)->live = true;
				mp.erase(((code[i])->result)->value);
            }
            else{
                if((code[i])->op == "mem_assign_list"){
					//code.erase(code.begin()+i);
					((code[i])->result)->live = false;
				}
            }
        }
    }
}

int main(int argc, char *argv[]){

	bool input_file_found = false;
	string output_dot_file;
	for(int i = 1; i<argc; i++){
	/*	--help  */
		if(string(argv[i]) == "--help" || string(argv[i]) == "-h"){
			print_help_manual();
			return -1;
		}
		/*	--input	*/
		else if(string(argv[i]) == "--input" || string(argv[i]) == "-i"){
			if((i+1) < argc){
				FILE* file = fopen(argv[i+1],"r");
				if(!file){
					cout << "Error: Input programa file could not be opened" << endl;
					return -1;
				}

				yyin = file;
				input_file_found = true;
				i++;
			}
			else{
				cout << "Error: No Input program file given" << endl;
				return -1;
			}
		}
		/*	--output	*/
		else if(string(argv[i]) == "--output" || string(argv[i]) == "-o"){
			if((i+1) < argc){
				output_dot_file = string(argv[i+1]);
				FILE* temp = fopen(argv[i+1],"w");
				if(!temp){
					cout << "Error: Output program file could not be opened" << endl;
					return -1;
				}

				i++;
			}
			else{
				cout << "Warning: No Output program file given" << endl;
				cout << "Printing dot script to default filename..." << endl;
				/* Unlike to the input file case, it won't terminate here
					rather will print the dot script to a default filename*/
			}
		}
		/*	--verbose	*/
		else if(string(argv[i]) == "--verbose" || string(argv[i]) == "-v"){
			verbose_parser = true;
		}
		else{
			if(!input_file_found){
				/* Input file not found yet */
				int n = string(argv[i]).size();
				if(argv[i][n-1] == 'y' && argv[i][n-2] == 'p' && argv[i][n-3] == '.'){
					FILE* file = fopen(argv[i],"r");
					if(!file){
						cout << "Error: Input programa file could not be opened" << endl;
						return -1;
					}

					yyin = file;
					input_file_found = true;
					continue;
				}
			}

			cout << "Error: Invalid parameters" << endl;
			print_help_manual();
			return -1;

		}
	}

    if(!input_file_found){
        printf("READING FROM STANDARD INPUT\n");
        yyin = stdin;
    }

	/* Initializing the stack for indent and dedent */
	init_stack();
    yyparse();

	children = create_ast(root);
	/* Code to create AST ast root_ast */
	struct ast_node *root_ast = new_ast_node(LINE_IRRELEVANT,"","start","",children);


	/*	Generates a file.dot */
	generate_dot_file(root_ast,output_dot_file);
	symbol_table_head = new struct symbol_table_node;
	populate_symbol_table(root_ast,symbol_table_head);
	dump_symbol_table(symbol_table_head);
	populate_type(root_ast);
	generate_3ac(symbol_table_head,root_ast);
	live_feeder(root_ast->code);
    
    fstream fout;
    fout.open("final.s",ios::out);
    fout << gen_x86(root_ast->code) << endl;
    fout.close();
    if(yyin != stdin){
		fclose(yyin);
	}
    return 0;
}


void print_3ac(struct ast_node *root_ast)
{
	vector<struct quad_node*> &code = root_ast->code;
    cout << "size of instructions at root: " << code.size() << endl;
	for(int i = 0; i < code.size(); i++){
		string op = code[i]->op;
        string arg1 = "-";
        string arg2 = "-";
        string result = "-";
		int b1=-1,b2=-1,b3=-1;
        int w1=-19,w2=-19,w3=-19;
		if(code[i]->arg1){
            arg1 = code[i]->arg1->value;
			b1 = code[i]->arg1->live;
            w1 = code[i]->arg1->register_width;
		}
		if(code[i]->arg2){
		    arg2 = code[i]->arg2->value;
			b2 = code[i]->arg2->live;
            w2 = code[i]->arg2->register_width;
		}
		if(code[i]->result){
		    result = code[i]->result->value;
			b3 = code[i]->result->live;
            w3 = code[i]->result->register_width;
		}
		cout << "["<< i << "] " << result << ' ' << b3 << "#" << w3 << " = " 
        << arg1 << ' ' << b1 << "#" << w1 << " " << op 
        << " " << arg2 << ' ' << b2 << "#" << w2 << endl;
	}

}
void print_help_manual(){
	cout << "Usage: ./python [options]     \n\n";
    cout << "Commands:\n-h, --help \t\t\t\t\t Show help page\n";
    cout << "-i, --input <input_file_name> \t\t\t Give input file\n";
    cout << "-o, --output <output_file_name>\t\t\t Redirect dot file to output file\n";
    cout << "-v, --verbose \t\t\t\t\t Outputs the entire derivation in command line\n";
    return;
}

bool verbose_ast = true;
int counter=1;
queue<int> ast_node_no;
string print_ast_node(struct ast_node *current,int ast_node_number,int type){
    string s="";
    if(type==0){
		string st = (current->text2print),g="";
		for(int i=0;i<st.size();i++){
			if(st[i]!='"'){
				g=g+st[i];
			}
			else{
				g=g+"\\"+st[i];
			}
		}
		if(verbose_ast)
			g=g+"\nwidth = "+to_string(current->width);
        s="node" + to_string(ast_node_number) + " [label = " + "\"" +  g + "\"]";
    }
    else{
        s="node" + to_string(ast_node_number);
    }
    return s;
}

void generate_dot_file(struct ast_node* root, string output_file){
	if(output_file.size() == 0){
		output_file = "file.dot";
	}
	
	ofstream file(output_file);

    file << "digraph {\n";
    queue<struct ast_node *> q;
	if(root==NULL){
		file << "}\n";
		return;
	}
    file << "   ";
    file << print_ast_node(root,counter,0);
    file << "\n";
    q.push(root);
    ast_node_no.push(counter);
    counter++;
    while(!q.empty()){
        struct ast_node* temp = q.front();
        q.pop();
        int ast_node_number = ast_node_no.front();
        ast_node_no.pop();
        for(int i=0;i<(temp->children).size();i++){
			if((temp->children)[i]==NULL){
				continue;
			}
			file << "	";
			file << print_ast_node((temp->children)[i],counter,0);
			file << "\n";
            file << "   ";
            file << print_ast_node(temp,ast_node_number,1);
            file << " -> ";
            file << print_ast_node((temp->children)[i],counter,1);
            file << "\n";
            q.push((temp->children)[i]);
            ast_node_no.push(counter);
            counter++;
        }
    }
    file << "}\n";

    file.close();
}
