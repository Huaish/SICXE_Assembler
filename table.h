#pragma once
#include "color.h"
#include "table.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

struct OPCODE {
    unsigned int opcode;
    int format;
    int numOfOperands;
    bool exist;
    OPCODE()
    {
        opcode = 0;
        numOfOperands = 0;
        exist = false;
    }

    OPCODE(unsigned int opcode, int format, int numOfOperands, bool exist)
    {
        this->opcode = opcode;
        this->format = format;
        this->numOfOperands = numOfOperands;
        this->exist = exist;
    }
};

struct REG {
    int num;
    bool exist;
    REG() { exist = false; }
    REG(int num, bool exist)
    {
        this->num = num;
        this->exist = exist;
    }
};

struct DIRECTIVE {
    int size;
    bool exist;
    DIRECTIVE() { exist = false; }
    DIRECTIVE(int size, bool exist)
    {
        this->size = size;
        this->exist = exist;
    }
};

struct LABEL {
    int address;
    bool exist;
    LABEL()
    {
        address = 0;
        exist = false;
    }
    LABEL(int address, bool exist)
    {
        this->address = address;
        this->exist = exist;
    }
};

typedef map<string, OPCODE> OPTABLE;
typedef map<string, REG> REGTABLE;
typedef map<string, LABEL> LABELTABLE;
typedef map<string, DIRECTIVE> DIRECTIVETABLE;

extern OPTABLE optable;
extern REGTABLE regtable;
extern LABELTABLE symboltable;
extern DIRECTIVETABLE directiveTable;

void buildOpTable();
void buildRegTable();
void buildDirectiveTable();
void addSymbol(string label, int loc);
void printSymbolTable();
