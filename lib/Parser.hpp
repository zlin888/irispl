//
// Created by zlin on 2020/2/17.
//

#ifndef TYPED_SCHEME_PARSER_HPP
#define TYPED_SCHEME_PARSER_HPP

#include <string>
#include <iostream>
#include "Lexer.hpp"
#include "SchemeObject.hpp"
#include "Heap.hpp"
#include <stdexcept>
#include <boost/algorithm/string.hpp>

using namespace std;

class AST {
public:
    AST() {};

    AST(const string &source, string &moduleName) : source(source), moduleName(moduleName) {};

//    //Copy constructor, remember to change it when changing the fields list
//    AST(const AST &ast);


    Heap heap;
    string moduleName;
    string source;
    map<Handle, int> nodeSourceIndexesMap;
    map<string, string> moduleAliasPathMap;
    map<string, string> natives; // Not supported yet
    map<string, string> varUniqueOriginNameMap;
    // Fort the definedVar Origin Name is unique, but for other variables,
    // origin name could be duplicated, and this is exactly why we need unique name
    map<string, string> definedVarOriginUniqueNameMap;
    vector<Handle> lambdaHandles;
    vector<Handle> tailcalls;


    Handle getTopApplicationHandle();

    Handle getTopLambdaHandle();

    vector<Handle> getLambdaHandles();

    vector<Handle> getHandles();

    inline shared_ptr<SchemeObject> get(Handle handle) { return this->heap.get(handle); };

    vector<HandleOrStr> getTopLambdaBodies();

    void mergeAST(AST anotherAST);
};

Handle AST::getTopApplicationHandle() {
    // ((lambda () " + code + "))
    // the first sList
    Handle topApplicationHandle;
    for (auto &[handle, schemeObjPtr] : this->heap.dataMap) {
        if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION &&
            static_pointer_cast<ApplicationObject>(schemeObjPtr)->parentHandle == TOP_NODE_HANDLE) {
            topApplicationHandle = handle;
            break;
        }
    }
    if (topApplicationHandle.empty()) {
        throw runtime_error("topApplicationHandle not found");
    }
    return topApplicationHandle;
}


Handle AST::getTopLambdaHandle() {
    // ((lambda () " + code + "))
    // return the first lambda, which is the top lambda
    return static_pointer_cast<ApplicationObject>(this->heap.get(this->getTopApplicationHandle()))->childrenHoses[0];
}

vector<HandleOrStr> AST::getTopLambdaBodies() {
    // return by value !!!!!!!!!!!!!!
    return static_pointer_cast<LambdaObject>(this->heap.get(getTopApplicationHandle()))->bodies;
}

void AST::mergeAST(AST anotherAST) {

    this->source = anotherAST.source + "\n" + this->source;

    // merge heap
    for (auto &[handle, schemeObjPtr] : anotherAST.heap.dataMap) {
        this->heap.set(handle, schemeObjPtr);
    }

    // merge the top_node_lambda's body
    // the top_node_lambda's body (lambda () (*))
    // we only need to lambda's body because the top lambda not has any argument
    vector<HandleOrStr> anotherTopLambdaBodies = anotherAST.getTopLambdaBodies();

    Handle thisTopLambdaHandle = this->getTopLambdaHandle();
    vector<HandleOrStr> thisTopLambdaBodies = this->getTopLambdaBodies(); // return by value !!!!

    // append the thisTopLambdaBodies to the otherTopLambdaBodies
    anotherTopLambdaBodies.insert(anotherTopLambdaBodies.end(), thisTopLambdaBodies.begin(), thisTopLambdaBodies.end());

    static_pointer_cast<LambdaObject>(this->heap.get(this->getTopLambdaHandle()))->setBodies(anotherTopLambdaBodies);

    // Now we have, this ((lambda () (this bodies + anthoer bodies))
    // Then we need to change other's bodies' handles' parentHandle to thisLambda
    for (auto &handle : anotherTopLambdaBodies) {
        // only handle exists in the bodies
        this->heap.get(handle)->parentHandle = this->getTopLambdaHandle();
    }

    // we add them to the heap of thisAST
    this->heap.deleteHandle(anotherAST.getTopLambdaHandle());
    this->heap.deleteHandle(anotherAST.getTopApplicationHandle());

    // the heap have been merge, but not the other fields of ast
    // merge each field one by one
    for (auto[source, index] : this->nodeSourceIndexesMap) {
        this->nodeSourceIndexesMap[source] = index + anotherAST.source.size() + 1;
    }

    for (auto[source, index] : anotherAST.nodeSourceIndexesMap) {
        this->nodeSourceIndexesMap[source] = index;
    }

    for (auto lambdaHandle : anotherAST.getLambdaHandles()) {
        if (lambdaHandle != anotherAST.getTopLambdaHandle()) {
            this->lambdaHandles.push_back(lambdaHandle);
        }
    }

    for (auto handle : anotherAST.tailcalls) {
        if (handle != anotherAST.getTopApplicationHandle()) {
            this->tailcalls.push_back(handle);
        }
    }

    for (auto[unique, origin]: anotherAST.varUniqueOriginNameMap) {
        this->varUniqueOriginNameMap[unique] = origin;
    }

    for (auto[origin, unique] : anotherAST.definedVarOriginUniqueNameMap) {
        this->definedVarOriginUniqueNameMap[origin] = unique;
    }

    for (auto[alias, path] : anotherAST.moduleAliasPathMap) {
        this->moduleAliasPathMap[alias] = path;
    }

    for (auto[s1, s2] : anotherAST.natives) {
        this->natives[s1] = s2;
    }


}

vector<Handle> AST::getLambdaHandles() {
    if (this->lambdaHandles.empty()) {
        for (auto &[handle, schemeObjPtr] : this->heap.dataMap) {
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
                this->lambdaHandles.push_back(handle);
            }
        }
    }
    return this->lambdaHandles;
}

vector<Handle> AST::getHandles() {
    vector<Handle> result;
    for (auto &[handle, _] : this->heap.dataMap) {
        result.push_back(handle);
    }
    return result;
}

//AST::AST(const AST &ast) : heap(ast.heap), moduleName(ast.moduleName), source(ast.source),
//                           nodeSourceIndexesMap(ast.nodeSourceIndexesMap), moduleAliasPathMap(ast.moduleAliasPathMap),
//                           natives(ast.natives), varUniqueOriginNameMap(ast.varUniqueOriginNameMap),
//                           definedVarUniqueOriginNameMap(ast.definedVarUniqueOriginNameMap) {}

class Parser {
public:
    vector<string> stateStack;
    vector<HandleOrStr> nodeStack; //nodeStack can store both Handle and string

    // One Parser has one AST !!!
    AST ast;
    vector<Lexer::Token> tokens;

    int parseTerm(int index);

    Parser(const vector<Lexer::Token> &tokens, const string &moduleName) : tokens(tokens) {
        this->nodeStack.push_back(TOP_NODE_HANDLE);
        this->ast.moduleName = moduleName;
    }

    static AST parse(const vector<Lexer::Token> &tokens, const string &moduleName, const string &code);

    void parseLog(const string &msg);

    bool isSymbol(const string &tokenStr);

    int parseLambda(int index);

    int parseArgList(int index);

    int parseArgListSeq(int index);

    int parseArgSymbol(int index);

    int parseBody(int index);

    int parseBodyTail(int index);

    int parseBodyTerm(int index);

    int parseQuote(int index);

    int parseUnquote(int index);

    int parseUnquoteTerm(int index);

    int parseQuasiquote(int index);

    int parseQuasiquoteTerm(int index);

    int parseSList(int index);

    int parseSymbol(int index);

    int parseQuoteTerm(int index);

    int parseSListSeq(int index);

    void preProcessAnalysis();

};


AST Parser::parse(const vector<Lexer::Token> &tokens, const string &moduleName, const string &code) {
    Parser parser(tokens, moduleName);
    parser.parseTerm(0);
    parser.preProcessAnalysis();
    parser.ast.source = code;
    return parser.ast;
}

int Parser::parseTerm(int index) {
    int nextIndex;
    string quoteState = this->stateStack.empty() ? "" : this->stateStack.back();

    if (quoteState != "QUOTE" && quoteState != "QUASIQUOTE" && this->tokens[index].string == "(" &&
        this->tokens[index + 1].string == "lambda") {
        this->parseLog("<Term> → <Lambda>");
        return this->parseLambda(index);
    } else if (this->tokens[index].string == "(" && this->tokens[index + 1].string == "quote") {
        this->parseLog("<Term> → <Quote>");
        nextIndex = this->parseQuote(index + 1);
        if (this->tokens[nextIndex].string == ")") { return nextIndex + 1; }
        else {
            throw runtime_error("[Error] quote 右侧括号未闭合。");
        }
    } else if (this->tokens[index].string == "(" && this->tokens[index + 1].string == "unquote") {
        this->parseLog("<Term> → <Unquote>");
        nextIndex = this->parseUnquote(index + 1);
        if (this->tokens[nextIndex].string == ")") { return nextIndex + 1; }
        else {
            throw runtime_error("[Error] unquote 右侧括号未闭合。");
        }
    } else if (this->tokens[index].string == "(" && this->tokens[index + 1].string == "quasiquote") {
        this->parseLog("<Term> → <Quasiquote>");
        nextIndex = parseQuasiquote(index + 1);
        if (this->tokens[nextIndex].string == ")") { return nextIndex + 1; }
        else {
            throw runtime_error("[Error] quasiquote 右侧括号未闭合。");
        }
    } else if (this->tokens[index].string == "\'") {
        this->parseLog("<Term> → <Quote>");
        return this->parseQuote(index);
    } else if (this->tokens[index].string == ",") {
        this->parseLog("<Term> → <Unquote>");
        return this->parseUnquote(index);
    } else if (this->tokens[index].string == "`") {
        this->parseLog("<Term> → <Quasiquote>");
        return this->parseQuasiquote(index);
    } else if (this->tokens[index].string == "(") {
        this->parseLog("<Term> → <SList>");
        return this->parseSList(index);
    } else if (isSymbol(this->tokens[index].string)) {
        this->parseLog("<Term> → <Symbol>");
        return this->parseSymbol(index);
    } else {
        throw runtime_error("undefined token " + this->tokens[index].string);
    }
}

int Parser::parseLambda(int index) {
    this->parseLog("<Lambda> → ( ※ lambda <ArgList> <Body> )");

    Handle lambdaHandle = this->ast.heap.makeLambda(this->ast.moduleName, this->nodeStack.back());
    this->nodeStack.push_back(lambdaHandle);

    ast.nodeSourceIndexesMap[lambdaHandle] = this->tokens[index].sourceIndex;
    ast.lambdaHandles.push_back(lambdaHandle);

    int nextIndex = this->parseArgList(index + 2);
    nextIndex = this->parseBody(nextIndex);

    if (this->tokens[nextIndex].string == ")") {
        return nextIndex + 1;
    } else {
        throw runtime_error("<Lambda> ')' is not found -- in " + to_string(tokens[index].sourceIndex));
    }
}

int Parser::parseArgList(int index) {
    this->parseLog("<ArgList> → ( ※1 <ArgListSeq> ※2)");

    if (tokens[index].string != "(") {
        throw runtime_error("<ArgList> '(' is not found -- in " + to_string(tokens[index].sourceIndex));
    }

    this->stateStack.push_back("PARAMETER");
    int nextIndex = this->parseArgListSeq(index + 1);
    // Action2
    this->stateStack.pop_back();

    if (tokens[nextIndex].string == ")") {
        return nextIndex + 1;
    } else {
        throw runtime_error("<ArgList> ')' is not found -- in " + to_string(tokens[index].sourceIndex));
    }
}

int Parser::parseArgListSeq(int index) {
    this->parseLog("<ArgListSeq> → <ArgSymbol> ※ <ArgListSeq> | ε");
    if (this->isSymbol(tokens[index].string)) {
        int nextIndex = this->parseArgSymbol(index);

        // get symbol from nodeStack, and add it to the parameters of lambda node
        string parameter = this->nodeStack.back();
        this->nodeStack.pop_back();
        auto lambdaObjPtr = static_pointer_cast<LambdaObject>(this->ast.heap.get(this->nodeStack.back()));
        if (!lambdaObjPtr->addParameter(parameter)) {
            throw runtime_error("two parameter will same name");
        }
        nextIndex = this->parseArgListSeq(nextIndex);
        return nextIndex;
    } else {
        return index;
    }
}

int Parser::parseArgSymbol(int index) {
    parseLog("<ArgSymbol> → <Symbol>");
    return this->parseSymbol(index);
}

int Parser::parseBody(int index) {
    parseLog("<Body> → <BodyTerm> ※ <Body_>");
    int nextIndex = this->parseBodyTerm(index);

    // Action：从节点栈顶弹出节点，追加到新栈顶Lambda节点的body中。
    HandleOrStr bodyHos = this->nodeStack.back();
    this->nodeStack.pop_back();
    static_pointer_cast<LambdaObject>(this->ast.heap.get(this->nodeStack.back()))->addBody(bodyHos);

    nextIndex = this->parseBodyTail(nextIndex);
    return nextIndex;
}

int Parser::parseBodyTail(int index) {
    this->parseLog("<Body_> → <BodyTerm> ※ <Body_> | ε");
    string currentToken = tokens[index].string;
    if (currentToken == "(" || currentToken == "'" || currentToken == "," ||
        currentToken == "`" || isSymbol(currentToken)) {
        int nextIndex = this->parseBodyTerm(index);

        // Action：从节点栈顶弹出节点，追加到新栈顶Lambda节点的body中。
        HandleOrStr bodyHos = this->nodeStack.back();
        this->nodeStack.pop_back();
        static_pointer_cast<LambdaObject>(this->ast.heap.get(this->nodeStack.back()))->addBody(bodyHos);

        nextIndex = this->parseBodyTail(nextIndex);
        return nextIndex;
    } else {
        return index;
    }
}

int Parser::parseBodyTerm(int index) {
    this->parseLog("<BodyTerm> → <Term>");
    return this->parseTerm(index);
}

int Parser::parseQuote(int index) {
    this->parseLog("<Quote> → ' ※1 <QuoteTerm> ※2");
    // Action1
    this->stateStack.push_back("QUOTE");
    int nextIndex = this->parseQuoteTerm(index + 1);
    // Action2
    this->stateStack.pop_back();
    return nextIndex;
}

int Parser::parseQuoteTerm(int index) {
    this->parseLog("<QuoteTerm> → <Term>");
    return this->parseTerm(index);
}

int Parser::parseUnquote(int index) {
    this->parseLog("<Unquote> → , ※1 <UnquoteTerm> ※2");
    // Action1
    this->stateStack.push_back("UNQUOTE");
    int nextIndex = this->parseUnquoteTerm(index + 1);
    // Action2
    this->stateStack.pop_back();
    return nextIndex;
}

int Parser::parseUnquoteTerm(int index) {
    this->parseLog("<UnquoteTerm> → <Term>");
    return this->parseTerm(index);
}

int Parser::parseQuasiquote(int index) {
    this->parseLog("<Quasiquote> → ` ※1 <QuasiquoteTerm> ※2");
    // Action1
    this->stateStack.push_back("QUASIQUOTE");
    int nextIndex = this->parseQuasiquoteTerm(index + 1);
    // Action2
    this->stateStack.pop_back();
    return nextIndex;
}

int Parser::parseQuasiquoteTerm(int index) {
    this->parseLog("<QuasiquoteTerm> → <Term>");
    return this->parseTerm(index);
}

int Parser::parseSList(int index) {
    parseLog("<SList> → ( ※ <SListSeq> )");
    string quoteType = this->stateStack.empty() ? "" : this->stateStack.back();
    // sListHandle maybe point to quote, unquote, quasiquote object
    Handle sListHandle;

    if (quoteType == "QUOTE") {
        sListHandle = this->ast.heap.makeQuote(this->ast.moduleName, this->nodeStack.back());
    } else if (quoteType == "QUASIQUOTE") {
        sListHandle = this->ast.heap.makeQuasiquote(this->ast.moduleName, this->nodeStack.back());
    } else if (quoteType == "UNQUOTE") {
        sListHandle = this->ast.heap.makeUnquote(this->ast.moduleName, this->nodeStack.back());
    } else {
        sListHandle = this->ast.heap.makeApplication(this->ast.moduleName, this->nodeStack.back());
    }

    this->nodeStack.push_back(sListHandle);

    ast.nodeSourceIndexesMap[sListHandle] = this->tokens[index].sourceIndex;
    int nextIndex = this->parseSListSeq(index + 1);

    if (this->tokens[nextIndex].string == ")") {
        return nextIndex + 1;
    } else {
        throw runtime_error("<SList> left ) is not found");
    }
}

int Parser::parseSListSeq(int index) {
    parseLog("<SListSeq> → <Term> ※ <SListSeq> | ε");
    string quoteType = this->stateStack.empty() ? "" : this->stateStack.back();

    if (index > this->tokens.size()) {
        throw runtime_error("<SList> left ) is not found");
    }

    auto currentTokenStr = this->tokens[index].string;

    if (currentTokenStr == "(" || currentTokenStr == "'" || currentTokenStr == "," ||
        currentTokenStr == "`" || this->isSymbol(currentTokenStr)) {
        int nextIndex = this->parseTerm(index);

        // Action：从节点栈顶弹出节点，追加到新栈顶节点的children中。
        HandleOrStr childHos = nodeStack.back();
        nodeStack.pop_back();

        if (quoteType == "QUOTE") {
            static_pointer_cast<QuoteObject>(this->ast.heap.get(this->nodeStack.back()))->addChild(childHos);
        } else if (quoteType == "QUASIQUOTE") {
            static_pointer_cast<QuasiquoteObject>(this->ast.heap.get(this->nodeStack.back()))->addChild(childHos);
        } else if (quoteType == "UNQUOTE") {
            static_pointer_cast<UnquoteObject>(this->ast.heap.get(this->nodeStack.back()))->addChild(childHos);
        } else {
            static_pointer_cast<ApplicationObject>(this->ast.heap.get(this->nodeStack.back()))->addChild(childHos);
        }

        nextIndex = this->parseSListSeq(nextIndex);
        return nextIndex;
    } else {
        return index;
    }
}

int Parser::parseSymbol(int index) {
    string currentTokenStr = tokens[index].string;
    if (isSymbol(currentTokenStr)) {
        // Action
        string state = this->stateStack.empty() ? "" : this->stateStack.back();
        Type type = typeOfStr(currentTokenStr);
        if (state == "QUOTE" || state == "QUASIQUOTE") {
            // NUMBER and string in quote are not affected
            if (type == Type::NUMBER) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::STRING) {
                Handle stringHandle = this->ast.heap.makeString(this->ast.moduleName, currentTokenStr);
                this->nodeStack.push_back(stringHandle);
                this->ast.nodeSourceIndexesMap[stringHandle] = tokens[index].sourceIndex;
            } else if (type == Type::SYMBOL) {
                this->nodeStack.push_back(currentTokenStr);
            } else if ((type == Type::VARIABLE || type == Type::KEYWORD || type == Type::PORT) &&
                       currentTokenStr != "quasiquote" && currentTokenStr != "quote" && currentTokenStr != "unquote") {
                //quoted variable, keyword, port is pushed as symbol
                this->nodeStack.push_back("'" + currentTokenStr);
            } else { // 含boolean在内的变量、把柄等
                this->nodeStack.push_back(currentTokenStr);
            }
        } else if (state == "UNQUOTE") {
            // 符号会被解除引用
            if (type == Type::SYMBOL) {
                boost::replace_all(currentTokenStr, "'", "");
                this->nodeStack.push_back(currentTokenStr);
            }
                // 其他所有类型不受影响
            else if (type == Type::NUMBER) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::STRING) {
                Handle stringHandle = this->ast.heap.makeString(this->ast.moduleName, currentTokenStr);
                this->nodeStack.push_back(stringHandle);
                this->ast.nodeSourceIndexesMap[stringHandle] = tokens[index].sourceIndex;
            } else if (type == Type::VARIABLE || type == Type::KEYWORD || type == Type::BOOLEAN || type == Type::PORT) {
                // VARIABLE原样保留，在作用域分析的时候才被录入AST
                this->nodeStack.push_back(currentTokenStr);
            } else {
                throw runtime_error("<Symbol> Illegal symbol");
            }
        } else {
            if (type == Type::NUMBER) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::STRING) {
                Handle stringHandle = this->ast.heap.makeString(this->ast.moduleName, currentTokenStr);
                this->nodeStack.push_back(stringHandle);
                this->ast.nodeSourceIndexesMap[stringHandle] = tokens[index].sourceIndex;
            } else if (type == Type::SYMBOL) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::VARIABLE || type == Type::KEYWORD || type == Type::BOOLEAN || type == Type::PORT) {
                // VARIABLE原样保留，在作用域分析的时候才被录入AST
                this->nodeStack.push_back(currentTokenStr);
            } else {
                throw runtime_error("<Symbol> Illegal symbol");
            }
        }
        return index + 1;
    } else {
        throw runtime_error("<Symbol> Illegal symbol");
    }
}

void Parser::parseLog(const string &msg) {
//    cout << msg << endl;
}

bool Parser::isSymbol(const string &tokenStr) {
    if (tokenStr == "(" || tokenStr == ")" || tokenStr == "{" || tokenStr == "}" || tokenStr == "[" ||
        tokenStr == "]") {
        return false;
    } else if (tokenStr[0] == '\'' || tokenStr[0] == '`' || tokenStr[0] == ',') {
        // shouldn't starts with \' \` and \,
        return false;
    } else {
        // Others are symbol
        return true;
    }
}

void Parser::preProcessAnalysis() {
    for (auto &[handle, schemeObjPtr] : this->ast.heap.dataMap) {
        // Handle the import
        if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
            auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
            if (!applicationObjPtr->childrenHoses.empty() and applicationObjPtr->childrenHoses[0] == "import") {
                // make sure import has more three parameters
                if (applicationObjPtr->childrenHoses.size() != 3) {
                    throw runtime_error(
                            "[preprocess] keyword 'import' receives two parameters: module_alias and module_path");
                } else {
                    // (import Utils handle_to_path->/path/to/module)
                    //     0     1      2
                    string moduleAlias = applicationObjPtr->childrenHoses[1];
                    Handle modulePathHandle = applicationObjPtr->childrenHoses[2];

                    // get the string from handle: handle -> /path/to/module
                    auto stringObjptr = this->ast.heap.get(modulePathHandle);
                    if (stringObjptr->schemeObjectType == SchemeObjectType::STRING) {
                        string modulePath = static_pointer_cast<StringObject>(stringObjptr)->content;
                        modulePath = modulePath.substr(1, modulePath.size() - 2); // trim " on both side

                        // set the alias and the path
                        this->ast.moduleAliasPathMap[moduleAlias] = modulePath;
                    } else {
                        throw runtime_error("[preprocess] module_path should be a string");
                    }
                }
            } else if (!applicationObjPtr->childrenHoses.empty() and applicationObjPtr->childrenHoses[0] == "native") {
                if (applicationObjPtr->childrenHoses.size() < 2) {
                    throw runtime_error("[preprocess] keyword 'native' has less than 2 variable");
                } else {
                    string native = applicationObjPtr->childrenHoses[1];
                    this->ast.natives[native] = "enabled";
                    // TODO: how to define native call?
                }
            }
        }
    }
}


#endif //TYPED_SCHEME_PARSER_HPP
