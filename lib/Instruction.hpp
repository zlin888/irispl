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

enum class ArgumentType {
    UNDEFINDED, LAMBDA, PORT, HANDLE, SYMBOL, LABEL, VARIABEL, STRING
};

using namespace std;

class Instruction{
public:
    InstructionType type;
    ArgumentType argumentType{};
    string instructionStr{};
    string mnemonic{};
    string argument{};
    explicit Instruction(string instString);

private:
    ArgumentType getArgumentType();
};


Instruction::Instruction(string instString){
    //process the instString and construct Instruction object
    instString = utils::trim(instString);
    if (instString[0] == ';'){
        this->type = InstructionType::COMMENT;
        this->instructionStr = instString;
        this->argument = instString;
    } else if(instString[0] == '@'){
        this->type = InstructionType::LABEL;
        this->instructionStr = instString;
        this->argument = instString;
    } else {
        this->type = InstructionType::INSTRUCTION;
        this->instructionStr = instString;

        //Split instructionStr string by whitespace, the first split is mnemonic and the last is argument
        string delimiter = " ";
        int splitIndex = instString.find(delimiter);

        this->mnemonic = instString.substr(0, splitIndex);
        if (splitIndex != -1 && splitIndex + 1 < instString.size()) {
            this->argument = instString.substr(splitIndex + 1, instString.size());
        }

        this->argumentType = this->getArgumentType();
//        cout << "arg -> " + this->argument + " -- " + this->argumentType + "\n";
//        string fields = instString.split(/\s+/i);
//        this->mnemonic = fields[0].toLowerCase();
//        this->argument = fields[1];
    }
}

// TODO
// NUMBER? BOOLEAN? VARIABLE?
// Doesn't really solve the type of parameters problem
ArgumentType Instruction::getArgumentType() {
    string arg = this->argument;
    if (arg.empty()) {
        return ArgumentType::UNDEFINDED;
    } else if (arg == "lambda") {
        return ArgumentType::LAMBDA;
    } else if (arg[0] == ':') {
        return ArgumentType::PORT;
    } else if (arg[0] == '&') {
        return ArgumentType::HANDLE;
    } else if (arg[0] == '\'') {
        return ArgumentType::SYMBOL;
    } else if (arg[0] == '@') {
        return ArgumentType::LABEL;
    } else if (arg[0] == '"' && arg[arg.size() - 1] == '"') {
        return ArgumentType::STRING;
    } else {
        return ArgumentType::VARIABEL;
    }
};


#endif //TYPED_SCHEME_INSTRUCTION_HPP
