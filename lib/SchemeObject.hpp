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

using namespace std;

typedef string Handle;
typedef string HandleOrStr;

enum class SchemeObjectType {
    CLOSURE, STRING, LIST, LAMBDA, APPLICATION, QUOTE, QUASIQUOTE, UNQUOTE, CONTINUATION, SchemeChildrenHosesObject
};

map<SchemeObjectType, string> SchemeObjectTypeStrMap = {
        {SchemeObjectType::CLOSURE,                   "CLOSURE"},
        {SchemeObjectType::STRING,                    "STRING"},
        {SchemeObjectType::LIST,                      "LIST"},
        {SchemeObjectType::LAMBDA,                    "LAMBDA"},
        {SchemeObjectType::APPLICATION,               "APPLICATION"},
        {SchemeObjectType::QUOTE,                     "QUOTE"},
        {SchemeObjectType::UNQUOTE,                   "UNQUOTE"},
        {SchemeObjectType::QUASIQUOTE,                "QUASIQUOTE"},
        {SchemeObjectType::CONTINUATION,              "CONTINUATION"},
        {SchemeObjectType::SchemeChildrenHosesObject, "SchemeChildrenHosesObject"},
};

string schemeObjectTypeToStr(SchemeObjectType schemeObjectType) {
    if(SchemeObjectTypeStrMap.count(schemeObjectType)) {
        return SchemeObjectTypeStrMap[schemeObjectType];
    } else {
        return "";
    }
}



class SchemeObject {
public:
    SchemeObject(SchemeObjectType schemeObjectType, Handle parentHandle, Handle selfHandle) : schemeObjectType(schemeObjectType),
                                                                           parentHandle(parentHandle), selfHandle(selfHandle) {};

    SchemeObject(SchemeObjectType schemeObjectType) : schemeObjectType(schemeObjectType) {};

    SchemeObjectType schemeObjectType;
    Handle parentHandle;
    Handle selfHandle;

    static std::vector<HandleOrStr> &getChildrenHosesOrBodies(shared_ptr<SchemeObject> schemeObjPtr);
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
    ApplicationObject(Handle parentHandle, Handle selfHandle) : SchemeObject(SchemeObjectType::APPLICATION, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void ApplicationObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class ListObject : public SchemeObject {
public:
    bool isFake = false;
    shared_ptr<ListObject> realListObjPtr = nullptr;
    int currentIndex = 0;

    ListObject(Handle parentHandle, Handle selfHandle) : SchemeObject(SchemeObjectType::LIST, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);

    void pointTo(shared_ptr<ListObject> realListObjPtr, int index);

    HandleOrStr car();

    int size();
};

int ListObject::size() {
    if(this->isFake) {
        return this->realListObjPtr->childrenHoses.size() - currentIndex;
    } else {
        return this->childrenHoses.size();
    }

}

HandleOrStr ListObject::car() {
    if (isFake) {
        return realListObjPtr->childrenHoses[currentIndex];
    } else {
        return this->childrenHoses[0];
    }
}

void ListObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

// when using cdr, we fake a new List, and make the new List point to the real List
// reduce the cdr complexity
void ListObject::pointTo(shared_ptr<ListObject> realListObjPtr, int index) {
    this->isFake = true;
    this->currentIndex = index;
    this->realListObjPtr = realListObjPtr;
}

// [lambda, [param0, ... ], body0, ...]
class LambdaObject : public SchemeObject {
public:
    LambdaObject(Handle parentHandle, Handle selfHandle) : SchemeObject(SchemeObjectType::LAMBDA, parentHandle, selfHandle) {};

    vector<HandleOrStr> bodies;
    vector<Handle> parameters;
    SchemeObjectType schemeObjectType = SchemeObjectType::LAMBDA;

    bool hasParameter(string parameter);

    bool addParameter(string parameter);

    void addBody(HandleOrStr handleOrStr);

    void setBodies(vector<HandleOrStr> hoses);
};


bool LambdaObject::hasParameter(string parameter) {
    if (find(this->parameters.begin(), this->parameters.end(), parameter) != this->parameters.end()) {
        return true;
    } else {
        return false;
    }
}

bool LambdaObject::addParameter(string parameter) {
    if (this->hasParameter(parameter)) {
        return false;
    } else {
        this->parameters.push_back(parameter);
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
    QuoteObject(Handle parentHandle, Handle selfHandle) : SchemeObject(SchemeObjectType::QUOTE, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void QuoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}


class QuasiquoteObject : public SchemeObject {
public:
    QuasiquoteObject(Handle parentHandle, Handle selfHandle) : SchemeObject(SchemeObjectType::QUASIQUOTE, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void QuasiquoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class UnquoteObject : public SchemeObject {
public:
    UnquoteObject(Handle parentHandle, Handle selfHandle) : SchemeObject(SchemeObjectType::UNQUOTE, parentHandle, selfHandle) {};

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


map<Type, string> TypeStrMap = {
        {Type::UNDEFINED, "UNDEFINED"},
        {Type::LAMBDA,    "LAMBDA"},
        {Type::PORT,      "PORT"},
        {Type::HANDLE,    "HANDLE"},
        {Type::SYMBOL,    "SYMBOL"},
        {Type::LABEL,     "LABEL"},
        {Type::VARIABLE,  "VARIABLE"},
        {Type::STRING,    "STRING"},
        {Type::NUMBER,    "NUMBER"},
        {Type::KEYWORD,   "KEYWORD"},
        {Type::BOOLEAN,   "BOOLEAN"},
};

set<string> KEYWORDS = {
        "car", "cdr", "cons", "cond", "if", "else", "begin",
        "+", "-", "*", "/", "=", "%", "pow",
        "and", "or", "not", ">", "<", ">=", "<=", "eq?",
        "define", "set!", "null?", "atom?", "list?", "number?", "pair?",
        "display", "newline",
        "write", "read",
        "call/cc",
        "import", "native",
        "fork",
        "quote", "quasiquote", "unquote",
        "let", "apply"
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

std::vector<HandleOrStr> &SchemeObject::getChildrenHosesOrBodies(shared_ptr<SchemeObject> schemeObjPtr) {
    if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
        return static_pointer_cast<ApplicationObject>(schemeObjPtr)->childrenHoses;
    } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::UNQUOTE) {
        return static_pointer_cast<UnquoteObject>(schemeObjPtr)->childrenHoses;
    } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::QUOTE) {
        return static_pointer_cast<QuoteObject>(schemeObjPtr)->childrenHoses;
    } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::QUASIQUOTE) {
        return static_pointer_cast<QuasiquoteObject>(schemeObjPtr)->childrenHoses;
    } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
        return static_pointer_cast<LambdaObject>(schemeObjPtr)->bodies;
    } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::LIST) {
        return static_pointer_cast<ListObject>(schemeObjPtr)->childrenHoses;
    }
    throw std::runtime_error("[getChildrenHoses] not a application, unquote or quasiquote");
}


#endif //TYPED_SCHEME_SCHEMEOBJECT_HPP
