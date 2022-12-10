#pragma once
#include "table.h"
#include "utils.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string BASE = "";
class SICXE {

private:
    bool immediate;
    bool indirect;
    bool extended;
    bool indexed;

    int flag(int n, int i, int x, int b, int p, int e) { return n << 5 | i << 4 | x << 3 | b << 2 | p << 1 | e; }
    string hex(int n, int size)
    {
        stringstream ss;
        ss << sethex << n;
        string result = ss.str();
        return string(size - result.length(), '0') + result;
    }

public:
    int loc;
    int length;
    bool modified;
    string label;
    string mnemonic;
    OPCODE opcode;
    string operand1;
    string operand2;
    string objectCode;
    string block;

    SICXE()
    {
        loc = 0;
        length = 0;
        block = "";
        label = "";
        operand1 = "";
        operand2 = "";
        immediate = false;
        indirect = false;
        extended = false;
        indexed = false;
        modified = false;
    }

    SICXE(int loc, int length, string label, string mnemonic, OPCODE opcode, string operand1, string operand2,
        string objectCode)
    {
        this->loc = loc;
        this->length = length;
        this->label = label;
        this->mnemonic = mnemonic;
        this->opcode = opcode;
        this->operand1 = operand1;
        this->operand2 = operand2;
        this->objectCode = objectCode;
        immediate = false;
        indirect = false;
        extended = false;
        modified = false;

        if (directiveTable[mnemonic].exist) {
            ObjectCode();
        }

        if (operand1.length() > 0 && opcode.format == 3 || opcode.format == 4) {
            this->immediate = operand1[0] == '#';
            this->indirect = operand1[0] == '@';
            if (immediate || indirect)
                this->operand1 = operand1.substr(1, operand1.length() - 1);
            if (opcode.format == 4)
                this->extended = true;
        }

        if (operand2.length() > 0) {
            this->indexed = true;
        } else {
            this->indexed = false;
        }
    }

    string format1() { return hex(opcode.opcode, 2); }

    string format2()
    {
        int obj = opcode.opcode;
        obj <<= 4;
        if (operand1.length() > 0)
            obj += regtable[operand1].num;
        obj <<= 4;
        if (operand2.length() > 0)
            obj += regtable[operand2].num;
        return hex(obj, 4);
    }

    string format3()
    {
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

        if (immediate) {
            if (symboltable[operand1].exist) {
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
                else if (target - base >= 0 && target - base <= 4095) {
                    disp = target - base;
                    obj = optable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(0, 1, 0, 1, 0, 0);
                    obj <<= 12;
                    obj += disp;
                } else {
                    // throw("error: out of range");
                    return "";
                }
            } else {
                // immediate
                disp = stoi(operand1);
                disp = disp & 0b0000111111111111;
                obj = optable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 0, 0);
                obj <<= 12;
                obj += disp;
            }
        }

        else if (indirect) {
            if (symboltable[operand1].exist) {
                target = symboltable[operand1].address;
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
                else if (target - base >= 0 && target - base <= 4095) {
                    disp = target - base;
                    obj = optable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(1, 0, 0, 1, 0, 0);
                    obj <<= 12;
                    obj += disp;
                } else {
                    // throw("error: out of range");
                    return "";
                }
            } else {
                // throw("error: undefined symbol");
                return "";
            }
        }

        else { // simple addressing
            if (symboltable[operand1].exist) {
                target = symboltable[operand1].address;
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
                else if (target - base >= 0 && target - base <= 4095) {
                    disp = target - base;
                    obj = optable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(1, 1, indexed, 1, 0, 0);
                    obj <<= 12;
                    obj += disp;
                } else {
                    // throw("error: out of range");
                    return "";
                }
            } else {
                // throw("error: undefined symbol");
                return "";
            }
        }
        return hex(obj, 6);
    }

    string format4()
    {
        int obj = 0;
        if (immediate) {
            if (symboltable[operand1].exist) {
                int target = symboltable[operand1].address;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 0, 1);
                obj <<= 20;
                obj += target;
            } else {
                int disp = stoi(operand1);
                disp = disp & 0b00000000000011111111111111111111;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 0, 1);
                obj <<= 20;
                obj += disp;
            }
        } else if (indirect) {
            if (symboltable[operand1].exist) {
                int target = symboltable[operand1].address;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(1, 0, 0, 0, 0, 1);
                obj <<= 20;
                obj += target;
            } else {
                // throw("error: undefined symbol");
                return "";
            }
        } else {
            if (symboltable[operand1].exist) {
                int target = symboltable[operand1].address;
                obj = opcode.opcode;
                obj <<= 4;
                obj += flag(1, 1, indexed, 0, 0, 1);
                obj <<= 20;
                obj += target;
            } else {
                // throw("error: undefined symbol");
                return "";
            }
        }

        if (!immediate)
            modified = true;

        return hex(obj, 8);
    }

    string directive()
    {
        if (mnemonic == "START") {
            length = stoi(operand1);
        } else if (mnemonic == "END") {
            length = 0;
        } else if (mnemonic == "BYTE") {
            if (operand1[0] == 'X') { // hex
                length = (operand1.length() - 3) / 2;
                objectCode = operand1.substr(2, operand1.length() - 3);
            } else if (operand1[0] == 'C') { // char
                length = operand1.length() - 3;
                string str = operand1.substr(2, operand1.length() - 3);
                stringstream ss;
                for (int i = 0; i < str.length(); i++)
                    ss << sethex << (int)str[i];
                string result = ss.str();
                objectCode = string(6 - result.length(), '0') + result;

            } else {
                // throw "Invalid operand";
            }
        } else if (mnemonic == "WORD") {
            // TODO: check if operand is a number
            objectCode = string(6 - operand1.length(), '0') + operand1;
            length = 3;
        } else if (mnemonic == "RESB") {
            length = stoi(operand1);
        } else if (mnemonic == "RESW") {
            length = stoi(operand1) * 3;
        } else if (mnemonic == "BASE") {
            BASE = operand1;
        } else if (mnemonic == "NOBASE") {
            BASE = "";
        }
        return objectCode;
    }

    string ObjectCode()
    {
        if (directiveTable[mnemonic].exist) {
            objectCode = directive();
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
};
