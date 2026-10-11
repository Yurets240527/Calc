#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAN_LEN 30
#define MAXLINES 1000
#define MAX_LABELS 20
#define SyntaxError 1
#define CODE_CAPACITY 1024

enum Commands
{
    HLT    = 0,
    PUSH   = 1,
    ADD    = 2,
    SUB    = 3,
    DIVari = 4,
    OUT    = 5,
    MULT   = 6,
    PUSHr  = 7,
    POPr   = 8,
    JMP    = 9,
};

enum Registers
{
    NO_REGISTERS = -1,
    AX = 1,
    BX = 2,
    CX = 3,
    DX = 4,
};


int FillBuffer(int *buffer, char lines[][MAX_COMMAN_LEN], int Labels[], bool* NeedSecondTurn);

Registers ParceRegister(char *line);

int ParceLabel(char* line);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("too few arguments\n");
        return -1;
    }

    FILE *fpin =  fopen(argv[1], "r");
    FILE *fpout = fopen(argv[2], "wb");

    if (!fpin || !fpout)
    {
        printf("can't open files\n");
        if (fpin)  fclose(fpin);
        if (fpout) fclose(fpout);
        return -1;
    }

    char lines[MAXLINES][MAX_COMMAN_LEN] = {};

    int Labels[MAX_LABELS] ={};
    memset(Labels, -1, sizeof(Labels));

    int i = 0;
    while (fgets(lines[i], MAX_COMMAN_LEN, fpin)) i++;

    int buffer[CODE_CAPACITY] = {};

    bool NeedSecondTurn = false;

    size_t code_len  =  FillBuffer(buffer, lines, Labels, &NeedSecondTurn);

    if (NeedSecondTurn) FillBuffer(buffer, lines, Labels, &NeedSecondTurn);

    for(i = 0; i<code_len; i++) printf("%d ", buffer[i]);

    fwrite(buffer, sizeof(int), code_len, fpout);

    fclose(fpin);
    fclose(fpout);

    return 0;
}

int FillBuffer(int *buffer, char lines[][MAX_COMMAN_LEN], int Labels[], bool* NeedSecondTurn)
{
    char command[MAX_COMMAN_LEN] = {};
    int label = 0;
    int value = 0;
    int code_len = 0;
    int register_code = 0;

    int i = 0;
    while ((sscanf(lines[i], "%s", command)) == 1)
    {
        if (strcmp(command, "PUSH") == 0)
        {
            if (sscanf(lines[i], "PUSH %d", &value))
            {
                buffer[code_len++] = PUSH;
                buffer[code_len++] = value;
            }
            else if ((register_code = ParceRegister(&lines[i][5])) >= 0)
            {
                buffer[code_len++] = PUSHr;
                buffer[code_len++] = register_code;
            }
            else
            {
                printf("Error: PUSH without argument\n");
                return SyntaxError;
            }
               
        }

        else if (strcmp(command, "POP") == 0)
        {
            if ((register_code = ParceRegister(&lines[i][4])) >= 0)
            {
                buffer[code_len++] = POPr;
                buffer[code_len++] = register_code;
            }
            else
            {
                printf("Error: POP argument is wrong or doesn't exist\n");
                return SyntaxError;
            }
               
        }

        else if (strcmp(command, "JMP") == 0)
        {
            if (sscanf(lines[i], "JMP %d", &value))
            {
                buffer[code_len++] = JMP;
                buffer[code_len++] = value;
            }

            else if (sscanf(lines[i], "JMP :%d", &label))
            {
                buffer[code_len++] = JMP;
                buffer[code_len++] = Labels[label];

                if (Labels[label] < 0) *NeedSecondTurn = true;
            }

            else
            {
                printf("Error: JMP argument is wrong or doesn't exist\n");
                return SyntaxError;
            }
               
        }
        else if (strcmp(command, "ADD" ) == 0) buffer[code_len++] = ADD;
        
        else if (strcmp(command, "SUB" ) == 0) buffer[code_len++] = SUB;

        else if (strcmp(command, "MULT" ) == 0) buffer[code_len++] = MULT;

        else if (strcmp(command, "DIV" ) == 0) buffer[code_len++] = DIVari;

        else if (strcmp(command, "OUT" ) == 0) buffer[code_len++] = OUT;

        else if (strcmp(command, "HLT" ) == 0) buffer[code_len++] = HLT;

        else if ((label = ParceLabel(lines[i])) > 0) Labels[label] = code_len;

        else 
        {
            printf("Error: invalid syntax: <%s>\n", command);
            return SyntaxError;
        }

        i++;

    }

    return code_len;
}

Registers ParceRegister(char* line)
{
    if (strcmp(line, "AX\n") == 0) return AX;
    if (strcmp(line, "BX\n") == 0) return BX;
    if (strcmp(line, "CX\n") == 0) return CX;
    if (strcmp(line, "DX\n") == 0) return DX;

    return NO_REGISTERS;
    
}

int ParceLabel(char* line)
{
    int label = -1;

    sscanf(line, ":%d", &label);

    if (label > MAX_LABELS) return -1;

    return label;
}