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
    vector<Handle> nodeStack;

    // One Parser has one AST !!!
    AST ast;
    vector<Lexer::Token> tokens;

    void parse(const string &moduleName);

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

    int parseQuote(int index);

    int parseUnquote(int index);

    int parseQuasiquote(int index);

    int parseSList(int index);

    int parseSymbol(int index);

    int parseQuoteTerm(int index);

    int parseSListSeq(int index);
};


void Parser::parse(const string &moduleName) {
    this->parseTerm(0);
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
    int nextIndex = this->parseBody(nextIndex);

    if (this->tokens[nextIndex].string == ')') { return nextIndex + 1; }
    else { throw "<Lambda>"; }
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

// Action：从节点栈顶弹出节点（必须是符号），追加到新栈顶Lambda节点的parameters中。
        Handle parameter = this->nodeStack.pop();
        ast.GetNode(Top(NODE_STACK)).addParameter(parameter);

        nextIndex = ParseArgListSeq(tokens, nextIndex);
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
    let bodyNode = NODE_STACK.pop();
    ast.GetNode(Top(NODE_STACK)).addBody(bodyNode);

    nextIndex = ParseBodyTail(tokens, nextIndex);
    return nextIndex;
}

int Parser::parseQuote(int index) {
    parseLog("<Quote> → \' ※1 <QuoteTerm> ※2");
    // Action1
    this->stateStack.push_back("QUOTE");
    int nextIndex = this->parseQuoteTerm(index + 1);
    // Action2
    this->stateStack.pop_back();
    return nextIndex;
}

int Parser::parseQuoteTerm(int index) {
    parseLog("<QuoteTerm> → <Term>");
    return parseTerm(this->tokens, index);
}

int Parser::parseUnquote(int index) {

}

int Parser::parseQuasiquote(int index) {

}

int Parser::parseSList(int index) {
    parseLog("<SList> → ( ※ <SListSeq> )");

    string quoteType = this->stateStack.empty() ? "" : this->stateStack.back();
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

    if (index >= this->tokens.size())
        throw "<SList> left ) is not found";

    auto currentTokenStr = this->tokens[index].string;

    if (currentTokenStr == "(" || currentTokenStr == "'" || currentTokenStr == "," ||
        currentTokenStr == "`" || this->isSymbol(currentTokenStr)) {
        int nextIndex = this->parseTerm(index);

        // Action：从节点栈顶弹出节点，追加到新栈顶节点的children中。
        Handle childHandle = nodeStack.back();
        nodeStack.pop_back();
        this->ast.heap.get(nodeStack.back());
//        ast.GetNode(Top(NODE_STACK)).children.push(childHandle);

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
        if (state == "QUOTE" || state == "QUASIQUOTE") {
            Type type = typeOfStr(currentTokenStr);
            // 被quote的常量和字符串不受影响
            if (type == Type::NUMBER) {
                this->nodeStack.push_back(currentTokenStr);
            } else if (type == Type::STRING) {
                Handle stringHandle = this->ast.heap.makeString(currentTokenStr);
                this->nodeStack.push_back(stringHandle);
                this->ast.nodeSourceIndexes[stringHandle] = tokens[index].sourceIndex;
            } else if (type == Type::SYMBOL) {
                this->nodeStack.push_back(currentTokenStr);
            }
                // 被quote的变量和关键字（除了quote、unquote和quasiquote），变成symbol
            else if (type == Type::VARIABLE || type == "KEYWORD" || type == "PORT" ||
                     (currentTokenStr !=
                      "quasiquote" && currentTokenStr !=
                                      "quote" &&
                      currentTokenStr != "unquote")) {
                NODE_STACK.push(`'${currentToken}`);
            } else { // 含boolean在内的变量、把柄等
                NODE_STACK.push(currentTokenStr);
            }
        } else if (state == = 'UNQUOTE') {
            let type = TypeOfToken(currentTokenStr);
            // 符号会被解除引用
            if (type == = "SYMBOL") {
                NODE_STACK.push(currentTokenStr.replace( / ^\'*/gi, "")); // VARIABLE
            }
                // 其他所有类型不受影响
            else if (type == = "NUMBER") {
                NODE_STACK.push(parseFloat(currentTokenStr));
            } else if (type == = "STRING") {
                let stringHandle = ast.MakeStringNode(currentTokenStr);
                NODE_STACK.push(stringHandle);
                ast.nodeIndexes.set(stringHandle, tokens[index].index);
            } else if (type == = "VARIABLE" || type == = "KEYWORD" || type == = "BOOLEAN" || type == = "PORT") {
                NODE_STACK.push(currentTokenStr); // VARIABLE原样保留，在作用域分析的时候才被录入AST
            } else {
                throw `<Type> Illegal
                symbol.`
            }
        } else {
            let type = TypeOfToken(currentTokenStr);
            if (type == = "NUMBER") {
                NODE_STACK.push(parseFloat(currentTokenStr));
            } else if (type == = "STRING") {
                let stringHandle = ast.MakeStringNode(currentTokenStr);
                NODE_STACK.push(stringHandle);
                ast.nodeIndexes.set(stringHandle, tokens[index].index);
            } else if (type == = "SYMBOL") {
                NODE_STACK.push(currentTokenStr);
            } else if (type == = "VARIABLE" || type == = "KEYWORD" || type == = "BOOLEAN" || type == = "PORT") {
                NODE_STACK.push(currentTokenStr); // VARIABLE原样保留，在作用域分析的时候才被录入AST
            } else {
                throw `<Type> Illegal
                symbol.`
            }
        }
        return index + 1;
    } else {
        throw "<Symbol>";
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
