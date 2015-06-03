/*
	Interpreter of imperative language LUA.
	School project.
	Lexical analyzer  
	Author:
		Tomas Valek (xvalek02)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 	//strcmp
#include <ctype.h>		//isdigit(c),..
#include <errno.h>
#include "scanner.h" 

#define BULK				32
#define NUMBER_OF_KEYWORDS	24
#define ASCII				(48)
#define MAX_ESC				255
#define NUMBER_OF_ESC		3
#define DEFAULT_STATE		1000

#define ASCII_TEST (c > 1 && c < 32) && (c != 9 && c != 10 && c != 12 && c != 13 )

#define DEBUG 0		//show/hide debugging messages

//FINITE AUTOMAT STATES
enum {
	S_START = 0,
	S_ONE_SUB,	// -
	S_TWO_SUB,	// --
	S_TWO_SUB_SQ_BRACKET,		// --[
	S_TWO_SUB_TWO_SQ_BRACKET,	// --[[
	S_TWO_SUB_THREE_SQ_BRACKET, // --[[..]
	S_SMALLER,	// <
	S_GREATER,	// >
	S_EQUAL,	// =
	S_POINT,	// .
	S_TILDA,	// ~
	S_ID_KEYW,	// identifier or keyword
	S_NUM,		// number~=
	S_NUM_POINT,	//for example 8.
	S_NUM_POINT_NUM,//for example 8.8
	S_NUM_E,		//for example 8e
	S_NUM_E_ADD,	//for example 8e+ neboplus 8.8e+
	S_NUM_E_SUB,	//for example 8e- nebo 8.8e-
	S_NUM_E_ADD_NUM,//for example 8e+10
	S_S,			//string
	S_S_S,			//string22
	S_S_ESC,		//escape '\'
	S_S_ESC_NUM,	//escape number for example \002
};

//Keywords and reserved words
char *key_word[NUMBER_OF_KEYWORDS]={"do","else","end","false","function","if",
"local","nil","read","return","then","true","while","write"/*,"and"*/,"break",
"elseif","for","in"/*,"not","or"*/,"repeat","until","type","substr","find","sort"};

//Struct for string
typedef struct {
	char* string;		//pointer to string
	int alloc_size;		//size
	int lenght;			//length
} T_STRING;


//Global variables
extern FILE *fin;
static unsigned number_line = 1;	//number of line
static unsigned counter_esc = 0;	//counter of number in escape sequence for example \123 == counter_esc = 3
static unsigned number_esc = 0;		//number in escape sequence
static unsigned position = 0;		//position in string

/*******************************************************************************
***********************************STRING***************************************
*******************************************************************************/

/*Initialize string. Allocate memory for string, size:BULK(32bytes).
0 == OK or error
 "s" is pointer to T_STRING struct
*/
int init_string(T_STRING* s){

	if ( (s->string = (char*)malloc(BULK)) == NULL ){//malloc fail
		return 1;
	}

	s->string[0] = '\0';
	s->alloc_size = BULK;
	s->lenght = 0;

	return 0;
}

/*Check is it has more memory, and add 1 char to end of string. If program
has low memory, it malloc more.
OK == 0 or ERR_MALLOC.
 "s" is pointer to T_STRING struct*/
int add_char( char character, T_STRING* s ) {

	if ( s->lenght + 1 >= s->alloc_size  ) {//low memory -> realloc

		char* realloc_address;

		if ( (realloc_address = (char*)realloc(s->string,s->alloc_size*2 )) == NULL ) {
		//DEBUG
		//if ( realloc_address == NULL )
		
			free((void*)s->string);
			s->string = NULL;
			return 1;
		}

		s->alloc_size = s->alloc_size*2;	//edit length of size
		s->string = realloc_address;		//new pointer
	}

	//add character to string
	s->string[s->lenght] = character;
	s->lenght++;
	s->string[s->lenght] = '\0';

	return 0;
}
/*******************************************************************************
*******************************END OF STRINGS***********************************
*******************************************************************************/

/*******************************************************************************
*********************************FUNCTIONS**************************************
*******************************************************************************/

/* The main function in lexical analyzer. Function transitions between states.
If it does not to finite state, it is an error. After show error -> free memory
-> return token with error. If it does to finite state, return token with:
* type - type of token
* a union with:
	* p - pointer to string or identifier
	* i - integer
	* d - double
 About identifiers and strings it is necessary to release memory after calling
 free(token.val.p).
*/
struct token_t get_token(void){

	struct token_t token;

	int state = S_START;			//first state
	int c;							//loading character

	token.type = DEFAULT_STATE;		//default state of token
	token.val.p = NULL;

	T_STRING s;
	s.string = NULL;

	while (1)	{//loading char by char, until finite state

		c = getc(fin);
#if DEBUG
		printf("Character:\t%c\n", c);
#endif
		if ( c == '\n' ){
			++number_line;
		} else if ( ASCII_TEST ) {
		/*if it is on STDIN character with ASCII < 32 && > 1 (exception character as
		 tab, space, etc.) so program writes warning and ignore him. Viz.
		 macro ASCII_TEST*/
			fprintf(stderr,"Lex: Line %u, warning '%c'\n",number_line, c);
			continue;
		} else if ( c == EOF ) { //end of file
			token.type = T_EOF;

			if ( state != S_START ) {
				fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
				goto error;
			}
			free(s.string);
			return token;
		}

		switch (state) {//switch of finite state machine

			case S_START:	//first state
				
				if ( isspace(c) != 0){ //skip whitespace
					state = S_START;
				} else if ( (isalpha(c) != 0 ) || c == '_' ){ //char or '_'

					if ( init_string(&s) == 1 )
						goto system_error;

					if ( add_char(c, &s) == 1 )
						goto system_error;

					state = S_ID_KEYW;
				} else if ( isdigit(c) != 0 ) {				// number

					if ( init_string(&s) == 1 )
						goto system_error;

					if ( add_char(c, &s) == 1 )
						goto system_error;

					state = S_NUM;
				} else if ( c == '"' ) {					//string

					if ( init_string(&s) == 1 )
						goto system_error;

					state = S_S;
				}
				else if ( c == '+' )
					token.type = T_OP_ADD;
				else if ( c == '/' )
					token.type = T_OP_DIV;
				else if ( c == '*' )
					token.type = T_OP_MULT;
				else if ( c == '^' )
					token.type = T_OP_POW;
				else if ( c == '#' ) 						//string length
					token.type = T_OP_STRLEN;
				else if ( c == '(' )
					token.type = T_OP_LBRACKET;
				else if ( c == ')' )
					token.type = T_OP_RBRACKET;
				else if ( c == ',' )
					token.type = T_OP_COMMA;
				else if ( c == ';' )
					token.type = T_OP_SEMIC;
				else if ( c == '-' ) {						//minus
					if ( init_string(&s) == 1 )
						goto system_error;

					state = S_ONE_SUB;
					if ( add_char(c, &s) == 1 )
						goto system_error;

				} else if ( c == '<' )
					state = S_SMALLER;
				else if ( c == '>' )
					state = S_GREATER;
				else if ( c == '=' )
					state = S_EQUAL;
				else if ( c == '.' )
					state = S_POINT;
				else if ( c == '~' )
					state = S_TILDA;
				else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break; //break from switch and continue with while
/*==============================================================================
===========================================================================MINUS
==============================================================================*/
			case S_ONE_SUB:	// -
				if ( c == '-' ) {// --
					state = S_TWO_SUB;
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else {
					if ( c == '\n' )
						--number_line;
					token.type = T_OP_SUB;
					ungetc(c, fin);
					free(s.string);
					s.string = NULL;
				}
				break;
/*==============================================================================
========================================================================COMMENTS
==============================================================================*/
			case S_TWO_SUB:	//--
				if ( c == '[' )
					state = S_TWO_SUB_SQ_BRACKET;
				else {	//comment on oneline
					do {
						if ( c == '\n' || c == EOF )
							break;
						c = getc(fin);
					} while(c != EOF && c != '\n');
					free(s.string);
					s.string = NULL;
#if DEBUG
					printf("STRING DELETED \n");
#endif
					++number_line;
					state = S_START;
				}
				break;

			case S_TWO_SUB_SQ_BRACKET:	// --[
				if ( c == '[' )
					state = S_TWO_SUB_TWO_SQ_BRACKET;
				else {
					do {
						c = getc(fin);
					} while(c != EOF && c != '\n');
					free(s.string);
					s.string = NULL;
#if DEBUG
					printf("STRING DELETED \n");
#endif
					++number_line;
					state = S_START;
				}
				break;

			case S_TWO_SUB_TWO_SQ_BRACKET:	//--[[
				while (1) { //loading symbols
					c = getc(fin);
					if ( c == ']' )	{
						c = getc(fin);
						if ( c == ']' )
							break;
					} else if ( c == '\n' )
						++number_line;
					else if ( c == EOF ) {
						fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
						goto error;
					}
				}
				free(s.string);
				s.string = NULL;
#if DEBUG
				printf("STRING DELETED \n");
#endif
				state = S_START;
				break;
/*==============================================================================
============================================================================LESS
==============================================================================*/
			case S_SMALLER:	// <
				if ( c == '=' )
					token.type = T_OP_EQSM;
				else {
					if ( c == '\n' )
						--number_line;
					token.type = T_OP_SM;
					ungetc(c, fin);
				}
				break;
/*==============================================================================
=========================================================================GREATER
==============================================================================*/
			case S_GREATER:	// >
				if ( c == '=' )
					token.type = T_OP_EQBG;
				else {
					if ( c == '\n' )
						--number_line;
					token.type = T_OP_BG;
					ungetc(c, fin);
				}
				break;
/*==============================================================================
==========================================================================EQUALS
==============================================================================*/
			case S_EQUAL:	// =
				if ( c == '=' )
					token.type = T_OP_EQ;
				else {
					if ( c == '\n' )
						--number_line;
					token.type = T_OP_ASSIGN;
					ungetc(c, fin);
				}
				break;
/*==============================================================================
=============================================================================DOT
==============================================================================*/
			case S_POINT:	// .
				if ( c == '.' )
					token.type = T_OP_CAT;
				else {
					if ( c == '\n' )
						--number_line;
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				} 
				break;
/*==============================================================================
===========================================================================TILDA
==============================================================================*/
			case S_TILDA:	// ~
				if ( c == '=' )
					token.type = T_OP_NOTEQ;
				else {
					if ( c == '\n' )
						--number_line;
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				} 
				break;
/*==============================================================================
===================================================================ID or KEYWORD
==============================================================================*/
			case S_ID_KEYW:	// ID or keyword starts A-Za-z_
				if ( (isalnum(c) != 0) || c == '_' ){ //number, letter or '_'{
					// add to string
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else if ( c == ';' || c == ',' || c == '*' || c == '+' ||\
						 c == '-' || (isspace(c) != 0) || c == '/' || c == '<'\
						 || c == '>' || c == '.' || c == '~' || c == '('\
						 || c == ')' || c == '^' || c == '=' ) {
					//not a number, not a letter, not a '_' so ID ends
					if ( c == '\n' )
						--number_line;
					ungetc(c, fin);
#if DEBUG
					printf("Check: string = %s\n", s.string);
#endif

					for ( int i = 0; i < NUMBER_OF_KEYWORDS; i++ ) {
					//Check keyword
						if ( (strcmp(s.string, key_word[i])) == 0 ) {
							//keyword
							token.type = T_K_DO + i;
							free(s.string);
							s.string = NULL;
							return token;	//return KEY_WORD
						}
					}

					token.type = T_ID;
					token.val.p = s.string;
					//free memory will be outside the function
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;
/*==============================================================================
==========================================================================NUMBER
==============================================================================*/
			case S_NUM: // number
				if ( isdigit(c) != 0 ) { //add to string
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else if( (c == 'e') || (c == 'E') ) {
					if ( add_char(c, &s) == 1 )
						goto system_error;
					state = S_NUM_E;
				} else if ( c == '.' ) {
					if ( add_char(c, &s) == 1 )
						goto system_error;
					state = S_NUM_POINT;
				} else if ( isspace(c) != 0 || c == '+' || c == '-' || c == '*' \
						|| c == '/' || c == '^' || c == '=' || c == ';' || c ==\
						 ',' || c == ')' || c == '~' || c == '<' || c == '>' ) {
					/*These characters can be after number, but they are not
					 component of number.*/
					if ( c == '\n' )
						--number_line;
					ungetc(c, fin);
					token.type = T_VAL_INT;
					//string to int:
					if ( sscanf(s.string, "%d", &token.val.i) != 1 ) {
						//error sscanf
						int err = errno;
						fprintf(stderr, "Error converting '%s' to double: %s\n", s.string, strerror(err));
						goto system_error;
					}
					free(s.string);
					s.string = NULL;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;

			case S_NUM_POINT:	// for example 8.
				if ( isdigit(c) != 0 ) { //add to string
					if ( add_char(c, &s) == 1 )
						goto system_error;
					state = S_NUM_POINT_NUM;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;

			case S_NUM_POINT_NUM:	//for example 8.8
				if ( isdigit(c) != 0 ) {
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else if( (c == 'e') || (c == 'E') ) {
					if ( add_char(c, &s) == 1 )
						goto system_error;
					state = S_NUM_E;
				} else if ( isspace(c) != 0 || c == '+' || c == '-' || c == '*' \
						|| c == '/' || c == '^' || c == '=' || c == ';' || c ==\
						 ',' || c == ')' || c == '~' || c == '<' || c == '>' ) {
					/*These characters can be after number, but they are not
					 component of number.*/
					if ( c == '\n' )
						--number_line;
					ungetc(c, fin);
					token.type = T_VAL_DOUBLE;
					//string na double:
					if ( sscanf(s.string, "%lf", &token.val.d) != 1 ) {
						//error sscanf
						int err = errno;
						fprintf(stderr, "Error converting '%s' to double: %s\n", s.string, strerror(err));
						goto system_error;
					}
					free(s.string);
					s.string = NULL;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;

			case S_NUM_E:	//for example 8e or 8.8e
				if ( c == '+' || c == '-' ) {
					state = S_NUM_E_ADD;
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else if ( isdigit(c) != 0 ) {
					state = S_NUM_E_ADD_NUM;
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;

			case S_NUM_E_ADD:	// for example. 8e+- or 8e
				if ( isdigit(c) != 0 ) {
					state = S_NUM_E_ADD_NUM;
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;

			case S_NUM_E_ADD_NUM:	// for example. 8e+-8
				if ( isdigit(c) != 0 ) {
					state = S_NUM_E_ADD_NUM;
					if ( add_char(c, &s) == 1 )
						goto system_error;
				} else if ( isspace(c) != 0 || c == '+' || c == '-' || c == '*' \
						|| c == '/' || c == '^' || c == '=' || c == ';' || c ==\
						 ',' || c == ')' || c == '~' || c == '<' || c == '>' ) {
					/*These characters can be after number, but they are not
					 component of number.*/
					if ( c == '\n' )
						--number_line;
					ungetc(c, fin);
					token.type = T_VAL_DOUBLE;
					//string to double:
					if ( sscanf(s.string, "%lf", &token.val.d) != 1 ) {
						//error sscanf
						int err = errno;
						fprintf(stderr, "Error converting '%s' to double: %s\n", s.string, strerror(err));
						goto system_error;
					}
					free(s.string);
					s.string = NULL;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;
/*==============================================================================
==========================================================================STRING
==============================================================================*/
			case S_S:	// "
				if ( c == '"' )	{
					token.type = T_VAL_STRING;
					token.val.p = s.string;
					//free memory will be outside the function
				} else if ( c == '\\' ) {
				//these char is not add, because for example \n will be replaced: '\n'
					state = S_S_ESC;
				} else if ( c == 0 || c == EOF ) {//0 is ascii value
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				} else {
					if ( add_char(c, &s) == 1 )
						goto system_error;
					state = S_S;
				}
				break;
/*==============================================================================
==========================================================================ESCAPE
==============================================================================*/
			case S_S_ESC:	/*for example. "..\*/
				if ( isdigit(c) != 0 ) {
					position = s.lenght;	//save place where was:'\' will be replaced esc. sequence
					state = S_S_ESC_NUM;
					counter_esc++;	//sum counter assigned numbers after '\'
					number_esc = number_esc*10+(c-ASCII);
				} else if ( c == '\n' )	{
					state = S_S;
				} else if ( c == 'n' ) {
					if ( add_char('\n', &s) == 1 )
						goto system_error;
					state = S_S;
				} else if ( c == 't' ) {
					if ( add_char('\t', &s) == 1 )
						goto system_error;
					state = S_S;
				} else if ( c == '\\' )	{
					if ( add_char('\\', &s) == 1 )
						goto system_error;
					state = S_S;
				} else if ( c == '\"' )	{
					if ( add_char('\"', &s) == 1 )
						goto system_error;
					state = S_S;
				} else {
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n", number_line, c);
					goto error;
				}
				break;

			case S_S_ESC_NUM:	// for example "..\0-9			
				if ( counter_esc == NUMBER_OF_ESC && number_esc > 0 && number_esc <= MAX_ESC ) {
					//atoi
					//add three numbers from escape sequence:
					s.string[position] = number_esc;
					number_esc = 0;		//reset number for next sequence
					counter_esc = 0;	//reset counter
					s.lenght++;
					s.string[s.lenght] = '\0'; //after esc. sequence must be '\0'
					ungetc(c, fin);
					state = S_S;
				} else if ( isdigit(c) != 0 ) {
					counter_esc++;
					number_esc = number_esc*10+(c-ASCII);
					state = S_S_ESC_NUM;
				} else {
					if ( c == '\n' )
						--number_line;
					fprintf(stderr,"Lex: Line %u, invalid token '%c'\n",number_line, c);
					goto error;
				}
				break;

			default:
				goto error;
		}//end of case

		if ( token.type != DEFAULT_STATE ) {
#if DEBUG
			printf("Line number. %u\n", number_line);
#endif
			return token;
		}

	}//end of while
	printf("Line number. %u\n", number_line);
	return token;

error:
	token.type = T_INVALID;
	free((void*)s.string);
	return token;

system_error:
	token.type = T_ERROR;
	free((void*)s.string);
	return token;

}
