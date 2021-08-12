/*
  P-machine (PM/0)
  Author: Ryan Doherty

  An implementation of a virtual PM/0 CPU, a stack machine that runs
  instructions given in an input file from a command line argument.

  It has 4 registers, a program counter (pc), stack pointer (sp),
  base pointer (bp), and a instruction register (ir).

  Instruction Set Architecture (ISA):
  1: LIT 0, M: stores M at the top of the stack
  2: OPR 0, #: Executes various math and function operations
     #0 =  RTN: Return from subroutine
     #1 =  NEG: Negate number on top of stack
     #2 =  ADD: Adds sp and sp - 1 and store result at sp
     #3 =  SUB: Same as above but subtract
     #4 =  MUL: Multiply
     #5 =  DIV: Divide
     #6 =  ODD: Returns 0 if odd 1 otherwise (true and false)
     #7 =  MOD: sp - 1 modulus sp
     #8 =  EQU: 0 if equal
     #9 =  NEQ: 0 if not equal
     #10 = LSS: 0 if sp - 1 is less than sp
     #11 = LEQ: 0 if less than or equal to
     #12 = GTR: 0 if greater
     #13 = GEQ: 0 if greater than or equal to
  3: LOD L, M: Loads value M from level L to sp (top of stack)
  4: STO L, M: Stores value at sp at M in level L
  5: CAL L, M: Calls subroutine from L starting at program counter M
  6: INC 0, M: Increments sp by M (allocates memory on the stack)
  7: JMP 0, M: Jump to instruction at program counter M
  8: JPC 0, M: Jump if sp == 1
  9: SYS 0, #: Interacts with system.
     #1 = print sp to stdout
     #2 = input to sp from stdin
     #3 = halt machine
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_PAS_LENGTH 500

int base(int L);

int pc = 0;
int sp;
int bp;
int *pas;
int *ir;

int main(int argc, char **args) {
    // Allocate machine state
    pas = calloc(sizeof(int), MAX_PAS_LENGTH);
    ir = malloc(sizeof(int) * 3);

    // Input file into process address space and initialize bp and sp
    if (argc < 2){
        printf("No input file given\n");
        exit(1);
    }

    FILE *inputFile = fopen(args[1], "r");
    if (inputFile == NULL) {
        printf("Can't open file\n");
        exit(1);
    }

    int instructionCount = 0;
    char *currentLine = malloc(11);
    while (fgets(currentLine, 10, inputFile) != NULL) {
        // Chop off newlines
        currentLine[strcspn(currentLine, "\n\r")] = 0;

        // Each instruction is assumed to be in the format "opCode level M"
        char *separatedCurrentLine = strtok(currentLine, " ");
        int op = atoi(separatedCurrentLine);
        int l = atoi(strtok(NULL, " "));
        int m = atoi(strtok(NULL, " "));

        // Initialize text section (code section) of our machine
        pas[instructionCount] = op;
        pas[instructionCount + 1] = l;
        pas[instructionCount + 2] = m;
        instructionCount += 3;

        // Stack pointer is set to first index after text section
        sp = instructionCount - 1;
    }
    bp = sp + 1;
    int halt = 1;

    printf("\t\t\t\tPC\tBP\tSP\tstack\n");
    printf("Initial values:\t%d\t%d\t%d\n", pc, bp, sp);

    // Fetch Execute cycle
    while (pc < bp && halt) {
        // Fetch
        int initialPc = pc;
        ir[0] = pas[pc];
        ir[1] = pas[pc + 1];
        ir[2] = pas[pc + 2];
        pc = pc + 3;

        // Execute
        char* decodedInstruction = "";
        switch (ir[0]) {
            // LIT 0, M: Stores integer M on the top of the stack
            case 1:
                decodedInstruction = "LIT";
                sp = sp + 1;
                pas[sp] = ir[2];
                break;
            // OPR 0, #: Executes various math and function operations
            case 2:
                switch (ir[2]) {
                    case 0: // ReTurN
                        decodedInstruction = "RTN";
                        sp = bp - 1;
                        bp = pas[sp + 2];
                        pc = pas[sp + 3];
                        break;
                    case 1: // NEGative
                        decodedInstruction = "NEG";
                        pas[sp] = -1 * pas[sp];
                        break;
                    case 2: // ADD
                        decodedInstruction = "ADD";
                        sp--;
                        pas[sp] = pas[sp] + pas[sp + 1];
                        break;
                    case 3: // SUBtract
                        decodedInstruction = "SUB";
                        sp--;
                        pas[sp] = pas[sp] - pas[sp + 1];
                        break;
                    case 4: // MULtiply
                        decodedInstruction = "MUL";
                        sp--;
                        pas[sp] = pas[sp] * pas[sp + 1];
                        break;
                    case 5: // DIVide
                        decodedInstruction = "DIV";
                        sp--;
                        pas[sp] = pas[sp] / pas[sp + 1];
                        break;
                    case 6: // ODD
                        decodedInstruction = "ODD";
                        pas[sp] = !(pas[sp] % 2);
                        break;
                    case 7: // MODulous
                        decodedInstruction = "MOD";
                        sp--;
                        pas[sp] = pas[sp] % pas[sp + 1];
                        break;
                    case 8: // EQuaL
                        decodedInstruction = "EQL";
                        sp--;
                        pas[sp] = !(pas[sp] == pas[sp + 1]);
                        break;
                    case 9: // NotEQual
                        decodedInstruction = "NEQ";
                        sp--;
                        pas[sp] = !(pas[sp] != pas[sp + 1]);
                        break;
                    case 10: // LeSS
                        decodedInstruction = "LSS";
                        sp--;
                        pas[sp] = !(pas[sp] < pas[sp + 1]);
                        break;
                    case 11: // Less or EQual to
                        decodedInstruction = "LEQ";
                        sp--;
                        pas[sp] = !(pas[sp] <= pas[sp + 1]);
                        break;
                    case 12: // GreaTeR
                        decodedInstruction = "GTR";
                        sp--;
                        pas[sp] = !(pas[sp] > pas[sp + 1]);
                        break;
                    case 13: // Greater or EQual to
                        decodedInstruction = "GEQ";
                        sp--;
                        pas[sp] = !(pas[sp] >= pas[sp + 1]);
                        break;
                }
                break;
            //LOD L, M: Loads M from level L into sp + 1
            case 3:
                decodedInstruction = "LOD";
                sp++;
                pas[sp] = pas[base(ir[1]) + ir[2]];
                break;
            //STO L, M: Stores sp at M in level L
            case 4:
                decodedInstruction = "STO";
                pas[base(ir[1]) + ir[2]] = pas[sp];
                sp--;
                break;
            //CAL L, M: Calls a subroutine from level L starting at instruction M
            case 5:
                decodedInstruction = "CAL";
                pas[sp + 1] = base(ir[1]); // static link
                pas[sp + 2] = bp; // dynamic link
                pas[sp + 3] = pc; // return address
                bp = sp + 1; // move to new activation record
                pc = ir[2]; // jump to subroutine's instructions
                break;
            //INC 0, M: increments sp by M
            case 6:
                decodedInstruction = "INC";
                sp = sp + ir[2];
                break;
            // JMP 0, M: jumps to M
            case 7:
                decodedInstruction = "JMP";
                pc = ir[2];
                break;
            // JPC 0, M: conditionally jumps to M
            case 8:
                decodedInstruction = "JPC";
                if(pas[sp] == 1){
                    pc = ir[2];
                }
                sp--;
                break;
            // SYS 0, #: Interactions with the system
            case 9:
                decodedInstruction = "SYS";
                switch (ir[2]) {
                    // Outputs top of stack to stdout
                    case 1:
                        printf("\nOutput result is: %d", pas[sp]);
                        sp--;
                        break;
                    // Inputs an integer from stdin to the top of the stack
                    case 2:
                        sp++;
                        printf("\nPlease Enter an Integer: ");
                        scanf("%d", &pas[sp]);
                        break;
                    // Halts program
                    case 3:
                        halt = 0;
                        break;
                }
                break;
        }
        printf("\n%d\t%s   %d\t%d\t%d\t%d\t%d\t", initialPc, decodedInstruction, ir[1], ir[2], pc, bp, sp);

        // Find deepest Activation Record to print in proper format
        int levelCount = 0;
        int numStaticLinks = 0;

        // Some ARs have the same static link (as in fact.txt) so we count by dynamic links
        while (base(numStaticLinks) != 0){
            int index = pas[base(numStaticLinks) + 1];
            while (index != pas[base(numStaticLinks)]){
                index = pas[index + 1];
                levelCount++;
            }
            numStaticLinks++;
            levelCount++;
        }

        for(int i = levelCount; i > 0; i--){
            // Find the sp for each AR. The sp for the current AR is the bp of the AR above it
            int topOfCurrentAR = sp;
            int bottom = bp;
            if(i != 1){
                // Some ARs have the same static link (as in fact.txt) so we count by dynamic links
                int count = 0;
                int arIndex = bp;
                int lastArIndex = sp;
                while (count != i - 1){
                    lastArIndex = arIndex;
                    arIndex = pas[arIndex + 1];
                    count++;
                }
                topOfCurrentAR = lastArIndex - 1;
                bottom = arIndex;
            }
            for(int j = bottom; j < topOfCurrentAR + 1; j++){
                printf("%d ", pas[j]);
            }
            if (i != 1){
                // Don't print extra | if CAL
                if(!(ir[0] == 5 && i == 2)){
                    printf("| ");
                }
            }
        }
    }
    printf("\n");
    // Being good and freeing my memory
    fclose(inputFile);
    free(currentLine);
    free(pas);
}

int base(int L)
{
    int arb = bp; // arb = activation record base
    while (L > 0) //find base L levels down
    {
        arb = pas[arb];
        L--;
    }
    return arb;
}