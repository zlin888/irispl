//
// Created by Zhitao Lin on 2020/2/9.
//

#ifndef TYPED_SCHEME_SCHEMEOBJECT_HPP
#define TYPED_SCHEME_SCHEMEOBJECT_HPP

#include <memory>
#include <map>
#include <regex>
#include <set>
#include "Process.hpp"

typedef string Handle;

enum class SchemeObjectType {
    CLOSURE, STRING, LIST, LAMBDA, APPLICATION, QUOTE, QUASIQUOTE, UNQUOTE, CONTINUATION
};

map<SchemeObjectType, string> SchemeObjectTypeStrMap = {
        {SchemeObjectType::CLOSURE,      "CLOSURE"},
        {SchemeObjectType::STRING,       "STRING"},
        {SchemeObjectType::LIST,         "LIST"},
        {SchemeObjectType::LAMBDA,       "LAMBDA"},
        {SchemeObjectType::APPLICATION,  "APPLICATION"},
        {SchemeObjectType::QUOTE,        "QUOTE"},
        {SchemeObjectType::UNQUOTE,      "UNQUOTE"},
        {SchemeObjectType::QUASIQUOTE,   "QUASIQUOTE"},
        {SchemeObjectType::CONTINUATION, "CONTINUATION"},
};

class SchemeObject {
public:
    SchemeObjectType schemeObjectType;
};

class Closure : public SchemeObject {
public:

    int instructionAddress{};
    map<string, string> boundVariables;
    map<string, string> freeVariables;
    map<string, bool> dirtyFlags;
    std::shared_ptr<Closure> parentClosurePtr;
    SchemeObjectType schemeObjectType = SchemeObjectType::CLOSURE;
    Handle selfHandle;

    Closure() {};

    Closure(int instructionAddress, const shared_ptr<Closure> &parentClosurePtr, const Handle &selfHandle)
            : instructionAddress(
            instructionAddress), parentClosurePtr(parentClosurePtr), selfHandle(selfHandle) {};

    void setBoundVariable(const string &variableName, const string &variableValue, bool dirtyFlag);

    string getBoundVariable(const string &variableName);

    bool hasBoundVariable(const string &variableName);

    void setFreeVariable(const string &variableName, const string &variableValue, bool dirtyFlag);

    string getFreeVariable(const string &variableName);

    bool hasFreeVariable(const string &variableName);

    bool isDirtyVairable(const string &variableName);
};

class ApplicationObject : public SchemeObject {
public:
    explicit ApplicationObject(Handle parentHandle) : parentHandle(parentHandle) {};

    Handle parentHandle;
    vector<Handle> childrenHandles;
    SchemeObjectType schemeObjectType = SchemeObjectType::APPLICATION;
};

class LambdaObject : public SchemeObject {
public:
    explicit LambdaObject(Handle parentHandle) : parentHandle(parentHandle) {};
    Handle parentHandle;
    vector<Handle> childrenHandles;
    SchemeObjectType schemeObjectType = SchemeObjectType::LAMBDA;
};

class QuoteObject : public SchemeObject {
public:
    explicit QuoteObject(Handle parentHandle) : parentHandle(parentHandle) {};
    Handle parentHandle;
    SchemeObjectType schemeObjectType = SchemeObjectType::QUOTE;
};

class QuasiquoteObject : public SchemeObject {
public:
    explicit QuasiquoteObject(Handle parentHandle) : parentHandle(parentHandle) {};
    Handle parentHandle;
    SchemeObjectType schemeObjectType = SchemeObjectType::QUASIQUOTE;
};

class UnquoteObject : public SchemeObject {
public:
    explicit UnquoteObject(Handle parentHandle) : parentHandle(parentHandle) {};
    Handle parentHandle;
    SchemeObjectType schemeObjectType = SchemeObjectType::UNQUOTE;
};

class StringObject : public SchemeObject {
public:
    string content;
    SchemeObjectType schemeObjectType = SchemeObjectType::STRING;

    StringObject(string content) : content(content) {}
};

//=================================================================
//                    Closure's Closure
//=================================================================

void Closure::setBoundVariable(const string &variableName, const string &variableValue, bool dirtyFlag) {
    this->boundVariables[variableName] = variableValue;
    this->dirtyFlags[variableName] = dirtyFlag;
}

string Closure::getBoundVariable(const string &variableName) {
    return this->boundVariables[variableName];
}

void Closure::setFreeVariable(const string &variableName, const string &variableValue, bool dirtyFlag) {
    this->freeVariables[variableName] = variableValue;
    this->dirtyFlags[variableName] = dirtyFlag;
}

string Closure::getFreeVariable(const string &variableName) {
    return this->freeVariables[variableName];
}

bool Closure::hasBoundVariable(const string &variableName) {
    return this->boundVariables.count(variableName);
}

bool Closure::hasFreeVariable(const string &variableName) {
    return this->freeVariables.count(variableName);
}

bool Closure::isDirtyVairable(const string &variableName) {
    return this->dirtyFlags[variableName];
}


enum class Type {
    UNDEFINED, LAMBDA, PORT, HANDLE, SYMBOL, LABEL, VARIABLE, STRING, NUMBER, KEYWORD, BOOLEAN
};

set<string> KEYWORDS = {
        "car", "cdr", "cons", "cond", "if", "else", "begin",
        "+", "-", "*", "/", "=", "%", "pow",
        "and", "or", "not", ">", "<", ">=", "<=", "eq?",
        "define", "set!", "null?", "atom?", "list?", "number?",
        "display", "newline",
        "write", "read",
        "call/cc",
        "import", "native",
        "fork",
        "quote", "quasiquote", "unquote",
};

Type typeOfStr(const string &inputStr) {
    if (inputStr.empty()) {
        return Type::UNDEFINED;
    } else if (KEYWORDS.count(inputStr) != 0) {
        return Type::KEYWORD;
    } else if (inputStr == "#t" || inputStr == "#f") {
        return Type::BOOLEAN;
    } else if (inputStr == "lambda") {
        return Type::LAMBDA;
    } else if (inputStr[0] == ':') {
        return Type::PORT;
    } else if (inputStr[0] == '&') {
        return Type::HANDLE;
    } else if (inputStr[0] == '\'') {
        return Type::SYMBOL;
    } else if (inputStr[0] == '@') {
        return Type::LABEL;
    } else if (inputStr[0] == '"' && inputStr[inputStr.size() - 1] == '"') {
        return Type::STRING;
    } else if (std::regex_match(inputStr, std::regex("(-?[0-9]+([.][0-9]+)?)"))) {
        return Type::NUMBER;
    } else {
        return Type::VARIABLE;
    }
}

#endif //TYPED_SCHEME_SCHEMEOBJECT_HPP
