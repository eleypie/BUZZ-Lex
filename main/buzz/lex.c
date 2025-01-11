#include "lex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

unsigned int line = 1;
unsigned int column = 0;

int tokens_index = 0;
int lexeme_index = 0;
int char_class;

#define COMMENT_CLASS 0
#define LETTER 1
#define DIGIT 2
#define OTHER 3
#define RETURN_TOKEN 1
#define NOISE_WORD 2
#define COMMENT_BEGIN 1001
#define COMMENT_END 1003
#define INCREMENT 1
#define DECREMENT -1


Token *lex(FILE *file, size_t *token_count) {
    int type;

    // Initialize tokens
    int number_of_tokens = 12; // Placeholder value for number of tokens
    int tokens_size = 0;
    Token *tokens = malloc(sizeof(Token) * number_of_tokens);
    if (!tokens) {
        perror("Failed to allocate memory for tokens");
        exit(EXIT_FAILURE);
    }

    // Find length of file
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *lexeme = malloc(sizeof(char) * (length + 1));
    if (!lexeme) {
        perror("Failed to allocate memory for lexeme");
        free(tokens);
        exit(EXIT_FAILURE);
    }

    char ch;

    while ((ch = getNonBlank(file)) != EOF) {
        lexeme[lexeme_index++] = ch; // Build lexeme by character

        Token *token = malloc(sizeof(Token));
        if (!token) {
            perror("Failed to allocate memory for token");
            free(tokens);
            free(lexeme);
            exit(EXIT_FAILURE);
        }
        tokens_size++;

        // Expand token array if needed
        if (tokens_size >= number_of_tokens) {
            number_of_tokens *= 2;
            Token *new_tokens = realloc(tokens, sizeof(Token) * number_of_tokens);
            if (!new_tokens) {
                perror("Failed to reallocate memory for tokens");
                free(tokens);
                free(lexeme);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }

        // Group tokens by composition
        switch (char_class) {
            case COMMENT_CLASS: {
        int state = 0;

        // Start of block comment
        strcpy(lexeme, "<||");
        storeToken(token, tokens, lexeme, COMMENT_BEGIN);

        lexeme_index = 0;

        // Process the block comment content
        while (state != 2) {
            ch = getNextChar(file);

            // End of file: Handle unclosed block comment
            if (ch == EOF) {
                lexeme[lexeme_index] = '\0';
                storeToken(token, tokens, lexeme, COMMENT);
                return tokens; // Exit early for EOF
            }

            switch (state) {
                case 0: // Default state
                    if (ch == ':') {
                        state = 1; // Potential end marker
                    } else {
                        lexeme[lexeme_index++] = ch;
                    }
                    break;

                case 1: // After encountering ':'
                    if (ch == '>') {
                        state = 2; // Confirm end of block comment
                    } else {
                        lexeme[lexeme_index++] = ':';
                        lexeme[lexeme_index++] = ch;
                        state = 0; // Revert to default state
                    }
                    break;
            }
        }

        // Store the block comment content
        lexeme[lexeme_index] = '\0';
        storeToken(token, tokens, lexeme, COMMENT);

        // Store the end marker
        strcpy(lexeme, ":>");
        storeToken(token, tokens, lexeme, COMMENT_END);

        break;
    }


            case LETTER:
                ch = getNextChar(file);
                while (isalpha(ch) && ch != '\n') {
                    lexeme[lexeme_index++] = ch;
                    ch = getNextChar(file);
                }

                if (isdigit(ch)) {
                    while (!isspace(ch) || ch != '\n') {
                        lexeme[lexeme_index++] = ch;
                        ch = getNextChar(file);
                    }
                    storeToken(token, tokens, lexeme, INVALID);
                    break;
                }

                lexeme[lexeme_index] = '\0';
                ungetc(ch, file);

                if (isKeyword(lexeme, ch, &type, file, tokens)) {
                    storeToken(token, tokens, lexeme, type);
                } else if (isReservedWord(lexeme, ch, &type, file)) {
                    storeToken(token, tokens, lexeme, type);
                } else {
                    storeToken(token, tokens, lexeme, INVALID);
                }
                break;

            case DIGIT:
                if (isNumLiteral(lexeme, ch, &type, file)) {
                    storeToken(token, tokens, lexeme, type);
                } else {
                    storeToken(token, tokens, lexeme, INVALID);
                }
                break;

            case OTHER:
                if (isIdentifier(lexeme, ch, &type, file)) {
                    storeToken(token, tokens, lexeme, type);
                } else if (isDelimiter(lexeme, ch, &type, file)) {
                    storeToken(token, tokens, lexeme, type);
                } else if (isOperator(lexeme, ch, &type, file)) {
                    storeToken(token, tokens, lexeme, type);
                } else {
                    storeToken(token, tokens, lexeme, INVALID);
                }
                break;

            default:
                strcpy(lexeme, "Unknown exception");
                storeToken(token, tokens, lexeme, INVALID);
        }

        lexeme_index = 0;
        (*token_count)++;
    }

    // Mark end of tokens
    tokens[tokens_index].value = NULL;
    tokens[tokens_index].type = END_OF_TOKENS;

    free(lexeme);
    return tokens;
}


int isNumLiteral(char *lexeme, char ch, int *type, FILE *file) {
	int has_decimal = 0;
/* no handling for strings starting with numbers
   but ending in letters yet eg. (123sd, 422d)
   
   current output:
   123 - INT
   sd - INVALID
*/	
    
    ch = getNextChar(file);
    while(isdigit(ch) || ch == '.') {
    	if(ch == '.' && has_decimal == 1) {
    		while(isdigit(ch) || ch == '.') { // incorrect float format
				lexeme[lexeme_index++] = ch;  // store any subsequent numbers and decimal points then return INVALID
    			ch = getNextChar(file);
			}
			return 0;
		} else if(ch == '.' && has_decimal == 0) {
			has_decimal = 1;
		}
    	
    	lexeme[lexeme_index++] = ch;
    	ch = getNextChar(file);
	}
            
	ungetc(ch, file);  // Put back the non-numeric character
	column--;
    lexeme[lexeme_index] = '\0';
        
    if(has_decimal) {
    	*type = FLOAT;
	} else {
		*type = INTEGER;
	}
	
	return 1;
}

int isIdentifier(char *lexeme, char ch, int *type, FILE *file) {
	int state = 0;
	
    switch (state) {
    	case 0: // start state
            if (ch == '#' || ch == '~') {
            	state = 1;
            	ch = getNextChar(file);
            } else {
        		return 0; // not a variable
            }
        case 1: 
            if (isalpha(ch)) {
                state = 2; // first char is valid, move to state 2
                lexeme[lexeme_index++] = ch;
                ch = getNextChar(file);
            } else {
            	while(!(isspace(ch) || ch == '\n' || ch == '\t')) { // take entire invalid string
            		lexeme[lexeme_index++] = ch;
                	ch = getNextChar(file);
            	}
            	ungetc(ch, file);
            	column--;
            	*type = INVALID;
                return 0; // invalid variable
            }
        case 2: // checks if next character is valid
            while (isalpha(ch) || isdigit(ch) || ch == '_') {
                lexeme[lexeme_index++] = ch;
                ch = getNextChar(file);
            }

            ungetc(ch, file);
            column--;

            *type = VAR_IDENT;
            return 1; // valid variable identifier
        default:
            return 0; // Invalid variable
    }
}

int isKeyword(char *lexeme, char ch, int *type, FILE *file, Token *tokens) {
	(void)ch;
    (void)file;
    int i = 0;  // Index for lexeme
    ch = lexeme[i];  // Start by checking the first character

   switch(ch) {
    case 'b':  // Start with 'b' for the keyword 'beegin'
        if (strlen(lexeme) == 6 &&
            lexeme[1] == 'e' && 
            lexeme[2] == 'e' && 
            lexeme[3] == 'g' && 
            lexeme[4] == 'i' && 
            lexeme[5] == 'n') {
            *type = BEEGIN_TOKEN;
            return 1;  // Matched 'beegin'
        }
        else if (strlen(lexeme) == 7 &&
                 lexeme[1] == 'e' && 
                 lexeme[2] == 'e' && 
                 lexeme[3] == 'g' && 
                 lexeme[4] == 'o' &&
                 lexeme[5] == 'n' &&
                 lexeme[6] == 'e') {
            *type = BEEGONE_TOKEN;
            return 1; //matched 'beegone'
        }
        else if (strlen(lexeme) == 7 &&
                 lexeme[1] == 'u' && 
                 lexeme[2] == 'z' && 
                 lexeme[3] == 'z' && 
                 lexeme[4] == 'o' &&
                 lexeme[5] == 'u' &&
                 lexeme[6] == 't') {
            *type = BUZZOUT_TOKEN;
            return 1;  // Matched 'buzzout'
        }
        else if (strlen(lexeme) == 4 &&
                 lexeme[1] == 'u' && 
                 lexeme[2] == 'z' && 
                 lexeme[3] == 'z') {
            *type = BUZZ_TOKEN;
            return 1;  // Matched 'buzz'
        }
        break;

    case 'c':  // Start with 'c' for the keyword 'case'
        if (strlen(lexeme) == 4 &&
            lexeme[1] == 'a' && 
            lexeme[2] == 's' && 
            lexeme[3] == 'e') {
            *type = CASE_TOKEN;
            return 1;  // Matched 'case'
        }
        break;

    case 'd':  // Start with 'd' for the keyword 'do' or 'downto'
            if (lexeme[i + 1] == 'o'&&
                lexeme[i + 2] == '\0') {
                *type = DO_TOKEN;
                return 1;  // Matched 'do'
            }
            break;

    case 'e':  // Start with 'e' for the keyword 'else' or 'elseif'
        if (strlen(lexeme) == 6 &&
            lexeme[1] == 'l' && 
            lexeme[2] == 's' && 
            lexeme[3] == 'e' &&
            lexeme[4] == 'i' && 
            lexeme[5] == 'f') {
            *type = ELSEIF_TOKEN;
            return 1;  // Matched 'elseif'
        } 
        else if (strlen(lexeme) == 4 &&
                 lexeme[1] == 'l' && 
                 lexeme[2] == 's' && 
                 lexeme[3] == 'e') {
            *type = ELSE_TOKEN;
            return 1;  // Matched 'else'
        }
        break;

    case 'f':  // Start with 'f' for the keyword 'for'
        if (strlen(lexeme) == 3 &&
            lexeme[1] == 'o' && 
            lexeme[2] == 'r') {
            *type = FOR_TOKEN;
            return 1;  // Matched 'for'
        }
        break;

    case 'g':  // Start with 'g' for the keyword 'gather'
        if (strlen(lexeme) == 6 &&
            lexeme[1] == 'a' && 
            lexeme[2] == 't' && 
            lexeme[3] == 'h' && 
            lexeme[4] == 'e' && 
            lexeme[5] == 'r') {
            *type = GATHER_TOKEN;
            return 1;  // Matched 'gather'
        }
        break;

    case 'h':  // Start with 'h' for the keyword 'hive'
        if (strlen(lexeme) == 4 &&
            lexeme[1] == 'i' && 
            lexeme[2] == 'v' && 
            lexeme[3] == 'e') {
            *type = HIVE_TOKEN;
            return 1;  // Matched 'hive'
       }
        else if (strlen(lexeme) == 5 &&
                 lexeme[1] == 'o' &&
                 lexeme[2] == 'v' &&
                 lexeme[3] == 'e' &&
                 lexeme[4] == 'r') {
            *type = HOVER_TOKEN;
            return 1; //matched hover
        }
        break;

    case 'i':  // Start with 'i' for the keyword 'if' 
        if (strlen(lexeme) == 2 && 
            lexeme[1] == 'f') {
            *type = IF_TOKEN;
            return 1;  // Matched 'if'
        }
        break;

    case 'q':  // Start with 'q' for the keyword 'queenbee'
        if (strlen(lexeme) == 8 &&
            lexeme[1] == 'u' && 
            lexeme[2] == 'e' && 
            lexeme[3] == 'e' && 
            lexeme[4] == 'n' && 
            lexeme[5] == 'b' && 
            lexeme[6] == 'e' && 
            lexeme[7] == 'e') {
            *type = QUEENBEE_TOKEN;
            return 1;  // Matched 'queenbee'
        }
        break;
case 'r':  // Start with 'r' for the keyword 'return'
    if (lexeme[i + 1] == 'e' && 
        lexeme[i + 2] == 't' && 
        lexeme[i + 3] == 'u' && 
        lexeme[i + 4] == 'r' && 
        lexeme[i + 5] == 'n') {
        if (lexeme[i + 6] == '\0') {
            *type = RETURN_TOKEN; 
            return 1; // Matched 'return'
        } else if (lexeme[i + 6] == 'v' &&
                   lexeme[i + 7] == 'a' &&
                   lexeme[i + 8] == 'l' &&
                   lexeme[i + 9] == 'u' &&
                   lexeme[i + 10] == 'e' &&
                   lexeme[i + 11] == '\0') {
            Token *token = malloc(sizeof(Token)); // Declare and allocate memory for token
            if (!token) {
                perror("Failed to allocate memory for token");
                exit(EXIT_FAILURE);
            }

            strcpy(lexeme, "return"); // Replace strcpy_s with strcpy
            lexeme_index = 6;
            *type = RETURN_TOKEN;
            storeToken(token, tokens, lexeme, *type);

            // Prepare for "value" noise word
            strcpy(lexeme, "value"); // Replace strcpy_s with strcpy
            lexeme_index = 5;

            token = malloc(sizeof(Token)); // Allocate memory for a new token
            if (!token) {
                perror("Failed to allocate memory for token");
                exit(EXIT_FAILURE);
            }

            *type = NOISE_WORD;
            storeToken(token, tokens, lexeme, *type);
            free(token); // Free memory if no longer needed

            return 1;
        }
    }
    break;


    case 's':  // Start with 's' for the keyword 'size', 'sting', 'switch'
        if (strlen(lexeme) == 5 &&
                 lexeme[1] == 't' && 
                 lexeme[2] == 'i' && 
                 lexeme[3] == 'n' && 
                 lexeme[4] == 'g') {
            *type = STING_TOKEN;
            return 1;  // Matched 'sting'
        }
        else if (strlen(lexeme) == 6 &&
                 lexeme[1] == 'w' && 
                 lexeme[2] == 'i' && 
                 lexeme[3] == 't' && 
                 lexeme[4] == 'c' && 
                 lexeme[5] == 'h') {
            *type = SWITCH_TOKEN;
            return 1;  // Matched 'switch'
        }
        break;

    case 'w':  // Start with 'w' for the keyword 'while'
        if (strlen(lexeme) == 5 &&
            lexeme[1] == 'h' && 
            lexeme[2] == 'i' && 
            lexeme[3] == 'l' && 
            lexeme[4] == 'e') {
            *type = WHILE_TOKEN;
            return 1;  // Matched 'while'
        }
        break;
        default:
            return 0;
}
return 0;
}

int isReservedWord(char *lexeme, char ch, int *type, FILE *file) {
    (void)ch;   // To suppress unused parameter warning
    (void)file; // To suppress unused parameter warning

    size_t len = strlen(lexeme); // Use size_t to avoid signed/unsigned comparison issues

    switch (lexeme[0]) {
        case 'b':
            if (len == 4 &&
                lexeme[1] == 'o' &&
                lexeme[2] == 'o' &&
                lexeme[3] == 'l') {
                *type = BOOL_TOKEN;
                return 1; // Matched 'bool'
            }
            break;

        case 'c':
            if (len == 4 &&
                lexeme[1] == 'h' &&
                lexeme[2] == 'a' &&
                lexeme[3] == 'r') {
                *type = CHAR_TOKEN;
                return 1; // Matched 'char'
            } else if (len == 5 &&
                       lexeme[1] == 'h' &&
                       lexeme[2] == 'a' &&
                       lexeme[3] == 'i' &&
                       lexeme[4] == 'n') {
                *type = CHAIN_TOKEN;
                return 1; // Matched 'chain'
            }
            break;

        case 'i':
            if (len == 3 &&
                lexeme[1] == 'n' &&
                lexeme[2] == 't') {
                *type = INT_TOKEN;
                return 1; // Matched 'int'
            }
            break;

        case 'f':
            if (len == 5 &&
                lexeme[1] == 'l' &&
                lexeme[2] == 'o' &&
                lexeme[3] == 'a' &&
                lexeme[4] == 't') {
                *type = FLOAT_TOKEN;
                return 1; // Matched 'float'
            } else if (len == 5 &&
                       lexeme[1] == 'a' &&
                       lexeme[2] == 'l' &&
                       lexeme[3] == 's' &&
                       lexeme[4] == 'e') {
                *type = FALSE_TOKEN;
                return 1; // Matched 'false'
            }
            break;

        case 't':
            if (len == 4 &&
                lexeme[1] == 'r' &&
                lexeme[2] == 'u' &&
                lexeme[3] == 'e') {
                *type = TRUE_TOKEN;
                return 1; // Matched 'true'
            }
            break;

        default:
            break; // Not a reserved word
    }

    return 0; // Not a reserved word
}

int isNoiseWord(char *lexeme, char ch, int *type, FILE *file) {
    (void)ch;   // Suppress unused parameter warning
    (void)file; // Suppress unused parameter warning
    (void)type; // Suppress unused parameter warning

    size_t len = strlen(lexeme); // Use size_t to avoid signed/unsigned warnings

    switch (lexeme[0]) { // Check the first character of lexeme
        case 'r': // Check for 'returnvalue'
            if (len == 11 &&
                lexeme[1] == 'e' &&
                lexeme[2] == 't' &&
                lexeme[3] == 'u' &&
                lexeme[4] == 'r' &&
                lexeme[5] == 'n' &&
                lexeme[6] == 'v' &&
                lexeme[7] == 'a' &&
                lexeme[8] == 'l' &&
                lexeme[9] == 'u' &&
                lexeme[10] == 'e') {
                return 1; // Matched 'returnvalue'
            }
            break;

        default:
            break; // Not a noise word
    }

    return 0; // Not a noise word
}


int isDelimiter(char *lexeme, char ch, int *type, FILE *file) {
    (void)lexeme; // Suppress unused parameter warning
    (void)file;   // Suppress unused parameter warning

    switch (ch) {
        case ';':
            *type = SEMICOLON;
            return 1;

        case ',':
            *type = COMMA;
            return 1;

        case '(':
            *type = LEFT_PAREN;
            return 1;

        case ')':
            *type = RIGHT_PAREN;
            return 1;

        case '[':
            *type = LEFT_BRACKET;
            return 1;

        case ']':
            *type = RIGHT_BRACKET;
            return 1;

        case '{':
            *type = LEFT_BRACE;
            return 1;

        case '}':
            *type = RIGHT_BRACE;
            return 1;

        case '"':
            *type = DBL_QUOTE;
            return 1;

        case '\'':
            *type = SNGL_QUOTE;
            return 1;

        default:
            return 0; // Not a delimiter
    }
}


int isOperator(char *lexeme, char ch, int *type, FILE *file) {
    switch (ch) {
        case '+':    
            *type = ADDITION;
            ch = getc(file);
            if (ch == '+') {
                *type = INCREMENT;
                lexeme[lexeme_index++] = ch;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
            }
            lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
            return 1;

        case '-':
            *type = SUBTRACTION;
            ch = getc(file);
            if (ch == '-') {
                *type = DECREMENT;
                lexeme[lexeme_index++] = ch;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
            }
            lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
            return 1;

        case '*':
            *type = MULTIPLICATION;
            lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
            return 1;

        case '/':
            ch = getNextChar(file);
            if (ch == '/') {
                lexeme[lexeme_index++] = ch;
                *type = INT_DIVISION;
                lexeme[lexeme_index] ='\0';
                return 1;
            } else {
                ungetc(ch,file);
                *type = DIVISION;
                lexeme[lexeme_index] = '\0';
                return 1;
            }

        case '%':
            *type = MODULO;
            lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
            return 1;

        case '^':
            *type = EXPONENT;
            lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
            return 1;

        case '>':
            ch = getNextChar(file);
            if (ch == '=') {
                lexeme[lexeme_index++] = ch;
                *type = GREATER_EQUAL;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
                *type = GREATER_THAN;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            }

        case '<':
            ch = getNextChar(file);
            if (ch == '=') {
                lexeme[lexeme_index++] = ch;
                *type = LESS_EQUAL;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
                *type = LESS_THAN;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            }

        case '=':
            ch = getNextChar(file);
            if (ch == '=') {
                lexeme[lexeme_index++] = ch;
                *type = IS_EQUAL_TO;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
                *type = ASSIGNMENT_OP;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            }

        case '&':
            ch = getNextChar(file);
            if (ch == '&') {
                lexeme[lexeme_index++] = ch;
                *type = AND;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
                return 0; // Not a valid operator
            }

        case '|':
            ch = getNextChar(file);
            if (ch == '|') {
                lexeme[lexeme_index++] = ch;
                *type = OR;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
                return 0; // Not a valid operator
            }

        case '!':
            ch = getNextChar(file);
            if (ch == '=') {
                lexeme[lexeme_index++] = ch;
                *type = NOT_EQUAL;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            } else {
                ungetc(ch, file);
                *type = NOT;
                lexeme[lexeme_index] = '\0'; // Null-terminate lexeme
                return 1;
            }

        default:
            return 0; // Not an operator
    }
}



// return any character including spaces or newlines
char getNextChar(FILE *file) {
	char ch = fgetc(file), temp;
	column++;
	
	// seperate characters by class
	if(ch == '<') {
		temp = getc(file);
        if(temp == '|') {
            char_class = COMMENT_CLASS;
            ungetc(temp, file);
        } else {
            ungetc(temp, file);
        }
	} else if(isalpha(ch)) {
		char_class = LETTER;
	} else if(isdigit(ch)) {
		char_class = DIGIT;
	} else {
		char_class = OTHER;
	} 

	return ch;
}

// skip whitespaces and newline
char getNonBlank(FILE *file) {
    char ch = getNextChar(file); // Correct function name capitalization

    while (isspace(ch) || ch == '\t' || ch == '\n') {
        if (ch == '\n') {
            line++;   // Increment line number
            column = 0; // Reset column number
        }
        ch = getNextChar(file); // Get the next character
    }

    return ch; // Return the first non-blank character
}

void storeToken(Token *token, Token *tokens, char *lexeme, int type) {
    token->value = malloc(strlen(lexeme) + 1);
    lexeme[lexeme_index] = '\0';
    strcpy(token->value, lexeme);
    token->line = line;
    token->column = column;
    token->type = type;

    tokens[tokens_index] = *token;
    tokens_index++;

    // Debugging statement
    printf("DEBUG: Stored token -> LINE: %u, COLUMN: %u, LEXEME: '%s', TYPE: %d\n", 
       token->line, token->column, token->value ? token->value : "(null)", token->type);

}


