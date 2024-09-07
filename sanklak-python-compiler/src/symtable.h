#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <bits/stdc++.h>
using namespace std;

struct symbol_table_node{
	string 								 lexeme;
	string                       		 type;
	int                                  definition_line_num;
	int									 current_offset;
	int									 argument_offset;
	int 								 width;
	vector<struct symbol_table_entry*>   scope_content; /*{lexeme-> symbol_table_entry}*/
	struct symbol_table_node*            parent;
	vector<struct symbol_table_node*>    children;
	map<string, int>                     entry_finder_local;
	map<string, int>                     table_finder_local;

	/* In case of scope being a function definition */
	vector< struct symbol_table_entry* > arguments;
	map< string ,int >                   argument_finder;
	string                               return_type; // return type
	string                               parent_class;

    unsigned long long int               temporary_address_count = 0; //support for 3ac 
    unsigned long long int               label_count    = 0; //support for 3ac 
};

struct symbol_table_entry{
	 int 					line_num;
	 int					offset;
	 int 					width;
	 string 				lexeme;
	 string 				type;
	 struct symbol_table_node *scope; // pointer to the symbol table to which this entry belongs
};

/*	struct symbol_table_node* current_scope; */
struct symbol_table_node* populate_symbol_table(struct ast_node* root_ast,struct symbol_table_node* current);

void dump_symbol_table(struct symbol_table_node* symbol_table);

void generate_3ac(struct symbol_table_node* current_scope, struct ast_node* root_ast);

void make_3ac_clean(struct ast_node* node);
/* Populates the type field for the internal nodes and also checks type matching
	across various operators*/
void populate_type(struct ast_node* current_node);

#endif
