#define NUM_OF_COMMANDS 21

int hash_djb2(const char *str);

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

enum FunctionTypes
{
    BIN_FUNC = 1,
    JUMP     = 2,
    PUSHING  = 3,
    POPING   = 4,
    NO_ARG   = 5,
    INOUTPUT = 6,

};

struct Command
{
    const char* name;

    Opcodes opcode;

    int hash;

    FunctionTypes type;
};

Command Commands_Info[NUM_OF_COMMANDS] = 
{
    {"HLT",  HLT,    hash_djb2("HLT"),   NO_ARG  },
    {"PUSH",  PUSH,  hash_djb2("PUSH"),  PUSHING },
    {"ADD",  ADD,    hash_djb2("ADD"),   BIN_FUNC},
    {"SUB",  SUB,    hash_djb2("SUB"),   BIN_FUNC},
    {"DIV",  DIVari, hash_djb2("DIV"),   BIN_FUNC},
    {"OUT",  OUTP,   hash_djb2("OUT"),   INOUTPUT},
    {"MULT", MULT,   hash_djb2("MULT"),  BIN_FUNC},
    {"PUSH",  PUSHr, hash_djb2("PUSH"),  PUSHING },
    {"POP",  POPr,   hash_djb2("POP"),   POPING  },
    {"JMP",  JMP,    hash_djb2("JMP"),   JUMP    },
    {"JA",   JA,     hash_djb2("JA"),    JUMP    },
    {"IN",   INP,    hash_djb2("IN"),    INOUTPUT},
    {"CALL", CALL,   hash_djb2("CALL"),  JUMP    },
    {"RET",  RET,    hash_djb2("RET"),   NO_ARG  },
    {"SQRT", SQRT,   hash_djb2("SQRT"),  BIN_FUNC},
    {"JE",   JE,     hash_djb2("JE"),    JUMP    },
    {"PUSHM", PUSHM, hash_djb2("PUSHM"), PUSHING },
    {"PUSHM", PUSHMr,hash_djb2("PUSHM"), PUSHING },
    {"POPM", POPM,   hash_djb2("POPM"),  POPING  },
    {"POPM", POPMr,  hash_djb2("POPM"),  POPING  },
    {"RAM",  RAM,    hash_djb2("RAM"),   NO_ARG  },
    
};


