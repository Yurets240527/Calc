//#include <TXLib.h>
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
#define MAX_RAM 100
#define SCREEN_WIDE 10

#define RED "\x1b[31m"
#define RESETC "\x1b[0m"
#define GREEN "\x1b[32m"


struct File
{
    char *filename;
    int file_descriptor;
    struct stat st;
};

struct Code
{
    stack stk;

    int *buffer;
    size_t buffer_size;
    int ip;

    int *registers;

    stack ret;

    int* RAM;
};

int ReadFile(struct File *file, int buffer[]);

void CodeDump(Code* Calc);

int Add (Code* Calc);
int Sub (Code* Calc);
int Mult(Code* Calc);
int Div (Code* Calc);
int Sqrt(Code* Calc);

int Push(Code* Calc);
int Pushr(Code* Calc);
int PushM(Code* Calc);
int PushMr(Code* Calc);

int Popr(Code* Calc);
int PopM(Code* Calc);
int PopMr(Code* Calc);

int Jump(Code* Calc);
int Jump_E(Code* Calc);
int Jump_A(Code* Calc);

int In(Code* Calc);
int Outp(Code* Calc);

int Call(Code* Calc);
int Ret(Code* Calc);

int PrintRAM(Code* Calc);

int txPrintRAM(Code* Calc);

typedef int (*Func)(Code*);

Func Funcs[NUM_OF_COMMANDS] =
{
    NULL,    // [0]  HLT
    Push,    // [1]  PUSH
    Add,     // [2]  ADD
    Sub,     // [3]  SUB
    Div,     // [4]  DIVari
    Outp,    // [5]  OUTP
    Mult,    // [6]  MULT
    Pushr,   // [7]  PUSHr
    Popr,    // [8]  POPr
    Jump,    // [9]  JMP
    Jump_A,  // [10] JA
    In,      // [11] INP
    Call,    // [12] CALL
    Ret,     // [13] RET
    Sqrt,    // [14] SQRT
    Jump_E,  // [15] JE
    PushM,   // [16] PUSHM
    PushMr,  // [17] PUSHMr
    PopM,    // [18] POPM
    PopMr,   // [19] POPMr
    PrintRAM // [20] RAM
};

int RunCode(Code* Calc);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("too few arguments\n");
        return 1;
    }

    int buffer[CODE_CAPACITY] = {};
    int registers[MAX_REGISTERS] = {};
    int RAM[MAX_RAM] = {};

    Code Calc = {};
    Calc.buffer = buffer;
    Calc.registers = registers;
    Calc.RAM = RAM;
    InitStack(&Calc.ret, 10);
    InitStack(&Calc.stk, 10);

    File input_file = {};
    input_file.filename = argv[1];

    Calc.buffer_size = ReadFile(&input_file, Calc.buffer);

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

    int i = 0;
    while (i < Calc->buffer_size)
    {
        if (i == Calc->ip) printf(RED);

        if (Calc->buffer[i] == PUSH || Calc->buffer[i] == PUSHr
         || Calc->buffer[i] == POPr || Calc->buffer[i] == JA || Calc->buffer[i] == CALL)
        {
            printf("%d %d ", Calc->buffer[i], Calc->buffer[i+1]);
            i++;
        }
        else printf("%d ", Calc->buffer[i]);

        printf(RESETC);
        i++;
    }
    printf(" >\n-------------------------\n");

    /*int c = 0;
    while ((c = getchar()) != '\n');*/
}

int Add(Code* Calc)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    StackPush(&Calc->stk, a + b);

    Calc->ip++;

    return 0;
}

int Sub(Code* Calc)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    StackPush(&Calc->stk, b - a);

    Calc->ip++;

    return 0;
}

int Mult(Code* Calc)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    StackPush(&Calc->stk, a * b);

    Calc->ip++;

    return 0;
}

int Div(Code* Calc)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);
    if (a == 0)
    {
        printf("division by zero\n");
        abort();
    }

    StackPush(&Calc->stk, b / a);

    Calc->ip++;

    return 0;
}

int Sqrt(Code* Calc)
{
    double a = (double) StackPop(&Calc->stk);

    StackPush(&Calc->stk, (int) sqrt(a));

    Calc->ip++;

    return 0;
}

int Jump_A(Code* Calc)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    if (a > b) Calc->ip = Calc->buffer[Calc->ip + 1];
    else Calc->ip += 2;

    return 0;
}

int Jump_E(Code* Calc)
{
    int a = StackPop(&Calc->stk);
    int b = StackPop(&Calc->stk);

    if (a == b) Calc->ip = Calc->buffer[Calc->ip + 1];
    else Calc->ip += 2;

    return 0;
}

int Call(Code* Calc)
{
    StackPush(&Calc->ret, Calc->ip+2);
    Calc->ip = Calc->buffer[Calc->ip + 1];

    return 0;
}

int In(Code* Calc)
{
    int val = 0;
    if (!scanf("%d", &val)) abort();
    StackPush(&Calc->stk, val);
    Calc->ip ++;
    return 0;
}

int RunCode(Code* Calc)
{
    while (Calc->ip < Calc->buffer_size)
    {
        int opcode = Calc->buffer[Calc->ip];

        if (opcode == HLT)
            return 0;

        if (opcode < 0 || opcode >= NUM_OF_COMMANDS || Funcs[opcode] == NULL)
        {
            printf("unknown opcode %d at ip=%d\n", opcode, Calc->ip);
            return 1;
        }

        Funcs[opcode](Calc);
    }
    return 0;
}

int PrintRAM(Code* Calc)
{
    int i = 0;
    while (i < MAX_RAM)
    {
        if (Calc->RAM[i]) printf(GREEN "*" RESETC);
        else putchar('.');
        putchar(' ');
        
        if ((i + 1) % SCREEN_WIDE == 0) putchar('\n');

        i++;
    }

    Calc->ip++;
}

/*int txPrintRAM(Code* Calc)
{
    txCreateWindow(800, 600);
    return 0;

}*/

int Push(Code* Calc)
{
    StackPush(&Calc->stk, Calc->buffer[++Calc->ip]);
    Calc->ip++;

    return 0;
}

int Pushr(Code* Calc)
{
    StackPush(&Calc->stk, Calc->registers[Calc->buffer[++Calc->ip]]);
    Calc->ip++;

    return 0;
}

int PushM(Code* Calc)
{
    StackPush(&Calc->stk, Calc->RAM[Calc->buffer[++Calc->ip]]);
    Calc->ip++;

    return 0;
}

int PushMr(Code* Calc)
{
    StackPush(&Calc->stk, Calc->RAM[Calc->registers[Calc->buffer[++Calc->ip]]]);
    Calc->ip++;

    return 0;
}

int Popr(Code* Calc)
{
    Calc->registers[Calc->buffer[++Calc->ip]] = StackPop(&Calc->stk);
    Calc->ip++;

    return 0;
}

int PopM(Code* Calc)
{
    Calc->RAM[Calc->buffer[++Calc->ip]] = StackPop(&Calc->stk);
    Calc->ip++;

    return 0;
}

int PopMr(Code* Calc)
{
    Calc->RAM[Calc->registers[Calc->buffer[++Calc->ip]]] = StackPop(&Calc->stk);
    Calc->ip++;

    return 0;
}

int Jump(Code* Calc)
{
    Calc->ip = Calc->buffer[Calc->ip + 1];

    return 0;
}

int Ret(Code* Calc)
{
    Calc->ip = StackPop(&Calc->ret);

    return 0;
}

int Outp(Code* Calc)
{
    printf(GREEN "OUT OUTPUT: <%d>\n" RESETC, StackPop(&Calc->stk));
    Calc->ip++;

    return 0;
}