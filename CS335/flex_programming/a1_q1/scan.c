
/*
	This is the C code to parse the pattern
	returned by the lex file

	author	: Divyansh (210355)
	purpose	: CS335 Assignment 1

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scan.h"

extern int yylex();
extern int yylineno;
extern char* yytext;

char *tokens[] = {"NULL","KEYWORD","OPERATOR","IDENTIFIER","STRING","DELIMITER","INTEGER","FLOATING_POINT","HEXADECIMAL","COMMENT","STR_ERROR","ERROR"};

struct lex_list{
	char* str;
	struct lex_list* next;
};

/*new addition*/
struct lex_list* all_lexemes = NULL;

// Struct definition
struct lexeme{
	//char* parent_lexeme;          			   //All upercase representation of a particular keyword
	struct lex_list* list = all_lexemes;		   //pointer to head of list containing all the strings
	int count;					  				   //universal count for all the variations
	int token_type;				   				   //universal type
	struct lexeme* next;		   
};


/*array of lexemes*/
struct lexeme* lex_stream = NULL;

char *to_uppercase(char *str) {
    char *ptr = (char*)malloc(strlen(str));
	strcpy(ptr,str);
	char* temp = ptr;
    while (*temp) {
        *temp = toupper(*temp);
        temp++;
    }
    return ptr;  // Return the modified string
}

//Function to create a new str node in the list
struct lex_list* create_new_str(char * text){
	struct lex_list* new_str = (struct lex_list*)malloc(sizeof(struct lex_list));
	new_str->str = (char*)malloc(sizeof(text)+1);
	strcpy(new_str->str,text);
	new_str->next = NULL;

	return new_str;
}

//Function to create a new node in the list
struct lexeme* create_new_lex(char* text, int count, int type){

	struct lexeme* new_lex = (struct lexeme*)malloc(sizeof(struct lexeme));

	new_lex->parent_lexeme = (char*)malloc(sizeof(text)+1);
	strcpy(new_lex->parent_lexeme,text);
	//new_lex->list = create_new_str(text);
	new_lex->count = 1;
	new_lex->token_type = type;
	new_lex->next = NULL;

	return new_lex;

}

void print_str_list(struct lex_list* list){

	while(list){
		printf("%s\n",list->str);

		list = list->next;
	}
}
//Function to insert new str in the list
struct lex_list* insert_in_str_list(struct lex_list* list, char* text){
	if(list == NULL){
		struct lex_list* new_ = create_new_str(text);
		list = new_;
		return list;
	}

	struct lex_list* ptr = list;
	struct lex_list* prev = ptr;

	while(ptr){
		int diff = strcmp(ptr->str,text);
		
		if(diff == 0)
			return list;

		if(diff<0){
			
			prev = ptr;
			ptr = ptr->next;
			continue;

		}
		else{
			//str is greater than text
			// is str is ke pehle ghusana hai
			struct lex_list* new_str = create_new_str(text);
			
			if(ptr == list){
				//1st node se bhi  chhota hai bc
				new_str->next = list;
				list = new_str;
                
				return list;
			}
			else{
				//normal case
				new_str->next = ptr;
				prev->next = new_str;
				
				return list;
			}
		}
	}

	/*means reached the end of str list*/
	struct lex_list* new_str = create_new_str(text);
	prev->next = new_str;

	return list;
}

//Function to insert lexeme in list
void Insert_lex(int type, char* text){
	/*
	- lex_stream: start pointer of the list

	strcmp:
		0 if the two strings are equal
		less than 0 if str1 compares less than str2
		greater than 0 if str1 compares greater than str2
	*/	
	
	
	//If no element is there already
	if(!lex_stream){
		struct lexeme* new_lex = create_new_lex(text,1,type);
		all_lexemes = insert_in_str_list(all_lexemes,text);
		new_lex->list = all_lexemes;
		lex_stream = new_lex;

		return ;
	}
	else{
		//traverse and insert at the ascii order place

		struct lexeme* ptr = lex_stream;

		struct lexeme* prev = ptr;


		while(ptr){
			char* str1 = to_uppercase(ptr->parent);
			char* str2 = to_uppercase(text);
			int capital_diff = strcmp(str1,str2);     //comparison for all upper_cased version
			free(str1); free(str2);

			int diff = strcmp(ptr->parent,text);                                         // Normal comparison

			if((type == KEYWORD)|(type == OPERATOR) && capital_diff==0){
				ptr->count++;
				all_lexemes = insert_in_str_list(all_lexemes,text);
				ptr->list = all_lexemes;
				return;
			}
			if(diff == 0){
				/*strings matched*/
				ptr->count++;
				return;
			}
			
			
			if(diff > 0){
				struct lexeme* new_lex = create_new_lex(text,1,type);
				
				/*ptr->lex is greater than lex*/
				if(ptr == lex_stream){
					/*list ke smallest keyword se bhi chhota hai abhi
					  --> means ki start mein hi add krna hai*/

					new_lex->next = lex_stream;

					lex_stream = new_lex;
					return;
				}
				else{
					//aage mein daalo 
					//phir swap kr daalo

					new_lex->next = ptr;
					prev->next = new_lex;

				}
				return;
			}

			if(diff < 0){
				/*ptr->lex is smaller than lex*/
				prev = ptr;
				ptr = ptr->next;
			}
		}
		/*means reached the end of keyword list*/

		struct lexeme* new_lex = create_new_lex(text,1,type);
		prev->next = new_lex;
	}
	
}

// function to print list
void print_lex_stream(struct lexeme* lex_stream){
	
	printf("-------------------------------------------------\n");
	printf("  TOKEN    |     COUNT   |        LEXEME       \n");
	printf("-------------------------------------------------\n");
	while(lex_stream){
		struct lex_list* list = lex_stream->list;
		while(list){

			printf("  %s",tokens[lex_stream->token_type]);
		
			/*Code to perfect the identation while printing*/
			int left = 16 - strlen(tokens[lex_stream->token_type]);
			while(left--){
				printf(" ");
			}

			printf(" %d        %s\n",lex_stream->count,list->str);

			list = list->next;
		}
		lex_stream = lex_stream->next;
	}
}

int count_new_line(char *str){
	int n = strlen(str);
	int count = 0;
	for(int i=0; i<n; i++){
		if(str[i]=='\n'){
			count++;
		}
	}

	return count;
}

int main(){

	int token;
	// Head = lex_stream;

	token = yylex();

	while(token){
		switch(token){
			case(COMMENT):
				token = yylex();
				continue;
			
			case(ERROR):
				print_lex_stream(lex_stream);
				printf("Error found in line no: %d\nEncountered text: %s\n",yylineno,yytext);
				return 0;

			case(STR_ERROR):
				print_lex_stream(lex_stream);
				printf("Ill string found in line no: %d\nEncountered string: %s\n",yylineno-count_new_line(yytext),yytext);
				return 0;

			default:
				Insert_lex(token,yytext);
				/*getting another token stream*/
				token = yylex();
		}

	}
	print_lex_stream(lex_stream);
	return 0;

}

/*New insert node algorithm*/

/*
	* compare the to_uppercase(new_keyword) till a greaeter/equal keyword is found
		+ if(equal found){

			- increment the keyword count
			- insert_in_str_list(new_keyword) 
		}

		+ if(greater found){
			- create a new keyword node
			- setup the parent and all that + new keyword in the list as well
		}

		+ else if reached the end of keyword_list:
			- add the new keyword in the end of the keyword list	
*/
