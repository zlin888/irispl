//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_INSTRUCTION_HPP
#define TYPED_SCHEME_INSTRUCTION_HPP

#include "Utils.hpp"
#include <string>

enum InstructionType { LABEL, COMMENT, INSTRUCTION };

using namespace std;

class Instruction{
public:
    InstructionType type;
    string argType{};
    string instruction{};
    string mnemonic{};
    string argument{};
    explicit Instruction(string instString);
};


Instruction::Instruction(string instString){
    //process the instString and construct Instruction object
    instString = utils::trim(instString);
    if (instString[0] == ';'){
        this->type = COMMENT;
        this->instruction = instString;
    } else if(instString[0] == '@'){
        this->type = LABEL;
        this->instruction = instString;
    } else {
        this->type = INSTRUCTION;
        this->instruction = instString;
//        string fields = instString.split(/\s+/i);
//        this->mnemonic = fields[0].toLowerCase();
//        this->argument = fields[1];
    }
};

#endif //TYPED_SCHEME_INSTRUCTION_HPP
