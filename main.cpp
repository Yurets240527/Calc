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

enum Commands
{
    HLT    = 0,
    PUSH   = 1,
    ADD    = 2,
    SUB    = 3,
    DIVari = 4,
    OUT    = 5,
};

struct File
{
    char *filename;
    int file_descriptor;
    struct stat st;
};

int ReadFile(struct File *file, int buffer[]);


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("too few arguments\n");
        return -1;
    }

    int buffer[CODE_CAPACITY] = {};
    File input_file = {};
    input_file.filename = argv[1];
    int buffer_size = ReadFile(&input_file, buffer);


    stack stk = {};
    InitStack(&stk, 10);


    int ip = 0;
    int a = 0;
    int b = 0;
    while(ip < buffer_size)
    {
        switch (buffer[ip])
        {
            case PUSH: 
                StackPush(&stk, buffer[++ip]);
                break;
            case ADD:
                b = StackPop(&stk);
                a = StackPop(&stk);
                StackPush(&stk, a + b);
                break;
            case SUB:
                b = StackPop(&stk);
                a = StackPop(&stk);
                StackPush(&stk, a - b);
                break;
            case DIVari:
                b = StackPop(&stk);
                a = StackPop(&stk);
                if (b == 0) { printf("division by zero\n"); return 1; }
                StackPush(&stk, a / b);
                break;
            case OUT:
                printf("%d", StackPop(&stk));
                break;
            case HLT:
                return 0;
            default: break;
        }
        ip ++;

    }
    

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
