#pragma once
#include "table.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

extern string BASE;
#define byte unsigned char;

vector<string> getTokens(string str, char delim);
int handleDirective(string ditective, string operand, string& objectCode);
ostream& sethex(ostream& stream);
