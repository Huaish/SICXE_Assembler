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

    bool create_directories() {
        int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (status == -1) {
            if (errno != EEXIST) {
                cout << "Error: " << strerror(errno) << endl;
                return false;
            }
        }
        return true;
    }
};

vector<string> getTokens(string str, char delim);
ostream &sethex(ostream &stream);
ostream &setdec(ostream &stream);
void printFile(string filename, string title, string color);
bool getOption(int argc, char *argv[], File &input, File &output,
               bool &isPrint);
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
    cout << color << "        " + title + "        " << RESET << endl;
    while (getline(fin, line)) {
        cout << line << endl;
    }
}

bool getOption(int argc, char *argv[], File &input, File &output,
               bool &isPrint) {
    // i:o:ph
    const struct option longopts[] = {{"input", required_argument, NULL, 'i'},
                                      {"dir", required_argument, NULL, 'd'},
                                      {"help", no_argument, NULL, 'h'},
                                      {"print", no_argument, NULL, 'p'}};

    int opt, opterr = 0;
    isPrint = false;

    input = File("src/input.asm");
    output = File("result/");

    while ((opt = getopt_long(argc, argv, "i:d:ph", longopts, NULL)) != -1) {
        switch (opt) {
        case 'i':
            input = File(optarg);
            break;
        case 'd':
            output = File(optarg);
            break;
        case 'p':
            isPrint = true;
            break;
        case '?':
            if (optopt == 'i')
                cout << "Option -" << (char)optopt << " requires an argument."
                     << endl;
            else if (isprint(optopt))
                cout << "Unknown option -" << (char)optopt << "." << endl;
            else if (isprint(optopt))
                cout << "Unknown option -" << (char)optopt << "." << endl;
            else
                cout << "Unknown option character `-" << (char)optopt << "`."
                     << endl;
            return 0;
        case 'h':
            cout << "Usage: " << argv[0] << " [OPTION]..." << endl;
            cout << "  -i, --input FILE\tinput file" << endl;
            cout << "  -d, --dir DIR\t\toutput directory" << endl;
            cout << "  -p, --print\t\tprint the output on the screen" << endl;
            cout << "  -h, --help\t\tprint this help" << endl;
            return 0;
        default:
            return 0;
        }
    }

    return true;
}

bool isNumber(string str) {
    for (int i = 0; i < str.length(); i++) {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}

string strToObjCode(string str, int &length) {
    str = str.substr(1, str.length() - 1);
    string objectCode = "";
    if (str[0] == 'X') { // hex
        length = (str.length() - 3) / 2;
        objectCode = str.substr(2, str.length() - 3);
    } else if (str[0] == 'C') { // char
        length = str.length() - 3;
        str = str.substr(2, str.length() - 3);
        stringstream ss;
        for (int i = 0; i < str.length(); i++)
            ss << sethex << (int)str[i];
        string result = ss.str();
        if (result.length() > 6)
            objectCode = result;
        else
            objectCode = string(6 - result.length(), '0') + result;
    } else {
        length = 0;
    }

    return objectCode;
}