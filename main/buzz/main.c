#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"

const char* VALID_EXTENSION = ".bz";

// Function to check if the file extension is correct
void check_file_type(const char* filename, const char* expectedExtension);

int main(int argc, char *argv[]) {
    // Ensure correct usage of the program with two arguments (input filename and output filename)
    if (argc != 3) {
        fprintf(stderr, "Error: Correct syntax: %s <input_file.bz> <output_file.bz>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Validate input file extension
    check_file_type(argv[1], VALID_EXTENSION);

    // Validate output file extension
    check_file_type(argv[2], VALID_EXTENSION);

    // Open the input file for reading
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: Unable to open file '%s'.\n", argv[1]);
        perror("File Error");
        return EXIT_FAILURE;
    }

    // Debug: Check input file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size == 0) {
        fprintf(stderr, "Error: Input file is empty.\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Open the output file for writing
    FILE *outputFile = fopen(argv[2], "w");
    if (!outputFile) {
        fprintf(stderr, "Error: Unable to create file '%s'.\n", argv[2]);
        perror("File Error");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Tokenize the input file
    size_t token_count = 0;
    Token *tokens = lex(file, &token_count);
    fclose(file);  // Close the input file after lexing

    if (!tokens) {
        fprintf(stderr, "Error: Failed to tokenize input file\n");
        fclose(outputFile);
        return EXIT_FAILURE;
    }

    // Token type strings
    char *token_type[] = {
        "ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", 
        "MODULO", "EXPONENT", "INT_DIVISION", "ASSIGNMENT_OP",
        "GREATER_THAN", "LESS_THAN", "IS_EQUAL_TO",
        "GREATER_EQUAL", "LESS_EQUAL", "NOT_EQUAL",
        "AND", "OR", "NOT",
        "SEMICOLON", "COMMA", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACKET",
        "RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE", "DBL_QUOTE", "SNGL_QUOTE",
        "BUZZ_TOKEN", "BEEGIN_TOKEN", "QUEENBEE_TOKEN", "BEEGONE_TOKEN", "FOR_TOKEN", "THIS_TOKEN", 
        "IS_TOKEN", "WHILE_TOKEN", "DO_TOKEN", "UPTO_TOKEN", "DOWNTO_TOKEN", "HIVE_TOKEN", 
        "SIZE_TOKEN", "STING_TOKEN", "IF_TOKEN", "RETURNS_TOKEN", "ELSEIF_TOKEN", "ELSE_TOKEN", 
        "HOVER_TOKEN", "GATHER_TOKEN", "BUZZOUT_TOKEN", "SWITCH_TOKEN", "CASE_TOKEN",
        "CHAR_TOKEN", "CHAIN_TOKEN", "INT_TOKEN", "FLOAT_TOKEN", "BOOL_TOKEN", "TRUE_TOKEN", "FALSE_TOKEN",
        "INTEGER", "FLOAT", "STRING",
        "COMMENT", "VAR_IDENT", "FUNC_IDENT", "NOISE_WORD", "INVALID", "END_OF_TOKENS"
    };

    // Write tokens to the output file and print to the console
    fprintf(outputFile, "%-20s %-20s\n", "TOKEN", "TOKEN TYPE");
    fprintf(outputFile, "--------------------------------------------\n");

    printf("%-20s %-20s\n", "TOKEN", "TOKEN TYPE");
    printf("--------------------------------------------\n");

    for (int i = 0; tokens[i].type != END_OF_TOKENS; i++) {
        fprintf(outputFile, "%-20s %-20s\n",
                tokens[i].value ? tokens[i].value : "(null)", // Token value
                token_type[tokens[i].type]);                 // Token type

        printf("%-20s %-20s\n",
               tokens[i].value ? tokens[i].value : "(null)", // Token value
               token_type[tokens[i].type]);                  // Token type
    }

    // Free allocated memory for tokens
    for (int i = 0; tokens[i].type != END_OF_TOKENS; i++) {
        if (tokens[i].value) {
            free(tokens[i].value);
        }
    }
    free(tokens);

    fclose(outputFile);  // Close the output file
    printf("Lexical analysis complete. Tokens written to '%s'.\n", argv[2]);

    return EXIT_SUCCESS;
}

// Function to check if the file extension is correct
void check_file_type(const char* filename, const char* expectedExtension) {
    const char *dot = strrchr(filename, '.');
    if (!dot || strcmp(dot, expectedExtension) != 0) {
        fprintf(stderr, "Error: Invalid file type '%s'. Expected '%s'.\n", filename, expectedExtension);
        exit(EXIT_FAILURE);
    }
}
