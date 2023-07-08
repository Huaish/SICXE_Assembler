#pragma once
#include "color.h"
#include "utils.h"
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
    bool flagX; // true if instruction only on XE version

    OPCODE() {
        opcode = 0;
        numOfOperands = 0;
        exist = false;
        flagX = false;
    }

    OPCODE(unsigned int opcode, int format, int numOfOperands, bool flagX = false) {
        this->opcode = opcode;
        this->format = format;
        this->numOfOperands = numOfOperands;
        this->exist = true;
        this->flagX = flagX;
    }
};

struct REG {
    int num;
    bool exist;
    REG() { exist = false; }
    REG(int num) {
        this->num = num;
        this->exist = true;
    }
};

struct DIRECTIVE {
    int size;
    bool exist;
    DIRECTIVE() { exist = false; }
    DIRECTIVE(int size) {
        this->size = size;
        this->exist = true;
    }
};

struct LABEL {
    int address;
    bool exist;
    LABEL() {
        address = 0;
        exist = false;
    }
    LABEL(int address, bool exist) {
        this->address = address;
        this->exist = true;
    }
};

struct LITERAL {
    int address;
    bool exist;
    string literal;
    string objCode;
    int length;
    bool isPrinted;
    LITERAL() {
        address = 0;
        exist = false;
        objCode = "";
        length = 0;
        isPrinted = false;
    }

    LITERAL(string literal, string objCode, int length) {
        this->address = 0;
        this->literal = literal;
        this->objCode = objCode;
        this->length = length;
        this->exist = true;
        this->isPrinted = false;
    }

    LITERAL(string literal, string objCode, int address, int length) {
        this->address = address;
        this->literal = literal;
        this->objCode = objCode;
        this->length = length;
        this->exist = true;
        this->isPrinted = false;
    }
};

typedef map<string, OPCODE> OPTABLE;
typedef map<string, REG> REGTABLE;
typedef map<string, LABEL> LABELTABLE;
typedef map<string, DIRECTIVE> DIRECTIVETABLE;
typedef map<string, LITERAL> LITERALTABLE;

OPTABLE optable;
REGTABLE regtable;
LABELTABLE symboltable;
LITERALTABLE literaltable;
DIRECTIVETABLE directiveTable;

void buildOpTable() {
    optable["ADD"] = OPCODE(0x18, 3, 1, false);
    optable["ADDF"] = OPCODE(0x58, 3, 1, true);
    optable["ADDR"] = OPCODE(0x90, 2, 2, false);
    optable["AND"] = OPCODE(0x40, 3, 1, true);
    optable["CLEAR"] = OPCODE(0xB4, 2, 1, true);
    optable["COMP"] = OPCODE(0x28, 3, 1, false);
    optable["COMPF"] = OPCODE(0x88, 3, 1, true);
    optable["COMPR"] = OPCODE(0xA0, 2, 2, true);
    optable["DIV"] = OPCODE(0x24, 3, 1, false);
    optable["DIVF"] = OPCODE(0x64, 3, 1, true);
    optable["DIVR"] = OPCODE(0x9C, 2, 2, true);
    optable["FIX"] = OPCODE(0xC4, 1, 0, true);
    optable["FLOAT"] = OPCODE(0xC0, 1, 0, true);
    optable["HIO"] = OPCODE(0xF4, 1, 0, true);
    optable["J"] = OPCODE(0x3C, 3, 1, false);
    optable["JEQ"] = OPCODE(0x30, 3, 1, false);
    optable["JGT"] = OPCODE(0x34, 3, 1, false);
    optable["JLT"] = OPCODE(0x38, 3, 1, false);
    optable["JSUB"] = OPCODE(0x48, 3, 1, false);
    optable["LDA"] = OPCODE(0x00, 3, 1, false);
    optable["LDB"] = OPCODE(0x68, 3, 1, true);
    optable["LDCH"] = OPCODE(0x50, 3, 1, false);
    optable["LDF"] = OPCODE(0x70, 3, 1, true);
    optable["LDL"] = OPCODE(0x08, 3, 1, false);
    optable["LDS"] = OPCODE(0x6C, 3, 1, true);
    optable["LDT"] = OPCODE(0x74, 3, 1, true);
    optable["LDX"] = OPCODE(0x04, 3, 1, false);
    optable["LPS"] = OPCODE(0xD0, 3, 1, true);
    optable["MUL"] = OPCODE(0x20, 3, 1, false);
    optable["MULF"] = OPCODE(0x60, 3, 1, true);
    optable["MULR"] = OPCODE(0x98, 2, 2, true);
    optable["NORM"] = OPCODE(0xC8, 1, 0, true);
    optable["OR"] = OPCODE(0x44, 3, 1, false);
    optable["RD"] = OPCODE(0xD8, 3, 1, false);
    optable["RMO"] = OPCODE(0xAC, 2, 2, true);
    optable["RSUB"] = OPCODE(0x4C, 3, 0, false);
    optable["SHIFTL"] = OPCODE(0xA4, 2, 2, true);
    optable["SHIFTR"] = OPCODE(0xA8, 2, 2, true);
    optable["SIO"] = OPCODE(0xF0, 1, 0, true);
    optable["SSK"] = OPCODE(0xEC, 3, 1, true);
    optable["STA"] = OPCODE(0x0C, 3, 1, false);
    optable["STB"] = OPCODE(0x78, 3, 1, true);
    optable["STCH"] = OPCODE(0x54, 3, 1, false);
    optable["STF"] = OPCODE(0x80, 3, 1, true);
    optable["STI"] = OPCODE(0xD4, 3, 1, true);
    optable["STL"] = OPCODE(0x14, 3, 1, false);
    optable["STS"] = OPCODE(0x7C, 3, 1, true);
    optable["STSW"] = OPCODE(0xE8, 3, 1, false);
    optable["STT"] = OPCODE(0x84, 3, 1, true);
    optable["STX"] = OPCODE(0x10, 3, 1, false);
    optable["SUB"] = OPCODE(0x1C, 3, 1, false);
    optable["SUBF"] = OPCODE(0x5C, 3, 1, true);
    optable["SUBR"] = OPCODE(0x94, 2, 2, true);
    optable["SVC"] = OPCODE(0xB0, 2, 1, true);
    optable["TD"] = OPCODE(0xE0, 3, 1, false);
    optable["TIO"] = OPCODE(0xF8, 1, 0, true);
    optable["TIX"] = OPCODE(0x2C, 3, 1, false);
    optable["TIXR"] = OPCODE(0xB8, 2, 1, true);
    optable["WD"] = OPCODE(0xDC, 3, 1, false);
}

void buildRegTable() {
    regtable["A"] = REG(0);
    regtable["X"] = REG(1);
    regtable["L"] = REG(2);
    regtable["B"] = REG(3);
    regtable["S"] = REG(4);
    regtable["T"] = REG(5);
    regtable["F"] = REG(6);
    regtable["PC"] = REG(8);
    regtable["SW"] = REG(9);
}

void buildDirectiveTable() {
    directiveTable["START"] = DIRECTIVE(0);
    directiveTable["END"] = DIRECTIVE(0);
    directiveTable["BYTE"] = DIRECTIVE(1);
    directiveTable["WORD"] = DIRECTIVE(3);
    directiveTable["RESB"] = DIRECTIVE(1);
    directiveTable["RESW"] = DIRECTIVE(3);
    directiveTable["BASE"] = DIRECTIVE(0);
    directiveTable["NOBASE"] = DIRECTIVE(0);
    directiveTable["EQU"] = DIRECTIVE(9);
    directiveTable["LTORG"] = DIRECTIVE(10);
}

void addSymbol(string label, int loc) {
    if (symboltable[label].exist) {
        cout << "\033[1;31m"
             << "Error: Duplicate symbol " << label << "\033[0m" << endl;
        return;
    }
    symboltable[label] = LABEL(loc, true);
}

void addLiteral(LITERAL literal) {
    if (literaltable[literal.literal].exist) {
        return;
    }
    literaltable[literal.literal] = literal;
}

void printSymbolTable() {
    cout << On_Orange << "        Symbol Table        " << RESET << endl;

    cout << "Name\t\tValue" << endl;

    for (map<string, LABEL>::iterator it = symboltable.begin(); it != symboltable.end(); it++) {
        cout << it->first << "\t\t" << sethex << it->second.address << endl;
    }
}