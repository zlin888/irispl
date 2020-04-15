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
        Transfer::transferClass(ast);
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
                        string errorMessage = utils::createArgumentsNumberErrorMessage("class", 5,
                                                                                       applicationObjPtr->childrenHoses.size());
                        utils::raiseError(ast, handle, errorMessage, TRANSFER_PREFIX_TITLE);
                    }

                    // second children is variable
                    if (!utils::assertType(applicationObjPtr->childrenHoses[1], Type::VARIABLE)) {
                        string errorMessage = utils::createArgumentTypeErrorMessage("class", "first argument",
                                                                                    TypeStrMap[Type::VARIABLE],
                                                                                    utils::getActualTypeStr(ast,
                                                                                                            applicationObjPtr->childrenHoses[1]));
                        utils::raiseError(ast, handle, errorMessage,
                                          TRANSFER_PREFIX_TITLE);
                    }

                    // the rest of children are applications
                    for (int k = 2; k < 5; ++k) {
                        if (!utils::assertType(ast, applicationObjPtr->childrenHoses[k],
                                               SchemeObjectType::APPLICATION)) {
                            string errorMessage = utils::createArgumentTypeErrorMessage("class", "first argument",
                                                                                        SchemeObjectTypeStrMap[SchemeObjectType::APPLICATION],
                                                                                        applicationObjPtr->childrenHoses[k]);
                            utils::raiseError(ast, handle, errorMessage,
                                              TRANSFER_PREFIX_TITLE);
                        }
                    }


                    // (super (father-class arg0 arg1))
                    Handle originSuperAppHandle = applicationObjPtr->childrenHoses[3];
                    auto originSuperAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(originSuperAppHandle));

                    if (originSuperAppObjPtr->childrenHoses.size() != 2) {
                        string errorMessage = utils::createArgumentsNumberErrorMessage("class.super", 2,
                                                                                       originSuperAppObjPtr->childrenHoses.size());
                        utils::raiseError(ast, originSuperAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                    }

                    if (originSuperAppObjPtr->childrenHoses[0] != "super") {
                        string errorMessage = utils::createKeywordErrorMessage("class.super", "first argument",
                                                                               "super",
                                                                               originSuperAppObjPtr->childrenHoses[0]);
                        utils::raiseError(ast, originSuperAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                    }


                    // (class filter-cell  -> (value filter) <-
                    Handle originArgumentAppHandle = applicationObjPtr->childrenHoses[2];
                    auto originArgumentAppObjPtr = static_pointer_cast<ApplicationObject>(
                            ast.get(originArgumentAppHandle));

                    // they are all variable
                    for (int i = 0; i < originArgumentAppObjPtr->childrenHoses.size(); i++) {
                        auto hos = originArgumentAppObjPtr->childrenHoses[i];
                        if (!utils::assertType(hos, Type::VARIABLE)) {
                            string errorMessage = utils::createArgumentTypeErrorMessage("class.argument",
                                                                                        "argument " + to_string(i),
                                                                                        TypeStrMap[Type::VARIABLE],
                                                                                        utils::getActualTypeStr(
                                                                                                ast, hos));
                            utils::raiseError(ast, originArgumentAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                        }
                    }

                    // methods should be an application. each child should also be an application. childApp's children's
                    // size should be 2, the first child should be a variable
                    // ((store (lambda xxxx))
                    //  (double (lambda xxx)))
                    Handle originMethodsAppHandle = applicationObjPtr->childrenHoses[4];
                    auto originMethodsAppObjPtr = static_pointer_cast<ApplicationObject>(
                            ast.get(originMethodsAppHandle));

                    for (int i = 0; i < originMethodsAppObjPtr->childrenHoses.size(); i++) {
                        // application
                        if (!utils::assertType(ast, originMethodsAppObjPtr->childrenHoses[i],
                                               SchemeObjectType::APPLICATION)) {
                            string errorMessage = utils::createArgumentTypeErrorMessage("class.methods",
                                                                                        "method " + to_string(i),
                                                                                        SchemeObjectTypeStrMap[SchemeObjectType::APPLICATION],
                                                                                        utils::getActualTypeStr(
                                                                                                ast,
                                                                                                originMethodsAppObjPtr->childrenHoses[i]));
                            utils::raiseError(ast, originMethodsAppHandle, errorMessage,
                                              TRANSFER_PREFIX_TITLE);
                        }

                        // size of 2
                        Handle childAppHandle = originMethodsAppObjPtr->childrenHoses[i];
                        auto childAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(childAppHandle));
                        if (childAppObjPtr->childrenHoses.size() != 2) {
                            string errorMessage = utils::createArgumentsNumberErrorMessage("class.methods", 2,
                                                                                           childAppObjPtr->childrenHoses.size());
                            utils::raiseError(ast, childAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                        }

                        // first child is variable
                        if (!utils::assertType(childAppObjPtr->childrenHoses[0], Type::VARIABLE)) {
                            string errorMessage = utils::createArgumentTypeErrorMessage("class.methods",
                                                                                        "method " + to_string(i) +
                                                                                        "'s first argument",
                                                                                        TypeStrMap[Type::VARIABLE],
                                                                                        utils::getActualTypeStr(
                                                                                                ast,
                                                                                                childAppObjPtr->childrenHoses[0]));
                            utils::raiseError(ast, childAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                        }


                    }


                    // 1. change the class -> define
                    applicationObjPtr->childrenHoses[0] = "define";
                    // 2. create a lambda and set originArgumentAppHandle's children as it parameters
                    auto newLambdaHandle = ast.makeLambda(TRANSFER_PREFIX, applicationObjPtr->selfHandle);
                    auto newLambdaObjPtr = static_pointer_cast<LambdaObject>(ast.get(newLambdaHandle));

                    for (auto hos : originArgumentAppObjPtr->childrenHoses) {
                        newLambdaObjPtr->addParameter(hos);
                    }

                    // 3. let
                    auto letAppHandle = ast.makeApplication(TRANSFER_PREFIX, newLambdaHandle);
                    auto letAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(letAppHandle));
                    newLambdaObjPtr->addBody(letAppHandle);
                    letAppObjPtr->addChild("let");

                    auto bindingContainerAppHandle = ast.makeApplication(TRANSFER_PREFIX, letAppHandle);
                    auto bindingContainerAppObjPtr = static_pointer_cast<ApplicationObject>(
                            ast.get(bindingContainerAppHandle));
                    letAppObjPtr->addChild(bindingContainerAppHandle);

                    auto binding0AppHandle = ast.makeApplication(TRANSFER_PREFIX, bindingContainerAppHandle);
                    auto binding0AppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(binding0AppHandle));
                    bindingContainerAppObjPtr->addChild(binding0AppHandle);

                    binding0AppObjPtr->addChild("super");

                    // (super (father-class arg0 arg1))
                    // (super father-class)
                    // for the first case
                    if (typeOfStr(originSuperAppObjPtr->childrenHoses[1]) == Type::HANDLE) {
                        if (!utils::assertType(ast, originSuperAppObjPtr->childrenHoses[1],
                                               SchemeObjectType::APPLICATION)) {
                            string errorMessage = utils::createArgumentTypeErrorMessage("class.super",
                                                                                        "second argument",
                                                                                        SchemeObjectTypeStrMap[SchemeObjectType::APPLICATION],
                                                                                        utils::getActualTypeStr(
                                                                                                ast,
                                                                                                originSuperAppObjPtr->childrenHoses[1]));
                            utils::raiseError(ast, originSuperAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                        } else {
                            auto binding0InitAppHandle = ast.makeApplication(TRANSFER_PREFIX, binding0AppHandle);
                            auto binding0InitAppObjPtr = static_pointer_cast<ApplicationObject>(
                                    ast.get(binding0InitAppHandle));
                            binding0AppObjPtr->addChild(binding0InitAppHandle);

                            // move them
                            auto originSuperChildren1AppHandle = originSuperAppObjPtr->childrenHoses[1];
                            auto originSuperChildren1AppObjPtr = static_pointer_cast<ApplicationObject>(
                                    ast.get(originSuperChildren1AppHandle));
                            for (auto hos : originSuperChildren1AppObjPtr->childrenHoses) {
                                binding0InitAppObjPtr->addChild(hos);
                            }

                            // delete the children1 prevent it being deleted
                            // originSuperAppObjPtr->childrenHoses.pop_back();
                        }
                    } else {
                        // for the second case
                        if (!utils::assertType(originSuperAppObjPtr->childrenHoses[1],
                                               Type::VARIABLE)) {
                            string errorMessage = utils::createArgumentTypeErrorMessage("class.super",
                                                                                        "second argument",
                                                                                        TypeStrMap[Type::VARIABLE],
                                                                                        utils::getActualTypeStr(
                                                                                                ast,
                                                                                                originSuperAppObjPtr->childrenHoses[1]));
                            utils::raiseError(ast, originSuperAppHandle, errorMessage, TRANSFER_PREFIX_TITLE);
                        } else {
                            binding0AppObjPtr->addChild(originSuperAppObjPtr->childrenHoses[1]);
                        }
                    }

                    auto letBodyLambdaHandle = ast.makeLambda(TRANSFER_PREFIX, letAppHandle);
                    auto letBodyLambdaObjPtr = static_pointer_cast<LambdaObject>(ast.get(letBodyLambdaHandle));

                    letAppObjPtr->addChild(letBodyLambdaHandle);

                    letBodyLambdaObjPtr->addParameter("_selector");

                    auto condAppHandle = ast.makeApplication(TRANSFER_PREFIX, letBodyLambdaHandle);
                    auto condAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(condAppHandle));
                    condAppObjPtr->addChild("cond");

                    letBodyLambdaObjPtr->addBody(condAppHandle);

                    for (auto methodAppHandle : originMethodsAppObjPtr->childrenHoses) {
                        auto methodAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(methodAppHandle));

                        auto condBranchAppHandle = ast.makeApplication(TRANSFER_PREFIX, condAppHandle);
                        auto condBranchAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(condBranchAppHandle));
                        condAppObjPtr->addChild(condBranchAppHandle);

                        auto condBranchPredicateAppHandle = ast.makeApplication(TRANSFER_PREFIX, condBranchAppHandle);
                        auto condBranchPredicateAppObjPtr = static_pointer_cast<ApplicationObject>(
                                ast.get(condBranchPredicateAppHandle));
                        condBranchAppObjPtr->addChild(condBranchPredicateAppHandle);

//                        auto condBranchBodyAppHandle = ast.makeApplication(TRANSFER_PREFIX, condBranchAppHandle);
//                        auto condBranchBodyAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(condBranchBodyAppHandle));
//                        condBranchAppObjPtr->addChild(condBranchBodyAppHandle);

                        condBranchPredicateAppObjPtr->addChild("eq?");
                        condBranchPredicateAppObjPtr->addChild("_selector");

                        auto selectionQuoteHandle = ast.makeQuote(TRANSFER_PREFIX, condBranchPredicateAppHandle);
                        auto selectionQuoteObjPtr = static_pointer_cast<QuoteObject>(ast.get(selectionQuoteHandle));
                        condBranchPredicateAppObjPtr->addChild(selectionQuoteHandle);

                        selectionQuoteObjPtr->addChild("quote");
                        selectionQuoteObjPtr->addChild("'" + methodAppObjPtr->childrenHoses[0]);
                        condBranchAppObjPtr->addChild(methodAppObjPtr->childrenHoses[1]);
                        if (typeOfStr(methodAppObjPtr->childrenHoses[1]) == Type::HANDLE) {
                            auto schemeObjPtr = ast.get(methodAppObjPtr->childrenHoses[1]);
                            schemeObjPtr->parentHandle = condBranchAppHandle;
                        }
                    }

                    // cond else
                    auto condElseBranchAppHandle = ast.makeApplication(TRANSFER_PREFIX, condAppHandle);
                    auto condElseAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(condElseBranchAppHandle));
                    condAppObjPtr->addChild(condElseBranchAppHandle);
                    condElseAppObjPtr->addChild("else");

                    auto condElseBranchBodyAppHandle = ast.makeApplication(TRANSFER_PREFIX, condElseBranchAppHandle);
                    auto condElseBranchBodyAppObjPtr = static_pointer_cast<ApplicationObject>(ast.get(condElseBranchBodyAppHandle));
                    condElseAppObjPtr->addChild(condElseBranchBodyAppHandle);

                    condElseBranchBodyAppObjPtr->addChild("super");
                    condElseBranchBodyAppObjPtr->addChild("_selector");

                    // delete handle
                    // argument
                    ast.deleteHandle(originArgumentAppHandle);

                    // super
                    ast.deleteHandle(originSuperAppObjPtr->childrenHoses[1]);
                    ast.deleteHandle(originSuperAppHandle);

                    // methods
                    for (auto hos : originMethodsAppObjPtr->childrenHoses) {
                        ast.deleteHandle(hos);
                    }
                    ast.deleteHandle(originMethodsAppHandle);

                    applicationObjPtr->childrenHoses.pop_back();
                    applicationObjPtr->childrenHoses.pop_back();
                    applicationObjPtr->childrenHoses.pop_back();

                    applicationObjPtr->addChild(newLambdaHandle);
                }
            }
        }
    }
}


#endif //IRIS_TRANSFER_HPP
