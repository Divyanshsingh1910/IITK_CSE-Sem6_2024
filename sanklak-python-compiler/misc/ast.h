/* Structure definitions */
struct node{
    string terminal;		// represents terminal type such as NUMBER, NAME, etc.
    string text2print; 		// terminal -> lexeme | non-terminal -> label
    string nonterminal;		// represents non-terminal type such as funcdef, expr, etc.
    vector<node*> children;
};

node* new_node(string terminal,string text2print,string nonterminal,vector<node*> children){
    struct node* temp = new node;
    (temp->terminal)  = terminal;
    (temp->text2print) = text2print;
    (temp->nonterminal) = nonterminal;
    (temp->children) = children;
    return temp;
}

node* add_child_back(struct node* parent ,struct node* child){
    if(parent == NULL){
        return child;
    }
    else if( child == NULL){
        return parent;
    }
    else{
        (parent->children).push_back(child);
    }
    return parent;
}

node* add_child_front(struct node* parent ,struct node* child){
    if(parent == NULL){
        return child;
    }
    else if( child == NULL){
        return parent;
    }
    else{
        (parent->children).insert((parent->children).begin(), child);
    }
    return parent;
}

int is_terminal(struct node* root){
	return (root->children).empty();
}

vector<struct node*> create_ast(struct node* current){
	if(!current){ // current is NULL
		return {};
	}

	if(is_terminal(current)){
		/* node is a terminal */
		return {current};
	}
	/* Non-terminal hai */
	
	/* print krna hai ya nahi */
	if(!((current->text2print).empty())){
		struct node *temp = new_node(current->terminal, current->text2print, current->nonterminal, vector<struct node*>());
		for(int i=0; i < (current->children).size(); i++ ){
			vector<struct node*> children = create_ast((current->children)[i]);
			for(int j = 0; j < children.size(); j++){
				(temp->children).push_back(children[j]);
			}
		}
		return {temp};
	}
	else{
		vector<struct node*> all_children;
		for(int i=0; i < (current->children).size(); i++ ){
			vector<struct node*> children = create_ast((current->children)[i]);
			for(int j = 0; j < children.size(); j++){
				all_children.push_back(children[j]);
			}
		}
		return all_children;
	}
}

#include <fstream>

int counter=1;
queue<int > node_no;
string print_node(struct node* current,int node_number,int type){
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
        s="node" + to_string(node_number) + " [label = " + "\"" +  g + "\"]";
    }
    else{
        s="node" + to_string(node_number);
    }
    return s;
}

void dot_file(struct node* root, string output_file){
	if(output_file.size() == 0){
		output_file = "file.dot";
	}
	
	ofstream file(output_file);

    file << "digraph {\n";
    queue<struct node*> q;
	if(root==NULL){
		file << "}\n";
		return;
	}
    file << "   ";
    file << print_node(root,counter,0);
    file << "\n";
    q.push(root);
    node_no.push(counter);
    counter++;
    while(!q.empty()){
        struct node* temp = q.front();
        q.pop();
        int node_number = node_no.front();
        node_no.pop();
        for(int i=0;i<(temp->children).size();i++){
			if((temp->children)[i]==NULL){
				continue;
			}
			file << "	";
			file << print_node((temp->children)[i],counter,0);
			file << "\n";
            file << "   ";
            file << print_node(temp,node_number,1);
            file << " -> ";
            file << print_node((temp->children)[i],counter,1);
            file << "\n";
            q.push((temp->children)[i]);
            node_no.push(counter);
            counter++;
        }
    }
    file << "}\n";

    file.close();
}
