//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_INSTRUCTION_HPP
#define TYPED_SCHEME_INSTRUCTION_HPP

#include "Utils.hpp"
#include <string>
#include <regex>

enum class InstructionType {
    LABEL, COMMENT, INSTRUCTION
};

enum class InstructionArgumentType {
    UNDEFINED, LAMBDA, PORT, HANDLE, SYMBOL, LABEL, VARIABLE, STRING, NUMBER
};

using namespace std;

class Instruction{
public:
    InstructionType type;
    InstructionArgumentType argumentType{};
    string instructionStr{};
    string mnemonic{};
    string argument{};
    explicit Instruction(string instString);

    static InstructionArgumentType getArgumentType(string arg);
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

        this->argumentType = this->getArgumentType(this->argument);
//        cout << "arg -> " + this->argument + " -- " + this->argumentType + "\n";
//        string fields = instString.split(/\s+/i);
//        this->mnemonic = fields[0].toLowerCase();
//        this->argument = fields[1];
    }
}

// TODO
// NUMBER? BOOLEAN? VARIABLE?
// Doesn't really solve the type of parameters problem
InstructionArgumentType Instruction::getArgumentType(string arg) {
    if (arg.empty()) {
        return InstructionArgumentType::UNDEFINED;
    } else if (arg == "lambda") {
        return InstructionArgumentType::LAMBDA;
    } else if (arg[0] == ':') {
        return InstructionArgumentType::PORT;
    } else if (arg[0] == '&') {
        return InstructionArgumentType::HANDLE;
    } else if (arg[0] == '\'') {
        return InstructionArgumentType::SYMBOL;
    } else if (arg[0] == '@') {
        return InstructionArgumentType::LABEL;
    } else if (arg[0] == '"' && arg[arg.size() - 1] == '"') {
        return InstructionArgumentType::STRING;
    } else if (std::regex_match(arg, std::regex("(-?[0-9]+([.][0-9]+)?)"))) {
        return InstructionArgumentType::NUMBER;
    } else {
        return InstructionArgumentType::VARIABLE;
    }
};


#endif //TYPED_SCHEME_INSTRUCTION_HPP
