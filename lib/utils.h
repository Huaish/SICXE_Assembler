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

    File() {
        fullPath = "";
        dir = "";
        name = "";
    }

    File(string fullPath) {
        this->fullPath = fullPath;
        int pos = fullPath.find_last_of('/');
        if (pos == -1) {
            dir = "";
            name = fullPath;
        } else {
            dir = fullPath.substr(0, pos) + "/";
            name = fullPath.substr(pos + 1, fullPath.length() - pos - 1);
        }
    }
};

vector<string> getTokens(string str, char delim);
ostream &sethex(ostream &stream);
ostream &setdec(ostream &stream);
void printFile(string filename, string title, string color);
bool getOption(int argc, char *argv[], File &input, File &output, bool &isPrint);
bool isDecimal(const string &str);
bool isHexadecimal(const string &str);
bool isNumber(string str);

ostream &sethex(ostream &stream) {
    stream.unsetf(ios::dec | ios::oct);
    stream.setf(ios::hex | ios::uppercase | ios::right);

    return stream;
}

ostream &setdec(ostream &stream) {
    stream.unsetf(ios::hex | ios::oct);
    stream.setf(ios::dec | ios::uppercase | ios::right);

    return stream;
}

vector<string> getTokens(string str, char delim) {
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

void printFile(string filename, string title, string color) {
    ifstream fin(filename);
    string line;
    cout << color << setw(38) << setfill(' ') << title;
    cout << color << setw(34) << setfill(' ') << RESET << endl;
    while (getline(fin, line)) {
        cout << line << endl;
    }
}

bool getOption(int argc, char *argv[], File &input, File &output, bool &isPrint, int &mode) {
    // i:o:m:ph
    const struct option longopts[] = {{"input", required_argument, NULL, 'i'},
                                      {"output", required_argument, NULL, 'o'},
                                      {"mode", required_argument, NULL, 'm'},
                                      {"help", no_argument, NULL, 'h'},
                                      {"print", no_argument, NULL, 'p'}};

    int opt, opterr = 0;
    isPrint = false;

    input = File("src/input.asm");
    output = File("output.txt");

    while ((opt = getopt_long(argc, argv, "i:o:m:ph", longopts, NULL)) != -1) {
        switch (opt) {
        case 'i':
            input = File(optarg);
            break;
        case 'o':
            output = File(optarg);
            break;
        case 'm':
            if (isDecimal(optarg)) {
                mode = stoi(optarg);
            } else {
                cout << "Error: mode must be a number." << endl;
                return false;
            }
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
            cout << "  -o, --output FILE\toutput file" << endl;
            cout << "  -m, --mode NUMBER\tmode" << endl;
            cout << "  -p, --print\t\tprint the output on the screen" << endl;
            cout << "  -h, --help\t\tprint this help" << endl;
            return 0;
        default:
            return 0;
        }
    }

    return true;
}

bool isDecimal(const string &str) {
    try {
        size_t pos;
        std::stoi(str, &pos);
        return pos == str.length();
    } catch (const std::invalid_argument &) {
        return false;
    } catch (const std::out_of_range &) {
        return false;
    }
}

bool isHexadecimal(const string &str) {
    try {
        size_t pos;
        std::stoul(str, &pos, 16);
        return pos == str.length();
    } catch (const std::invalid_argument &) {
        return false;
    } catch (const std::out_of_range &) {
        return false;
    }
}

bool isNumber(string str) { return isDecimal(str) || isHexadecimal(str); }