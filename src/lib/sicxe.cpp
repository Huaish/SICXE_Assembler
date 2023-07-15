#include "sicxe.h"

using namespace std;

string BASE = "";
vector<LITERAL> literalBuffer;

int SICXE::flag(int n, int i, int x, int b, int p, int e) { return n << 5 | i << 4 | x << 3 | b << 2 | p << 1 | e; }

string SICXE::hex(int n, int size) {
    stringstream ss;
    ss << sethex << n;
    string result = ss.str();
    return string(size - result.length(), '0') + result;
}

void SICXE::initial() {
    this->state = STATE_EMPTY;
    this->type = TYPE_COMMENT;
    this->statementType = NONE_ADDRESSING;
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

SICXE::SICXE() { initial(); }

SICXE::SICXE(STATE state, TYPE type, string statement) {
    initial();
    this->state = state;
    this->type = type;
    this->statementType = NONE_ADDRESSING;
    this->statement = statement;
}

SICXE::SICXE(string statement, STATE state, int loc, int length, string label, string mnemonic, OPCODE opcode, string operand1, string operand2,
             string objectCode) {
    initial();
    this->statement = statement;
    this->state = state;
    this->type = TYPE_STATEMENT;

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
        this->type = TYPE_DIRECTIVE;
        ObjectCode();
    }

    if (state == STATE_PASS2 && opTable[this->mnemonic].exist) {
        ObjectCode();
    }
}

string SICXE::format0() {
    if (opcode.flagX) {
        state = STATE_ERROR;
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
        else if (symbolTable[operand1].exist)
            obj += symbolTable[operand1].address;
        else if (literalTable[operand1].exist)
            obj += literalTable[operand1].address;
    }

    return hex(obj, 6);
}

string SICXE::format1() { return hex(opcode.opcode, 2); }

string SICXE::format2() {
    int obj = opcode.opcode;
    obj <<= 4;
    if (this->operand1.length() > 0) {
        if (isDecimal(operand1))
            obj += stoi(operand1);
        else if (regTable[operand1].exist)
            obj += regTable[operand1].num;
        else {
            state = STATE_ERROR;
            errorMsg = "ERROR(Invalid register): " + operand1;
            return "";
        }
    }
    obj <<= 4;
    if (this->operand2.length() > 0) {
        if (isDecimal(operand2))
            obj += stoi(operand2);
        else if (regTable[operand2].exist)
            obj += regTable[operand2].num;
        else {
            state = STATE_ERROR;
            errorMsg = "ERROR(Invalid register): " + operand2;
            return "";
        }
    }
    return hex(obj, 4);
}

string SICXE::format3() {
    int obj = 0;
    int disp = 0;
    int pc = loc + 3;
    int base = symbolTable[BASE].address;
    int target = 0;
    if (opcode.numOfOperands == 0) {
        obj = opcode.opcode;
        obj <<= 4;
        obj += flag(1, 1, 0, 0, 0, 0);
        obj <<= 12;
        return hex(obj, 6);
    }

    if (statementType == IMMEDIATE_ADDRESSING) {
        if (symbolTable[operand1].exist) {
            if (symbolTable[operand1].exist == false) {
                state = STATE_ERROR;
                errorMsg = "ERROR(Invalid operand): " + operand1;
                return "";
            }
            target = symbolTable[operand1].address;
            // pc relative
            if (target - pc >= -2048 && target - pc <= 2047) {
                disp = target - pc;
                disp = disp & 0b0000111111111111;
                obj = opTable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(0, 1, 0, 0, 1, 0);
                obj <<= 12;
                obj += disp;
            }
            // base relative
            else {
                if (target - base >= 0 && target - base <= 4095) {
                    disp = target - base;
                    obj = opTable[mnemonic].opcode;
                    obj <<= 4;
                    obj += flag(0, 1, 0, 1, 0, 0);
                    obj <<= 12;
                    obj += disp;
                } else {
                    state = STATE_ERROR;
                    errorMsg = "ERROR(Out of range): " + operand1;
                    return "";
                }
            }
        } else {
            // immediate
            if (isDecimal(operand1) == false) {
                state = STATE_ERROR;
                errorMsg = "ERROR(Invalid operand): " + operand1;
                return "";
            }

            disp = stoi(operand1);
            disp = disp & 0b0000111111111111;
            obj = opTable[mnemonic].opcode;
            obj <<= 4;
            obj += flag(0, 1, 0, 0, 0, 0);
            obj <<= 12;
            obj += disp;
        }
    } else if (statementType == INDIRECT_ADDRESSING) {
        if (symbolTable[operand1].exist) {
            target = symbolTable[operand1].address;
        } else if (literalTable[operand1].exist) {
            target = literalTable[operand1].address;
        } else if (isDecimal(operand1)) {
            disp = stoi(operand1);
            disp = disp & 0b0000111111111111;
            obj = opTable[mnemonic].opcode;
            obj <<= 4;
            obj += flag(0, 0, 0, 0, 0, 0);
            obj <<= 12;
            obj += disp;
            return hex(obj, 6);
        } else {
            state = STATE_ERROR;
            errorMsg = "ERROR(Invalid operand): " + operand1;
            return "";
        }

        // pc relative
        if (target - pc >= -2048 && target - pc <= 2047) {
            disp = target - pc;
            disp = disp & 0b0000111111111111;
            obj = opTable[mnemonic].opcode;
            obj <<= 4;
            obj += flag(1, 0, 0, 0, 1, 0);
            obj <<= 12;
            obj += disp;
        }
        // base relative
        else {
            if (target - base >= 0 && target - base <= 4095) {
                disp = target - base;
                obj = opTable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(1, 0, 0, 1, 0, 0);
                obj <<= 12;
                obj += disp;
            } else {
                // throw("error: out of range");
                state = STATE_ERROR;
                errorMsg = "ERROR(out of range)";
                return "";
            }
        }
    } else if (statementType == DIRECT_ADDRESSING) { // simple addressing
        if (symbolTable[operand1].exist) {
            target = symbolTable[operand1].address;
        } else if (literalTable[operand1].exist) {
            target = literalTable[operand1].address;
        } else if (isDecimal(operand1)) {
            disp = stoi(operand1);
            disp = disp & 0b0000111111111111;
            obj = opTable[mnemonic].opcode;
            obj <<= 4;
            obj += flag(1, 1, this->indexed, 0, 0, 0);
            obj <<= 12;
            obj += disp;
            return hex(obj, 6);
        } else {
            state = STATE_ERROR;
            errorMsg = "ERROR(undefined symbol): " + operand1;
            return "";
        }

        // pc relative
        if (target - pc >= -2048 && target - pc <= 2047) {
            disp = target - pc;
            disp = disp & 0b0000111111111111;
            obj = opTable[mnemonic].opcode;
            obj <<= 4;
            obj += flag(1, 1, indexed, 0, 1, 0);
            obj <<= 12;
            obj += disp;
        }
        // base relative
        else {
            if (target - base >= 0 && target - base <= 4095) {
                disp = target - base;
                obj = opTable[mnemonic].opcode;
                obj <<= 4;
                obj += flag(1, 1, indexed, 1, 0, 0);
                obj <<= 12;
                obj += disp;
            } else {
                state = STATE_ERROR;
                errorMsg = "ERROR(out of range)";
                return "";
            }
        }
    } else {
        return "";
    }

    return hex(obj, 6);
}

string SICXE::format4() {
    int obj = 0;
    if (statementType == IMMEDIATE_ADDRESSING) {
        if (symbolTable[operand1].exist) {
            int target = symbolTable[operand1].address;
            obj = opcode.opcode;
            obj <<= 4;
            obj += flag(0, 1, 0, 0, 0, 1);
            obj <<= 20;
            obj += target;
        } else {
            if (isDecimal(operand1) == false) {
                state = STATE_ERROR;
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
    } else if (statementType == INDIRECT_ADDRESSING) {
        if (symbolTable[operand1].exist) {
            int target = symbolTable[operand1].address;
            obj = opcode.opcode;
            obj <<= 4;
            obj += flag(1, 0, 0, 0, 0, 1);
            obj <<= 20;
            obj += target;
        } else {
            state = STATE_ERROR;
            errorMsg = "ERROR(undefined symbol): " + operand1;
            return "";
        }
    } else if (statementType == DIRECT_ADDRESSING) { // simple addressing
        int target = 0;
        if (symbolTable[operand1].exist) {
            target = symbolTable[operand1].address;
        } else if (literalTable[operand1].exist) {
            target = literalTable[operand1].address;
        } else {
            state = STATE_ERROR;
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

string SICXE::directive() {
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
            state = STATE_ERROR;
            errorMsg = "ERROR(Invalid operand): " + operand1;
            return "";
        }
    } else if (mnemonic == "RESB") {
        length = stoi(operand1);
    } else if (mnemonic == "RESW") {
        length = stoi(operand1) * 3;
    } else if (mnemonic == "BASE") {
        if (opcode.format == 0) {
            state = STATE_ERROR;
            errorMsg = "ERROR(undefined symbol): " + operand1;
            return "";
        }
        BASE = operand1;
    } else if (mnemonic == "NOBASE") {
        if (opcode.format == 0) {
            state = STATE_ERROR;
            errorMsg = "ERROR(undefined symbol): " + operand1;
            return "";
        }
        BASE = "";
    } else if (mnemonic == "EQU") {
        if (operand1 == "*") {

        } else {
            if (symbolTable[operand1].exist) {
                int tmp = this->loc - symbolTable[operand1].address;
                this->length += tmp;
                this->loc = symbolTable[operand1].address;
            } else if (isDecimal(operand1)) {
                int tmp = this->loc - stoi(operand1);
                this->length += tmp;
                this->loc = stoi(operand1);
            } else {
                state = STATE_ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
        }
    } else if (mnemonic == "LTORG") {
        if (opcode.format == 0) {
            state = STATE_ERROR;
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
            if (symbolTable[operand1].exist) {
                this->loc = symbolTable[operand1].address;
                this->length = 0;
            } else if (isDecimal(operand1)) {
                this->loc = stoi(operand1);
                this->length = 0;
            } else {
                state = STATE_ERROR;
                errorMsg = "ERROR(undefined symbol): " + operand1;
                return "";
            }
        }
    }
    return this->objectCode;
}

string SICXE::ObjectCode() {
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

string SICXE::parseOperand(int op) {
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
            this->statementType = INDIRECT_ADDRESSING;
            operand = operand.substr(1, operand.length() - 1);
        } else if (operand[0] == '#') {
            this->statementType = IMMEDIATE_ADDRESSING;
            operand = operand.substr(1, operand.length() - 1);
        } else {
            this->statementType = DIRECT_ADDRESSING;
        }

        if (operand[0] == '=') {
            if (this->state == STATE_PASS1) {
                int len = 0;
                string code = strToObjCode(operand, len);
                literalBuffer.push_back(LITERAL(operand, code, len));
            }
        }
    }

    return operand;
}

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