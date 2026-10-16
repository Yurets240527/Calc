#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "Stack.cpp"

#define CODE_CAPACITY 10000
#define MAX_REGISTERS 5
#define MAX_RAM 100
#define SCREEN_WIDE 10

#define RED "\x1b[31m"
#define RESETC "\x1b[0m"
#define GREEN "\x1b[32m"

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

struct File
{
    char *filename;
    int file_descriptor;
    struct stat st;
};

struct Code
{
    int *buffer;
    size_t buffer_size;
    int ip;

    int *registers;

    stack ret;

    int* RAM;
};

int ReadFile(struct File *file, int buffer[]);

void CodeDump(Code* Calc);

int Add (stack* stk, Code* Calc);
int Sub (stack* stk, Code* Calc);
int Mult(stack* stk, Code* Calc);
int Div (stack* stk, Code* Calc);

int Jump_A(stack* stk, Code* Calc);
int In(stack* stk, Code* Calc);

int PrintRAM(int* RAM);

int RunCode(Code* Calc, stack* stk);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("too few arguments\n");
        return -1;
    }

    int buffer[CODE_CAPACITY] = {};
    int registers[MAX_REGISTERS] = {};
    int RAM[MAX_RAM] = {};

    Code Calc = {};
    Calc.buffer = buffer;
    Calc.registers = registers;
    Calc.RAM = RAM;
    InitStack(&Calc.ret, 10);

    File input_file = {};
    input_file.filename = argv[1];

    Calc.buffer_size = ReadFile(&input_file, Calc.buffer);

    stack stk = {};
    InitStack(&stk, 10);

    RunCode(&Calc, &stk);

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

int Add(stack* stk, Code* Calc)
{
    int a = StackPop(stk);
    int b = StackPop(stk);

    StackPush(stk, a + b);

    Calc->ip++;
}

int Sub(stack* stk, Code* Calc)
{
    int a = StackPop(stk);
    int b = StackPop(stk);

    StackPush(stk, b - a);

    Calc->ip++;

}

int Mult(stack* stk, Code* Calc)
{
    int a = StackPop(stk);
    int b = StackPop(stk);

    StackPush(stk, a * b);

    Calc->ip++;
}

int Div(stack* stk, Code* Calc)
{
    int a = StackPop(stk);
    int b = StackPop(stk);
    if (a == 0)
    {
        printf("division by zero\n");
        abort();
    }

    StackPush(stk, b / a);

    Calc->ip++;
}

int Sqrt(stack* stk, Code* Calc)
{
    double a = (double) StackPop(stk);

    StackPush(stk, (int) sqrt(a));

    Calc->ip++;
}

int Jump_A(stack* stk, Code* Calc)
{
    int a = StackPop(stk);
    int b = StackPop(stk);

    if (a > b) Calc->ip = Calc->buffer[Calc->ip + 1];
    else Calc->ip += 2;
}

int Jump_E(stack* stk, Code* Calc)
{
    int a = StackPop(stk);
    int b = StackPop(stk);

    if (a == b) Calc->ip = Calc->buffer[Calc->ip + 1];
    else Calc->ip += 2;
}

int Call(Code* Calc)
{
    StackPush(&Calc->ret, Calc->ip+2);
    Calc->ip = Calc->buffer[Calc->ip + 1];
}

int In(stack* stk, Code* Calc)
{
    int val = 0;
    if (!scanf("%d", &val)) abort();
    StackPush(stk, val);
    Calc->ip ++;
    return 0;
}

int RunCode(Code* Calc, stack* stk)
{
    while(Calc->ip < Calc->buffer_size)
    {
        //CodeDump(Calc);
        //STACK_DUMP(*stk);
        //getchar();

        switch (Calc->buffer[Calc->ip])
        {
            case PUSH: 
                StackPush(stk, Calc->buffer[++Calc->ip]);
                Calc->ip++;
                break;
            case PUSHr:
                StackPush(stk, Calc->registers[Calc->buffer[++Calc->ip]]);
                Calc->ip++;
                break;
            case PUSHM: 
                StackPush(stk, Calc->RAM[Calc->buffer[++Calc->ip]]);
                Calc->ip++;
                break;
            case PUSHMr:
                StackPush(stk, Calc->RAM[Calc->registers[Calc->buffer[++Calc->ip]]]);
                Calc->ip++;
                break;

            case IN:
                In(stk, Calc);
                break;
            
            case POPr:
                Calc->registers[Calc->buffer[++Calc->ip]] = StackPop(stk);
                Calc->ip++;
                break;
            case POPM:
                Calc->RAM[Calc->buffer[++Calc->ip]] = StackPop(stk);
                Calc->ip++;
                break;
            case POPMr:
                Calc->RAM[Calc->registers[Calc->buffer[++Calc->ip]]] = StackPop(stk);
                Calc->ip++;
                break;

            case ADD:
                Add (stk, Calc);
                break;
            case SUB:
                Sub (stk, Calc);
                break;
            case MULT:
                Mult(stk, Calc);
                break;
            case DIVari:
                Div (stk, Calc);
                break;
            case SQRT:
                Sqrt(stk, Calc);
                break;

            case JMP:
                Calc->ip = Calc->buffer[Calc->ip + 1];
                break;
            case JA:
                Jump_A(stk, Calc);
                break;
            case JE:
                Jump_E(stk, Calc);
                break;

            case CALL:
                Call(Calc);
                break;
            case RET:
                //STACK_DUMP(Calc->ret);
                Calc->ip = StackPop(&Calc->ret);
                break;

            case OUT:
                printf(GREEN "OUT OUTPUT: <%d>\n" RESETC, StackPop(stk));
                Calc->ip++;
                break;
            case RAM:
                PrintRAM(Calc->RAM);
                Calc->ip++;
                break;

            case HLT:
                return 0;
            default: break;
        }
        
    } 
}

int PrintRAM(int* RAM)
{
    int i = 0;
    while (i < MAX_RAM)
    {
        if (RAM[i]) putchar('*');
        else putchar('.');

        if ((i + 1) % SCREEN_WIDE == 0) putchar('\n');

        i++;
    }
}