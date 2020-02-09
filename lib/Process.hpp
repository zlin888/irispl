#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <utility>
#include <vector>
#include <map>
#include <stdexcept>
#include "Instruction.hpp"
#include "Utils.hpp"
#include "ModuleLoader.hpp"

using namespace std;

typedef int PID;
typedef string Handle;

enum ProcessState {
    READY, RUNNING, SLEEPING, SUSPENDED, STOPPED
};

// Used as Memory
class Heap {
public:
    map<Handle, Closure> dataMap;
    int handleCounter;

    bool hasHandle(Handle handle);

    closure get(Handle handle);
};

class Closure {
public:

    int instructionAddress{};
    map<string, string> boundVariables;
    map<string, string> freeVariables;
    map<string, bool> dirtyFlags;
    Closure *parentClosure;

    Closure(int instructionAddress, Closure *parentClosure) : instructionAddress(instructionAddress),
                                                              parentClosure(parentClosure) {}

    void setBoundVariable(const string &variable, const string &value, bool dirtyFlag);

    string getBoundVariable(const string &variable);

    void setFreeVariable(const string &variable, const string &value, bool dirtyFlag);

    string getFreeVariable(const string &variable);
};

class StackFrame {
public:
    StackFrame(const Closure &closure, int returnAddress) : closure(closure), returnAddress(returnAddress) {}

    Closure closure;
    int returnAddress;
};

class Process {
public:
    vector<string> opStack;
    vector<StackFrame> fStack;
    vector<Instruction> instructions;
    ProcessState state = READY;
    PID pid = 0;
    int PC = 0;

    Process() {};

    Process(PID newPid, const ModuleLoader &moduleLoader);

    Instruction currentInstruction();

    inline void step() { this->PC++; };

    string popOperand();

    void pushStackFrame(const Closure &closure, int returnAddress);

    StackFrame popStackFrame();

    void pushOperand(const string &value);

    string dereference(const string &variableName);
};


//=================================================================
//                    PROCESS
//=================================================================

Instruction Process::currentInstruction() {
    return this->instructions[PC];
}

Process::Process(PID newPid, const ModuleLoader &moduleLoader) {
    this->pid = newPid;
    // from module loader loads instruction (string -> instruction)
    for (auto &i : moduleLoader.ILCode) {
        this->instructions.emplace_back(Instruction(i));
    }
};

void Process::pushOperand(const string &value) {
    this->opStack.push_back(value);
}

string Process::popOperand() {
    string popValue = this->opStack.back();
    this->opStack.pop_back();
    return popValue;
}

void Process::pushStackFrame(const Closure &closure, int returnAddress) {
    StackFrame sf(closure, returnAddress);
    this->fStack.push_back(sf);
}

StackFrame Process::popStackFrame() {
    StackFrame sf = this->fStack.back();
    return sf;
}

string Process::dereference(const string &variableName) {

}





//=================================================================
//                    Closure's Closure
//=================================================================

void Closure::setBoundVariable(const string &variable, const string &value, bool dirtyFlag) {
    this->boundVariables[variable] = value;
    this->dirtyFlags[variable] = dirtyFlag;
}

string Closure::getBoundVariable(const string &variable) {
    return this->boundVariables[variable];
}

void Closure::setFreeVariable(const string &variable, const string &value, bool dirtyFlag) {
    this->freeVariables[variable] = value;
    this->dirtyFlags[variable] = dirtyFlag;
}

string Closure::getFreeVariable(const string &variable) {
    return this->freeVariables[variable];
}

//=================================================================
//                           Heap
//=================================================================

bool Heap::hasHandle(Handle handle) {
    return this->dataMap.count(handle);
}

closure Heap::get(Handle handle) {
    if (this->hasHandle(std::move(handle))) {
        return this->dataMap[handle];
    } else {
        throw std::out_of_range("[ERROR] handle holds nothing -- Heap::get");
    }
}


#endif // !PROCESS_HPP