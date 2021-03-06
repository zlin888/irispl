#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <stdexcept>
#include <memory>
#include "Instruction.hpp"
#include "Utils.hpp"
#include "ModuleLoader.hpp"
#include "IrisObject.hpp"
#include "Heap.hpp"

using namespace std;

typedef int PID;

enum class ProcessState {
    READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
};


class StackFrame {
public:
    StackFrame(std::shared_ptr<Closure> closure, int returnAddress) : closurePtr(closure),
                                                                      returnAddress(returnAddress) {}

    std::shared_ptr<Closure> closurePtr;
    int returnAddress;
};

class Process {

public:
    vector<string> opStack;
    vector<StackFrame> fStack;
    vector<Instruction> instructions;
    map<string, int> labelAddressMap;
    ProcessState state = ProcessState::READY;
    Heap heap;
    AST ast;
    PID pid = 0;
    int PC = 0;
    std::shared_ptr<Closure> currentClosurePtr;

    Process(PID newPid, const Module &module);

    Instruction currentInstruction();
    Instruction nextInstruction();

    inline void step() { this->PC++; };

    string popOperand();

    void pushStackFrame(shared_ptr<Closure> closurePtr, int returnAddress);

    StackFrame popStackFrame();

    void pushOperand(const string &value);

    string dereference(const string &variableName);

    void pushCurrentClosure(int returnAddress);

    Handle newClosure(int instructionAddress);

    shared_ptr<struct Closure> getClosurePtr(Handle closureHandle);

    void setCurrentClosure(Handle closureHandle);

    void gotoAddress(int instructionAddress);

private:
    void initLabelLineMap();

};

//=================================================================
//                    Preprocess
//=================================================================
void Process::initLabelLineMap() {
    // Label is normally the indication of beginning of a function (lambda)
    // this mapping represents the mapping between the label and the sourceIndex of the corresponding instructionStr
    for (int i = 0; i < this->instructions.size(); ++i) {
        Instruction instruction = this->instructions[i];
        if (instruction.type == InstructionType::LABEL) {
            this->labelAddressMap[instruction.instructionStr] = i;
        }
    }
}



//=================================================================
//                    PROCESS
//=================================================================

Instruction Process::currentInstruction() {
    return this->instructions[PC];
}

Instruction Process::nextInstruction(){
    return this->instructions[PC + 1];
}

Process::Process(PID newPid, const Module &module) {
    this->pid = newPid;

    // from module loader loads instructionStr (string -> instructionStr)
    for (auto &i : module.ILCode) {
        this->instructions.emplace_back(Instruction(i));
    }

    // The top closure (not need to worry about this, because this is just a lambda (closure) acted as a beginner
    // > at the top of everything
    this->currentClosurePtr = std::shared_ptr<Closure>(new Closure(-1, nullptr, TOP_NODE_HANDLE));
    this->heap = module.ast.heap;
    this->ast = module.ast;
    this->heap.set(TOP_NODE_HANDLE, this->currentClosurePtr);

    this->initLabelLineMap();
};

void Process::pushOperand(const string &value) {
    this->opStack.push_back(value);
}

string Process::popOperand() {
    if (this->opStack.empty()) {
        throw std::overflow_error("[ERROR] pop from empty opStack : Process::popOperand");
    } else {
        string popValue = this->opStack.back();
        this->opStack.pop_back();
        return popValue;
    }
}

// Process Closure related

void Process::pushStackFrame(std::shared_ptr<Closure> closurePtr, int returnAddress) {
    StackFrame sf(closurePtr, returnAddress);
    this->fStack.push_back(sf);
}

StackFrame Process::popStackFrame() {
    if (this->fStack.empty()) {
        throw std::overflow_error("[ERROR] pop from empty fStack : Process::popStackFrame");
    } else {
        StackFrame sf = this->fStack.back();
        this->fStack.pop_back();
        return sf;
    }
}

void Process::pushCurrentClosure(int returnAddress) {
    StackFrame sf(this->currentClosurePtr, returnAddress);
    this->fStack.push_back(sf);
}

string Process::dereference(const string &variableName) {
    // if variable is bounded, return it
    if (this->currentClosurePtr->hasBoundVariable(variableName)) {
        return this->currentClosurePtr->getBoundVariable(variableName);
    }

    // if variable is free,
    if (this->currentClosurePtr->hasFreeVariable(variableName)) {
        string freeVariableValue = this->currentClosurePtr->getFreeVariable(variableName);

        auto closurePtr = this->currentClosurePtr;
        auto topClosurePtr = this->getClosurePtr(TOP_NODE_HANDLE);
        while (closurePtr != topClosurePtr) {
            if (closurePtr->hasBoundVariable(variableName)) {
                string boundVariableValue = closurePtr->getBoundVariable(variableName);
                if (freeVariableValue != boundVariableValue) {
                    if (closurePtr->isDirtyVairable(variableName)) {
                        // If set! is used in one of the parentHandle closures to change the variable value in the closure
                        // where the varaible is bounded, the return variable will refer to the set! value
                        return boundVariableValue;
                    } else {
                        // If define is used to change the variable value, the variable value in the children closure
                        // will be return
                        return freeVariableValue;
                    }
                } else {
                    return freeVariableValue;
                }
            }
            closurePtr = closurePtr->parentClosurePtr;
        }
    }

    throw std::runtime_error("'" + variableName + "'" + "is undefined : dereference");
    // from current closure backtrack to the top_node_handle
}

Handle Process::newClosure(int instructionAddress) {
    Handle newClosureHandle = this->heap.allocateHandle(IrisObjectType::CLOSURE);
    this->heap.set(newClosureHandle, std::shared_ptr<Closure>(
            new Closure(instructionAddress, this->currentClosurePtr, newClosureHandle)));
    return newClosureHandle;
}

shared_ptr<Closure> Process::getClosurePtr(Handle closureHandle) {
    auto schemeObjectPtr = this->heap.get(closureHandle);
    if (schemeObjectPtr->irisObjectType == IrisObjectType::CLOSURE) {
        return static_pointer_cast<Closure>(schemeObjectPtr);
    } else {
        throw std::invalid_argument("[ERROR] Handle is not a closureHandle : Process::getClosurePtr");
    }
}

void Process::setCurrentClosure(Handle closureHandle) {
    auto closurePtr = this->getClosurePtr(closureHandle);
    this->currentClosurePtr = closurePtr;
}

void Process::gotoAddress(int instructionAddress) {
    this->PC = instructionAddress;
}

#endif // !PROCESS_HPP