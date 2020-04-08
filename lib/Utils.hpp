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

    void coutContext(AST &ast, const Handle &handle, string message) {
        string moduleName = ast.sourceCodeMapper.getModuleName(handle);
        string path = ast.sourceCodeMapper.getPath(handle);
        int index = ast.sourceCodeMapper.getIndex(handle);
        string source = ast.sourceCodeMapper.getSourceCode(handle);

        cout << "Description: " + message << endl;
        cout << "Happened in: " + moduleName + " " + path << endl;

        int line = 1;
        for (int i = 0; i < source.size(); i++) {
            if (i == index) {
                break;
            }
            if (source[i] == '\n') {
                line++;
            }
        }

        vector<string> fields;
        boost::split(fields, source, boost::is_any_of("\n"));

        int left = line - 1 - 2;
        int right = line - 1 + 2;
        for (int i = left; i <= right; i++) {
            // fields[-1] is always '))', used to complete the top_lambda, we don't want to show it to user
            if (i >= 0 && i < fields.size() - 1) {
                cout << "  " + to_string(i + 1) + ": ";
                cout << fields[i];
                if (i == line - 1) {
                    cout << "                 <--- Oooops~" << endl;
                } else {
                    cout << endl;
                }
            }
        }
    }

}

#endif //TYPED_SCHEME_UTILS_HPP
