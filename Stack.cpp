#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef int StackElem;
#define STACK_ELEM_FMT "%d"

#define STACK_DUMP(stk) StackDump(&stk, __FILE__, __LINE__, __func__)

#define InitStack(stk, capacity) InitStackInfo((stk), (capacity), __FILE__, __LINE__, __func__)

#define MAX_CAPACITY 1000000

#define  LEFT_PETUSHARA 3802
#define RIGHT_PETUSHARA 3565

#define POISON 671488

enum Errors
{
    OK = 0,
    NULL_PTR = 1,
    MEMORY_ERROR = 2,
    STACK_UNDERFLOW = 3,
    KILLED_PETUSHARA = 4,
};

struct StackInfo
{
    const char* file;
    int         line;
    const char* func;
};

struct stack //ERROR_CODE, INFO EBOUT STACK
{
    size_t size;
    size_t capacity;
    StackElem *data;

    Errors err_code;

    struct StackInfo InitInfo;
};

Errors InitStackInfo(stack *stk, size_t capacity, const char* file, int line, const char* func);

Errors StackPush(stack *stk, StackElem value);

StackElem StackPop(stack *stk);

Errors StackDestroy(stack *stk);

Errors StackResize(stack *stk);

int PrintError(int err);

Errors StackVerifier(stack *stk);

int StackDump(stack *stk, const char *file, int line, const char *func);


Errors InitStackInfo(stack *stk, size_t capacity, const char* file, int line, const char* func)
{
    //TODO POIZON 
    if (!stk) return NULL_PTR;

    stk->capacity = capacity;

    if (capacity == 0 || capacity > MAX_CAPACITY) return MEMORY_ERROR;

    stk->data = (StackElem *) calloc(capacity+2, sizeof(StackElem));

    if (!stk->data) return MEMORY_ERROR;

    for (size_t i = 0; i < capacity + 2; i++) stk->data[i] = POISON;

    stk->data[0]          =  LEFT_PETUSHARA;
    stk->data[capacity+1] = RIGHT_PETUSHARA;

    stk->data++;
    stk->size = 0;

    stk->InitInfo.file = file;
    stk->InitInfo.line = line;
    stk->InitInfo.func = func;

    return StackVerifier(stk);
}
//TODO ENUM PISHI VEZDE (ERRORS)
Errors StackPush(stack *stk, StackElem value)
{
    StackVerifier(stk);
    if (stk->err_code) return stk->err_code;

    if(stk->size >= stk->capacity) StackResize(stk);

    stk->data[stk->size++] = value;

    return StackVerifier(stk);
}

StackElem StackPop(stack *stk)
{ 
    if (StackVerifier(stk)) return POISON;

    StackElem pop_elem = stk->data[--stk->size];
    stk->data[stk->size] = POISON;
    
    if (StackVerifier(stk)) return POISON;

    return pop_elem;
}

Errors StackDestroy(stack *stk)
{
    StackVerifier(stk);
    if (stk->err_code) return stk->err_code;

    free(stk->data - 1);
    stk->data = NULL;
    stk->size = 0;
    stk->capacity = 0;
    stk->err_code = OK;

    return OK;
}

Errors StackResize(stack *stk)
{
    StackVerifier(stk);
    if (stk->err_code) return stk->err_code;

    printf("Changed capacity from %d to ", stk->capacity);
    stk->data = (StackElem *) realloc(stk->data-1, (stk->capacity+2) * 2 * sizeof(StackElem));

    if (!stk->data) return (stk->err_code = MEMORY_ERROR);

    stk->data++;

    for (size_t i = stk->capacity; i < stk->capacity*2; i++) stk->data[i] = POISON;

    stk->capacity*=2;
    printf("%d\n", stk->capacity);

    stk->data[-1]            =  LEFT_PETUSHARA;
    stk->data[stk->capacity] = RIGHT_PETUSHARA;


    return StackVerifier(stk);
}

int PrintError(int err)
{
    printf("Error code %d ", err);
    switch (err)
    {
        case OK:
            printf("No errors\n");
            break;
        case NULL_PTR:
            printf("NULL pointer error\n");
            break;
        case MEMORY_ERROR:
            printf("Memory error\n");
            break;
        case STACK_UNDERFLOW:
            printf("Empty stack\n");
            break;
        case KILLED_PETUSHARA:
            printf("Killed cannery");
            break;
        default:
            printf("No errors\n");
            break;
    }
}
//TODO STACKVERIFIER VEZDE PISHI EGO
Errors StackVerifier(stack *stk)
{
    if (!stk)                      return (stk->err_code = NULL_PTR);
    if (stk->err_code)             return stk->err_code;
    if (!stk->data)                return (stk->err_code = NULL_PTR);
    if (!stk->capacity)            return (stk->err_code = NULL_PTR);
    if (stk->size > stk->capacity) return (stk->err_code = MEMORY_ERROR);
    if (stk->size < 0 ||
        stk->size > MAX_CAPACITY)  return (stk->err_code = STACK_UNDERFLOW);
    if (stk->data[-1] != LEFT_PETUSHARA || 
    stk->data[stk->capacity] != RIGHT_PETUSHARA) return (stk->err_code = KILLED_PETUSHARA);

    return (stk->err_code = OK);
}

int StackDump(stack *stk, const char *file, int line, const char *func)
{
    printf("\n=================== STACK DUMP ===================\n");
    printf("Called from: %s:%d  in  %s()\n", file, line, func);
    printf("Created    : %s:%d  in  %s()\n", stk->InitInfo.file, stk->InitInfo.line, stk->InitInfo.func);

    if (!stk)
    {
        printf("  stk pointer : NULL  -> nothing to dump\n");
        printf("==================================================\n\n");
        return NULL_PTR;
    }

    printf("  stk address : %p\n", stk);
    printf("  size        : %d\n", stk->size);
    printf("  capacity    : %d\n", stk->capacity);
    printf("  data ptr    : %p\n", stk->data);
    printf("  error code  : %d\n", stk->err_code);

    printf("  --- checks ---\n");
    if (!stk->data)
    {
        printf("  [!] data == NULL\n");
        return NULL_PTR;
    }

    if (stk->capacity == 0)
        printf("  [!] capacity == 0\n");

    if (stk->size > stk->capacity)
        printf("  [!] size > capacity  (size=%d, capacity=%d)\n",
               stk->size, stk->capacity);

    if (stk->size == 0 && stk->capacity > 0)
        printf("  [i] stack is empty\n");

    if (stk->data[-1] != LEFT_PETUSHARA)
        printf("  [!] LEFT  PETUSHARA IS SPOILED");

    if (stk->data[stk->capacity] != RIGHT_PETUSHARA)
        printf("  [!] RIGHT PETUSHARA IS SPOILED");


    if (stk->capacity == 0)
    {
        printf("  (capacity is 0, no buffer to print)\n");
        return MEMORY_ERROR;
    }

    printf("  --- data[0..%d] ---\n\n", stk->capacity - 1);

    printf("    Left petushara: " STACK_ELEM_FMT "\n\n", stk->data[-1]);
    for (size_t i = 0; i < stk->capacity; i++)
    {
        const char *is_used = NULL;

        if (i + 1 == stk->size)
            is_used = "<- top";

        else if (i < stk->size)
            is_used = "used";

        else
            is_used = "unused";

        printf("    data[%3d] = " STACK_ELEM_FMT "  (%s)\n",
                i, stk->data[i], is_used);
    }
    printf("\n    Right petushara: " STACK_ELEM_FMT "\n\n", stk->data[stk->capacity]);
    

    printf("  top pointer : data + size = %p\n",
           (stk->data + stk->size));
    printf("==================================================\n\n");
}