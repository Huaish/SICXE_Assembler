#include "../lib/color.h"
#include "../lib/sicxe.h"
#include "../lib/table.h"
#include "../lib/utils.h"
#include <boost/algorithm/string.hpp>
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

string NAME;
int STARTLOC, ENDLOC, STARTEXEC;
vector<string> textList;
vector<int> modList;

enum STEP { LABEL, OPCODE_STEP, OPERAND, END };

SICXE parseLine(string line, int loc, STATE state) {
    string statement = line;
    boost::to_upper(line);
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
                token = token.substr(1, token.length() - 1);
                format4 = true;
            }
            if (optable[token].exist) {
                opcode = optable[token];
                mnemonic = token;
                if (format4) {
                    opcode.format = 4;
                }
                length = opcode.format;
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
                    return SICXE(sicxe::ERROR, sicxe::STATEMENT, statement);
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

    return SICXE(statement, state, loc, length, label, mnemonic, opcode,
                 operand1, operand2, objectCode);
}

void printLine(int line, ofstream &fout, SICXE sourceLine = SICXE(),
               string errorMsg = "") {
    fout << setw(4) << setdec << line * 5 << "  ";

    if (sourceLine.type == sicxe::COMMENT) {
        fout << setw(6) << setfill(' ') << "";
        fout << setw(15) << setfill(' ') << "";
        fout << sourceLine.statement;
    } else {
        if (sourceLine.state == sicxe::PASS2 && sourceLine.mnemonic != "END" &&
            sourceLine.mnemonic != "BASE")
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
            boost::to_upper(tokenUpper);
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
                tokenUpper.erase(
                    remove(tokenUpper.begin(), tokenUpper.end(), '+'),
                    tokenUpper.end());
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
    }

    if (sourceLine.state == sicxe::ERROR) {
        fout << errorMsg;
    }
    fout << sourceLine.objectCode << endl;
}

bool pass1(File input) {
    ifstream fin(input.fullPath);

    string inputStr = "";
    int loc = 0;
    SICXE sourceLine;
    while (getline(fin, inputStr)) {
        if (inputStr[0] == '.') // comment
            continue;
        try {
            sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length,
                                   sicxe::PASS1);
            // loc += sourceLine.length;
            // if (sourceLine.mnemonic == "START") {
            //     NAME = sourceLine.label;
            //     loc = stoi(sourceLine.operand1, nullptr, 16);
            //     STARTLOC = loc;
            // } else if (sourceLine.mnemonic == "END") {
            //     ENDLOC = loc;
            // }
            if (sourceLine.label != "") {
                addSymbol(sourceLine.label, sourceLine.loc);
            }

        } catch (const char *msg) {
            cout << Red << msg << RESET << endl;
            return false;
        }
    }
    fin.close();
    return true;
}

bool pass2(File input, File output) {
    ifstream fin(input.fullPath);
    ofstream fout;
    fout.open(output.dir + "objectCodeTable.txt");

    string inputStr = "";
    int loc = 0;
    SICXE sourceLine;

    int opcode = 0;
    fout << "Line  Location  Source code                          "
            "    Object "
            "code"
         << endl;
    fout << "----  -------- -------------------------             "
            "    "
            "-----------"
         << endl;
    int line = 1;
    while (getline(fin, inputStr)) {
        if (inputStr[0] == '.') { // comment
            printLine(line++, fout, SICXE(PASS2, COMMENT, inputStr));
            continue;
        }
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length,
                               sicxe::PASS2);
        if (sourceLine.mnemonic == "START") {
            // loc = stoi(sourceLine.operand1);
        } else if (sourceLine.mnemonic == "END") {
            printLine(line++, fout, sourceLine);
            for (map<string, LITERAL>::iterator it = literaltable.begin();
                 it != literaltable.end(); it++) {
                LITERAL literal = it->second;
                SICXE literalLine(it->first, PASS2, literal.address,
                                  literal.length, "", "", OPCODE(), "", "",
                                  literal.objCode);
                printLine(line++, fout, literalLine);
                loc += literal.length;
            }
            break;
        }
        // else if (sourceLine.mnemonic == "RESW" ||
        //            sourceLine.mnemonic == "RESB") {
        //     loc = sourceLine.loc + sourceLine.length;
        // }

        printLine(line++, fout, sourceLine);
    }

    fin.close();
    fout.close();

    return true;
}

int main(int argc, char **argv) {
    string fileName = "../src/input.asm";
    bool isPrint = false;
    File input, output;
    if (!getOption(argc, argv, input, output, isPrint))
        return 0;

    cout << Yellow << "Compiling " << fileName << RESET << endl;
    buildOpTable();
    buildRegTable();
    buildDirectiveTable();
    try {
        if (!pass1(input)) {
            cout << Red << "Error: Cannot compile the file" << RESET << endl;
            return 0;
        }
        if (isPrint)
            printFile(output.dir + "symbolTable.txt", "Symbol Table",
                      On_Orange);
        if (!pass2(input, output)) {
            cout << Red << "Error: Cannot compile the file" << RESET << endl;
            return 0;
        }
        if (isPrint)
            printFile(output.dir + "objcode.txt", "Object Code", On_Cyan);

        cout << Green << "Done!" << RESET << endl;
        cout << "Check the output files in the " + output.dir + " folder."
             << endl;
        return 0;
    } catch (const char *msg) {
        cout << Red << msg << RESET << endl;
        return 0;
    }
}