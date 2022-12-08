#include "utils.h"
#include "table.h"
#include <fstream>

using namespace std;

string BASE = "";

ostream& sethex(ostream& stream)
{
    stream.unsetf(ios::dec | ios::oct);
    stream.setf(ios::hex | ios::uppercase | ios::right);

    return stream;
}

vector<string> getTokens(string str, char delim)
{
    str = regex_replace(str, regex("[\r\n]"), "");
    str = regex_replace(str, regex("\t"), " ");
    if (delim != ' ')
        str = regex_replace(str, regex(" "), "");
    vector<string> result;
    stringstream ss(str);
    string item;

    while (getline(ss, item, delim)) {
        if (!item.empty())
            result.push_back(item);
    }

    return result;
}

string strToObjCode(string str)
{
    stringstream ss;
    if (str[0] == 'C')
        str = str.substr(2, str.length() - 3);
    else if (str[0] == 'X')
        str = str.substr(2, str.length() - 3);
    for (int i = 0; i < str.length(); i++)
        ss << sethex << (int)str[i];
    string result = ss.str();
    return string(6 - result.length(), '0') + result;
}

int handleDirective(string ditective, string operand, string& objectCode)
{
    if (ditective == "START") {
        return stoi(operand);
    } else if (ditective == "END") {
        return 0;
    } else if (ditective == "BYTE") {
        int numOfBytes = 0;
        if (operand[0] == 'X') { // hex
            numOfBytes = (operand.length() - 3) / 2;
            objectCode = operand.substr(2, operand.length() - 3);
        } else if (operand[0] == 'C') { // char
            numOfBytes = operand.length() - 3;
            objectCode = strToObjCode(operand);
        } else {
            throw "Invalid operand";
            return -1;
        }
        return numOfBytes;
    } else if (ditective == "WORD") {
        // TODO: check if operand is a number
        objectCode = string(6 - operand.length(), '0') + operand;
        return 3;
    } else if (ditective == "RESB") {
        return stoi(operand);
    } else if (ditective == "RESW") {
        return stoi(operand) * 3;
    } else if (ditective == "BASE") {
        BASE = operand;
        return 0;
    } else if (ditective == "NOBASE") {
        BASE = "";
        return 0;
    }
    return 0;
}
