/*
    Lexical Analyzer for PL/0
    Author: Ryan Doherty

    This program implements a lexical analyzer for PL/0.
    It splits an input file into tokens and interprets the type of
    those tokens. This is a necessary step for making a compiler.
*/
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"

typedef struct node {
    char *token;
    struct node *next;
} token_node;

lexeme *list;
int lex_index;
int input_index;

void printerror(int type);
void printtokens();

int lex_next_token(char *input);

int is_symbol(char c);
int is_reserved(char *string);

token_type symbol_type(char symbol);
token_type reserved_type(char first_char, char second_char);

lexeme *lexanalyzer(char *input) {
    list = malloc(500 * sizeof(lexeme));
    lex_index = 0;
    input_index = 0;

    token_node *token_list = NULL;
    char *current_token = malloc(12);
    int token_index = 0;

    // Parse through each token in my token list
    int has_lex_next_token = 1;
    while (has_lex_next_token) {
        int end_of_token = lex_next_token(input);
        for (int j = input_index; j <= abs(end_of_token); ++j) {
            current_token[token_index] = input[j];
            token_index++;
            if (end_of_token < 0 && j == abs(end_of_token)) {
                has_lex_next_token = 0;
                break;
            }
        }

        // Make a list of tokens
        token_node *new_token = malloc(sizeof(token_node));
        new_token->token = current_token;
        new_token->next = NULL;
        if (token_list == NULL) {
            token_list = new_token;
        } else {
            token_node *current_node = token_list;
            while (current_node->next != NULL) {
                current_node = current_node->next;
            }
            current_node->next = new_token;
        }

        token_index = 0;
        current_token = malloc(11);
        input_index = end_of_token + 1;
    }

    lexeme *current_lexeme;
    token_node *current_node = token_list;
    while (current_node) {
        current_lexeme = &list[lex_index];
        token_type type = -1;

        char first_char = current_node->token[0];
        if (isspace(first_char) || iscntrl(first_char)) {
            current_node = current_node->next;
            continue;
        } else if (is_symbol(first_char)) {
            if (first_char == '/' && current_node->next != NULL && current_node->next->token[0] == '*') {
                int comment_ends = 0;
                while (current_node->next != NULL) {
                    if (current_node->token[0] == '*') {
                        if (current_node->next != NULL && current_node->next->token[0] == '/') {
                            current_node = current_node->next;
                            comment_ends = 1;
                            break;
                        }
                    }
                    current_node = current_node->next;
                }
                if (!comment_ends) {
                    printerror(5);
                    exit(0);
                }
                current_node = current_node->next;
                continue;
            }
            switch (first_char) {
                case ':':
                    if (current_node->next != NULL && current_node->next->token[0] == '=') {
                        type = becomessym;
                        current_node = current_node->next;
                    }
                    break;
                case '=':
                    if (current_node->next != NULL && current_node->next->token[0] == '=') {
                        type = eqlsym;
                        current_node = current_node->next;
                    }
                    break;
                case '<':
                    if (current_node->next != NULL) {
                        if (current_node->next->token[0] == '>') {
                            type = neqsym;
                            current_node = current_node->next;
                            break;
                        } else if (current_node->next->token[0] == '=') {
                            type = leqsym;
                            current_node = current_node->next;
                            break;
                        }
                    }
                    // If it falls through use default branch
                case '>':
                    if (current_node->next != NULL && current_node->next->token[0] == '=') {
                        type = geqsym;
                        current_node = current_node->next;
                        break;
                    }
                default:
                    // Handles all the single symbol tokens
                    type = symbol_type(first_char);
                    break;
            }
        } else if (isalpha(first_char)) {
            if (is_reserved(current_node->token)) {
                type = reserved_type(first_char, current_node->token[1]);
            } else {
                if (strlen(current_node->token) < 12) {
                    type = identsym;
                    strcpy(current_lexeme->name, current_node->token);
                } else {
                    printerror(4);
                    exit(0);
                }
            }
        } else if (isdigit(first_char)) {
            int token_length = strlen(current_node->token);
            for (int i = 0; i < token_length; ++i) {
                if (!isdigit(current_node->token[i])) {
                    printerror(2);
                    exit(0);
                } else if (i >= 5){
                    printerror(3);
                    exit(0);
                }
            }
            current_lexeme->value = atoi(current_node->token);
            type = numbersym;
        } else {
            // If not a digit, letter, control char, or valid symbol, its an invalid symbol
            printerror(1);
            exit(0);
        }

        current_lexeme->type = type;
        list[lex_index] = *current_lexeme;
        lex_index++;
        current_node = current_node->next;
    }
//    printtokens();
    return list;
}

// Returns index of end of the next token after the current input_index
int lex_next_token(char *input) {
    int end_index = input_index;
    char current_char = input[end_index];
    // If the first char is a token or space, return
    // This makes spaces and symbols tokens in our initial list
    // Spaces get removed and symbols get interpreted
    if (isspace(current_char) || is_symbol(current_char)) {
        return end_index;
    }

    while (1) {
        current_char = input[end_index];
        if (current_char == '\0') {
            // End index is negative if we hit the end of file on this token
            return -(end_index);
        } else if (isspace(current_char) || is_symbol(current_char)) {
            return end_index - 1;
        }
        end_index++;
    }
}

// Checks if string is a valid symbol
int is_symbol(char c) {
    char symbols[14] = {'<', '>', '=', ':', ';', ',', '.', '+', '-', '*', '/', '%', '(', ')'};
    if (!isalpha(c) && !isdigit(c)) {
        for (int i = 0; i < 14; ++i) {
            if (c == symbols[i]) {
                return 1;
            }
        }
    }
    return 0;
}

// Checks if string is a reserved word
int is_reserved(char *string) {
    char *reserved_words[14] = {
            "begin", "call", "const", "do", "else", "end", "if",
            "odd", "procedure", "read", "then", "var", "while", "write"
    };
    int reserved_word = 0;
    for (int i = 0; i < 14; ++i) {
        if (strcmp(string, reserved_words[i]) == 0) {
            reserved_word = 1;
        }
    }
    return reserved_word;
}

// Return the type for the single character symbols
token_type symbol_type(char symbol) {
    switch (symbol) {
        case '/':
            return slashsym;
        case '*':
            return multsym;
        case '+':
            return plussym;
        case '-':
            return minussym;
        case '(':
            return lparentsym;
        case ')':
            return rparentsym;
        case '%':
            return modsym;
        case '<':
            return lessym;
        case '>':
            return gtrsym;
        case ';':
            return semicolonsym;
        case ',':
            return commasym;
        case '.':
            return periodsym;
    }
}

// Return the type for each reserved word
token_type reserved_type(char first_char, char second_char){
    token_type  type = 0;
    switch (first_char) {
        case 'b':
            type = beginsym;
            break;
        case 'c':
            if (second_char == 'a') {
                type = callsym;
            } else if (second_char == 'o') {
                type = constsym;
            }
            break;
        case 'd':
            type = dosym;
            break;
        case 'e':
            if (second_char == 'n') {
                type = endsym;
            } else if (second_char == 'l') {
                type = elsesym;
            }
            break;
        case 'i':
            type = ifsym;
            break;
        case 'o':
            type = oddsym;
            break;
        case 'p':
            type = procsym;
            break;
        case 'r':
            type = readsym;
            break;
        case 't':
            type = thensym;
            break;
        case 'v':
            type = varsym;
            break;
        case 'w':
            if (second_char == 'h') {
                type = whilesym;
            } else if (second_char == 'r') {
                type = writesym;
            }
            break;
    }
    return type;
}

void printtokens() {
    int i;
    printf("Lexeme Table:\n");
    printf("lexeme\t\ttoken type\n");
    for (i = 0; i < lex_index; i++) {
        switch (list[i].type) {
            case oddsym:
                printf("%11s\t%d", "odd", oddsym);
                break;
            case eqlsym:
                printf("%11s\t%d", "==", eqlsym);
                break;
            case neqsym:
                printf("%11s\t%d", "<>", neqsym);
                break;
            case lessym:
                printf("%11s\t%d", "<", lessym);
                break;
            case leqsym:
                printf("%11s\t%d", "<=", leqsym);
                break;
            case gtrsym:
                printf("%11s\t%d", ">", gtrsym);
                break;
            case geqsym:
                printf("%11s\t%d", ">=", geqsym);
                break;
            case modsym:
                printf("%11s\t%d", "%", modsym);
                break;
            case multsym:
                printf("%11s\t%d", "*", multsym);
                break;
            case slashsym:
                printf("%11s\t%d", "/", slashsym);
                break;
            case plussym:
                printf("%11s\t%d", "+", plussym);
                break;
            case minussym:
                printf("%11s\t%d", "-", minussym);
                break;
            case lparentsym:
                printf("%11s\t%d", "(", lparentsym);
                break;
            case rparentsym:
                printf("%11s\t%d", ")", rparentsym);
                break;
            case commasym:
                printf("%11s\t%d", ",", commasym);
                break;
            case periodsym:
                printf("%11s\t%d", ".", periodsym);
                break;
            case semicolonsym:
                printf("%11s\t%d", ";", semicolonsym);
                break;
            case becomessym:
                printf("%11s\t%d", ":=", becomessym);
                break;
            case beginsym:
                printf("%11s\t%d", "begin", beginsym);
                break;
            case endsym:
                printf("%11s\t%d", "end", endsym);
                break;
            case ifsym:
                printf("%11s\t%d", "if", ifsym);
                break;
            case thensym:
                printf("%11s\t%d", "then", thensym);
                break;
            case elsesym:
                printf("%11s\t%d", "else", elsesym);
                break;
            case whilesym:
                printf("%11s\t%d", "while", whilesym);
                break;
            case dosym:
                printf("%11s\t%d", "do", dosym);
                break;
            case callsym:
                printf("%11s\t%d", "call", callsym);
                break;
            case writesym:
                printf("%11s\t%d", "write", writesym);
                break;
            case readsym:
                printf("%11s\t%d", "read", readsym);
                break;
            case constsym:
                printf("%11s\t%d", "const", constsym);
                break;
            case varsym:
                printf("%11s\t%d", "var", varsym);
                break;
            case procsym:
                printf("%11s\t%d", "procedure", procsym);
                break;
            case identsym:
                printf("%11s\t%d", list[i].name, identsym);
                break;
            case numbersym:
                printf("%11d\t%d", list[i].value, numbersym);
                break;
        }
        printf("\n");
    }
    printf("\n");
    printf("Token List:\n");
    for (i = 0; i < lex_index; i++) {
        if (list[i].type == numbersym)
            printf("%d %d ", numbersym, list[i].value);
        else if (list[i].type == identsym)
            printf("%d %s ", identsym, list[i].name);
        else
            printf("%d ", list[i].type);
    }
    printf("\n");
    list[lex_index++].type = -1;
}

void printerror(int type) {
    if (type == 1)
        printf("Lexical Analyzer Error: Invalid Symbol\n");
    else if (type == 2)
        printf("Lexical Analyzer Error: Invalid Identifier\n");
    else if (type == 3)
        printf("Lexical Analyzer Error: Excessive Number Length\n");
    else if (type == 4)
        printf("Lexical Analyzer Error: Excessive Identifier Length\n");
    else if (type == 5)
        printf("Lexical Analyzer Error: Neverending Comment\n");
    else
        printf("Implementation Error: Unrecognized Error Type\n");

    free(list);
    return;
}