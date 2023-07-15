#include "lib/color.h"
#include "lib/sicxe.h"
#include "lib/table.h"
#include "lib/utils.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;

enum STEP { STEP_LABEL, STEP_OPCODE, STEP_OPERAND, STEP_END };
enum MODE { SIC_MODE, SICXE_MODE };
int mode = SICXE_MODE;

SICXE parseLine(string line, int loc, STATE state) {
    string statement = line;
    transform(line.begin(), line.end(), line.begin(), ::toupper);
    int length = 0;
    string label = "", mnemonic = "";
    struct OPCODE opcode;
    string operand1 = "", operand2 = "", objectCode = "";
    vector<string> tokens = getTokens(line, ' ');

    string token = "";
    STEP step = STEP_OPCODE;

    while (!tokens.empty() && step != STEP_END) {
        token = tokens.front();
        tokens.erase(tokens.begin());
        switch (step) {
        case STEP_OPCODE: {
            bool format4 = false;
            if (token[0] == '+') {
                if (mode == SIC_MODE) {
                    string errorMsg = "ERROR(Invalid mnemonic): " + token;
                    cout << Red << errorMsg << RESET << endl;
                    SICXE result = SICXE(STATE_ERROR, TYPE_STATEMENT, statement);
                    result.errorMsg = errorMsg;
                    return result;
                }
                token = token.substr(1, token.length() - 1);
                format4 = true;
            }
            if (opTable[token].exist) {
                opcode = opTable[token];
                mnemonic = token;
                if (mode == SIC_MODE) {
                    opcode.format = 0;
                    length = 3;
                } else {
                    if (format4) {
                        opcode.format = 4;
                    }
                    length = opcode.format;
                }
                step = STEP_OPERAND;
                break;
            } else if (directiveTable[token].exist) {
                mnemonic = token;
                step = STEP_OPERAND;
                break;
            } else {
                if (label.empty()) {
                    step = STEP_LABEL;
                } else {
                    string errorMsg = "ERROR(Invalid mnemonic): " + token;
                    cout << Red << errorMsg << RESET << endl;
                    SICXE result = SICXE(STATE_ERROR, TYPE_STATEMENT, statement);
                    result.errorMsg = errorMsg;
                    return result;
                }
            }
        }
        case STEP_LABEL:
            label = token;
            step = STEP_OPCODE;
            break;
        case STEP_OPERAND: {
            vector<string> operands = getTokens(token, ',');
            if (operands.size() == 1) {
                operand1 = operands[0];
            } else if (operands.size() == 2) {
                operand1 = operands[0];
                operand2 = operands[1];
            }
            step = STEP_END;
            break;
        }
        default:
            step = STEP_END;
            break;
        }
    }

    return SICXE(statement, state, loc, length, label, mnemonic, opcode, operand1, operand2, objectCode);
}

void printLine(int line, ofstream &fout, SICXE sourceLine = SICXE(), string errorMsg = "") {
    if (sourceLine.statement == "") {
        fout << endl;
        return;
    }
    fout << setw(4) << setdec << line * 5 << "  ";

    if (sourceLine.type == TYPE_COMMENT) {
        fout << setw(6) << setfill(' ') << "";
        fout << setw(15) << setfill(' ') << "";
        fout << sourceLine.statement << endl;
    } else {
        if (sourceLine.mnemonic != "END" && sourceLine.mnemonic != "BASE")
            fout << setw(4) << setfill('0') << sethex << sourceLine.loc << "  ";
        else
            fout << setw(6) << setfill(' ') << "";

        vector<string> tokens = getTokens(sourceLine.statement, ' ');
        string statement = "";
        STEP step = STEP_LABEL;
        while (!tokens.empty()) {
            string token = tokens.front();
            string tokenUpper = tokens.front();
            tokens.erase(tokens.begin());
            transform(tokenUpper.begin(), tokenUpper.end(), tokenUpper.begin(), ::toupper);
            switch (step) {
            case STEP_LABEL:
                step = STEP_OPCODE;
                if (symbolTable[tokenUpper].exist) {
                    fout << setw(15) << left << setfill(' ') << token;
                    break;
                } else
                    fout << setw(15) << left << setfill(' ') << "";
            case STEP_OPCODE:
                step = STEP_OPERAND;
                tokenUpper.erase(remove(tokenUpper.begin(), tokenUpper.end(), '+'), tokenUpper.end());
                if (opTable[tokenUpper].exist) {
                    fout << setw(10) << left << setfill(' ') << token;
                    break;
                } else if (directiveTable[tokenUpper].exist) {
                    fout << setw(10) << left << setfill(' ') << token;
                    break;
                }
            default:
                statement += token + " ";
                break;
            }
        }

        fout << setw(20) << left << setfill(' ') << statement;

        if (sourceLine.state == STATE_ERROR) {
            fout << sourceLine.errorMsg << endl;
        } else {
            fout << sourceLine.objectCode << endl;
        }
    }
}

bool pass1(File input) {
    ifstream fin(input.fullPath);

    string inputStr = "";
    int loc = 0;
    SICXE sourceLine;
    while (getline(fin, inputStr)) {
        if (inputStr[0] == '.') // comment
            continue;
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length, STATE_PASS1);

        if (sourceLine.label != "") {
            addSymbol(sourceLine.label, sourceLine.loc);
        }
    }
    fin.close();
    return true;
}

bool pass2(File input, File output) {
    ifstream fin(input.fullPath);
    ofstream fout(output.fullPath);

    string inputStr = "";
    int loc = 0;
    SICXE sourceLine;

    int opcode = 0;
    fout << "Line  Location  Source code                              Object code" << endl;
    fout << "----  -------- -------------------------                 -----------" << endl;
    int line = 1;
    while (getline(fin, inputStr)) {
        if (inputStr[0] == '.') { // comment
            printLine(line++, fout, SICXE(STATE_PASS2, TYPE_COMMENT, inputStr));
            continue;
        }
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length, STATE_PASS2);
        if (sourceLine.mnemonic == "END" || sourceLine.mnemonic == "LTORG") {
            printLine(line++, fout, sourceLine);
            for (map<string, LITERAL>::iterator it = literalTable.begin(); it != literalTable.end(); it++) {
                LITERAL literal = it->second;
                if (literal.exist == false || literal.isPrinted == true)
                    continue;
                literalTable[it->first].isPrinted = true;
                SICXE literalLine(it->first, STATE_PASS2, literal.address, literal.length, "", "", OPCODE(), "", "", literal.objCode);
                printLine(line++, fout, literalLine);
                loc += literal.length;
            }
            continue;
        }

        printLine(line++, fout, sourceLine);
    }

    fin.close();
    fout.close();

    return true;
}

int main(int argc, char **argv) {
    File input, output;
    if (!getOption(argc, argv, input, output, mode))
        return 0;

    cout << Yellow << "Compiling " << input.name << RESET << endl;

    // Initialize
    buildOpTable();
    buildRegTable();
    buildDirectiveTable();
    try {
        pass1(input);
        pass2(input, output);

        cout << Green << "Done!" << RESET << endl;
        cout << "Check the output in the " + output.fullPath << endl;
        return 0;
    } catch (const char *msg) {
        cout << Red << msg << RESET << endl;
        return 0;
    }
}