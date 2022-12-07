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

void printLine(ofstream& fout, Source sourceLine)
{
    fout << setw(4) << setfill('0') << sethex << sourceLine.loc;
    fout << setw(4) << setfill(' ') << "";
    fout << setw(8) << setfill(' ') << left << sourceLine.block;
    fout << setw(8) << setfill(' ') << left << sourceLine.label;
    fout << setw(8) << setfill(' ') << left << sourceLine.mnemonic;
    fout << setw(12) << setfill(' ') << left
         << sourceLine.operand1 + (sourceLine.operand2.length() > 0 ? "," : " ") + sourceLine.operand2;
    fout << setw(8) << setfill(' ') << sethex << sourceLine.objectCode;
    fout << endl;
}

Source parseLine(string line, int loc)
{
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
                loc = loc + opcode.format;
                step = "operand";
            } else if (directiveTable[token].exist) {
                string operand = tokens.empty() ? "" : tokens.front();
                mnemonic = token;
                operand1 = operand;
                loc = loc + handleDirective(token, operand, objectCode);
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

    return Source(loc, label, mnemonic, opcode, operand1, operand2, objectCode);
}

void pass1(string fileName)
{
    ifstream fin;
    fin.open(fileName);
    ofstream fout;
    fout.open("pass1.txt");

    string inputStr = "";
    int loc = 0;
    while (getline(fin, inputStr)) {
        boost::to_upper(inputStr);
        Source sourceLine;
        try {
            sourceLine = parseLine(inputStr, loc);
            loc = sourceLine.loc;
            if (sourceLine.mnemonic == "START")
                NAME = sourceLine.label;
            if (sourceLine.label != "")
                addSymbol(sourceLine.label, sourceLine.loc);
        } catch (const char* msg) {
            cout << Red << msg << RESET << endl;
            return;
        }
    }
    fin.close();
    fout.close();
}

void pass2(string fileName)
{
    ifstream fin2;
    fin2.open(fileName);
    ofstream fout2;
    fout2.open("pass2.txt");

    string inputStr = "";
    int loc = 0;

    int opcode = 0;
    fout2 << "Loc\t\tBlock\t\t\tSource Statement\tObject Code" << endl;
    fout2 << "----------------------------------------------------------------" << endl;
    while (getline(fin2, inputStr)) {
        boost::to_upper(inputStr);
        Source sourceLine = parseLine(inputStr, loc);
        loc = sourceLine.loc;
        if (optable[sourceLine.mnemonic].exist) {
            sourceLine.ObjectCode();
        }
        printLine(fout2, sourceLine);
    }
    fin2.close();
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