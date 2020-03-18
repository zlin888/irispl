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

    int uniqueStrCounter = 0;

    explicit Compiler(AST ast) : ast(std::move(ast)) {};

    static vector<Instruction> compile(AST ast);

    void addInstruction(string inst);

    void beginCompile();

    void compileLambda(Handle lambdaHandle);

    void compileApplication(Handle handle);

    void compileQuasiquote(Handle handle);

    void addComment(string inst);

    void compileDefine(Handle handle);

    void compileComplexApplication(Handle handle);

    string makeUniqueString();

    void compileHos(HandleOrStr hos);
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
        this->compileHos(lambdaObjPtr->bodies[i]);
    }

    this->addInstruction("return");
}

void Compiler::compileHos(HandleOrStr hos) {
    Type hosType = typeOfStr(hos);

    if (hosType == Type::HANDLE) {
        auto schemeObjPtr = this->ast.get(hos);
        SchemeObjectType schemeObjectType = schemeObjPtr->schemeObjectType;

        if (schemeObjectType == SchemeObjectType::LAMBDA) {
            this->addInstruction("loadclosure @" + hos);
        } else if (schemeObjectType == SchemeObjectType::QUOTE) {
            this->addInstruction("push " + hos);
        } else if (schemeObjectType == SchemeObjectType::QUASIQUOTE) {
            this->compileQuasiquote(hos);
        } else if (schemeObjectType == SchemeObjectType::APPLICATION ||
                   schemeObjectType == SchemeObjectType::UNQUOTE) {
            this->compileApplication(hos);
        }
    } else if (this->ast.isNativeCall(hos)) {
        this->addInstruction("push " + hos);
    } else if (hosType == Type::VARIABLE) {
        this->addInstruction("load " + hos);
    } else if (hosType == Type::UNDEFINED) {
        throw std::runtime_error("[compileHos] hos '" + hos + "'type is undefined");
    } else {
        // TYPE is number || boolean || symbol || string ||keyword || port || quote
        this->addInstruction("push " + hos);
    }
}

void Compiler::compileApplication(Handle handle) {
    shared_ptr<ApplicationObject> applicationPtr = static_pointer_cast<ApplicationObject>(this->ast.get(handle));

    auto childrenHoses = applicationPtr->childrenHoses;

    if (childrenHoses.empty()) {
        return;
    }

    string first = childrenHoses[0];
    Type firstType = typeOfStr(first);
    if (first == "import") { return; }
    else if (first == "native") { return; }
//    else if(first == "call/cc") { return CompileCallCC(nodeHandle); }
    else if (first == "define") { return this->compileDefine(handle); }
//    else if(first == "set!")    { return CompileSet(nodeHandle); }
//    else if(first == "cond")    { return CompileCond(nodeHandle);}
//    else if(first == "if")      { return CompileIf(nodeHandle);}
//    else if(first == "and")     { return CompileAnd(nodeHandle);}
//    else if(first == "or")      { return CompileOr(nodeHandle);}
//    else if(first == "fork")    { AddInstruction(`fork ${children[1]}`); return;

    if (firstType == Type::HANDLE && this->ast.get(first)->schemeObjectType == SchemeObjectType::APPLICATION) {
        this->compileComplexApplication(handle);
        return;
    } else if (utils::makeSet<Type>(3, Type::HANDLE, Type::VARIABLE, Type::KEYWORD).count(firstType)) {
        // handle parameters
        for (int i = 1; i < childrenHoses.size(); ++i) {
            this->compileHos(childrenHoses[i]);
        }

        // 处理调用。需要做这样几件事情：
        // 1、确保首项是合法的可调用项，变量、Native、Primitive、Lambda
        // 2、处理import的外部变量名称（Native不必处理，保留原形）
        //    TODO 外部变量的处理方式根据整个系统对多模块的支持方式不同而不同。这里采取的策略是：暂不处理，交给运行时的模块加载器去动态地处理。
        // 3、处理尾递归

        // Primitive
        // handle the expected parameters better
        if (firstType == Type::KEYWORD) {
            if (this->primitiveInstructionMap.count(first)) {
                this->addInstruction(this->primitiveInstructionMap[first]);
            } else {
                this->addInstruction(first);
            }
        } else if (std::find(this->ast.tailcalls.begin(), this->ast.tailcalls.end(), handle) !=
                   this->ast.tailcalls.end()) {
            // we don't has tailcalls right now
            if(firstType == Type::HANDLE && this->ast.get(first)->schemeObjectType == SchemeObjectType::LAMBDA) {
                this->addInstruction("tailcall " + first);
            }
            else if(firstType == Type::VARIABLE) {
                // include native
                this->addInstruction("tailcall " + first);
            } else {
                throw std::runtime_error("[compileApplication] the first argument is not callable.");
            }
        } else {
            if(firstType == Type::HANDLE && this->ast.get(first)->schemeObjectType == SchemeObjectType::LAMBDA) {
                this->addInstruction("call " + first);
            }
            else if(firstType == Type::VARIABLE) {
                // include native
                this->addInstruction("call " + first);
            } else {
                throw std::runtime_error("[compileApplication] the first argument is not callable.");
            }
        }
    }
}

// eta-conversion
// (A 1 2 ..) → ((lambda (F x y ..) (F x y ..)) A 1 2 ..)
void Compiler::compileComplexApplication(Handle handle) {
    shared_ptr<ApplicationObject> applicationPtr = static_pointer_cast<ApplicationObject>(this->ast.get(handle));

    vector<HandleOrStr> childrenHoses = applicationPtr->childrenHoses;

    string uniqueStr = this->makeUniqueString();

    string entryLabel = "@ENTRY_LABEL_" + uniqueStr;
    this->addInstruction("goto " + entryLabel);

    // ------------------------------------------------------- TMP LAMBDA ----------------------------
    // a temporary lambda function (lambda (F x y ..) (F x y ..))
    string tmpLambdaLabel = "@TMP_LAMBDA_LABEL_" + uniqueStr;
    this->addInstruction(tmpLambdaLabel);

    vector<string> tmpLambdaParams;
    for (auto childrenHos : childrenHoses) {
        tmpLambdaParams.push_back(childrenHos);
    }

    for (int i = childrenHoses.size() - 1; i >= 0; --i) {
        this->addInstruction("store " + tmpLambdaParams[i]);
    }

    for (int i = 1; i < childrenHoses.size(); ++i) {
        this->addInstruction("load " + tmpLambdaParams[i]);
    }

    // tmpLambdaParams[0] is always a Handle(Application)!!
    // call it before further execution
    this->addInstruction("tailcall " + tmpLambdaParams[0]);
    this->addInstruction("return");
    // ------------------------------------------------------- TMP LAMBDA ----------------------------

    this->addInstruction(entryLabel);

    // Compile : (tmp_lambda A 1 2 ..)
    for (auto childrenHose : childrenHoses) {
        this->compileHos(childrenHose);
    }

    // call the tmp lambda
    this->addInstruction("call " + tmpLambdaLabel);

}

void Compiler::compileDefine(Handle handle) {
    shared_ptr<ApplicationObject> applicationPtr = static_pointer_cast<ApplicationObject>(this->ast.get(handle));

    auto childrenHoses = applicationPtr->childrenHoses;

    if (childrenHoses.size() != 3) {
        throw std::runtime_error("[compileDefine] define " + handle + " should have only two children");
    }

    if (typeOfStr(childrenHoses[1]) != Type::VARIABLE) {
        throw std::runtime_error(
                "[compileDefine] define's first argument " + childrenHoses[1] + " should be a variable");
    }

    if (typeOfStr(childrenHoses[2]) == Type::HANDLE) {
        auto schemeObjPtr = this->ast.get(childrenHoses[2]);

        if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
            this->addInstruction("push @" + childrenHoses[2]); // go to the label
        } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::QUOTE) {
            this->addInstruction("push " + childrenHoses[2]);
        } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::QUASIQUOTE) {
            this->compileQuasiquote(childrenHoses[2]);
        } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::STRING) {
            this->addInstruction("push " + childrenHoses[2]);
        } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION ||
                   schemeObjPtr->schemeObjectType == SchemeObjectType::UNQUOTE) {
            this->compileApplication(childrenHoses[2]);
        } else {
            throw std::runtime_error("[compileDefine] define's second argument " + childrenHoses[2] + " is invalid");
        }
    } else if (utils::makeSet<Type>(4, Type::NUMBER, Type::BOOLEAN, Type::KEYWORD, Type::PORT).count(
            typeOfStr(childrenHoses[2])) || this->ast.isNativeCall(childrenHoses[2])) {
        this->addInstruction("push " + childrenHoses[2]);
    } else if (typeOfStr(childrenHoses[2]) == Type::VARIABLE) {
        this->addInstruction("load " + childrenHoses[2]);
    } else {
        throw std::runtime_error("[compileDefine] define's second argument " + childrenHoses[2] + " is invalid");
    }

    // store
    this->addInstruction("store " + childrenHoses[1]);


}

void Compiler::compileQuasiquote(Handle handle) {

}

string Compiler::makeUniqueString() {
    string uniqueStr = this->ast.moduleName + ".UniqueStrID" + to_string(this->uniqueStrCounter);
    this->uniqueStrCounter++;
    return uniqueStr;
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
