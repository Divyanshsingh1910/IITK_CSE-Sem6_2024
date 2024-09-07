#include<bits/stdc++.h>
#include "x86.h"
#include "ast.h"

using namespace std;

int label_count = 0;
string EOL = "\n\t";
string x86 = "";
string len_function = "\n\nlen:\n\tpushq %rbp\n\tmovq %rsp, %rbp\n\tmovq 16(%rbp), %rax\n\tmovl (%rax), %eax\n\tpopq %rbp\n\tret\n\n";
#define R64 64 
#define R32 32

vector<string> calling;

/* Used Data structures */
    // map: temps -> regs 
    map <string,string> map_temp2reg32;

    /* map: regs -> temp */
    map <string, string> map_32reg2temp;

    /* map: 32bit -> 64bit regs*/
    map <string, string> map_32to64;
    /* Creating a list of regs to make them serialize
     * while using, I think this will help in debugging
     * since each time we will have the same temporary
     */
    vector<string> reg32_list;

void dump_32totemp_map()
{
    cout << "==================" << endl;
    for ( auto& pair : map_32reg2temp) {
        cout << "Key: " << pair.first 
            << ", Value: " << pair.second << endl;
    }
    cout << "==================" << endl;
    for ( auto& pair : map_temp2reg32) {
        cout << "Key: " << pair.first 
            << ", Value: " << pair.second << endl;
    }
    cout << "==================" << endl;
}

void print_address(struct address_node* addr)
{
    cout << "/////////////////"<<endl;
    cout << "type: " << addr->type <<endl;
    cout << "value: " << addr->value <<endl;
    cout << "offset: " << addr->offset <<endl;
    cout << "reg_width: " << addr->register_width <<endl;
    cout << "/////////////////"<<endl;
    
}

bool is_caller_saved(string reg)
{
    if(reg != get_32of64(reg)){
        cout << "ERROR: is_caller_saved() didn't 32bit reg instead got " << reg << endl; 
    }
    if(reg == "%eax")
        return true;
    if(reg == "%edi")
        return true;
    if(reg == "%esi")
        return true;
    if(reg == "%edx")
        return true;
    if(reg == "%ecx")
        return true;
    if(reg == "%r8d")
        return true;
    if(reg == "%r9d")
        return true;
    if(reg == "%r10d")
        return true;
    if(reg == "%r11d")
        return true;

    return false;
    

}

int calc_num_of_used_regs()
{
    int count = 0;
    for(int i = 0; i<reg32_list.size(); i++){
        string reg = reg32_list[i];

        if(map_32reg2temp[reg] != "" && is_caller_saved(reg)){
            count++;
        }
    }

    return count;
}


int num_of_used_regs;
int extra_rsp_shift;

void push_caller_saved_regs(){

    num_of_used_regs = calc_num_of_used_regs();
    if(!num_of_used_regs){
        return;
    }
    extra_rsp_shift = 0;
    int used_regs_width = num_of_used_regs * 8;
    if(used_regs_width%16 != 0){
        extra_rsp_shift  = 8;   //hardcoded shift 
    }

    if(extra_rsp_shift){
        x86 += "subq $8, %rsp    # align 16 " + EOL;
    }

    if(num_of_used_regs){
        x86 += "#   pushing caller saved regs" + EOL;
    }

    for(int i=0; i<reg32_list.size(); i++){
        string reg = reg32_list[i];
        if(map_32reg2temp[reg] != "" && is_caller_saved(reg)){
            /* reg is used, need to save before malloc call */
            calling.push_back(reg); // storing the reg saved 
            x86 += "pushq " + map_32to64[reg] + EOL;
        }
    }

}

void pop_caller_saved_regs()
{

    if(!num_of_used_regs){
        return;
    }
    if(num_of_used_regs){
        x86 += "#   popping caller saved reg " + EOL;
    }


    /* Popping the saved values */
    for(int i=calling.size()-1; i>=0; i--){
        //reverse iterating when popping the value 
        string reg = calling[i];
        x86 += "popq " + map_32to64[reg] + EOL;
    }
    calling.clear(); // emptying the saved infos 
    if(extra_rsp_shift){
        x86 += "addq $8, %rsp     #bring back to 16 align " + EOL;
    }
}

string gen_x86(vector<struct quad_node*> &qcode)
{
    /*	- Starting routines
     *	- Intermediate codes
     *	- Ending routines
     */
    populate_data_structures();
    string data,text;

    map<string,string> data_st;
    int st=0;

    data = ".section .data\n";
    data += "format:\n\t .string \"%d\\n\" \n";

    text = ".section .text" + EOL;

    for(int i=0; i<qcode.size(); i++)
     {
        // adding the original code:
        x86 += "# Instruction number: " + to_string(i) + EOL;
        //cout <<  "# Instruction number: " << i << endl;

        if(qcode[i]->result && qcode[i]->arg1 && qcode[i]->arg2){
            x86 += "# 3AC: " + qcode[i]->result->value + " | = "
                    + qcode[i]->arg1->value + " | " + qcode[i]->op 
                    + " | " + qcode[i]->arg2->value + EOL;
        }

          //if(i == i || i == i)
            //  dump_32totemp_map();

        if(qcode[i]->result && qcode[i]->arg1 && (!qcode[i]->arg2)){

            x86 += "# 3AC: " + qcode[i]->result->value + " | "
                    + qcode[i]->op + " | "
                    + qcode[i]->arg1->value + EOL;
        }

        if(qcode[i]->arg1 && !(qcode[i]->arg2)
            && !(qcode[i]->result)){
            x86 += "# 3AC: " + qcode[i]->op + " | "
                + qcode[i]->arg1->value + EOL;
        }


        string arg1 = "";
        if(qcode[i]->arg1)
            arg1 = get_address_of(qcode[i]->arg1);
        string arg2 = "";
        if(qcode[i]->arg2)    
            arg2  = get_address_of(qcode[i]->arg2);
        string result = "";
        if(qcode[i]->result)
            result = get_address_of(qcode[i]->result);
        string opr  = qcode[i]->op;
        
        /*cout << "ALL ADDRESS POPULATED" << endl;
        if(qcode[i]->result && qcode[i]->result->value == "t_0")
            cout << " t_0 got " << result << endl;
        */

        //if(qcode[i]->arg2 && qcode[i]->arg2->value == "a")
          //  print_address(qcode[i]->arg2);
		if(opr == "assign_to_memory"){
			//cout << "Instr: " << i << endl;
			//cout << "t_1 got " << result << endl;
			string dest = get_free_reg(R64);
			//cout << "upper" << endl;

			x86 += "movq " + result + ", " + dest + EOL;

			if(qcode[i]->arg1->register_width == R64){
				string src = get_free_reg(R64);

			//cout << "upper middle start" << endl;
				x86 += "movq " + arg1 + ", " + src + EOL;
				x86 += "movq " + src + ", (" + dest + ")" + EOL;
				
				make_free(src,R64);
			//cout << "lower middle end" << endl;
			}
			else{
				string src = get_free_reg(R32);
			//cout << "lower middle start" << endl;

				x86 += "movl " + arg1 + ", " + src + EOL;
				x86 += "movl " + src + ", (" + dest + ")" + EOL;
				
				make_free(src,R32);
			//cout << "lower middle end" << endl;
			}

			make_free(dest,R64);
			//cout << "lower " << endl;
		}
        else if(opr == "mem_assign"){
            string base = get_free_reg(R64);
            //cout << "mem_assign got r64 : " << base << "mapped to: "
              //  << map_32reg2temp[get_32of64(base)] << endl;
            string imdt1 = get_free_reg(R32);
            
            // get address of the first element
            x86 += "movq " + result + ", " + base + EOL;
            
            // base = base + i*width 
            if(qcode[i]->arg2->type == "list_index"){
                // this is list initialization where arg2 index harcoded by me
                // actuall byte mein hii hain index --> indentifier for this is list_index 
                 x86 += "addq $" + to_string(qcode[i]->arg2->offset)
                    + ", " + base + EOL;

            }
            else{
                //this is data[index:arg2] = a, where index can be anything 
                // isme byte mein convert krna pdega + 4 wala offset bhi dena hoga 
                string temp = get_free_reg(R32);
                
                x86 += "movl " + arg2 + ", " + temp + EOL;
                if(qcode[i]->arg1->register_width == R64){
                    x86 += "imul $8, " + temp + EOL;
                }
                else{
                   x86 += "imul $4, " + temp + EOL; 
                }
                    x86 += "addq " + map_32to64[temp] + ", " + base + EOL;
                    x86 += "addq $4, " + base + "           # 4byte for length of list" + EOL;

                make_free(temp,R32);
            }
           
            x86 += "movl " + arg1 + ", " + imdt1 + EOL;

            //base[0] = val 
            x86 += "movl " + imdt1 + ", (" + base + ")" + EOL;

            make_free(imdt1,R32);
            make_free(base,R64);
        }
        if(opr == "mem_assign_list"){
            string base = get_free_reg(R64);
            //cout << "mem_assign got r64 : " << base << "mapped to: "
              //  << map_32reg2temp[get_32of64(base)] << endl;
            string imdt1 = get_free_reg(R64);
            
            // get address of the first element
            x86 += "movq " + result + ", " + base + EOL;
           
            if(qcode[i]->arg2->type == "list_index"){
            // base = base + i*width 
                x86 += "addq $" + to_string(qcode[i]->arg2->offset)
                    + ", " + base + EOL;
            }
            else{
                string temp = get_free_reg(R32);
                
                x86 += "movl " + arg2 + ", " + temp + EOL;
                if(qcode[i]->arg1->register_width == R64){
                    x86 += "imul $8, " + temp + EOL;
                }
                else{
                   x86 += "imul $4, " + temp + EOL; 
                }
                    x86 += "addq " + map_32to64[temp] + ", " + base + EOL;
                    x86 += "addq $4, " + base + "           # 4byte for length of list" + EOL;

                make_free(temp,R32);

            }

            x86 += "movq " + arg1 + ", " + imdt1 + EOL;

            //base[0] = val 
            x86 += "movq " + imdt1 + ", (" + base + ")" + EOL;

            make_free(imdt1,R64);
            make_free(base,R64);
        }
        else if(opr == "mem_alloc"){

            push_caller_saved_regs();

            //get_this_reg("%rax");
            //get_this_reg("%rdi");
            
            //map_32reg2temp["%eax"] = "USED";
            //map_32reg2temp["%edi"] = "USED";

            x86 += "movq $" + to_string(qcode[i]->arg1->offset) + 
                ", %rdi" + EOL;

            x86 += "call malloc " + EOL;

            x86 += "movq %rax, " + result + EOL;
            
            //make_free("%edi",R32);
            //make_free("%eax",R32);

            pop_caller_saved_regs();
        }
        else if(opr == "="){
            /* a = b*/
            
            if(qcode[i]->result->register_width == R64){
                string imdt1 = get_free_reg(R64);

                x86 += "movq " + arg1 + ", " + imdt1 + EOL;
                x86 += "movq " + imdt1 + ", " + result + EOL;

                make_free(imdt1,R64);
            }
            else{
                string imdt1 = get_free_reg(R32);



                x86 += "movl " + arg1 + ", " + imdt1 + EOL;
                x86 += "movl " + imdt1 + ", " + result + EOL;

                make_free(imdt1,R32);
            }
        }
        /* add src, dst -> dst += src */
        else if(opr == "+" || opr == "-" || opr == "*")
        {
           
            string imdt1 = get_free_reg(R32);

		    if(qcode[i]->result->register_width == R64){
		   
			    x86 += "movq " + arg1  + ", "  + map_32to64[imdt1] + EOL;
                if(opr == "+")
			    	x86 += "addq " + arg2  + ", "  + map_32to64[imdt1] + EOL;
                else if(opr == "-")
			    	x86 += "subq " + arg2  + ", "  + map_32to64[imdt1] + EOL;
                else
			    	x86 += "imuq " + arg2  + ", "  + map_32to64[imdt1] + EOL;
		        x86 += "movq " + map_32to64[imdt1] + ", "  + result + EOL;
		   }
		   else{
		   
			    x86 += "movl " + arg1  + ", "  + imdt1 + EOL;
			    if(opr == "+")
			    	x86 += "addl " + arg2  + ", "  + imdt1 + EOL;
                else if(opr == "-")
			    	x86 += "subl " + arg2  + ", "  +imdt1 + EOL;
                else
			    	x86 += "imul " + arg2  + ", "  + imdt1 + EOL;
			    x86 += "movl " + imdt1 + ", "  + result + EOL;
		   }
				make_free(imdt1,R32);
        }
        /* idivq S -> signed divide
                      R[%rdx] <- R[%rdx]:R[%rax] mod S
                      R[%rax] <- R[%rdx]:R[%rax] / S */
        else if(opr == "/" || opr == "%" || opr == "//")
        {
            get_this_reg("%edx");
            get_this_reg("%eax");

            map_32reg2temp["%edx"] = "USED";
            map_32reg2temp["%eax"] = "USED";
            /* Since due to above functions, some register mapping 
             * might have changed, we need to get the new mapped reg 
             * but only in case of temps ig :-)
             */
            if( qcode[i]->arg1 && qcode[i]->arg1->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->arg1->value))
            {
                    arg1 = map_temp2reg32[qcode[i]->arg1->value];
                    if(qcode[i]->arg1->register_width == R64)
                        arg1 = map_32to64[arg1];
            }

            if( qcode[i]->arg2 && qcode[i]->arg2->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->arg2->value))
            {

                    arg2 = map_temp2reg32[qcode[i]->arg2->value];

                    if(qcode[i]->arg2->register_width == R64)
                            arg2 = map_32to64[arg2];
            }

            if( qcode[i]->result && qcode[i]->result->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->result->value))
            {
  
                result = map_temp2reg32[qcode[i]->result->value];                  
                    
                    if(qcode[i]->result->register_width == R64)
                        result = map_32to64[result];
            }
            

            x86 += "movl $0, %edx         # clearing already assigned value" + EOL;
    
            x86 += "movl " + arg1 + ", %eax" + EOL;

            //string imdt1 = get_free_reg(R32);

            //x86 += "movl " + arg1 + ", %eax" + EOL;
            x86 += "cltd                   # seems like an special instruction for div" + EOL;
            string temp = get_free_reg(R32);
            x86 += "movl " + arg2 + ", " + temp + EOL; //arg2 constant bhi ho skta 
            x86 += "idivl " + temp + EOL;
            make_free(temp,R32);

            if(opr == "/" || opr == "//")
                x86 += "movl %eax, " + result + EOL;
            else 
                x86 += "movl %edx, " + result + EOL;
        
            //make_free(imdt1,R32);
            make_free("%eax",R32);
            make_free("%edx",R32);
        }
        else if(opr == "==" || opr == "!="
              || opr == ">" || opr == "<"
             || opr == ">=" || opr == "<=")
		{
            //cout << "arg2 before get_this_reg " << arg2 << endl;

            if(qcode[i]->arg1->register_width == 64 && qcode[i]->arg2->register_width == 64 ){

                //relation operation among strings 
                get_this_reg("%rdi");
                get_this_reg("%rsi");
                get_this_reg("%rax");
                /* Since due to above functions, some register mapping 
                * might have changed, we need to get the new mapped reg 
                * but only in case of temps ig :-)
                */


                if( qcode[i]->arg1 && qcode[i]->arg1->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->arg1->value))
                {
                    arg1 = map_temp2reg32[qcode[i]->arg1->value];
                    if(qcode[i]->arg1->register_width == R64)
                        arg1 = map_32to64[arg1];
                }

                if( qcode[i]->arg2 && qcode[i]->arg2->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->arg2->value))
                {

                    arg2 = map_temp2reg32[qcode[i]->arg2->value];

                    if(qcode[i]->arg2->register_width == R64)
                        arg2 = map_32to64[arg2];
                }

                if( qcode[i]->result && qcode[i]->result->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->result->value))
                {
  
                    result = map_temp2reg32[qcode[i]->result->value];                  
                    
                    if(qcode[i]->result->register_width == R64)
                        result = map_32to64[result];
                }

                    //cout << "arg2 before get_this_reg " << arg2 << endl;
    
                if(map_32reg2temp.count(arg1)){
                    x86 += "movq " + map_32to64[arg1]  + ", %rdi" + EOL;
                }
                else{
                    x86 += "movq " + arg1  + ", %rdi" + EOL;
                }
                if(map_32reg2temp.count(arg2)){
                    x86 += "movq " + map_32to64[arg2]  + ", %rsi" + EOL;
                }
                else{
                    x86 += "movq " + arg2  + ", %rsi" + EOL;
                }
                x86 += "call strcmp" + EOL;
                x86 += "cmpl $0, %eax" + EOL;

                string exit_label = get_new_label();

                if(opr == "=="){
                    x86 += "movl $1, " + result + EOL;
                    x86 += "je " + exit_label + EOL;
                    x86 += "movl $0, " + result + EOL;
                }
                else if(opr == "!="){
                    x86 += "movl $0, " + result + EOL;
                    x86 += "je " + exit_label + EOL;
                    x86 += "movl $1, " + result + EOL;
                }
                else if(opr == ">"){
                    x86 += "movl $1, " + result + EOL;
                    x86 += "jg " + exit_label + EOL;
                    x86 += "movl $0, " + result + EOL;

                }
                else if(opr == "<"){
                    x86 += "movl $1, " + result + EOL;
                    x86 += "jl " + exit_label + EOL;
                    x86 += "movl $0, " + result + EOL;
                }
                else if(opr == ">="){
                    x86 += "movl $0, " + result + EOL;
                    x86 += "jl " + exit_label + EOL;
                    x86 += "movl $1, " + result + EOL;

                }
                else if(opr == "<="){
                    x86 += "movl $0, " + result + EOL;
                    x86 += "jg " + exit_label + EOL;
                    x86 += "movl $1, " + result + EOL;

                }
                else 
                    cout << "Invalid Operator among relational ones" << endl;

                
                //****************
                x86.pop_back();
                x86 += exit_label + ":" + EOL;

                make_free("%edi",R32);
                make_free("%esi",R32);
                make_free("%eax",R32);
            }
            else{
                // args are not string 
                get_this_reg("%eax");
                /* Since due to above functions, some register mapping 
                * might have changed, we need to get the new mapped reg 
                * but only in case of temps ig :-)
                */


                if( qcode[i]->arg1 && qcode[i]->arg1->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->arg1->value)){
                        arg1 = map_temp2reg32[qcode[i]->arg1->value];
                        if(qcode[i]->arg1->register_width == R64)
                            arg1 = map_32to64[arg1];
                }

                if( qcode[i]->arg2 && qcode[i]->arg2->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->arg2->value)){

                        arg2 = map_temp2reg32[qcode[i]->arg2->value];

                        if(qcode[i]->arg2->register_width == R64)
                            arg2 = map_32to64[arg2];
                }

                if( qcode[i]->result && qcode[i]->result->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->result->value)){
  
                        result = map_temp2reg32[qcode[i]->result->value];                  
                    
                        if(qcode[i]->result->register_width == R64)
                            result = map_32to64[result];
                }

                //cout << "arg2 before get_this_reg " << arg2 << endl;

                map_32reg2temp["%eax"] = "USED";
                x86 += "movl $0, %eax         #making zero just in case" + EOL;
            
                x86 += "movl " + arg1  + ", %eax" + EOL;
                x86 += "cmpl " + arg2  + ", %eax" + EOL;

                // operator fill zone
                if(opr == "==")
                    x86 += "sete %al" + EOL;
                else if(opr == "!=")
                    x86 += "setne %al" + EOL;
                else if(opr == ">")
                    x86 += "setg %al" + EOL;
                else if(opr == "<")
                    x86 += "setl %al" + EOL;
                else if(opr == ">=")
                    x86 += "setge %al" + EOL;
                else if(opr == "<=")
                    x86 += "setle %al" + EOL;
                else 
                    cout << "Invalid Operator among relational ones" << endl;


                x86 += "movzbl %al, %eax" + EOL;
                x86 += "movl %eax, "  + result + EOL;

                make_free("%eax",R32);
            }
        }
		/* andl src, dst dst &= src */
		else if(opr == "and")
        {
            string imdt1 = get_free_reg(R32);
            string label1 = get_new_label();
            string label2 = get_new_label();

        
            string reg_for_arg1 = arg1,reg_for_arg2 = arg2;

            if(qcode[i]->arg1->type == "INT" ||
                qcode[i]->arg1->type == "BOOL"){
                
                reg_for_arg1 = get_free_reg(R32);
                x86 += "movl " + arg1 + ", " + reg_for_arg1 + EOL;
                arg1 = reg_for_arg1;
            }

            if(qcode[i]->arg2->type == "INT" ||
                qcode[i]->arg2->type == "BOOL"){
                
                reg_for_arg2 = get_free_reg(R32);
                x86 += "movl " + arg2 + ", " + reg_for_arg2 + EOL;
                arg2 = reg_for_arg2;
            }

            x86 += "cmpl $0, " + arg1   + EOL;
            x86 += "je " + label1 + EOL;
            x86 += "cmpl $0, " + arg2   + EOL;
            x86 += "je " + label1 + EOL;
            x86 += "movl $1, "       + result + EOL;
            x86 += "jmp " + label2 + EOL;

            x86 += label1 + ":" + EOL;
                x86 += "movl $0, "   + result + EOL;
            
            x86 += label2 + ":" + EOL;
            
            make_free(imdt1,R32);

            if(map_32reg2temp.count(arg1) && map_32reg2temp[arg1] == "USED"){
                make_free(arg1,R32);
            }
            if(map_32reg2temp.count(arg2) && map_32reg2temp[arg2] == "USED"){
                make_free(arg2,R32);
            }
        }
		/* orl src, dst dst |= src */
        else if(opr == "or")
        {
            string reg_for_arg1 = arg1,reg_for_arg2 = arg2;

            if(qcode[i]->arg1->type == "INT" ||
                qcode[i]->arg1->type == "BOOL"){
                
                reg_for_arg1 = get_free_reg(R32);
                x86 += "movl " + arg1 + ", " + reg_for_arg1 + EOL;
                arg1 = reg_for_arg1;
            }

            if(qcode[i]->arg2->type == "INT" ||
                qcode[i]->arg2->type == "BOOL"){
                
                reg_for_arg2 = get_free_reg(R32);
                x86 += "movl " + arg2 + ", " + reg_for_arg2 + EOL;
                arg2 = reg_for_arg2;
            }

            string imdt1  = get_free_reg(R32);
            string label1 = get_new_label();
            string label2 = get_new_label();
            string label3 = get_new_label();

            x86 += "cmpl $0, " + arg1   + EOL;
            x86 += "jne " + label1 + EOL;
            x86 += "cmpl $0, " + arg2   + EOL;
            x86 += "je " + label2 + "\n";

            x86 += label1 + ":" + EOL;
                x86 += "movl $1, "  + result + EOL;
                x86 += "jmp " + label3 + "\n";

            x86 += label2 + ":" + EOL;
                x86 += "movl $0, " + result + "\n";

            x86 += label3 + ":" + EOL;

            make_free(imdt1,R32);

            if(map_32reg2temp.count(arg1) && map_32reg2temp[arg1] == "USED"){
                make_free(arg1,R32);
            }
            if(map_32reg2temp.count(arg2) && map_32reg2temp[arg2] == "USED"){
                make_free(arg2,R32);
            }
        }
        else if(opr == "addition"){
            /* Unary - do nothing */
            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1+ ", " + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", " + result + EOL;

            make_free(imdt1,R32);
        }
        else if(opr == "minus"){
            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1+ ", " + imdt1 + EOL;
            x86 += "neg " + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", " + result + EOL;

            make_free(imdt1,R32);
        }
        else if(opr == "~"){
            string imdt1 = get_free_reg(R32);
            x86 += "movl " + arg1 + ", " + imdt1 + EOL;
            x86 += "not " + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", " + result + EOL;
            make_free(imdt1,R32);
        }
		/* notl dst dst = ~dst (bitwise inverse) */
        else if(opr == "not")
        {
            string reg_for_arg1 = arg1;

            if(qcode[i]->arg1->type == "INT" ||
                qcode[i]->arg1->type == "BOOL"){
                
                reg_for_arg1 = get_free_reg(R32);
                x86 += "movl " + arg1 + ", " + reg_for_arg1 + EOL;
                arg1 = reg_for_arg1;
            }

            get_this_reg("%eax");
            map_32reg2temp["%eax"] = "USED";

            x86 += "cmpl $0, " + arg1 + EOL;
            x86 += "sete %al" + EOL;
            x86 += "movzbl %al, " + result + EOL;

            make_free("%eax",R32);

            if(map_32reg2temp.count(arg1) && map_32reg2temp[arg1] == "USED"){
                make_free(arg1,R32);
            }
        }
        else if(opr == "&")
        {
            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1 + ", "  + imdt1 + EOL;
            x86 += "andl "  + arg2 + ", " + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", " + result + EOL;

            make_free(imdt1,R32);
        }
        else if(opr == "|")
        {
            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1 + ", "  + imdt1 + EOL;
            x86 += "orl "  + arg2 + ", "  + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", " + result + EOL;

            make_free(imdt1,R32);
        }
        else if (opr == "[obj]") {
                string imdt1 = get_free_reg(R64);

                x86 += "movq " + arg1 + ", " + imdt1 + EOL; //address of first element 
                /* only for list of int as of now */
                x86 += "addq " + arg2 + ", " + imdt1 + EOL;

                if(qcode[i]->result->register_width == R64)
                    x86 += "movq (" + imdt1 + "), " + result + EOL;
                else if(qcode[i]->result->register_width == R32)
                    x86 += "movl (" + imdt1 + "), " + result + EOL;
                else 
                    cout << "ERROR: Invalid Reg width on [obj] handling" << endl;

                make_free(imdt1,R64);

        }
        else if(opr == "[]")
        {
            /* result = arg1[arg2] */ 
            /* -16(rbp) <- arg1 
             * edi <- arg2
             * 
             * movl -16(rbp,rdi,4) %r12
             * movl %r12, result*/

            if(qcode[i]->result->register_width == R32){
                // string offset = get_offset_from_addr(arg1);
                // string reg1   = get_reg_from_addr(arg1);           
                string imdt1 = get_free_reg(R64);
                string imdt2 = get_free_reg(R32);

                x86 += "movq " + arg1 + ", " + imdt1 + EOL; //address of first element 
                /* only for list of int as of now */
                x86 += "movl " + arg2 + ", " + imdt2 + EOL;
                x86 += "imul $4, " + imdt2 + EOL;

                x86 += "addq " + map_32to64[imdt2] + ", " + imdt1 + EOL;
                x86 += "addq $4, " + imdt1 + "      # 4 bytes for len" + EOL;

                x86 += "movl (" + imdt1 + "), " + get_32of64(imdt1) + EOL;

                x86 += "movl " + get_32of64(imdt1) + ", " + result + EOL;


                make_free(imdt2,R32);
                make_free(imdt1,R64);
            }
            else{
                string imdt1 = get_free_reg(R64);
                string imdt2 = get_free_reg(R32);

                x86 += "movq " + arg1 + ", " + imdt1 + EOL; //address of first element 
                /* only for list of int as of now */
                x86 += "movl " + arg2 + ", " + imdt2 + EOL;
                x86 += "imul $8, " + imdt2 + EOL;

                x86 += "addq " + map_32to64[imdt2] + ", " + imdt1 + EOL;
                x86 += "addq $4, " + imdt1 + "      # 4 bytes for len" + EOL;

                x86 += "movq (" + imdt1 + "), " + imdt1 + EOL;

                x86 += "movq " + imdt1 + ", " + result + EOL;


                make_free(imdt2,R32);
                make_free(imdt1,R64);
            }
        }
        else if(opr == "**")
        {
            string imdt1 = get_free_reg(R32);
            string imdt2 = get_free_reg(R32);
            
            if(qcode[i]->arg2->type == "INT" || 
                qcode[i]->arg2->type == "BOOL"){
                string temp = get_free_reg(R32);
                x86 += "movl " + arg2 + ", " + temp + EOL;
                arg2 = temp;
            }

            x86 += "movl " + arg1 + ", " + imdt1 + EOL;
            x86 += "movl $1, "      + imdt2 + EOL;

            x86 += ".powerloop: " + EOL;

                x86 += "imul "  + arg1 +  ", " + imdt1 + EOL;
                x86 += "addl $1, "       + imdt2 + EOL;
                x86 += "cmpl " + imdt2 + ", " + arg2  + EOL;
                x86 += "jne .powerloop " + EOL;
            
            x86 += "movl " + imdt1 + ", " + result + EOL;

            make_free(imdt1,R32);
            make_free(imdt2,R32);

            if(map_32reg2temp.count(arg2) && map_32reg2temp[arg2]=="USED"){
                make_free(arg2,R32);
            }

        }
		/* xor src, dst dst ^= src */
        else if(opr == "^")
        {

            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1  + ", "  + imdt1 + EOL;
            x86 += "xor " + arg2  + ", "  + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", "  + result + EOL;

            make_free(imdt1,R32);
        }
        /* sal count, dst -> dst <<= count */
        else if(opr == "<<" || opr == ">>")
        {
            get_this_reg("%ecx");

            /* Since due to above functions, some register mapping 
             * might have changed, we need to get the new mapped reg 
             * but only in case of temps ig :-)
             */
            map_32reg2temp["%ecx"] = "USED";

            if( qcode[i]->arg1 && qcode[i]->arg1->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->arg1->value)){
                    arg1 = map_temp2reg32[qcode[i]->arg1->value];
                    if(qcode[i]->arg1->register_width == R64)
                        arg1 = map_32to64[arg1];
            }

            if( qcode[i]->arg2 && qcode[i]->arg2->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->arg2->value)){

                arg2 = map_temp2reg32[qcode[i]->arg2->value];

                if(qcode[i]->arg2->register_width == R64)
                        arg2 = map_32to64[arg2];
            }

            if( qcode[i]->result && qcode[i]->result->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->result->value)){
  
                result = map_temp2reg32[qcode[i]->result->value];                  
                    
                    if(qcode[i]->result->register_width == R64)
                        result = map_32to64[result];
            }
            
            x86 += "movl $0, %ecx       # making it zero just in case " + EOL;
            x86 += "movl " + arg2 + ", %ecx" + EOL;
            /* map %ecx so that ye wala use na ho */
            //map_32reg2temp["%ecx"] = "USED"; 

            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1  + ", "  + imdt1 + EOL;
            if(opr == "<<")
                x86 += "shll %cl, "  + imdt1 + EOL;
            else
                x86 += "shr %cl, "  + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", "  + result + EOL;

            make_free(imdt1,R32);
            make_free("%ecx",32);

        }
        else if(opr == "BOOL2INT"){
            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1 + ", " + imdt1 + EOL;
            x86 += "movl " + imdt1 + ", " + result + EOL;

            make_free(imdt1,R32);
        }
        else if(opr == "INT2BOOL"){
            /*
             *  cmpl    $0, -4(%rbp) <--- arg1
             *  je      .L2
             *  movl    $1, -12(%rbp) <--- result 
             *  jmp     .L3
             *.L2:
             *  movl    $0, -12(%rbp)
             *.L3:
             */
            string label1 = get_new_label();
            string label2 = get_new_label();
            string reg;
            if(qcode[i]->arg1->register_width == 32){
                reg = get_free_reg(R32);
                x86 += "movl " + arg1 + ", " + reg +EOL; 
            }
            else{
                reg = get_free_reg(R64);
                x86 += "movq " + arg1 + ", " + reg +EOL;
            }
            x86 += "cmpl  $0, " + reg + EOL;
            x86 += "je " + label1 + EOL;
            x86 += "movl  $1, " + result + EOL;
            x86 += "jmp " + label2 + "\n";
            x86 += label1 + ":" + EOL;
            x86 += "movl $0, " + result + "\n";
            x86 += label2 + ":" + EOL;

            make_free(get_32of64(reg),R32);
        }
        else if( opr == "str_alloc"){
            if(data_st.count(qcode[i]->arg1->value) == 0){
                if((qcode[i]->arg1->value).size()>0 && (qcode[i]->arg1->value)[0] == '\''){
                    (qcode[i]->arg1->value)[0] = '\"';
                    (qcode[i]->arg1->value)[(qcode[i]->arg1->value).size()-1] = '\"';
                }
                data += "str_" +to_string(st)+":\n\t .string " + qcode[i]->arg1->value + "\n\n";
                data_st[qcode[i]->arg1->value] = "str_" + to_string(st);
                st++; 
            }
            
            x86 += "lea " + data_st[qcode[i]->arg1->value] + "(%rip), " + result + EOL; 
            
        }
        else if(is_label(qcode[i]))
        {
            string func_end = "L_func_end";
            int pos1 = func_end.size();

            string begin_func = "L_begin_func";
            int pos2 = begin_func.size();

            string label = (qcode[i])->op;
            
            if(qcode[i]->arg1){
                int temp = stoi(qcode[i]->arg1->value);
                //cout << "///////////////////////" << endl;
                //cout << "func offset was " << temp << endl;
                if(temp%16 != 0){
                    temp = temp + (16 - temp%16);
                    qcode[i]->arg1->value = to_string(temp);
                }
                //cout << "func offset is " << temp << endl;
                //cout << "///////////////////////" << endl;
            }

            if(label.substr(0,pos1) == func_end){
                
                x86.pop_back();
                x86 += "." + label + ": " + EOL;
                x86 += "addq $" +  qcode[i]->arg1->value + ", %rsp" + EOL;
                x86 += "popq %rbp" + EOL;
                x86 += "ret" + EOL;
            }
            else if(label.substr(0,pos2) == begin_func){
                x86 += "pushq %rbp" + EOL;
                x86 += "movq %rsp, %rbp" + EOL;
                x86 += "subq $" + qcode[i]->arg1->value + ", %rsp" + EOL; 
            }
            else{
                x86.pop_back();
                x86 += "." + label + ": " + EOL;
            }
        }
        else if(is_func_name(qcode[i]))
        {
            x86.pop_back();
            x86 += "\n.globl " + opr + "\n";
            x86 += ".type " + opr + ", @function\n";
            x86 += opr + ": \n\t";   
        }
        else if(is_conditional_jump(qcode[i]))
        {
            string imdt1 = get_free_reg(R32);

            x86 += "movl " + arg1 + ", " + imdt1 + EOL;
            x86 += "cmpl $0, " + imdt1 + EOL;
            x86 += "je " + result + EOL;

            make_free(imdt1,R32);
        }
        else if(is_jump(qcode[i])){
            if(arg1 != "")
                x86 += "jmp " + arg1 + EOL;
        }
        else if(is_push_param(qcode[i]))
        {
            if(i+1< qcode.size() && is_call(qcode[i+1]) 
                && qcode[i+1]->arg1->value == "print" 
                && qcode[i]->arg1->register_width == 64){
                   
                push_caller_saved_regs();
                    
                
                if(map_32reg2temp.count(arg1)){
                    x86 += "movq " + map_32to64[arg1] + ", %rdi " + EOL;
                }
                else{
                    x86 += "movq " + arg1 + ", %rdi " + EOL;
                }
                x86 += "movl $0, %eax";

            }
            else if(i+1< qcode.size() && is_call(qcode[i+1]) && qcode[i+1]->arg1->value == "print"){

                push_caller_saved_regs();

                x86 += "movl " + arg1 + ", %esi" + EOL;
                x86 += "lea format(%rip), %rdi " + EOL;
                x86 += "movl $0, %eax";
                
            }
            else{
                /* movl arg1, imdt1
                 * subq $4, %rsp 
                 * movl imdt1, (%rsp)
                 */
                string imdt1;               
                if(qcode[i]->arg1->register_width == R64){
                    imdt1 = get_free_reg(R64);
                    
                    x86 += "movq " + arg1 + ", " + imdt1 + EOL;
                    x86 += "subq $8, %rsp" + EOL;
                    x86 += "movq " + imdt1 + ", (%rsp)" + EOL;
                    
                    make_free(imdt1,R64);
                }
                else if(qcode[i]->arg1->register_width == R32){
                    imdt1 = get_free_reg(R32);

                    x86 += "movl " + arg1 + ", " + imdt1 + EOL;
                    x86 += "subq $4, %rsp" + EOL;
                    x86 += "movl " + imdt1 + ", (%rsp)" + EOL;

                    make_free(imdt1,R32);
                }
                else{
					cout << qcode[i]->arg1->register_width << "\n";
                    cout << "ERROR in is_push_param: Register width out of range" << endl; 
					exit(-1);
				}
                //x86 += "pushq " + arg1 + EOL;
            }
        }
        else if(is_return_value(qcode[i])){
            
             get_this_reg("%eax");

            /* Since due to above functions, some register mapping 
             * might have changed, we need to get the new mapped reg 
             * but only in case of temps ig :-)
             */

            /*map_32reg2temp["%eax"] = "USED";*/

            if( qcode[i]->arg1 && qcode[i]->arg1->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->arg1->value)){
                    arg1 = map_temp2reg32[qcode[i]->arg1->value];
                    if(qcode[i]->arg1->register_width == R64)
                        arg1 = map_32to64[arg1];
                }

             if( qcode[i]->arg2 && qcode[i]->arg2->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->arg2->value)){

                arg2 = map_temp2reg32[qcode[i]->arg2->value];

                if(qcode[i]->arg2->register_width == R64)
                        arg2 = map_32to64[arg2];
             }

             if( qcode[i]->result && qcode[i]->result->type == "temporary" &&
                map_temp2reg32.count(qcode[i]->result->value)){
  
                result = map_temp2reg32[qcode[i]->result->value];                  
                    
                    if(qcode[i]->result->register_width == R64)
                        result = map_32to64[result];
                }

            if(qcode[i]->arg1->register_width == R64)
                x86 += "movq " + arg1 + ", %rax" + EOL;
            else if(qcode[i]->arg1->register_width == R32)
                x86 += "movl " + arg1 + ",%eax" + EOL;
            
            make_free("%eax",R32);
            
            // yahan free nhi hoga 
            // jab finally access kr lenge return value 
            // tb free krna hai 
            /* Update: Code generation pass direction 
             * is different from Code execution pass direction 
             * toh free kr dete hain */



        }
        else if(is_access_return_val(qcode[i]))
        {
            get_this_reg("%eax");
            map_32reg2temp["%eax"] = "USED";

            if(qcode[i]->result->register_width == R64)
                x86 += "movq %rax, " + result + EOL;
            else if(qcode[i]->result->register_width == R32)
                x86 += "movl %eax, " + result + EOL;
            else 
                cout << "ERROR> is_access_param: invalid reg width" << endl;

            /* Finally hamne callee populated eax use kr liye 
             * ab free kr denge usko  */
            make_free("%eax",R32);
            

        }
        else if(is_increment_rsp(qcode[i])){

            x86 += "addq " + arg1 + ", %rsp" + EOL;

        }
        else if(is_call(qcode[i]))
        { 
             

            /*if(qcode[i]->arg1->register_width == R64)
                x86 += "movq " + arg1 + ", %rax" + EOL;
            else if(qcode[i]->arg1->register_width == R32)
                x86 += "movl " + arg1 + ",%eax" + EOL;*/
            
             if(qcode[i]->arg1->value == "print"){
                qcode[i]->arg1->value = "printf";
                x86 += "call " + qcode[i]->arg1->value + EOL;

                pop_caller_saved_regs();
             }
             else{
                get_this_reg("%eax");

                /* Since due to above functions, some register mapping 
                * might have changed, we need to get the new mapped reg 
                * but only in case of temps ig :-)
                */

                map_32reg2temp["%eax"] = "USED";

                if( qcode[i]->arg1 && qcode[i]->arg1->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->arg1->value))
                    arg1 = map_temp2reg32[qcode[i]->arg1->value];

                if( qcode[i]->arg2 && qcode[i]->arg2->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->arg2->value))
                    arg2 = map_temp2reg32[qcode[i]->arg2->value];

                if( qcode[i]->result && qcode[i]->result->type == "temporary" &&
                    map_temp2reg32.count(qcode[i]->result->value))
                    result = map_temp2reg32[qcode[i]->result->value];
                
                x86 += "call " + qcode[i]->arg1->value + EOL;
                make_free("%eax",R32);
             }
        }

        x86 += EOL; //additional line to separate instructions
        check_make_free(qcode[i]);
    }
   
    x86 = data + text  + x86;
    x86 += len_function;
    return x86;
}

/* ----------------------------------------------------------
 *             Register Manipulation functions               
 * ----------------------------------------------------------*/
string get_32of64(string reg64)
{
    //cout << "=========" << endl;
    //cout << "called get_32of64 for " << reg64 << "=========\n" <<endl;
    /* If passed reg 32bit only toh sidhaa return */
    if(map_32reg2temp.find(reg64) != map_32reg2temp.end())
        return reg64;

    //cout << "We are here as well " << endl;

    string reg32;

    for(int i=0; i< reg32_list.size(); i++){

        reg32 = reg32_list[i];

        if( map_32to64[reg32] == reg64)
            return reg32;
    }

    cout << "ERROR: couldn't find the corresponding 32bit reg for "
         << reg64 << endl;
    return "";
}

string get_free_reg(int width)
{
    for(int i=0; i < reg32_list.size(); i++){
        
        string reg = reg32_list[i];

        if(map_32reg2temp[reg] == ""){ // reg is not mapped to any temp
        
            if(width == R32){
                map_32reg2temp[reg] = "USED";
                return reg;
            }
            else 
            if(width == R64){
                //cout << "alloting a r64 of " << reg << endl; 
                map_32reg2temp[reg] = "USED";
                return map_32to64[reg];
            }
            else{
                cout << "Invalid width passed, try 32 or 64" << endl;
                //exit(0);
            }
        }
    }
    cout << "ERROR: No free register found" << endl;
    exit(0);
    return "";
}

void make_free(string reg, int width)
{
    
    /*if(reg == "%rdi" || reg == "%edi"){
        cout << "%edi ko free krne k liye bheja gya iss instr mein" << endl;
    }*/
    //cout << "call make free for " << reg << " | width: " << width << endl;
    if(width == R64){
        //cout << "width == R64 condition" << endl;
        reg = get_32of64(reg);
    }
    
    //cout << "---------\n reg should be r32 " << reg << "-----\n" <<endl;

    if(map_32reg2temp[reg] == ""){
        cout << reg << " is already unmapped, seems like an error" << endl;
        exit(0);
    }

    map_32reg2temp[reg] = "";
    //if(reg == "%eax")
        //cout << " %eax free ho gya " << endl;
}

void check_make_free(struct quad_node* current_instr)
{
   struct address_node* arg1   = current_instr->arg1; 
   struct address_node* arg2   = current_instr->arg2; 
   struct address_node* result = current_instr->result;

    string reg; // reg for each of this operand
    
    /*Argument-1*/
   if(arg1 && arg1->type == "temporary" && !arg1->live){
        //not live after this instr, unmap the reg 
        
        reg = map_temp2reg32[arg1->value];

       // cout << " reg mila " << reg << endl;

        map_temp2reg32.erase(arg1->value); //remove temp2reg mapping
        //map_32reg2temp[get_32of64(reg)] = ""; // remove reg2temp mapping 
        make_free(reg,arg1->register_width);
    }

    /*Argument-2*/
   if(arg2 && arg2->type == "temporary" && !arg2->live){
        //not live after this instr, unmap the reg 
        
        reg = map_temp2reg32[arg2->value];

        map_temp2reg32.erase(arg2->value); //remove temp2reg mapping
        //map_32reg2temp[get_32of64(reg)] = ""; // remove reg2temp mapping 
        make_free(reg,arg2->register_width);
    }

    /*Result-operand*/
   if(result && result->type == "temporary" && !result->live){
        
        reg = map_temp2reg32[result->value];

        map_temp2reg32.erase(result->value); //remove temp2reg mapping
        //map_32reg2temp[get_32of64(reg)] = ""; // remove reg2temp mapping 
        make_free(reg,result->register_width);
    }
}

void get_this_reg( string this_reg)
{
    /* Essentially, I wrote this code for get_edx() to free the edx 
     * but I am extending this to free any specific reg, but the 
     * comments are relative the edx register to bear with it :)
     */

    /*if(this_reg == "%rdi" || this_reg == "%edi"){
        cout << "%edi ki demand hui hai iss instr mein" << endl;
    }*/
    if(!map_32reg2temp.count(this_reg)){
        this_reg = get_32of64(this_reg);
    }
    // string edx = "%rdx";
    string edx = this_reg;
    string t = map_32reg2temp[edx]; // the stored temporary

    /* if already not used toh chill kr*/
    if( t == ""){
        map_32reg2temp[edx] = "USED";
        return;

    }
    else{
        
        if(t == "USED"){
            cout << this_reg << " can't be allocated, already in use" << endl;
        }

        string reg = map_temp2reg32[t];
        if(reg != edx){
            cout << "temp->reg & reg->temp both side mapping not matched" << " reg,edx " << reg << " " << edx << endl;
            exit(0);
        }
        
        string new_reg; // reg to replace edx 

        if(reg == get_32of64(reg)){
            /* width for this temp was 32 */
            new_reg = get_free_reg(R32); //fetch new reg to replace edx
            
        }
        else{
            /* width is 64 bit */
            new_reg = get_free_reg(R64);
        }

        // map the temp to new reg 
        map_temp2reg32[t] = get_32of64(new_reg);

        //map the new reg to the temp 
        //cout << "BEFORE: " << new_reg << " was mapped to " << map_32reg2temp[get_32of64(new_reg)] << endl;
        map_32reg2temp[get_32of64(new_reg)] = t; //may be the new reg is R64 
        //cout << "AFTER: " << new_reg << " is mapped to " << map_32reg2temp[get_32of64(new_reg)] << endl;

        // unmap edx 
        map_32reg2temp[edx] = "";
        //cout << " Reg-exchange between " << edx << " & " << new_reg << " for " << t << endl;
        x86 += "movq " + map_32to64[get_32of64(edx)] + ", " + map_32to64[get_32of64(new_reg)] + " # this instruction is for reg exchange\n\t";  
    
        map_32reg2temp[edx] = "USED";
    }

    map_32reg2temp[edx] = "USED";
}

string get_new_label()
{
    label_count++;
    string label = ".X_" + to_string(label_count); 
    return label;
}

string get_offset_from_addr(string reg){
    int pos = reg.find('(');

    return reg.substr(0,pos);
}

string get_reg_from_addr(string reg){
    int pos1 = reg.find('(');
    int pos2 = reg.find(')');

    return reg.substr(pos1+1,pos2-pos1-1);
}

/* ----------------------------------------------------------
 *    Helpers to find the type of curr instruction               
 * ----------------------------------------------------------*/
bool is_label(struct quad_node* code_entry)
{
    if( code_entry->arg2 == NULL
    && code_entry->result 
    &&  code_entry->op == code_entry->result->value
    &&  (code_entry->op)[0] == 'L' 
    &&  (code_entry->op)[1] == '_')
        return true;
    


    return false;
}

bool is_func_name(struct quad_node* code_entry)
{
    if( code_entry->arg1 == NULL && 
        code_entry->arg2 == NULL &&
        code_entry->result == NULL &&
        code_entry->op != "")
        return true;
    return false;
}

bool is_conditional_jump(struct quad_node* code_entry)
{
    /* ifZ t_1 Goto L2 
     * Check: if the first one is "ifZ"*/

    if(code_entry->op == "ifZ")
        return true;

    return false;
}

bool is_jump(struct quad_node* code_entry)
{
    if(code_entry->op == "Goto")
        return true;

    return false;
}

bool is_push_param(struct quad_node* code_entry)
{
    if(code_entry->op == "push_param")
        return true;

    return false;
}

bool is_return_value(struct quad_node* code_entry)
{
    if(code_entry->op == "return_value")
        return true;
    return false;
}

bool is_access_return_val(struct quad_node* code_entry)
{
    if(code_entry->op == "pop_return_value")
        return true;

    return false;
}

bool is_increment_rsp(struct quad_node* code_entry)
{
    if(code_entry->op == "increment_rsp")
        return true;
    return false;
}

bool is_call(struct quad_node* code_entry)
{
    if(code_entry->op == "call")
        return true;
    return false;
}

bool is_mem_allocation(struct quad_node* code_entry)
{
    /*__unhandled__*/
    return false;
}
bool is_str_allocation(struct quad_node* code_entry)
{
    /*__unhandled__*/
    return false;
}

/* ----------------------------------------------------------
 *      Helper to icorporate the operand info               
 * ----------------------------------------------------------*/

string get_address_of(struct address_node* operand)
{
   if(operand->type == "temporary"){
        /* if temp is already mapped then return 
         * else fetch a new one and return that reg*/
        if(map_temp2reg32.find(operand->value) == map_temp2reg32.end())
        {
            string reg = get_free_reg(operand->register_width);

            // do the mapping 
            map_temp2reg32[operand->value] = get_32of64(reg);
            map_32reg2temp[get_32of64(reg)] = operand->value;
            /*if(operand->value == "t_0")
                cout << "giving out a reg " << reg 
                    << " of width " << operand->register_width << endl;
            */
            return reg;
        }
        else{
            // can be an R32 or an R64 
            switch (operand->register_width) {
                case(R32):
                    return map_temp2reg32[operand->value];

                case(R64):
                    return map_32to64[map_temp2reg32[operand->value]];

                default: 
                    cout << "Default reached here " << endl;
                    break;
            }
        }
    }
    else if(operand->type == "label"){
        return "."+operand->value;
    }
    else if(operand->type == "stack" 
        || operand->type == "heap"
        || operand->type == "list_addr"){
        int offset = operand->offset;
        string addr = to_string(offset*(-1)) + "(%rbp)";

        return addr;
    }
    /*else if(operand->type == "heap"){
        
    }*/
    else if(operand->type == "INT" 
        || operand->type == "BOOL")
    {
        return "$" + operand->value;
    }
    /* __unhandled__ 
     * Many cases are needed to be incorporated here */
    // else{
    //     /* For this block, the operand can be 
    //      * @int 
    //      * @bool 
    //      * @str 
    //      */
    // }
   
    //cout << "Error in func get_address_of" << endl;
    string s="";
    return s;

}



void populate_data_structures()
{

    
    reg32_list.push_back("%edx");
    reg32_list.push_back("%ecx");
    reg32_list.push_back("%r8d");
    reg32_list.push_back("%r9d");
    reg32_list.push_back("%ebx");
    reg32_list.push_back("%r12d");
    reg32_list.push_back("%r13d");
    reg32_list.push_back("%r14d");
    reg32_list.push_back("%r15d");
    /* Intentionally putting this valuable registers below 
     * since these are some special ones are demanded repeatedly*/
    reg32_list.push_back("%eax"); 
    reg32_list.push_back("%edi");  
    reg32_list.push_back("%esi");

    map_32to64["%eax" ] = "%rax";
    map_32to64["%edi" ] = "%rdi";
    map_32to64["%esi" ] = "%rsi";
    map_32to64["%edx" ] = "%rdx";
    map_32to64["%ecx" ] = "%rcx";
    map_32to64["%r8d" ] = "%r8" ;
    map_32to64["%r9d" ] = "%r9" ;
    map_32to64["%ebx" ] = "%rbx";
    map_32to64["%r12d"] = "%r12";
    map_32to64["%r13d"] = "%r13";
    map_32to64["%r14d"] = "%r14";
    map_32to64["%r15d"] = "%r15";

    map_32reg2temp["%eax" ] = "";
    map_32reg2temp["%edi" ] = "";
    map_32reg2temp["%esi" ] = "";
    map_32reg2temp["%edx" ] = "";
    map_32reg2temp["%ecx" ] = "";
    map_32reg2temp["%r8d" ] = "";
    map_32reg2temp["%r9d" ] = "";
    map_32reg2temp["%ebx" ] = "";
    map_32reg2temp["%r12d"] = "";
    map_32reg2temp["%r13d"] = "";
    map_32reg2temp["%r14d"] = "";
    map_32reg2temp["%r15d"] = "";

}
