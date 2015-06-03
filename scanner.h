/*
	Interpreter of imperative language LUA.
	School project.
	Lexical analyzer  
	Author:
		Tomas Valek (xvalek02)
	Version: 1.6
*/

#ifndef __SCANNER__H
#define __SCANNER__H

#include <stdio.h>

// LIST OF TOKENS
enum {
	// 1-character
	T_OP_LBRACKET = 0,	//	'('
	T_OP_RBRACKET,		//	')'
	T_OP_COMMA, 		//	','
	T_OP_SEMIC, 		//	';'
	
	T_OP_FIRST, 		//stopper beginning of the block of operators = 4
	T_OP_ADD, 			//	'+'
	T_OP_SUB, 			// 	'-'
	T_OP_DIV, 			// 	'/'
	T_OP_MULT, 			// 	'*'
	T_OP_POW, 			// 	'^'
	T_OP_SM, 			// 	'<'
	T_OP_BG, 			// 	'>'
	T_OP_ASSIGN, 		// 	'='
	T_OP_STRLEN , 		// 	'#'		13

	// 2-characters
	T_OP_EQSM, 			// 	'<='
	T_OP_EQBG, 			// 	'>='
	T_OP_CAT, 			// 	'..'	concate only for tring
	T_OP_EQ, 			// 	'=='
	T_OP_NOTEQ, 		//	'~='
	T_OP_LAST, 			//stopper ending of the block of operators
	
	// multi-characters
	T_ID, 				// identifier 20

	//data types
	T_VAL_INT,
	T_VAL_DOUBLE,
	T_VAL_STRING,		//23

	//keywords
	T_K_DO = 128,		//from 128, because 128 is faster for bit multiplication
	T_K_ELSE,
	T_K_END,
	T_K_FALSE,
	T_K_FUNCTION,
	T_K_IF,
	T_K_LOCAL,
	T_K_NIL,
	T_K_READ,
	T_K_RETURN,
	T_K_THEN, //138
	T_K_TRUE,
	T_K_WHILE,
	T_K_WRITE,
//	T_K_AND,		logical operator are disabled
	T_K_BREAK,
	T_K_ELSEIF,
	T_K_FOR,
	T_K_IN,
//	T_K_NOT,
//	T_K_OR,
	T_K_REPEAT,
	T_K_UNTIL,
	//functions
	T_BF_TYPE,		//151, 148 if logical operator are disabled
	T_BF_SUBSTR,
	T_BF_FIND,
	T_BF_SORT,

	//errors
	T_EOF = EOF, 	//end of file
	T_ERROR = -2, 	//system error -- out of memory, ...
	T_INVALID = -3,	//error of lex. analyzer, bad input
	T_START = -4
};

//Struct of token
struct token_t {
	int type;
	union {
		void *p;	//for string
		int i;		//for int
		double d;	//for double
	} val;
};

struct token_t get_token(void);

#endif
