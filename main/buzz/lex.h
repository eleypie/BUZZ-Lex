#ifndef LEX_H
#define LEX_H

#include <stdio.h>

typedef enum {
    // arithmetic operators
    ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION, 
    MODULO, EXPONENT, INT_DIVISION, ASSIGNMENT_OP,
    
    // relational operators
    GREATER_THAN, LESS_THAN, IS_EQUAL_TO,
    GREATER_EQUAL, LESS_EQUAL, NOT_EQUAL,
    
    // logical operators
    AND, OR, NOT,
    
    // delimiters
    SEMICOLON, COMMA, LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET,
    RIGHT_BRACKET, LEFT_BRACE, RIGHT_BRACE, DBL_QUOTE, SNGL_QUOTE,
    
    // keywords
    BUZZ_TOKEN, BEEGIN_TOKEN, QUEENBEE_TOKEN, BEEGONE_TOKEN, FOR_TOKEN, THIS_TOKEN, 
    IS_TOKEN, WHILE_TOKEN, DO_TOKEN, UPTO_TOKEN, DOWNTO_TOKEN, HIVE_TOKEN, 
    SIZE_TOKEN, STING_TOKEN, IF_TOKEN, RETURNS_TOKEN, ELSEIF_TOKEN, ELSE_TOKEN, 
    HOVER_TOKEN, GATHER_TOKEN, BUZZOUT_TOKEN, SWITCH_TOKEN, CASE_TOKEN, 

    // reserved words
    CHAR_TOKEN, CHAIN_TOKEN, INT_TOKEN, FLOAT_TOKEN, BOOL_TOKEN, TRUE_TOKEN, FALSE_TOKEN, 

    // literals
    INTEGER, FLOAT, STRING,
    
    COMMENT,
    VAR_IDENT,
    FUNC_IDENT,
    NOISE_WORD,
    INVALID,
    END_OF_TOKENS
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    unsigned int line;    // Line number
    unsigned int column;  // Column number
} Token;


// Function Prototypes
Token* lex(FILE *file, size_t *token_count);
int isNumLiteral(char *lexeme, char ch, int *type, FILE *file);
int isKeyword(char *lexeme, char ch, int *type, FILE *file, Token *tokens);
int isReservedWord(char *lexeme, char ch, int *type, FILE *file);
int isNoiseWord(char *lexeme, char ch, int *type, FILE *file);
int isIdentifier(char *lexeme, char ch, int *type, FILE *file);
int isDelimiter(char *lexeme, char ch, int *type, FILE *file);
int isOperator(char *lexeme, char ch, int *type, FILE *file);
char getNextChar(FILE *file);
char getNonBlank(FILE *file);
void storeToken(Token *token, Token *tokens, char *lexeme, int type);

#endif
