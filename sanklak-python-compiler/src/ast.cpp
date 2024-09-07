#include "ast.h"

struct ast_node* new_ast_node(
								int line_num,
								string terminal,
								string text2print,
								string nonterminal,
								vector<ast_node *> children
							 )
{
    struct ast_node *temp = new ast_node;
	temp->line_num = line_num;
    temp->terminal  = terminal;
    temp->text2print = text2print;
    temp->nonterminal = nonterminal;
    temp->children = children;
    return temp;
}

struct terminal_node* new_terminal_node(
											int line_num,
											string type,
											string lexeme
							 		   )	
{
    struct terminal_node *temp = new terminal_node;
    temp->line_num  = line_num;
    temp->type = type;
    temp->lexeme = lexeme;
    return temp;
}

struct ast_node* add_child_back(struct ast_node* parent ,struct ast_node* child){
    if(!parent)
        return child;
    else if(!child)
        return parent;
    else
        (parent->children).push_back(child);
    return parent;
}

struct ast_node* add_child_front(struct ast_node *parent ,struct ast_node *child){
    if(!parent)
        return child;
    else if(!child)
        return parent;
    else
        (parent->children).insert((parent->children).begin(), child);
    return parent;
}

void print_ast_node(struct ast_node *node)
{
	cout << "========== start ==========\n\n" ;
	cout << "address: " << node << endl;
	cout << "line_num: " << node->line_num << endl;
	cout << "what: " << node->what << endl;
	cout << "terminal: " << node->terminal << endl;
	cout << "text2print: " << node->text2print << endl;
	cout << "nonterminal: " << node->nonterminal << endl;
	cout << "#children: " << (node->children).size() << endl;
    cout << "address: " << node->address << endl;
    cout << "type: " << node->type << endl;
    cout << "width: " << node->width << endl;
    cout << "offset: " << node->offset << endl;
	cout << "========== end ==========\n\n"; 
}

int is_terminal(struct ast_node *root){
	return (root->children).empty();
}

vector<struct ast_node *> create_ast(struct ast_node *current){
	if(!current){ // current is NULL
		return {};
	}

	if(is_terminal(current)){
		/* ast_node is a terminal */
		return {current};
	}
	/* Non-terminal hai */
	
	/* print krna hai ya nahi */
	if(!((current->text2print).empty())){
		struct ast_node *temp = new_ast_node(current->line_num,current->terminal, current->text2print, current->nonterminal, no_child);
		for(int i=0; i < (current->children).size(); i++ ){
			vector<struct ast_node *> children = create_ast((current->children)[i]);
			for(int j = 0; j < children.size(); j++){
				(temp->children).push_back(children[j]);
			}
		}
		return {temp};
	}
	else{
		vector<struct ast_node *> all_children;
		for(int i=0; i < (current->children).size(); i++ ){
			vector<struct ast_node *> children = create_ast((current->children)[i]);
			for(int j = 0; j < children.size(); j++){
				all_children.push_back(children[j]);
			}
		}
		return all_children;
	}
}

struct quad_node* new_quad(string op,
								struct address_node *arg1,
								struct address_node *arg2,
								struct address_node *result)
{
	struct quad_node *temp = new quad_node;
	temp->op = op;
	temp->arg1 = arg1;
	temp->arg2 = arg2;
	temp->result = result;
	return temp;
}

