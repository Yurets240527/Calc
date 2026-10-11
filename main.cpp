#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "Stack.cpp"

#define CODE_CAPACITY 1024
#define MAX_REGISTERS 5

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
};

int ReadFile(struct File *file, int buffer[]);

void CodeDump(Code* Calc);

int Add(stack* stk);
int Sub(stack* stk);
int Mult(stack* stk);
int Div(stack* stk);

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

    Code Calc = {};
    Calc.buffer = buffer;
    Calc.registers = registers;

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

        if (Calc->buffer[i] == PUSH || Calc->buffer[i] == PUSHr || Calc->buffer[i] == POPr || Calc->buffer[i] == JMP)
        {
            printf("%d %d ", Calc->buffer[i], Calc->buffer[i+1]);
            i++;
        }
        else printf("%d ", Calc->buffer[i]);

        printf(RESETC);
        i++;
    }
    printf("0 >\n-------------------------\n");

    int c = 0;
    while ((c = getchar()) != '\n');
}

int Add(stack* stk)
{
    int a = StackPop(stk);
    int b = StackPop(stk);
    StackPush(stk, a + b);
}

int Sub(stack* stk)
{
    int a = StackPop(stk);
    int b = StackPop(stk);
    StackPush(stk, a - b);
}

int Mult(stack* stk)
{
    int a = StackPop(stk);
    int b = StackPop(stk);
    StackPush(stk, a * b);
}

int Div(stack* stk)
{
    int a = StackPop(stk);
    int b = StackPop(stk);
    if (b == 0)
    {
        printf("division by zero\n");
        abort();
    }
    StackPush(stk, a / b);
}

int RunCode(Code* Calc, stack* stk)
{
    while(Calc->ip < Calc->buffer_size)
    {
        CodeDump(Calc);
        getchar();

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
            case POPr:
                Calc->registers[Calc->buffer[++Calc->ip]] = StackPop(stk);
                Calc->ip++;
                break;
            case ADD:
                Add(stk);
                Calc->ip++;
                break;
            case SUB:
                Sub(stk);
                Calc->ip++;
                break;
            case MULT:
                Mult(stk);
                Calc->ip++;
                break;
            case DIVari:
                Div(stk);
                Calc->ip++;
                break;
            case JMP:
                Calc->ip = Calc->buffer[Calc->ip + 1];
                break;
            case OUT:
                printf(GREEN "OUT OUTPUT: <%d>\n" RESETC, StackPop(stk));
                Calc->ip++;
                break;
            case HLT:
                return 0;
            default: break;
        }
        
        //Calc->ip++;

    } 
}