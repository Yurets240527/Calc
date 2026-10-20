#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../include/Commands.h"

#define MAX_COMMAN_LEN 1000
#define MAXLINES 1000
#define MAX_LABELS 20
#define SyntaxError -1
#define CODE_CAPACITY 1024

struct File
{
    char *filename;
    int file_descriptor;
    struct stat st;
};


struct ReadData
{
    File fpin;
    FILE* fpout;
    
    int* Labels;

    char* data;

    char** Lines;

    int* buffer;
    int buffer_size;

    int code_len;
    
    bool NeedSecondTurn;

};

int CodeCtor(ReadData* Calc, char* input, char* output);

int ReadFile(struct File *file, char buffer[]);

int CountSymbol(char *buffer, char sym, size_t size);

int GetCode(ReadData *Calc);

int FillLines(char * index[], char buffer[], size_t size);

int FillBuffer(char** lines, ReadData* Calc);

int ParceRegister(char *line);

int ParceLabel(char* line);

int ParceJump(char* line, ReadData* Calc, char* command);

int ParceCall(char* line, ReadData* Calc);

int ParcePush(char* line, ReadData* Calc, char* command);

int ParcePop(char* line, ReadData* Calc, char* command);

int ParceSimpleCommands(ReadData* Calc, char* command);

int strip_comment(char *line);

int CodePrint(ReadData* Calc);


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("too few arguments\n");
        return 1;
    }

    struct ReadData Calc = {};

    CodeCtor(&Calc, argv[1], argv[2]);
    //TODO: readalka from onegin

    GetCode(&Calc);

    FillBuffer(Calc.Lines, &Calc);

    if (Calc.NeedSecondTurn) FillBuffer(Calc.Lines, &Calc);

    CodePrint(&Calc);

    fwrite(Calc.buffer, sizeof(int), Calc.code_len, Calc.fpout);

    return 0;
}

//TODO:
// [
//     {"SUM", SUM},
//     {}
// ]


int FillBuffer(char** lines, ReadData* Calc)
{
    char command[MAX_COMMAN_LEN] = {};
    int label = 0;
    int value = 0;
    int register_code = 0;

    Calc->code_len = 0;

    int i = 0;
    while (lines[i] != NULL)
    {
        strip_comment(lines[i]);

        if (sscanf(lines[i], "%s", command) != 1) { i++; continue; }

        else if (ParcePush(lines[i], Calc, command) > 0) {i++; continue;}

        else if (ParcePop (lines[i], Calc, command) > 0) {i++; continue;}

        else if (ParceJump(lines[i], Calc, command) > 0) {i++; continue;}

        else if (strcmp(command, "CALL") == 0) ParceCall(lines[i], Calc);

        else if (ParceSimpleCommands(Calc, command) > 0) {i++; continue;}
        
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

    return -1;
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

    else return SyntaxError;

    return 1;
}

int CodeCtor(ReadData* Calc, char* input, char* output)
{
    assert(Calc);

    Calc->fpin.filename = input;
    Calc->fpin.file_descriptor = open(input, O_RDONLY);
    Calc->fpout = fopen(output, "wb");

    if ((Calc->fpin.file_descriptor == -1) || !Calc->fpout)
    {
        printf("can't open files\n");
        if (Calc->fpout) fclose(Calc->fpout);
        return 1;
    }

    Calc->Labels = (int*) calloc(MAX_LABELS, sizeof(int));
    assert(Calc->Labels);

    memset(Calc->Labels, -1, MAX_LABELS * sizeof(int));

    Calc->buffer = (int*) calloc(CODE_CAPACITY, sizeof(int));
    assert(Calc->buffer);

    Calc->NeedSecondTurn = false;

}

int CodePrint(ReadData* Calc)
{
    for(int i = 0; i < Calc->code_len; i++) printf("%d ", Calc->buffer[i]);
    printf("\nLABELS: ");
    
    for(int i = 0; i < MAX_LABELS; i++) printf("%d ", Calc->Labels[i]);

    return 0;
}

int SizeOfFile(struct File *file)
{
    assert(file);
    assert(file->filename);

    stat(file->filename, &file->st);

    file->file_descriptor = open(file->filename, O_RDONLY);

    size_t max_size = file->st.st_size;

    if (file->file_descriptor == -1) return -1;
    
    return max_size;
}

int GetCode(ReadData *Calc)
{
    assert(Calc);

    int max_size = SizeOfFile(&(Calc->fpin));

    Calc->data = (char *) calloc(max_size, sizeof(char));

    if (!Calc->data)
    {
        printf("Memory error");
        return 1;
    }

    Calc->buffer_size = ReadFile(&(Calc->fpin), Calc->data);

    int num_of_lines = CountSymbol(Calc->data, '\n', Calc->buffer_size) + 1;
    
    Calc->Lines = (char **) calloc(num_of_lines, sizeof(char *));

    if (!Calc->Lines)
    {
        printf("Memory error");
        return 1;
    }
    
    FillLines(Calc->Lines, Calc->data, Calc->buffer_size);

    return 0;
}

int ReadFile(struct File *file, char buffer[])
{
    assert(file);
    assert(file->filename);

    file->file_descriptor = open(file->filename, O_RDONLY);

    size_t max_size = file->st.st_size;

    if (file->file_descriptor == -1)
    {
        printf("Cant open the file\n");
        return -1;
    }
    size_t real_buffer_size = read(file->file_descriptor, buffer, max_size);

    close(file->file_descriptor);

    buffer[real_buffer_size] = '\0';

    return real_buffer_size;
}

int CountSymbol(char *buffer, char sym, size_t size)
{
    assert(buffer);

    int count = 0;
    for (int i = 0; i < size; i++)
        if (buffer[i] == sym) count++;

    return count;
}

int FillLines(char * index[], char buffer[], size_t size)
{
    assert(index);
    assert(buffer);

    int current_index = 0;

    char *p = buffer;
    char *end = buffer + size;
    char *prev = buffer;

    index[current_index] = p;
    current_index++;

    while (p < end && (p = strchr(p, '\n')) != NULL)
    {
        *p = '\0';
        char *next = p + 1;

        while (*next == ' ') next++;

        if (*next != '\0' && *next != '\n' && next < end)
        {
            index[current_index++] = next;
        }

        p = next;
        prev = next;
    }

    index[current_index] = NULL;

    return current_index;
}

int ParceSimpleCommands(ReadData* Calc, char* command)
{
    assert(Calc);

    for (int i = 0; i < NUM_OF_COMMANDS && Commands_Info[i].name != NULL; i++)
        if (strcmp(command, Commands_Info[i].name) == 0)
        {
            Calc->buffer[Calc->code_len++] = Commands_Info[i].opcode;
            return Commands_Info[i].opcode;
        }

    return -1;
}