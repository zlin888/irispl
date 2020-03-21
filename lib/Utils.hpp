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

    void log(std::string message, std::string FileName = "Unset", std::string FunctionName = "Unset", int LineNumber = -1)
    {
        if(log_flag) {
            vector<string> fields;
            boost::split(fields, FileName, boost::is_any_of("/"));
            if (!fields.empty()) {
                FileName = fields[fields.size() - 1];
            }

            cout << FileName + ">" + FunctionName + "(" + to_string(LineNumber) + "): "+ message<< endl;

        }
    }
}

#endif //TYPED_SCHEME_UTILS_HPP
