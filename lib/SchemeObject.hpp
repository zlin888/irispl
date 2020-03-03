//
// Created by Zhitao Lin on 2020/2/9.
//

#ifndef TYPED_SCHEME_SCHEMEOBJECT_HPP
#define TYPED_SCHEME_SCHEMEOBJECT_HPP

#include <memory>
#include <map>
#include <regex>
#include <set>
#include <algorithm>
#include "Process.hpp"

typedef string Handle;
typedef string HandleOrStr;

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
    SchemeObject(SchemeObjectType schemeObjectType, Handle parentHandle) : schemeObjectType(schemeObjectType), parentHandle(parentHandle) {};
    SchemeObject(SchemeObjectType schemeObjectType) : schemeObjectType(schemeObjectType) {};

    SchemeObjectType schemeObjectType;
    Handle parentHandle;

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

//    Closure() : SchemeObject(SchemeObjectType::CLOSURE) {};

    Closure(int instructionAddress, const shared_ptr<Closure> &parentClosurePtr, const Handle &selfHandle)
            : SchemeObject(SchemeObjectType::CLOSURE), instructionAddress(
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
    ApplicationObject(Handle parentHandle) : SchemeObject(SchemeObjectType::APPLICATION, parentHandle) {};

    vector<HandleOrStr> childrenHoses;
    void addChild(HandleOrStr childHos);
};

void ApplicationObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

// [lambda, [param0, ... ], body0, ...]
class LambdaObject : public SchemeObject {
public:
    LambdaObject(Handle parentHandle) : SchemeObject(SchemeObjectType::LAMBDA, parentHandle) {};

    vector<HandleOrStr> bodies;
    vector<Handle> parameters;
    SchemeObjectType schemeObjectType = SchemeObjectType::LAMBDA;

    bool hasParameter(string paramter);

    bool addParameter(string paramter);

    void addBody(HandleOrStr handleOrStr);

    void setBodies(vector<HandleOrStr> hoses);
};


bool LambdaObject::hasParameter(string paramter) {
    if (find(this->parameters.begin(), this->parameters.end(), paramter) != this->parameters.end()) {
        return true;
    } else {
        return false;
    }
}

bool LambdaObject::addParameter(string paramter) {
    if (this->hasParameter(paramter)) {
        return false;
    } else {
        this->parameters.push_back(paramter);
        return true;
    }
}


void LambdaObject::addBody(HandleOrStr handleOrStr) {
    this->bodies.push_back(handleOrStr);
}

void LambdaObject::setBodies(vector<HandleOrStr> newBodies) {
    this->bodies.clear();
    this->bodies = newBodies;
//this.children = this.children.slice(0, 2).concat(bodies);
}

class QuoteObject : public SchemeObject {
public:
    QuoteObject(Handle parentHandle) : SchemeObject(SchemeObjectType::QUOTE, parentHandle) {};

    vector<HandleOrStr> childrenHoses;
    void addChild(HandleOrStr childHos);
};

void QuoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}


class QuasiquoteObject : public SchemeObject {
public:
    QuasiquoteObject(Handle parentHandle) : SchemeObject(SchemeObjectType::QUASIQUOTE, parentHandle) {};

    vector<HandleOrStr> childrenHoses;
    void addChild(HandleOrStr childHos);
};

void QuasiquoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class UnquoteObject : public SchemeObject {
public:
    UnquoteObject(Handle parentHandle) : SchemeObject(SchemeObjectType::UNQUOTE, parentHandle) {};

    vector<HandleOrStr> childrenHoses;
    void addChild(HandleOrStr childHos);
};

void UnquoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class StringObject : public SchemeObject {
public:
    string content;
    SchemeObjectType schemeObjectType = SchemeObjectType::STRING;

    StringObject(string content) : SchemeObject(SchemeObjectType::STRING), content(content) {}
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
