#include "../lib/color.h"
#include "../lib/sicxe.h"
#include "../lib/table.h"
#include "../lib/utils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace sicxe;

#define no_argument 0
#define required_argument 1
#define optional_argument 2

enum STEP { LABEL, OPCODE_STEP, OPERAND, END };
int MODE = 1; // 0: SIC, 1: SIC/XE

SICXE parseLine(string line, int loc, STATE state) {
    string statement = line;
    transform(line.begin(), line.end(), line.begin(), ::toupper);
    int length = 0;
    string label = "", mnemonic = "";
    struct OPCODE opcode;
    string operand1 = "", operand2 = "", objectCode = "";
    vector<string> tokens = getTokens(line, ' ');

    string token = "";
    STEP step = OPCODE_STEP;

    while (!tokens.empty() && step != END) {
        token = tokens.front();
        tokens.erase(tokens.begin());
        switch (step) {
        case OPCODE_STEP: {
            bool format4 = false;
            if (token[0] == '+') {
                if (MODE == 0) {
                    string errorMsg = "ERROR(Invalid mnemonic): " + token;
                    cout << Red << errorMsg << RESET << endl;
                    SICXE result = SICXE(sicxe::ERROR, sicxe::STATEMENT, statement);
                    result.errorMsg = errorMsg;
                    return result;
                }
                token = token.substr(1, token.length() - 1);
                format4 = true;
            }
            if (optable[token].exist) {
                opcode = optable[token];
                mnemonic = token;
                if (MODE == 0) {
                    opcode.format = 0;
                    length = 3;
                } else {
                    if (format4) {
                        opcode.format = 4;
                    }
                    length = opcode.format;
                }
                step = OPERAND;
                break;
            } else if (directiveTable[token].exist) {
                mnemonic = token;
                step = OPERAND;
                break;
            } else {
                if (label.empty()) {
                    step = LABEL;
                } else {
                    string errorMsg = "ERROR(Invalid mnemonic): " + token;
                    cout << Red << errorMsg << RESET << endl;
                    SICXE result = SICXE(sicxe::ERROR, sicxe::STATEMENT, statement);
                    result.errorMsg = errorMsg;
                    return result;
                }
            }
        }
        case LABEL:
            label = token;
            step = OPCODE_STEP;
            break;
        case OPERAND: {
            vector<string> operands = getTokens(token, ',');
            if (operands.size() == 1) {
                operand1 = operands[0];
            } else if (operands.size() == 2) {
                operand1 = operands[0];
                operand2 = operands[1];
            }
            step = END;
            break;
        }
        default:
            step = END;
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

    if (sourceLine.type == sicxe::COMMENT) {
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
        STEP step = LABEL;
        while (!tokens.empty()) {
            string token = tokens.front();
            string tokenUpper = tokens.front();
            tokens.erase(tokens.begin());
            transform(tokenUpper.begin(), tokenUpper.end(), tokenUpper.begin(), ::toupper);
            switch (step) {
            case LABEL:
                step = OPCODE_STEP;
                if (symboltable[tokenUpper].exist) {
                    fout << setw(15) << left << setfill(' ') << token;
                    break;
                } else
                    fout << setw(15) << left << setfill(' ') << "";
            case OPCODE_STEP:
                step = OPERAND;
                tokenUpper.erase(remove(tokenUpper.begin(), tokenUpper.end(), '+'), tokenUpper.end());
                if (optable[tokenUpper].exist) {
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

        if (sourceLine.state == sicxe::ERROR) {
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
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length, sicxe::PASS1);

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
            printLine(line++, fout, SICXE(PASS2, COMMENT, inputStr));
            continue;
        }
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length, sicxe::PASS2);
        if (sourceLine.mnemonic == "END" || sourceLine.mnemonic == "LTORG") {
            printLine(line++, fout, sourceLine);
            for (map<string, LITERAL>::iterator it = literaltable.begin(); it != literaltable.end(); it++) {
                LITERAL literal = it->second;
                if (literal.exist == false || literal.isPrinted == true)
                    continue;
                literaltable[it->first].isPrinted = true;
                SICXE literalLine(it->first, PASS2, literal.address, literal.length, "", "", OPCODE(), "", "", literal.objCode);
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
    bool isPrint = false;
    File input, output;
    if (!getOption(argc, argv, input, output, isPrint, MODE))
        return 0;

    cout << Yellow << "Compiling " << input.name << RESET << endl;

    // Initialize
    buildOpTable();
    buildRegTable();
    buildDirectiveTable();
    try {
        pass1(input);
        pass2(input, output);

        if (isPrint)
            printFile(output.fullPath, output.fullPath, On_Cyan);

        cout << Green << "Done!" << RESET << endl;
        cout << "Check the output in the " + output.fullPath << endl;
        return 0;
    } catch (const char *msg) {
        cout << Red << msg << RESET << endl;
        return 0;
    }
}