#ifndef AST_H
#define AST_H

#include <bits/stdc++.h>
using namespace std;
#define LINE_IRRELEVANT -1
#define no_child vector<struct ast_node *> ()

#include "symtable.h"

struct address_node
{
	/*
			Type			Value
		------------	 -----------
		1. Temporary		t_1
		2. Stack			-offset(%rbp)
		3. Bool				True / False
		4. Integer			4
		5. Float			4.5
		6. String			"hello world"
		7. Label			L_1
	*/
	string type;
	string value;
	
	/* 	
		width of register (in bits) required to store value pointed to by this address
		for address pointing to int, width = 32
		for address pointing to string or list or object, width = 64
	*/
	int register_width;	
    int offset;	
	bool live;	/* dead or live in case of temporary type */
};

/*-----------------------		--------- quad code ---------
| result = arg1 op arg2 |	->	| op | arg1 | arg2 | result |
-------------------------		----------------------------*/
struct quad_node
{
	string op;
	struct address_node *arg1;
	struct address_node *arg2;
	struct address_node *result;
};


/* Structure definitions */
struct ast_node{
        int                         line_num;
        string                      type;
        int                         width = 32;
        int                         offset;
        int                         register_width;
        string                      what;
        string                      terminal;		  // represents terminal type such as NUMBER, NAME, etc.
        string                      text2print; 	  // terminal -> lexeme | non-terminal -> label
        string                      nonterminal;	  // represents non-terminal type such as funcdef, expr, etc.
        vector<struct ast_node *>   children;
        struct address_node*        address;          /* address during 3ac for eg: t_0,t_1*/
        struct address_node*        laddress;         /* address during 3ac for eg: t_0,t_1*/
        vector<struct quad_node *>  code;             /* 3ac code built till this node */

		struct symbol_table_entry	*entry;
};

struct quad_node* new_quad(string op,
                           struct address_node* arg1,
                           struct address_node* arg2,
                           struct address_node* result);

struct ast_node* new_ast_node(
        int                 line_num,
        string              terminal,
        string              text2print,
        string              nonterminal,
        vector<ast_node *>  children
);

struct terminal_node
{
        int     line_num;
        string  type;
        string  lexeme;
};

struct terminal_node* new_terminal_node(
        int     line_num,
        string  type,
        string  lexeme
);

struct ast_node* add_child_back(struct ast_node* parent ,struct ast_node* child);

struct ast_node* add_child_front(struct ast_node *parent ,struct ast_node *child);

void copy_ast_node(struct ast_node *source, struct ast_node* destination);
void print_ast_node(struct ast_node *node);

int is_terminal(struct ast_node *root);

vector<struct ast_node *> create_ast(struct ast_node *current);

string print_ast_node(struct ast_node *current,int ast_node_number,int type);

void dot_file(struct ast_node* root, string output_file);

#endif
