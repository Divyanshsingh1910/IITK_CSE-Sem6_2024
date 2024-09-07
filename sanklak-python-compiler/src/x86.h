
#include <bits/stdc++.h>
using namespace std;
/* This header file will contain the list of all the function
 * related to x86 assembly generation
 */


/* Register manipulation functions */
string get_free_reg(int width);
void make_free(string reg, int width);

//arg is not necessarily reg
void check_make_free(struct quad_node* current_instr); 

string get_32of64(string reg64);

void get_this_reg(string reg);
/*void get_rdx();
void get_rax();*/

string get_new_label();
string get_offset_from_addr(string reg);
string get_reg_from_addr(string reg);
/* Case finders */
bool is_label(struct quad_node* code_entry);
bool is_func_name(struct quad_node* code_entry);
bool is_conditional_jump(struct quad_node* code_entry);
bool is_jump(struct quad_node* code_entry);
bool is_push_param(struct quad_node* code_entry);
bool is_return_value(struct quad_node* code_entry);
bool is_access_return_val(struct quad_node* code_entry);
bool is_increment_rsp(struct quad_node* code_entry);
bool is_call(struct quad_node* code_entry);
bool is_mem_allocation(struct quad_node* code_entry);
bool is_str_allocation(struct quad_node* code_entry);


/* Helper functions to process the arguments from a entry of 
 * quad-code.
 * Note:
 *      These functions should return the respective address
 *      or the operator in ready to use state.
 */
string get_address_of(struct address_node* operand);


/*Single function to gen code using all the helpers */
string gen_x86(vector <struct quad_node*> &qcode);

void populate_data_structures();
