
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
extern FILE* yyin;

char *tokens[] = {"NULL","KEYWORD","OPERATOR","IDENTIFIER","STRING","DELIMITER","INTEGER","FLOATING_POINT","HEXADECIMAL","COMMENT","STR_ERROR","ERROR"};

// Struct definition
struct lexeme{
	char* parent_lexeme;          			   
	int count;					  				   
	int token_type;				   				   
	struct lexeme* next;		   
};


/*array of lexemes*/
struct lexeme* lex_stream = NULL;

char *to_uppercase(char *str) {
    //char *ptr = (char*)malloc((strlen(str)+1)*sizeof(char));
    char *ptr = (char*)malloc((strlen(str)+1)*sizeof(char));
	strcpy(ptr,str);
	char* temp = ptr;
    while (*temp) {
        *temp = toupper(*temp);
        temp++;
    }
    return ptr;  // Return the modified string
}

//Function to create a new node in the list
struct lexeme* create_new_lex(char* text, int new_count, int type){
	
	struct lexeme* new_lex = (struct lexeme*)malloc(sizeof(struct lexeme));

	new_lex->parent_lexeme = (char*)malloc(strlen(text)+1);
	strcpy(new_lex->parent_lexeme,text);
	new_lex->count = new_count;
	new_lex->token_type = type;
	new_lex->next = NULL;

	return new_lex;

}

void update_count(int new_count, char* text){
	struct lexeme* ptr = lex_stream;

	while(ptr){
		if((ptr->token_type == KEYWORD)|(ptr->token_type == OPERATOR)){

			char* str1 = to_uppercase(ptr->parent_lexeme);
			char* str2 = to_uppercase(text);
			 int capital_diff = strcmp(str1,str2);   
			free(str1); free(str2);
			
			if((capital_diff == 0) && ((ptr->token_type == KEYWORD)|(ptr->token_type == OPERATOR)) ){
				ptr->count = new_count;
			}
		}

		ptr = ptr->next;
	}
}

//Function to insert new str in the list
void insert_in_str_list(struct lexeme* new_lex){

	struct lexeme* ptr = lex_stream;
	struct lexeme* prev = ptr;

	if(ptr == NULL){
		lex_stream = new_lex;
		return;
	}

	while(ptr){
		int diff = strcmp(ptr->parent_lexeme,new_lex->parent_lexeme);
		
		if(diff == 0)
			return;

		if(diff<0){
			
			prev = ptr;
			ptr = ptr->next;
			continue;

		}
		else{
			//str is greater than text
			// is str is ke pehle ghusana hai
			
			if(ptr == lex_stream){
				//1st node se bhi  chhota hai bc
				new_lex->next = lex_stream;
				lex_stream = new_lex;
                
				return;
			}
			else{
				//normal case
				new_lex->next = ptr;
				prev->next = new_lex;
				
				return;
			}
		}
	}

	/*means reached the end of str list*/
	prev->next = new_lex;

	return;
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
		lex_stream = new_lex;

		return ;
	}
	else{
		//traverse and insert at the ascii order place

		struct lexeme* ptr = lex_stream;


		while(ptr){
			char* str1 = to_uppercase(ptr->parent_lexeme);
			char* str2 = to_uppercase(text);
			int capital_diff = strcmp(str1,str2);     //comparison for all upper_cased version
			free(str1); free(str2);

			int diff = strcmp(ptr->parent_lexeme,text);                                         // Normal comparison
			/*
			if(strcmp(text,":=")==0){
				printf("Insert_lex mein toh ghusa hai kam se km\n");
			}
			*/
			if(capital_diff!=0 && diff<0){
				ptr = ptr->next;
				continue;
			}

			if((type == KEYWORD)|(type == OPERATOR)){
				int new_count = 0;
				if(diff == 0){
					//exact same
					new_count = ptr->count+1;
					update_count(new_count,text);
					return;
				}
				if(capital_diff==0){
					new_count = ptr->count+1;
					update_count(new_count,text);

					struct lexeme* new_lex = create_new_lex(text,new_count,type);
					insert_in_str_list(new_lex);

					return;
				}

				//totally different lexeme
				/*
					if(strcmp(text,":=")==0){
						printf("Kya yahan par aaya hai?\n");
					}
				*/
				ptr = ptr->next;
				continue;
			}
			else{
				if(diff ==0){
					//exact same lexeme
					ptr->count++;
					return;
				}
				else{
					//different lexeme
					struct lexeme* new_lex = create_new_lex(text,1,type);
					insert_in_str_list(new_lex);
					return;
				}
			}
		}
		/*means reached the end of keyword list*/

		struct lexeme* new_lex = create_new_lex(text,1,type);
		insert_in_str_list(new_lex);
	}
	
}

// function to print list
void print_lex_stream(struct lexeme* lex_stream){
	
	printf("-------------------------------------------------\n");
	printf("  TOKEN    |     COUNT   |        LEXEME       \n");
	printf("-------------------------------------------------\n");
	while(lex_stream){

			printf("  %s",tokens[lex_stream->token_type]);
		
			/*Code to perfect the identation while printing*/
			int left = 16 - strlen(tokens[lex_stream->token_type]);
			while(left--){
				printf(" ");
			}

			printf(" %d        %s\n",lex_stream->count,lex_stream->parent_lexeme);

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
int contains_ext(char* filestr){
	char* where_dot = strrchr(filestr, '.');

	if(where_dot == NULL || strcmp(where_dot, ".knp")){
		return 0;
	}

	return 1;
}
int main(int argc, char* argv[]){

	if(argc != 2){
		printf("No file found, reading std input:\n");
		yyin = stdin;
	}
	else {
		yyin = fopen(argv[1],"r");
		if(yyin == NULL){
			printf("Can't locate the input file\n");
			return -1;
		}
		if(!contains_ext(argv[1])){
			printf("wrong file format found\n");
			return -1;
		}
	}

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
				//printf("Enter Insert_lex for %s\n",yytext);
				Insert_lex(token,yytext);
				//printf("Returned from Insert_lex for %s\n",yytext);
				/*getting another token stream*/
				token = yylex();
		}

	}
	print_lex_stream(lex_stream);
	return 0;

}

