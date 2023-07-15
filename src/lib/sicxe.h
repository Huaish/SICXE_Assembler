#pragma once
#include "table.h"
#include "utils.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

enum STATE { STATE_EMPTY, STATE_PASS1, STATE_PASS2, STATE_ERROR };
enum TYPE { TYPE_STATEMENT, TYPE_DIRECTIVE, TYPE_COMMENT };
enum ADDRESSING { NONE_ADDRESSING, DIRECT_ADDRESSING, INDIRECT_ADDRESSING, IMMEDIATE_ADDRESSING };

string strToObjCode(string str, int &length);

extern string BASE;
extern vector<LITERAL> literalBuffer;

class SICXE {
  private:
    ADDRESSING statementType;
    bool immediate;
    bool indexed;

    int flag(int n, int i, int x, int b, int p, int e);
    string hex(int n, int size);

  public:
    STATE state;
    TYPE type;
    string statement;

    int loc;
    int length;
    string label;
    string mnemonic;
    OPCODE opcode;
    string operand1;
    string operand2;
    string objectCode;
    string errorMsg;

    void initial();

    SICXE();

    SICXE(STATE state, TYPE type, string statement);

    SICXE(string statement, STATE state, int loc, int length, string label, string mnemonic, OPCODE opcode, string operand1, string operand2,
          string objectCode);

    string format0();

    string format1();

    string format2();

    string format3();

    string format4();

    string directive();

    string ObjectCode();

    string parseOperand(int op);
};