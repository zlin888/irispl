//
// Created by bytedance on 2020/3/2.
//

#ifndef TYPED_SCHEME_ANALYSER_HPP
#define TYPED_SCHEME_ANALYSER_HPP

#include "SchemeObject.hpp"
#include "Parser.hpp"
#include "Utils.hpp"
#include "Transfer.hpp"
#include <map>
#include <set>

string ANALYZER_PREFIX = "_!!!analyzer_prefix!!!_";
string ANALYZER_PREFIX_TITLE = "Analyzer Error";

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

    string makeUniqueVariableName(Handle parentLambdaHandle, string variable);

    bool isNativeOrImportVariable(string variable);

//    static void analyse(AST &ast);

    static AST analyse(AST ast);

    void tailCallAnalyse();

    static void emptyApplicationDetection(AST &ast);

};

// from this handle, find the lambda that has bounded this variable
Handle Analyser::searchVariableLambdaHandle(string var, Handle fromHandle) {
    Handle currentHandle = fromHandle;
    while (currentHandle != TOP_NODE_HANDLE) {
        auto schemeObjPtr = this->ast.get(currentHandle);
        if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
            if (this->scopes[currentHandle].hasVariable(var)) {
                return currentHandle;
            }
        }
        currentHandle = schemeObjPtr->parentHandle;
    }
    return "";
}

string Analyser::makeUniqueVariableName(Handle parentLambdaHandle, string variable) {
    return parentLambdaHandle.substr(1, parentLambdaHandle.size()) + "." + variable;
}

// from this lambda handle, find the parent_lambda_handle
Handle Analyser::getParentLambdaHandle(Handle fromHandle) {
    Handle currentHandle = fromHandle;
    while (currentHandle != TOP_NODE_HANDLE) {
        auto schemeObjPtr = this->ast.heap.get(currentHandle);
        if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
            return currentHandle;
        }
        currentHandle = schemeObjPtr->parentHandle;
    }
    return "";
}

bool Analyser::isNativeOrImportVariable(string variable) {
    vector<string> fields;
    boost::split(fields, variable, boost::is_any_of("."));
    string variablePrefix = fields[0];
    if (!this->ast.natives.count(variablePrefix) &&
        !this->ast.moduleAliasPathMap.count(variablePrefix)) {
        return false;
    }
    return true;
}

// make sure each variable has its globally unique name
// therefore, we need check where does each variable defined, that is the scope
// with scope, we know where the variable is belong to so that we can change the name
// for example, (lambda (x y) (+ (+ x y) k)), we has x y, and we should change the name to
// some.module.name.lambda1.x and some.module.name.lambda12.k
// noted that k is defined somewhere else, so we need to track the scope of each variable to
// make sure the name changing is correct
// KEEP IN MIND !!!!!!!!!! ONLY LAMBDA AND APPLICATION HAS VARIABLE
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
            // add the parameters to the lambda bounded variables set
            for (string parameter : static_pointer_cast<LambdaObject>(schemeObjPtr)->parameters) {
                scopes[handle].boundVariables.insert(parameter);
            }
        } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
            auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
            if (applicationObjPtr->childrenHoses[0] == "define" || applicationObjPtr->childrenHoses[0] == "class") {
                // define will init variable, therefore, here, we can find out some of the scope of variables
                // class is actually a define. Usually we have (class apple xxx), and it is equivalent to (define apple xxx)
                Handle parentLambdaHandle = getParentLambdaHandle(handle);

                // code is like ( (lambda () (define xxxx xxx) ))
                // therefore, parentHandle always is not ""
                if (parentLambdaHandle != "") {
                    string variable = applicationObjPtr->childrenHoses[1];
                    scopes[parentLambdaHandle].addBoundVariables(variable);
                } else {
                    throw std::runtime_error(
                            "[scope analysis] error in 'define', where " + handle + "doesn't have parent lambda ");
                }
            }
        }
    }


    //change the name of all variable
    for (auto &handle : this->ast.getHandles()) {
        shared_ptr<SchemeObject> schemeObjPtr = this->ast.get(handle);

        if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
            auto lambdaObjPtr = static_pointer_cast<LambdaObject>(schemeObjPtr);

            //change the parameter names for lambda
            //(lambda (x y) (+ x y)) => (lambda (utils.lambda12.x , utils.lambda12.y) (+ x y))
            for (string &parameter : lambdaObjPtr->parameters) {
                string uniqueName = this->makeUniqueVariableName(handle, parameter);
                string originName = parameter;
                parameter = uniqueName;

                // TODO: Do I need to make a deep copy here??
                this->ast.varUniqueOriginNameMap[uniqueName] = originName;
            }

            //handle the variable directly exists in lambda
            //(lambda (k) *k*) => (lambda (k) (utils.lambda12.k))
            for (HandleOrStr &body : lambdaObjPtr->bodies) {
                if (typeOfStr(body) == Type::VARIABLE) {
                    // TODO handle body is not defined
                    Handle boundLambdaHandle = this->searchVariableLambdaHandle(body, handle);
                    //Can't find the lambda which bounds this variable
                    //Variable is not defined or is defined in other module (Utils.max_number)
                    if (boundLambdaHandle.empty()) {
                        if (!this->isNativeOrImportVariable(body)) {
                            throw std::runtime_error("[scope analysis] variable " + body + " is not defined");
                        }
                    } else {
                        string uniqueName = this->makeUniqueVariableName(boundLambdaHandle, body);
                        string originName = body;
                        body = uniqueName;

                        this->ast.varUniqueOriginNameMap[uniqueName] = originName;
                    }
                }
            }

        } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION ||
                   schemeObjPtr->schemeObjectType == SchemeObjectType::UNQUOTE ||
                   schemeObjPtr->schemeObjectType == SchemeObjectType::QUASIQUOTE) {

            auto &childrenHoses = SchemeObject::getChildrenHosesOrBodies(schemeObjPtr);
            Handle parentHandle = schemeObjPtr->parentHandle;

            // change the variable name in application, unquote and quasiquote
            // firstly, pass some special APPLICATION!o!
            if (childrenHoses.empty() || childrenHoses[0] == "native" || childrenHoses[0] == "import") {
                continue;
            }

            for (HandleOrStr &hos : childrenHoses) {
                if (typeOfStr(hos) == Type::VARIABLE) {
                    // find the bounded lambda
                    Handle boundLambdaHandle;
                    try {
                        boundLambdaHandle = this->searchVariableLambdaHandle(hos, handle);
                    } catch (exception &e) {
                        auto objPtr = static_pointer_cast<ApplicationObject>(ast.get(handle));
                        utils::raiseError(ast, handle, "hos " + hos + "'s parent Handle " + handle + " not found", ANALYZER_PREFIX);
                    }

                    if (boundLambdaHandle.empty()) {
                        if (!this->isNativeOrImportVariable(hos)) {
                            string errorMessage = utils::createVariableUndefinedMessage(hos);
                            utils::raiseError(ast, schemeObjPtr->selfHandle, errorMessage, ANALYZER_PREFIX_TITLE);
                        }
                    } else {
                        string uniqueName = this->makeUniqueVariableName(boundLambdaHandle, hos);
                        string originName = hos;
                        hos = uniqueName;
                        this->ast.varUniqueOriginNameMap[uniqueName] = originName;
                    }
                }
            }

            // finally, set the definedVarUniqueOriginNameMap,
            // Because the name of the defined variable has been changed
            if (childrenHoses[0] == "define" && parentHandle == this->ast.getTopLambdaHandle()) {
                string uniqueName = childrenHoses[1];
                string originName = this->ast.varUniqueOriginNameMap[uniqueName];

                if (this->ast.definedVarOriginUniqueNameMap.count(originName)) {
                    string errorMessage = utils::createRepeatedDefinitionMessage(originName);
                    utils::raiseError(ast, schemeObjPtr->selfHandle, errorMessage, ANALYZER_PREFIX_TITLE);
                    throw std::runtime_error("[scope analysis] define variable " + originName + " repeatedly");
                } else {
                    this->ast.definedVarOriginUniqueNameMap[originName] = uniqueName;
                }
            }
        }
    }
}

void Analyser::tailCallAnalyse() {

}

 void Analyser::emptyApplicationDetection(AST &ast) {
    for (auto handle : ast.getHandles()) {
        shared_ptr<SchemeObject> schemeObjPtr;
        schemeObjPtr = ast.get(handle);

        if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
            auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
            if (applicationObjPtr->childrenHoses.empty()) {
                string errorMessage = utils::createEmptyApplicationMessage();
                utils::raiseError(ast, handle, errorMessage, ANALYZER_PREFIX_TITLE);
            }
        }
    }
}

AST Analyser::analyse(AST ast) {
    Analyser::emptyApplicationDetection(ast);
    Transfer::transfer(ast);
    Analyser analyser(ast);
    analyser.scopeAnalyse();
    // TODO: tail call analyse
    return analyser.ast;
}

#endif //TYPED_SCHEME_ANALYSER_HPP
