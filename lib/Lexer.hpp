//
// Created by Zhitao Lin on 2020/2/13.
//

#ifndef TYPED_SCHEME_LEXER_HPP
#define TYPED_SCHEME_LEXER_HPP

#include <string>
#include <utility>
#include <vector>
#include <regex>
#include <iostream>
#include <stdexcept>

using namespace std;

namespace Lexer {

    class Token {
    public:

        std::string string;
        int index;

        Token(std::string string, int index) : string(std::move(string)), index(index) {};

        friend std::ostream &operator<<(std::ostream &os, const Token &token);
    };

    std::ostream &operator<<(std::ostream &os, const Token &token) {
        os << token.string << " / " << std::to_string(token.index) << "\n";
        return os;
    }

    std::vector<Token> lexer(std::string code) {
        // TODO: deal with escaped chars
        code = code + "\n";

        std::vector<Token> tokens;
        std::vector<std::string> tempTokens;

        for (int i = 0; i < code.size(); ++i) {
            // pass comment
            if (code[i] == ';') {
                while (code[i] != '\n' && code[i] != '\r') {
                    i++;
                }
                continue;
            } else if (code[i] == '(' || code[i] == ')' || code[i] == '[' || code[i] == ']' || code[i] == '{' ||
                       code[i] == '}' || code[i] == '\'' || code[i] == ',' || code[i] == '`' || code[i] == '"') {
                if (!tempTokens.empty()) {
                    std::string newTokenStr;
                    for (std::string &c : tempTokens) {
                        newTokenStr += c;
                    }
                    tokens.push_back(Token(newTokenStr, i - newTokenStr.size()));
                    tempTokens.clear();
                }
                // Matching String Token
                if (code[i] == '"') {
                    int originIndex = i;
                    // ? means greedy matching
                    string resultStr = "\"";
                    i += 1;
                    bool stringMatchFlag = false;

                    while (i < code.size()) {
                        if (code[i] == '"') {
                            resultStr += code[i];
                            stringMatchFlag = true;
                            i += 1;
                            break;
                        } else if (code[i] == '\\' && i + 1 < code.size()) {
                            resultStr += code[i] + code[i + 1];
                            i += 2;
                        } else {
                            resultStr += code[i];
                            i += 1;
                        }
                    }

                    if (!stringMatchFlag) {
                        // Doesn't match a proper string
                        throw std::runtime_error("[error] can't match '\"'" + code.substr(originIndex,
                                                                                          originIndex + 5 < code.size()
                                                                                          ? originIndex + 5
                                                                                          : code.size()) +
                                                 "... : Lexer::lexer_string_matching");
                    } else {
                        tokens.push_back(Lexer::Token(resultStr, i));
                    }
                } else {
                    tokens.push_back(Lexer::Token(std::string(1, code[i]), i));
                }

            } else if (code[i] == ' ' || code[i] == '\t' || code[i] == '\n' || code[i] == '\r') {
                if (!tempTokens.empty()) {
                    std::string newTokenStr;
                    for (std::string &c : tempTokens) {
                        newTokenStr += c;
                    }
                    tokens.push_back(Token(newTokenStr, i - newTokenStr.size()));
                    tempTokens.clear();
                }
            } else {
                tempTokens.push_back(std::string(1, code[i]));
            }
        }
        return tokens;
    }
}

#endif //TYPED_SCHEME_LEXER_HPP