#include <TXLib.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../include/Stack.cpp"
#include "../include/Commands.h"

#define CODE_CAPACITY 10000
#define MAX_REGISTERS 5
#define SCREEN_WIDE 100
#define SCREEN_HIGH 100
#define MAX_RAM (SCREEN_WIDE * SCREEN_HIGH)

#define RED "\x1b[31m"
#define RESETC "\x1b[0m"
#define GREEN "\x1b[32m"

struct File
{
    char *filename;
    int file_descriptor;
    struct stat st;
};

struct Buffer
{
    int* data;
    size_t size;
};

struct Code
{
    stack stk;

    Buffer buffer;
    int ip;

    int *registers;

    stack ret;

    int* RAM;
};

int ReadFile(struct File *file, int buffer[]);

void CodeDump(Code* Calc);

int BinFuncs(Code* Calc, int opcode);

int Pushing(Code* Calc, int opcode);

int Poping(Code* Calc, int opcode);

int Jumping(Code* Calc, int opcode);

int Inoutput(Code* Calc, int opcode);

int NoArgFuncs(Code* Calc, int opcode);

int RunCode(Code* Calc);

int txPrintRAM(Code* Calc);

void InitWindow(const int CELL, const int MARGIN);

int SPUctor(Code* Calc);


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("too few arguments\n");
        return 1;
    }

    InitWindow(10, 0);
    
    Code Calc = {};

    SPUctor(&Calc);

    File input_file = {};
    input_file.filename = argv[1];

    Calc.buffer.size = ReadFile(&input_file, Calc.buffer.data);

    RunCode(&Calc);

    return 0;

}

int ReadFile(struct File *file, int buffer[])
{
    assert(file);
    assert(file->filename);

    file->file_descriptor = open(file->filename, O_RDONLY);

    stat(file->filename, &file->st);

    size_t max_size = file->st.st_size;

    if (file->file_descriptor == -1)
    {
        printf("Cant open the file\n");
        return -1;
    }
    size_t real_buffer_size = read(file->file_descriptor, buffer, max_size) / sizeof(int);

    close(file->file_descriptor);

    return real_buffer_size;
}

void CodeDump(Code* Calc)
{
    printf("-------CODE DUMP---------\n< ");

    unsigned int i = 0;
    while (i < Calc->buffer.size)
    {
        if ((int) i == Calc->ip) printf(RED);

        if (Calc->buffer.data[i] == PUSH || Calc->buffer.data[i] == PUSHr
         || Calc->buffer.data[i] == POPr || Calc->buffer.data[i] == JA || Calc->buffer.data[i] == CALL)
        {
            printf("%d %d ", Calc->buffer.data[i], Calc->buffer.data[i+1]);
            i++;
        }
        else printf("%d ", Calc->buffer.data[i]);

        printf(RESETC);
        i++;
    }
    printf(" >\n-------------------------\n");

    /*int c = 0;
    while ((c = getchar()) != '\n');*/
}


int RunCode(Code* Calc)
{
    while (Calc->ip < (int) Calc->buffer.size)
    {
        int opcode = Calc->buffer.data[Calc->ip];

        if (opcode == HLT)
            return 0;

        if (opcode < 0 || opcode >= NUM_OF_COMMANDS)
        {
            printf("unknown opcode %d at ip=%d\n", opcode, Calc->ip);
            return 1;
        }

        switch(Commands_Info[opcode].type)
        {
            case BIN_FUNC: {BinFuncs(Calc, opcode);   break;}

            case PUSHING:  {Pushing(Calc, opcode);    break;}

            case POPING:   {Poping(Calc, opcode);     break;}

            case JUMP:     {Jumping(Calc, opcode);    break;}

            case INOUTPUT: {Inoutput(Calc, opcode);   break;}

            case NO_ARG:   {NoArgFuncs(Calc, opcode); break;}

            default: break;
            
        }

    }
    return 0;
}


int BinFuncs(Code* Calc, int opcode)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    switch (opcode)
    {
        case ADD:    {StackPush(&Calc->stk, a + b); break;}

        case SUB:    {StackPush(&Calc->stk, b - a); break;}

        case MULT:   {StackPush(&Calc->stk, a * b); break;}

        case DIVari:
            if (a == 0)
                {
                    printf("division by zero\n");
                    abort();
                }

            StackPush(&Calc->stk, b / a);
            break;
        
        case SQRT:   {StackPush(&Calc->stk, (int) sqrt((double) a)); break;}

        default: break;

    }

    Calc->ip++;

    return 0;

}

int Pushing(Code* Calc, int opcode)
{
    switch(opcode)
    {
        case PUSH:   {StackPush(&Calc->stk, Calc->buffer.data[++Calc->ip]); break;}

        case PUSHr:  {StackPush(&Calc->stk, Calc->registers[Calc->buffer.data[++Calc->ip]]); break;}

        case PUSHM:  {StackPush(&Calc->stk, Calc->RAM[Calc->buffer.data[++Calc->ip]]); break;}

        case PUSHMr: {StackPush(&Calc->stk, Calc->RAM[Calc->registers[Calc->buffer.data[++Calc->ip]]]); break;}

        default: break;

    }

    Calc->ip++;

    return 0;

}

int Poping(Code* Calc, int opcode)
{
    switch(opcode)
    {
        case POPr:  {Calc->registers[Calc->buffer.data[++Calc->ip]] = StackPop(&Calc->stk); break;}

        case POPM:  {Calc->RAM[Calc->buffer.data[++Calc->ip]] = StackPop(&Calc->stk); break;}

        case POPMr: {Calc->RAM[Calc->registers[Calc->buffer.data[++Calc->ip]]] = StackPop(&Calc->stk); break;}

        default: break;
    }
    
    Calc->ip++;

    return 0;
}

int Jumping(Code* Calc, int opcode)
{
    if (opcode == JMP)
    {
        Calc->ip = Calc->buffer.data[Calc->ip + 1];
        return 0;
    }

    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    switch(opcode)
    {
        case JA:
            if (a > b) Calc->ip = Calc->buffer.data[Calc->ip + 1];
            else Calc->ip += 2;
            break;
        case JE:
            if (a == b) Calc->ip = Calc->buffer.data[Calc->ip + 1];
            else Calc->ip += 2;
            break;

        case CALL:
            StackPush(&Calc->ret, Calc->ip+2);
            Calc->ip = Calc->buffer.data[Calc->ip + 1];

        default: break;

    }

    return 0;
    
}

int Inoutput(Code* Calc, int opcode)
{
    switch(opcode)
    {
        case INP:
        {
            int val = 0;
            if (!scanf("%d", &val)) abort();
            StackPush(&Calc->stk, val);
            break;
        }
        case OUTP: {printf(GREEN "OUT OUTPUT: <%d>\n" RESETC, StackPop(&Calc->stk)); break;}

        default: break;

    }
    
    Calc->ip ++;
    return 0;

}

int NoArgFuncs(Code* Calc, int opcode)
{
    switch(opcode)
    {
        case RAM:
        {
            /*int i = 0;
            while (i < MAX_RAM)
            {
                if (Calc->RAM[i]) printf(GREEN "*" RESETC);
                else putchar('.');
                putchar(' ');
                
                if ((i + 1) % SCREEN_WIDE == 0) putchar('\n');

                i++;
            }

            Calc->ip++;*/
            txPrintRAM(Calc);

            break;
        }
        case RET: {Calc->ip = StackPop(&Calc->ret); break;}

        default: break;

    }

    return 0;

}

int txPrintRAM(Code* Calc)
{
    const int CELL   = 10;
    const int MARGIN = 0;

    txBegin();

    txSetFillColor(TX_WHITE);
    txClear();

    for (int i = 0; i < MAX_RAM; i++)
    {
        int x = MARGIN + (i % SCREEN_WIDE) * CELL;
        int y = MARGIN + (i / SCREEN_WIDE) * CELL;

        COLORREF c = Calc->RAM[i] ? TX_GREEN : TX_WHITE;

        txSetFillColor(c);
        txSetColor(c);
        txRectangle(x, y, x + CELL, y + CELL);
    }

    txEnd();

    txSleep(10);
    return 0;
}

void InitWindow(const int CELL, const int MARGIN)
{
    txCreateWindow(MARGIN * 2 + CELL * SCREEN_WIDE,
                   MARGIN * 2 + CELL * SCREEN_HIGH);
}

int SPUctor(Code* Calc)
{
    int *buffer =    (int*) calloc(CODE_CAPACITY, sizeof(int));
    int *registers = (int*) calloc(MAX_REGISTERS, sizeof(int));
    int *RAM =       (int*) calloc(MAX_RAM, sizeof(int));

    if(!buffer || !registers || !RAM) return printf("Calloc\n");

    Calc->buffer.data = buffer;
    Calc->registers = registers;
    Calc->RAM = RAM;
    InitStack(&Calc->ret, 10);
    InitStack(&Calc->stk, 10);

    return 0;
}