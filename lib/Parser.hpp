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
    Heap heap;
    string moduleName;
    string source;
    map<Handle, int> nodeSourceIndexes;

};

class Parser {
public:
    vector<string> stateStack;
    vector<HandleOrStr> nodeStack; //nodeStack can store both Handle and string

    // One Parser has one AST !!!
    AST ast;
    vector<Lexer::Token> tokens;

    AST parse();

    int parseTerm(int index);

    Parser(const vector<Lexer::Token> &tokens, const string &moduleName) : tokens(tokens) {
        this->nodeStack.push_back(TOP_NODE_HANDLE);
        this->ast.moduleName = moduleName;
    }

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
};


AST Parser::parse() {
    this->parseTerm(0);
    return this->ast;
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
            throw "[Error] quote 右侧括号未闭合。";
        }
    } else if (this->tokens[index].string == "(" && this->tokens[index + 1].string == "unquote") {
        this->parseLog("<Term> → <Unquote>");
        nextIndex = this->parseUnquote(index + 1);
        if (this->tokens[nextIndex].string == ")") { return nextIndex + 1; }
        else {
            throw "[Error] unquote 右侧括号未闭合。";
        }
    } else if (this->tokens[index].string == "(" && this->tokens[index + 1].string == "quasiquote") {
        this->parseLog("<Term> → <Quasiquote>");
        nextIndex = parseQuasiquote(index + 1);
        if (this->tokens[nextIndex].string == ")") { return nextIndex + 1; }
        else {
            throw "[Error] quasiquote 右侧括号未闭合。";
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
        throw "undefined token " + this->tokens[index].string;
    }
}

int Parser::parseLambda(int index) {
    this->parseLog("<Lambda> → ( ※ lambda <ArgList> <Body> )");

    Handle lambdaHandle = this->ast.heap.makeLambda(this->ast.moduleName, this->nodeStack.back());
    this->nodeStack.push_back(lambdaHandle);

    ast.nodeSourceIndexes[lambdaHandle] = this->tokens[index].sourceIndex;

    int nextIndex = this->parseArgList(index + 2);
    nextIndex = this->parseBody(nextIndex);

    if (this->tokens[nextIndex].string == ")") {
        return nextIndex + 1;
    } else {
        throw "<Lambda> ')' is not found -- in " + to_string(tokens[index].sourceIndex);
    }
}

int Parser::parseArgList(int index) {
    this->parseLog("<ArgList> → ( ※1 <ArgListSeq> ※2)");

    if (tokens[index].string != "(") {
        throw "<ArgList> '(' is not found -- in " + to_string(tokens[index].sourceIndex);
    }

    this->stateStack.push_back("PARAMETER");
    int nextIndex = this->parseArgListSeq(index + 1);
    // Action2
    this->stateStack.pop_back();

    if (tokens[nextIndex].string == ")") {
        return nextIndex + 1;
    } else {
        throw "<ArgList> ')' is not found -- in " + to_string(tokens[index].sourceIndex);
    }
}

int Parser::parseArgListSeq(int index) {
    this->parseLog("<ArgListSeq> → <ArgSymbol> ※ <ArgListSeq> | ε");
    if (this->isSymbol(tokens[index].string)) {
        int nextIndex = this->parseArgSymbol(index);

        // get symbol from nodeStack, and add it to the parameters of lamdba node
        string parameter = this->nodeStack.back();
        this->nodeStack.pop_back();
        static_pointer_cast<LambdaObject>(this->ast.heap.get(this->nodeStack.back()))->addParameter(parameter);

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
    int nextIndex = this->parseUnquoteTerm(index+1);
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
    int nextIndex = this->parseQuasiquoteTerm(index+1);
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
    Handle sListHandle = this->ast.heap.makeApplication(this->ast.moduleName, this->nodeStack.back(), quoteType);

    this->nodeStack.push_back(sListHandle);

    ast.nodeSourceIndexes[sListHandle] = this->tokens[index].sourceIndex;
    int nextIndex = this->parseSListSeq(index + 1);

    if (this->tokens[nextIndex].string == ")") {
        return nextIndex + 1;
    } else {
        throw "<SList> left ) is not found";
    }
}

int Parser::parseSListSeq(int index) {
    parseLog("<SListSeq> → <Term> ※ <SListSeq> | ε");
    string quoteType = this->stateStack.empty() ? "" : this->stateStack.back();

    if (index >= this->tokens.size())
        throw runtime_error("<SList> left ) is not found");

    auto currentTokenStr = this->tokens[index].string;

    if (currentTokenStr == "(" || currentTokenStr == "'" || currentTokenStr == "," ||
        currentTokenStr == "`" || this->isSymbol(currentTokenStr)) {
        int nextIndex = this->parseTerm(index);

        // Action：从节点栈顶弹出节点，追加到新栈顶节点的children中。
        HandleOrStr childHos = nodeStack.back();
        nodeStack.pop_back();
        if (quoteType.empty()) {
            //nodeStack.back() is a handle to application
            static_pointer_cast<ApplicationObject>(this->ast.heap.get(this->nodeStack.back()))->addChild(childHos);
        }
        // TODO: nodeStack.back() is a handle to quote, unquote, quasiquoteand
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
        string state = this->stateStack.back();
        Type type = typeOfStr(currentTokenStr);
        if (state == "QUOTE" || state == "QUASIQUOTE") {
            // NUMBER and string in quote are not affected
            if (type == Type::NUMBER) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::STRING) {
                Handle stringHandle = this->ast.heap.makeString(this->ast.moduleName, currentTokenStr);
                this->nodeStack.push_back(stringHandle);
                this->ast.nodeSourceIndexes[stringHandle] = tokens[index].sourceIndex;
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
                this->ast.nodeSourceIndexes[stringHandle] = tokens[index].sourceIndex;
            } else if (type == Type::VARIABLE || type == Type::KEYWORD || type == Type::BOOLEAN || type == Type::PORT) {
                // VARIABLE原样保留，在作用域分析的时候才被录入AST
                this->nodeStack.push_back(currentTokenStr);
            } else {
                throw "<Symbol> Illegal symbol";
            }
        } else {
            if (type == Type::NUMBER) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::STRING) {
                Handle stringHandle = this->ast.heap.makeString(this->ast.moduleName, currentTokenStr);
                this->nodeStack.push_back(stringHandle);
                this->ast.nodeSourceIndexes[stringHandle] = tokens[index].sourceIndex;
            } else if (type == Type::SYMBOL) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::VARIABLE || type == Type::KEYWORD || type == Type::BOOLEAN || type == Type::PORT) {
                // VARIABLE原样保留，在作用域分析的时候才被录入AST
                this->nodeStack.push_back(currentTokenStr);
            } else {
                throw "<Symbol> Illegal symbol";
            }
        }
        return index + 1;
    } else {
        throw "<Symbol> Illegal symbol";
    }
}

void Parser::parseLog(const string &msg) {
    cout << msg << endl;
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


#endif //TYPED_SCHEME_PARSER_HPP
