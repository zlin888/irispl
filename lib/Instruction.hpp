//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_INSTRUCTION_HPP
#define TYPED_SCHEME_INSTRUCTION_HPP

#include "Utils.hpp"
#include <string>

enum class InstructionType {
    LABEL, COMMENT, INSTRUCTION
};

using namespace std;

class Instruction{
public:
    InstructionType type;
    string argumentType{};
    string instruction{};
    string mnemonic{};
    string argument{};
    explicit Instruction(string instString);

private:
    string getArgumentType();
};


Instruction::Instruction(string instString){
    //process the instString and construct Instruction object
    instString = utils::trim(instString);
    if (instString[0] == ';'){
        this->type = InstructionType::COMMENT;
        this->instruction = instString;
    } else if(instString[0] == '@'){
        this->type = InstructionType::LABEL;
        this->instruction = instString;
    } else {
        this->type = InstructionType::INSTRUCTION;
        this->instruction = instString;

        //Split instruction string by whitespace, the first split is mnemonic and the last is argument
        string delimiter = " ";
        int splitIndex = instString.find(delimiter);

        this->mnemonic = instString.substr(0, splitIndex);
        if (splitIndex != -1 && splitIndex + 1 < instString.size()) {
            this->argument = instString.substr(splitIndex + 1, instString.size());
        }

        this->argumentType = this->getArgumentType();
        cout << "arg -> " + this->argument + " -- " + this->argumentType + "\n";
//        string fields = instString.split(/\s+/i);
//        this->mnemonic = fields[0].toLowerCase();
//        this->argument = fields[1];
    }
}

// TODO
// NUMBER? BOOLEAN? VARIABLE?
// Doesn't really solve the type of parameters problem
string Instruction::getArgumentType() {
    string arg = this->argument;
    if (arg.empty()) {
        return "";
    } else if (arg == "lambda") {
        return "LAMBDA";
    } else if (arg[0] == ':') {
        return "PORT";
    } else if (arg[0] == '&') {
        return "HANDLE";
    } else if (arg[0] == '\'') {
        return "SYMBOL";
    } else if (arg[0] == '@') {
        return "LABEL";
    } else if (arg[0] == '"' && arg[arg.size() - 1] == '"') {
        return "STRING";
    } else {
        return "VARIABLE";
    }
};


#endif //TYPED_SCHEME_INSTRUCTION_HPP
