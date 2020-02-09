//
// Created by Zhitao Lin on 2020/2/9.
//

#ifndef TYPED_SCHEME_SCHEMEOBJECT_HPP
#define TYPED_SCHEME_SCHEMEOBJECT_HPP

#include <memory>

enum class SchemeObjectType {
    CLOSURE
};

class SchemeObject{
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

    Closure() {};

    Closure(int instructionAddress, const shared_ptr<Closure> &parentClosurePtr) : instructionAddress(
            instructionAddress), parentClosurePtr(parentClosurePtr) {};

    void setBoundVariable(const string &variable, const string &value, bool dirtyFlag);

    string getBoundVariable(const string &variable);

    void setFreeVariable(const string &variable, const string &value, bool dirtyFlag);

    string getFreeVariable(const string &variable);
};

//=================================================================
//                    Closure's Closure
//=================================================================

void Closure::setBoundVariable(const string &variable, const string &value, bool dirtyFlag) {
    this->boundVariables[variable] = value;
    this->dirtyFlags[variable] = dirtyFlag;
}

string Closure::getBoundVariable(const string &variable) {
    return this->boundVariables[variable];
}

void Closure::setFreeVariable(const string &variable, const string &value, bool dirtyFlag) {
    this->freeVariables[variable] = value;
    this->dirtyFlags[variable] = dirtyFlag;
}

string Closure::getFreeVariable(const string &variable) {
    return this->freeVariables[variable];
}


#endif //TYPED_SCHEME_SCHEMEOBJECT_HPP
