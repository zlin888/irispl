//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_UTILS_HPP
#define TYPED_SCHEME_UTILS_HPP

#include <stdexcept>
#include <fstream>
#include <vector>
#include <stdarg.h>
#include <set>
#include <boost/algorithm/string.hpp>
#include "SchemeObject.hpp"
#include "Parser.hpp"

using namespace std;

namespace utils {
    bool log_flag = true;

    inline std::string trim(const std::string &s) {
        auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) { return std::isspace(c); });
        auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) { return std::isspace(c); }).base();
        return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
    }

    string readFileToOneString(const string &path) {
        std::fstream fs(path);
        string resultStr;

        if (fs.is_open()) {
            std::string line;
            while (std::getline(fs, line)) {
                resultStr += line + '\n';
            }
            fs.close();
        } else {
            throw std::runtime_error("file not file");
        }

        return resultStr;
    }

    template<class T>
    set<T> makeSet(int count, ...) {
        va_list ap;
        set<T> aSet;
        va_start(ap, count);
        for (int i = 0; i < count; i++) {
            aSet.insert(va_arg(ap, T));
        }
        va_end(ap);
        return std::move(aSet);
    }

    void
    log(std::string message, std::string FileName = "Unset", std::string FunctionName = "Unset", int LineNumber = -1) {
        if (log_flag) {
            vector<string> fields;
            boost::split(fields, FileName, boost::is_any_of("/"));
            if (!fields.empty()) {
                FileName = fields[fields.size() - 1];
            }

            cout << FileName + ">" + FunctionName + "(" + to_string(LineNumber) + "): " + message << endl;

        }
    }

    void coutContext(AST &ast, const Handle &handle, string message, int prefixSize) {
        int index = ast.handleSourceIndexesMap[handle];

        int left = index;
        int right = index;
        int contextLineNum = 2;
        for (int i = 0; i < contextLineNum; i++) {
            // roll back one sentence * 2
            while (left > 0 && ast.source[left] != '\n') {
                left--; //left will be either 0 or '\n'
            }
            if (left > 0) {
                left--;
            }
        }

        for (int i = 0; i < contextLineNum; i++) {
            // roll back one sentence * 2
            while (right < ast.source.size() - 1 && ast.source[right] != '\n') {
                right++;
            }
            if (right < ast.source.size() - 1) {
                right++;
            }
        }

        // find out the current line
        int line = 1;
        for (int i = 0; i < ast.source.size(); i++) {
            if (i == index) {
                break;
            }
            if (ast.source[i] == '\n') {
                line++;
            }
        }

        int outputedLineNum = 2;
        bool inTheLine = false;

        cout << message << endl;
        for (int i = left; i <= right; i++) {
            if(ast.source[i-1] == '\n') {
                cout << to_string(line - outputedLineNum) + ": ";
                outputedLineNum--;
            }
            if (i == index) {
                inTheLine = true;
            }
            if (ast.source[i] == '\n') {
                if (inTheLine) {
                    inTheLine = false;
                    cout << "          <-- Ooops";
                }
            }
            cout << ast.source[i];
        }
        cout << endl;
        for (int i = 0; i < prefixSize; i++) {
            cout << "-";
        }
        cout << endl;
    }
}

#endif //TYPED_SCHEME_UTILS_HPP
