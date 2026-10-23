#define NUM_OF_COMMANDS 21

int hash_djb2(const char *str)
{
    int hash = 5381;
    int c = 0;
    while ((c = (int)*str++))
        hash = ((hash << 5) + hash) + c;

    return hash;
}

enum Opcodes
{
    HLT    = 0,
    PUSH   = 1,
    ADD    = 2,
    SUB    = 3,
    DIVari = 4,
    OUTP    = 5,
    MULT   = 6,
    PUSHr  = 7,
    POPr   = 8,
    JMP    = 9,
    JA     = 10,
    INP     = 11,
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

struct Command
{
    const char* name;

    Opcodes opcode;

    int hash;
};

Command Commands_Info[NUM_OF_COMMANDS] = 
{
    {"HLT",  HLT,    hash_djb2("HLT")},
    {"PUSH",  PUSH,  hash_djb2("PUSH")},
    {"ADD",  ADD,    hash_djb2("ADD")},
    {"SUB",  SUB,    hash_djb2("SUB")},
    {"DIV",  DIVari, hash_djb2("DIV")},
    {"OUT",  OUTP,   hash_djb2("OUT")},
    {"MULT", MULT,   hash_djb2("MULT")},
    {"PUSH",  PUSHr, hash_djb2("PUSH")},
    {"POP",  POPr,   hash_djb2("POP")},
    {"JMP",  JMP,    hash_djb2("JMP")},
    {"JA",   JA,     hash_djb2("JA")},
    {"IN",   INP,    hash_djb2("IN")},
    {"CALL", CALL,   hash_djb2("CALL")},
    {"RET",  RET,    hash_djb2("RET")},
    {"SQRT", SQRT,   hash_djb2("SQRT")},
    {"JE",   JE,     hash_djb2("JE")},
    {"PUSHM", PUSHM, hash_djb2("PUSHM")},
    {"PUSHM", PUSHMr,hash_djb2("PUSHM")},
    {"POPM", POPM,   hash_djb2("POPM")},
    {"POPM", POPMr,  hash_djb2("POPM")},
    {"RAM",  RAM,    hash_djb2("RAM")},
    
};


