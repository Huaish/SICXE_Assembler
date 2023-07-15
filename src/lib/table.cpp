#include "table.h"

map<string, OPCODE> opTable;
map<string, REG> regTable;
map<string, LABEL> symbolTable;
map<string, LITERAL> literalTable;
map<string, DIRECTIVE> directiveTable;

OPCODE::OPCODE() {
    opcode = 0;
    numOfOperands = 0;
    exist = false;
    flagX = false;
}

OPCODE::OPCODE(unsigned int opcode, int format, int numOfOperands, bool flagX) {
    this->opcode = opcode;
    this->format = format;
    this->numOfOperands = numOfOperands;
    this->exist = true;
    this->flagX = flagX;
}

void buildOpTable() {
    opTable["ADD"] = OPCODE(0x18, 3, 1, false);
    opTable["ADDF"] = OPCODE(0x58, 3, 1, true);
    opTable["ADDR"] = OPCODE(0x90, 2, 2, false);
    opTable["AND"] = OPCODE(0x40, 3, 1, true);
    opTable["CLEAR"] = OPCODE(0xB4, 2, 1, true);
    opTable["COMP"] = OPCODE(0x28, 3, 1, false);
    opTable["COMPF"] = OPCODE(0x88, 3, 1, true);
    opTable["COMPR"] = OPCODE(0xA0, 2, 2, true);
    opTable["DIV"] = OPCODE(0x24, 3, 1, false);
    opTable["DIVF"] = OPCODE(0x64, 3, 1, true);
    opTable["DIVR"] = OPCODE(0x9C, 2, 2, true);
    opTable["FIX"] = OPCODE(0xC4, 1, 0, true);
    opTable["FLOAT"] = OPCODE(0xC0, 1, 0, true);
    opTable["HIO"] = OPCODE(0xF4, 1, 0, true);
    opTable["J"] = OPCODE(0x3C, 3, 1, false);
    opTable["JEQ"] = OPCODE(0x30, 3, 1, false);
    opTable["JGT"] = OPCODE(0x34, 3, 1, false);
    opTable["JLT"] = OPCODE(0x38, 3, 1, false);
    opTable["JSUB"] = OPCODE(0x48, 3, 1, false);
    opTable["LDA"] = OPCODE(0x00, 3, 1, false);
    opTable["LDB"] = OPCODE(0x68, 3, 1, true);
    opTable["LDCH"] = OPCODE(0x50, 3, 1, false);
    opTable["LDF"] = OPCODE(0x70, 3, 1, true);
    opTable["LDL"] = OPCODE(0x08, 3, 1, false);
    opTable["LDS"] = OPCODE(0x6C, 3, 1, true);
    opTable["LDT"] = OPCODE(0x74, 3, 1, true);
    opTable["LDX"] = OPCODE(0x04, 3, 1, false);
    opTable["LPS"] = OPCODE(0xD0, 3, 1, true);
    opTable["MUL"] = OPCODE(0x20, 3, 1, false);
    opTable["MULF"] = OPCODE(0x60, 3, 1, true);
    opTable["MULR"] = OPCODE(0x98, 2, 2, true);
    opTable["NORM"] = OPCODE(0xC8, 1, 0, true);
    opTable["OR"] = OPCODE(0x44, 3, 1, false);
    opTable["RD"] = OPCODE(0xD8, 3, 1, false);
    opTable["RMO"] = OPCODE(0xAC, 2, 2, true);
    opTable["RSUB"] = OPCODE(0x4C, 3, 0, false);
    opTable["SHIFTL"] = OPCODE(0xA4, 2, 2, true);
    opTable["SHIFTR"] = OPCODE(0xA8, 2, 2, true);
    opTable["SIO"] = OPCODE(0xF0, 1, 0, true);
    opTable["SSK"] = OPCODE(0xEC, 3, 1, true);
    opTable["STA"] = OPCODE(0x0C, 3, 1, false);
    opTable["STB"] = OPCODE(0x78, 3, 1, true);
    opTable["STCH"] = OPCODE(0x54, 3, 1, false);
    opTable["STF"] = OPCODE(0x80, 3, 1, true);
    opTable["STI"] = OPCODE(0xD4, 3, 1, true);
    opTable["STL"] = OPCODE(0x14, 3, 1, false);
    opTable["STS"] = OPCODE(0x7C, 3, 1, true);
    opTable["STSW"] = OPCODE(0xE8, 3, 1, false);
    opTable["STT"] = OPCODE(0x84, 3, 1, true);
    opTable["STX"] = OPCODE(0x10, 3, 1, false);
    opTable["SUB"] = OPCODE(0x1C, 3, 1, false);
    opTable["SUBF"] = OPCODE(0x5C, 3, 1, true);
    opTable["SUBR"] = OPCODE(0x94, 2, 2, true);
    opTable["SVC"] = OPCODE(0xB0, 2, 1, true);
    opTable["TD"] = OPCODE(0xE0, 3, 1, false);
    opTable["TIO"] = OPCODE(0xF8, 1, 0, true);
    opTable["TIX"] = OPCODE(0x2C, 3, 1, false);
    opTable["TIXR"] = OPCODE(0xB8, 2, 1, true);
    opTable["WD"] = OPCODE(0xDC, 3, 1, false);
}

void buildRegTable() {
    regTable["A"] = REG(0);
    regTable["X"] = REG(1);
    regTable["L"] = REG(2);
    regTable["B"] = REG(3);
    regTable["S"] = REG(4);
    regTable["T"] = REG(5);
    regTable["F"] = REG(6);
    regTable["PC"] = REG(8);
    regTable["SW"] = REG(9);
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
    if (symbolTable[label].exist) {
        cout << "\033[1;31m"
             << "Error: Duplicate symbol " << label << "\033[0m" << endl;
        return;
    }
    symbolTable[label] = LABEL(loc, true);
}

void addLiteral(LITERAL literal) {
    if (literalTable[literal.literal].exist) {
        return;
    }
    literalTable[literal.literal] = literal;
}