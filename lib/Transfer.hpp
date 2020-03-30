//
// Created by bytedance on 2020/3/29.
//

#ifndef IRIS_TRANSFER_HPP
#define IRIS_TRANSFER_HPP

#include "Parser.hpp"
#include "Heap.hpp"
#include "SchemeObject.hpp"

namespace Transfer {

    string TRANSFER_PREFIX = "_!!!transfer_prefix!!!_";

    void transferLet(AST &ast);

    void transfer(AST &ast) {
        Transfer::transferLet(ast);
    }

    void transferLet(AST &ast) {
        for (auto handle : ast.getHandles()) {
            shared_ptr<SchemeObject> schemeObjPtr;
            try {
                schemeObjPtr = ast.get(handle);
            } catch (exception &e) {
                // the code below will delete some applicationObj from heap, therefore, we need to skip the deleted handle here
                continue;
            }

            // find let application
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {

                auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
                if (applicationObjPtr->childrenHoses[0] == "let") {

                    if (applicationObjPtr->childrenHoses.size() != 3) {
                        throw std::runtime_error(
                                "[transfer] let syntex distakes " + to_string(ast.handleSourceIndexesMap[handle]));
                    }

                    // (let bindingApplication expression(hos))
                    Handle bindingsHandle = applicationObjPtr->childrenHoses[1];
                    auto bindingsAppPtr = static_pointer_cast<ApplicationObject>(ast.get(bindingsHandle));

                    HandleOrStr expressionHandle = applicationObjPtr->childrenHoses[2];

                    // transfer the let application to (lambda argument...)
                    // create a lambda first
                    Handle lambdaHandle = ast.heap.makeLambda(TRANSFER_PREFIX, applicationObjPtr->selfHandle);
                    auto lambdaObjPtr = static_pointer_cast<LambdaObject>(ast.get(lambdaHandle));
                    ast.addLambdaHandle(lambdaHandle);

                    // add the lambda to the let application, and erase the let application children (we have take them out)
                    applicationObjPtr->childrenHoses[0] = lambdaHandle;
                    applicationObjPtr->childrenHoses.erase(applicationObjPtr->childrenHoses.begin() + 1);
                    applicationObjPtr->childrenHoses.erase(applicationObjPtr->childrenHoses.begin() + 1);


                    // expression can be a schemeobject, if so change its parent to the let lambda
                    if (typeOfStr(expressionHandle) == Type::HANDLE) {
                        auto expressionAppPtr = ast.get(expressionHandle);
                        expressionAppPtr->parentHandle = lambdaObjPtr->selfHandle;
                    }

                    lambdaObjPtr->addBody(expressionHandle);

                    // begin to handle to (binding init) application
                    // put the binding as the parameter of the let lambda
                    // put the init as the argument of the let lambda
                    for (Handle bindingHandle: bindingsAppPtr->childrenHoses) {
                        auto bindingSchemeObjptr = ast.get(bindingHandle);
                        if (bindingSchemeObjptr->schemeObjectType == SchemeObjectType::APPLICATION) {
                            auto bindingAppObjPtr = static_pointer_cast<ApplicationObject>(bindingSchemeObjptr);
                            if (bindingAppObjPtr->childrenHoses.size() != 2) {
                                throw std::runtime_error(
                                        "[transfer] lexical binding should be pairs of variable and init " +
                                        to_string(ast.handleSourceIndexesMap[handle]));
                            }
                            if (typeOfStr(bindingAppObjPtr->childrenHoses[0]) != Type::VARIABLE) {
                                throw std::runtime_error(
                                        "[transfer] lexical binding's first argument should be a variable, but get a " +
                                        bindingsAppPtr->childrenHoses[0] + " " +
                                        to_string(ast.handleSourceIndexesMap[handle]));
                            }

                            // add as parameter
                            lambdaObjPtr->addParameter(bindingAppObjPtr->childrenHoses[0]);

                            // them are originally in the bindingApplication
                            // thus, if init is a handle, change its parent to the let application
                            if (typeOfStr(bindingAppObjPtr->childrenHoses[1]) == Type::HANDLE) {
                                auto initSchemeObjptr = ast.get(bindingAppObjPtr->childrenHoses[1]);
                                initSchemeObjptr->parentHandle = applicationObjPtr->selfHandle;
                            }
                            applicationObjPtr->childrenHoses.push_back(bindingAppObjPtr->childrenHoses[1]);

                            ast.heap.deleteHandle(bindingHandle);
                        }
                    }
                }
            }
        }
    }
}


#endif //IRIS_TRANSFER_HPP
