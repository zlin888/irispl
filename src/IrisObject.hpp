//
// Created by Zhitao Lin on 2020/2/9.
//

#ifndef TYPED_IRIS_IRISOBJECT_HPP
#define TYPED_IRIS_IRISOBJECT_HPP

#include <memory>
#include <map>
#include <regex>
#include <set>
#include <algorithm>

using namespace std;

typedef string Handle;
typedef string HandleOrStr;

enum class IrisObjectType {
    CLOSURE, STRING, LIST, LAMBDA, APPLICATION, QUOTE, QUASIQUOTE, UNQUOTE, CONTINUATION, SchemeChildrenHosesObject
};

map<IrisObjectType, string> IrisObjectTypeStrMap = {
        {IrisObjectType::CLOSURE,                   "CLOSURE"},
        {IrisObjectType::STRING,                    "STRING"},
        {IrisObjectType::LIST,                      "LIST"},
        {IrisObjectType::LAMBDA,                    "LAMBDA"},
        {IrisObjectType::APPLICATION,               "APPLICATION"},
        {IrisObjectType::QUOTE,                     "QUOTE"},
        {IrisObjectType::UNQUOTE,                   "UNQUOTE"},
        {IrisObjectType::QUASIQUOTE,                "QUASIQUOTE"},
        {IrisObjectType::CONTINUATION,              "CONTINUATION"},
        {IrisObjectType::SchemeChildrenHosesObject, "SchemeChildrenHosesObject"},
};

string irisObjectTypeToStr(IrisObjectType irisObjectType) {
    if(IrisObjectTypeStrMap.count(irisObjectType)) {
        return IrisObjectTypeStrMap[irisObjectType];
    } else {
        return "";
    }
}



class IrisObject {
public:
    IrisObject(IrisObjectType irisObjectType, Handle parentHandle, Handle selfHandle) : irisObjectType(irisObjectType),
                                                                                          parentHandle(parentHandle), selfHandle(selfHandle) {};

    IrisObject(IrisObjectType irisObjectType) : irisObjectType(irisObjectType) {};

    IrisObjectType irisObjectType;
    Handle parentHandle;
    Handle selfHandle;

    static std::vector<HandleOrStr> &getChildrenHosesOrBodies(shared_ptr<IrisObject> irisObjPtr);
};


class Closure : public IrisObject {
public:

    int instructionAddress{};
    map<string, string> boundVariables;
    map<string, string> freeVariables;
    map<string, bool> dirtyFlags;
    std::shared_ptr<Closure> parentClosurePtr;
    IrisObjectType irisObjectType = IrisObjectType::CLOSURE;
    Handle selfHandle;

//    Closure() : IrisObject(irisObjectType::CLOSURE) {};

    Closure(int instructionAddress, const shared_ptr<Closure> &parentClosurePtr, const Handle &selfHandle)
            : IrisObject(IrisObjectType::CLOSURE), instructionAddress(
            instructionAddress), parentClosurePtr(parentClosurePtr), selfHandle(selfHandle) {};

    void setBoundVariable(const string &variableName, const string &variableValue, bool dirtyFlag);

    string getBoundVariable(const string &variableName);

    bool hasBoundVariable(const string &variableName);

    void setFreeVariable(const string &variableName, const string &variableValue, bool dirtyFlag);

    string getFreeVariable(const string &variableName);

    bool hasFreeVariable(const string &variableName);

    bool isDirtyVairable(const string &variableName);
};

class ApplicationObject : public IrisObject {
public:
    ApplicationObject(Handle parentHandle, Handle selfHandle) : IrisObject(IrisObjectType::APPLICATION, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void ApplicationObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class ListObject : public IrisObject {
public:
    bool isFake = false;
    shared_ptr<ListObject> realListObjPtr = nullptr;
    int currentIndex = 0;

    ListObject(Handle parentHandle, Handle selfHandle) : IrisObject(IrisObjectType::LIST, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);

    void pointTo(shared_ptr<ListObject> realListObjPtr, int index);

    HandleOrStr car();

    int size();

    vector<HandleOrStr> getChildrenHoses();
};

vector<HandleOrStr> ListObject::getChildrenHoses() {
    return vector<HandleOrStr>(this->childrenHoses.begin() + currentIndex, this->childrenHoses.end());
}

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
class LambdaObject : public IrisObject {
public:
    LambdaObject(Handle parentHandle, Handle selfHandle) : IrisObject(IrisObjectType::LAMBDA, parentHandle, selfHandle) {};

    vector<HandleOrStr> bodies;
    vector<Handle> parameters;
    IrisObjectType irisObjectType = IrisObjectType::LAMBDA;

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

class QuoteObject : public IrisObject {
public:
    QuoteObject(Handle parentHandle, Handle selfHandle) : IrisObject(IrisObjectType::QUOTE, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void QuoteObject::addChild(HandleOrStr childHos) {
    if (!childHos.starts_with("'")) {
        childHos = "'" + childHos;
    }
    this->childrenHoses.push_back(childHos);
}


class QuasiquoteObject : public IrisObject {
public:
    QuasiquoteObject(Handle parentHandle, Handle selfHandle) : IrisObject(IrisObjectType::QUASIQUOTE, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void QuasiquoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class UnquoteObject : public IrisObject {
public:
    UnquoteObject(Handle parentHandle, Handle selfHandle) : IrisObject(IrisObjectType::UNQUOTE, parentHandle, selfHandle) {};

    vector<HandleOrStr> childrenHoses;

    void addChild(HandleOrStr childHos);
};

void UnquoteObject::addChild(HandleOrStr childHos) {
    this->childrenHoses.push_back(childHos);
}

class StringObject : public IrisObject {
public:
    string content;
    IrisObjectType irisObjectType = IrisObjectType::STRING;

    StringObject(string content) : IrisObject(IrisObjectType::STRING), content(content) {}
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
        "car", "cdr", "list", "cond", "if", "else", "begin", "cons",
        "+", "-", "*", "/", "=", "%", "pow",
        "and", "or", "not", ">", "<", ">=", "<=", "eq?",
        "define", "set!", "null?", "atom?", "list?", "number?", "pair?",
        "display", "newline",
        "write", "read",
        "call/cc",
        "import", "native",
        "fork",
        "quote", "quasiquote", "unquote",
        "let", "apply",
        "class", "isinstance?",
        "exit", "type",
};

map<string, string> primitiveInstructionMap{
        {"+",    "add"},
        {"-",    "sub"},
        {"*",    "mul"},
        {"/",    "div"},
        {"%",    "mod"},
        {"=",    "eqn"},
        {"<",    "lt"},
        {">",    "gt"},
        {"<=",   "le"},
        {">=",   "ge"},
        {"set!", "set"}
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

std::vector<HandleOrStr> &IrisObject::getChildrenHosesOrBodies(shared_ptr<IrisObject> irisObjPtr) {
    if (irisObjPtr->irisObjectType == IrisObjectType::APPLICATION) {
        return static_pointer_cast<ApplicationObject>(irisObjPtr)->childrenHoses;
    } else if (irisObjPtr->irisObjectType == IrisObjectType::UNQUOTE) {
        return static_pointer_cast<UnquoteObject>(irisObjPtr)->childrenHoses;
    } else if (irisObjPtr->irisObjectType == IrisObjectType::QUOTE) {
        return static_pointer_cast<QuoteObject>(irisObjPtr)->childrenHoses;
    } else if (irisObjPtr->irisObjectType == IrisObjectType::QUASIQUOTE) {
        return static_pointer_cast<QuasiquoteObject>(irisObjPtr)->childrenHoses;
    } else if (irisObjPtr->irisObjectType == IrisObjectType::LAMBDA) {
        return static_pointer_cast<LambdaObject>(irisObjPtr)->bodies;
    } else if (irisObjPtr->irisObjectType == IrisObjectType::LIST) {
        return static_pointer_cast<ListObject>(irisObjPtr)->childrenHoses;
    }
    throw std::runtime_error("[getChildrenHoses] not a application, unquote or quasiquote");
}


#endif //TYPED_IRIS_IRISOBJECT_HPP
