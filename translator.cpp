#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAN_LEN 1000
#define MAXLINES 1000
#define MAX_LABELS 20
#define SyntaxError -1
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
    JA     = 10,
    IN     = 11,
    CALL   = 12,
    RET    = 13,
    SQRT   = 14,
    JE     = 15,
    PUSHM  = 16,
    PUSHMr = 17,
    POPM   = 18,
    POPMr  = 19,
    RAM    = 20,
};

enum Registers
{
    NO_REGISTERS = -1,
    AX = 1,
    BX = 2,
    CX = 3,
    DX = 4,
};

struct ReadData
{
    int* Labels;

    int* buffer;
    int code_len;
    
    bool NeedSecondTurn;

};


int FillBuffer(char lines[][MAX_COMMAN_LEN], ReadData* Calc);

int ParceRegister(char *line);

int ParceLabel(char* line);

int ParceJump(char* line, ReadData* Calc, char* command);

int ParceCall(char* line, ReadData* Calc);

int ParcePush(char* line, ReadData* Calc, char* command);

int ParcePop(char* line, ReadData* Calc, char* command);

int strip_comment(char *line);

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

    struct ReadData Calc = {};

    char lines[MAXLINES][MAX_COMMAN_LEN] = {};

    int Labels[MAX_LABELS] ={};

    Calc.Labels = Labels;

    memset(Calc.Labels, -1, sizeof(Labels));

    int i = 0;
    while (fgets(lines[i], MAX_COMMAN_LEN, fpin)) i++;

    int buffer[CODE_CAPACITY] = {};
    Calc.buffer = buffer;

    Calc.NeedSecondTurn = false;
    

    FillBuffer(lines, &Calc);

    if (Calc.NeedSecondTurn) FillBuffer(lines, &Calc);

    for(i = 0; i<Calc.code_len; i++) printf("%d ", Calc.buffer[i]);
    printf("\nLABELS: ");
    
    for(i = 0; i<MAX_LABELS; i++) printf("%d ", Labels[i]);

    fwrite(Calc.buffer, sizeof(int), Calc.code_len, fpout);

    fclose(fpin);
    fclose(fpout);

    return 0;
}

int FillBuffer(char lines[][MAX_COMMAN_LEN], ReadData* Calc)
{
    char command[MAX_COMMAN_LEN] = {};
    int label = 0;
    int value = 0;
    int register_code = 0;

    Calc->code_len = 0;

    int i = 0;
    while (i < MAXLINES)
    {
        strip_comment(lines[i]);

        if (sscanf(lines[i], "%s", command) != 1) { i++; continue; }

        else if (ParcePush(lines[i], Calc, command) > 0) {i++; continue;}

        else if (ParcePop (lines[i], Calc, command) > 0) {i++; continue;}

        else if (ParceJump(lines[i], Calc, command) > 0) {i++; continue;}

        else if (strcmp(command, "CALL") == 0) ParceCall(lines[i], Calc);

        else if (strcmp(command, "RET" ) == 0) Calc->buffer[Calc->code_len++] = RET;

        else if (strcmp(command, "IN" ) == 0) Calc->buffer[Calc->code_len++] = IN;

        else if (strcmp(command, "ADD" ) == 0) Calc->buffer[Calc->code_len++] = ADD;
        
        else if (strcmp(command, "SUB" ) == 0) Calc->buffer[Calc->code_len++] = SUB;

        else if (strcmp(command, "MULT" ) == 0) Calc->buffer[Calc->code_len++] = MULT;

        else if (strcmp(command, "DIV" ) == 0) Calc->buffer[Calc->code_len++] = DIVari;

        else if (strcmp(command, "SQRT" ) == 0) Calc->buffer[Calc->code_len++] = SQRT;

        else if (strcmp(command, "OUT" ) == 0) Calc->buffer[Calc->code_len++] = OUT;

        else if (strcmp(command, "RAM" ) == 0) Calc->buffer[Calc->code_len++] = RAM;

        else if (strcmp(command, "HLT" ) == 0) Calc->buffer[Calc->code_len++] = HLT;

        else if ((label = ParceLabel(lines[i])) >= 0) Calc->Labels[label] = Calc->code_len;

        else 
        {
            printf("Error: invalid syntax: <%s>\n", command);
            return SyntaxError;
        }

        i++;

    }

    return Calc->code_len;
}

int ParceRegister(char* line)
{
    char reg_code = 0;
    if (sscanf(line, "%cX%*s", &reg_code) == 1) return reg_code - 64;

    return NO_REGISTERS;
}

int ParceLabel(char* line)
{
    int label = -1;

    sscanf(line, ":%d", &label);

    if (label > MAX_LABELS) return -1;

    return label;
}

int ParceJump(char* line, ReadData* Calc, char* command)
{
    int label = 0;
    int value = 0;

    int opcode = 0;
    if      (strcmp(command, "JMP") == 0) opcode = JMP;
    else if (strcmp(command, "JA") == 0)  opcode = JA;
    else if (strcmp(command, "JE") == 0)  opcode = JE;
    else return -1;

    if (sscanf(line, "%*s %d", &value) == 1)
    {
        Calc->buffer[Calc->code_len++] = opcode;
        Calc->buffer[Calc->code_len++] = value;
    }
        
    else if (sscanf(line, "%*s :%d", &label) == 1)
    {
        Calc->buffer[Calc->code_len++] = opcode;
        Calc->buffer[Calc->code_len++] = Calc->Labels[label];

        if (Calc->Labels[label] < 0) Calc->NeedSecondTurn = true;
    }
    else
    {
        printf("Error: %s argument is wrong or doesn't exist\n", command);
        return SyntaxError;
    }

    return opcode;

}

int ParceCall(char* line, ReadData* Calc)
{
    int label = 0;
  
    if (sscanf(line, "%*s :%d%*s", &label) == 1)
    {
        Calc->buffer[Calc->code_len++] = CALL;
        Calc->buffer[Calc->code_len++] = Calc->Labels[label];

        if (Calc->Labels[label] < 0) Calc->NeedSecondTurn = true;
    }
    else
    {
        printf("Error: CALL argument is wrong or doesn't exist\n");
        return SyntaxError;
    }

    return 0;

}

int strip_comment(char *line)
{
    for (int i = 0; line[i]; i++)
    {
        if (line[i] == ';' || line[i] == '#')
        {
            line[i] = '\0';
            return 0;
        }
    }
}

int ParcePush(char* line, ReadData* Calc, char* command)
{
    int value = 0;
    int reg = 0;
    if (strcmp(command, "PUSH") == 0)
    {
        if (sscanf(line, "PUSH %d%*s", &value))
        {
            Calc->buffer[Calc->code_len++] = PUSH;
            Calc->buffer[Calc->code_len++] = value;
        }
        else if ((reg = ParceRegister(line+5)) >= 0)
        {
            Calc->buffer[Calc->code_len++] = PUSHr;
            Calc->buffer[Calc->code_len++] = reg;
        }
        else
        {
            printf("Error: PUSH without argument\n");
            return SyntaxError;
        }        
    }

    else if (strcmp(command, "PUSHM") == 0)
    {
        if (sscanf(line, "PUSHM [%d]%*s", &value))
        {
            Calc->buffer[Calc->code_len++] = PUSHM;
            Calc->buffer[Calc->code_len++] = value;
        }
        else if ((reg = ParceRegister(line+7)) >= 0)
        {
            Calc->buffer[Calc->code_len++] = PUSHMr;
            Calc->buffer[Calc->code_len++] = reg;
        }
        else
        {
            printf("Error: PUSHM argument is spoiled or doesn't exist\n");
            return SyntaxError;
        }
               
    }
    
    else return -1;

    return 1;
}

int ParcePop(char* line, ReadData* Calc, char* command)
{
    int value = 0;
    int reg = 0;

    if (strcmp(command, "POP") == 0)
    {
        if ((reg = ParceRegister(line + 4)) >= 0)
        {
            Calc->buffer[Calc->code_len++] = POPr;
            Calc->buffer[Calc->code_len++] = reg;
        }
        else
        {
            printf("Error: POP argument is wrong or doesn't exist\n");
            return SyntaxError;
        }
               
    }

    else if (strcmp(command, "POPM") == 0)
    {
        if (sscanf(line, "POPM [%d]%*s", &value))
        {
            Calc->buffer[Calc->code_len++] = POPM;
            Calc->buffer[Calc->code_len++] = value;
        }
        else if ((reg = ParceRegister(line + 6)) >= 0)
        {
            Calc->buffer[Calc->code_len++] = POPMr;
            Calc->buffer[Calc->code_len++] = reg;
        }
        else
        {
            printf("Error: POPM argument is spoiled or doesn't exist\n");
            return SyntaxError;
        }
               
    }

    else return -1;

    return 1;
}