#pragma once
#include "table.h"
#include <cctype>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;

#define byte unsigned char;

class File {
  public:
    string fullPath;
    string dir;
    string name;

    File();
    File(string fullPath);
};

vector<string> getTokens(string str, char delim);
ostream &sethex(ostream &stream);
ostream &setdec(ostream &stream);
bool getOption(int argc, char *argv[], File &input, File &output, int &mode);
bool isDecimal(const string &str);
bool isHexadecimal(const string &str);
bool isNumber(string str);