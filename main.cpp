#include "SICXE.h"
#include "color.h"
#include "table.h"
#include "utils.h"
#include <boost/algorithm/string.hpp>
#include <cctype>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
using namespace std;

string NAME;
int STARTLOC, ENDLOC, STARTEXEC;

Source parseLine(string line, int loc)
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
                string operand = tokens.empty() ? "" : tokens.front();
                mnemonic = token;
                operand1 = operand;
                length = handleDirective(token, operand, objectCode);
                step = "end";
            } else {
                if (label.empty()) {
                    step = "label";
                } else {
                    throw "Invalid opcode";
                    return Source();
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

    return Source(loc, length, label, mnemonic, opcode, operand1, operand2, objectCode);
}

void printLine(ofstream& fout, Source sourceLine)
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

string genObjCode(char record, int loc, vector<string> objList)
{

    stringstream ss;
    if (record == 'H') {
        ss << record << "^";
        ss << setw(6) << setfill(' ') << left << NAME << "^";
        ss << setw(6) << setfill('0') << sethex << STARTLOC << "^";
        ss << setw(6) << setfill('0') << sethex << ENDLOC - STARTLOC << endl;
        return ss.str();
    } else if (record == 'E') {
        ss << record << "^";
        ss << setw(6) << setfill('0') << sethex << STARTEXEC << endl;
        return ss.str();
    }

    if (objList.size() == 0)
        return "";

    int size = 0;
    string objCode = "";
    for (int i = 0; i < objList.size(); i++) {
        if (objList[i] == "")
            continue;
        size += objList[i].length() / 2;
        objCode += "^" + objList[i];
    }

    ss << record << "^";
    ss << setw(6) << setfill('0') << sethex << loc << "^";
    ss << setw(2) << setfill('0') << sethex << size;
    ss << objCode << endl;

    return ss.str();
}

void pass1(string fileName)
{
    ifstream fin;
    fin.open(fileName);
    ofstream fout;

    string inputStr = "";
    int loc = 0;
    while (getline(fin, inputStr)) {
        boost::to_upper(inputStr);
        if (inputStr[0] == '.') // comment
            continue;
        Source sourceLine;
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
            if (sourceLine.label != "")
                addSymbol(sourceLine.label, sourceLine.loc);

        } catch (const char* msg) {
            cout << Red << msg << RESET << endl;
            return;
        }
    }
    fin.close();
}

void pass2(string fileName)
{
    ifstream fin2;
    fin2.open(fileName);
    ofstream fout1, fout2;
    fout1.open("objcode.txt");
    fout2.open("pass2.txt");

    vector<string> objList;

    string inputStr = "";
    int loc = 0;
    Source sourceLine;

    int opcode = 0;
    fout2 << "Loc\t\tBlock\t\t\tSource Statement\tObject Code" << endl;
    fout2 << "----------------------------------------------------------------" << endl;
    while (getline(fin2, inputStr)) {
        boost::to_upper(inputStr);
        if (inputStr[0] == '.') // comment
            continue;
        sourceLine = parseLine(inputStr, sourceLine.loc + sourceLine.length);
        if (sourceLine.mnemonic == "START") {
            loc = stoi(sourceLine.operand1);
            fout1 << genObjCode('H', loc, objList);
        } else if (sourceLine.mnemonic == "END") {
            if (objList.size() > 0) {
                fout1 << genObjCode('T', loc, objList);
                objList.clear();
            }

            if (symboltable[sourceLine.operand1].exist) {
                STARTEXEC = symboltable[sourceLine.operand1].address;
                fout1 << genObjCode('E', loc, objList);
            } else {
                // throw("Invalid operand");
            }
        } else if (sourceLine.mnemonic == "RESW" || sourceLine.mnemonic == "RESB") {
            fout1 << genObjCode('T', loc, objList);
            objList.clear();
            loc = sourceLine.loc + sourceLine.length;
        } else {
            if (!directiveTable[sourceLine.mnemonic].exist)
                sourceLine.ObjectCode();
            if (sourceLine.objectCode.length() > 0) {
                objList.push_back(sourceLine.objectCode);
                if (objList.size() == 10) {
                    fout1 << genObjCode('T', loc, objList);
                    objList.clear();
                    loc = sourceLine.loc + sourceLine.length;
                }
            }
        }

        printLine(fout2, sourceLine);
    }
    fin2.close();
    fout1.close();
    fout2.close();
}

int main()
{
    buildOpTable();
    buildRegTable();
    buildDirectiveTable();
    pass1("input.txt");
    printSymbolTable();
    pass2("input.txt");
    return 0;
}