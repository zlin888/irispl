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

    void compileApplication(Handle handle);

    void compileQuasiquote(Handle handle);

    void addComment(string inst);

    void compileDefine(Handle handle);
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

void Compiler::addComment(string inst) {
    boost::trim(inst);
    Instruction instruction(inst);
    if (inst.empty()) {
        return;
    } else {
        this->addInstruction(";; " + inst);
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

            if (schemeObjectType == SchemeObjectType::LAMBDA) {
                this->addInstruction("loadclosure @" + body);
            } else if (schemeObjectType == SchemeObjectType::QUASIQUOTE) {
                this->compileQuasiquote(body);
            } else if (schemeObjectType == SchemeObjectType::APPLICATION ||
                       schemeObjectType == SchemeObjectType::UNQUOTE) {
                this->compileApplication(body);
            }
        } else if (this->ast.hasNative(body)) {
            this->addInstruction("push " + body);
        } else if (bodyType == Type::VARIABLE) {
            this->addInstruction("load " + body);
        } else if (bodyType == Type::UNDEFINED) {
            throw std::runtime_error("[compileLambda] lambda body '" + body + "'type is undefined");
        } else {
            // TYPE is number || boolean || symbol || string ||keyword || port || quote
            this->addInstruction("load " + body);
        }
    }

    this->addInstruction("return");
}

void Compiler::compileApplication(Handle handle) {
    shared_ptr<ApplicationObject> applicationPtr = static_pointer_cast<ApplicationObject>(this->ast.get(handle));

    this->addComment(handle);

    auto childrenHoses =  applicationPtr->childrenHoses;

    if(childrenHoses.empty()) {
        return;
    }

    string first = childrenHoses[0];
    if(first == "import")       { return; }
    else if(first == "native")  { return; }
//    else if(first == "call/cc") { return CompileCallCC(nodeHandle); }
    else if(first == "define")  { return this->compileDefine(handle); }
//    else if(first == "set!")    { return CompileSet(nodeHandle); }
//    else if(first == "cond")    { return CompileCond(nodeHandle);}
//    else if(first == "if")      { return CompileIf(nodeHandle);}
//    else if(first == "and")     { return CompileAnd(nodeHandle);}
//    else if(first == "or")      { return CompileOr(nodeHandle);}
//    else if(first == "fork")    { AddInstruction(`fork ${children[1]}`); return;


}

void Compiler::compileDefine(Handle handle) {
    shared_ptr<ApplicationObject> applicationPtr = static_pointer_cast<ApplicationObject>(this->ast.get(handle));

    this->addComment("compileDefine"+ handle);

}

void Compiler::compileQuasiquote(Handle handle) {

}


void Compiler::beginCompile() {
    this->addInstruction(";; IrisCompiler GOGOGO");
    this->addInstruction("call @" + this->ast.getTopLambdaHandle());
    this->addInstruction("halt");

    // ( (lambda () ( bodies ) )
    for (auto lambdaHandle : this->ast.getLambdaHandles()) {
        this->compileLambda(lambdaHandle);
    }

    for (auto &inst : this->ILCode) {
        cout << inst.instructionStr << endl;
    }
}


#endif //TYPED_SCHEME_COMPILER_HPP
