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

    OPCODE();

    OPCODE(unsigned int opcode, int format, int numOfOperands, bool flagX = false);
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

extern map<string, OPCODE> opTable;
extern map<string, REG> regTable;
extern map<string, LABEL> symbolTable;
extern map<string, LITERAL> literalTable;
extern map<string, DIRECTIVE> directiveTable;

void buildOpTable();

void buildRegTable();

void buildDirectiveTable();

void addSymbol(string label, int loc);

void addLiteral(LITERAL literal);