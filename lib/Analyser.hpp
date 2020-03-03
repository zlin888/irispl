//
// Created by bytedance on 2020/3/2.
//

#ifndef TYPED_SCHEME_ANALYSER_HPP
#define TYPED_SCHEME_ANALYSER_HPP

#include "SchemeObject.hpp"
#include "Parser.hpp"
#include <map>
#include <set>


namespace Analyser {
    class Scope {
    public:
        Handle parentHandle;
        vector<HandleOrStr> children;
        set<string> boundVariables;

        explicit Scope(const Handle &parentHandle) : parentHandle(parentHandle) {
            this->parentHandle = parentHandle;
        }

        Scope() {
            this->parentHandle = "";
        }

        void addChild(HandleOrStr child);

        void addBoundVariables(string var);

        bool hasVariable(string var);
    };

    void Scope::addChild(HandleOrStr child) {
        this->children.push_back(child);
    }

    void Scope::addBoundVariables(string var) {
        if (!this->hasVariable(var)) {
            this->boundVariables.insert(var);
        }
    }

    bool Scope::hasVariable(string var) {
        return this->boundVariables.count(var);
    }


    class Analyser {
    public:
        map<Handle, Scope> scopes;
        AST ast;

        Analyser(AST &ast) : ast(ast) {};

        string searchVariableLambdaHandle(string var, Handle fromHandle);

        Handle getParentLambdaHandle(Handle fromHandle);

        void scopeAnalyse();
    };

    // from this handle, find the lambda that has bounded this variable
    Handle Analyser::searchVariableLambdaHandle(string var, Handle fromHandle) {
        Handle currentHandle = fromHandle;
        while (currentHandle != TOP_NODE_HANDLE) {
            auto schemeObjPtr = this->ast.heap.get(currentHandle);
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
                if (this->scopes[currentHandle].hasVariable(var)) {
                    return currentHandle;
                }
            }
        }
        return "";
    }

    // from this lambda handle, find the parent_lambda_handle
    Handle Analyser::getParentLambdaHandle(Handle fromHandle) {
        Handle currentHandle = fromHandle;
        while (currentHandle != TOP_NODE_HANDLE) {
            auto schemeObjPtr = this->ast.heap.get(currentHandle);
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
                return currentHandle;
            }
        }
        return "";
    }

    // make sure each variable has its globally unique name
    // therefore, we need check where does each variable defined, that is the scope
    // with scope, we know where the variable is belong to so that we can change the name
    // for example, (lambda (x y) (+ (+ x y) k)), we has x y, and we should change the name to
    // some.module.name.lambda1.x and some.module.name.lambda12.k
    // noted that k is defined somewhere else, so we need to track the scope of each variable to
    // make sure the name changing is correct
    void Analyser::scopeAnalyse() {
        Handle topLambdaHandle = this->ast.getTopLambdaHandle();

        // init scopes, each lambda handle responses to one scope
        for (auto lambdaHandle : this->ast.getLambdaHandles()) {
            Scope scope;
            scopes[lambdaHandle] = scope;
        }

        // 1. init the scopes, scope's parent and children
        // 2. put the defined variable to its corresponding scope
        for (auto handle : this->ast.getHandles()) {
            shared_ptr<SchemeObject> schemeObjPtr = this->ast.get(handle);

            // init the scope for each lambda
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
                Handle parentHandle = getParentLambdaHandle(handle);
                // not the top lambda
                if (!parentHandle.empty()) {
                    this->scopes[handle].parentHandle = parentHandle;
                    this->scopes[parentHandle].addChild(handle);
                } else {
                    scopes[handle].parentHandle = TOP_NODE_HANDLE;
                }
            } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
                auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
                if (applicationObjPtr->childrenHoses[0] == "define") {
                    // define will init variable, therefore, here, we can find out some of the scope of some varaible
                    Handle parentLambdaHandle = getParentLambdaHandle(handle);

                    // code is like ( (lambda () (define xxxx xxx) ))
                    // therefore, parentHandle always is not ""
                    if (parentLambdaHandle != "") {
                        string variable = applicationObjPtr->childrenHoses[1];
                        scopes[parentLambdaHandle].addBoundVariables(variable);
                    } else {
                        throw std::runtime_error("[scope analysis] error in 'define'");
                    }
                }
            }
        }


    }


    void analyse(AST &ast) {
        Analyser analyser(ast);
        analyser.scopeAnalyse();
    }


}

#endif //TYPED_SCHEME_ANALYSER_HPP
