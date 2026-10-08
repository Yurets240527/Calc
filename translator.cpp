#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAN_LEN 10
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
};

int FillBuffer(int *buffer, FILE *fpin);

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

    int buffer[CODE_CAPACITY] = {};

    FillBuffer(buffer, fpin);

    fwrite(buffer, sizeof(int), sizeof(buffer)/sizeof(int), fpout);

    fclose(fpin);
    fclose(fpout);

    return 0;
}

int FillBuffer(int *buffer, FILE *fpin)
{
    char command[MAX_COMMAN_LEN] = {};
    int value = 0;
    int code_len = 0;

    while ((fscanf(fpin, "%s", command)) == 1)
    {
        if (strcmp(command, "PUSH") == 0)
        {
            if (!fscanf(fpin, "%d", &value))
            {
                printf("Error: PUSH without argument\n");
                return SyntaxError;
            }
            buffer[code_len++] = PUSH;
            buffer[code_len++] = value;
        }
        else if (strcmp(command, "ADD" ) == 0) buffer[code_len++] = ADD;
        
        else if (strcmp(command, "SUB" ) == 0) buffer[code_len++] = SUB;

        else if (strcmp(command, "DIV" ) == 0) buffer[code_len++] = DIVari;

        else if (strcmp(command, "OUT" ) == 0) buffer[code_len++] = OUT;

        else if (strcmp(command, "HLT" ) == 0) buffer[code_len++] = HLT;

        else 
        {
            printf("Error: invalid syntax: <%s>\n", command);
            return SyntaxError;
        }

    }

    return code_len;
}