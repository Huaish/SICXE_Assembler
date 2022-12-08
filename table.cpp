#include "table.h"
#include "color.h"
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
using namespace std;

OPTABLE optable;
REGTABLE regtable;
LABELTABLE symboltable;
DIRECTIVETABLE directiveTable;

void buildOpTable()
{
    optable["ADD"] = OPCODE(0x18, 3, 1, true);
    optable["ADDF"] = OPCODE(0x58, 3, 1, true);
    optable["ADDR"] = OPCODE(0x90, 2, 2, true);
    optable["AND"] = OPCODE(0x40, 3, 1, true);
    optable["CLEAR"] = OPCODE(0xB4, 2, 1, true);
    optable["COMP"] = OPCODE(0x28, 3, 1, true);
    optable["COMPF"] = OPCODE(0x88, 3, 1, true);
    optable["COMPR"] = OPCODE(0xA0, 2, 2, true);
    optable["DIV"] = OPCODE(0x24, 3, 1, true);
    optable["DIVF"] = OPCODE(0x64, 3, 1, true);
    optable["DIVR"] = OPCODE(0x9C, 2, 2, true);
    optable["FIX"] = OPCODE(0xC4, 1, 0, true);
    optable["FLOAT"] = OPCODE(0xC0, 1, 0, true);
    optable["HIO"] = OPCODE(0xF4, 1, 0, true);
    optable["J"] = OPCODE(0x3C, 3, 1, true);
    optable["JEQ"] = OPCODE(0x30, 3, 1, true);
    optable["JGT"] = OPCODE(0x34, 3, 1, true);
    optable["JLT"] = OPCODE(0x38, 3, 1, true);
    optable["JSUB"] = OPCODE(0x48, 3, 1, true);
    optable["LDA"] = OPCODE(0x00, 3, 1, true);
    optable["LDB"] = OPCODE(0x68, 3, 1, true);
    optable["LDCH"] = OPCODE(0x50, 3, 1, true);
    optable["LDF"] = OPCODE(0x70, 3, 1, true);
    optable["LDL"] = OPCODE(0x08, 3, 1, true);
    optable["LDS"] = OPCODE(0x6C, 3, 1, true);
    optable["LDT"] = OPCODE(0x74, 3, 1, true);
    optable["LDX"] = OPCODE(0x04, 3, 1, true);
    optable["LPS"] = OPCODE(0xD0, 3, 1, true);
    optable["MUL"] = OPCODE(0x20, 3, 1, true);
    optable["MULF"] = OPCODE(0x60, 3, 1, true);
    optable["MULR"] = OPCODE(0x98, 2, 2, true);
    optable["NORM"] = OPCODE(0xC8, 1, 0, true);
    optable["OR"] = OPCODE(0x44, 3, 1, true);
    optable["RD"] = OPCODE(0xD8, 3, 1, true);
    optable["RMO"] = OPCODE(0xAC, 2, 2, true);
    optable["RSUB"] = OPCODE(0x4C, 3, 0, true);
    optable["SHIFTL"] = OPCODE(0xA4, 2, 2, true);
    optable["SHIFTR"] = OPCODE(0xA8, 2, 2, true);
    optable["SIO"] = OPCODE(0xF0, 1, 0, true);
    optable["SSK"] = OPCODE(0xEC, 3, 1, true);
    optable["STA"] = OPCODE(0x0C, 3, 1, true);
    optable["STB"] = OPCODE(0x78, 3, 1, true);
    optable["STCH"] = OPCODE(0x54, 3, 1, true);
    optable["STF"] = OPCODE(0x80, 3, 1, true);
    optable["STI"] = OPCODE(0xD4, 3, 1, true);
    optable["STL"] = OPCODE(0x14, 3, 1, true);
    optable["STS"] = OPCODE(0x7C, 3, 1, true);
    optable["STSW"] = OPCODE(0xE8, 3, 1, true);
    optable["STT"] = OPCODE(0x84, 3, 1, true);
    optable["STX"] = OPCODE(0x10, 3, 1, true);
    optable["SUB"] = OPCODE(0x1C, 3, 1, true);
    optable["SUBF"] = OPCODE(0x5C, 3, 1, true);
    optable["SUBR"] = OPCODE(0x94, 2, 2, true);
    optable["SVC"] = OPCODE(0xB0, 2, 1, true);
    optable["TD"] = OPCODE(0xE0, 3, 1, true);
    optable["TIO"] = OPCODE(0xF8, 1, 0, true);
    optable["TIX"] = OPCODE(0x2C, 3, 1, true);
    optable["TIXR"] = OPCODE(0xB8, 2, 1, true);
    optable["WD"] = OPCODE(0xDC, 3, 1, true);
}

void buildRegTable()
{
    regtable["A"] = REG(0, true);
    regtable["X"] = REG(1, true);
    regtable["L"] = REG(2, true);
    regtable["B"] = REG(3, true);
    regtable["S"] = REG(4, true);
    regtable["T"] = REG(5, true);
    regtable["F"] = REG(6, true);
    regtable["PC"] = REG(8, true);
    regtable["SW"] = REG(9, true);
}

void buildDirectiveTable()
{
    directiveTable["START"] = DIRECTIVE(0, true);
    directiveTable["END"] = DIRECTIVE(0, true);
    directiveTable["BYTE"] = DIRECTIVE(1, true);
    directiveTable["WORD"] = DIRECTIVE(3, true);
    directiveTable["RESB"] = DIRECTIVE(1, true);
    directiveTable["RESW"] = DIRECTIVE(3, true);
    directiveTable["BASE"] = DIRECTIVE(0, true);
    directiveTable["NOBASE"] = DIRECTIVE(0, true);
    // directiveTable["EQU"] = DIRECTIVE(9, true);
    // directiveTable["LTORG"] = DIRECTIVE(10, true);
}

void addSymbol(string label, int loc)
{
    if (symboltable[label].exist) {
        // throw("Error: Duplicate symbol " + label);
        cout << "\033[1;31m"
             << "Error: Duplicate symbol " << label << "\033[0m" << endl;
        return;
    }
    symboltable[label] = LABEL(loc, true);
}

void printSymbolTable()
{
    cout << On_ICyan << "        Symbol Table        " << RESET << endl;

    cout << "Name\t\tValue" << endl;

    for (map<string, LABEL>::iterator it = symboltable.begin(); it != symboltable.end(); it++) {
        cout << it->first << "\t\t" << hex << it->second.address << endl;
    }
}