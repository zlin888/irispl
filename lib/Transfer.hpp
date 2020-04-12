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
    string TRANSFER_PREFIX_TITLE = "Transfer Error";

    void transferLet(AST &ast);

    void transferClass(AST &ast);

    void raiseError(AST &ast, Handle handle, string message);

    void transfer(AST &ast) {
        Transfer::transferLet(ast);
        Transfer::transferClass(ast);
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
                if (applicationObjPtr->childrenHoses.size() >= 1 && applicationObjPtr->childrenHoses[0] == "let") {

                    if (applicationObjPtr->childrenHoses.size() != 3) {
                        throw std::runtime_error(
                                "[transfer] let syntex distakes " + to_string(ast.sourceCodeMapper.getIndex(handle)));
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
                                        to_string(ast.sourceCodeMapper.getIndex(handle)));
                            }
                            if (typeOfStr(bindingAppObjPtr->childrenHoses[0]) != Type::VARIABLE) {
                                throw std::runtime_error(
                                        "[transfer] lexical binding's first argument should be a variable, but get a " +
                                        bindingsAppPtr->childrenHoses[0] + " " +
                                        to_string(ast.sourceCodeMapper.getIndex(handle)));
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

    void transferClass(AST &ast) {
        for (auto handle : ast.getHandles()) {
            shared_ptr<SchemeObject> schemeObjPtr;
            try {
                schemeObjPtr = ast.get(handle);
            } catch (exception &e) {
                // the code below will delete some applicationObj from heap, therefore, we need to skip the deleted handle here
                continue;
            }


            if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
                auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
                if (applicationObjPtr->childrenHoses[0] == "class") {
                    // inside of the class application

                    // class should has 5 children
                    if (applicationObjPtr->childrenHoses.size() != 5) {
                        string errorMessage = utils::createWrongArgumentsNumberErrorMessage("class", 5,
                                                                                            applicationObjPtr->childrenHoses.size());
                        utils::raiseError(ast, handle, errorMessage, TRANSFER_PREFIX_TITLE);
                    }

                    // second children is variable
                    if (!utils::assertType(applicationObjPtr->childrenHoses[1], Type::VARIABLE)) {
                        string errorMessage = utils::createWrongArgumentTypeErrorMessage("class", "first argument",
                                                                                         TypeStrMap[Type::VARIABLE],
                                                                                         utils::getActualTypeStr(ast,
                                                                                                                 applicationObjPtr->childrenHoses[1]));
                        utils::raiseError(ast, handle, errorMessage, TRANSFER_PREFIX_TITLE);
                    }

                    // the rest of children are applications
                    for (int k = 2; k < 5; ++k) {
                        if (!utils::assertType(ast, applicationObjPtr->childrenHoses[k],
                                               SchemeObjectType::APPLICATION)) {
                            string errorMessage = utils::createWrongArgumentTypeErrorMessage("class", "first argument",
                                                                                             SchemeObjectTypeStrMap[SchemeObjectType::APPLICATION],
                                                                                             applicationObjPtr->childrenHoses[k]);
                            utils::raiseError(ast, handle, errorMessage, TRANSFER_PREFIX_TITLE);
                        }
                    }


                    // 1. change the class -> define
                    applicationObjPtr->childrenHoses[0] = "define";
                    // 2. create a lambda and set childrenHoses[2] as it parameters
                    auto newLambdaHandle = ast.makeLambda(TRANSFER_PREFIX, applicationObjPtr->selfHandle);
                    auto newLambdaObjPtr = static_pointer_cast<LambdaObject>(ast.get(newLambdaHandle));
                    auto childrenHos2AppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(applicationObjPtr->childrenHoses[2]));
                    for (auto hos : childrenHos2AppObjPtr->childrenHoses) {
                        newLambdaObjPtr->addParameter(hos);
                    }
                    // 3. 

                    for (int j = 0; j < applicationObjPtr->childrenHoses.size(); ++j) {
                        auto hos = applicationObjPtr->childrenHoses[j];
                        if (typeOfStr(hos) == Type::HANDLE) {
                            auto hosObjPtr = ast.get(hos);
                            if (hosObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
                                auto hosAppObjPtr = static_pointer_cast<ApplicationObject>(hosObjPtr);
                                int i = 1;
                            }
                        }
                    }

                }
            }
        }
    }
}


#endif //IRIS_TRANSFER_HPP
