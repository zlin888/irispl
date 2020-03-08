//
// Created by zlin on 2020/3/8.
//

#ifndef TYPED_SCHEME_COMPILER_HPP
#define TYPED_SCHEME_COMPILER_HPP

#include <boost/algorithm/string.hpp>
#include <utility>
#include <vector>
#include <string>
#include "Parser.hpp"
#include "SchemeObject.hpp"
#include "Instruction.hpp"

using namespace std;

class Compiler {
public:
    AST ast;
    vector<Instruction> ILCode;

    explicit Compiler(AST ast) : ast(std::move(ast)) {};

    static vector<Instruction> compile(AST ast);

    void addInstruction(string inst);

    void beginCompile();

    void compileLambda(Handle lambdaHandle);
};

vector<Instruction> Compiler::compile(AST ast) {
    Compiler compiler(std::move(ast));
    compiler.beginCompile();
    return compiler.ILCode;
}

void Compiler::addInstruction(string inst) {
    boost::trim(inst);
    Instruction instruction(inst);
    if (inst.empty()) {
        return;
    } else if (inst.length() >= 2 && inst[0] == ';' && inst[1] == ';') {
        // handle IL comments
        this->ILCode.push_back(instruction);
    } else {
        this->ILCode.push_back(instruction);
    }
}

void Compiler::compileLambda(Handle lambdaHandle) {
    auto lambdaObjPtr = static_pointer_cast<LambdaObject>(this->ast.get(lambdaHandle));
    // label, for jumping
    this->addInstruction("@" + lambdaHandle);

    for (string parameter : lambdaObjPtr->parameters) {
        this->addInstruction("store " + parameter);
    }

    for (int i = 0; i < lambdaObjPtr->bodies.size(); i++) {
        HandleOrStr body = lambdaObjPtr->bodies[i];
        Type bodyType = typeOfStr(body);

        if (bodyType == Type::HANDLE) {
            auto schemeObjPtr = this->ast.get(body);
            SchemeObjectType schemeObjectType = schemeObjPtr->schemeObjectType;

            if(schemeObjectType == SchemeObjectType::LAMBDA) {
                this->addInstruction("loadclosure @" + body);
            } else if(schemeObjectType == SchemeObjectType::QUOTE) {
                this->addInstruction("push " + body);
            } else if(schemeObjectType == SchemeObjectType::QUASIQUOTE) {

            } else if(schemeObjectType == SchemeObjectType::STRING) {

            } else if(schemeObjectType == SchemeObjectType::APPLICATION || schemeObjectType == SchemeObjectType::UNQUOTE) {

            }
        }
//        if(typeOfStr(hos) == Type::VARIABLE) {
//            if(i == lambdaObjPtr->bodies.size() - 1) {
//                this->addInstruction("store " + hos);
//            }
//        }
    }
}

void Compiler::beginCompile() {
    this->addInstruction(";; IrisCompiler GOGOGO");
    this->addInstruction("call @" + this->ast.getTopLambdaHandle());
    this->addInstruction("halt");

    // ( (lambda () ( bodies ) )
    for (auto lambdaHandle : this->ast.getLambdaHandles()) {
        this->compileLambda(lambdaHandle);
    }


}

#endif //TYPED_SCHEME_COMPILER_HPP
