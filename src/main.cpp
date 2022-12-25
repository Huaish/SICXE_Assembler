#include "../lib/color.h"
#include "../lib/sicxe.h"
#include "../lib/table.h"
#include "../lib/utils.h"
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

#define no_argument 0
#define required_argument 1
#define optional_argument 2

string NAME;
int STARTLOC, ENDLOC, STARTEXEC;
vector<string> textList;
vector<int> modList;

SICXE parseLine(string line, int loc)
{
    int length = 0;
    string label = "", mnemonic = "";
    OPCODE opcode;
    string operand1 = "", operand2 = "", objectCode = "";
    vector<string> tokens = getTokens(line, ' ');

    string token = "";
    string step = "opcode";
    while (!tokens.empty() && step != "end") {
        if (step == "label") {
            label = token;
            step = "opcode";
        } else if (step == "opcode") {
            token = tokens.front();
            tokens.erase(tokens.begin());

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
                step = "operand";
            } else if (directiveTable[token].exist) {
                mnemonic = token;
                step = "operand";
            } else {
                if (label.empty()) {
                    step = "label";
                } else {
                    throw "Invalid opcode";
                    return SICXE();
                }
            }
        } else if (step == "operand") {
            token = tokens.front();
            tokens.erase(tokens.begin());
            vector<string> operands = getTokens(token, ',');
            if (operands.size() == 1) {
                operand1 = operands[0];
            } else if (operands.size() == 2) {
                operand1 = operands[0];
                operand2 = operands[1];
            }
            step = "end";
        }
    }

    return SICXE(loc, length, label, mnemonic, opcode, operand1, operand2, objectCode);
}

void printLine(ofstream& fout, SICXE sourceLine)
{
    fout << setw(4) << setfill('0') << sethex << sourceLine.loc;
    fout << setw(4) << setfill(' ') << "";
    fout << setw(8) << setfill(' ') << left << sourceLine.block;
    fout << setw(8) << setfill(' ') << left << sourceLine.label;
    fout << setw(8) << setfill(' ') << left << sourceLine.mnemonic;
    fout << setw(12) << setfill(' ') << left
         << sourceLine.operand1 + (sourceLine.operand2.length() > 0 ? "," : " ") + sourceLine.operand2;
    fout << setw(8) << setfill(' ') << sethex << left << sourceLine.objectCode;
    fout << endl;
}

string genObjCode(char record, int loc)
{

    stringstream ss;
    if (record == 'H') {
        ss << record << "^";
        ss << setw(6) << setfill(' ') << left << NAME << "^";
        ss << setw(6) << setfill('0') << sethex << STARTLOC << "^";
        ss << setw(6) << setfill('0') << sethex << ENDLOC - STARTLOC << endl;
        return ss.str();
    }

    if (record == 'M') {
        for (int i = 0; i < modList.size(); i++) {
            ss << "M^";
            ss << setw(6) << setfill('0') << sethex << modList[i] + 1;
            ss << "^05" << endl;
        }
        return ss.str();
    }

    if (record == 'T') {
        if (textList.empty())
            return "";
        int size = 0;
        string objCode = "";
        for (int i = 0; i < textList.size(); i++) {
            if (textList[i] == "")
                continue;
            size += textList[i].length() / 2;
            objCode += "^" + textList[i];
        }
        ss << record << "^";
        ss << setw(6) << setfill('0') << sethex << loc << "^";
        ss << setw(2) << setfill('0') << sethex << size;
        ss << objCode << endl;
        return ss.str();
    }

    if (record == 'E') {
        ss << record << "^";
        ss << setw(6) << setfill('0') << sethex << STARTEXEC << endl;
        return ss.str();
    }

    return "";
}

bool pass1(File input, File output)
{
    ifstream fin(input.fullPath);
    if (!output.create_directories()) {
        cout << "Error: Cannot create output directory" << endl;
        return false;
    }
    ofstream fout(output.dir + "symbolTable.txt");

    fout << "Name\t\tValue" << endl;
    string inputStr = "";
    int loc = 0;
    while (getline(fin, inputStr)) {
        boost::to_upper(inputStr);
        if (inputStr[0] == '.') // comment
            continue;
        SICXE sourceLine;
        try {
            sourceLine = parseLine(inputStr, loc);
            loc += sourceLine.length;
            if (sourceLine.mnemonic == "START") {
                NAME = sourceLine.label;
                loc = stoi(sourceLine.operand1);
                STARTLOC = loc;
            } else if (sourceLine.mnemonic == "END") {
                ENDLOC = loc;
            }
            if (sourceLine.label != "") {
                addSymbol(sourceLine.label, sourceLine.loc);
                fout << sourceLine.label << "\t\t" << sethex << sourceLine.loc << endl;
            }

        } catch (const char* msg) {
            cout << Red << msg << RESET << endl;
            return false;
        }
    }
    fin.close();
    fout.close();
    return true;
}

bool pass2(File input, File output)
{
    ifstream fin2(input.fullPath);
    ofstream fout1, fout2;
    fout1.open(output.dir + "objcode.txt");
    fout2.open(output.dir + "objectCodeTable.txt");

    string inputStr = "";
    int loc = 0;
    SICXE sourceLine;

    int opcode = 0;
    fout2 << "Loc\t\tBlock\t\t\tSICXE Statement\tObject Code" << endl;
    fout2 << "----------------------------------------------------------------" << endl;
    while (getline(fin2, inputStr)) {
        boost::to_upper(inputStr);
        if (inputStr[0] == '.') // comment
            continue;
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length);
        if (sourceLine.mnemonic == "START") {
            loc = stoi(sourceLine.operand1);
            fout1 << genObjCode('H', loc);
        } else if (sourceLine.mnemonic == "END") {
            if (textList.size() > 0) {
                fout1 << genObjCode('T', loc);
                textList.clear();
            }

            if (modList.size() > 0) {
                fout1 << genObjCode('M', loc);
                modList.clear();
            }

            if (symboltable[sourceLine.operand1].exist) {
                STARTEXEC = symboltable[sourceLine.operand1].address;
                fout1 << genObjCode('E', loc);
            } else {
                // throw("Invalid operand");
                return false;
            }
        } else if (sourceLine.mnemonic == "RESW" || sourceLine.mnemonic == "RESB") {
            fout1 << genObjCode('T', loc);
            textList.clear();
            loc = sourceLine.loc + sourceLine.length;
        } else {
            if (!directiveTable[sourceLine.mnemonic].exist)
                sourceLine.ObjectCode();
            if (sourceLine.objectCode.length() > 0) {
                textList.push_back(sourceLine.objectCode);
                if (textList.size() == 10) {
                    fout1 << genObjCode('T', loc);
                    textList.clear();
                    loc = sourceLine.loc + sourceLine.length;
                }
            }

            if (sourceLine.modified) {
                modList.push_back(sourceLine.loc);
            }
        }

        printLine(fout2, sourceLine);
    }

    fin2.close();
    fout1.close();
    fout2.close();

    return true;
}

int main(int argc, char** argv)
{
    string fileName = "../src/input.asm";
    bool isPrint = false;
    File input, output;
    if (!getOption(argc, argv, input, output, isPrint))
        return 0;

    cout << Yellow << "Compiling " << fileName << RESET << endl;
    buildOpTable();
    buildRegTable();
    buildDirectiveTable();
    if (!pass1(input, output)) {
        cout << Red << "Error: Cannot compile the file" << RESET << endl;
        return 0;
    }
    if (isPrint)
        printFile(output.dir + "symbolTable.txt", "Symbol Table", On_Orange);
    if (!pass2(input, output)) {
        cout << Red << "Error: Cannot compile the file" << RESET << endl;
        return 0;
    }
    if (isPrint)
        printFile(output.dir + "objcode.txt", "Object Code", On_Cyan);

    cout << Green << "Done!" << RESET << endl;
    cout << "Check the output files in the " + output.dir + " folder." << endl;
    return 0;
}