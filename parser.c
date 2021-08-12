/*
    HW3: Parser for PL/0
    Author: Ryan Doherty
    Email: rdoherty20@knights.ucf.edu

    This program accepts input from the lexer in the
    form of a lexeme array and checks the grammar of the
    file given to the lexer against the given grammar for
    PL/0 and creates a symbol table.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "compiler.h"

symbol *table;
int parser_sym_index;
int error;
lexeme *parser_token_list;
lexeme token;
int parser_token_index = 0;
int parser_level = 0;
int level_current_addr = 3;

void printtable();
void errorend(int x);
lexeme get_next_token();
void add_to_sym_table(token_type type, char *name, int parameter);
bool find_in_sym_table(char *name, token_type type, int declaring);
void parser_mark_level();

void program_declaration();
void block_declaration();
void const_declaration();
void var_declaration();
void proc_declaration();
void statement_declaration();
void expression_declaration();
void condition_declaration();
void term_declaration();
void factor_declaration();
void end_on_error(int i);

symbol *parse(lexeme *input) {
    table = malloc(1000 * sizeof(symbol));
    parser_sym_index = 0;
    error = 0;
    parser_token_list = input;

    // Main is implicit so add it to symbol table
    add_to_sym_table(procsym, "main", 0);
    get_next_token();
    // Start parse tree
    program_declaration();

    // We stop execution and print errors using end_on_error()
//    printtable();
    return table;
}

void errorend(int x) {
    switch (x) {
        case 1:
            printf("Parser Error: Competing Symbol Declarations\n");
            break;
        case 2:
            printf("Parser Error: Unrecognized Statement Form\n");
            break;
        case 3:
            printf("Parser Error: Programs Must Close with a Period\n");
            break;
        case 4:
            printf("Parser Error: Symbols Must Be Declared with an Identifier\n");
            break;
        case 5:
            printf("Parser Error: Constants Must Be Assigned a Value at Declaration\n");
            break;
        case 6:
            printf("Parser Error: Symbol Declarations Must Be Followed By a Semicolon\n");
            break;
        case 7:
            printf("Parser Error: Undeclared Symbol\n");
            break;
        case 8:
            printf("Parser Error: while Must Be Followed By do\n");
            break;
        case 9:
            printf("Parser Error: if Must Be Followed By then\n");
            break;
        case 10:
            printf("Parser Error: begin Must Be Followed By end\n");
            break;
        case 11:
            printf("Parser Error: while and if Statements Must Contain Conditions\n");
            break;
        case 12:
            printf("Parser Error: Conditions Must Contain a Relational-Operator\n");
            break;
        case 13:
            printf("Parser Error: ( Must Be Followed By )\n");
            break;
        case 14:
            printf("Parser Error: call and read Must Be Followed By an Identifier\n");
            break;
        default:
            printf("Implementation Error: Unrecognized Error Code\n");
            break;
    }
}

void printtable() {
    int i;
    printf("Symbol Table:\n");
    printf("Kind | Name        | Value | Level | Address\n");
    printf("--------------------------------------------\n");
    for (i = 0; i < parser_sym_index; i++)
        printf("%4d | %11s | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level,
               table[i].addr);
}

void end_on_error(int i) {
    if (error == 0) {
        error = i;
    }
    // Print error and exit program
    errorend(i);
    free(table);
    exit(0);
}

lexeme get_next_token() {
    // almost definitely unnecessary but to cover our bases
    if (parser_token_index < 500){
        token = parser_token_list[parser_token_index++];
    }
    return token;
}

int is_token(token_type expected_symbol) {
    if (token.type == expected_symbol) {
        return 1;
    }
    return 0;
}

int is_relation(token_type symbol) {
    token_type relational_ops[] = {geqsym, leqsym, gtrsym, lessym, eqlsym, neqsym};
    for (int i = 0; i < 6; ++i) {
        if (symbol == relational_ops[i]) {
            return 1;
        }
    }
    return 0;
}

void add_to_sym_table(token_type type, char *name, int parameter) {
    // Make sure a symbol with a matching type isn't already in the table
    if (find_in_sym_table(name, type, 1)) {
        end_on_error(1);
    }

    // Otherwise add it to table with appropriate values per type
    strcpy(table[parser_sym_index].name, name);
    table[parser_sym_index].level = parser_level;
    table[parser_sym_index].addr = 0;
    switch (type) {
        case constsym:
            table[parser_sym_index].kind = 1;
            table[parser_sym_index].val = parameter;
            break;
        case varsym:
            table[parser_sym_index].kind = 2;
            table[parser_sym_index].addr = level_current_addr++;
            break;
        case procsym:
            table[parser_sym_index].kind = 3;
            break;
        default:
            break;
    }
    parser_sym_index++;
}

bool find_in_sym_table(char *name, token_type type, int declaring) {
    for (int i = 0; i < parser_sym_index; ++i) {
        if (strcmp(name, table[i].name) == 0) {
            // We can have procedures with the same name as consts/vars
            // So skip the "found" condition if the types are wrong
            if (type == procsym){
                if (table[i].kind != 3){
                    continue;
                }
            } else if (type == varsym || type == constsym){
                if (table[i].kind == 3){
                    continue;
                }
            }
            // We can't declare symbols with the same name on the same parser_level
            // We can't use variables from other levels that are marked
            if ((declaring && parser_level == table[i].level) || (!declaring && !table[i].mark)) {
                return 1;
            }
        }
    }
    return 0;
}

void parser_mark_level() {
    // When finished with a procedure, mark its variables so they can't be accessed
    // from the same parser_level in another procedure
    for (int i = 0; i < parser_sym_index; ++i) {
        if (table[i].level == parser_level) {
            table[i].mark = 1;
        }
    }
}

// These functions implement the parse tree outlined in the notes
void program_declaration() {
    block_declaration();
    // Program must end in a period
    if (!is_token(periodsym)) {
        end_on_error(3);
    }
}

void block_declaration() {
    if (is_token(constsym)) {
        const_declaration();
    }
    if (is_token(varsym)) {
        var_declaration();
    }
    if (is_token(procsym)) {
        proc_declaration();
    }
    statement_declaration();
}

void const_declaration() {
    do {
        get_next_token();
        if (!is_token(identsym)) {
            end_on_error(4);
        }
        char *name = malloc(12);
        strcpy(name, token.name);
        get_next_token();
        if (!is_token(becomessym)) {
            end_on_error(5);
            return;
        }
        get_next_token();
        if (!is_token(numbersym)) {
            end_on_error(5);
        }
        // Already declared symbols are handled by add_to_sym_table
        add_to_sym_table(constsym, name, token.value);
        get_next_token();
    } while (is_token(commasym));
    // Consts must end with ;
    if (!is_token(semicolonsym)) {
        end_on_error(6);
    }
    get_next_token();
}

void var_declaration() {
    do {
        get_next_token();
        if (!is_token(identsym)) {
            end_on_error(4);
        }
        char *name = malloc(12);
        strcpy(name, token.name);
        get_next_token();
        add_to_sym_table(varsym, name, 0);
    } while (is_token(commasym));
    if (!is_token(semicolonsym)) {
        // Var declarations must end with ;
        end_on_error(6);
    }
    get_next_token();
}

void proc_declaration() {
    while (is_token(procsym)) {
        get_next_token();
        // Procedures must be named
        if (!is_token(identsym)) {
            end_on_error(4);
        }
        char *name = malloc(12);
        strcpy(name, token.name);
        add_to_sym_table(procsym, name, 0);
        get_next_token();
        // must be followed by a ;
        if (!is_token(semicolonsym)) {
            end_on_error(6);
        }
        get_next_token();

        // Increment the parser_level until we exit the parser_level
        parser_level++;
        int prev_level_addr = level_current_addr;
        level_current_addr = 3;
        block_declaration();
        level_current_addr = prev_level_addr;
        // Mark parser_level when done
        parser_mark_level();
        parser_level--;

        // proc declaration must end with ;
        if (!is_token(semicolonsym)) {
            end_on_error(6);
        }
        get_next_token();
    }
}

void statement_declaration() {
    if (is_token(identsym)) {
        // Look for var with matching name
        if (!find_in_sym_table(token.name, varsym, 0)) {
            end_on_error(7);
        }
        get_next_token();
        if (!is_token(becomessym)) {
            end_on_error(2);
        }
        get_next_token();
        expression_declaration();
        // Handle weird statements that don't match the parse tree
        if (!is_token(semicolonsym) && !is_token(endsym) && !is_token(periodsym)) {
            end_on_error(2);
        }
    } else if (is_token(callsym)) {
        get_next_token();
        if (!is_token(identsym)) {
            end_on_error(14);
        }
        // We can only call procedures
        if (!find_in_sym_table(token.name, procsym, 0)) {
            end_on_error(7);
        }
        get_next_token();
    } else if (is_token(readsym)) {
        get_next_token();
        if (!is_token(identsym)) {
            end_on_error(14);
        }
        // We can only read into variables
        if (!find_in_sym_table(token.name, varsym, 0)) {
            end_on_error(7);
        }
        get_next_token();
    } else if (is_token(writesym)) {
        get_next_token();
        expression_declaration();
    } else if (is_token(beginsym)) {
        get_next_token();
        statement_declaration();
        while (is_token(semicolonsym)) {
            get_next_token();
            statement_declaration();
        }
        if (!is_token(endsym)) {
            end_on_error(10);
        }
        get_next_token();
    } else if (is_token(ifsym)) {
        get_next_token();
        condition_declaration();
        if (!is_token(thensym)) {
            end_on_error(9);
        }
        get_next_token();
        statement_declaration();
    } else if (is_token(whilesym)) {
        get_next_token();
        condition_declaration();
        if (!is_token(dosym)) {
            end_on_error(8);
        }
        get_next_token();
        statement_declaration();
    }
}

void condition_declaration() {
    if (is_token(oddsym)) {
        get_next_token();
        expression_declaration();
    } else if (is_token(dosym) || is_token(thensym)) {
        end_on_error(11);
    } else {
        expression_declaration();
        if (!is_relation(token.type)) {
            end_on_error(12);
        }
        get_next_token();
        expression_declaration();
    }
}

void expression_declaration() {
    if (is_token(plussym) || is_token(minussym)) {
        get_next_token();
    }
    term_declaration();
    while (is_token(plussym) || is_token(minussym)) {
        get_next_token();
        term_declaration();
    }
}

void term_declaration() {
    factor_declaration();
    while (is_token(multsym) || is_token(slashsym) || is_token(modsym)) {
        get_next_token();
        factor_declaration();
    }
}

void factor_declaration() {
    if (is_token(identsym)) {
        if (!find_in_sym_table(token.name, varsym, 0)) {
            end_on_error(7);
        }
        get_next_token();
    } else if (is_token(numbersym)) {
        get_next_token();
    } else if (is_token(lparentsym)) {
        get_next_token();
        expression_declaration();
        if (!is_token(rparentsym)) {
            end_on_error(13);
        }
        get_next_token();
    }
}
