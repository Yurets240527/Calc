#define NUM_OF_COMMANDS 21

enum Opcodes
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

struct Command
{
    const char* name;

    Opcodes opcode;
};

Command Commands_Info[NUM_OF_COMMANDS] = 
{
    {"RET", RET},
    {"IN",   IN},
    {"ADD", ADD},
    {"SUB", SUB},
    {"MULT", MULT},
    {"DIV", DIVari},
    {"SQRT", SQRT},
    {"OUT", OUT},
    {"RAM", RAM},
    {"HLT", HLT},
    
};