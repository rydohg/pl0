/*
	Code Generator for PL/0
    Author: Ryan Doherty

    This program takes in the resulting list of lexemes from
    the lexer/HW2 and the symbol table made by the parser/HW3
    and generates code from the parse tree into object code
    that can run on vm.c/HW1 completing the code needed to
    run PL/0 code on our virtual machines.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

instruction *code;
lexeme *token_list;
symbol *symbol_table;

int code_index = 0;
int token_index = 0;
int sym_index = 0;
int level = 0;

lexeme get_token();
lexeme next_token(int num_times);
void unmark_symbol(char *ident_name, int kind);
void mark_level(int start_index, int end_index);
int find_ident(char *ident_name, int type);
int scoped_find_ident(char* name, int kind);
void gen_code(int op, int l, int m);
void printcode();

void program_gen();
void block_gen();
void statement_gen();
int const_gen();
int var_gen();
int proc_gen();
void expression_gen();

// Character representations of instruction codes
typedef enum {
    LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SYS
} instruction_type;

instruction *generate_code(lexeme *tokens, symbol *symbols) {
    code = malloc(500 * sizeof(instruction));
    symbol_table = symbols;
    token_list = tokens;
    // Initialize level to be negative since block_gen()
    // increments level each time
    level = -1;
    sym_index = 0;

    program_gen();

    printcode();
    return code;
}

void condition_gen() {
    lexeme token = get_token();
    if (token.type == oddsym){
        // Add the ODD instruction after the expression that follows the odd symbol
        next_token(1);
        expression_gen();
        gen_code(OPR, 0, 6);
    } else {
        // Otherwise generate the expressions then use the appropriate relation on it after
        expression_gen();
        token_type relation = get_token().type;
        next_token(1);
        expression_gen();
        switch (relation) {
            // Each case generates the matching instruction
            case eqlsym:
                gen_code(OPR, 0, 8);
                break;
            case neqsym:
                gen_code(OPR, 0, 9);
                break;
            case lessym:
                gen_code(OPR, 0, 10);
                break;
            case leqsym:
                gen_code(OPR, 0, 11);
                break;
            case gtrsym:
                gen_code(OPR, 0, 12);
                break;
            case geqsym:
                gen_code(OPR, 0, 13);
                break;
            default:
                break;
        }
    }
}

void factor_gen() {
    lexeme token = get_token();
    if (token.type == identsym){
        int index = scoped_find_ident(token.name, 4);
        // Put the value onto the stack if it's a const
        if (symbol_table[index].kind == 1){
            gen_code(LIT, 0, symbol_table[index].val);
        }
        // Load variable from its level
        else if (symbol_table[index].kind == 2){
            gen_code(LOD, level - symbol_table[index].level, symbol_table[index].addr);
        }
        next_token(1);
    } else if (token.type == numbersym){
        // Put the number's value on the stack
        gen_code(LIT, 0, token.value);
        next_token(1);
    }
    // Otherwise it is an expression
    else {
        next_token(1);
        expression_gen();
        next_token(1);
    }
}

void term_prime_gen() {
    // Generate code for mult/div/modulus
    lexeme token = token_list[token_index];
    if (token.type == multsym){
        next_token(1);
        factor_gen();
        gen_code(OPR, 0, 4);
        term_prime_gen();
    } else if (token.type == slashsym){
        next_token(1);
        factor_gen();
        gen_code(OPR, 0, 5);
        term_prime_gen();
    } else if (token.type == modsym){
        next_token(1);
        factor_gen();
        gen_code(OPR, 0, 7);
        term_prime_gen();
    }
}

void term_gen() {
    // Generate the code to get the factor onto the stack then apply the operation
    factor_gen();
    term_prime_gen();
}

void expression_prime_gen() {
    // Implement order of operations by generating the terms (higher precedence)
    // before the +/- operations
    lexeme token = token_list[token_index];
    if (token.type == plussym){
        next_token(1);
        term_gen();
        gen_code(OPR, 0, 2);
        expression_prime_gen();
    } else if (token.type == minussym){
        next_token(1);
        term_gen();
        gen_code(OPR, 0, 3);
        expression_prime_gen();
    }
}

void expression_gen() {
    // Handle positive/negative then generate the rest of the expression as a list
    lexeme token = token_list[token_index];
    if (token.type == plussym){
        next_token(1);
        term_gen();
        expression_prime_gen();
    } else if (token.type == minussym){
        next_token(1);
        term_gen();
        // Negate value
        gen_code(OPR, 0, 1);
    } else {
        term_gen();
        expression_prime_gen();
    }
}

void statement_list_gen(){
    // Keep generating statement code until we hit a line not ending with ;
    lexeme token = get_token();
    if (token.type == semicolonsym){
        next_token(1);
        statement_gen();
        statement_list_gen();
    }
}

void statement_gen(){
    lexeme token = token_list[token_index];
    if (token.type == identsym){
        // Assignment stores the valwith ue generated by the expression code into the address
        // of the closest var in scope
        int index = scoped_find_ident(token.name, 2);
        next_token(2);
        expression_gen();
        gen_code(STO, level - symbol_table[index].level, symbol_table[index].addr);
    } else if (token.type == callsym){
        // Call the procedure whose code index is stored in its val property
        token = next_token(1);
        int index = scoped_find_ident(token.name, 3);
        next_token(1);
        gen_code(CAL, level - symbol_table[index].level, symbol_table[index].val * 3);
    } else if (token.type == writesym){
        // Write the result of the given expression to the screen
        next_token(1);
        expression_gen();
        gen_code(SYS, 0, 1);
    } else if (token.type == readsym){
        // Read from the system then store into the closest in scope variable
        token = next_token(1);
        int index = scoped_find_ident(token.name, 2);
        next_token(1);
        gen_code(SYS, 0, 2);
        gen_code(STO, level - symbol_table[index].level, symbol_table[index].addr);
    } else if (token.type == beginsym){
        // Generate a statement then continue if the statement ends with a ;
        next_token(1);
        statement_gen();
        statement_list_gen();
        next_token(1);
    } else if (token.type == ifsym){
        // Generate the code for the condition
        next_token(1);
        condition_gen();
        // Store the index of the conditional jump instruction
        // so we can update the address after more code gen
        int jpc_index = code_index;
        gen_code(JPC, 0, 0);
        next_token(1);
        // Generate the if true code
        statement_gen();

        token = get_token();
        if (token.type == elsesym){
            // If there's an else branch set the JPC's addr
            // to right after the JMP (execute only else branch if false)
            // generate the else code, then set the JMP to skip the else
            // branch if the if branch executes
            next_token(1);
            int jmp_index = code_index;
            gen_code(JMP, 0, 0);
            code[jpc_index].m = code_index * 3;
            statement_gen();
            code[jmp_index].m = code_index * 3;
        } else {
            // Otherwise set the JPC to after the if branch
            code[jpc_index].m = code_index * 3;
        }
    } else if (token.type == whilesym){
        next_token(1);
        // Keep track of JMP and JPC to be updated later
        int jmp_index = code_index;
        // Generate condition code
        condition_gen();
        next_token(1);
        int jpc_index = code_index;
        gen_code(JPC, 0, 0);
        // Generate statements inside loop then update jump addrs to after
        // these statements
        statement_gen();
        // Jump back to start of loop if condition is true
        gen_code(JMP, 0, jmp_index * 3);
        // Jump out of loop if condition is false
        code[jpc_index].m = code_index * 3;
    }
}

int proc_gen() {
    int num_procs = 1;
    lexeme token = next_token(1);
    // Unmark procedure
    unmark_symbol(token.name, 3);
    next_token(2);
    // Generate its code
    block_gen();
    token = next_token(1);
    // Return after its code is executed
    gen_code(OPR, 0, 0);
    // Recursively generate multiple procedures and keep track of how many
    if (token.type == procsym){
        num_procs += proc_gen();
    }
    return num_procs;
}

int var_gen() {
    // Recursively unmark and count num of vars
    int num_vars = 1;
    lexeme token = next_token(1);
    unmark_symbol(token.name, 2);
    token = next_token(1);
    if (token.type == commasym){
        num_vars += var_gen();
    }
    return num_vars;
}

int const_list_gen(){
    // Recursively unmark a list of constants
    int num_consts = 1;
    unmark_symbol(next_token(1).name, 1);
    lexeme token = next_token(3);
    if (token.type == commasym){
        num_consts += const_list_gen();
    }
    return num_consts;
}

int const_gen() {
    // Unmark const and recursively count the following in the list if there are any
    int num_consts = 1;
    lexeme token = next_token(1);
    unmark_symbol(token.name, 1);
    token = next_token(3);
    if (token.type == commasym){
        num_consts += const_list_gen();
    }
    return num_consts;
}

void block_gen() {
    // Keep track of the procedures index
    // which is 1 less since unmark advances sym_index
    int proc_index = sym_index - 1;
    // Increment level to implement scoping
    level++;
    lexeme token = get_token();
    int num_consts = 0;
    if (token.type == constsym) {
        num_consts = const_gen();
        token = next_token(1);
    }
    int num_vars = 0;
    if (token.type == varsym) {
        num_vars = var_gen();
        token = next_token(1);
    }
    int num_procs = 0;
    if (token.type == procsym) {
        num_procs = proc_gen();
    }
    // Store the start of this procedure's code in its
    // val property to jump to later
    symbol_table[proc_index].val = code_index;
    // Allocate space for this procedure's variables in this level
    gen_code(INC, 0, num_vars + 3);
    // Generate this procedure's code
    statement_gen();
    // Mark this level's variables/consts/procedures once they can't be used
    // to implement scoping
    mark_level(proc_index + 1, proc_index + 1 + num_consts + num_vars + num_procs);
    // Decrement level when done for scope
    level--;
}

void program_gen() {
    // Unmark main
    unmark_symbol("main", 3);
    // Generate a jump which gets updated to the start of main's code
    // after all other code is generated
    gen_code(JMP, 0, 0);
    block_gen();
    // Halt when program is done
    gen_code(SYS, 0, 3);
    code[0].m = symbol_table[0].val * 3;
}

lexeme get_token() {
    return token_list[token_index];
}

lexeme next_token(int num_times) {
    // Increments the token_index and returns that token
    lexeme type;
    for (int i = 0; i < num_times; ++i) {
        token_index++;
        type = token_list[token_index];
    }
    return type;
}

void unmark_symbol(char *ident_name, int kind) {
    int index = find_ident(ident_name, kind);
    if (index != -1) {
        symbol_table[index].mark = 0;
        sym_index++;
    }
}

void mark_level(int start_index, int end_index) {
    symbol* temp_symbol;
    for (int i = start_index; i < end_index; ++i) {
        temp_symbol = &symbol_table[i];
        temp_symbol->mark = 1;
    }
}

int find_ident(char *ident_name, int type) {
    // Finds first marked symbol with a matching type
    // since symbols after sym_index are marked
    int index = sym_index;
    symbol temp_symbol = symbol_table[index];
    while (temp_symbol.kind != -1 && strcmp(temp_symbol.name, "") != 0) {
        if (strcmp(symbol_table[index].name, ident_name) == 0 && symbol_table[index].kind == type) {
            return index;
        }
        index++;
        temp_symbol = symbol_table[index];
    }
    return -1;
}

int scoped_find_ident(char* name, int kind){
    // Find the matching symbol closest in scope
    int index = -1;
    int closest_level = -1;
    for (int i = 0; i < sym_index; ++i) {
        if (symbol_table[i].kind == -1){
            return -1;
        }
        // Match either vars or consts if kind = 4 or just same kind otherwise
        int right_kind;
        if (kind == 4){
            right_kind = symbol_table[i].kind == 1 || symbol_table[i].kind == 2;
        } else {
            right_kind = symbol_table[i].kind == kind;
        }
        // Find match with highest level
        // out of scope variables would be marked so only
        // in scope variables will match
        if (strcmp(symbol_table[i].name, name) == 0 && right_kind) {
            if (symbol_table[i].level > closest_level && !symbol_table[i].mark){
                index = i;
                closest_level = symbol_table[i].level;
            }
        }
    }
    return index;
}

void gen_code(int op, int l, int m) {
    code[code_index].opcode = op;
    code[code_index].l = l;
    code[code_index].m = m;
    code_index++;
}

void printcode() {
    int i;
    printf("Line\tOP Code\tOP Name\tL\tM\n");
    for (i = 0; i < code_index; i++) {
        printf("%d\t", i);
        printf("%d\t", code[i].opcode);
        switch (code[i].opcode) {
            case 1:
                printf("LIT\t");
                break;
            case 2:
                switch (code[i].m) {
                    case 0:
                        printf("RTN\t");
                        break;
                    case 1:
                        printf("NEG\t");
                        break;
                    case 2:
                        printf("ADD\t");
                        break;
                    case 3:
                        printf("SUB\t");
                        break;
                    case 4:
                        printf("MUL\t");
                        break;
                    case 5:
                        printf("DIV\t");
                        break;
                    case 6:
                        printf("ODD\t");
                        break;
                    case 7:
                        printf("MOD\t");
                        break;
                    case 8:
                        printf("EQL\t");
                        break;
                    case 9:
                        printf("NEQ\t");
                        break;
                    case 10:
                        printf("LSS\t");
                        break;
                    case 11:
                        printf("LEQ\t");
                        break;
                    case 12:
                        printf("GTR\t");
                        break;
                    case 13:
                        printf("GEQ\t");
                        break;
                    default:
                        printf("err\t");
                        break;
                }
                break;
            case 3:
                printf("LOD\t");
                break;
            case 4:
                printf("STO\t");
                break;
            case 5:
                printf("CAL\t");
                break;
            case 6:
                printf("INC\t");
                break;
            case 7:
                printf("JMP\t");
                break;
            case 8:
                printf("JPC\t");
                break;
            case 9:
                switch (code[i].m) {
                    case 1:
                        printf("WRT\t");
                        break;
                    case 2:
                        printf("RED\t");
                        break;
                    case 3:
                        printf("HAL\t");
                        break;
                    default:
                        printf("err\t");
                        break;
                }
                break;
            default:
                printf("err\t");
                break;
        }
        printf("%d\t%d\n", code[i].l, code[i].m);
    }
}