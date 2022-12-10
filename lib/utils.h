#pragma once
#include "table.h"
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

#define byte unsigned char;

vector<string> getTokens(string str, char delim);
ostream& sethex(ostream& stream);

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

void printFile(string filename, string title, string color)
{
    ifstream fin(filename);
    string line;
    cout << color << "        " + title + "        " << RESET << endl;
    while (getline(fin, line)) {
        cout << line << endl;
    }
}

bool getOption(int argc, char* argv[], string& fileName, bool& isPrint)
{
    const struct option longopts[] = { { "input", required_argument, NULL, 'i' }, { "help", no_argument, NULL, 'h' },
        { "print", no_argument, NULL, 'p' } };

    int opt, opterr = 0;
    isPrint = false;

    while ((opt = getopt_long(argc, argv, "i:ph", longopts, NULL)) != -1) {
        switch (opt) {
        case 'i':
            fileName = optarg;
            break;
        case 'p':
            isPrint = true;
            break;
        case '?':
            if (optopt == 'i')
                cout << "Option -" << (char)optopt << " requires an argument." << endl;
            else if (isprint(optopt))
                cout << "Unknown option -" << (char)optopt << "." << endl;
            else if (isprint(optopt))
                cout << "Unknown option -" << (char)optopt << "." << endl;
            else
                cout << "Unknown option character `-" << (char)optopt << "`." << endl;
            return 0;
        case 'h':
            cout << "Usage: " << argv[0] << " [OPTION]..." << endl;
            cout << "  -i, --input FILE\tinput file" << endl;
            cout << "  -p, --print\t\tprint the output on the screen" << endl;
            cout << "  -h, --help\t\tprint this help" << endl;
            return 0;
        default:
            return 0;
        }
    }

    return true;
}