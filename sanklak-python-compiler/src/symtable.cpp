#include <bits/stdc++.h>
using namespace std;
#include "symtable.h"
#include "ast.h"

int universal_label_count = 0;
int universal_temporary_address_count = 0;

string space = "";
void print_symbol_table_entry(struct symbol_table_entry *entry)
{
	cout << space << "====== SYMBOL TABLE ENTRY -> START =========\n";	
	cout << space << "lexeme: " << entry->lexeme << endl;
	cout << space << "offset: " << entry->offset << endl;
	cout << space << "type: " << entry->type << endl;
	cout << space << "width: " << entry->width << endl;
	cout << space << "scope: " << entry->scope->lexeme << endl;
	cout << space << "====== SYMBOL TABLE ENTRY -> END =========\n";	
}

void print_symbol_table_node(struct symbol_table_node *table)
{
	cout << space << "================================ SYMBOL TABLE -> START =========================\n";
	cout << space << "lexeme: " << table->lexeme << endl;
	cout << space << "type: " << table->type << endl;
	cout << space << "definition_line_num: " << table->definition_line_num << endl;
	cout << space << "argument offset: " << table->argument_offset << endl;
	cout << space << "current offset: " << table->current_offset << endl;
	cout << space << "return type: " << table->return_type << endl;
	cout << space << "parent class: " << table->parent_class << endl;
	vector<struct symbol_table_entry*> &scope_content = table->scope_content;
	cout << space << "======================== SCOPE CONTENT -> START =======================\n";
	for(int i = 0; i < scope_content.size(); i++)
		print_symbol_table_entry(scope_content[i]);
	
	cout << space << "======================== SCOPE CONTENT -> END =======================\n";
	
	vector<struct symbol_table_entry*> &arguments = table->arguments;
	cout << space << "======================== ARGUMENTS -> START =======================\n";
	for(int i = 0; i < arguments.size(); i++)
		print_symbol_table_entry(arguments[i]);
	
	cout << space << "======================== ARGUMENTS -> END =======================\n";
	
	vector<struct symbol_table_node*> &children = table->children;
	cout << space << "============================ CHILDREN -> START ====================\n";	
	space += "\t";
	for(int i = 0; i < children.size(); i++)
		print_symbol_table_node(children[i]);
	cout << space << "============================ CHILDREN -> END ====================\n";	

	cout << space << "================================ SYMBOL TABLE -> END =========================\n";
}

/* Definition of functions */
string get_new_address(struct symbol_table_node *current_scope);
string get_new_loop_addr(struct symbol_table_node *current_node);
int is_func_call(struct ast_node *current_node);
int is_func_definition(struct ast_node *current_node);
int is_class_method_call(struct ast_node *current_node);
int is_class_definition(struct ast_node *current_node);
int is_identifier(struct ast_node *current_node, struct symbol_table_node *scope);
int is_constant(struct ast_node *current_node);
struct symbol_table_node *get_new_scope(struct ast_node *node, struct symbol_table_node *scope);
int is_class_object(struct ast_node *node, struct symbol_table_node *scope);
string get_type(struct ast_node *node, struct symbol_table_node *scope);
int is_list(struct ast_node *node);
struct symbol_table_node *find_scope_of(string &lexeme, struct symbol_table_node *parent_scope);

/* Saviour function :-) */
struct address_node* make_copy_of_address(struct address_node* src);

/* Generic heuristic functions */
string get_class_name(struct ast_node *current_node);
string get_func_name(struct ast_node *current_node);
string get_indentifier_name(string &text2print);
string get_operator(string text2print);
string get_constant(struct ast_node *current_node);
string get_width_for_list_element(string &type);
int is_typecasting(struct ast_node *node);
void backpatch(struct ast_node *node, string from, string to);

pair<string, pair<int, int>> get_info(struct ast_node *current_node,
										struct symbol_table_node *current_scope);


/* string requried while writing the 3ac code */
int debug_3ac = 0;
string NEW_LINE_STR = "\n\t";
string CALL = "call ";
string EQUAL = " = ";
string CLASS_CODE = "%%";
string VAR_ON_STACK = "stack";
string VAR_ON_HEAP = "heap";
string LIST_ADDR = "list_addr";

// struct symbol_table_node* current;
struct symbol_table_node *symbol_table_head;
map<string, int> entry_finder_global;
map<string, int> table_finder_global;

string name_compute(string name)
{
	int pos = name.find(':');
	return name.substr(pos + 2);
};

string type_compute(struct ast_node *ast, string type_check)
{
	string type;
	if (type_check == "list [ ]")
	{
		string type_name = (((ast->children)[0])->text2print);
		if (type_name == "int")
		{
			type = "LIST_INT";
		}
		else if (type_name == "float")
		{
			type = "LIST_FLOAT";
		}
		else if (type_name == "str")
		{
			type = "LIST_STR";
		}
		else if (type_name == "bool")
		{
			type = "LIST_BOOL";
		}
		else
		{
			type_name = name_compute(type_name);
			if (table_finder_global.count(type_name) && ((symbol_table_head->children)[table_finder_global[type_name]])->type == "CLASS_DECLARATION")
			{
				type = "LIST_" + type_name;
			}
			else
			{
				cout << "list of unknown type : " << type_name << " used in declaration at line no. : " << ast->line_num;
				exit(-1);
			}
		}
	}
	else
	{
		string type_name = type_check;
		if (type_name == "int")
		{
			type = "INT";
		}
		else if (type_name == "float")
		{
			type = "FLOAT";
		}
		else if (type_name == "str")
		{
			type = "STR";
		}
		else if (type_name == "bool")
		{
			type = "BOOL";
		}
		else if (type_name == "None")
		{
			type = "NONE";
		}
		else
		{
			type_name = name_compute(type_name);
			if (table_finder_global.count(type_name) && ((symbol_table_head->children)[table_finder_global[type_name]])->type == "CLASS_DECLARATION")
			{
				type = type_name;
			}
			else
			{
				cout << "Unknown type : " << type_name << " used in declaration at line no. : " << ast->line_num;
				exit(-1);
			}
		}
	}
	return type;
}

void fill_type(struct ast_node *node, string type)
{
	if(!node)
		return;

	node->type = type;
	
	for(int i = 0; i < (node->children).size(); i++)
		fill_type((node->children)[i], type);
}

pair<string, string> name_type_compute(struct ast_node *root_ast)
{
	string name = (((root_ast->children)[0])->text2print);

	if (name == "."){
		name = (((root_ast->children)[0])->children[1])->nonterminal;
		(((root_ast->children)[0])->children[1])->what = "IDENTIFIER";
		
		/* To be used in generate_3ac */
		(root_ast->children)[0]->what = "SELF_VARIABLE_ACCESS";
	}
	else{
		name = ((root_ast->children)[0])->nonterminal;
		((root_ast->children)[0])->what == "IDENTIFIER";
	}
	
	string type = type_compute((root_ast->children)[1], ((root_ast->children)[1])->text2print);

	fill_type(root_ast, type);

	return {name, type};
}

struct symbol_table_entry *create_entry(struct symbol_table_node *scope, string lexeme, int line_num, string type, int width, int offset)
{
	struct symbol_table_entry *entry = new symbol_table_entry;

	entry->lexeme = lexeme;
	entry->line_num = line_num;
	entry->type = type;
	entry->width = width;
	entry->offset = offset;
	entry->scope = scope;

	return entry;
}

int lookup_class(string class_name, string lexeme)
{
	if ((symbol_table_head->table_finder_local).count(class_name))
	{
		struct symbol_table_node *temp = symbol_table_head->children[((symbol_table_head->table_finder_local)[class_name])];
		if ((temp->entry_finder_local).count(lexeme))
		{
			return 1;
		}
		if (temp->parent_class != "")
		{
			return lookup_class(temp->parent_class, lexeme);
		}
	}
	return 0;
}

int find_width(string type)
{
	if (type == "INT")
		return 4;
	
	else if (type == "FLOAT")
		return 8;
	
	else if (type == "BOOL")
	{
		return 4;
	}
	else if (type == "NONE")
		return 4;
	
	return 8;
}

void insert(struct symbol_table_node *current_scope,
			struct ast_node *root_ast,
			string lexeme,
			int line_num,
			string type)
{
	int offset = 0, width = 0;
	if (type == "CLASS_DECLARATION")
	{
		if (entry_finder_global.count(lexeme))
		{
			cout << "Redeclartion of name : " << lexeme << " as a CLASS at line no. : " << line_num << endl;
			exit(-1);
		}
	}
	else if (type == "CLASS_METHOD")
	{
		/* function overloading not supported  */
		if (lexeme != "__init__" && lookup_class((current_scope->lexeme), lexeme) == 1)
		{
			cout << "Redeclartion of name : " << lexeme << " as a CLASS_METHOD at line no. : " << line_num << endl;
			exit(-1);
		}
	}
	else if (type == "FUNCTION_DEF")
	{
		if (entry_finder_global.count(lexeme))
		{
			cout << "Redeclartion of name : " << lexeme << " as a FUNCTION at line no. : " << line_num << endl;
			exit(-1);
		}
	}
	else if (current_scope->type == "CLASS_DECLARATION")
	{
		struct symbol_table_node *temp = current_scope;
		if ((lookup_class((temp->lexeme), lexeme) == 1))
		{
			cout << "Redeclartion of name : " << lexeme << " as a VARIABLE at line no. : " << line_num << endl;
			exit(-1);
		}
		width = find_width(type);
		offset = current_scope->current_offset + width;	
		current_scope->current_offset = offset;
	}
	else if (current_scope->type == "CLASS_METHOD")
	{
		if ((current_scope->argument_finder).count(lexeme))
		{
			cout << "Redeclartion of argument : " << lexeme << " of CLASS_METHOD as a VARIABLE at line no. : " << line_num << endl;
			exit(-1);
		}
		struct symbol_table_node *temp = current_scope->parent;
		if (lookup_class((temp->lexeme), lexeme) == 1)
		{
			cout << "Redeclartion of name : " << lexeme 
                << " as a VARIABLE at line no. : " << line_num << endl;
			exit(-1);
		}
		width = find_width(type);
		offset = current_scope->current_offset + width;	
		current_scope->current_offset = offset;
	}
	else
	{
		if ((entry_finder_global.count(lexeme)) 
            || ((current_scope->entry_finder_local).count(lexeme)) 
            || (current_scope->argument_finder).count(lexeme))
		{
			cout << "Redeclartion of name : " << lexeme << 
                " as a VARIABLE at line no. : " << line_num << endl;
			exit(-1);
		}
		width = find_width(type);
		offset = current_scope->current_offset + width;	
		current_scope->current_offset = offset;
	}
	struct symbol_table_entry *entry = create_entry(current_scope, lexeme, line_num, type, width, offset);

	root_ast->entry = entry;

	if (current_scope == symbol_table_head)
		entry_finder_global[lexeme] = (current_scope->scope_content).size();

	(current_scope->scope_content).push_back(entry);
	(current_scope->entry_finder_local)[lexeme] = ((current_scope->scope_content).size() - 1);
}

struct symbol_table_node *create_symbol_table_node(
	string lexeme,
	string parent_class,
	int definition_line_num,
	string type,
	string return_type,
	struct symbol_table_node *parent)
{
	struct symbol_table_node *new_symbol_table = new symbol_table_node();

	new_symbol_table->lexeme = lexeme;
	new_symbol_table->type = type;
	new_symbol_table->parent = parent;
	new_symbol_table->definition_line_num = definition_line_num;
	new_symbol_table->return_type = "";
	new_symbol_table->parent_class = "";
	new_symbol_table->current_offset = 0;
	new_symbol_table->argument_offset = -16; /* hardcoded */
	new_symbol_table->width = 0;
	if (type == "CLASS_DECLARATION")
	{
		new_symbol_table->parent_class = parent_class;
		new_symbol_table->return_type = lexeme;
		
		/* parent class exists */
		if(!parent_class.empty()){
			/*	Whether parent class exists or not has been checked previously 
				So, in this case, must exists.
			*/
			int index = table_finder_global[parent_class];
			struct symbol_table_node *parent_class_table = (symbol_table_head->children)[index];
			new_symbol_table->current_offset = parent_class_table->current_offset;
		}
	}
	else
		new_symbol_table->return_type = return_type;
	

	return new_symbol_table;
}

void insert_argument(struct symbol_table_node *current,
					 struct ast_node *root_ast,
					 string lexeme,
					 int line_num,
					 string type)
{
	if ((current->argument_finder).count(lexeme))
	{
		cout << "Redeclartion of function argument : " << lexeme 
			<< " as a argument at line no. : " << line_num << endl;
		exit(-1);
	}
	int width = find_width(type);
	int offset = current->argument_offset;
    current->argument_offset = current->argument_offset - width;
	
	struct symbol_table_entry *entry = create_entry(current, lexeme, line_num, type, width, offset);
	
	root_ast->entry = entry;

	(current->arguments).push_back(entry);
	(current->argument_finder)[lexeme] = ((current->arguments).size() - 1);
}

void fill_argument(struct ast_node *root_ast, struct symbol_table_node *current, int type)
{
	/* FUNCTION_DEF */
	if (type == 1)
	{
		if ((root_ast->children).size() == 1 
            && ((root_ast->children)[0])->text2print == "No Parameter")
		{
			return;
		}
		for (int i = 0; i < (root_ast->children).size(); i++)
		{
			if (((root_ast->children)[i])->text2print == ":")
			{
				auto [name, type] = name_type_compute((root_ast->children)[i]);
				insert_argument(current, (root_ast->children)[i], name, ((root_ast->children)[i])->line_num, type);
			}
			else
			{
				cout << "argument in wrong format in function defition at line no." << root_ast->line_num << endl;
				exit(-1);
			}
		}
	}
	/* CLASS METHOD */
	else
	{
		if ((root_ast->children).size() == 1 
            && ((root_ast->children)[0])->text2print == "No Parameter")
		{
			cout << "self is not passed as an argument to class_method" << root_ast->line_num << endl;
			exit(-1);
		}
		for (int i = 0; i < (root_ast->children).size(); i++)
		{
			if (i == 0)
			{
				string name = name_compute(((root_ast->children)[i])->text2print);
				if (name != "self")
				{
					cout << "self is not passed as 1st argument to class_method" 
                        << root_ast->line_num << endl;
						exit(-1);
				}
				string type = (current->parent->lexeme);
				insert_argument(current, (root_ast->children)[i], name, ((root_ast->children)[i])->line_num, type);
			}
			else if (((root_ast->children)[i])->text2print == ":")
			{
				auto [name, type] = name_type_compute((root_ast->children)[i]);
				insert_argument(current, (root_ast->children)[i], name, ((root_ast->children)[i])->line_num, type);
			}
			else
			{
				cout << "argument in wrong format in method definition at line no." << root_ast->line_num << endl;
				exit(-1);
			}
		}
	}
}

struct symbol_table_node* insert_new_table(
	string lexeme,
	string parent_name,
	int definition_line_num,
	string type,
	string return_type,
	struct ast_node *root_ast,
	struct symbol_table_node *current)
{
	struct symbol_table_node *new_table = create_symbol_table_node(lexeme,
                                                        parent_name, 
                                                        definition_line_num, 
                                                        type, return_type, current);
	if (type == "FUNCTION_DEF")
		fill_argument((root_ast->children)[0], new_table, 1); // check at all directed used vector location

	else if (type == "CLASS_METHOD")
		fill_argument((root_ast->children)[0], new_table, 0); // check at all directed used vector location
	
	if (current == symbol_table_head)
		table_finder_global[lexeme] = (current->children).size();

	(current->children).push_back(new_table);
	(current->table_finder_local)[lexeme] = ((current->children).size() - 1);
	return new_table;
}

pair<struct symbol_table_node*, struct symbol_table_entry*>
lookup_class_content(string class_name, string lexeme)
{
	if (table_finder_global.count(class_name))
	{
		int index = table_finder_global[class_name];
		struct symbol_table_node *class_table = (symbol_table_head->children)[index];

		if ((class_table->entry_finder_local).count(lexeme))
		{
			index = (class_table->entry_finder_local)[lexeme];
			return {class_table, (class_table->scope_content)[index]};
		}

		if (class_table->parent_class != "")
			return lookup_class_content(class_table->parent_class, lexeme);
		
	}
	return {NULL, NULL};
}

struct symbol_table_node *populate_symbol_table(struct ast_node *root_ast,
                                                struct symbol_table_node *current)
{
	if (!root_ast)
		return current;

	/* Scenarios: where new scope or new entry needs to be created */
	if ((root_ast->text2print == ":"))
	{
		auto [name, type] = name_type_compute(root_ast); // checks for type error
		
		/*
					: <- root_ast
				   / \
				  .   int
				 / \
			 self	x
		*/
		if ((((root_ast->children)[0])->text2print) == ".")
		{
			if(current->type == "CLASS_METHOD"){
				if(current->lexeme == "__init__"){
					if(((((root_ast->children)[0])->children)[0])->nonterminal == "self"){
						insert(current->parent, root_ast, name, root_ast->line_num, type);
						(root_ast->children)[0]->entry = root_ast->entry;
						((root_ast->children)[0]->children)[0]->entry = root_ast->entry;
						((root_ast->children)[0]->children)[1]->entry = root_ast->entry;
					}
					else{
						cout << "Error on lineno.: " << root_ast->line_num
								<< " self not used to represent class\n";
						exit(-1);
					}
				}
				else{
					cout << "Error on lineno.: " << root_ast->line_num
							<< " new variable declaration outside __init__\n";
					exit(-1);
				}
			}
			else{
				cout << "Error on lineno.: " << root_ast->line_num
					<< " stray variable declaration found\n";
				exit(-1);
			}
		}
		/*
				: <- root_ast
			   / \
			  a   int
		*/
		else{
			insert(current, root_ast, name, root_ast->line_num, type);
			(root_ast->children)[0]->entry = root_ast->entry;
			(root_ast->children)[1]->entry = root_ast->entry;
		}
		
		return current;
	}
	else if (root_ast->nonterminal == "funcdef")
	{
		root_ast->what = "FUNCTION_DEF";
		string name = (root_ast->terminal);
		string return_name;
		string return_type;
		struct symbol_table_node *new_table;
		if ((root_ast->children).size() == 2 && name != "main" && name != "__init__")
		{
			cout << "no return type error for function : " 
                << name << " at line no. : " << root_ast->line_num << endl;
			exit(-1);
		}
		if ((root_ast->children).size() == 3)
		{
			return_name = name_compute(((root_ast->children)[2])->text2print);
			return_type = type_compute(((root_ast->children)[2]), return_name);
			if (current->type == "CLASS_DECLARATION")
			{
				insert(current, root_ast, name, root_ast->line_num, "CLASS_METHOD");
				root_ast->type = "CLASS_METHOD";
				new_table = insert_new_table(name, "", root_ast->line_num, "CLASS_METHOD", return_type, root_ast, current);
			}
			else
			{
				cout << current->type << "\n";
                insert(current, root_ast, name, root_ast->line_num, "FUNCTION_DEF");
				(root_ast->type) = "FUNCTION_DEF";
				new_table = insert_new_table(name, "", root_ast->line_num, "FUNCTION_DEF", return_type, root_ast, current);
			}

			return populate_symbol_table(((root_ast->children)[1]), 
                                ((current->children).back()));
		}
		else if ((root_ast->children).size() == 2)
		{
			if (current->type == "CLASS_DECLARATION")
			{
				insert(current, root_ast, name, root_ast->line_num, "CLASS_METHOD");
				root_ast->type = "CLASS_METHOD";
			}
			else
			{
				insert(current, root_ast, name, root_ast->line_num, "FUNCTION_DEF");
				root_ast->type = "FUNCTION_DEF";
			}
			if (name == "main")
			{
				new_table = insert_new_table(name, "", root_ast->line_num, "FUNCTION_DEF", "NONE", root_ast, current);
			}
			else
			{
				new_table = insert_new_table(name, "", root_ast->line_num, "CLASS_METHOD", "NONE", root_ast, current);
			}
			return populate_symbol_table(((root_ast->children)[1]), 
                                ((current->children).back()));
		}
		else
		{
			cout << "error invalid function declaration : " 
                << name << " at line no. : " << root_ast->line_num << endl;
			exit(-1);
		}
		cout << "============== exit ==============\n";
	}
	else if (root_ast->nonterminal == "classdef")
	{
		/*
			Without inheritance
			-------------------
			
			class A:
				body
						-----------
						| class   |
						| NAME: A |
						-----------
							 |
						   suite
			
			With inheritance
			----------------

			class B (A):
				body
						------------------------
						| class   			   |
						| NAME: B 			   |
						| Inherited from class |
						| NAME: A              |
						------------------------
							 		|
						   		  suite
		*/
		root_ast->what = "CLASS_DECLARATION";
		root_ast->type = "CLASS_DECLARATION";
		
		string class_name = root_ast->terminal;
		string parent_class_name = "";
		
		int pos = (root_ast->text2print).find(':');
		pos = (root_ast->text2print).find(':', pos + 1);
		if (pos >= 0 && pos < (root_ast->text2print).size())
			parent_class_name = (root_ast->text2print).substr(pos + 2);
		
		insert(current, root_ast, class_name, root_ast->line_num, "CLASS_DECLARATION");
		insert_new_table(class_name, parent_class_name, root_ast->line_num,
                   "CLASS_DECLARATION", "", root_ast, current); /* current is global symbol table */
		return populate_symbol_table((root_ast->children)[0],
                               (current->children).back());
	}
	else if (root_ast->text2print == ".")
	{
		string name = (root_ast->children[0])->nonterminal;
		string class_content = (root_ast->children[1])->nonterminal;
		
		/*
							. <- root_ast
				   		   / \
			   name -> self	  x <- class_content
		*/
		if (name == "self")
		{	
			if (current->type != "CLASS_METHOD")
			{
				cout << "self used other than class_method"
					 << " at line no. : " << root_ast->line_num << endl;
				exit(-1);
			}

			auto [class_table, entry] = lookup_class_content(current->parent->lexeme, class_content);
			if(class_table){
				// self.get_x()
				if((class_table->table_finder_local).count(class_content)){
					int index = (class_table->table_finder_local)[class_content];
					struct symbol_table_node *method_table = (class_table->children)[index];

					root_ast->entry = entry;
					root_ast->type = method_table->return_type;
					root_ast->width = find_width(root_ast->type);
					root_ast->offset = -16; /* hardcoded */
					root_ast->what = "SELF_METHOD_CALL";
					// saving class name in terminal
					root_ast->terminal = class_table->lexeme;
				
					/* Moving on to arguments passed */
					if(((root_ast->children)[1]->children).empty()){
						cout << "Error on lineno. " << root_ast->line_num
							<< " ( ) missing around the method call: " << class_content << "\n";
						exit(-1);
					}
					if(((root_ast->children)[1]->children)[0]->text2print != "( )"){
						for(int i = 0; i < ((root_ast->children)[1]->children).size(); i++)
							populate_symbol_table(((root_ast->children)[1]->children)[i], current);
						
					}
				}
				// self.x
				else{
					root_ast->entry = entry;
					(root_ast->children)[0]->entry = entry;
					(root_ast->children)[1]->entry = entry;
					root_ast->type = entry->type;
					root_ast->width = entry->width;
					root_ast->offset = entry->offset;
					root_ast->what = "SELF_VARIABLE_ACCESS";
				}
			}
			else{
				cout << "Error on lineno. " << root_ast->line_num
				<< " class feature: " << class_content << " without declaration\n";
				exit(-1);
			}
		}
		/*
			class A:
				def __init__(self, x: int):
					self.x : int = x

			class B (A):
				def __init__(self, x: int, y: int):
					A.__init__(self, x) <- this case
					self.y : int = y
		*/
		else if ((table_finder_global).count(name)
				 && ((symbol_table_head->children)[table_finder_global[name]])->type == "CLASS_DECLARATION")
		{
			if (current->type != "CLASS_METHOD"){
				cout << "Error on lineno.: " << root_ast->line_num
					<< " stray method call\n";
				exit(-1);
			}

			int index = table_finder_global[name];
			struct symbol_table_node *class_table = (symbol_table_head->children)[index];

			if((class_table->table_finder_local).count(class_content)
				&& (class_table->children)[(class_table->table_finder_local)[class_content]]->type == "CLASS_METHOD")
			{
				index = (class_table->table_finder_local)[class_content];
				struct symbol_table_node *method_table = (class_table->children)[index];

				index = (class_table->entry_finder_local)[class_content];
				struct symbol_table_entry *method_entry = (class_table->scope_content)[index];

				root_ast->entry = method_entry;
				root_ast->type = method_table->return_type;
				root_ast->width = find_width(root_ast->type);
				root_ast->offset = -16; /* hardcoded */
				root_ast->what = "PARENT_METHOD_CALL";
				// saving class name in terminal
				root_ast->terminal = name;
				
				if(((root_ast->children)[1]->children).empty()){
					cout << "Error on lineno. " << root_ast->line_num
						<< " ( ) missing around the method call: " << class_content << "\n";
					exit(-1);
				}
				if(((root_ast->children[1])->children)[0]->text2print != "( )"){
					/* Moving on to arguments passed */
					for (int i = 0; i < ((root_ast->children[1])->children).size(); i++)
						populate_symbol_table(((root_ast->children[1])->children)[i], current);
				}
			}
			else{
				cout << "Error on lineno. " << root_ast->line_num
				<< " method: " << class_content << " doesn't exist in the class: " << name << "\n";
				exit(-1);
			}
		}
		/* 	
			class A:
				def __init__(self, x):
					self.x : int = x
				
				def get_x(self) -> int:
					return self.x

			def main():
				a: A = A();
				a.x = 5; 			<-| these two cases
				x: int = a.get_x(); <-|
		*/
		else if (	(current->entry_finder_local).count(name)
				||	(current->argument_finder).count(name)
				)
		{
			int index;
			struct symbol_table_entry *object_entry;

			if((current->entry_finder_local).count(name)){
				index = (current->entry_finder_local)[name];
				object_entry = (current->scope_content)[index];
			}
			else{
				index = (current->argument_finder)[name];
				object_entry = (current->arguments)[index];
			}

			string class_name = object_entry->type;

			auto [class_table, entry] = lookup_class_content(class_name, class_content);
			if(class_table){ 
				// a.get_x()
				if((class_table->table_finder_local).count(class_content)){
					int index = (class_table->table_finder_local)[class_content];
					struct symbol_table_node *method_table = (class_table->children)[index];
					
					root_ast->entry = entry;
					root_ast->type = method_table->return_type;
					root_ast->width = find_width(root_ast->type);
					root_ast->offset = object_entry->offset;
					root_ast->what = "OBJECT_METHOD_CALL";
					root_ast->terminal = class_name;

					if(((root_ast->children)[1]->children).empty()){
						cout << "Error on lineno. " << root_ast->line_num
							<< " ( ) missing around the method call: " << class_content << "\n";
						exit(-1);
					}
					if(((root_ast->children[1])->children)[0]->text2print != "( )"){
						/* Moving on to arguments passed */
						for (int i = 0; i < ((root_ast->children[1])->children).size(); i++)
							populate_symbol_table(((root_ast->children[1])->children)[i], current);
					}
				}
				// a.x
				else{
					root_ast->entry = entry;
					(root_ast->children)[0]->entry = entry;
					(root_ast->children)[1]->entry = entry;
					root_ast->type = entry->type;
					root_ast->width = find_width(entry->type);
					root_ast->offset = entry->offset;
					root_ast->what = "OBJECT_VARIABLE_ACCESS";
				}
			}
			else
			{
				cout << "Error on lineno. " << root_ast->line_num
				<< "variable or method: " << class_content << " doesn't exits in the class" << class_name << endl;
				exit(-1);
			}
		}
		else
		{
			cout << "Error on lineno. " << root_ast->line_num
			<< name << " has not been declared\n";
			exit(-1);
		}
		return current;
	}
	else if (root_ast->terminal == "NAME" || root_ast->terminal == "DUNDER")
	{ 
		string name = root_ast->nonterminal;
		if ((current->argument_finder).count(name))
		{
			int index = (current->argument_finder)[name];
			struct symbol_table_entry *entry = (current->arguments)[index];

			root_ast->type = entry->type;
			root_ast->offset = entry->offset;
			root_ast->width = entry->width;
			root_ast->entry = entry;
			root_ast->what = "IDENTIFIER";
		}
		else if ((current->entry_finder_local).count(name))
		{
			int index = (current->entry_finder_local)[name];
			struct symbol_table_entry *entry = (current->scope_content)[index];

			root_ast->type = entry->type;
			root_ast->offset = entry->offset;
			root_ast->width = entry->width;
			root_ast->entry = entry;
			root_ast->what = "IDENTIFIER";
		}
		else if ((entry_finder_global).count(name))
		{
			if (((symbol_table_head->scope_content)[entry_finder_global[name]])->type == "FUNCTION_DEF")
			{
				root_ast->entry = (symbol_table_head->scope_content)[entry_finder_global[name]];
				root_ast->what = "FUNCTION_CALL";
			}
			else if (((symbol_table_head->scope_content)[entry_finder_global[name]])->type == "CLASS_DECLARATION")
			{
				int index = table_finder_global[name]; 
				struct symbol_table_node *class_table = (symbol_table_head->children)[index];

				/*	In case of a class, the width of an object is stored in the current_offset
					I have filled argument width of __init__ in offset for later use
					In this case, entry is irrelevant.
				*/
				root_ast->type = class_table->lexeme;
				root_ast->width = class_table->current_offset;

				index = (class_table->entry_finder_local)["__init__"];
				struct symbol_table_entry *init_entry = (class_table->scope_content)[index];
				
				index = (class_table->table_finder_local)["__init__"];
				struct symbol_table_node *init_table =
						(class_table->children)[index];

				root_ast->offset = -(init_table->argument_offset + 16); /* + 16 to take into account initial argument offset = -16 */
				
				root_ast->entry = init_entry;
				root_ast->what = "OBJECT_INSTANTIATION";
			}
			else
			{
				int index = entry_finder_global[name];
				struct symbol_table_entry *entry = (symbol_table_head->scope_content)[index];

				root_ast->type = entry->type;
				root_ast->offset = entry->offset;
				root_ast->width = entry->width;
				root_ast->entry = entry;
				root_ast->what = "IDENTIFIER";
			}
		}
		else if (name == "range" || name == "len")
		{ // hardcoded to change
			root_ast->type = "INT";
		}
		else if (name == "print")
		{
			root_ast->type = "NONE";
		}
		else if (name == "__name__" || name == "__init__" || name == "__main__")
		{ // hardcoded to change
			root_ast->type = "STR";
			return current;
		}
		else
		{
			cout << "error used before declaration : " 
                << name << " at line no. : " << root_ast->line_num << endl;
			exit(-1);
		}
	}
	else if (root_ast->text2print == "list [ ]")
	{
		root_ast->what = "LIST_DECLARATION";
		root_ast->width = find_width((root_ast->children)[0]->type)*(root_ast->children).size();
	}

	for (int i = 0; i < (root_ast->children).size(); i++)
		populate_symbol_table((root_ast->children)[i], current);
	
	return current;
}

string tab = "";

void print_symbol_table(struct symbol_table_node *symbol_table)
{
	fstream fout;

	// opens an existing csv file or creates a new file
	for (int i = 0; i < (symbol_table->scope_content).size(); i++)
	{
		string lexeme = ((symbol_table->scope_content)[i])->lexeme;
		string type = ((symbol_table->scope_content)[i])->type;
		if (type == "CLASS_DECLARATION")
		{
			int index = (symbol_table->table_finder_local)[lexeme];
			struct symbol_table_node *table = (symbol_table->children)[index];
			// print_symbol_table_node(table);
			
			fout.open("dump_symbol_table.csv", ios::out | ios::app);
			fout << table->lexeme << ", "
				 << "CLASS" << ", "
				 << table->definition_line_num << ", "
				 << table->argument_offset << ", "
				 << table->current_offset << ", "
				 << table->return_type << ", "
				 << "-\n";
			fout.close();
			tab = tab + "\t";
			print_symbol_table((symbol_table->children)[(symbol_table->table_finder_local)[lexeme]]);
			tab.pop_back();
		}
		else if (type == "FUNCTION_DEF")
		{
			fout.open("dump_symbol_table.csv", ios::out | ios::app);
			int index = (symbol_table->table_finder_local)[lexeme];
			struct symbol_table_node *table = (symbol_table->children)[index];
			// print_symbol_table_node(table);
			fout << table->lexeme << ", "
				 << "FUNCTION" << ", "
				 << table->definition_line_num << ", "
				 << table->argument_offset << ", "
				 << table->current_offset << ", "
				 << table->return_type << ", "
				 << "-\n";
			tab = tab + "\t";
			for (int i = 0; i < (table->arguments).size(); i++)
			{
				fout << ((table->arguments)[i])->lexeme << ", " 
                    << ((table->arguments)[i])->type << ", "
					<< ((table->arguments)[i])->line_num  << ", "
					<< ((table->arguments)[i])->offset << ", "
					<< "-, -, -\n";
			}
			fout.close();
			print_symbol_table(table);
			tab.pop_back();
		}
		else if (type == "CLASS_METHOD")
		{
			fout.open("dump_symbol_table.csv", ios::out | ios::app);
			struct symbol_table_node *table = (symbol_table->children)[(symbol_table->table_finder_local)[lexeme]];
			// print_symbol_table_node(table);
			fout << table->lexeme << ", "
				 << "CLASS_METHOD" << ", "
				 << table->definition_line_num << ", "
				 << table->argument_offset << ", "
				 << table->current_offset << ", "
				 << table->return_type << ", "
				 << "-\n";
			tab = tab + "\t";
			for (int i = 0; i < (table->arguments).size(); i++)
			{
				fout << ((table->arguments)[i])->lexeme << ", " 
                    << ((table->arguments)[i])->type << ", "
					<< ((table->arguments)[i])->line_num  << ", "
					<< ((table->arguments)[i])->offset << ", "
					<< "-, -, -\n";
			}
			fout.close();
			print_symbol_table(table);
			tab.pop_back();
		}
		else
		{
			fout.open("dump_symbol_table.csv", ios::out | ios::app);
			struct symbol_table_entry *temp = (symbol_table->scope_content)[i];
			fout << temp->lexeme << ", " 
                << temp->type << ", "
				<< temp->line_num  << ", "
				<< temp->offset << ", "
				<< "-, -, -\n";
			fout.close();
		}
	}
}

void dump_symbol_table(struct symbol_table_node *symbol_table)
{
	fstream fout;
	// opens an existing csv file or creates a new file.
	fout.open("dump_symbol_table.csv", ios::out);
	fout << "lexeme,type,definition_line_num,argument_offset,"
			<< "current_offset,return_type,no_of_arguments\n";
	fout.close();
	print_symbol_table(symbol_table);
}

void typecast(struct ast_node *node, string type)
{
	/*
	 *
	 *		parent         parent
	 *		  |			     |
	 * node-(INT)   ->  node-(float())
	 *						 |
	 *                   temp-(INT)
	 *
	 */

	/* temp is a copy of node */
	struct ast_node *temp = new ast_node;
	*temp = *node;
	
	node->line_num = temp->line_num;
	node->type = type;          	
	node->width = find_width(type);
	node->register_width = 32;
	node->what = "typecast";
	node->terminal = "";	   
	node->text2print = temp->type + "2" + type;    
	node->nonterminal = "typecast";   
	node->children = {temp};

}

void populate_type(struct ast_node *current_node)
{
	/* Assumption: terminals contain their type */
	if (!current_node)
		return;

	/* Populate child */
	for (int i = 0; i < (current_node->children).size(); i++)
	{
		populate_type((current_node->children)[i]);
	}

	string node_value = current_node->text2print;
	if(node_value == "NAME: print"){
		if((current_node->children).size()!=1){
			cout << "Wrong number of argument given to print function , No. of argument given: " << (current_node->children).size() << endl;
			cout << "AT line no: " << current_node->line_num << endl;
			exit(-1);
		}
		if(!((current_node->children)[0]->type == "INT" ||
		 (current_node->children)[0]->type == "BOOL" ||
		  (current_node->children)[0]->type == "STR")){
			cout << "Wrong argument type given to print function , argument type given: " << (current_node->children)[0]->type << endl;
			cout << "AT line no: " << current_node->line_num << endl;
			exit(-1);
		}
	}
	else if(node_value == "NAME: range"){
		if((current_node->children).size()<1 || (current_node->children).size()>2){
			cout << "Wrong number of argument given to range function , No. of argument given: " << (current_node->children).size() << endl;
			cout << "AT line no: " << current_node->line_num << endl;
			exit(-1);
		}
		if((current_node->children).size() == 1 && (current_node->children)[0]->type != "INT"){
			cout << "Wrong argument type given to range function , argument type given: " << (current_node->children)[0]->type << endl;
			cout << "AT line no: " << current_node->line_num << endl;
			exit(-1);
		}
		if((current_node->children).size() == 2 ){
			if((current_node->children)[0]->type != "INT"){
				cout << "Wrong start argument type given to range function , argument type given: " << (current_node->children)[0]->type << endl;
				cout << "AT line no: " << current_node->line_num << endl;
				exit(-1);
			}
			if((current_node->children)[1]->type != "INT"){
				cout << "Wrong end argument type given to range function , argument type given: " << (current_node->children)[1]->type << endl;
				cout << "AT line no: " << current_node->line_num << endl;
				exit(-1);
			}	
		}
	}
	else if(node_value == "NAME: len"){
		if((current_node->children).size()!=1){
			cout << "Wrong number of argument given to len function , No. of argument given: " << (current_node->children).size() << endl;
			cout << "AT line no: " << current_node->line_num << endl;
			exit(-1);
		}
		if(((current_node->children)[0]->type).substr(0,4) != "LIST"){
			cout << "Wrong argument type given to len function , argument type given: " << (current_node->children)[0]->type << endl;
			cout << "AT line no: " << current_node->line_num << endl;
			exit(-1);
		}
	}
	else if (node_value == "ASSIGNMENT OPERATOR: =")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		if (type1.substr(0, 4) == "LIST")
		{
			if (((type1 == "LIST_FLOAT") || (type1 == "LIST_BOOL")) && type2 == "LIST_INT")
			{
				child2->type = child1->type;
			}
			else if ((type1 == "LIST_INT") && type2 == "LIST_FLOAT")
			{
				child2->type = child1->type;
			}
			else if ((type1 == "LIST_INT") && type2 == "LIST_BOOL")
			{
				child2->type = child1->type;
			}
			else if (type1 == type2)
			{
				child2->type = child1->type;
			}
			else
			{
				cout << "TypeError at line number: " << current_node->line_num << endl;
				cout << "LIST type mismatch "
					 << "expected type:" << type1 << " actual type: " << type2 << endl;
				exit(-1);
			}
		}
		else if (type1 == type2)
		{
			current_node->type = type1;
            current_node->width = child1->width;
		}
		else if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
            current_node->width = 8;
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
            current_node->width = 4;
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child2 into float */
			typecast(child2, "FLOAT");
			current_node->type = "FLOAT";
            current_node->width = 8;
            current_node->register_width = 64;
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child2 into bool */
			typecast(child2, "BOOL");
			current_node->type = "BOOL";
            current_node->width = 4;
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;

			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: +")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;

        current_node->register_width = 32;

		if (type1 == type2 && (type1 == "INT" 
            || type1 == "FLOAT" || type1 == "BOOL"))
		{
			current_node->type = type1;
		}
		else if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child1 into float */
			typecast(child1, "FLOAT");
			current_node->type = "FLOAT";
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child2 into float */
			typecast(child2, "FLOAT");
			current_node->type = "FLOAT";
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			current_node->type = "INT";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;

			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: -" 
        || node_value == "ARITHMETIC OPERATOR: *" 
        || node_value == "ARITHMETIC OPERATOR: /")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;

        current_node->register_width = 32;

		if (type1 == type2 
            && (type1 == "INT" || type1 == "FLOAT" || type1 == "BOOL"))
		{
			current_node->type = type1;
		}
		else if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child1 into float */
			typecast(child1, "FLOAT");
			current_node->type = "FLOAT";
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child2 into float */
			typecast(child2, "FLOAT");
			current_node->type = "FLOAT";
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			current_node->type = "INT";
		}
		else
		{
			cout << "TypeError at line number: " << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: //")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;


        current_node->register_width = 32;

		if (type1 == type2 
            && (type1 == "INT" || type1 == "FLOAT" || type1 == "BOOL"))
		{
			current_node->type = "INT";
		}
		else if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			current_node->type = "INT";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: %")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;

        current_node->register_width = 32;

		if (type1 == "BOOL")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			type1 = "INT";
		}
		if (type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			type2 = "INT";
		}
		if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " 
                << current_node->line_num << endl;

			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: **")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;

        current_node->register_width = 32;

		if (type1 == "BOOL")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			type1 = "INT";
		}
		if (type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			type2 = "INT";
		}
		if ((type1 == "INT" || type1 == "FLOAT") && type2 == "INT")
		{
			current_node->type = type1;
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;

			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "RELATIONAL OPERATOR: ==" 
        || node_value == "RELATIONAL OPERATOR: !=" 
        || node_value == "RELATIONAL OPERATOR: <" 
        || node_value == "RELATIONAL OPERATOR: >" 
        || node_value == "RELATIONAL OPERATOR: <=" 
        || node_value == "RELATIONAL OPERATOR: >=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;

        current_node->register_width = 32;

		if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child1 into float */
			typecast(child1, "FLOAT");
			type1 = "FLOAT";
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			type2 = "INT";
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child2 into float */
			typecast(child2, "FLOAT");
			type2 = "FLOAT";
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			type1 = "INT";
		}
		if (type1 == type2)
		{
			current_node->type = "BOOL";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (node_value == "LOGICAL OPERATOR: and" 
        || node_value == "LOGICAL OPERATOR: or")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" || type1 == "FLOAT")
		{
			/* typecast child1 into bool */
			typecast(child1, "BOOL");
			type1 = "BOOL";
		}
		if (type2 == "INT" || type2 == "FLOAT")
		{
			/* typecast child2 into bool */
			typecast(child2, "BOOL");
			type2 = "BOOL";
		}
		if (type1 == "BOOL" && type2 == "BOOL")
		{
			current_node->type = "BOOL";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;

			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "LOGICAL OPERATOR: not")
	{
		struct ast_node *child1 = (current_node->children)[0];
		string type1 = child1->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" || type1 == "FLOAT")
		{
			/* typecast child1 into bool */
			typecast(child1, "BOOL");
			type1 = "BOOL";
		}
		if (type1 == "BOOL")
		{
			current_node->type = "BOOL";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " type" << endl;
			exit(-1);
		}
	}
	else if (node_value == "BITWISE OPERATOR: &" 
        || node_value == "BITWISE OPERATOR: |" 
        || node_value == "BITWISE OPERATOR: ^" 
        || node_value == "BITWISE OPERATOR: ~" 
        || node_value == "BITWISE OPERATOR: <<" 
        || node_value == "BITWISE OPERATOR: >>")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "BOOL")
		{
			/* typecast child1 into int */
			typecast(child1, "INT");
			type1 = "INT";
		}
		if (type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			type2 = "INT";
		}
		if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (	(current_node->terminal == "NAME" 
        	||	current_node->terminal == "DUNDER") 
        	&&	(current_node->type).substr(0, 4) == "LIST")
	{
		current_node->what = "LIST_DEREFERENCE";

		if ((current_node->children).empty())
			return;

		struct ast_node *child = (current_node->children)[0];
		string child_type = child->type;
		if (child_type != "INT")
		{
			cout << "TypeError at line number: " << current_node->line_num << endl;
			cout << "Dereferencing"
				 << " is not supported over " << child_type << endl;
			exit(-1);
		}
		current_node->type = (current_node->type).substr(5); /* LIST_INT -> INT and so on */
	}
	else if (node_value == "UNARY OPERATOR: +" 
        || node_value == "UNARY OPERATOR: -" 
        || node_value == "UNARY OPERATOR: ~")
	{
		struct ast_node *child = (current_node->children)[0];
		string child_type = child->type;

        current_node->register_width = 32;

		if (child_type == "INT" || child_type == "FLOAT")
		{
			current_node->type = child_type;
		}
		else
		{
			cout << "TypeError at line number: " << current_node->line_num << endl;
			cout << node_value << " is not supported for " << child_type << endl;
			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: +=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == type2 && (type1 == "INT" || type1 == "FLOAT" 
            || type1 == "BOOL"))
		{
			current_node->type = type1;
		}
		else if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child2 into float */
			typecast(child2, "FLOAT");
			current_node->type = "FLOAT";
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child2 into bool */
			typecast(child2, "BOOL");
			current_node->type = "BOOL";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (  node_value == "ARITHMETIC OPERATOR: -=" 
            || node_value == "ARITHMETIC OPERATOR: *=" 
            || node_value == "ARITHMETIC OPERATOR: /=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == type2 
            && (type1 == "INT" || type1 == "FLOAT" || type1 == "BOOL"))
		{
			current_node->type = type1;
		}
		else if (type1 == "INT" && type2 == "FLOAT")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "INT" && type2 == "BOOL")
		{
			/* typecast child2 into int */
			typecast(child2, "INT");
			current_node->type = "INT";
		}
		else if (type1 == "FLOAT" && type2 == "INT")
		{
			/* typecast child2 into float */
			typecast(child2, "FLOAT");
			current_node->type = "FLOAT";
		}
		else if (type1 == "BOOL" && type2 == "INT")
		{
			/* typecast child2 into bool */
			typecast(child2, "BOOL");
			current_node->type = "BOOL";
		}
		else
		{
			cout << "TypeError at line number: " 
                << current_node->line_num << endl;

			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: %=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: &=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: |=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;

			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: ^=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: <<=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (node_value == "ARITHMETIC OPERATOR: >>=")
	{
		struct ast_node *child1 = (current_node->children)[0];
		struct ast_node *child2 = (current_node->children)[1];
		string type1 = child1->type;
		string type2 = child2->type;
		
        current_node->register_width = 32;

        if (type1 == "INT" && type2 == "INT")
		{
			current_node->type = "INT";
		}
		else
		{
			cout << "typeerror at line number: " 
                << current_node->line_num << endl;
			cout << node_value << " is not supported for " 
                << type1 << " and " << type2 << " types" << endl;
			exit(-1);
		}
	}
	else if (current_node->what == "FUNCTION_CALL")
	{
		string name = current_node->nonterminal;
		struct symbol_table_node *temp = 
            ((symbol_table_head->children)[table_finder_global[name]]);
		
        current_node->register_width = (find_width(temp->return_type))*8;

        if((current_node->children).empty()){
			cout << "Error on lineno. " << current_node->line_num
				<< " ( ) missing around the function call: " << current_node->nonterminal << "\n";
			exit(-1);
		}
		int root_child_no;
		if (((current_node->children)[0])->text2print == "( )")
		{
			root_child_no = 0;
		}
		else
		{
			root_child_no = (current_node->children).size();
		}
		if (root_child_no != (temp->arguments).size())
		{
			cout << "No. of parameters does not match for function: " 
                << temp->lexeme << " at line no. : " << current_node->line_num << endl;
			exit(-1);
		}
		for (int i = 0; i < (temp->arguments).size(); i++)
		{
			struct ast_node *child2 = (current_node->children)[i];
			string type1 = ((temp->arguments)[i])->type;
			string type2 = ((current_node->children)[i])->type;
			if (type1 == type2)
			{
				continue; // current_node->type = type1;
			}
			else if (type1 == "INT" && type2 == "FLOAT")
			{
				/* typecast child2 into int */
				typecast(child2, "INT");
			}
			else if (type1 == "INT" && type2 == "BOOL")
			{
				/* typecast child2 into int */
				typecast(child2, "INT");
			}
			else if (type1 == "FLOAT" && type2 == "INT")
			{
				/* typecast child2 into float */
				typecast(child2, "FLOAT");
			}
			else if (type1 == "BOOL" && type2 == "INT")
			{
				/* typecast child2 into bool */
				typecast(child2, "BOOL");
			}
			else
			{
				cout << "TypeError at line number: " << current_node->line_num << endl;
				cout << i + 1 << "-th argument type mismatch "
					 << "expected type:" << type1 << " actual type: " << type2 << endl;
				exit(-1);
			}
		}
		current_node->type = temp->return_type;
	}
	else if (current_node->what == "OBJECT_INSTANTIATION")
	{
		string name = current_node->nonterminal;
		struct symbol_table_node *temp = 
            ((symbol_table_head->children)[table_finder_global[name]]);
		temp = ((temp->children)[(temp->table_finder_local)["__init__"]]);
		int root_child_no;
		if (((current_node->children)[0])->text2print == "( )")
		{
			root_child_no = 0;
		}
		else
		{
			root_child_no = (current_node->children).size();
		}
		if (root_child_no != ((temp->arguments).size() - 1))
		{
			cout << "No. of parameters does not match for object: " 
                << name << " at line no. : " << current_node->line_num << endl;
			exit(-1);
		}
		for (int i = 1; i < (temp->arguments).size(); i++)
		{
			struct ast_node *child2 = (current_node->children)[i - 1];
			string type1 = ((temp->arguments)[i])->type;
			string type2 = ((current_node->children)[i - 1])->type;
			if (type1 == type2)
			{
				continue; // current_node->type = type1;
			}
			else if (type1 == "INT" && type2 == "FLOAT")
			{
				/* typecast child2 into int */
				typecast(child2, "INT");
			}
			else if (type1 == "INT" && type2 == "BOOL")
			{
				/* typecast child2 into int */
				typecast(child2, "INT");
			}
			else if (type1 == "FLOAT" && type2 == "INT")
			{
				/* typecast child2 into float */
				typecast(child2, "FLOAT");
			}
			else if (type1 == "BOOL" && type2 == "INT")
			{
				/* typecast child2 into bool */
				typecast(child2, "BOOL");
			}
			else
			{
				cout << "TypeError at line number: " << current_node->line_num << endl;
				cout << i << "-th argument type mismatch "
					 << "expected type:" << type1 << " actual type: " << type2 << endl;
				exit(-1);
			}
		}
		current_node->type = name;
	}
	else if (	current_node->what == "PARENT_METHOD_CALL"
			||	current_node->what == "SELF_METHOD_CALL"
			||	current_node->what == "OBJECT_METHOD_CALL"
			)
	{
		/*
			class_name.method_name -> In this case, self is passed as an argument
			self.method_name -> In this case, self is not passed as an argument
			object_name.method_name -> In this case, self is not passed as an argument
		*/
		string class_name = current_node->terminal;
		string method_name = (current_node->children)[1]->nonterminal;
		
		struct symbol_table_node *class_table =
            (symbol_table_head->children)[table_finder_global[class_name]];
		
		struct symbol_table_node *method_table =
			(class_table->children)[(class_table->table_finder_local)[method_name]];
		
		vector<struct ast_node*> parameters = (current_node->children)[1]->children;
	
		if(current_node->what == "PARENT_METHOD_CALL"){
			if(parameters[0]->text2print != "NAME: self"){
				cout << "Error on lineno. " << current_node->line_num << ": "
				<< "Class method: " << method_name << " called without passing self as 1st argument\n";
				exit(-1);
			}

			int nparameters = parameters.size();
			
			/* Checking no. of parameters */
			if(nparameters != (method_table->arguments).size()){
				cout << "Error on lineno. " << current_node->line_num
				<< " no. of passed parameters not matching for method call: " << method_name
				<< " of class: " << class_name << "\n";
				exit(-1);
			}

			/*
				Checking type of parameters
				Skipping i = 0 corresponding to self
			*/
			for(int i = 1; i < nparameters; i++){ 
				if(parameters[i]->type != (method_table->arguments)[i]->type){
					cout << "Error on lineno. " << current_node->line_num << ": "
					<< i << "-th argument type mismatch\n"
					<< "Expected type: " << (method_table->arguments)[i]->type << " "
					<< "Passed type: " << parameters[i]->type << "\n";
					exit(-1);
				}
			}
		}
		else{
			int nparameters;

			if (parameters[0]->text2print == "( )")
				nparameters = 0;
			else
				nparameters = parameters.size();
			
			/*
				Checking no. of parameters
				-1 to exclude self
			*/
			if(nparameters != (method_table->arguments).size() - 1){
				cout << "Error on lineno. " << current_node->line_num
				<< " no. of passed parameters not matching for method call: " << method_name
				<< " of class: " << class_name << "\n";
				exit(-1);
			}

			/*
				Checking type of parameters
				(i + 1) to take self into account
			*/
			for(int i = 0; i < nparameters; i++){ 
				if(parameters[i]->type != (method_table->arguments)[i + 1]->type){
					cout << "Error on lineno. " << current_node->line_num << ": "
					<< (i + 1) << "-th argument type mismatch\n"
					<< "Expected type: " << (method_table->arguments)[i + 1]->type << " "
					<< "Passed type: " << parameters[i]->type << "\n";
					exit(-1);
				}
			}
		}
	}
	else if (current_node->what == "LIST_DECLARATION")
	{
		(current_node->type) = ("LIST_" + ((current_node->children)[0])->type);
	}
	/* class method call
	 *
	 *			  .
	 *			 / \
	 *	class_name  function_name
	 *					|
	 *				arguments
	 *
	 */
	/* function call */
	/* object instantiation */
}




struct address_node* new_address_node(string type,
                                      string value,
                                      int register_width = 32)
{
    struct address_node* temp = new address_node;

    temp->type = type;
    temp->value = value;
    temp->register_width = register_width;
	temp->live = false;

    return temp;
}

struct address_node* new_temporary_address(struct symbol_table_node *current_scope,
											struct ast_node *current_node)
{
	string temporary_address = "t_" + to_string(universal_temporary_address_count);
	universal_temporary_address_count++;
	struct address_node *temp = new_address_node("temporary", temporary_address, current_node->register_width);

    //cout << "temporary: " << temporary_address << endl;
    //cout << "width passed: " << current_node->register_width << endl;

	return temp;
}
struct quad_node* new_goto_quad(string label)
{
	return new_quad("Goto", new_address_node("label", label), NULL, NULL);
}

// data = mem_alloc total_width 
struct quad_node* new_memalloc_quad(int memory,
                            struct address_node* array_base)
{
    struct address_node* total_width = new address_node;
    total_width->value = to_string(memory);
    total_width->type = "constant";
    total_width->offset = memory;

    return new_quad("mem_alloc",total_width,NULL,array_base);
}

struct quad_node* new_label_quad(struct symbol_table_node *current_scope,
								string label = "",
								struct address_node *constant = NULL)
{
	if(label.empty()){
		label = "L_" + to_string(universal_label_count);
		universal_label_count++;
	}
    else{
        label = "L_" + label;
    }
	return new_quad(label, constant, NULL, new_address_node("label", label));
}

/* 
		ifZ a goto L_1
			|		|
	  address		label
*/
struct quad_node* new_ifZ_quad(struct address_node *address, struct address_node *label)
{
	return new_quad("ifZ", address, NULL, label);
}

/* call func_name */
struct quad_node *new_call_quad(string func_name)
{
	return new_quad("call", new_address_node("label", func_name), NULL, NULL);
}

/* address = pop */
struct quad_node *new_pop_quad(struct address_node *address)
{
	return new_quad("pop_return_value", NULL, NULL, address);
}

struct quad_node *new_increment_rsp_quad(int width)
{
    struct address_node* dummy_addr = new address_node;
    dummy_addr->type = "INT";
    dummy_addr->value = to_string(width);

    return new_quad("increment_rsp",dummy_addr,NULL,NULL);
}

/* push address */
struct quad_node *new_push_quad(struct address_node *address)
{
	return new_quad("push_param", address, NULL, NULL);
}

/* used for returning values from function */
struct quad_node *new_return_quad(struct address_node *address)
{
	return new_quad("return_value", address, NULL, NULL);
}

/* result = arg1 op arg2 */
struct quad_node *new_operator_quad(string op,
									struct address_node *arg1,
									struct address_node *arg2,
									struct address_node *result)
{
	return new_quad(op, arg1, arg2, result);
}

struct address_node* new_constant_address(string type, string value, int register_width)
{
	return new_address_node(type, value, register_width);
}

/* Returns true only for binary operators */
bool is_operator(string str);

void merge(vector<struct quad_node *> &code1, vector<struct quad_node *> &code2)
{
	code1.insert(code1.end(), code2.begin(), code2.end());
}

map<string,int> no_of_element_list;
/*	
	object.variable
	For example:
		1. self.x
		2. a.x
*/
void class_variable_access(struct ast_node *object,
							struct ast_node *variable,
							struct ast_node *current_node,
							struct symbol_table_node *current_scope)
{
	vector<struct quad_node*> &current_code = current_node->code;
	/*
		3AC to access data
		------------------
		current_node->address = object->address[variable->entry->offset]

		3AC to access address
		---------------------
		current_node->laddress = object->address + variable->entry->offset
	*/

	current_node->register_width = ((current_node->entry)->width)*8;	/* hardcoded */

	current_node->address = new_temporary_address(current_scope, current_node);

assert(object->address);
    assert(current_node->address);
	current_code.push_back(new_quad("[obj]",
			make_copy_of_address(object->address),
			new_constant_address("INT", to_string(current_node->entry->offset - current_node->entry->width), 32),
			make_copy_of_address(current_node->address)));

    current_node->register_width = 64; //hardcoded
	current_node->laddress = new_temporary_address(current_scope, current_node);
assert(object->address);
    assert(current_node->laddress);
	current_code.push_back(new_operator_quad("+",
				make_copy_of_address(object->address),
				new_constant_address("INT", to_string(current_node->entry->offset - current_node->entry->width), 32),
				make_copy_of_address(current_node->laddress)));
}

/*
	Cases:
		1. PARENT_METHOD_CALL
		2. SELF_METHOD_CALL
		3. OBJECT_METHOD_CALL
*/
void class_method_call(struct ast_node *object,
						struct ast_node *method,
						struct ast_node *current_node,
						struct symbol_table_node *current_scope)
{

	/*
		push param_(n - 1)
				:
		push param_1
		push param_0 (only in 3rd case because in 1st and 2nd case param_0 is self which we are explicitly pushing anyways)
		push self
		call method
	*/
	vector<struct quad_node*> &current_code = current_node->code;	
	vector<struct ast_node*> &children = current_node->children;

	current_node->register_width = (current_node->width)*8;

	string method_name = method->nonterminal;
	
	// current_node->entry will contain entry of the method in the symbol table of the class
	string class_name = current_node->entry->scope->lexeme;

	vector<struct ast_node*> &parameters = children[1]->children;
	int nparameters = parameters.size();
	
	int total_args_width = 8; /* 8 bytes corresponding to self */

	/* Computing this before hand because of 16 ka multiple banana hai */
	if(parameters[0]->text2print != "( )"){
		for(int i = nparameters - 1; i >= 1; i--){
			/* 
				Confusion: Will this assert statement cause any problem?
				Can't we compute register_width right away here?
			*/
			assert(parameters[i]->address->register_width == 32
					|| parameters[i]->address->register_width == 64);
			total_args_width += (parameters[i]->address->register_width)/8;
		}
		if(current_node->what != "PARENT_METHOD_CALL"){
			assert(parameters[0]->address->register_width == 32
					|| parameters[0]->address->register_width == 64);
			total_args_width += (parameters[0]->address->register_width)/8;
		}
	}

	int extra_rsp_shift = 0;
	if(total_args_width%16 != 0)
		extra_rsp_shift = 16 - (total_args_width%16);

	/* Extra shift in rsp before func call */
	if(extra_rsp_shift){
		//because it this makes the increment quad 
		current_code.push_back(new_increment_rsp_quad(-extra_rsp_shift));
	}
	
	if(parameters[0]->text2print != "( )"){
		for(int i = nparameters - 1; i >= 1; i--){
            assert(parameters[i]->address);
			current_code.push_back(new_push_quad(make_copy_of_address(
											parameters[i]->address)));
		}
        assert(parameters[0]->address);
		if(current_node->what != "PARENT_METHOD_CALL"){
			current_code.push_back(new_push_quad(make_copy_of_address(
												parameters[0]->address)));
		}
	}
	
	/*
		pushing heap address

		push +current_node->offset(%rbp)
	*/
	struct address_node *temp = new address_node;
	temp->type = VAR_ON_STACK;
	temp->value = name_compute(object->text2print);
	temp->offset = current_node->offset;
	temp->register_width = 64; /* hardcoded because heap addresses are stored in 64 bytes register */
	temp->live = true;
    
    assert(temp != NULL);
	current_code.push_back(new_quad("push_param", make_copy_of_address(temp), NULL, NULL));
	
	current_code.push_back(new_call_quad(class_name + "_" + method_name));
	
	string return_type = current_node->type;

	if(method_name != "__init__" && return_type != "NONE"){
		current_node->address = new_temporary_address(current_scope, current_node); 
		current_code.push_back(new_pop_quad(current_node->address));	
	}

	total_args_width += extra_rsp_shift;
	
	current_code.push_back(new_increment_rsp_quad(total_args_width));
}

bool is_inside_loop = false;
void generate_3ac(struct symbol_table_node *current_scope, struct ast_node *current_node)
{
	if(!current_node)
		return;

	vector<struct quad_node *> &current_code = current_node->code;
	vector<struct ast_node *> &children = current_node->children;
	int nchildren = children.size();

	string text2print = current_node->text2print;

	if(text2print == "for_stmt" || text2print == "while")
		is_inside_loop = true;

	if(	text2print == "if_stmt"
		&&	nchildren
		&&	(children[0]->children).size() > 1 
		&&	(children[0]->children[0]->children).size() > 1
		&&	(children[0]->children[0]->children[1])->text2print == "STRING: \"__main__\"")
	{
			return;
	}
	
	for(int i = 0; i < nchildren; i++){
		struct symbol_table_node *new_scope = get_new_scope(children[i], current_scope);
		generate_3ac(new_scope, children[i]);

		merge(current_code, (children[i])->code);
	}
	
	// self.x
	if(current_node->what == "SELF_VARIABLE_ACCESS")
	{
		children[0]->address->offset = -16; /* harcoded because class_variable_access() uses this offset*/
		class_variable_access(children[0], children[1], current_node, current_scope);
	}
	// a.x
	else if(current_node->what == "OBJECT_VARIABLE_ACCESS")
	{
		class_variable_access(children[0], children[1], current_node, current_scope);
	}
	else if	(	current_node->what == "PARENT_METHOD_CALL"
			||	current_node->what == "OBJECT_METHOD_CALL"
			|| 	current_node->what == "SELF_METHOD_CALL"
			)
	{
		class_method_call(children[0], children[1], current_node, current_scope);
	}
	/* Class declaration */
	else if(current_node->nonterminal == "classdef"){
		current_code.clear();

		/*
			class_name
			begin_class
				method[0]->body
					   :
				method[n - 1]->body
			end_class
		*/
		
		string class_name = current_node->terminal;
		
		current_code.push_back(new_quad(class_name, NULL, NULL, NULL));

		for(int i = 0; i < nchildren; i++)
			merge(current_code, children[i]->code);
		
	}
	else if(text2print == "break" || text2print == "continue"){
		if(!is_inside_loop){
			cout << "Error on lineno. " << current_node->line_num
			<< " " << text2print << " must be inside loop\n";
			exit(-1);
		}
		current_code.push_back(new_goto_quad("@@_fill_in_backpatch_" + text2print));
	}
	else if (is_operator(text2print)){
		string op = get_operator(text2print);
	

		if((op.back() == '=') 
            && (op != "==" && op != "!="
            && op != "<=" && op != ">="))
        {
			op.pop_back();
            //cout << "opr of *= kind " << endl;
            assert(children[0]->address);
			current_node->address = make_copy_of_address(children[0]->address);
            //cout << "## value aa rha: " << current_node->address->value << endl;
		}
		else{
            current_node->register_width = 32;
            //cout << "Yahan se temp ke liye bheja" << endl;
			current_node->address = new_temporary_address(current_scope, current_node);
            //cout << "Temp bnake aa gya " << endl << endl;		
        }
        assert(children[0]->address);
        assert(children[1]->address);

		current_code.push_back(new_operator_quad(op,
							make_copy_of_address(children[0]->address),
							make_copy_of_address(children[1]->address),
												current_node->address));
        //cout << "is_opr se toh nikal gya" << endl;
        
	}
	else if (	text2print == "UNARY OPERATOR: +" 
			|| 	text2print == "UNARY OPERATOR: -" 
			|| 	text2print == "UNARY OPERATOR: ~" 
			|| 	text2print == "LOGICAL OPERATOR: not"
			)
	{

		if(is_constant(children[0]) ||  text2print == "UNARY OPERATOR: -" 
            ||  text2print == "UNARY OPERATOR: +"){
            //cout << "value of child is " << children[0]->address->value << endl;
            assert(children[0]->address);
			current_node->address = make_copy_of_address(children[0]->address);
            
            if(text2print == "UNARY OPERATOR: -"){

                if((current_node->address->value)[0] == '-'){
                    current_node->address->value = (current_node->address->value).substr(1); 
                }
                else{
                    current_node->address->value = '-' + (current_node->address->value);
                }
            }
        
		}
		else{
			string op = get_operator(text2print);
			if(op == "+")
				op = "addition";
			if(op == "-")
				op = "minus";


			current_node->address = new_temporary_address(current_scope, current_node);
        assert(children[0]->address);
			current_code.push_back(new_operator_quad(op,
													make_copy_of_address(children[0]->address),
													NULL,
													current_node->address));
		}
	}
	else if(text2print == "if_stmt"){
		current_code.clear();

		/*
			labels[0]
				if->code
			ifZ if->condition->address Goto labels[1]
			Goto end_label
			labels[1]
				elif->code
			ifZ elif->condition->address Goto labels[2]
			Goto end_label
			labels[2]
				else->code
			Goto end_label
			end_label
		*/

		vector<struct quad_node *> labels(nchildren + 1);
		for(int i = 0; i < nchildren + 1; i++)
			labels[i] = new_label_quad(current_scope);
		
		struct quad_node *end_label = labels[nchildren];

        if(!end_label)
            cout << "END_LABEL is NULL in if_stmt" << endl;

		struct ast_node *child;
		for(int i = 0; i < nchildren; i++){
			child = children[i];
			current_code.push_back(labels[i]);

			if(child->text2print == "if" || child->text2print == "elif" || child->text2print == "else"){
				if(child->text2print != "else"){
					struct ast_node *condition = (child->children)[0];
				
					merge(current_code, condition->code);
                    //cout << "Calling copy-addr from if_stmt with i: " << i << endl;
                    assert(condition->address);
                    assert(labels[i+1]->result);
					current_code.push_back(new_ifZ_quad(
                        make_copy_of_address(condition->address),
                        make_copy_of_address(labels[i + 1]->result)));
				}

				struct ast_node *body = (child->children)[child->text2print != "else"];
				
				merge(current_code, body->code);
				current_code.push_back(new_goto_quad(end_label->op));
			}
			else{
				printf("line_num.: %d Error: if_stmt of some different kind apart from if, elif, and else\n", child->line_num);
				exit(-1);
			}
		}
		current_code.push_back(end_label);	
	}
	else if(text2print == "if" || text2print == "elif" || text2print == "else"){
		current_code.clear();
		
		/*
			label
				body->code
		*/

		struct ast_node *body = children[text2print != "else"];

		(body->code).insert((body->code).begin(), new_label_quad(current_scope));
	}
	else if(text2print == "while"){
		current_code.clear();
		
		/*
			start_label
				condition->code
			ifZ condition->address Goto end_label
			body_label
				body->code
			Goto start_label
			end_label
		*/
		
		struct quad_node *start_label = new_label_quad(current_scope);
		struct quad_node *body_label = new_label_quad(current_scope);
		struct quad_node *end_label = new_label_quad(current_scope);
	
		struct ast_node *condition = children[0];
		struct ast_node *body = children[1];
		
		current_code.push_back(start_label);
		merge(current_code, condition->code);
        assert(condition->address);
        assert(end_label->result);
		current_code.push_back(new_ifZ_quad(make_copy_of_address(condition->address),
                                      make_copy_of_address(end_label->result))); 
                                      // end_label ka bhi copy bna diya hun 
                                      // even if not needed 

		current_code.push_back(body_label);
		merge(current_code, body->code);
		current_code.push_back(new_goto_quad(start_label->op));	
		current_code.push_back(end_label);
	
		backpatch(current_node, "@@_fill_in_backpatch_break", end_label->op);
		backpatch(current_node, "@@_fill_in_backpatch_continue", start_label->op);
	}
	else if(text2print == "for_stmt"){
		if(children[3]->text2print != "NAME: range"){
			cout << "For loop is not supported non \"range\" iterators" << endl;
			cout << "Try range(n) or range(start, end) for loop iteration"  << endl;
			exit(-1);
		}
		
		current_code.clear();
	
		/*
			start_label
				iterator = iterator_start
				t_1 = iterator < iterator_end
			ifZ t_1 Goto end_label
			body_start_label
				body->code
			body_end_label
				iterator = iterator + 1
				t_1 = iterator < iterator_end
			ifZ t_1 Goto end_label
			Goto body_start_label
			end_label
		*/

		struct quad_node *start_label = new_label_quad(current_scope);
		struct quad_node *body_start_label = new_label_quad(current_scope);
		struct quad_node *body_end_label = new_label_quad(current_scope);
		struct quad_node *end_label = new_label_quad(current_scope);
		
        assert(children[1]->address);
		struct address_node *iterator = make_copy_of_address(children[1]->address); 
		struct address_node *iterator_start, *iterator_end;
		struct address_node *temp = new_temporary_address(current_scope, current_node); // t_1
        temp->register_width = 32;
        current_node->register_width = 32;
		/* range(n) */
		if((children[3]->children).size() == 1){
			iterator_start = new_address_node("INT", to_string(0), 32);
			merge(current_code,(children[3]->children[0])->code);
            assert(children[3]->children[0]->address);
			iterator_end = make_copy_of_address((children[3]->children)[0]->address);
		}
		/* range(start, end) */
		else{
			merge(current_code,(children[3]->children[0])->code);
            assert(children[3]->children[0]->address);
			iterator_start = 
                make_copy_of_address((children[3]->children)[0]->address);
			merge(current_code,(children[3]->children[1])->code);
            assert(children[3]->children[1]->address);
			iterator_end = 
                make_copy_of_address((children[3]->children)[1]->address);
		}

		struct ast_node *body = children[4];

		current_code.push_back(start_label);
	
		current_code.push_back(new_operator_quad("=", iterator_start, NULL, iterator));
		current_code.push_back(new_operator_quad("<", iterator, iterator_end, temp));
        assert(temp);
		current_code.push_back(new_ifZ_quad(make_copy_of_address(temp), end_label->result));

		current_code.push_back(body_start_label);
		merge(current_code, body->code);
		current_code.push_back(body_end_label);

		current_code.push_back(new_operator_quad("+",
										iterator,
										new_constant_address("INT", to_string(1), 32),
										iterator));

		/*if((children[3]->children).size() == 1){
            if((children[3]->children[0])->code.size()){
            assert(((children[3]->children[0])->code).back()->result);
                ((children[3]->children[0])->code).back()->result = 
                make_copy_of_address(((children[3]->children[0])->code).back()->result);
            } 
                    
			merge(current_code,(children[3]->children[0])->code);
		}
		// range(start, end) 
		else{
             if((children[3]->children[1])->code.size()){
                 assert(((children[3]->children[1])->code).back()->result);
                ((children[3]->children[1])->code).back()->result = 
                make_copy_of_address(((children[3]->children[1])->code).back()->result );
            } 

			merge(current_code,(children[3]->children[1])->code);
		}*/
		current_code.push_back(new_operator_quad("<", iterator, iterator_end, temp));
		assert(temp);
        current_code.push_back(new_ifZ_quad(make_copy_of_address(temp), end_label->result));
		current_code.push_back(new_goto_quad(body_start_label->op));
		current_code.push_back(end_label);
		backpatch(current_node, "@@_fill_in_backpatch_break", end_label->op);
		backpatch(current_node, "@@_fill_in_backpatch_continue", body_end_label->op);
	}
	else if(is_func_call(current_node)){
        //cout << "ENTER: func_call " << get_func_name(current_node) << endl;
		/*
			push param_(n - 1)
				 :
			push param_0
			call func_name
			current_node->address = pop
		*/
        
        /* #function call support:
         * --> We are pushing args on stack which lowers the rsp by the 
         *     size of combined width of all arguments 
         * --> So we need to increment the rsp again to match original 
         *     stack mapping. I am storing that in total_args_width 
         */

			int total_args_width = 0;
			/* Computing this before hand because of 16 ka multiple banana hai */
			if(children[0]->text2print != "( )"){
				for(int i = nchildren - 1; i >= 0; i--){
					total_args_width += (children[i]->address->register_width)/8;
				}
			}

			/*assert(total_args_width == current_node->offset &&
				"Offset filled in the func_call node is not correct");*/

			int extra_rsp_shift = 0;
			if(total_args_width%16 != 0)
				extra_rsp_shift = 16 - (total_args_width%16);

			/* Extra shift in rsp before func call */
			if(extra_rsp_shift && get_func_name(current_node)!="print"){
				//because it this makes the increment quad 
				current_code.push_back(new_increment_rsp_quad(-extra_rsp_shift));
			}

			string func_name = get_func_name(current_node);
			
			if(children[0]->text2print != "( )"){
				for(int i = nchildren - 1; i >= 0; i--){
				    assert(children[i]->address);	
                    current_code.push_back(new_push_quad(
											make_copy_of_address(children[i]->address)));
				}
			}
			
			current_code.push_back(new_call_quad(func_name));

			string return_type = current_node->type;
			if(		func_name != "main"
				&&	func_name != "range"
				&&	func_name != "print"
				&&	return_type != "NONE")
			{
                if(func_name == "len")
                    current_node->register_width = 32;
                
				current_node->address = new_temporary_address(current_scope, current_node); 
				current_code.push_back(new_pop_quad(current_node->address));	
			}

			/* func call is done, we got the return value as well 
			* time to bring back the rsp to older position 
			*/
			total_args_width += extra_rsp_shift;
			if(total_args_width && func_name!="print")
				current_code.push_back(new_increment_rsp_quad(total_args_width));
            
        //cout << "OUT: func_call of " << func_name << endl;
	}
	else if(is_func_definition(current_node)){
		current_code.clear();

		/*
			class_name_method_name / func_name
			begin_func
				method->body
			end_func
		*/

		string func_name = get_func_name(current_node);
		string func_end_label;
		
		/* Class method definition */
        if(current_node->type == "CLASS_METHOD"){
			struct symbol_table_node *class_table = current_scope->parent;
			string class_name = class_table->lexeme;
			
			current_code.push_back(new_quad(class_name + "_" + func_name,
									NULL,
									NULL,
									NULL));
			func_end_label = "func_end_" + func_name + "_" + class_name;
		}
		else{
			current_code.push_back(new_quad(func_name,NULL,NULL,NULL));
			func_end_label = "func_end_" + func_name;
		}

        current_code.push_back(new_label_quad(current_scope, "begin_func",
								new_constant_address("INT",
								to_string(current_scope->current_offset),
								32)));
		
		struct ast_node *body = children[1];
		merge(current_code, body->code);

		current_code.push_back(new_label_quad(current_scope, func_end_label,
								new_constant_address("INT",
								to_string(current_scope->current_offset),
								32)));
	}
	else if(text2print == ":"){
        /* Allowing the same content to pass here */
		current_node->address = children[0]->address;
		current_node->laddress = children[0]->laddress;
		current_node->width = children[0]->width;
		current_node->what = children[0]->what;
	}
	else if(text2print == "ASSIGNMENT OPERATOR: ="){
	    //cout << "%% = mein in for line " << current_node->line_num << endl;	
        /* For assigning a new value to any element of list 
         * there will be an useless line of code since when I am 
         * on the data[i] node, I don't know where it is
         * a = data[i] or data[i] = a 
         * We will tackle data[i] = a as if it is mem_assign
         */

        if(is_list(children[0]) &&
            is_identifier(children[0],current_scope) &&
            children[1]->text2print != "list [ ]")
        {
            //cout << "list diffr mein " << endl;
            //cout << "Isme ghusa hai " << endl;
          // bit of recomputation here ik it sucks :-)
             //print_ast_node(children[0]);
            // need index and list_base 
            pair< string, pair<int, int>> all_info = get_info
                                    (children[0], current_scope);
            int offset = all_info.second.first;

            //cout << "Did we reach here1" << endl;
            struct address_node* list_base = new address_node;
            list_base->type = "heap";
            list_base->value = children[0]->nonterminal;
            list_base->offset = offset;
            list_base->register_width = 64;
            list_base->live = false;

            //cout << "Did we reach here2" << endl;
            struct address_node* index = (children[0])->children[0]->address; //i

            //index->offset += 4; //smjh rhe ho na :-) 

			if(children[0]->type == "INT" || children[0]->type == "BOOL"){
				assert(children[1]->address);
                assert(index);
                assert(list_base);
                current_code.push_back(new_quad("mem_assign",
									make_copy_of_address(children[1]->address), // a
									make_copy_of_address(index),                // i
									make_copy_of_address(list_base)));          // arr (base)
			}
			else{
				assert(children[1]->address);
                assert(index);
                assert(list_base);
				current_code.push_back(new_quad("mem_assign_list",
									make_copy_of_address(children[1]->address), // a
									make_copy_of_address(index),                // i
									make_copy_of_address(list_base)));          // arr (base)
			}
            //cout << ">>>>Did we reach here  " << endl;
          
        }
        
		else if(children[0]->laddress){
            //cout << "laddr mein aa gye " << endl;
			current_code.push_back(
					new_quad("assign_to_memory", children[1]->address, NULL, children[0]->laddress));
		}
		/* List declaration */
        else if(children[1]->text2print == "list [ ]" &&
            children[1]->what == "LIST_DECLARATION")
        {
            //cout << "LIST DECL mein ghuse " << endl;
            int num_of_elements;
		    if((children[1]->children).size() == 1 
                    && (children[1])->children[0]->text2print == "[ ]"){
			    num_of_elements = 0;
                cout << "ERROR: List is empty" << endl;
                exit(-1);
		    }
		    else{
			    num_of_elements = (children[1])->children.size();
		    }
			no_of_element_list[((children[0])->children[0])->nonterminal] = num_of_elements;
            // width mein populated hai shi wala ~ @rajeev
            int memory_to_allocate = children[1]->width; 
            
            // data = mem_alloc total_width 
            // extra 4 bytes for length of list 
            assert((children[0])->address);
            current_code.push_back(new_memalloc_quad(memory_to_allocate + 4, //***smjh rhe ho na 4 
                                    make_copy_of_address(children[0]->address)));

            
            // data[i] = val_i
            /* "mem_assign" | val_i->addr  |  constant  | type-> "heap"       
             *                            |  index<- i  | value-> data        
             *                            |             | reg_width-> R64     
             *                            |             | offset-> -12(%rbp)  
             *                            |             | bool-> false        
             */

            vector<ast_node*> &list_elements = children[1]->children;
            int element_width = list_elements[0]->width;
            if(element_width == 32 || 
                element_width == 64 ){
                element_width /= 8;
            }
            struct address_node* list_base = children[0]->address;

                /*-----------------------------------------------------------------
                 *        Populating length of list                               
                 *-----------------------------------------------------------------*/
                 struct address_node* list_len = new address_node;
                list_len->type = "INT";
                list_len->value = to_string(list_elements.size()); //i
                list_len->offset = 0; //smjh rhe ho na  :-)

                struct address_node* dummy_offset = new address_node;
                dummy_offset->type = "list_index";
                dummy_offset->value = "list_len"; //i
                dummy_offset->offset = 0; //smjh rhe ho na  :-)
                
            assert(list_base);
                current_code.push_back(new_quad("mem_assign",
                                    list_len,
                                    dummy_offset,
                                    make_copy_of_address(list_base)));
                /*-----------------------------END--------------------------------*/

			if(children[1]->type == "LIST_INT" || children[1]->type == "LIST_BOOL"){
        
				for(int i=0; i<list_elements.size(); i++){
					struct address_node* index = new address_node;
					index->type = "list_index";
					index->value = to_string(i); //i
					index->offset = i*element_width + 4; //smjh rhe ho na 4 :-)
					assert(list_elements[i]->address);
                    assert(list_base);
					current_code.push_back(new_quad("mem_assign",
										make_copy_of_address(list_elements[i]->address),
										index,
										make_copy_of_address(list_base)));
				}
			}
			else if(children[1]->type == "LIST_STR"){
				for(int i=0; i<list_elements.size(); i++){
					struct address_node* index = new address_node;
					index->type = "list_index";
					index->value = to_string(i); //i
					index->offset = i*8 + 4;

					struct address_node* temp = new_constant_address(current_node->children[1]->children[i]->type,
                                                         get_operator(current_node->children[1]->children[i]->text2print),
                                                         64);
					current_node->children[1]->children[i]->register_width = 64;
					current_node->children[1]->children[i]->address = new_temporary_address(current_scope, current_node->children[1]->children[i]);
					
					current_code.push_back(new_operator_quad("str_alloc", temp,NULL,current_node->children[1]->children[i]->address));

assert(list_elements[i]->address);
                    assert(list_base);
					current_code.push_back(new_quad("mem_assign_list",
										make_copy_of_address(list_elements[i]->address),
										index,
										make_copy_of_address(list_base)));
				}
			}
			else{
				for(int i=0; i<list_elements.size(); i++){
					struct address_node* index = new address_node;
					index->type = "list_index";
					index->value = to_string(i); //i
					index->offset = i*8 + 4;

assert(list_elements[i]->address);
                    assert(list_base);
					current_code.push_back(new_quad("mem_assign_list",
										make_copy_of_address(list_elements[i]->address),
										index,
										make_copy_of_address(list_base)));
				}
			}

           // cout << "LIST DECLARATION DONE" << endl;
        }
		/*------------------------------------------------------------------
								List declaration done
		 ------------------------------------------------------------------*/



		/*	Object instantiation -> same as calling __init__ of the class
			except in this case we need to have pointer to the allocated memory
			as return value
		*/
		else if(children[1]->what == "OBJECT_INSTANTIATION"){
            //cout << "OBJ INST mein ghuse " << endl;
			/*
				class A:
					def __init__(self, x: int, y: int):
						self.x : int = x;
						self.y : int = y;
						return;

				def main();
					a: A = A(x, y);

				------------------ 3AC ----------------

				a = mem_alloc(8)
				push y
				push x
				push a
				call A___init__

			*/
			int width_of_object = children[1]->width;
			string class_name = children[1]->type;
			int total_args_width = children[1]->offset;
			
			(children[0]->address)->type = VAR_ON_HEAP;

            assert(children[0]->address);
			current_code.push_back(new_memalloc_quad(width_of_object,
							make_copy_of_address(children[0]->address)));	

			vector<struct ast_node*> &parameters = children[1]->children;
			int nparameters = parameters.size();
                
            int extra_rsp_shift = 0;
            if(total_args_width%16 != 0){
                extra_rsp_shift = 16 - (total_args_width%16);
            }

            if(extra_rsp_shift){
                current_code.push_back(new_increment_rsp_quad(-extra_rsp_shift));
            }

			if(parameters[0]->text2print != "( )"){
				for(int i = nparameters - 1; i >= 0; i--){
					current_code.push_back(new_push_quad(make_copy_of_address(parameters[i]->address)));
			        assert(parameters[i]->address);
                }
            }

			        assert(children[0]->address);
			current_code.push_back(new_push_quad(make_copy_of_address(children[0]->address)));
			
			current_code.push_back(new_call_quad(class_name + "___init__"));

            total_args_width += extra_rsp_shift;
			current_code.push_back(new_increment_rsp_quad(total_args_width));
		}
        else{
            //cout << "LAST wala " << endl;
            //current_node->address = new_temporary_address(current_scope, current_node);
			        assert(children[1]->address);
			        assert(children[0]->address);
            current_code.push_back(new_operator_quad("=", 
                                make_copy_of_address(children[1]->address),
                                NULL,
                                make_copy_of_address(children[0]->address)));
        }

		//cout << "LEAVING = at line: " << current_node->line_num << endl;
	}
	else if(text2print == "return"){
		if(!children.empty()){
			        assert(children[0]->address);
			current_code.push_back(new_return_quad(make_copy_of_address(children[0]->address)));
        }
		string func_name = current_scope->lexeme;
		string func_end_label;

		if(current_scope->type == "CLASS_METHOD"){
			string class_name = current_scope->parent->lexeme;
			func_end_label = "L_func_end_" + func_name + "_" + class_name; 
		}
		else if(current_scope->type == "FUNCTION_DEF")
			func_end_label = "L_func_end_" + func_name;
		else{
			cout << "Error on lineno. " << current_node->line_num
			<< " return must be inside either function definition or class method definition\n";
			exit(-1);
		}
		/* return must be inside either FUNCTION_DEF or CLASS_METHOD */

		current_code.push_back(new_goto_quad(func_end_label));
	}
	else if(is_identifier(current_node, current_scope) || text2print == "DUNDER: __name__"){
        /*
                            |
                Name: var   |     Name: arr
                            |         |
                            |     Number: 6
        */

        string id_name = get_operator(text2print); // this function should do?

        if(is_list(current_node) && children.size()){
            /* current_node->addr := addr of list (arr)
             * current_node->child0->addr := index value (i)
             * code:      t = arr[i]
             * 3AC-quad:  [] | arr | i | t
             */

            // arg1 : arr
            //print_ast_node(current_node);
            if(children.size() == 0){
                cout << "ERROR: is_list ke andar #children 0 hai" << endl;
                return;
            }
            
            pair< string, pair<int, int>> all_info = get_info
                                    (current_node, current_scope);

            int offset = all_info.second.first;


            struct address_node* arg1 = new address_node;
            arg1->type = LIST_ADDR;
            arg1->value = id_name;
            arg1->offset = offset; //smjh rhe ho na :-)
            arg1->register_width = 64;
            arg1->live = false;

            assert((current_node->children)[0]->address);
            // index: i
            struct address_node* arg2 = make_copy_of_address((current_node->children)[0]->address);
            // result: t
			if(current_node->type == "LIST_INT" || current_node->type == "LIST_BOOL"
                || current_node->type == "INT" || current_node->type == "BOOL"){
            	current_node->register_width = 32;
			}
			else{
				current_node->register_width = 64;
			}

            current_node->address = new_temporary_address(current_scope,current_node);

            struct quad_node* quad = new_operator_quad("[]",
                                arg1, arg2, current_node->address);

            current_code.push_back(quad);

        }
        else{

            struct address_node* id_addr = new address_node;
            pair< string, pair<int, int>> all_info = get_info
                                    (current_node, current_scope);

            string scope = all_info.first;
            int offset = all_info.second.first;
            int width = all_info.second.second;
                
            //cout << "text2print: " << text2print << endl;
            //cout << "width" <<  width << endl;

            id_addr->type = (scope == "local")? VAR_ON_STACK : VAR_ON_HEAP;
            id_addr->value = id_name;
			id_addr->offset = offset;
            width = width * 8;
            id_addr->register_width = width;
            id_addr->live = false;
            

            current_node->address = id_addr;
        }
    }
    else if(is_constant(current_node)){
			if(current_node->type == "STR"){
            	struct address_node* temp = new_constant_address(current_node->type,
                                                         get_operator(text2print),
                                                         64);
                current_node->register_width = 64;
				current_node->address = new_temporary_address(current_scope, current_node);
                
				current_code.push_back(new_operator_quad("str_alloc", temp,NULL,current_node->address));

			}
			else if(text2print.find(':') == -1){
				if(text2print == "True"){
					current_node->address = new_constant_address(current_node->type,
															to_string(1),
															32);
				}
				else if(text2print == "False"){
					current_node->address = new_constant_address(current_node->type,
															to_string(0),
															32);
				}
				else{
					current_node->address = new_constant_address(current_node->type,
															text2print,
															32);
				}
			}
			else{
            	current_node->address = new_constant_address(current_node->type,
                                                         get_operator(text2print),
                                                         32);
			}
    }
	else if(is_typecasting(current_node)){
		current_node->address = new_temporary_address(current_scope, current_node);
		current_code.push_back(new_operator_quad(text2print, children[0]->address, 
												NULL, current_node->address));
	}
	
	if(text2print == "for_stmt" || text2print == "while")
		is_inside_loop = true;
}

/* Returns true only for binary operators */
bool is_operator(string text2print)
{
	return		text2print == "ARITHMETIC OPERATOR: +"
			||	text2print == "ARITHMETIC OPERATOR: -"
			||	text2print == "ARITHMETIC OPERATOR: *"
			||	text2print == "ARITHMETIC OPERATOR: /"
			||	text2print == "ARITHMETIC OPERATOR: %"
			||	text2print == "ARITHMETIC OPERATOR: //"
			||	text2print == "ARITHMETIC OPERATOR: **"
			
			||	text2print == "RELATIONAL OPERATOR: ==" 
			|| 	text2print == "RELATIONAL OPERATOR: !=" 
			||  text2print == "RELATIONAL OPERATOR: <" 
			||  text2print == "RELATIONAL OPERATOR: >" 
			||  text2print == "RELATIONAL OPERATOR: <=" 
			||  text2print == "RELATIONAL OPERATOR: >="
			
			||  text2print == "LOGICAL OPERATOR: and" 
			||  text2print == "LOGICAL OPERATOR: or"
			
			|| 	text2print == "BITWISE OPERATOR: &" 
			||  text2print == "BITWISE OPERATOR: |" 
			||  text2print == "BITWISE OPERATOR: ^" 
			||  text2print == "BITWISE OPERATOR: <<" 
			||  text2print == "BITWISE OPERATOR: >>" 
			
			||	text2print == "ARITHMETIC OPERATOR: +="
			||	text2print == "ARITHMETIC OPERATOR: -="
			||	text2print == "ARITHMETIC OPERATOR: *="
			||	text2print == "ARITHMETIC OPERATOR: /="
			||	text2print == "ARITHMETIC OPERATOR: %="
			||	text2print == "ARITHMETIC OPERATOR: //="
			||	text2print == "ARITHMETIC OPERATOR: **="
			
			||	text2print == "ARITHMETIC OPERATOR: &="
			||	text2print == "ARITHMETIC OPERATOR: |="
			||	text2print == "ARITHMETIC OPERATOR: ^="
			||	text2print == "ARITHMETIC OPERATOR: <<="
			||	text2print == "ARITHMETIC OPERATOR: >>="
			
			;
}

/*
	Input: "ARITHMETIC OPERATOR: +"
	Output: "+"
*/
string get_operator(string text2print)
{
	return text2print.substr(text2print.find(':') + 2);
}
/* Definition of functions */

/*					.
				   / \
				self  method_call <- current node
*/

int is_func_call(struct ast_node *current_node)
{
	/*Support to check built in functions len and print */
	string built_in_funcs = current_node->nonterminal;
	if (built_in_funcs == "len" || built_in_funcs == "print")
		return 1;

	string name = current_node->nonterminal;
	if (entry_finder_global.count(name) && ((symbol_table_head->scope_content)[entry_finder_global[name]])->type == "FUNCTION_DEF")
	{
		return 1;
	}
	return 0;
	/*search in the symtable map with the lexeme of func */
}

int is_func_definition(struct ast_node *current_node)
{

	if (current_node->nonterminal == "funcdef")
	{
		// cout << current_node->text2print << " is a funcdef " << endl;
		return 1;
	}
	// cout << current_node->text2print << " is not a funcdef " << endl;
	return 0;
	/* find "function" in the text2print || dual check with children */
}

int is_class_method_call(struct ast_node *current_node)
{
	string name = current_node->nonterminal;
	if (entry_finder_global.count(name) && ((symbol_table_head->scope_content)[entry_finder_global[name]])->type == "FUNCTION_DEF")
	{
		return 1;
	}
	return 0;
	/* search in the symtable map with the method lexeme */
}

int is_class_definition(struct ast_node *current_node)
{
	if (current_node->nonterminal == "classdef")
	{
		return 1;
	}
	return 0;
	/* find "class" in the text2print || dual check with children */
}

int is_identifier(struct ast_node *current_node, struct symbol_table_node *scope)
{

	string name = current_node->text2print;
	if (name.find("NAME: ") == 0)
		return 1;
	// cout << current_node->text2print << " is not an indentifier" << endl;
	// return 0;
	/* identifier can be in passed parameter if the scope is function definition */
	if (scope->type == "FUNCTION_DEF")
	{
		map<string, int> &args_map = scope->argument_finder;
		if (args_map.count(name))
			return 1;
	}
	return 0;
}

int is_typecasting(struct ast_node *node)
{
	if (node->what == "typecast")
		return 1;
	return 0;
}

int is_constant(struct ast_node *current_node)
{
	return current_node->what == "CONSTANT";
}

string get_type(struct ast_node *node, struct symbol_table_node *scope)
{
	/* Lookup the symbol table and return the type(string format) */
	string lexeme = name_compute(node->text2print);
	int index = (scope->entry_finder_local)[lexeme];
	string type = (scope->scope_content)[index]->type;
	return type;
}

// struct symbol_table_node* find_scope_of(string lexeme, struct symbol_table_node* parent_scope){
/* @lexeme: this can be any function/class/method, which can start on a
 *          new scope inside the @parent_scope scope
 * @parent_scope: Scope inside which I need to find the @lexeme
 *
 * The function should ideally find scope with the @lexem and return
 * struct symbol_table_head*
 */
//}

int is_class_object(struct ast_node *node, struct symbol_table_node *scope)
{
	string object_name = node->nonterminal;
	if ((scope->entry_finder_local).count(object_name))
	{
		int index = (scope->entry_finder_local)[object_name];
		string class_name = ((scope->scope_content)[index])->type;
		if (entry_finder_global.count(class_name))
		{
			index = entry_finder_global[class_name];
			if (((symbol_table_head->scope_content)[index])->type == "CLASS_DECLARATION")
			{
				return 1;
			}
		}
	}
	return 0;
}

int is_list(struct ast_node *node)
{
	return node->what == "LIST_DEREFERENCE" && (node->children).size();
}

struct symbol_table_node *find_scope_of(string &lexeme, struct symbol_table_node *parent_scope)
{
	return (parent_scope->children)[(parent_scope->table_finder_local)[lexeme]];
}

/* Generic heuristic functions */
string get_func_name(struct ast_node *node)
{
	/* heuristics on text2print */
	return name_compute(node->text2print);
}

string get_indentifier_name(string &text2print)
{
	/* heuristics on text2print */
	return name_compute(text2print);
}

string get_class_name(struct ast_node *node)
{
	return name_compute(node->text2print);
}

string get_width_for_list_element(string &type)
{
	/*type check --> bytes needed for that type */
	if (type == "LIST_INT" || type == "INT")
		return to_string(sizeof(int));
	else if (type == "LIST_FLOAT" || type == "FLOAT")
		return to_string(sizeof(float));
	else if (type == "LIST_BOOL" || type == "BOOL")
		return to_string(sizeof(bool));
	else if (type == "LIST_STR" || type == "STR")
		return to_string(8);
	else
	{
		/* element is class object */
		int pos = type.find("_");
		string class_name = type.substr(pos + 1);
		map<string, int> &global_map = symbol_table_head->table_finder_local;
		int class_index = global_map[class_name];
		int width = (symbol_table_head->children)[class_index]->width;
		return to_string(width);
	}
}

string get_constant(struct ast_node *current_node)
{
	if (current_node->type == "BOOL")
	{
		return current_node->text2print;
	}
	return name_compute(current_node->text2print);
}

void backpatch(struct ast_node *current_node, string from, string to)
{
	vector<struct quad_node *> &current_code = current_node->code;
	for(int i = 0; i < current_code.size(); i++){
		if((current_code[i])->arg1
            && ((current_code[i])->arg1)->value == from)
        {
			((current_code[i])->arg1)->value = to;
        }
	}
}

struct address_node* make_copy_of_address(struct address_node* src){
    if(!src){
        cout << "make_copy_of_address() received a null pointer " << endl;
        return NULL;
    }

    struct address_node* dst = new address_node;
    *dst = *src;  //kya ye chalega?
    
    return dst;
}

struct symbol_table_node *get_new_scope(struct ast_node *node, struct symbol_table_node *scope)
{
	/*scope will change in case of:
	 * function defintion
	 * class defintion
	 * class method defintion
	 */

	/*is a function definition?
	 * --> performing three checks just to be sure about it
	 *  1. text2print has "function" in it
	 *  2. node has exactly 2 children
	 *  3. the 2 children are "parameters" & "body"
	 *
	 *  Note: I know this check can easily surpassed if someone wants to_string
	 *        but for time being I am sticking with this
	 */
	if (node->nonterminal == "funcdef" && node->type != "CLASS_METHOD")
	{
		// cout << "funcdef: current scope was:" << scope->lexeme << endl << "Next node is: " << node->text2print << endl;
		string func_name = node->terminal;
		// cout << "Newscope is: " << (find_scope_of(func_name,symbol_table_head))->lexeme << endl;
		// cout << "-----------------------------------------" << endl;
		return find_scope_of(func_name, symbol_table_head);
	}
	/* Is a class defintion?
	 * Similar to what I have done function (above case)
	 * I am going to find "class" in the text2print
	 * then check #children and the first function to be __init__*/
	if (node->nonterminal == "classdef")
	{
		// cout << "classdef: current scope was:" << scope->lexeme << endl << "Next node is: " << node->text2print << endl;
		string class_name = node->terminal;

		// cout << "Newscope is: " << (find_scope_of(class_name,symbol_table_head))->lexeme << endl;
		// cout << "-----------------------------------------" << endl;
		return find_scope_of(class_name, symbol_table_head);
	}

	/* Check for method call */
	if (node->type == "CLASS_METHOD")
	{
		// cout << "method: current scope was:" << scope->lexeme << endl << "Next node is: " << node->text2print << endl;
		string method_name = node->terminal;

		// cout << "Newscope is: " << (find_scope_of(method_name,scope))->lexeme << endl;
		// cout << "-----------------------------------------" << endl;
		/* Passing the scope of parent class */
		return find_scope_of(method_name, scope);
	}

	/* If none of the above mentioned case is found --> scope remains same */
	return scope;
}

pair<string, pair<int, int>> get_info(struct ast_node *current_node,
										struct symbol_table_node *current_scope)
{
	string name = name_compute(current_node->text2print);
    struct symbol_table_entry* entry_content;

	if((current_scope->argument_finder).count(name)){
		int index = (current_scope->argument_finder)[name];
        entry_content = (current_scope->arguments)[index];

		return {"local",
				{entry_content->offset,
				entry_content->width}};
	}
	
	if((current_scope->entry_finder_local).count(name)){
		int index = (current_scope->entry_finder_local)[name];
        entry_content = (current_scope->scope_content)[index];
		
		return {"local",
				{entry_content->offset,
				entry_content->width}};
    }

	if((symbol_table_head->entry_finder_local).count(name)){
		int index = (symbol_table_head->entry_finder_local)[name];
        entry_content = (symbol_table_head->scope_content)[index];
		
		return {"global",
				{entry_content->offset,
				entry_content->width}};
    }
    else{
		struct symbol_table_entry *entry = current_node->entry;
		if(entry){
			return{"global", /*just to indicate that this is on heap */
					{entry->offset,
					entry->width}};
		}
		//print_ast_node(current_node);
        return {"", {-1, -1}};
    }
}
