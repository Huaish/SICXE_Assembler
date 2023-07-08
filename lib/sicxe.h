#pragma once
#include "table.h"
#include "utils.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace sicxe {

enum STATE { EMPTY, PASS1, PASS2, ERROR };
enum TYPE { STATEMENT, DIRECTIVE, COMMENT };
enum STATEMENT_TYPE { NONE_TYPE, DIRECT, INDIRECT, IMMEDIATE };

string strToObjCode(string str, int &length);

string BASE = "";
vector<LITERAL> literalBuffer;
class SICXE {
  private:
    STATEMENT_TYPE statementType;
    bool immediate;
    bool indexed;

    int flag(int n, int i, int x, int b, int p, int e) { return n << 5 | i << 4 | x << 3 | b << 2 | p << 1 | e; }
    string hex(int n, int size) {
        stringstream ss;
        ss << sethex << n;
        string result = ss.str();
        return string(size - result.length(), '0') + result;
    }

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

    void initial() {
        this->state = EMPTY;
        this->type = COMMENT;
        this->statementType = NONE_TYPE;
        this->indexed = false;

        this->loc = 0;
        this->length = 0;
        this->statement = "";
        this->label = "";
        this->mnemonic = "";
        this->operand1 = "";
        this->operand2 = "";
        this->objectCode = "";
        this->errorMsg = "";
    }

    SICXE() { initial(); }

    SICXE(STATE state, TYPE type, string statement) {
        initial();
        this->state = state;
        this->type = type;
        this->statementType = NONE_TYPE;
        this->statement = statement;
    }

    SICXE(string statement, STATE state, int loc, int length, string label, string mnemonic, OPCODE opcode, string operand1, string operand2,
          string objectCode) {
        initial();
        this->statement = statement;
        this->state = state;
        this->type = STATEMENT;

        this->loc = loc;
        this->length = length;
        this->label = label;
        this->mnemonic = mnemonic;
        this->opcode = opcode;
        this->operand1 = operand1;
        this->operand2 = operand2;
        this->objectCode = objectCode;

        this->indexed = this->operand2 == "X";

        this->operand1 = parseOperand(1);
        this->operand2 = parseOperand(2);

        if (directiveTable[this->mnemonic].exist) {
            this->type = DIRECTIVE;
            ObjectCode();
        }

        if (state == PASS2 && optable[this->mnemonic].exist) {
            ObjectCode();
        }
    }

    string format0() {
        if (opcode.flagX) {
            state = ERROR;
            errorMsg = "Instruction only on XE version";
            return "";
        }
        int obj = opcode.opcode;
        obj <<= 1;
        obj += indexed;
        obj <<= 15;
        if (operand1.length() > 0) {
            if (isDecimal(operand1))
                obj += stoi(operand1);
            else if (symboltable[operand1].exist)
                obj += symboltable[operand1].address;
            else if (literaltable[operand1].exist)
                obj += literaltable[operand1].address;
        }

        return hex(obj, 6);
    }

    string format1() { return hex(opcode.opcode, 2); }

    string format2() {
        int obj = opcode.opcode;
        obj <<= 4;
        if (this->operand1.length() > 0) {
            if (isDecimal(operand1))
                obj += stoi(operand1);
            else if (regtable[operand1].exist)
                obj += regtable[operand1].num;
            else {
                state = ERROR;
                errorMsg = "ERROR(Invalid register): " + operand1;
                return "";
            }
        }
        obj <<= 4;
        if (this->operand2.length() > 0) {
            if (isDecimal(operand2))
                obj += stoi(operand2);
            else if (regtable[operand2].exist)
                obj += regtable[operand2].num;
            else {
                state = ERROR;
                errorMsg = "ERROR(Invalid register): " + operand2;
                return "";
            }
        }
        return hex(obj, 4);
    }

    string format3() {
        int obj = 0;
        int disp = 0;
        int pc = loc + 3;
        int base = symboltable[BASE].address;
        int target = 0;
        if (opcode.numOfOperands == 0) {
            obj = opcode.opcode;
            obj <<= 4;
            obj += flag(1, 1, 0, 0, 0, 0);
            obj <<= 12;
            return hex(obj, 6);
        }

        if (statementType == IMMEDIATE) {
            if (symboltable[operand1].exist) {
                if (symboltable[operand1].exist == false) {
                    state = ERROR;
                    errorMsg = "ERROR(Invalid operand): " + operand1;
                    return "";
                }
                target = symboltable[operand1].address;
                // pc relative
                if (target - pc >= -2048 && target - pc <= 2047) {
                    disp = target - pc;
                    disp = disp & 0b0000111111111111;
                    obj = optable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(0, 1, 0, 0, 1, 0);
                    obj <<= 12;
                    obj += disp;
                }
                // base relative
                else {
                    if (target - base >= 0 && target - base <= 4095) {
                        disp = target - base;
                        obj = optable[mnemonic].opcode;
                        obj <<= 4;
                        obj += flag(0, 1, 0, 1, 0, 0);
                        obj <<= 12;
                        obj += disp;
                    } else {
                        state = ERROR;
                        errorMsg = "ERROR(Out of range): " + operand1;
                        return "";
                    }
                }
            } else {
                // immediate
                if (isDecimal(operand1) == false) {
                    state = ERROR;
                    errorMsg = "ERROR(Invalid operand): " + operand1;
                    return "";
                }

                disp = stoi(operand1);
                disp = disp & 0b0000111111111111;
                obj = optable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 0, 0);
                obj <<= 12;
                obj += disp;
            }
        } else if (statementType == INDIRECT) {
            if (symboltable[operand1].exist) {
                target = symboltable[operand1].address;
            } else if (literaltable[operand1].exist) {
                target = literaltable[operand1].address;
            } else if (isDecimal(operand1)) {
                disp = stoi(operand1);
                disp = disp & 0b0000111111111111;
                obj = optable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(0, 0, 0, 0, 0, 0);
                obj <<= 12;
                obj += disp;
                return hex(obj, 6);
            } else {
                state = ERROR;
                errorMsg = "ERROR(Invalid operand): " + operand1;
                return "";
            }

            // pc relative
            if (target - pc >= -2048 && target - pc <= 2047) {
                disp = target - pc;
                disp = disp & 0b0000111111111111;
                obj = optable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(1, 0, 0, 0, 1, 0);
                obj <<= 12;
                obj += disp;
            }
            // base relative
            else {
                if (target - base >= 0 && target - base <= 4095) {
                    disp = target - base;
                    obj = optable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(1, 0, 0, 1, 0, 0);
                    obj <<= 12;
                    obj += disp;
                } else {
                    // throw("error: out of range");
                    state = ERROR;
                    errorMsg = "ERROR(out of range)";
                    return "";
                }
            }
        } else if (statementType == DIRECT) { // simple addressing
            if (symboltable[operand1].exist) {
                target = symboltable[operand1].address;
            } else if (literaltable[operand1].exist) {
                target = literaltable[operand1].address;
            } else if (isDecimal(operand1)) {
                disp = stoi(operand1);
                disp = disp & 0b0000111111111111;
                obj = optable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(1, 1, this->indexed, 0, 0, 0);
                obj <<= 12;
                obj += disp;
                return hex(obj, 6);
            } else {
                state = ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }

            // pc relative
            if (target - pc >= -2048 && target - pc <= 2047) {
                disp = target - pc;
                disp = disp & 0b0000111111111111;
                obj = optable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(1, 1, indexed, 0, 1, 0);
                obj <<= 12;
                obj += disp;
            }
            // base relative
            else {
                if (target - base >= 0 && target - base <= 4095) {
                    disp = target - base;
                    obj = optable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(1, 1, indexed, 1, 0, 0);
                    obj <<= 12;
                    obj += disp;
                } else {
                    state = ERROR;
                    errorMsg = "ERROR(out of range)";
                    return "";
                }
            }
        } else {
            return "";
        }

        return hex(obj, 6);
    }

    string format4() {
        int obj = 0;
        if (statementType == IMMEDIATE) {
            if (symboltable[operand1].exist) {
                int target = symboltable[operand1].address;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 0, 1);
                obj <<= 20;
                obj += target;
            } else {
                if (isDecimal(operand1) == false) {
                    state = ERROR;
                    errorMsg = "ERROR(Invalid operand): " + operand1;
                    return "";
                }
                int disp = stoi(operand1);
                disp = disp & 0b00000000000011111111111111111111;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 0, 1);
                obj <<= 20;
                obj += disp;
            }
        } else if (statementType == INDIRECT) {
            if (symboltable[operand1].exist) {
                int target = symboltable[operand1].address;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(1, 0, 0, 0, 0, 1);
                obj <<= 20;
                obj += target;
            } else {
                state = ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
        } else if (statementType == DIRECT) { // simple addressing
            int target = 0;
            if (symboltable[operand1].exist) {
                target = symboltable[operand1].address;
            } else if (literaltable[operand1].exist) {
                target = literaltable[operand1].address;
            } else {
                state = ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
            obj = opcode.opcode;
            obj <<= 4;
            obj += flag(1, 1, indexed, 0, 0, 1);
            obj <<= 20;
            obj += target;
        } else {
            return "";
        }

        return hex(obj, 8);
    }

    string directive() {
        if (mnemonic == "START") {
            loc = stoi(operand1, nullptr, 16);
            length = 0;
        } else if (mnemonic == "END") {
            int pc = loc + length;
            while (!literalBuffer.empty()) {
                LITERAL literal = literalBuffer.front();
                literalBuffer.erase(literalBuffer.begin());
                literal.address = pc;
                addLiteral(literal);
                pc += literal.length;
            }
            this->length = 0;
        } else if (mnemonic == "BYTE") {
            objectCode = strToObjCode(operand1, length);
        } else if (mnemonic == "WORD") {
            if (isNumber(operand1)) {
                operand1 = hex(stoi(operand1), 6);
                objectCode = string(6 - operand1.length(), '0') + operand1;
                length = 3;
            } else {
                state = ERROR;
                errorMsg = "ERROR(Invalid operand): " + operand1;
                return "";
            }
        } else if (mnemonic == "RESB") {
            length = stoi(operand1);
        } else if (mnemonic == "RESW") {
            length = stoi(operand1) * 3;
        } else if (mnemonic == "BASE") {
            if (opcode.format == 0) {
                state = ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
            BASE = operand1;
        } else if (mnemonic == "NOBASE") {
            if (opcode.format == 0) {
                state = ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
            BASE = "";
        } else if (mnemonic == "EQU") {
            if (operand1 == "*") {

            } else {
                if (symboltable[operand1].exist) {
                    int tmp = this->loc - symboltable[operand1].address;
                    this->length += tmp;
                    this->loc = symboltable[operand1].address;
                } else if (isDecimal(operand1)) {
                    int tmp = this->loc - stoi(operand1);
                    this->length += tmp;
                    this->loc = stoi(operand1);
                } else {
                    state = ERROR;
                    errorMsg = "ERROR(undefined symbol): " + operand1;
                    return "";
                }
            }
        } else if (mnemonic == "LTORG") {
            if (opcode.format == 0) {
                state = ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
            int pc = loc + length;
            while (!literalBuffer.empty()) {
                LITERAL literal = literalBuffer.front();
                literalBuffer.erase(literalBuffer.begin());
                literal.address = pc;
                addLiteral(literal);
                pc += literal.length;
            }
            this->length = 0;
        } else if (mnemonic == "ORG") {
            if (operand1 == "*") {
                this->length = 0;
            } else {
                if (symboltable[operand1].exist) {
                    this->loc = symboltable[operand1].address;
                    this->length = 0;
                } else if (isDecimal(operand1)) {
                    this->loc = stoi(operand1);
                    this->length = 0;
                } else {
                    state = ERROR;
                    errorMsg = "ERROR(undefined symbol): " + operand1;
                    return "";
                }
            }
        }
        return this->objectCode;
    }

    string ObjectCode() {
        if (directiveTable[mnemonic].exist) {
            objectCode = directive();
        } else if (opcode.format == 0) {
            objectCode = format0();
        } else if (opcode.format == 1) {
            objectCode = format1();
        } else if (opcode.format == 2) {
            objectCode = format2();
        } else if (opcode.format == 3) {
            objectCode = format3();
        } else if (opcode.format == 4) {
            objectCode = format4();
        }
        return objectCode;
    }

    string parseOperand(int op) {
        string operand = "";
        if (op == 1) {
            operand = operand1;
        }
        if (op == 2) {
            operand = operand2;
        }

        if (operand.empty()) {
            return "";
        }

        if (opcode.format == 3 || opcode.format == 4) {
            if (operand[0] == '@') {
                this->statementType = INDIRECT;
                operand = operand.substr(1, operand.length() - 1);
            } else if (operand[0] == '#') {
                this->statementType = IMMEDIATE;
                operand = operand.substr(1, operand.length() - 1);
            } else {
                this->statementType = DIRECT;
            }

            if (operand[0] == '=') {
                if (this->state == PASS1) {
                    int len = 0;
                    string code = strToObjCode(operand, len);
                    literalBuffer.push_back(LITERAL(operand, code, len));
                }
            }
        }

        return operand;
    }
};

string strToObjCode(string str, int &length) {
    if (str[0] == '=')
        str = str.substr(1, str.length() - 1);
    string objectCode = "";
    if (str[0] == 'X') { // hex
        length = (str.length() - 3) / 2;
        objectCode = str.substr(2, str.length() - 3);
    } else if (str[0] == 'C') { // char
        length = str.length() - 3;
        str = str.substr(2, str.length() - 3);
        stringstream ss;
        for (int i = 0; i < str.length(); i++)
            ss << sethex << (int)str[i];
        string result = ss.str();
        if (result.length() > 6)
            objectCode = result;
        else
            objectCode = string(6 - result.length(), '0') + result;
    } else {
        length = 0;
    }

    return objectCode;
}

} // namespace sicxe