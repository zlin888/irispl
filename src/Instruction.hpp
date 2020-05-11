//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_INSTRUCTION_HPP
#define TYPED_SCHEME_INSTRUCTION_HPP

#include "Utils.hpp"
#include "IrisObject.hpp"
#include <string>
#include <regex>

enum class InstructionType {
    LABEL, COMMENT, INSTRUCTION
};

typedef Type InstructionArgumentType;

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

InstructionArgumentType Instruction::getArgumentType(string arg) {
    return typeOfStr(arg);
};


#endif //TYPED_SCHEME_INSTRUCTION_HPP
