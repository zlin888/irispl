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
#include "AST.hpp"
#include <cstdlib>
#include <vector>
#include <map>

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

        int contextLineNum = 2;
        int left = line - 1 - contextLineNum;
        int right = line - 1 + contextLineNum;
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

    string generatePrefix(string title) {
        string prefix = "------------ " + title + " ------------";
        return prefix;
    }

    string generatePostfix(int prefixSize) {
        string postfix = "";
        for (int j = 0; j < prefixSize; ++j) {
            postfix += "-";
        }
        return postfix;

    }

    string createArgumentsNumberErrorMessage(string functionName, int expectedNum, int actualNum) {
        string be = actualNum > 1 ? " are " : " is ";
        string add_s = expectedNum > 1 ? "s" : "";
        string message = "[ArgumentNumberError] " + functionName + " expects " + to_string(expectedNum) + " argument" + add_s +
                         ", " + to_string(actualNum) + be + "given";
        return message;
    }

    string createArgumentTypeErrorMessage(string functionName, string whichArgument, string expectedType,
                                          string actualType) {
        string message =
                "[TypeError] " + functionName + "'s " + whichArgument + " should be a " + expectedType + ", a " +
                actualType + " is given";
        return message;
    }

    string createKeywordErrorMessage(string functionName, string whichArgument, string expectedName,
                                     string actualName) {
        string message =
                "[KeywordError] " + functionName + "'s " + whichArgument + " should be a " + expectedName + ", a " +
                actualName + " is given";
        return message;
    }

    string createVariableUndefinedMessage(string variableName) {
        string message =
                "[UndefinedError] variable '" + variableName + "' used but is undefined";
        return message;
    }

    string createEmptyApplicationMessage() {
        string message =
                "[EmptyApplicationError] application can not be empty";
        return message;
    }

    string createRepeatedDefinitionMessage(string variableName) {
        string message = "[RepeatedDefinitionError] variable " + variableName + " is defined repeatedly";
        return message;
    }

    void raiseError(AST &ast, Handle handle, string message, string prefixTitle) {
        string prefix = utils::generatePrefix(prefixTitle);
        string postfix = utils::generatePostfix(prefix.size());

        cout << prefix << endl;
        utils::coutContext(ast, handle, message);
        cout << postfix;
        throw std::runtime_error("");
    }

    string getActualTypeStr(AST &ast, HandleOrStr hos) {

        // if its a handle, return the schemeObjType
        // else return the type is fine
        Type type = typeOfStr(hos);
        string actualType =
                type == Type::HANDLE ? SchemeObjectTypeStrMap[ast.get(hos)->schemeObjectType] : TypeStrMap[type];
        return actualType;
    }

    bool assertType(HandleOrStr hos, Type type) {
        if (typeOfStr(hos) != type) {
            return false;
        }
        return true;
    }

    bool assertType(AST &ast, HandleOrStr hos, SchemeObjectType schemeObjectType) {
        if (typeOfStr(hos) == Type::HANDLE && ast.get(hos)->schemeObjectType == schemeObjectType) {
            return true;
        }
        return false;
    }

    string getIRISPATH(){
        string stdLibPath = getenv("IRISPATH");
        return stdLibPath;
    }

    string getIRISLIBPath(){
        string stdLibPath = getenv("IRISLIB");
        return stdLibPath;
    }


    inline bool exists(const std::string& name) {
        ifstream f(name.c_str());
        return f.good();
    }

    string getStdLibPath(string moduleName) {
        string irisLibPath = utils::getIRISLIBPath();
        vector<string> fields;
        boost::split(fields, irisLibPath, boost::is_any_of(":"));
        for (auto field : fields) {
            string path;
            path = moduleName.ends_with(".scm") ? field + "/" + moduleName : field + "/" + moduleName + ".scm";
            if (utils::exists(path)) {
                return path;
            }
        }
        throw runtime_error("moduleName not found");
    }
}

#endif //TYPED_SCHEME_UTILS_HPP
