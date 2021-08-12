#include <stdlib.h>
#include <stdio.h>
#include "compiler.h"

int main(int argc, char **argv) {
    FILE *file;
    char *inputfile;
    char c;
    lexeme *list;
    symbol *table;
    instruction *code;
    int i;

    if (argc < 2) {
        printf("Error : please include the file name");
        return 0;
    }

    file = fopen(argv[1], "r");
    inputfile = malloc(500 * sizeof(char));
    i = 0;

    c = fgetc(file);
    while (1) {
        inputfile[i++] = c;
        c = fgetc(file);
        if (c == EOF)
            break;
    }
    inputfile[i] = '\0';

    list = lexanalyzer(inputfile);
    if (list == NULL) {
        free(inputfile);
        return 0;
    }

    table = parse(list);
    if (table == NULL) {
        free(inputfile);
        free(list);
        return 0;
    }

    code = generate_code(list, table);

    free(list);
    free(inputfile);
    free(table);
    free(code);
    return 0;
}